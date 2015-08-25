/* PiKrellCam
|
|  Copyright (C) 2015 Bill Wilson    billw@gkrellm.net
|
|  PiKrellCam is free software: you can redistribute it and/or modify it
|  under the terms of the GNU General Public License as published by
|  the Free Software Foundation, either version 3 of the License, or
|  (at your option) any later version.
|
|  PiKrellCam is distributed in the hope that it will be useful, but WITHOUT
|  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
|  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
|  License for more details.
|
|  You should have received a copy of the GNU General Public License
|  along with this program. If not, see http://www.gnu.org/licenses/
|
|  This file is part of PiKrellCam.
*/

#include "pikrellcam.h"
#include <signal.h>
#include <locale.h>


PiKrellCam	pikrellcam;
TimeLapse	time_lapse;

static char	*pgm_name;
static boolean	quit_flag;

  /* Substitute fmt_arg into str to replace a "$V" substitution variable. 
  |  "str" argument must be allocated memory.
  */
char *
substitute_var(char *str, char V, char *fmt_arg)
	{
	char	*fmt, *copy;

	if (V < 'A' || !fmt_arg)
		return str;
	for (fmt = str; (fmt = strchr(fmt, '$')) != NULL; ++fmt)
		{
	    if (*(fmt + 1) == V)
			{
			*fmt = '%';
			*(fmt + 1) = 's';
			asprintf(&copy, str, fmt_arg);
			free(str);
			str = copy;
			break;
			}
		}
	return str;
	}

  /* Make a media pathname out of a filename which can have embedded strftime()
  |  conversions and up to two pikrellcam filename substitution variables.
  |  Return string is allocated memory.
  */
char *
media_pathname(char *dir, char *fname,
				char var1, char *arg1, char var2, char *arg2)
	{
	char buf[200], *path;

	/* Do strftime() first to get rid of '%' specifiers in fname.
	*/
	buf[0] = '\0';
	strftime(buf, sizeof(buf), fname, &pikrellcam.tm_local);
	asprintf(&path, "%s/%s", dir, buf);

	/* Now process any $V substitution variables.
	*/
	path = substitute_var(path, var1, arg1);
	path = substitute_var(path, var2, arg2);
	return path;
	}

char *
fname_base(char *path)
	{
	char *s, *slash;

	for (s = path, slash = NULL; *s; ++s)
		if (*s == '/')
			slash = s;
	return slash ? slash + 1 : path;
	}

void
log_printf_no_timestamp(char *fmt, ...)
	{
	va_list	args;
	FILE	*f;

	va_start(args, fmt);
	if (pikrellcam.verbose)
		vfprintf(stderr, fmt, args);
	if ((f = fopen(pikrellcam.log_file, "a")) != NULL)
		{
		vfprintf(f, fmt, args);
		fclose(f);
		}
	va_end(args);
	}

void
log_printf(char *fmt, ...)
	{
	va_list	args;
	FILE	*f;
	char	tbuf[32];

	va_start(args, fmt);
	strftime(tbuf, sizeof(tbuf), "%T", localtime(&pikrellcam.t_now));
	if (pikrellcam.verbose)
		{
		fprintf(stderr, "%s : ", tbuf);
		vfprintf(stderr, fmt, args);
		}
	if ((f = fopen(pikrellcam.log_file, "a")) != NULL)
		{
		fprintf(f, "%s : ", tbuf);
		vfprintf(f, fmt, args);
		fclose(f);
		}
	va_end(args);
	}

