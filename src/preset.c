
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



#define	PREV_POSITION		0
#define	NEXT_POSITION		1
#define	PREV_SETTINGS		2
#define	NEXT_SETTINGS		3
#define GOTO				4
#define PRESET_NEW			5
#define PRESET_COPY			6
#define PRESET_DELETE		7
#define MOVE_ONE			8
#define MOVE_ALL			9

typedef struct
	{
	char *name;
	int	 id,
	     n_args;
	}
PresetCommand;

static PresetCommand preset_commands[] =
	{
		{ "prev_position",	PREV_POSITION,	0 },
		{ "next_position",	NEXT_POSITION,	0 },
		{ "prev_settings",	PREV_SETTINGS,	0 },
		{ "next_settings",	NEXT_SETTINGS,	0 },
		{ "goto",			GOTO,     		1 },
		{ "move_one",		MOVE_ONE,     	0 },
		{ "move_all",		MOVE_ALL,     	0 },
		{ "new",			PRESET_NEW,		0 },
		{ "copy",			PRESET_COPY,	0 },
		{ "delete",			PRESET_DELETE,	0 }
	};

#define N_PRESET_COMMANDS    (sizeof(preset_commands) / sizeof(PresetCommand))


static void
preset_notify(int count)
	{
	Event	*event;

	event_list_lock();
	if ((event = event_find("preset notify")) != NULL)
		event->count = count;		/* extend time of existing notify */
	event_list_unlock();
	if (!event)
		event_count_down_add("preset notify", count,
				event_notify_expire, &pikrellcam.preset_notify);
	pikrellcam.preset_notify = TRUE;
	}

static int
pan_position_cmp(void *data1, void *data2)
	{
	PresetPosition	*pos1 = (PresetPosition *) data1,
					*pos2 = (PresetPosition *) data2;

	if (pos1->pan > pos2->pan)
		return 1;
	return 0;
	}

PresetPosition *
preset_find_at_position(int pan, int tilt)
	{
	PresetPosition	*pos = NULL;
	SList			*list;
	int				index = 0;

	for (list = pikrellcam.preset_position_list; list; list = list->next)
		{
		pos = (PresetPosition *) list->data;
		if (pos->pan == pan && (tilt == 0 || tilt == pos->tilt))
			{
			pikrellcam.preset_position_index = index;
			break;
			}
		++index;
		pos = NULL;
		}
	return pos;
	}


  /* Called from locked servo_control.mutex in servo_thread()
  */
void
preset_on_check(int pan, int tilt)
	{
	PresetPosition	*pos;

	if ((pos = preset_find_at_position(pan, tilt)) != NULL)
		{
		pikrellcam.on_preset = TRUE;
		pikrellcam.preset_last_on = pos;
		preset_load_values(FALSE);
		preset_notify(22);
		}
	}

void
preset_pan_range(int *max, int *min)
	{
	PresetPosition	*pos = NULL;
	SList			*list;

	*max = SERVO_MIN_WIDTH;
	*min = SERVO_MAX_WIDTH;

	for (list = pikrellcam.preset_position_list; list; list = list->next)
		{
		pos = (PresetPosition *) list->data;
		if (pos->pan > *max)
			*max = pos->pan;
		if (pos->pan < *min)
			*min = pos->pan;
		}
	}

void
preset_tilt_range(int *max, int *min)
	{
	PresetPosition	*pos = NULL;
	SList			*list;

	*max = SERVO_MIN_WIDTH;
	*min = SERVO_MAX_WIDTH;

	for (list = pikrellcam.preset_position_list; list; list = list->next)
		{
		pos = (PresetPosition *) list->data;
		if (pos->tilt > *max)
			*max = pos->tilt;
		if (pos->tilt < *min)
			*min = pos->tilt;
		}
	}

