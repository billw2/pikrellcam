/* PiKrellCam
|
|  Copyright (C) 2015-2016 Bill Wilson    billw@gkrellm.net
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
#include <sys/wait.h>
#include <sys/inotify.h>

#include "sunriset.h"

#define IBUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))


static pthread_mutex_t	event_mutex;
static SList			*event_list,
						*at_command_list;

static boolean     exec_with_session = TRUE;

typedef struct
	{
	boolean initialized;
	int		sun_valid,
			civil_valid,
			nautical_valid;

	double	longitude,
			latitude;

	double	d_sunrise,			/* time of day in hours  */
			d_sunset,
			d_dawn,				/* civil twilight morning */
			d_dusk,				/* civil twilight evening */
			d_nautical_dawn,
			d_nautical_dusk;

	int		sunrise,			/* time of day in minutes */
			sunset,
			dawn,
			dusk,
			nautical_dawn,
			nautical_dusk;
	}
	Sun;

static Sun	sun;


void
set_exec_with_session(boolean set)
	{
	exec_with_session = set;
	}

  /* exec a command with the given arg.  Any strftime() % replacements should
  |  have been done before calling this so there will remain only pikrellcam
  |  specific $X conversions.  Change all '$X' to '%s' and printf in what we
  |  want according to X.
  */
static int
exec_command(char *command, char *arg, boolean wait, pid_t *pid)
	{
	struct tm		*tm_now;
	PresetPosition	*pos;
	CompositeVector	*frame_vec = &motion_frame.final_preview_vector;
	char			specifier, *fmt, *fmt_arg, *copy, *cmd_line, *name, buf[BUFSIZ];
	int				t, i, status = 0;

	if (!command || !*command)
		return -1;

	copy = strdup(command);
	cmd_line = copy;
	while (   (fmt = strchr(copy, '$')) != NULL
	       && *(fmt + 1)
	      )
		{
		specifier = *(fmt + 1);
		*fmt++ = '%';
		*fmt = 's';
		switch (specifier)
			{
			case 'F':
				fmt_arg = arg ? arg : "";
				break;
			case 'H':
				fmt_arg = pikrellcam.hostname;
				break;
			case 'E':
				fmt_arg = pikrellcam.effective_user;
				break;
			case 'I':
				fmt_arg = pikrellcam.install_dir;
				break;
			case 's':
				fmt_arg = pikrellcam.still_last;
				break;
			case 'S':
				fmt_arg = pikrellcam.still_dir;
				break;
			case 'v':
				fmt_arg = pikrellcam.video_last;
				break;
			case 'V':
				fmt_arg = pikrellcam.video_dir;
				break;
			case 't':
				fmt_arg = pikrellcam.thumb_dir;
				break;

			case 'T':
				snprintf(buf, sizeof(buf), "%d", time_lapse.period);
				name = media_pathname(pikrellcam.video_dir,
						pikrellcam.video_timelapse_name_format, 0,
						'n',  buf, 'H', pikrellcam.hostname);
				snprintf(buf, sizeof(buf), "%s", name);
				free(name);
				fmt_arg = buf;
				dup_string(&pikrellcam.timelapse_video_pending, fmt_arg);
				break;
			case 'l':
				snprintf(buf, sizeof(buf), "%05d", time_lapse.series);
				name = strdup(pikrellcam.timelapse_format);
				name = substitute_var(name, 'n', buf);
				name = substitute_var(name, 'N', "%05d");
				snprintf(buf, sizeof(buf), "%s", name);
				free(name);
				fmt_arg = buf;
				break;
			case 'L':
				fmt_arg = pikrellcam.timelapse_dir;
				break;
			case 'a':
				fmt_arg = pikrellcam.archive_dir;
				break;
			case 'm':
				fmt_arg = pikrellcam.media_dir;
				break;
			case 'M':
				fmt_arg = pikrellcam.mjpeg_filename;
				break;
			case 'P':
				fmt_arg = pikrellcam.command_fifo;
				break;
			case 'p':
				pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list,
								pikrellcam.preset_position_index);
				snprintf(buf, sizeof(buf), "%d %d",
						pikrellcam.preset_position_index + 1,
						pos ? pos->settings_index + 1 : 1);
				fmt_arg = buf;
				break;
			case 'c':
				fmt_arg = pikrellcam.scripts_dist_dir;
				break;
			case 'C':
				fmt_arg = pikrellcam.scripts_dir;
				break;
			case 'G':
				fmt_arg = pikrellcam.log_file;
				break;
			case 'N':
				snprintf(buf, sizeof(buf), "%05d", time_lapse.sequence);
				fmt_arg = buf;
				break;
			case 'n':
				snprintf(buf, sizeof(buf), "%05d", time_lapse.series);
				fmt_arg = buf;
				break;

			case 'A':	/* thumb filename that is motion area jpg.*/
				fmt_arg = pikrellcam.preview_thumb_filename;
				break;
			case 'i':	/* width of motion area */
				t = frame_vec->box_w;
				snprintf(buf, sizeof(buf), "%d", t);
				fmt_arg = buf;
				break;
			case 'J':	/* height of motion area */
				t = frame_vec->box_h;
				snprintf(buf, sizeof(buf), "%d", t);
				fmt_arg = buf;
				break;
			case 'K':	/* center X of motion area */
				t = frame_vec->x;
				snprintf(buf, sizeof(buf), "%d", t);
				fmt_arg = buf;
				break;
			case 'Y':	/* center Y of motion area */
				t = frame_vec->y;
				snprintf(buf, sizeof(buf), "%d", t);
				fmt_arg = buf;
				break;
			case 'D':	/* current_minute dawn sunrise sunset dusk */
				tm_now = localtime(&pikrellcam.t_now);
				snprintf(buf, sizeof(buf), "%d %d %d %d %d",
						tm_now->tm_hour * 60 + tm_now->tm_min,
						sun.dawn, sun.sunrise, sun.sunset, sun.dusk);
				fmt_arg = buf;
				break;
			case 'Z':
				fmt_arg = pikrellcam.version;
				break;

			default:
				fmt_arg = "?";
				break;
			}
		if (!fmt_arg || !fmt_arg)
			log_printf("  Bad fmt_arg %p for specifier %c\n", fmt_arg, specifier);
		asprintf(&cmd_line, copy, fmt_arg);
		free(copy);
		copy = cmd_line;
		}

	log_printf("execl:[%s]\n", cmd_line);

	if ((*pid = fork()) == 0)
		{			/* child - execute command in background */
		for (i = getdtablesize(); i > 2; --i)
			close(i);
		if (exec_with_session)
			setsid();		/* new session group - ie detach */
		execl("/bin/sh", "sh", "-c", cmd_line, " &", NULL);
		_exit (EXIT_FAILURE);
		}
	else if (*pid < 0)
		{
		perror("Fork failed.");
		status = -1;
		}
	else if (wait)		/* If parent needs to wait */
		{
		if (waitpid (*pid, &status, 0) != *pid)
			status = -1;
		}
	free(cmd_line);
	return status;
	}