void
camera_start(void)
	{
	MMAL_STATUS_T	status;

	motion_init();
	circular_buffer_init();

	camera_create();

	/* ====== Create the camera preview port path ====== :
	|  preview --(tunnel)--> resizer --> I420_callback --> jpeg_encoder --> mjpeg_callback
	|                                   (draws on frame)                   (writes stream mjpeg.jpg)
	*/
	pikrellcam.mjpeg_height = pikrellcam.mjpeg_width *
					pikrellcam.camera_config.video_height / pikrellcam.camera_config.video_width;
	pikrellcam.mjpeg_width &= ~0xf;		/* Make resize multiple of 16 */
	pikrellcam.mjpeg_height &= ~0xf;
	resizer_create("stream_resizer", &stream_resizer,
							camera.component->output[CAMERA_PREVIEW_PORT],
							pikrellcam.mjpeg_width, pikrellcam.mjpeg_height);
	ports_tunnel_connect(&camera, CAMERA_PREVIEW_PORT, &stream_resizer);
	jpeg_encoder_create("mjpeg_encoder", &mjpeg_encoder,
					stream_resizer.component->output[0], pikrellcam.mjpeg_quality);
	ports_callback_connect(&stream_resizer, 0, &mjpeg_encoder,
					I420_video_callback);
	out_port_callback(&mjpeg_encoder, 0, mjpeg_callback);


	/* ====== Create the camera still port path ====== :
	|  camera_capture --(tunnel)--> jpeg_encoder --> still_jpeg__callback
	|                                               (writes stills and timelapse jpegs)
	*/
	jpeg_encoder_create("still_jpeg_encoder", &still_jpeg_encoder,
					NULL, pikrellcam.camera_adjust.still_quality);
	ports_tunnel_connect(&camera, CAMERA_CAPTURE_PORT, &still_jpeg_encoder);
	out_port_callback(&still_jpeg_encoder, 0, still_jpeg_callback);


	/* ====== Create the camera video port path ====== :
	|  camera_video--(tunnel)-->h264 encoder-->video_h264_encoder_callback
	|                                         (writes data into video circular buffer)
	|                                         (records video / checks motion vectors)
	|                                         (schedules mjpeg.jpg copy into previews)
	*/
	h264_encoder_create("video_h264_encoder", &video_h264_encoder, NULL);
	ports_tunnel_connect(&camera, CAMERA_VIDEO_PORT, &video_h264_encoder);
	out_port_callback(&video_h264_encoder, 0, video_h264_encoder_callback);

	time(&pikrellcam.t_start);

	/* Turn on the video stream. It free runs into the video circular buffer.
	*/
	if ((status = mmal_port_parameter_set_boolean(
				camera.component->output[CAMERA_VIDEO_PORT],
				MMAL_PARAMETER_CAPTURE, 1)) != MMAL_SUCCESS)
		log_printf("Video capture startup failed. Status %s\n",
					mmal_status[status]);

	/* With everything created and running, set the config'ed camera params.
	*/
	mmalcam_config_parameters_set_camera();

	display_init();
	video_circular_buffer.state = VCB_STATE_NONE;
	video_circular_buffer.pause = FALSE;
	}


void
camera_stop(void)
	{
	mmal_port_parameter_set_boolean(camera.component->output[CAMERA_VIDEO_PORT],
				MMAL_PARAMETER_CAPTURE, 0);

	camera_object_destroy(&video_h264_encoder);
	camera_object_destroy(&still_jpeg_encoder);
	camera_object_destroy(&stream_resizer);
	camera_object_destroy(&mjpeg_encoder);
	camera_object_destroy(&camera);
	}

void
camera_restart(void)
	{
	VideoCircularBuffer	*vcb = &video_circular_buffer;

	pthread_mutex_lock(&vcb->mutex);
	video_record_stop(vcb);
	vcb->state = VCB_STATE_RESTARTING;
	pikrellcam.camera_adjust = camera_adjust_temp;	/* May not be changed */
	pthread_mutex_unlock(&vcb->mutex);

	camera_stop();
	camera_start();
	}

boolean
still_capture(char *fname)
	{
	Event			*event;
	int				n;
	MMAL_STATUS_T	status;
	boolean			result = FALSE;

	/* timelapse_shapshot() also uses the still_jpeg_encoder, so wait if busy.
	*/
	for (n = 0; n < 5; ++n)
		{
		if (still_jpeg_encoder.file == NULL)
			break;
		usleep(50000);
		}
	if (still_jpeg_encoder.file != NULL)
		{
		/* inform() */
		log_printf("still capture failed because jpeg encoder is busy.\n");
		return FALSE;
		}

	if ((still_jpeg_encoder.file = fopen(fname, "w")) == NULL)
		log_printf("Could not create still file %s.  %m\n", fname);
	else
		{
		if ((status = mmal_port_parameter_set_boolean(
						camera.component->output[CAMERA_CAPTURE_PORT],
						MMAL_PARAMETER_CAPTURE, 1)) != MMAL_SUCCESS)
			{
			fclose(still_jpeg_encoder.file);
			still_jpeg_encoder.file = NULL;
			log_printf("Still capture startup failed. Status %s\n",
						mmal_status[status]);
			}
		else
			{
			result = TRUE;
			log_printf("Still: %s\n", fname);
			dup_string(&pikrellcam.still_last_save, fname);
			n = pikrellcam.notify_duration * EVENT_LOOP_FREQUENCY;

			if ((event = event_find("still saved")) != NULL)
				event->count = n;	/* rapid stills, extend the time */
			else
				event_count_down_add("still saved", n,
						event_notify_expire, &pikrellcam.still_notify);
			pikrellcam.still_capture_event = TRUE;
			pikrellcam.still_notify = TRUE;
			}
		}
	return result;
	}