static int
preset_position_prev(void)
	{
	PresetPosition	*pos;
	SList			*list;
	int				idx, pan, tilt;

	servo_get_position(&pan, &tilt);
	idx = -1;
	for (list = pikrellcam.preset_position_list; list; list = list->next)
		{
		pos = (PresetPosition *) list->data;
		if (pos->pan >= pan)
			{
			if (pos->pan == pan && pos->tilt != tilt)
				++idx;	/* stay on same pan position */
			break;
			}
		++idx;
		}
	if (idx >= pikrellcam.n_preset_positions)
		idx = pikrellcam.n_preset_positions - 1;
	if (idx == -1)
		idx = 0;
	return idx;
	}

static int
preset_position_next(void)
	{
	PresetPosition	*pos;
	SList			*list;
	int				idx, pan, tilt;

	servo_get_position(&pan, &tilt);
	idx = 0;
	for (list = pikrellcam.preset_position_list; list; list = list->next)
		{
		pos = (PresetPosition *) list->data;
		if (   (pos->pan == pan && pos->tilt != tilt)
		    || pos->pan > pan
		   )
			break;
		++idx;
		}
	if (idx >= pikrellcam.n_preset_positions)
		idx = pikrellcam.n_preset_positions - 1;
	return idx;
	}

void
preset_load_values(boolean do_pan)
	{
	PresetPosition	*pos;
	PresetSettings	*settings;
	Event			*event;
	MotionFrame		*mf = &motion_frame;
	SList			*rlist;
	char			*region, buf[100];
	boolean			save_show;

	pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list,
				pikrellcam.preset_position_index);
	if (!pos)
		return;

//printf("preset_load %d %d (%d %d) servo:%d %d\n",
//pikrellcam.have_servos, do_pan,
//pikrellcam.preset_position_index, pos->settings_index, pos->pan, pos->tilt);

	if (pikrellcam.have_servos && do_pan && pos->pan > 0)
		{
		event_list_lock();
		if ((event = event_find("preset notify")) != NULL)
			if (event->count > 0)
				event->count = 1;
		event_list_unlock();
		servo_move(pos->pan, pos->tilt, pikrellcam.servo_preset_step_msec);

		/* preset_load_values(FALSE) will be called again after move
		|  if the move ends up on a preset
		*/
		return;
		}
	if (!pikrellcam.have_servos)
		{
		pikrellcam.on_preset = TRUE;
		pikrellcam.preset_last_on = pos;
		}

	settings = (PresetSettings *) slist_nth_data(pos->settings_list, pos->settings_index);
	if (settings)
		{
		pikrellcam.motion_magnitude_limit = settings->mag_limit;
		pikrellcam.motion_magnitude_limit_count = settings->mag_limit_count;
		pikrellcam.motion_burst_count = settings->burst_count;
		pikrellcam.motion_burst_frames = settings->burst_frames;
		}
	save_show = mf->show_preset;
	motion_command("delete_regions all");
	for (rlist = settings->region_list; rlist; rlist = rlist->next)
		{
		region = (char *) rlist->data;
		snprintf(buf, sizeof(buf), "%s", region);
		motion_command(buf);
		}
	mf->show_preset = save_show;
	pikrellcam.preset_modified_warning = FALSE;
	pikrellcam.state_modified = TRUE;
	}

static void
preset_settings_regions_set(PresetSettings *settings)
	{
	MotionFrame		*mf = &motion_frame;
	MotionRegion	*mreg;
	SList			*list;
	char			*region;

	if (!settings)
		return;
	slist_and_data_free(settings->region_list);
	settings->region_list = NULL;
	for (list = mf->motion_region_list; list; list = list->next)
		{
		mreg = (MotionRegion *) list->data;
		asprintf(&region, "add_region %.3f %.3f %.3f %.3f\n",
				mreg->xf0, mreg->yf0, mreg->dxf, mreg->dyf);
		settings->region_list = slist_append(settings->region_list, region);
		}
	}