int
exec_wait(char *command, char *arg)
	{
	pid_t	pid;

	return exec_command(command, arg, TRUE, &pid);
	}

void
exec_no_wait(char *command, char *arg)
	{
	pid_t	pid;

	if (*command == '@')
		command_process(command + 1);
	else
		exec_command(command, arg, FALSE, &pid);
	}

#define	REBOOT	1

static void
event_shutdown_expire(void *arg)
	{
	boolean	shutdown_how = (int) arg;

	if (shutdown_how == REBOOT)
		display_inform("\"System Reboot request not confirmed.\" 4 3 1");
	else
		display_inform("\"System Halt request not confirmed.\" 4 3 1");
	display_inform("timeout 2");
	}

void
event_shutdown_request(boolean shutdown_how) /* 0 - halt  1 - reboot */
	{
	Event	*event;

	display_inform_clear();
	pthread_mutex_lock(&event_mutex);
	if ((event = event_find("shutdown")) != NULL)
		{
		event_list = slist_remove(event_list, event);
		if (event->data == (void *) shutdown_how)
			{
			if (shutdown_how == REBOOT)
				exec_no_wait("sudo shutdown -r now", NULL);
			else
				exec_no_wait("sudo shutdown -h now", NULL);
			}
		else
			{
			free(event);
			event = NULL;
			}
		}
	pthread_mutex_unlock(&event_mutex);
	if (!event)
		{
		event_count_down_add("shutdown", 11 * EVENT_LOOP_FREQUENCY,
				event_shutdown_expire, (void *) shutdown_how);
		if (shutdown_how == REBOOT)
			{
			display_inform("\"System Reboot requested!\" 3 3 1");
			display_inform("\"Click Reboot again within 10 seconds\" 4 3 1");
			display_inform("\"to confirm reboot.\" 5 3 1");
			}
		else
			{
			display_inform("\"System Halt requested!\" 3 3 1");
			display_inform("\"Click Halt again within 10 seconds\" 4 3 1");
			display_inform("\"to confirm halt.\" 5 3 1");
			}
		display_inform("timeout 10");
		}
	}

  /* Create a unactivated event and store the child pid of an exec() in the
  |  event.  Event will be activated only after the caller fills in the
  |  Event func() pointer and the child exit is caught where the Event time
  |  will be set to t_now.
  */
