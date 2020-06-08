/* PiKrellCam
|
|  Copyright (C) 2015-2020 Bill Wilson    billw@gkrellm.net
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

static char				*filter_parent;

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


  /* Any strftime() % replacements should have been done before calling this
  |  so only pikrellcam specific $X conversions remain.  Change all '$X' to
  |  '%s' and printf in according to X.  Returns allocated storage.
  */
char *
expand_command(char *command, char *arg)
	{
	struct tm			*tm_now;
	VideoCircularBuffer	*vcb = &video_circular_buffer;
	PresetPosition		*pos;
	MotionFrame			*mf = &motion_frame;
	CompositeVector		*frame_vec = &motion_frame.final_preview_vector;
	int					t;
	char				specifier, *fmt, *fmt_arg, *copy, *cmd_line, *dir,
						*name, buf[BUFSIZ];

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
			case 'h':
				fmt_arg = pikrellcam.multicast_from_hostname;
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
			case 'X':
				snprintf(buf, sizeof(buf), "%d", pikrellcam.video_motion_sequence);
				fmt_arg = buf;
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
			case 'z':
				fmt_arg = pikrellcam.loop_dir;
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
			case 'o':
				snprintf(buf, sizeof(buf), "%s",
						motion_frame.motion_enable ? "on" : "off");
				fmt_arg = buf;
				break;
			case 'A':
				if (vcb->state & VCB_STATE_LOOP_RECORD)
					dir = pikrellcam.loop_thumb_dir;
				else if (vcb->state & VCB_STATE_MANUAL_RECORD)
					dir = pikrellcam.thumb_dir;
				else if (pikrellcam.motion_stills_enable)
					dir = pikrellcam.still_thumb_dir;
				else	/* VCB_STATE_MOTION_RECORD */
					dir = pikrellcam.thumb_dir;
				snprintf(buf, sizeof(buf), "%s/%s", dir, pikrellcam.thumb_name);
				fmt_arg = buf;
				break;
			case 'r':
				if (pikrellcam.motion_stills_enable)
					fmt_arg = "stills";
				else
					fmt_arg = "video";
				break;
			case 'e':
				if (pikrellcam.external_motion_record_event)
					snprintf(buf, sizeof(buf), "%s",
						motion_frame.fifo_detects
							? mf->fifo_trigger_code : "audio");
				else
					snprintf(buf, sizeof(buf), "%s", "motion");
				fmt_arg = buf;
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
				log_printf("Bad specifier %c in %s\n", specifier, command);
				fmt_arg = "?";
				break;
			}

		if (!fmt_arg || *fmt_arg == '\0')
			fmt_arg = "none";

		asprintf(&cmd_line, copy, fmt_arg);
		free(copy);
		copy = cmd_line;
		}
	return cmd_line;
	}

static int
exec_command(char *command, char *arg, boolean wait, pid_t *pid, boolean do_log)
	{
	pid_t	child;
	char	*cmd_line;
	int		i, status = -1;

	if (!command || !*command)
		return -1;
	cmd_line = expand_command(command, arg);

	if ((child = fork()) == 0)
		{
		for (i = getdtablesize(); i > 2; --i)
			close(i);
		if (!wait && fork() > 0)
			exit(0);
		execl("/bin/sh", "sh", "-c", cmd_line, NULL);
		_exit (EXIT_FAILURE);
		}
	else if (child > 0)
		{
		if (do_log)
			log_printf("  execl[wait:%d]: %s\n", wait, cmd_line);
		if (pid)
			*pid = child;
		waitpid(child, &status, 0);
		}
	else
		log_printf("fork() failed: %m\n");

	free(cmd_line);
	return status;
	}

int
exec_wait(char *command, char *arg)
	{
	pid_t	pid;

	return exec_command(command, arg, TRUE, &pid, TRUE);
	}