void
preset_settings_set_modified(void)
	{
	PresetPosition	*pos = NULL;
	PresetSettings	*settings = NULL;
	int				pan, tilt;

	servo_get_position(&pan, &tilt);
	if ((pos = preset_find_at_position(pan, tilt)) != NULL)
		{
		settings = (PresetSettings *) slist_nth_data(pos->settings_list,
						pos->settings_index);
		if (settings)
			{
			settings->mag_limit       = pikrellcam.motion_magnitude_limit;
			settings->mag_limit_count = pikrellcam.motion_magnitude_limit_count;
			settings->burst_count  = pikrellcam.motion_burst_count;
			settings->burst_frames = pikrellcam.motion_burst_frames;
			pikrellcam.preset_modified = TRUE;
			}
		}
	else
		pikrellcam.preset_modified_warning = TRUE;
	}

  /* Called from locked mf->region_list_mutex when a region is modified.
  */
void
preset_regions_set_modified(void)
	{
	PresetPosition	*pos = NULL;
	PresetSettings	*settings = NULL;
	int				pan, tilt;

	servo_get_position(&pan, &tilt);
	if ((pos = preset_find_at_position(pan, tilt)) != NULL)
		{
		settings = (PresetSettings *) slist_nth_data(pos->settings_list,
						pos->settings_index);
		preset_settings_regions_set(settings);
		pikrellcam.preset_modified = TRUE;
		}
	else
		pikrellcam.preset_modified_warning = TRUE;
	}

static void
preset_new(PresetPosition *pos_src)
	{
	PresetPosition	*pos = NULL;
	PresetSettings	*settings = NULL, *settings_src;
	SList			*slist, *rlist;
	char			*region;
	int				pan, tilt;

	servo_get_position(&pan, &tilt);
	if (pikrellcam.preset_position_list)
		{
		pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list,
						pikrellcam.preset_position_index);
		if (   pikrellcam.have_servos
		    && (pos->pan != pan || pos->tilt != tilt)
		   )
			pos = NULL;
		}
	if (!pos)
		{
		pos = calloc(1, sizeof(PresetPosition));
		pos->pan = pan;
		pos->tilt = tilt;
		pos->n_settings = 0;
		pos->settings_index = -1;
		pikrellcam.preset_position_list =
				slist_insert_sorted(pikrellcam.preset_position_list, pos,
						pan_position_cmp);
		pikrellcam.n_preset_positions += 1;
		pikrellcam.preset_position_index =
				slist_index(pikrellcam.preset_position_list, pos);
		if (pikrellcam.preset_position_index == -1)
			pikrellcam.preset_position_index = 0;
		}
	else
		pos_src = NULL;

	if (pos_src)
		{
		for (slist = pos_src->settings_list; slist; slist = slist->next)
			{
			settings_src = (PresetSettings *) slist->data;
			settings = calloc(1, sizeof(PresetSettings));
			settings->mag_limit = settings_src->mag_limit;
			settings->mag_limit_count = settings_src->mag_limit_count;
			settings->burst_count = settings_src->burst_count;
			settings->burst_frames = settings_src->burst_frames;
			for (rlist = settings_src->region_list; rlist; rlist = rlist->next)
				{
				region = strdup((char *) rlist->data);
				settings->region_list = slist_append(settings->region_list, region);
				}
			pos->settings_list = slist_append(pos->settings_list, settings);
			pos->n_settings += 1;
			}
		pos->settings_index = pos_src->settings_index;
		}
	else
		{
		settings = calloc(1, sizeof(PresetSettings));
		pos->settings_index += 1;
		pos->settings_list = slist_insert(pos->settings_list,
				settings, pos->settings_index);
		pos->n_settings += 1;
		settings->mag_limit = pikrellcam.motion_magnitude_limit;
		settings->mag_limit_count = pikrellcam.motion_magnitude_limit_count;
		settings->burst_count = pikrellcam.motion_burst_count;
		settings->burst_frames = pikrellcam.motion_burst_frames;
		preset_settings_regions_set(settings);
		preset_notify(18);
		}
	pikrellcam.preset_modified = TRUE;
	pikrellcam.preset_modified_warning = FALSE;
	pikrellcam.on_preset = TRUE;
	pikrellcam.preset_last_on = pos;
	}