Event *
exec_child_event(char *event_name, char *command, char *arg)
	{
	Event	*event;

	/* event is unactivated if time and count are zero or func() is NULL
	*/
	event = event_add(event_name, 0, 0, NULL, NULL);
	exec_command(command, arg, FALSE, &event->child_pid);
	return event;
	}

  /* Time for displaying a notify on the stream jpeg has expired.
  */
void
event_notify_expire(boolean *notify)
	{
	*notify = FALSE;
	}

void
event_child_signal(int sig_num)
	{
	siginfo_t	siginfo;
	int			retval;
	Event		*event;
	SList		*list;

	siginfo.si_pid = 0;
	retval = waitid(P_ALL, 0, &siginfo, WEXITED | WNOHANG);
	if (pikrellcam.verbose)
		printf("child exit: ret=%d pid=%d %m\n", retval, siginfo.si_pid);

	pthread_mutex_lock(&event_mutex);
	for (list = event_list; list; list = list->next)
		{
		event = (Event *) list->data;
		if (event->child_pid == siginfo.si_pid)
			{
			event->time = pikrellcam.t_now;
			break;
			}
		}
	pthread_mutex_unlock(&event_mutex);
	}


  /* Handle various savings of a jpeg associated with a video recording.
  |  For motion records: save a copy of the mjpeg.jpg for later processing
  |  with on_motion_preview_save_cmd.  If mode is "best", this copy may be
  |  written multiple times.  This copy will be disposed of.
  */
void
event_preview_save(void)
	{
	FILE	*f_src, *f_dst;
	char	*s, *base, *path, *thumb, buf[BUFSIZ];
	int		n;

	path = strdup(pikrellcam.video_pathname);
	if (   (s = strstr(path, ".mp4")) != NULL
	    || (s = strstr(path, ".h264")) != NULL
	   )
		{
		*s = '\0';
		asprintf(&thumb, "%s.th.jpg", path);
		strcpy(s, ".jpg");

		/* Copy the current mjpeg.jpg into a motion preview file.
		*/
		if ((f_src = fopen(pikrellcam.mjpeg_filename, "r")) != NULL)
			{
			base = fname_base(path);	// motion_xxx.jpg
			if (pikrellcam.preview_filename)
				free(pikrellcam.preview_filename);
			asprintf(&pikrellcam.preview_filename, "%s/%s",
							pikrellcam.tmpfs_dir, base);

			/* thumb motion_xxx.th.jpg will be created in _thumb script, so
			|  mirror create the filename here so it can be passed to
			|  preview_save_cmd script.
			*/
			base = fname_base(thumb);	// motion_xxx.th.jpg
			if (pikrellcam.preview_thumb_filename)
				free(pikrellcam.preview_thumb_filename);
			asprintf(&pikrellcam.preview_thumb_filename, "%s/%s",
							pikrellcam.thumb_dir, base);

			log_printf("event preview save: copy %s -> %s\n",
								pikrellcam.mjpeg_filename,
								pikrellcam.preview_filename);
			if ((f_dst = fopen(pikrellcam.preview_filename, "w")) != NULL)
				{
				while ((n = fread(buf, 1, sizeof(buf), f_src)) > 0)
					{
					if (fwrite(buf, 1, n, f_dst) != n)
						break;
					}
				fclose(f_dst);
				}
			fclose(f_src);
			}
		free(thumb);
		}
	free(path);

	/* Let mmalcam.c mjpeg_callback() continue renaming the mjpeg file.
	*/
	pikrellcam.mjpeg_rename_holdoff = FALSE;
	}

  /* Generate a motion area thumb.
  */
void
event_motion_area_thumb(void)
	{
	char	*cmd = NULL;

	asprintf(&cmd, "%s/scripts-dist/_thumb $F $m $P $G $i $J $K $Y",
			pikrellcam.install_dir);
	exec_wait(cmd, pikrellcam.preview_filename);
	if (cmd)
		free(cmd);
	}

  /* Useful for emailing a motion event preview jpeg.
  */
void
event_preview_save_cmd(char *cmd)
	{
	if (!cmd || !*cmd)
		return;

//	log_printf("event_preview_save_cmd(); running %s %s\n", cmd,
//					pikrellcam.preview_filename);
	exec_wait(cmd, pikrellcam.preview_filename);
	}

  /* Do something with a motion video, eg scp to archive, etc.
  */
void
event_motion_end_cmd(char *cmd)
	{
	log_printf("event_motion_end_cmd(); running %s\n", cmd);
	exec_no_wait(cmd, NULL);
	}

  /* Motion events are finished with any preview jpeg handling, so delete it.
  */