void
timelapse_capture(void)
	{
	char			*path, seq_buf[12], series_buf[12];
	int				n, nd;
	MMAL_STATUS_T	status;

	if (time_lapse.on_hold)
		return;

	/* still_capture()  also uses the still_jpeg_encoder, so wait if busy.
	*/
	for (n = 0; n < 5; ++n)
		{
		if (still_jpeg_encoder.file == NULL)
			break;
		usleep(50000);
		}
	if (still_jpeg_encoder.file != NULL)
		{
		log_printf("timelapse capture failed because jpeg encoder is busy.\n");
		return;
		}

	snprintf(seq_buf, sizeof(seq_buf), "%05d", time_lapse.sequence);
	snprintf(series_buf, sizeof(series_buf), "%05d", time_lapse.series);
	path = media_pathname(pikrellcam.timelapse_dir,
						pikrellcam.timelapse_format,
						'N',  seq_buf,
						'n',  series_buf);
	time_lapse.sequence += 1;

	if ((still_jpeg_encoder.file = fopen(path, "w")) == NULL)
		log_printf("Could not create timelapse file %s.  %m\n", path);
	else
		{
		if ((status = mmal_port_parameter_set_boolean(
						camera.component->output[CAMERA_CAPTURE_PORT],
						MMAL_PARAMETER_CAPTURE, 1)) != MMAL_SUCCESS)
			{
			fclose(still_jpeg_encoder.file);
			still_jpeg_encoder.file = NULL;
			log_printf("Timelapse capture startup failed. Status %s\n",
						mmal_status[status]);
			}
		else
			{
			log_printf("Timelapse still: %s\n", path);
			dup_string(&pikrellcam.timelapse_last_save, path);

			/* timelapse_capture() is an event call (inside the event loop)
			|  and we here add an event to the list.
			|  This only modifies the last event list next pointer, and
			|  the event loop is not there yet.
			*/
			nd = pikrellcam.notify_duration;
			if (nd > time_lapse.period - 3)
				nd = time_lapse.period / 2;
			if (nd > 1)
				nd *= EVENT_LOOP_FREQUENCY;
			else
				nd = EVENT_LOOP_FREQUENCY / 2;

			event_count_down_add("timelapse saved", nd,
					event_notify_expire, &pikrellcam.timelapse_notify);
			pikrellcam.timelapse_notify = TRUE;
			}
		}
	free(path);
	}

void
timelapse_inform_convert(void)
	{
	struct stat st;
	char		fname[200];

	if (!time_lapse.convert_name || !*time_lapse.convert_name)
		return;
	snprintf(fname, sizeof(fname), "%s/%s",
			pikrellcam.timelapse_dir, time_lapse.convert_name);
	st.st_size = 0;
	stat(fname, &st);
	time_lapse.convert_size = st.st_size;
	}

  /* vcb should be locked before calling video_record_start()
  */