void
exec_no_wait(char *command, char *arg, boolean do_log)
	{
	pid_t	pid;

	if (!command)
		return;
	if (*command == '@')
		command_process(command + 1);
	else if (*command == '!')
		exec_command(command + 1, arg, FALSE, &pid, FALSE);
	else
		exec_command(command, arg, FALSE, &pid, do_log);
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
			pikrellcam_cleanup();
			if (shutdown_how == REBOOT)
				exec_no_wait("sudo shutdown -r now", NULL, TRUE);
			else
				exec_no_wait("sudo shutdown -h now", NULL, TRUE);
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

  /* Time for displaying a notify on the stream jpeg has expired.
  */
void
event_notify_expire(boolean *notify)
	{
	*notify = FALSE;
	}

void
event_motion_enable_cmd(char *cmd)
	{
	log_printf("on_motion_enable: - running: %s\n", cmd);
	exec_no_wait(cmd, NULL, pikrellcam.verbose_log);
	}

void
event_motion_begin_cmd(char *cmd)
	{
	log_printf("on_motion_begin: - running: %s\n", cmd);
	exec_no_wait(cmd, NULL, pikrellcam.verbose_log);
	}

void
event_still_capture_cmd(char *cmd)
	{
	if (!cmd || !*cmd)
		return;

	exec_wait(cmd, NULL);
	}

void
event_motion_still_capture(void *p)
	{
	if (!pikrellcam.motion_still_pathname)
		return;
	still_capture(pikrellcam.motion_still_pathname, TRUE);
	free(pikrellcam.motion_still_pathname);
	pikrellcam.motion_still_pathname = NULL;
	}

static boolean
diskfree_is_low(char *dir)
	{
	struct statvfs	st;
	int				free_pct;
	boolean			is_low;

	statvfs(dir, &st);
	free_pct = (int) (100LL * (uint64_t) st.f_bavail / (uint64_t) st.f_blocks);
	is_low = (free_pct <= pikrellcam.diskfree_percent);
	if (is_low && pikrellcam.verbose_log)
		log_printf("%s: free space %d <= Diskfree_percent limit %d\n",
					dir, free_pct, pikrellcam.diskfree_percent);
	return is_low;
	}

static int
jpg_filter(const struct dirent *entry)
	{
	char	*jpg;

	jpg = strstr(entry->d_name, ".jpg");
	return (jpg ? 1 : 0);
	}

static int
mp4_filter(const struct dirent *entry)
	{
	char	*mp4;

	mp4 = strstr(entry->d_name, ".mp4");
	if (!mp4)
		mp4 = strstr(entry->d_name, ".mp3");  // to clean out stray mp3 files
	return (mp4 ? 1 : 0);
	}

static int
dir_filter(const struct dirent *entry)
	{
	struct stat st;
	char		filter_dir[512];

	if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
		return 0;
	snprintf(filter_dir, sizeof(filter_dir), "%s/%s",
			filter_parent, entry->d_name);
	stat(filter_dir, &st);
	return (st.st_mode & S_IFDIR);
	}

static int
date_compare(const struct dirent **e1, const struct dirent **e2)
	{
	char	*a, *b, *s;

	s = strchr((*e1)->d_name, (int) '_');	/* skip past motion_ or manual_ */
	a = (s ? s + 1 : (char *)(*e1)->d_name);

	s = strchr((*e2)->d_name, (int) '_');
	b = (s ? s + 1 : (char *) (*e2)->d_name);

	return strcmp(a, b);
	}

static boolean
video_diskfree_percent_delete(char *media_dir, boolean *empty)
	{
	struct dirent	**mp4_list = NULL;
	char			*s, video_dir[256], thumb_dir[256], fname[512];
	int				i, n, low_space;
	boolean			done = FALSE;

	if (empty)
		*empty = FALSE;
	if ((low_space = diskfree_is_low(media_dir)) == FALSE)
		return TRUE;

	snprintf(video_dir, sizeof(video_dir), "%s/%s", media_dir,
				PIKRELLCAM_VIDEO_SUBDIR);
	snprintf(thumb_dir, sizeof(thumb_dir), "%s/%s", media_dir,
				PIKRELLCAM_THUMBS_SUBDIR);
	if ((n = scandir(video_dir, &mp4_list, mp4_filter, date_compare)) < 0)
		{
		log_printf("video diskfree percent delete: scandir(%s) failed.\n", video_dir);
		return FALSE;
		}
	if (n == 0 && empty)
		*empty = TRUE;

	for (i = 0; i < n; ++i)
		{
		if (low_space)
			{
			snprintf(fname, sizeof(fname), "%s/%s",
						video_dir, mp4_list[i]->d_name);
			unlink(fname);
			log_printf("Low disk space, deleted: %s\n", fname);

			if ((s = strstr(fname, ".mp4")) != NULL)
				strcpy(s, ".csv");
			unlink(fname);

			snprintf(fname, sizeof(fname), "%s/%s",
						thumb_dir, mp4_list[i]->d_name);
			if ((s = strstr(fname, ".mp4")) != NULL)
				strcpy(s, ".th.jpg");
			unlink(fname);
			if (empty && i == n - 1)
				*empty = TRUE;
			low_space = diskfree_is_low(media_dir);
			}
		if (!low_space)
			done = TRUE;
		free(mp4_list[i]);
		}
	if (mp4_list)
		free(mp4_list);
	return done;
	}

void
event_video_diskfree_percent(char *type)
	{
	video_diskfree_percent_delete(pikrellcam.media_dir, NULL);
	}

static boolean
still_diskfree_percent_delete(char *media_dir, char *sub_dir, boolean *empty)
	{
	struct dirent	**jpg_list = NULL;
	char			*s, still_dir[256], fname[600], thumb_base[300];
	int				i, n, low_space;
	boolean			done = FALSE;

	if (empty)
		*empty = FALSE;
	if ((low_space = diskfree_is_low(media_dir)) == FALSE)
		return TRUE;

	snprintf(still_dir, sizeof(still_dir), "%s/%s", media_dir, sub_dir);
	if ((n = scandir(still_dir, &jpg_list, jpg_filter, date_compare)) < 0)
		{
		log_printf("still diskfree percent delete: scandir(%s) failed.\n", still_dir);
		return FALSE;
		}
	if (n == 0 && empty)
		*empty = TRUE;

	for (i = 0; i < n; ++i)
		{
		if (low_space)
			{
			snprintf(fname, sizeof(fname), "%s/%s",
						still_dir, jpg_list[i]->d_name);
			unlink(fname);

			snprintf(thumb_base, sizeof(thumb_base), "%s", jpg_list[i]->d_name);
			if ((s = strstr(thumb_base, ".jpg")) != NULL)
				{
				*s = '\0';
				snprintf(fname, sizeof(fname), "%s/.thumbs/%s.th.jpg",
							still_dir, thumb_base);
				unlink(fname);
				}
			log_printf("Low disk space, deleted: %s\n", fname);

			if (empty && i == n - 1)
				*empty = TRUE;
			low_space = diskfree_is_low(still_dir);
			}
		if (!low_space)
			done = TRUE;
		free(jpg_list[i]);
		}
	if (jpg_list)
		free(jpg_list);
	return done;
	}

void
event_still_diskfree_percent(char *subdir)
	{
	still_diskfree_percent_delete(pikrellcam.media_dir, subdir, NULL);
	}

void
event_archive_diskfree_percent(char *type)
	{
	struct dirent	**year_list = NULL,
					**month_list = NULL,
					**day_list = NULL;;
	int				y, n_yr, m, n_mon, d, n_day;
	char			year_dir[300], mon_dir[600], day_dir[800], del_dir[1000];
	boolean			done = FALSE, dir_empty;

	if (!diskfree_is_low(pikrellcam.archive_dir))
		return;
	filter_parent = pikrellcam.archive_dir;
	n_yr = scandir(pikrellcam.archive_dir, &year_list, dir_filter, alphasort);
	for (y = 0; y < n_yr; ++y)
		{
		snprintf(year_dir, sizeof(year_dir), "%s/%s",
					pikrellcam.archive_dir, year_list[y]->d_name);
		filter_parent = year_dir;
		if ((n_mon = scandir(year_dir, &month_list, dir_filter, alphasort)) <= 0)
			continue;
		for (m = 0; m < n_mon; ++m)
			{
			snprintf(mon_dir, sizeof(mon_dir), "%s/%s/%s",
						pikrellcam.archive_dir, year_list[y]->d_name,
						month_list[m]->d_name);
			filter_parent = mon_dir;
			if ((n_day = scandir(mon_dir, &day_list,
							dir_filter, alphasort)) <= 0)
				continue;
			for (d = 0; d < n_day; ++d)
				{
				if (!done)
					{
					snprintf(day_dir, sizeof(day_dir), "%s/%s/%s/%s",
								pikrellcam.archive_dir, year_list[y]->d_name,
								month_list[m]->d_name, day_list[d]->d_name);
					done = video_diskfree_percent_delete(day_dir, &dir_empty);
					if (dir_empty)
						{
						snprintf(del_dir, sizeof(del_dir), "%s/videos", day_dir);
						rmdir(del_dir);
						snprintf(del_dir, sizeof(del_dir), "%s/thumbs", day_dir);
						rmdir(del_dir);
						rmdir(day_dir);
						if (done && d == n_day - 1)
							{
							/* rmdir() here fails if there are stills */
							rmdir(mon_dir);
							if (done && m == n_mon - 1)
								rmdir(year_dir);
							}
						}
					if (!done)
						{
						done = still_diskfree_percent_delete(day_dir,
									PIKRELLCAM_STILL_SUBDIR, &dir_empty);
						if (dir_empty)
							{
							snprintf(del_dir, sizeof(del_dir),
										"%s/stills/.thumbs", day_dir);
							rmdir(del_dir);
							snprintf(del_dir, sizeof(del_dir),
										"%s/stills", day_dir);
							rmdir(del_dir);
							if (done && d == n_day - 1)
								{
								rmdir(mon_dir);
								if (done && m == n_mon - 1)
									rmdir(year_dir);
								}
							}
						}
					}
				free(day_list[d]);
				}
			free(month_list[m]);
			if (day_list)
				free(day_list);
			}
		free(year_list[y]);
		if (month_list)
			free(month_list);
		}
	if (year_list)
		free(year_list);
	return;
	}

void
event_loop_diskusage_percent(void)
	{
	struct statvfs	st;
	struct stat		sb;
	DIR				*dir;
	struct dirent	*entry, **mp4_list = NULL;
	int				i, n, used_percent;
	boolean			diskfree_low;
	char			*loop_dir = pikrellcam.loop_dir;
	char			fname[500], *s;
	uint64_t		used_blocks = 0, fs_blocks;

	if (   !loop_dir
		|| statvfs(loop_dir, &st) != 0
	   )
		{
		log_printf("loop diskusage percent: failed to access %s", loop_dir);
		return;
		}

	/* Work with 512B blocks. st_blocks is 512B, f_blocks is f_frsize (4096)
	*/ 
	if ((dir = opendir(pikrellcam.loop_video_dir)) != NULL)
		{
		for (entry = readdir(dir); entry; entry = readdir(dir))
			{
			if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
				continue;
			snprintf(fname, sizeof(fname), "%s/%s",
						pikrellcam.loop_video_dir, entry->d_name);
			if (lstat(fname, &sb) == 0)
				used_blocks += sb.st_blocks;
			}
		closedir(dir);
		}
	if ((dir = opendir(pikrellcam.loop_thumb_dir)) != NULL)
		{
		for (entry = readdir(dir); entry; entry = readdir(dir))
			{
			if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
				continue;
			snprintf(fname, sizeof(fname), "%s/%s",
						pikrellcam.loop_thumb_dir, entry->d_name);
			if (lstat(fname, &sb) == 0)
				used_blocks += sb.st_blocks;
			}
		closedir(dir);
		}


	fs_blocks = (uint64_t) st.f_blocks * (uint64_t) (st.f_frsize / 512);
	used_percent = (int) ((100LL * used_blocks) / fs_blocks);
	diskfree_low = diskfree_is_low(pikrellcam.loop_dir);

	if (used_percent < pikrellcam.loop_diskusage_percent && !diskfree_low)
		return;

	n = scandir(pikrellcam.loop_video_dir, &mp4_list,
					mp4_filter, date_compare);
	if (n < 0)
		{
		log_printf("loop diskusage percent: scandir(%s) failed.",
				pikrellcam.loop_video_dir);
		return;
		}
	for (i = 0; i < n; ++i)
		{
		if (   used_percent >= pikrellcam.loop_diskusage_percent
		    || diskfree_low
		   )
			{
			if (pikrellcam.verbose_log)
				log_printf("loop delete(%d used,%d low): %s\n",
							used_percent, diskfree_low, fname);

			snprintf(fname, sizeof(fname), "%s/%s",
						pikrellcam.loop_video_dir, mp4_list[i]->d_name);
			if (lstat(fname, &sb) == 0)
				used_blocks -= sb.st_blocks;
			unlink(fname);

			snprintf(fname, sizeof(fname) - 3, "%s/%s",
						pikrellcam.loop_thumb_dir, mp4_list[i]->d_name);
			if ((s = strstr(fname, ".mp4")) != NULL)
				{
				strcpy(s, ".th.jpg");
				if (lstat(fname, &sb) == 0)
					used_blocks -= sb.st_blocks;
				unlink(fname);
				}
			used_percent = (int) ((100LL * used_blocks) / fs_blocks);
			diskfree_low = diskfree_is_low(pikrellcam.loop_dir);
			}
		free(mp4_list[i]);
		}
	if (mp4_list)
		free(mp4_list);
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

	if (!fname_part)
		asprintf(&fname_part, "%s.part", pikrellcam.state_filename);
	f = fopen(fname_part, "w");

	fprintf(f, "motion_enable %s\n", mf->motion_enable ? "on" : "off");
	fprintf(f, "motion_stills_enable %s\n",
						pikrellcam.motion_stills_enable ? "on" : "off");
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

	if (vcb->state & VCB_STATE_LOOP_RECORD)
		state = "loop";
	else if (vcb->state & VCB_STATE_MOTION_RECORD)
		state = "motion";
	else if (vcb->state == VCB_STATE_MANUAL_RECORD)
		state = "manual";
	else
		state = "stop";
	fprintf(f, "video_record_state %s\n", state);

	if (vcb->state & VCB_STATE_MOTION_RECORD)
		state = "video";
	else if (pikrellcam.motion_stills_record)
		state = "stills";
	else
		state = "none";
	fprintf(f, "motion_record_state %s\n", state);


	fprintf(f, "video_last %s\n",
			pikrellcam.video_last ? pikrellcam.video_last : "none");
	fprintf(f, "video_last_frame_count %d\n", pikrellcam.video_last_frame_count);
	fprintf(f, "video_last_time %.2f\n", (float) pikrellcam.video_last_time);
	fprintf(f, "video_last_fps %.2f\n", (float) pikrellcam.video_last_fps);
	fprintf(f, "audio_last_frame_count %d\n", pikrellcam.audio_last_frame_count);
	fprintf(f, "audio_last_rate %d\n", pikrellcam.audio_last_rate);

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

	fprintf(f, "motion_detects_fifo_enable %s\n",
			pikrellcam.motion_detects_fifo_enable ? "on" : "off");

	fprintf(f, "current_minute %d\n",
			pikrellcam.tm_local.tm_hour * 60 + pikrellcam.tm_local.tm_min);
	fprintf(f, "dawn %d\n", sun.dawn);
	fprintf(f, "sunrise %d\n", sun.sunrise);
	fprintf(f, "sunset %d\n", sun.sunset);
	fprintf(f, "dusk %d\n", sun.dusk);

	fprintf(f, "multicast_group_IP %s\n", pikrellcam.multicast_group_IP);
	fprintf(f, "multicast_group_port %d\n", pikrellcam.multicast_group_port);

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

	if (pikrellcam.config_media_sequence_modified)
		config_media_sequence_save();

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

		if (pikrellcam.preset_state_modified && five_minute_tick)
			{
			pikrellcam.preset_state_modified = FALSE;
			preset_state_save();
			}

		if (day_tick && !start)
			{
			pikrellcam.video_motion_sequence = 0;
			pikrellcam.video_manual_sequence = 0;
			pikrellcam.motion_stills_sequence = 0;
			pikrellcam.still_sequence = 0;
			}

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
					exec_no_wait(at->command, NULL, TRUE);
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
	"#                $C - user scripts directory full path\n"
	"#                $c - distribution scripts directory full path\n"
	"#                $I - the PiKrellCam install directory\n"
	"#                $a - archive directory full path\n"
	"#                $m - media directory full path\n"
	"#                $M - mjpeg file full path\n"
	"#                $P - command FIFO full path\n"
	"#                $G - log file full path\n"
	"#                $H - hostname\n"
	"#                $h - multicast from hostname\n"
	"#                $E - effective user running PiKrellCam\n"
	"#                $V - video files directory full path\n"
	"#                $t - thumb files directory full path\n"
	"#                $z - loop files directory full path\n"
	"#                $v - last video saved full path filename\n"
	"#                $S - still files directory full path\n"
	"#                $s - last still saved full path filename\n"
	"#                $A - last motion video or still thumb saved full path\n"
	"#                     If video with only audio or external detects,\n"
	"#                     this thumb will be renamed after the video ends:\n"
	"#                         the motion_ prefix will change to ext-XXX\n"
	"#                $L - timelapse files directory full path\n"
	"#                $l - timelapse current series filename format: tl_sssss_%%05d.jpg\n"
	"#                     in timelapse sub directory.  If used in any script\n"
	"#                     arg list, $l must be the last argument.\n"
	"#                $N - timelapse sequence last number\n"
	"#                $n - timelapse series\n"
	"#                $T - timelapse video full path filename in video sub directory\n"
	"#                $o - motion enable state\n"
	"#                $r - current motion record mode: stills or video\n"
	"#                $e - motion record type: motion, audio, FIFO\n"
	"#                     (or FIFO code).  Video can start as audio or FIFO\n"
	"#                     type and change to motion if motion is detected.\n"
	"#                $p - current preset number pair: position setting\n"
	"#                $D - current_minute dawn sunrise sunset dusk\n"
	"#                $X - motion video sequence number\n"
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