void
event_preview_dispose(void)
	{
	if (! *pikrellcam.preview_filename)
		return;

	log_printf("event_preview_dispose(); removing %s\n",
					pikrellcam.preview_filename);
	unlink(pikrellcam.preview_filename);
	dup_string(&pikrellcam.preview_filename, "");
	}

void
event_still_capture_cmd(char *cmd)
	{
	if (!cmd || !*cmd)
		return;

	exec_wait(cmd, NULL);
	}

void
log_lines(void)
	{
	char	buf[128];

	if (pikrellcam.log_lines > 0)
		{
		snprintf(buf, sizeof(buf), "%s/scripts-dist/_log-lines %d $G",
					pikrellcam.install_dir, pikrellcam.log_lines);
		exec_wait(buf, NULL); 
		}
	}

void
state_file_write(void)
	{
	static char         *fname_part;
	FILE                *f;
	MotionFrame			*mf = &motion_frame;
	VideoCircularBuffer *vcb = &video_circular_buffer;
	PresetPosition		*pos;
	PresetSettings		*settings = NULL;
	char                *state;
	int					pan, tilt;
	double				ftmp, fps;

	if (!fname_part)
		asprintf(&fname_part, "%s.part", pikrellcam.state_filename);
	f = fopen(fname_part, "w");

	fprintf(f, "motion_enable %s\n", mf->motion_enable ? "on" : "off");
	fprintf(f, "show_preset %s\n",   mf->show_preset ? "on" : "off");
	fprintf(f, "show_vectors %s\n",  mf->show_vectors ? "on" : "off");

	servo_get_position(&pan, &tilt);
	pos = preset_find_at_position(pan, tilt);
	if (pos)
		{
		fprintf(f, "preset %d %d\n", pikrellcam.preset_position_index + 1,
					pos ? pos->settings_index + 1 : 1);
		settings = (PresetSettings *) slist_nth_data(pos->settings_list, pos->settings_index);
		if (settings)
			{
			fprintf(f, "magnitude_limit %d\n", settings->mag_limit);
			fprintf(f, "magnitude_count %d\n", settings->mag_limit_count);
			fprintf(f, "burst_count %d\n", settings->burst_count);
			fprintf(f, "burst_frames %d\n", settings->burst_frames);
			}
		}
	else
		{
		fprintf(f, "preset 0 0\n");
		fprintf(f, "magnitude_limit 0\n");
		fprintf(f, "magnitude_count 0\n");
		fprintf(f, "burst_count 0\n");
		fprintf(f, "burst_frames 0\n");
		}
	if (pikrellcam.have_servos)
		{
		fprintf(f, "pan %d\n", pan);
		fprintf(f, "tilt %d\n", tilt);
		}

	if (vcb->state & VCB_STATE_MOTION)
		state = "motion";
	if (vcb->state & VCB_STATE_MANUAL)
		state = "manual";
	else
		state = "stop";
	fprintf(f, "video_record_state %s\n", state);

	fprintf(f, "video_last %s\n",
			pikrellcam.video_last ? pikrellcam.video_last : "none");
	fprintf(f, "video_last_frame_count %d\n", pikrellcam.video_last_frame_count);

	/* The pts end-start diff is from frame start of 1st frame to frame start
	|  of last frame so is the time of frame_count - 1 frames.
	*/
	if (pikrellcam.video_last_frame_count > 1)
		{
		ftmp = (double) (pikrellcam.video_end_pts - pikrellcam.video_start_pts) / 1e6;
		ftmp /= pikrellcam.video_last_frame_count - 1;
		}
	else
		ftmp = 0;
	ftmp *= pikrellcam.video_last_frame_count;
	fprintf(f, "video_last_time %.2f\n", (float) ftmp);
	if (ftmp > 0)
		fps = (double) pikrellcam.video_last_frame_count / ftmp;
	else
		fps = 0;
	fprintf(f, "video_last_fps %.2f\n", (float) fps);

	fprintf(f, "still_last %s\n",
			pikrellcam.still_last ? pikrellcam.still_last : "none");

	fprintf(f, "show_timelapse %s\n", time_lapse.show_status ? "on" : "off");
	fprintf(f, "timelapse_period %d\n", time_lapse.period);
	fprintf(f, "timelapse_active %s\n",
			time_lapse.activated ? "on" : "off");
	fprintf(f, "timelapse_hold %s\n",
			time_lapse.on_hold ? "on" : "off");
	fprintf(f, "timelapse_jpeg_last %s\n",
			(pikrellcam.timelapse_jpeg_last && *pikrellcam.timelapse_jpeg_last)
					? pikrellcam.timelapse_jpeg_last : "none");
	fprintf(f, "timelapse_converting %s\n",
			(time_lapse.convert_name && *time_lapse.convert_name)
					? time_lapse.convert_name : "none");
	fprintf(f, "timelapse_video_last %s\n",
			pikrellcam.timelapse_video_last ? pikrellcam.timelapse_video_last : "none");

	fprintf(f, "current_minute %d\n",
			pikrellcam.tm_local.tm_hour * 60 + pikrellcam.tm_local.tm_min);
	fprintf(f, "dawn %d\n", sun.dawn);
	fprintf(f, "sunrise %d\n", sun.sunrise);
	fprintf(f, "sunset %d\n", sun.sunset);
	fprintf(f, "dusk %d\n", sun.dusk);

	fclose(f);
	rename(fname_part, pikrellcam.state_filename);
	}

  /* Add an event to trigger once after counting down the time to zero.
  |  since the event process loop runs many times/second, this allows higher
  |  resolution future one shot events.
  */