void
video_record_start(VideoCircularBuffer *vcb, int start_state)
	{
	char	*s, *tag, *path, seq_buf[12];
	int		*seq;

	if (vcb->state == VCB_STATE_MANUAL_RECORD)
		return;

	if (start_state == VCB_STATE_MOTION_RECORD_START)
		{
		tag = pikrellcam.video_motion_tag;
		seq = &pikrellcam.video_motion_sequence;
		}
	else
		{
		tag = pikrellcam.video_manual_tag;
		seq = &pikrellcam.video_manual_sequence;
		}

	snprintf(seq_buf, sizeof(seq_buf), "%d", *seq);
	path = media_pathname(pikrellcam.video_dir, pikrellcam.video_filename,
						'N',  seq_buf,
						'M', tag);
	*seq += 1;
	dup_string(&pikrellcam.video_pathname, path);
	free(path);
	path = pikrellcam.video_pathname;

	if ((s = strstr(path, ".mp4")) != NULL && *(s + 4) == '\0')
		{
		asprintf(&path, "%s.h264", pikrellcam.video_pathname);
		dup_string(&pikrellcam.video_h264, path);
		free(path);
		path = pikrellcam.video_h264;
		pikrellcam.video_mp4box = TRUE;
		}
	else
		pikrellcam.video_mp4box = FALSE;

	if ((vcb->file = fopen(path, "w")) == NULL)
		log_printf("Could not create video file %s.  %m\n", path);
	else
		{
		log_printf("Video record: %s ...\n", path);
		vcb->state = start_state;
		}
	}

  /* vcb should be locked before calling video_record_stop()
  */
void
video_record_stop(VideoCircularBuffer *vcb)
	{
	Event	*event = NULL;
	char	*cmd;

	if (!vcb->file)
		return;

	fclose(vcb->file);
	vcb->file = NULL;

	log_printf("Video %s record stopped. Header size: %d  h264 file size: %d\n",
			(vcb->state & VCB_STATE_MOTION_RECORD) ? "motion" : "manual",
			pikrellcam.video_header_size, pikrellcam.video_size);
	if (pikrellcam.verbose_motion && !pikrellcam.verbose)
		printf("***Motion record stop: %s\n", pikrellcam.video_pathname);

	if (pikrellcam.video_mp4box)
		{
		asprintf(&cmd, "(MP4Box %s -fps %d -add %s %s %s && rm %s)",
				pikrellcam.verbose ? "" : "-quiet",
				pikrellcam.camera_adjust.video_mp4box_fps,
				pikrellcam.video_h264, pikrellcam.video_pathname,
				pikrellcam.verbose ? "" : "2> /dev/null",
				pikrellcam.video_h264);
		if (   (vcb->state & VCB_STATE_MOTION_RECORD)
		    && *pikrellcam.on_motion_end_cmd
		   )
			event = exec_child_event("motion end command", cmd, NULL);
		else
			exec_no_wait(cmd, NULL);
		free(cmd);
		}
	dup_string(&pikrellcam.video_last_save, pikrellcam.video_pathname);

	pikrellcam.video_notify = TRUE;
	event_count_down_add("video saved notify",
				pikrellcam.notify_duration * EVENT_LOOP_FREQUENCY,
				event_notify_expire, &pikrellcam.video_notify);
	if (vcb->state & VCB_STATE_MOTION_RECORD)
		{
		if (!strcmp(pikrellcam.motion_preview_save_mode, "best"))
			{
			motion_preview_area_fixup();
			event_add("motion area thumb", pikrellcam.t_now, 0,
					event_motion_area_thumb, NULL);
			event_add("preview save command", pikrellcam.t_now, 0,
					event_preview_save_cmd,
					pikrellcam.on_motion_preview_save_cmd);
			}
		if (event)	/* a mp4 video save event needs a MP4Box child exit */
			{
			event->data = pikrellcam.on_motion_end_cmd;
			event->func = event_motion_end_cmd;
			}
		else if (*pikrellcam.on_motion_end_cmd)	/* a h264 video save */
			event_add("motion end command", pikrellcam.t_now, 0,
					event_motion_end_cmd, pikrellcam.on_motion_end_cmd);
		}
	event_add("preview dispose", pikrellcam.t_now, 0,
					event_preview_dispose, NULL);
	vcb->state = VCB_STATE_NONE;
	vcb->pause = FALSE;
	}

static boolean
get_arg_pass1(char *arg)
	{
	if (!strcmp(arg, "-quit"))
		quit_flag = TRUE;

	if (!strcmp(arg, "-V") || !strcmp(arg, "--version"))
		{
		printf("%s\n", pikrellcam.version);
		exit(0);
		}
	else if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
		{
		/* XXX */
		exit(0);
		}
#if 0
	else if (!strcmp(arg, "-d") || !strcmp(arg, "--detach"))
		{
		if (getppid() != 1)  /* if not already a daemon */
			{
			if (daemon(0, 0)) /* Detach from terminal, reparent to pid 1 */
				{
				printf("Detach failed\n");
				exit(1);
				}
			}
		}
#endif
	else if (!strcmp(arg, "-v"))
		pikrellcam.verbose = TRUE;
	else if (!strcmp(arg, "-vm"))
		pikrellcam.verbose_motion = TRUE;
	else
		return FALSE;
	return TRUE;
	}