static void
preset_delete(void)
	{
	PresetPosition	*pos = NULL;
	PresetSettings	*settings = NULL;
	int				pan, tilt, idx;
	char			buf[100];

	if (!pikrellcam.preset_position_list)
		return;
	if (pikrellcam.have_servos)
		{
		servo_get_position(&pan, &tilt);
		if ((pos = preset_find_at_position(pan, tilt)) == NULL)
			{
			display_inform("\"Position is not on a preset.\" 3 3 1");
			display_inform("\"Cannot delete.\" 4 3 1");
			display_inform("timeout 3");
			return;
			}
		}
	else
		pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list,
					pikrellcam.preset_position_index);

	if (pikrellcam.n_preset_positions == 1 && pos->n_settings == 1)
		{
		display_inform("\"You must have at least one preset.\" 3 3 1");
		display_inform("\"Cannot delete your only preset.\" 4 3 1");
		display_inform("timeout 3");
		return;
		}
	settings = (PresetSettings *) slist_nth_data(pos->settings_list, pos->settings_index);
	slist_and_data_free(settings->region_list);
	pos->settings_list = slist_remove(pos->settings_list, settings);
	free(settings);

	pos->n_settings -= 1;
	if ((idx = pos->settings_index) >= pos->n_settings)
		pos->settings_index -= 1;
	if (pos->n_settings <= 0)
		{
		display_inform("\"Deleted preset at current position.\" 3 3 1");
		display_inform("timeout 2");
		pikrellcam.preset_position_list = slist_remove(pikrellcam.preset_position_list, pos);
		free(pos);
		pikrellcam.n_preset_positions -= 1;
		pikrellcam.on_preset = FALSE;
		pikrellcam.preset_last_on = NULL;
		if (pikrellcam.preset_position_index > 0)
			pikrellcam.preset_position_index -= 1;
		}
	else
		{
		snprintf(buf, sizeof(buf), "\"Deleted preset settings %d.\" 3 3 1", idx + 1);
		display_inform(buf);
		display_inform("timeout 2");
		}
	preset_load_values(FALSE);
	preset_notify(18);
	pikrellcam.preset_modified = TRUE;
	}