Event *
event_count_down_add(char *name, int count,
				void (*func)(), void *data)
	{
	Event *event;

	event = calloc(1, sizeof(Event));
	event->name = name;
	event->count = count;
	event->period = 0;			/* one time event */
	event->func = func;
	event->data = data;

	pthread_mutex_lock(&event_mutex);
	event_list = slist_append(event_list, event);
	pthread_mutex_unlock(&event_mutex);

	if (pikrellcam.verbose)
		printf("Event count down add [%s] count=%d\n",
						event->name, (int) event->count);
	return event;
	}

  /* Add an event which will trigger at a time and reset to a
  |  new time + period if period is > 0.
  */
Event *
event_add(char *name, time_t time, time_t period,
				void (*func)(), void *data)
	{
	Event *event;

	event = calloc(1, sizeof(Event));
	event->name = name;
	event->time = time;
	event->period = period;
	event->func = func;
	event->data = data;

	pthread_mutex_lock(&event_mutex);
	event_list = slist_append(event_list, event);
	pthread_mutex_unlock(&event_mutex);

	if (pikrellcam.verbose)
		printf("Event add [%s] period=%d\n",
						event->name, (int) event->period);
	return event;
	}

void
event_list_lock(void)
	{
	pthread_mutex_lock(&event_mutex);
	}

void
event_list_unlock(void)
	{
	pthread_mutex_unlock(&event_mutex);
	}

Event *
event_find(char *name)
	{
	Event	*event;
	SList	*list;

	for (list = event_list; list; list = list->next)
		{
		event = (Event *) list->data;
		if (!strcmp(event->name, name))
			return event;
		}
	return NULL;
	}

void
event_remove(Event *event)
	{
	if (!event)
		return;
	pthread_mutex_lock(&event_mutex);
	event_list = slist_remove(event_list, event);
	pthread_mutex_unlock(&event_mutex);
	free(event);
	}

boolean
event_remove_name(char *name)
	{
	Event	*event;
	boolean	found = FALSE;

	pthread_mutex_lock(&event_mutex);
	if ((event = event_find(name)) != NULL)
		{
		event_list = slist_remove(event_list, event);
		free(event);
		found = TRUE;
		}
	pthread_mutex_unlock(&event_mutex);
	return found;
	}

void
sun_times_init(void)
	{
	struct tm	*tm = &pikrellcam.tm_local;
	int		n, year, month, mday;

	if (   sscanf(pikrellcam.latitude, "%lf", &sun.latitude) == 1
	    && sscanf(pikrellcam.longitude, "%lf", &sun.longitude) == 1
	   )
		{
		n = strlen(pikrellcam.latitude);
		if (pikrellcam.latitude[n - 1] == 'S' || pikrellcam.latitude[n - 1] == 's')
			sun.latitude = -sun.latitude;
		n = strlen(pikrellcam.longitude);
		if (pikrellcam.longitude[n - 1] == 'W' || pikrellcam.longitude[n - 1] == 'w')
			sun.longitude = -sun.longitude;

		year  = tm->tm_year + 1900;
		month = tm->tm_mon  + 1;
		mday  = tm->tm_mday + 1;

		sun.sun_valid = sun_rise_set(year, month, mday,
						sun.longitude, sun.latitude,
						&sun.d_sunrise, &sun.d_sunset);
		sun.civil_valid = civil_twilight(year, month, mday,
						sun.longitude, sun.latitude,
						&sun.d_dawn, &sun.d_dusk);
		sun.nautical_valid = nautical_twilight(year, month, mday,
						sun.longitude, sun.latitude,
						&sun.d_nautical_dawn, &sun.d_nautical_dusk);

		sun.d_sunrise = TMOD(sun.d_sunrise + tm->tm_gmtoff / 3600);
		sun.sunrise = sun.d_sunrise * 60;

		sun.d_sunset  = TMOD(sun.d_sunset  + tm->tm_gmtoff / 3600);
		sun.sunset = sun.d_sunset * 60;

		sun.d_dawn = TMOD(sun.d_dawn + tm->tm_gmtoff / 3600);
		sun.dawn = sun.d_dawn * 60;

		sun.d_dusk = TMOD(sun.d_dusk + tm->tm_gmtoff / 3600);
		sun.dusk = sun.d_dusk * 60;

		sun.d_nautical_dawn = TMOD(sun.d_nautical_dawn + tm->tm_gmtoff / 3600);
		sun.nautical_dawn = sun.d_nautical_dawn * 60;

		sun.d_nautical_dusk = TMOD(sun.d_nautical_dusk + tm->tm_gmtoff / 3600);
		sun.nautical_dusk = sun.d_nautical_dusk * 60;

		log_printf_no_timestamp("sunrise/sunset times: %s  dawn/dusk times: %s\n",
			sun.sun_valid ? "invalid" : "valid",
			sun.civil_valid ? "invalid" : "valid");
		log_printf_no_timestamp("  dawn:    %d:%02d\n", sun.dawn / 60, sun.dawn % 60);
		log_printf_no_timestamp("  sunrise: %d:%02d\n", sun.sunrise / 60, sun.sunrise % 60);
		log_printf_no_timestamp("  sunset:  %d:%02d\n", sun.sunset / 60, sun.sunset % 60);
		log_printf_no_timestamp("  dusk:    %d:%02d\n", sun.dusk / 60, sun.dusk % 60);
		}
	}