typedef enum
	{
	record,
	record_pause,
	still,

	tl_start,
	tl_end,
	tl_hold,
	tl_inform_convert,
	tl_show_status,

	motion_cmd,
	motion_enable,
	display_cmd,        /* Placement above here can affect OSD.  If menu */
		                /* or adjustment is showing, above commands redirect */
	                    /* to cancel the menu or adjustment. */
	video_fps,
	video_mp4box_fps,
	inform,
	save_config,
	delete_log,
	upgrade,
	quit
	}
	CommandCode;
	
typedef struct
	{
	char		*name;
	CommandCode	code;
	int			n_args;
	}
	Command;

static Command commands[] =
	{
	{ "record",      record,       1 },
	{ "record_pause", record_pause, 0 },
	{ "pause",       record_pause, 0 },
	{ "still",       still,        0 },

	{ "tl_start",    tl_start,   1 },
	{ "tl_end",      tl_end,     0 },
	{ "tl_hold",    tl_hold,   1 },
	{ "tl_show_status",  tl_show_status,   1 },

	{ "motion",        motion_cmd,     1 },
	{ "motion_enable", motion_enable,  1 },

	/* Above commands are redirected to abort a menu or adjustment display
	*/
	{ "display",       display_cmd,     1 },

	/* Below commands are not redirected to abort a menu or adjustment */
	{ "tl_inform_convert",    tl_inform_convert,   1 },

	{ "video_fps", video_fps,  1 },
	{ "video_mp4box_fps", video_mp4box_fps,  1 },
	{ "inform", inform,    1 },
	{ "save_config", save_config,    0 },
	{ "delete_log", delete_log,    0 },
	{ "upgrade", upgrade,    0 },
	{ "quit",        quit,    0 },
	};

#define COMMAND_SIZE	(sizeof(commands) / sizeof(Command))