void
preset_command(char *cmd_line)
	{
	PresetCommand 	*pcmd;
	PresetPosition	*pos;
	SList			*list;
	int				i, n, id = -1;
	int				pan, tilt, dpan, dtilt;
	char			buf[64], arg1[32];
	boolean			move_all = FALSE;

	arg1[0] = '\0';
	n = sscanf(cmd_line, "%63s %[^\n]", buf, arg1);
	if (n < 1)
		return;

	for (i = 0; i < N_PRESET_COMMANDS; ++i)
		{
		pcmd = &preset_commands[i];
		if (!strcmp(pcmd->name, buf))
			{
			if (pcmd->n_args <= n - 1)
				id = pcmd->id;
			break;
			}
		}
	if (id == -1)
		{
//      inform_message("Bad preset command.");
		return;
		}

	servo_get_position(&pan, &tilt);
	switch (id)
		{
		case PREV_POSITION:
			pikrellcam.preset_position_index = preset_position_prev();
			preset_load_values(TRUE);
			break;

		case NEXT_POSITION:
			pikrellcam.preset_position_index = preset_position_next();
			preset_load_values(TRUE);
			break;

		case PREV_SETTINGS:
			pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list, pikrellcam.preset_position_index);
			if (pos)
				{
				if (pos->tilt == tilt && pos->pan == pan)
					{
					if (pos->settings_index > 0)
						{
						--pos->settings_index;
						preset_load_values(FALSE);
						preset_notify(18);
						}
					else
						preset_notify(10);
					}
				else
					preset_load_values(TRUE);
				}
			break;

		case NEXT_SETTINGS:
			pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list, pikrellcam.preset_position_index);
			if (pos)
				{
				if (pos->tilt == tilt && pos->pan == pan)
					{
					n = slist_length(pos->settings_list);
					if (pos->settings_index < n - 1)
						{
						++pos->settings_index;
						preset_load_values(FALSE);
						preset_notify(18);
						}
					else
						preset_notify(10);
					}
				else
					preset_load_values(TRUE);
				}
			break;

		case MOVE_ALL:
			move_all = TRUE;
		case MOVE_ONE:
			if (pikrellcam.on_preset || !pikrellcam.preset_last_on)
				{
				if (pikrellcam.on_preset)
					display_inform("\"Servo position is on a preset.\" 2 3 1");
				else
					display_inform("\"Move to a preset first, then off.\" 2 3 1");
				display_inform("\"Servos must be moved off a preset\" 3 3 1");
				display_inform("\"before the preset can be moved.\" 4 3 1");
				display_inform("timeout 3");
				}
			else if (pikrellcam.preset_last_on)
				{
				dpan = pan - pikrellcam.preset_last_on->pan;
				dtilt = tilt - pikrellcam.preset_last_on->tilt;
				for (list = pikrellcam.preset_position_list; list; list = list->next)
					{
					pos = (PresetPosition *) list->data;
					if (move_all || pos == pikrellcam.preset_last_on)
						{
						if (   pos->pan + dpan > pikrellcam.servo_pan_max
						    || pos->pan + dpan < pikrellcam.servo_pan_min
						    || pos->tilt + dtilt > pikrellcam.servo_tilt_max
						    || pos->tilt + dtilt < pikrellcam.servo_tilt_min
						   )
							{
							display_inform("\"Error:\" 2 3 1");
							display_inform("\"Can't move a preset past\" 3 3 1");
							display_inform("\"servo pan/tilt limits.\" 4 3 1");
							display_inform("timeout 3");
							dpan = dtilt = 0;
							break;
							}
						}
					}
				if (move_all)
					{
					for (list = pikrellcam.preset_position_list; list; list = list->next)
						{
						pos = (PresetPosition *) list->data;
						pos->pan += dpan;
						pos->tilt += dtilt;
						}
					}
				else
					{
					pos = pikrellcam.preset_last_on;
					pos->pan += dpan;
					pos->tilt += dtilt;

					/* Re insert sorted in case moved past another preset.
					*/
					pikrellcam.preset_position_list =
						slist_remove(pikrellcam.preset_position_list, pos);
					pikrellcam.preset_position_list =
						slist_insert_sorted(pikrellcam.preset_position_list,
								pos, pan_position_cmp);
					}
				if (dpan != 0 || dtilt != 0)
					{
					preset_on_check(pan, tilt);
					pikrellcam.preset_modified = TRUE;
					}
				}
			break;

		case GOTO:
			if (sscanf(arg1, "%d %d", &n, &i) == 2)
				{
				if (--n < 0)	/* n was 1 based position number */
					n = pikrellcam.preset_position_index;
				pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list, n);
				if (pos)
					{
					pikrellcam.preset_position_index = n;
					--i;		/* settings number was 1 based */
					if (i < 0 || i >= slist_length(pos->settings_list))
						i = 0;
					pos->settings_index = i;
					preset_load_values(TRUE);
					preset_notify(18);
					}
				}
			break;

		case PRESET_NEW:
			pos = preset_find_at_position(pan, 0);
			if (pos)
				{
				if (pos->tilt == tilt)
					{
					preset_new(NULL);
					display_inform("\"Created new settings at current preset\" 3 3 1");
					display_inform("timeout 2");
					}
				else
					{
					display_inform("\"Pan position is on a preset but tilt\" 2 3 1");
					display_inform("\"is not.  Cannot create new settings\" 3 3 1");
					display_inform("\"for this preset.\" 4 3 1");
					display_inform("timeout 3");
					}
				}
			else
				{
				preset_new(NULL);
				display_inform("\"Created preset at new position.\" 3 3 1");
				display_inform("timeout 2");
				}
			break;

		case PRESET_COPY:
			pos = preset_find_at_position(pan, 0);
			if (pos || !pikrellcam.preset_last_on)
				{
				if (pos)
					display_inform("\"Pan position is on a preset.\" 2 3 1");
				else
					display_inform("\"Move to a preset first, then off.\" 2 3 1");
				display_inform("\"Pan servo must be moved off a\" 3 3 1");
				display_inform("\"preset before you can copy it.\" 4 3 1");
				display_inform("timeout 3");
				}
			else
				{
				preset_new(pikrellcam.preset_last_on);
				display_inform("\"Copied preset into new position.\" 3 3 1");
				display_inform("timeout 2");
				}
			break;

		case PRESET_DELETE:
			preset_delete();
			break;
		}
	}