static char *weekdays = "SunMonTueWedThuFriSatSun";

void
event_process(void)
	{
	Event		*event;
	AtCommand	*at;
	SList	*list,
			*prev_link = NULL,
			*next_link,
			*expired_link,
			tmp_link;
	int		minute_tick, five_minute_tick, ten_minute_tick,
			fifteen_minute_tick, thirty_minute_tick, hour_tick, day_tick;
	char    *cmd;
	struct tm		*tm_now;
	static struct tm tm_prev;
	static time_t	t_prev;

	pikrellcam.second_tick = (pikrellcam.t_now == t_prev) ? FALSE : TRUE;
	t_prev = pikrellcam.t_now;

	if (pikrellcam.second_tick)
		{
		tm_prev = pikrellcam.tm_local;
		tm_now = localtime(&pikrellcam.t_now);
		minute_tick = (tm_now->tm_min != tm_prev.tm_min)  ? TRUE : FALSE;
		pikrellcam.tm_local = *tm_now;
		}
	else
		minute_tick = FALSE;

	if (pikrellcam.state_modified || minute_tick)
		{
		pikrellcam.state_modified = FALSE;
		state_file_write();
		}

	tmp_link.next = NULL;
	for (list = event_list; list; prev_link = list, list = list->next)
		{
		/* Event loop processing is done with the list unlocked until we
		|  have an expired link that must be removed from the list so that
		|  called event functions cad add events to the list.
		|  If another thread adds an event, only the last list->next
		|  will be modified and this loop will catch it or it won't.
		*/
		event = (Event *) list->data;
		if (   (event->time == 0 && event->count == 0)
			|| event->func == NULL
		   )				/* not activated */
			continue;

		if (event->time > 0 && event->time > pikrellcam.t_now)
			continue;
		else if (event->count > 0 && --event->count > 0)
			continue;

		if (pikrellcam.verbose)
			printf("Event func -> [%s] period=%d\n",
						event->name, (int) event->period);
		if (event->func)
			(*event->func)(event->data);
		if (event->period > 0)
			event->time += event->period;
		else
			{
			pthread_mutex_lock(&event_mutex);
			expired_link = list;
			if (list == event_list)
				{
				event_list = list->next;
				tmp_link.next = list->next;
				list = &tmp_link;
				}
			else
				{
				next_link = list->next;
				list = prev_link;
				list->next = next_link;
				}
			free(expired_link->data);
			free(expired_link);
			pthread_mutex_unlock(&event_mutex);
			}
		}

	if (minute_tick)
		{
		char		*p, buf[IBUF_LEN];
		int			i, n, minute_now, minute_at, minute_offset = 0;
		static int	start = TRUE;
		static int	at_notify_fd, at_notify_wd;
		struct inotify_event *event;

		if (at_notify_fd == 0)
			{
			at_notify_fd = inotify_init();
			if (at_notify_fd > 0)
				{
				fcntl(at_notify_fd, F_SETFL,
						fcntl(at_notify_fd, F_GETFL) | O_NONBLOCK);
				at_notify_wd = inotify_add_watch(at_notify_fd,
						pikrellcam.config_dir, IN_CREATE | IN_MODIFY);
				}
			}
		else if (at_notify_wd > 0)
			{
			n = read(at_notify_fd, buf, IBUF_LEN);
			if (n > 0)
				{
				for (i = 0; i < n; i += sizeof(*event) + event->len)
					{
					event = (struct inotify_event *) &buf[i];
					if (   event->len > 0
					    && !strcmp(event->name,  PIKRELLCAM_AT_COMMANDS_CONFIG)
					   )
						at_commands_config_load(pikrellcam.at_commands_config_file);
					}
				}
			}

		tm_now = &pikrellcam.tm_local;
		minute_now = tm_now->tm_hour * 60 + tm_now->tm_min;

		five_minute_tick = ((tm_now->tm_min % 5) == 0) ? TRUE : FALSE;
		ten_minute_tick = ((tm_now->tm_min % 10) == 0) ? TRUE : FALSE;
		fifteen_minute_tick = ((tm_now->tm_min % 15) == 0) ? TRUE : FALSE;
		thirty_minute_tick = ((tm_now->tm_min % 10) == 0) ? TRUE : FALSE;
		hour_tick = (tm_now->tm_hour  != tm_prev.tm_hour)  ? TRUE : FALSE;
		day_tick =  (tm_now->tm_mday  != tm_prev.tm_mday)  ? TRUE : FALSE;

		if (day_tick || !sun.initialized)
			{
			if (sun.initialized)
				{
				char	tbuf[32];

				log_printf_no_timestamp("\n========================================================\n");
				strftime(tbuf, sizeof(tbuf), "%F", localtime(&pikrellcam.t_now));
				log_printf_no_timestamp("%s ================== New Day ==================\n", tbuf);
				log_printf_no_timestamp("========================================================\n");

				strftime(tbuf, sizeof(tbuf), "%F", localtime(&pikrellcam.t_now));
				}
			sun_times_init();
			sun.initialized = TRUE;
			log_lines();
			state_file_write();
			}

		for (list = at_command_list; list; list = list->next)
			{
			at = (AtCommand *) list->data;

			if ((p = strchr(at->at_time, '+')) != NULL)
				minute_offset = atoi(p + 1);
			else if ((p = strchr(at->at_time, '-')) != NULL)
				minute_offset = -atoi(p + 1);

			if (!strcmp(at->at_time, "start"))
				minute_at = start ? minute_now : -1;
			else if (!strcmp(at->at_time, "minute"))
				minute_at = minute_now;
			else if (!strcmp(at->at_time, "5minute"))
				minute_at = five_minute_tick ? minute_now : -1;
			else if (!strcmp(at->at_time, "10minute"))
				minute_at = ten_minute_tick ? minute_now : -1;
			else if (!strcmp(at->at_time, "15minute"))
				minute_at = fifteen_minute_tick ? minute_now : -1;
			else if (!strcmp(at->at_time, "30minute"))
				minute_at = thirty_minute_tick ? minute_now : -1;
			else if (!strcmp(at->at_time, "hour"))
				minute_at = hour_tick ? minute_now : -1;
			else if (!strncmp(at->at_time, "dawn", 4))
				minute_at = sun.dawn + minute_offset;
			else if (!strncmp(at->at_time, "dusk", 4))
				minute_at = sun.dusk + minute_offset;
			else if (!strncmp(at->at_time, "sunrise", 7))
				minute_at = sun.sunrise + minute_offset;
			else if (!strncmp(at->at_time, "sunset", 6))
				minute_at = sun.sunset + minute_offset;
			else if (!strncmp(at->at_time, "nautical_dawn", 13))
				minute_at = sun.nautical_dawn + minute_offset;
			else if (!strncmp(at->at_time, "nautical_dusk", 13))
				minute_at = sun.nautical_dusk + minute_offset;
			else
				{
				minute_at = (int) strtol(at->at_time, &p, 10) * 60;
				if (*p == ':')
					minute_at += strtol(p + 1, NULL, 10);
				else
					{
					minute_at = -1;	/* error in at_time string */
					log_printf("Error in at_command: [%s] bad at_time: [%s]\n",
							at->command, at->at_time);
					}
				}
			if (minute_now != minute_at)
				continue;

			/* Have a time match so check frequency.
			*/
			if (   !strcmp(at->frequency, "daily")
			    || (   !strcmp(at->frequency, "Sat-Sun")
			        && (tm_now->tm_wday == 0 || tm_now->tm_wday == 6)
			       )
			    || (   !strcmp(at->frequency, "Mon-Fri")
			        && tm_now->tm_wday > 0 && tm_now->tm_wday < 6
			       )
			    || (   (p = strstr(weekdays, at->frequency)) != NULL
			        && (p - weekdays) / 3 == tm_now->tm_wday
			       )
			   )
				{
				if (*(at->command) == '@')
					{
					cmd = strdup(at->command + 1);
					cmd = substitute_var(cmd, 'H', pikrellcam.hostname);
					command_process(cmd);
					free(cmd);
					}
				else
					exec_no_wait(at->command, NULL);
				}
			}
		start = FALSE;
		if (pikrellcam.config_modified)
			config_save(pikrellcam.config_file);
		if (pikrellcam.preset_modified)
			preset_config_save();
		}
	}