void
command_process(char *command_line)
	{
	VideoCircularBuffer	*vcb = &video_circular_buffer;
	Command	*cmd;
	char	command[64], args[128], buf[128], *path;
	int		i, n;

	if (!command_line || *command_line == '\0')
		return;

	n = sscanf(command_line, "%63s %[^\n]", command, args);
	if (n < 1 || command[0] == '#')
		return;
	for (cmd = NULL, i = 0; i < COMMAND_SIZE; cmd = NULL, ++i)
		{
		cmd = &commands[i];
		if (!strcmp(command, cmd->name))
			{
			if (cmd->n_args != n - 1)
				{
				log_printf("Wrong number of args for command: %s\n", command);
				return;
				}
			break;
			}
		}
	if (!cmd || (cmd->code != display_cmd && cmd->code != inform))
		log_printf("command_process: %s\n", command_line);
	if (!cmd)
		{
		if (   !config_set_option(command, args, FALSE)
		    && !mmalcam_config_parameter_set(command, args, TRUE)
	       )
			log_printf("Bad command: [%s] [%s]\n", command, args);
		else
			pikrellcam.config_modified = TRUE;
		return;
		}

	if (cmd->code < display_cmd && !display_is_default())
		{
		display_set_default();
		return;
		}

	switch (cmd->code)
		{
		case record:
			pthread_mutex_lock(&vcb->mutex);
			if (config_boolean_value(args) == TRUE)
				{
				if (vcb->pause)
					vcb->pause = FALSE;
				else
					{
					if (vcb->state == VCB_STATE_MOTION_RECORD)
						video_record_stop(vcb);
					video_record_start(vcb, VCB_STATE_MANUAL_RECORD_START);
					}
				}
			else
				video_record_stop(vcb);
			pthread_mutex_unlock(&vcb->mutex);
			break;

		case record_pause:
			/* Can pause manual record only.  Because of event_gap/capture
			|  times, I'm not even sure what it would mean to pause a
			|  motion record.
			*/
			pthread_mutex_lock(&vcb->mutex);
			if (vcb->state == VCB_STATE_MANUAL_RECORD)
				vcb->pause = vcb->pause ? FALSE : TRUE;
			else
				vcb->pause = FALSE;
			pthread_mutex_unlock(&vcb->mutex);
			break;

		case still:
			snprintf(buf, sizeof(buf), "%d", pikrellcam.still_sequence);
			path = media_pathname(pikrellcam.still_dir, pikrellcam.still_filename,
							'N',  buf,
							'\0', NULL);
			pikrellcam.still_sequence += 1;
			still_capture(path);
			free(path);
			break;

		case tl_start:
			if ((n = atoi(args)) < 1)
				n = 0;
			time_lapse.activated = TRUE;
			time_lapse.on_hold = FALSE;
			if (!time_lapse.event && n > 0)
				{
				time_lapse.sequence = 0;
				++time_lapse.series;
				time_lapse.event = event_add("timelapse",
							pikrellcam.t_now, n, timelapse_capture, NULL);
				}
			else if (n > 0)		/* Change the period */
				{
				time_lapse.event->time += (n - time_lapse.period);
				time_lapse.event->period = n;
				}
			if (n > 0)
				time_lapse.period = n;	/* n == 0 just sets on_hold FALSE */
			config_timelapse_save_status();	
			break;

		case tl_hold:
				config_set_boolean(&time_lapse.on_hold, args);
				config_timelapse_save_status();
			break;

		case tl_end:
			if (time_lapse.activated)
				{
				event_remove(time_lapse.event);
				time_lapse.event = NULL;
				time_lapse.activated = FALSE;
				time_lapse.on_hold = FALSE;
				config_timelapse_save_status();
				exec_no_wait(pikrellcam.on_timelapse_end_cmd, NULL);
				}
			break;

		case tl_inform_convert:
			if (!strcmp(args, "done"))
				{
				event_remove(time_lapse.inform_event);
				dup_string(&time_lapse.convert_name, "");
				time_lapse.convert_size = 0;
				}
			else
				{
				dup_string(&time_lapse.convert_name, args);
				time_lapse.inform_event = event_add("tl_inform_convert",
						pikrellcam.t_now, 5, timelapse_inform_convert, NULL);
				}
			break;

		case tl_show_status:
			config_set_boolean(&time_lapse.show_status, args);
			break;

		case display_cmd:
			display_command(args);
			break;

		case motion_cmd:
			motion_command(args);
			break;

		case motion_enable:
			n = motion_frame.motion_enable;
			config_set_boolean(&motion_frame.motion_enable, args);

			if (n && !motion_frame.motion_enable)
				{
				pthread_mutex_lock(&vcb->mutex);
				if (vcb->state == VCB_STATE_MOTION_RECORD)
					video_record_stop(vcb);
				pthread_mutex_unlock(&vcb->mutex);
				}
			break;

		case video_fps:
			if ((n = atoi(args)) < 1)
				n = 1;
			if (n > 49)
				n = 24;
			camera_adjust_temp.video_fps = n;
			pikrellcam.camera_adjust.video_fps = n;
			camera_restart();
			pikrellcam.config_modified = TRUE;
			break;

		case video_mp4box_fps:
			n = atoi(args);
			camera_adjust_temp.video_mp4box_fps = n;
			pikrellcam.camera_adjust.video_mp4box_fps = n;
			pikrellcam.config_modified = TRUE;
			break;

		case inform:
			display_inform(args);
			break;

		case save_config:
			config_save(pikrellcam.config_file);
			break;

		case delete_log:
			unlink(pikrellcam.log_file);
			break;

		case upgrade:
			snprintf(buf, sizeof(buf), "%s/scripts-dist/_upgrade $I $P $G $Z",
						pikrellcam.install_dir);
			exec_no_wait(buf, NULL);
			break;

		case quit:
			config_timelapse_save_status();
			if (pikrellcam.config_modified)
				config_save(pikrellcam.config_file);
			display_quit();
			exit(0);
			break;

		default:
			log_printf_no_timestamp("command in table with no action!\n");
			break;
		}
	}