void
preset_config_load(void)
	{
	FILE			*f;
	PresetPosition	*pos = NULL;
	PresetSettings	*settings = NULL;
	char			*region;
	int				pan, tilt;
	char			buf[100];

	if ((f = fopen(pikrellcam.preset_config_file, "r")) == NULL)
		{
		preset_new(NULL);
		preset_config_save();
		return;
		}
	while (fgets(buf, sizeof(buf), f) != NULL)
		{
		if (buf[0] == '#' || buf[0] == '\n')
			continue;
		if (sscanf(buf, "<position %d %d>", &pan, &tilt) == 2)
			{
			pos = calloc(1, sizeof(PresetPosition));
			pos->pan = pan;
			pos->tilt = tilt;
			pikrellcam.preset_position_list = slist_append(pikrellcam.preset_position_list, pos);
			pikrellcam.n_preset_positions += 1;
			continue;
			}
		if (!pos)
			continue;
		if (!strncmp(buf, "<settings>", 10))
			{
			settings = calloc(1, sizeof(PresetSettings));
			pos->settings_list = slist_append(pos->settings_list, settings);
			pos->n_settings += 1;
			continue;
			}
		if (!settings)
			continue;

		if (!strncmp(buf, "motion", 6))
			sscanf(buf + 7, "%d %d %d %d",
				&settings->mag_limit, &settings->mag_limit_count,
				&settings->burst_count, &settings->burst_frames);

		if (!strncmp(buf, "magnitude_limit", 15))
			sscanf(buf + 16, "%d", &settings->mag_limit);
		else if (!strncmp(buf, "magnitude_count", 15))
			sscanf(buf + 16, "%d", &settings->mag_limit_count);
		else if (!strncmp(buf, "burst_count", 11))
			sscanf(buf + 12, "%d", &settings->burst_count);
		else if (!strncmp(buf, "burst_frames", 12))
			sscanf(buf + 13, "%d", &settings->burst_frames);
		else if (!strncmp(buf, "add_region", 10))
			{
			region = strdup(buf);	/* string "add_region xf0 yf0 dxf dyf" */
			settings->region_list = slist_append(settings->region_list, region);
			}
		}
	fclose(f);
	/* read preset state file to load position_index, settings_index */
	}