void
at_command_add(char *at_line)
	{
	AtCommand	*at;
	char		 frequency[32], at_time[64], cmd[200];
	int			 n;

	frequency[0] = '\0';
	n = sscanf(at_line, "%31s %63s \"%199[^\n\"]", frequency, at_time, cmd);
	if (frequency[0] == '#' || frequency[0] == '\n' || frequency[0] == '\0')
		return;
	if (n != 3)
		{
		log_printf_no_timestamp("Bad at command: %s\n", at_line);
		return;
		}
	at = (AtCommand *) calloc(1, sizeof(AtCommand));
	at->frequency = strdup(frequency);
	at->at_time = strdup(at_time);
	at->command = strdup(cmd);
	at_command_list = slist_append(at_command_list, at);

	log_printf_no_timestamp("at_command_add [%s] at: %s  command: [%s]\n",
				at->frequency, at->at_time, at->command);
	}

  /* This config save just initializes the at-command config file if it does
  |  not already exist.  User edits it afterwards.
  */
void
at_commands_config_save(char *config_file)
	{
	FILE		*f;
	SList		*list;
	AtCommand	*at;

	if (   !config_file
	    || (f = fopen(config_file, "w")) == NULL
	   )
		{
		log_printf("Failed to save at command config file %s. %m\n", config_file);
		return;
		}

	fprintf(f,
	"# List of commands to execute at a desired time and frequency.\n"
	"#     frequency: daily Mon-Fri Sat-Sun Mon Tue Wed Thu Fri Sat Sun\n"
	"#     time:      hh:mm start dawn dusk sunrise sunset minute hour\n"
	"#     command:   a command with possible substitution variables:\n"
	"#                $C - script commands directory full path\n"
	"#                $I - the PiKrellCam install directory\n"
	"#                $a - archive directory full path\n"
	"#                $m - media directory full path\n"
	"#                $M - mjpeg file full path\n"
	"#                $P - command FIFO full path\n"
	"#                $G - log file full path\n"
	"#                $H - hostname\n"
	"#                $E - effective user running PiKrellCam\n"
	"#                $V - video files directory full path\n"
	"#                $t - thumb files directory full path\n"
	"#                $v - last video saved full path filename\n"
	"#                $S - still files directory full path\n"
	"#                $s - last still saved full path filename\n"
	"#                $L - timelapse files directory full path\n"
	"#                $l - timelapse current series filename format: tl_sssss_%%05d.jpg\n"
	"#                     in timelapse sub directory.  If used in any script\n"
	"#                     arg list, $l must be the last argument.\n"
	"#                $T - timelapse video full path filename in video sub directory\n"
	"#                $N - timelapse sequence last number\n"
	"#                $D - current_minute dawn sunrise sunset dusk\n"
	"#                $Z - pikrellcam version\n"
	"# \n"
	"# Commands must be enclosed in quotes.\n"
	"# Frequency and time strings must not contain any spaces.\n"
	"# Commands may be system commands/scripts or may be internal pikrellcam\n"
	"# commands.  Begin the command with \'@\' for internal commands.\n"
	"# Examples:\n"
	"# \n"
	"#     daily   start   \"@motion load_regions front\"\n"
	"#     Mon-Fri 16:30   \"@motion_enable off\"\n"
	"#     Mon-Fri 19:00   \"@motion_enable on\"\n"
	"#     daily   sunset+5 \"@tl_hold on\"\n"
	"#     daily   sunrise-5 \"@tl_hold off\"\n"
	"# If you write a custom video_archive script:\n"
	"#     daily   hour    \"$C/video_archive $P $V\"\n"
	"#\n"
	"# If this file is modified, PiKrellCam will automatically reload it.\n"
	"#\n"
	);

	for (list = at_command_list; list; list = list->next)
		{
		at = (AtCommand *) list->data;

		fprintf(f, "%s %s %s\n",
				at->frequency, at->at_time, at->command);
		}
	fclose(f);
	}

boolean
at_commands_config_load(char *config_file)
	{
	FILE	*f;
	char	buf[200];

	if (   !config_file
	    || (f = fopen(config_file, "r")) == NULL
	    )
		return FALSE;

	slist_and_data_free(at_command_list);
	at_command_list = NULL;

	while (fgets(buf, sizeof(buf), f) != NULL)
		at_command_add(buf);
	fclose(f);
	return TRUE;
	}