static void
check_modes(char *fname, int mode)
	{
	struct stat		st;
	struct group	*grp;
	struct passwd	*pwd;
	char			ch_cmd[200];

	if (stat(fname, &st) == 0)
		{
		grp = getgrgid(st.st_gid);
		pwd = getpwuid(st.st_uid);

		if (   strcmp(pwd->pw_name, pikrellcam.effective_user)
		    || strcmp(grp->gr_name, "www-data")
		   )
			{
			snprintf(ch_cmd, sizeof(ch_cmd), "sudo chown %s.www-data %s",
					pikrellcam.effective_user, fname);
			exec_wait(ch_cmd, NULL);
			}
		if ((st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)) != mode)
			{
			snprintf(ch_cmd, sizeof(ch_cmd), "sudo chmod %o %s", mode, fname);
			exec_wait(ch_cmd, NULL);
			}
		}
	}


boolean
make_fifo(char *fifo_path)
    {
    boolean		fifo_exists;

	if ((fifo_exists = isfifo(fifo_path)) == FALSE)
		{
        if (mkfifo(fifo_path, 0664) < 0)
		    log_printf_no_timestamp("Make fifo %s failed: %m\n", fifo_path);
		else
			fifo_exists = TRUE;
		}
	if (fifo_exists)
		check_modes(fifo_path, 0664);
	return fifo_exists;
	}

static boolean
make_dir(char *dir)
	{
	boolean 		dir_exists;

	if ((dir_exists = isdir(dir)) == FALSE)
		{
		exec_wait("sudo mkdir -p $F", dir);
		if ((dir_exists = isdir(dir)) == FALSE)
			log_printf_no_timestamp("Make directory failed: %s\n", dir);
		else
			dir_exists = TRUE;
		}
	if (dir_exists)
		check_modes(dir, 0775);
	return dir_exists;
	}

static void
signal_quit(int sig)
	{
	config_timelapse_save_status();
	if (pikrellcam.config_modified)
		config_save(pikrellcam.config_file);
	display_quit();
	log_printf("quit signal received - exiting!\n");
	exit(0);
	}