void
preset_config_save(void)
	{
	FILE			*f;
	SList			*list, *slist, *rlist;
	PresetPosition	*pos;
	PresetSettings	*settings;
	char			*region;

	if ((f = fopen(pikrellcam.preset_config_file, "w")) == NULL)
		{
		log_printf("Failed to save preset config file %s. %m\n",
				pikrellcam.preset_config_file);
		return;
		}
	for (list = pikrellcam.preset_position_list; list; list = list->next)
		{
		pos = (PresetPosition *) list->data;
		fprintf(f, "<position %d %d>\n", pos->pan, pos->tilt);
		for (slist = pos->settings_list; slist; slist = slist->next)
			{
			settings = (PresetSettings *) slist->data;
			fprintf(f, "<settings>\n");
			fprintf(f, "magnitude_limit %d\n", settings->mag_limit);
			fprintf(f, "magnitude_count %d\n", settings->mag_limit_count);
			fprintf(f, "burst_count %d\n", settings->burst_count);
			fprintf(f, "burst_frames %d\n", settings->burst_frames);
			for (rlist = settings->region_list; rlist; rlist = rlist->next)
				{
				region = (char *) rlist->data;	/* string "add_region xf0 yf0 dxf dyf" */
				fprintf(f, "%s", region);
				}
			}
		fprintf(f, "\n");
		}
	fclose(f);
	pikrellcam.preset_modified = FALSE;
	}

void
preset_state_save(void)
	{
	FILE			*f;
	PresetPosition	*pos;
	SList			*list;
	int				pan, tilt;

	f = fopen(pikrellcam.preset_state_file, "w");
	if (f)
		{
		servo_get_position(&pan, &tilt);
		fprintf(f, "pan %d\n", pan);
		fprintf(f, "tilt %d\n", tilt);
		fprintf(f, "position_index %d\n", pikrellcam.preset_position_index);
		for (list = pikrellcam.preset_position_list; list; list = list->next)
			{
			pos = (PresetPosition *) list->data;
			fprintf(f, "settings_index %d\n", pos->settings_index);
			}
		fclose(f);
		}
	}

void
preset_state_load(void)
	{
	FILE			*f;
	SList			*list = pikrellcam.preset_position_list;
	PresetPosition	*pos;
	int				pan = 150, tilt = 150, i;
	char	buf[100];

	if ((f = fopen(pikrellcam.preset_state_file, "r")) == NULL)
		{
		preset_load_values(TRUE);
		return;
		}
	while (fgets(buf, sizeof(buf), f) != NULL)
		{
		if (buf[0] == '#' || buf[0] == '\n')
			continue;
/* XXX */
		if (!strncmp(buf, "position_index settings_index pan tilt", 38))
			{
			fgets(buf, sizeof(buf), f);
			sscanf(buf, "%d %d %d %d\n", &pikrellcam.preset_position_index,
					&i, &pan, &tilt);
			break;
			}
/* XXX */

		if (!strncmp(buf, "pan", 3))
			sscanf(buf + 4, "%d", &pan);
		else if (!strncmp(buf, "tilt", 4))
			sscanf(buf + 5, "%d", &tilt);
		else if (!strncmp(buf, "position_index", 14))
			sscanf(buf + 15, "%d", &pikrellcam.preset_position_index);
		else if (!strncmp(buf, "settings_index", 14) && list)
			{
			pos = (PresetPosition *) list->data;
			sscanf(buf + 15, "%d", &pos->settings_index);
			if (   pos->settings_index < 0
			    || pos->settings_index > slist_length(pos->settings_list) - 1
			   )
				pos->settings_index = 0;
			list = list->next;
			}
		}
	fclose(f);
	if (   pikrellcam.preset_position_index < 0
	    || pikrellcam.preset_position_index > pikrellcam.n_preset_positions - 1
	   )
		pikrellcam.preset_position_index = 0;
	if (pan < pikrellcam.servo_pan_min || pan > pikrellcam.servo_pan_max)
		pan = 150;
	if (tilt < pikrellcam.servo_tilt_min || tilt > pikrellcam.servo_tilt_max)
		tilt = 150;

	/* Don't move to the preset position because we may not have been on
	|  a preset when stopped.  Just move to saved pan/tilt.
	*/
	preset_load_values(FALSE);
	if (pikrellcam.have_servos)
		servo_move(pan, tilt, pikrellcam.servo_move_step_msec);
	else
		preset_notify(22);
	}