int
main(int argc, char *argv[])
	{
	int		fifo;
	int	 	i, n;
	char	*opt, *arg, *equal_arg, *homedir, *user;
	char	*line, *eol, buf[4096];

	pgm_name = argv[0];
	bcm_host_init();
	setlocale (LC_ALL, "");

	time(&pikrellcam.t_now);

	config_set_defaults();

	for (i = 1; i < argc; i++)
		get_arg_pass1(argv[i]);

	if (!config_load(pikrellcam.config_file))
		config_save(pikrellcam.config_file);
	if (!motion_regions_config_load(pikrellcam.motion_regions_config_file))
		motion_regions_config_save(pikrellcam.motion_regions_config_file);

	if (*pikrellcam.log_file != '/')
		{
		snprintf(buf, sizeof(buf), "%s/%s", pikrellcam.install_dir, pikrellcam.log_file);
		dup_string(&pikrellcam.log_file, buf);
		}
	if (!quit_flag)
		{
		log_printf_no_timestamp("\n========================================================\n");
		strftime(buf, sizeof(buf), "%F %T", localtime(&pikrellcam.t_now));
		log_printf_no_timestamp("%s ===== PiKrellCam %s started =====\n", buf, pikrellcam.version);
		log_printf_no_timestamp("========================================================\n");
		}

	if (!at_commands_config_load(pikrellcam.at_commands_config_file))
		at_commands_config_save(pikrellcam.at_commands_config_file);

	for (i = 1; i < argc; i++)
		{
		if (get_arg_pass1(argv[i]))
			continue;
		opt = argv[i];

		/* Just for initial install-pikrellcam.sh run to create config files.
		*/
		if (!strcmp(opt, "-quit"))
			exit(0);

		/* Accept: --opt arg   -opt arg    opt=arg    --opt=arg    -opt=arg
		*/
		for (i = 0; i < 2; ++i)
			if (*opt == '-')
				++opt;
		if ((equal_arg = strchr(opt, '=')) != NULL)
			{
			*equal_arg++ = '\0';
			arg = equal_arg;
			++i;
			}
		else
			arg = argv[i + 1];

		/* For camera parameters, do not set the camera, only replace
		|  values in the parameter table.
		*/
		if (   !config_set_option(opt, arg, TRUE)
		    && !mmalcam_config_parameter_set(opt, arg, FALSE)
		   )
			{
			log_printf_no_timestamp("Bad arg: %s\n", opt);
			exit(1);
			}
		}

	homedir = getpwuid(geteuid())->pw_dir;
	user = strrchr(homedir, '/');
	pikrellcam.effective_user = strdup(user ? user + 1 : "pi");

	if (*pikrellcam.media_dir != '/')
		{
		snprintf(buf, sizeof(buf), "%s/%s", pikrellcam.install_dir, pikrellcam.media_dir);
		dup_string(&pikrellcam.media_dir, buf);
		}

	snprintf(buf, sizeof(buf), "%s/%s", pikrellcam.install_dir, "www");
	check_modes(buf, 0775);

	asprintf(&pikrellcam.command_fifo, "%s/www/FIFO", pikrellcam.install_dir);
	asprintf(&pikrellcam.script_dir, "%s/scripts", pikrellcam.install_dir);
	asprintf(&pikrellcam.mjpeg_filename, "%s/mjpeg.jpg", pikrellcam.mjpeg_dir);

	log_printf_no_timestamp("using FIFO: %s\n", pikrellcam.command_fifo);
	log_printf_no_timestamp("using mjpeg: %s\n", pikrellcam.mjpeg_filename);


	/* Subdirs must match www/config.php and the init script is supposed
	|  to take care of that.
	*/
	asprintf(&pikrellcam.video_dir, "%s/%s", pikrellcam.media_dir, PIKRELLCAM_VIDEO_SUBDIR);
	asprintf(&pikrellcam.thumb_dir, "%s/%s", pikrellcam.media_dir, PIKRELLCAM_THUMBS_SUBDIR);
	asprintf(&pikrellcam.still_dir, "%s/%s", pikrellcam.media_dir, PIKRELLCAM_STILL_SUBDIR);
	asprintf(&pikrellcam.timelapse_dir, "%s/%s", pikrellcam.media_dir, PIKRELLCAM_TIMELAPSE_SUBDIR);

	if (!make_dir(pikrellcam.media_dir))
		exit(1);

	snprintf(buf, sizeof(buf), "%s/scripts-dist/_init $I $m $M $P $G",
								pikrellcam.install_dir);
	exec_wait(buf, NULL);

	/* User may have enabled a mount disk on media_dir
	*/
	exec_wait(pikrellcam.on_startup_cmd, NULL);
	check_modes(pikrellcam.media_dir, 0775);
	check_modes(pikrellcam.log_file, 0664);

	if (   !make_dir(pikrellcam.mjpeg_dir)
	    || !make_dir(pikrellcam.video_dir)
	    || !make_dir(pikrellcam.thumb_dir)
	    || !make_dir(pikrellcam.still_dir)
	    || !make_dir(pikrellcam.timelapse_dir)
	    || !make_fifo(pikrellcam.command_fifo)
	   )
		exit(1);

	if ((fifo = open(pikrellcam.command_fifo, O_RDONLY | O_NONBLOCK)) < 0)
		{
		log_printf("Failed to open FIFO: %s.  %m\n", pikrellcam.command_fifo);
		exit(1);
		}

	fcntl(fifo, F_SETFL, 0);
	read(fifo, buf, sizeof(buf));
	
	camera_start();
	config_timelapse_load_status();

	signal(SIGINT, signal_quit);
	signal(SIGTERM, signal_quit);
	signal(SIGCHLD, event_child_signal);

	while (1)
		{
		usleep(1000000 / EVENT_LOOP_FREQUENCY);
		event_process();

		/* Process lines in the FIFO.  Single lines via an echo "xxx" > FIFO
		|  or from a web page may not have a terminating \n.
		|  Local scripts may dump multiple \n terminated lines into the FIFO.
		*/
		if ((n = read(fifo, buf, sizeof(buf) - 2)) > 0)
			{
			if (buf[n - 1] != '\n')
				buf[n++] = '\n';	/* ensures all lines in buf end in \n */
			buf[n] = '\0';
			line = buf;
			eol = strchr(line, '\n');

			while (eol > line)
				{
				*eol++ = '\0';
				command_process(line);
				while (*eol == '\n')
					++eol;
				line = eol;
				eol = strchr(line, '\n');
				}
			}
		}
	return 0;
	}
