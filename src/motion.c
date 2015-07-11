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

#define EXPMA_SMOOTHING	0.01


MotionFrame		motion_frame;

static int		any_count,
				cvec_count;

static CompositeVector	zero_cvec;


static boolean
composite_vector_best(CompositeVector *cvec1, CompositeVector *cvec2)
	{
	int		d1, d2,
			xm = motion_frame.width / 2,
			xt = motion_frame.width / 3;
	boolean	result = FALSE;

	d1 = abs(xm - cvec1->x);
	d2 = abs(xm - cvec2->x);
	if (d2 < xt)
		{
		if (d1 < xt && cvec1->mag2_count > cvec2->mag2_count)
			result = TRUE;
		}
	else if (d1 < d2)
		result = TRUE;

	return result;
	}

static void
get_composite_vector(MotionFrame *mf, MotionRegion *mreg)
	{
	CompositeVector	*cvec, tvec;
	MotionVector	*mv;
	int				x, y, mag2, mb_index, mvmag2, dot, cos2,
					x0, y0, x1, y1,
					in_box_count;
	int16_t			*pm, *pp, *pn;

	cvec = &mreg->vector;
	*cvec = zero_cvec;
	tvec = zero_cvec;
	mreg->reject_count = 0;
	mreg->sparkle_count = 0;

	/* Don't look at frame perimeter blocks except when excluding sparkles.
	|  (They have limited vector direction).
	*/
	if ((y0 = mreg->y) == 0)
		y0 = 1;
	if ((y1 = mreg->y + mreg->dy) == mf->height)
		y1 = mf->height - 1;
	if ((x0 = mreg->x) == 0)
		x0 = 1;
	if ((x1 = mreg->x + mreg->dx) == mf->width)
		y1 = mf->width - 1;

	/* First pass, filter out any vector < mag2_limit
	*/
	for (y = y0; y < y1; ++y)
		{
		for (x = x0; x < x1; ++x)
			{
			mb_index = mf->width * y + x;
			mv = &mf->vectors[mb_index];
			mag2 = mv->vx * mv->vx + mv->vy * mv->vy;
			if (mag2 >= mf->mag2_limit)
				{
				*(mf->trigger + mb_index) = mag2;
				any_count += 1;
				}
			}
		}

	/* Remove isolated sparkles & build the initial composite vector which
	|  will then consist of motion vector clustering of at least count 2.
	|  Camera can produce large sparkle counts during dim light (dusk/dawn).
	*/
	for (y = y0; y < y1; ++y)
		{
		for (x = x0; x < x1; ++x)
			{
			pm = mf->trigger + mf->width * y + x;
			pp = pm - mf->width;
			pn = pm + mf->width;
			if (   *pm
			    && !*(pm - 1) && !*(pm + 1)
				&& !*(pp - 1) && !*pp && !*(pp + 1)
				&& !*(pn - 1) && !*pn && !*(pn + 1)
			   )
				{
				*pm = 1;	/* mag2 value becomes a sparkle flag */
				mf->sparkle_count += 1;
				mreg->sparkle_count += 1;
				}
			if (*pm > 1)
				{
				mv = &mf->vectors[mf->width * y + x];
				tvec.mag2_count += 1;
				tvec.vx += mv->vx;
				tvec.vy += mv->vy;
				tvec.x  += x;
				tvec.y  += y;
				}
			}
		}

	/* If we are left with enough counts for a composite vector, filter out
	|  motion vectors not pointing in the composite directon.
	|  Vectors of sufficient mag2 but not pointing in the right direction
	|  will be rejects and their count can be large for noisy frames.
	|  Dot product to allow a spread,
	|    (avoiding sqrt() to get magnitude and scale by 100 for integer math):
	|
	|  cos(a) = (v1 dot v2) / mag(v1) * mag(v2))	# cos(25) = 0.906
	|  100 * cos(25)^2 = 82 = 100 * (v1 dot v2)^2 / mag(v1)^2 * mag(v2)^2)
	*/
	if (tvec.mag2_count > mf->mag2_limit_count)
		{
		tvec.vx /= tvec.mag2_count;
		tvec.vy /= tvec.mag2_count;
		tvec.mag2 = tvec.vx * tvec.vx + tvec.vy * tvec.vy;

		for (y = y0; y < y1; ++y)
			{
			for (x = x0; x < x1; ++x)
				{
				mb_index = mf->width * y + x;
				if ((mvmag2 = *(mf->trigger + mb_index)) <= 1)
					continue;

				mv = &mf->vectors[mb_index];
				dot = tvec.vx * mv->vx + tvec.vy * mv->vy;
				if (   dot > 0		/* angle at least < 90 deg */
				    && mvmag2 > 0
				    && tvec.mag2 > 0
					&& (cos2 = 100 * dot * dot / (tvec.mag2 * mvmag2)) >= 82
				   )
					{
					mf->trigger_count += 1;
					cvec->mag2_count += 1;
					cvec->vx += mv->vx;
					cvec->vy += mv->vy;
					cvec->x  += x;
					cvec->y  += y;
					}
				else
					{
					*(mf->trigger + mf->width * y + x) = 2;
					mf->reject_count += 1;
					mreg->reject_count += 1;
					}
				}
			}
		}

	/* Now every vector in mag2_count has magnitude >= mag2_limit and points
	|  with some spread in the same direction.  If there is enough of them,
	|  we have a final composite vector.  But look at the motion vector
	|  distribution/concentration within the region to filter for final
	|  motion detection.
	*/
	mreg->motion = 0;
	if (cvec->mag2_count >= mf->mag2_limit_count)
		{
		cvec_count += 1;
		cvec->x /= cvec->mag2_count;
		cvec->y /= cvec->mag2_count;
		cvec->vx /= cvec->mag2_count;
		cvec->vy /= cvec->mag2_count;
		cvec->mag2 = cvec->vx * cvec->vx + cvec->vy * cvec->vy;

		/* Set a vertical flag.  The idea was vertical filtering would be
		|  usefull for rain, but it turns out it's just as likely the
		|  vectors will match in some random direction and reject counts
		|  are a better rain filter, but look again later.
		*/
		cvec->vertical =
			(cvec->vy * cvec->vy > 20 * cvec->vx * cvec->vx) ? TRUE : FALSE;
		if (cvec->vertical)
			mf->vertical_count += 1;

		/* Get a box that will hold at least 2x mag2_limit_count vectors
		|  and count the vectors within the box.
		*/
		for (mreg->box_w = 4, mreg->box_h = 4;
					mreg->box_w * mreg->box_h <= 2 * cvec->mag2_count;   )
			{
			if (mreg->box_h <= mreg->box_w)
				mreg->box_h += 2;
			else
				mreg->box_w += 2;
			}

		in_box_count = 0;
		for (y = cvec->y - mreg->box_h / 2; y <= cvec->y + mreg->box_h / 2; ++y)
			{
			for (x = cvec->x - mreg->box_w / 2; x <= cvec->x + mreg->box_w / 2; ++x)
				{
				if (*(mf->trigger + mf->width * y + x) > 2)
					in_box_count += 1;
				}
			}

		/* Filter out smaller fast moving objects which can be fast bird fly
		|  bys or close flying insects.
		|  For smaller objects, always enforce reject_count rejections
		|  (filters noisy frames: rain, camera burps).
		|  Comparison ratios here are empirical from looking at
		|  motion-debug output wrt drawn OSD motion vector frames.
		*/
		if (   !cvec->vertical
		    || !pikrellcam.motion_vertical_filter
		   )
			{
			if (cvec->mag2_count < 15)	/* Small object size could be configurable */
				{
				if (   in_box_count > cvec->mag2_count * 8 / 10
					&& cvec->mag2 < 5 * mf->mag2_limit
				    && mreg->reject_count < cvec->mag2_count / 2
				   )
					mreg->motion = 1;
				}
			else if (   in_box_count > cvec->mag2_count * 7 / 10
			         || (   in_box_count > cvec->mag2_count * 5 / 10
			             && mreg->reject_count < cvec->mag2_count * 4 / 10
			            )
			        )
				mreg->motion = 2;
			}
			/* else assume it's not concentrated enough for a motion cvec */

		if (   mreg->motion > 0
		    && composite_vector_best(cvec, &mf->best_frame_vector)
		   )
			mf->best_frame_vector = *cvec;

		if (pikrellcam.verbose_motion)
			{
			printf(
"cvec[%d]: x,y(%d,%d) dx,dy(%d,%d) mag2,count(%d,%d) reject:%d box:%dx%d\n",
				mreg->region_number,
				cvec->x, cvec->y, cvec->vx, cvec->vy, cvec->mag2,
				cvec->mag2_count, mreg->reject_count,
				mreg->box_w, mreg->box_h);
			printf(
"   in_box_count:%d motion:%d vetical: %d sparkle:%d\n",
				in_box_count, mreg->motion,
				cvec->vertical, mreg->sparkle_count);
			}
		}
	else
		*cvec = zero_cvec;
	}

void
motion_frame_process(VideoCircularBuffer *vcb, MotionFrame *mf)
	{
	MotionRegion	*mreg;
	SList 			*mrlist;
	int				motion_count, fail_count;
	char			tbuf[50];

	/* Allow some startup camera settle time before motion detecting.
	*/
	if (pikrellcam.t_now < pikrellcam.t_start + 3)
		return;

	mf->trigger_count = 0;
	mf->motion_status = MOTION_NONE;

	mf->sparkle_count = 0;
	mf->reject_count = 0;
	mf->vertical_count = 0;

	memset(mf->trigger, 0, MF_TRIGGER_SIZE);
	any_count = 0;
	cvec_count = 0;
	mf->best_frame_vector = zero_cvec;

	pthread_mutex_lock(&mf->region_list_mutex);
	for (mrlist = mf->motion_region_list; mrlist; mrlist = mrlist->next)
		{
		mreg = (MotionRegion *) mrlist->data;

		/* Reset mag2 values at each call in case later I do some kind of
		|  mag2_limit auto adjust in get_composite_vector().
		*/
		mf->mag2_limit  = pikrellcam.motion_magnitude_limit * pikrellcam.motion_magnitude_limit;
		mf->mag2_limit_count = pikrellcam.motion_magnitude_limit_count;
		get_composite_vector(mf, mreg);
		}
	pthread_mutex_unlock(&mf->region_list_mutex);

	/* To be used (large sparkle counts after sunset / before sunrise).
	*/
	mf->sparkle_expma = EXPMA_SMOOTHING * (float) mf->sparkle_count +
							(1.0 - EXPMA_SMOOTHING) * mf->sparkle_expma;

	if (pikrellcam.verbose_motion && any_count > 3)
		printf("any:%d reject:%d sparkle:%d sparkle_expma:%.1f\n",
			any_count, mf->reject_count, mf->sparkle_count, mf->sparkle_expma);

	motion_count = 0;
	fail_count = 0;

	for (mrlist = mf->motion_region_list; mrlist; mrlist = mrlist->next)
		{
		mreg = (MotionRegion *) mrlist->data;

		if (   (   mreg->vector.mag2_count > 0
		        || mreg->sparkle_count > 3   /* Look more at this */
		       )
		    && mreg->motion == 0
		   )
			fail_count += 1;
		else if (mreg->motion > 0)
			motion_count += 1;
		}

	mf->motion_status = MOTION_NONE;
	if (   motion_count > 0
	    && fail_count == 0
	   )
		{
		if (mf->frame_window == 0 && pikrellcam.motion_times.confirm_gap > 0)
			mf->frame_window = pikrellcam.camera_adjust.video_fps *
					pikrellcam.motion_times.confirm_gap / pikrellcam.mjpeg_divider;
		else
			mf->motion_status = MOTION_DETECTED;
		}
	if (mf->frame_window > 0)
		mf->frame_window -= 1;

	if (pikrellcam.verbose_motion && any_count > 3)
		{
		strftime(tbuf, sizeof(tbuf), "%T", &pikrellcam.tm_local);
		printf(
"%s motion count:%d fail:%d window:%d  %s\n",
			tbuf, motion_count, fail_count, mf->frame_window,
			(mf->motion_status == MOTION_DETECTED) ? "***MOTION***" : "");
		printf("\n");
		}

	if (mf->motion_status == MOTION_DETECTED  && mf->motion_enable)
		{
		vcb->motion_last_detect_time = pikrellcam.t_now;

		/* Motion detection will be ignored if a manual record is in progress.
		*/
		if (vcb->state == VCB_STATE_NONE)
			{
			/* Always preview save in case there is a motion preview save
			|  command. For preview save mode "first", set flag so mjpeg
			|  callback can immediately schedule the on_preview_save command
			|  so there is no wait to execute it.
			*/
			video_record_start(vcb, VCB_STATE_MOTION_RECORD_START);
			mf->do_preview_save = TRUE;
			mf->best_motion_vector = mf->best_frame_vector;
			if (!strcmp(pikrellcam.motion_preview_save_mode, "first"))
				mf->do_preview_save_cmd = TRUE;
			}
		else if (vcb->state == VCB_STATE_MOTION_RECORD)
			{
			/* Already recording, so each motion trigger bumps up the record
			|  time to now + post capture time.
			|  If mode "best" and better composite vector, save a new preview.
			*/
			vcb->motion_sync_time = pikrellcam.t_now + pikrellcam.motion_times.post_capture;
			if (   !strcmp(pikrellcam.motion_preview_save_mode, "best")
			    && composite_vector_best(&mf->best_frame_vector, &mf->best_motion_vector)
			   )
				{
				mf->best_motion_vector = mf->best_frame_vector;
				mf->do_preview_save = TRUE;
				/* on_preview_save_cmd for save mode "best" is handled
				|  in video_record_stop().
				*/
				}
			}
		}
	}


  /* ================ Motion Region Config =============== */

static char *
regions_custom_config(char *custom)
	{
	char	*home_dir, *fname, *path;

	if (*custom && strcmp(custom, "default"))
		asprintf(&fname, PIKRELLCAM_MOTION_REGIONS_CUSTOM_CONFIG, custom);
	else
		asprintf(&fname, PIKRELLCAM_MOTION_REGIONS_CONFIG);

	home_dir = getpwuid(geteuid())->pw_dir;
	asprintf(&path, "%s/%s/%s", home_dir, PIKRELLCAM_CONFIG_DIR, fname);
	free(fname);

	if (pikrellcam.verbose)
		printf("regions config file: %s\n", path);
	return path;
	}

static boolean
atof_range(float *result, char *value, double low, double high)
	{
	double	tmp;

	tmp = atof(value);
	if (tmp >= low && tmp <= high)
		{
		*result = tmp;
		return TRUE;
		}
	return FALSE;
	}

#define	ADD_REGION		0
#define MOVE_REGION		1
#define MOVE_COARSE		2
#define MOVE_FINE		3
#define ASSIGN_REGION	4
#define	SAVE_REGIONS	5
#define LOAD_REGIONS	6
#define DELETE_REGIONS	7
#define	SET_LIMITS		8
#define	SELECT_REGION	9
#define	SHOW_REGIONS	10
#define	SHOW_VECTORS	11


typedef struct
	{
	char	*name;
	int		id,
			n_args;
	}
	MotionCommand;

static MotionCommand motion_commands[] =
	{
	{ "show_regions", SHOW_REGIONS,    1 },
	{ "show_vectors", SHOW_VECTORS,    1 },
	{ "add_region",    ADD_REGION,    4 },
	{ "move_region",   MOVE_REGION,   5 },
	{ "move_coarse",   MOVE_COARSE, 2 },
	{ "move_fine",     MOVE_FINE,   2 },
	{ "assign_region", ASSIGN_REGION, 5 },
	{ "save_regions",   SAVE_REGIONS,  1 },
	{ "load_regions",   LOAD_REGIONS,  1 },
	{ "delete_regions", DELETE_REGIONS, 1 },
	{ "select_region", SELECT_REGION,    1 },
	{ "limits", SET_LIMITS,    2 }
	};

#define N_MOTION_COMMANDS	(sizeof(motion_commands) / sizeof(MotionCommand))

static void
motion_region_fixup(MotionRegion *mreg)
	{
	float	delta;

	if (mreg->xf0 < 0.0)
		mreg->xf0 = 0.0;
	if (mreg->yf0 < 0.0)
		mreg->yf0 = 0.0;

	if (motion_frame.width > 0)
		{
		delta = 2.0 / (float) motion_frame.width;
		if (mreg->xf0 > 1.0 - delta)
			mreg->xf0 = 1.0 - delta;
		if (mreg->dxf < delta)
			mreg->dxf = delta;
		delta = 2.0 / (float) motion_frame.height;
		if (mreg->yf0 > 1.0 - delta)
			mreg->yf0 = 1.0 - delta;
		if (mreg->dyf < delta)
			mreg->dyf = delta;
		}

	if (mreg->xf0 + mreg->dxf > 1.0)
		mreg->dxf = 1.0 - mreg->xf0;
	if (mreg->yf0 + mreg->dyf > 1.0)
		mreg->dyf = 1.0 - mreg->yf0;

	mreg->x  = motion_frame.width  * mreg->xf0;
	mreg->y  = motion_frame.height * mreg->yf0;
	mreg->dx = motion_frame.width  * mreg->dxf;
	mreg->dy = motion_frame.height * mreg->dyf;
	}

static boolean
get_motion_args(MotionRegion *mreg, char *xs, char *ys, char *dxs, char *dys,
				double low, double high)
	{
	if (   atof_range(&mreg->xf0, xs, low, high)
	    && atof_range(&mreg->yf0, ys, low, high)
	    && atof_range(&mreg->dxf, dxs, low, high)
	    && atof_range(&mreg->dyf, dys, low, high)
	   )
		return TRUE;
	return FALSE;
	}

void
motion_command(char *cmd_line)
	{
	MotionCommand	*mcmd;
	MotionFrame		*mf = &motion_frame;
	MotionRegion	*mreg, mrtmp;
	SList			*list;
	char			cmd[32], arg1[32], arg2[32], arg3[32], arg4[32], arg5[32];
	char			*path;
	int				i, n, id = -1;
	float			delta = 0.0;

	n = sscanf(cmd_line, "%31s %31s %31s %31s %31s %31s",
					cmd, arg1, arg2, arg3, arg4, arg5);
	if (n < 1 || cmd[0] == '#')
		return;

	for (i = 0; i < N_MOTION_COMMANDS; ++i)
		{
		mcmd = &motion_commands[i];
		if (!strcmp(mcmd->name, cmd))
			{
			if (mcmd->n_args == n - 1)
				id = mcmd->id;;
			break;
			}
		}
	if (id == -1)
		{
//		inform_message("Bad motion command.");
		return;
		}

	switch (id)
		{
		case SHOW_REGIONS:
			config_set_boolean(&mf->show_regions, arg1);
			break;
		case SHOW_VECTORS:
			config_set_boolean(&mf->show_vectors, arg1);
			break;
		case ADD_REGION:		/* add_region x y dx dy */
			if (get_motion_args(&mrtmp, arg1, arg2, arg3, arg4, 0.00, 1.0))
				{
				pthread_mutex_lock(&mf->region_list_mutex);
				mreg = calloc(1, sizeof(MotionRegion));
				*mreg = mrtmp;
				motion_region_fixup(mreg);
				mf->motion_region_list =
							slist_append(mf->motion_region_list, mreg);
				mf->n_regions = slist_length(mf->motion_region_list);
				mreg->region_number = mf->n_regions - 1;
				mf->selected_region = mreg->region_number;
				mf->show_regions = TRUE;
				pthread_mutex_unlock(&mf->region_list_mutex);
				}
			else
				;
			break;

		case SELECT_REGION:		/* select_region last / select_region < / select_region > (def) */
			if (mf->n_regions > 0)
				{
				if (!strcmp(arg1, "last"))
					mf->selected_region = mf->n_regions - 1;
				else if (mf->selected_region == -1)	/* short out < or > */
					mf->selected_region = mf->prev_selected_region;
				else
					{
					mf->selected_region += (arg1[0] == '<') ? -1 : 1;
					if (mf->selected_region < 0)
						mf->selected_region = mf->n_regions - 1;
					else if (mf->selected_region >= mf->n_regions)
						mf->selected_region = 0;
					}
				mf->prev_selected_region = mf->selected_region;
				mf->show_regions = TRUE;
				}
			break;

							/* move_fine {x,y,dx,dy} {+,p,-,m} */
		case MOVE_FINE:		/* Try to move one macro block */
			if (arg1[0] == 'x' || arg1[1] == 'x')
				delta = 1.0 / (float) motion_frame.width;
			else
				delta = 1.0 / (float) motion_frame.height;

		case MOVE_COARSE:	/* move_coarse {x,y,dx,dy} {+,p,-,m} */
			if ((n = mf->selected_region) >= 0)
				{
				pthread_mutex_lock(&mf->region_list_mutex);
				if (delta == 0.0)
					delta = 0.1;
				if (arg2[0] == '-' || arg2[0] == 'm')
					delta = -delta;
				if ((mreg = slist_nth_data(mf->motion_region_list, n)) != NULL)
					{
					if (!strcmp(arg1, "x"))
						mreg->xf0 += delta;
					if (!strcmp(arg1, "y"))
						mreg->yf0 += delta;
					if (!strcmp(arg1, "dx"))
						mreg->dxf += delta;
					if (!strcmp(arg1, "dy"))
						mreg->dyf += delta;
					motion_region_fixup(mreg);
					}
				pthread_mutex_unlock(&mf->region_list_mutex);
				}
			break;

		case MOVE_REGION:	/* 	move_region r  x y dx dy */
			pthread_mutex_lock(&mf->region_list_mutex);
			n = atoi(arg1);
			if (   (mreg = slist_nth_data(mf->motion_region_list, n)) != NULL
			    && get_motion_args(&mrtmp, arg2, arg3, arg4, arg5, -1.0, 1.0)
		       )
				{
				mreg->xf0 += mrtmp.xf0;
				mreg->yf0 += mrtmp.yf0;
				mreg->dxf += mrtmp.dxf;
				mreg->dyf += mrtmp.dyf;
				motion_region_fixup(mreg);
				}
			pthread_mutex_unlock(&mf->region_list_mutex);
			break;

		case ASSIGN_REGION:		/* assign_region r  x y dx dy */
			pthread_mutex_lock(&mf->region_list_mutex);
			n = atoi(arg1);
			if (   (mreg = slist_nth_data(mf->motion_region_list, n)) != NULL
			    && get_motion_args(&mrtmp, arg2, arg3, arg4, arg5, 0.0, 1.0)
			   )
				{
				mreg->xf0 = mrtmp.xf0;
				mreg->yf0 = mrtmp.yf0;
				mreg->dxf = mrtmp.dxf;
				mreg->dyf = mrtmp.dyf;
				motion_region_fixup(mreg);
				}
			pthread_mutex_unlock(&mf->region_list_mutex);
			break;

		case SAVE_REGIONS:		/* save_regions config-name */
			path = regions_custom_config(arg1);
			motion_regions_config_save(path);
			free(path);
			break;

		case LOAD_REGIONS:		/* load_regions config-name */
			path = regions_custom_config(arg1);
			motion_regions_config_load(path);
			free(path);
			break;

		case DELETE_REGIONS:		/* delete_regions all / delete r */
			pthread_mutex_lock(&mf->region_list_mutex);
			if (!strcmp(arg1, "all"))
				{
				slist_and_data_free(mf->motion_region_list);
				mf->motion_region_list = NULL;
				}
			else if (   (   !strcmp(arg1, "selected")
			             && mf->selected_region >= 0
			             && (mreg = slist_nth_data(mf->motion_region_list,
								mf->selected_region)) != NULL
			            )
			         || (   isdigit(arg1[0])
			             && (mreg = slist_nth_data(mf->motion_region_list,
									atoi(arg1))) != NULL
			            )
			        )
				{
				mf->motion_region_list =
						slist_remove(mf->motion_region_list, mreg);
				free(mreg);
				}
			for (i = 0, list = mf->motion_region_list; list; list = list->next)
				{
				mreg = (MotionRegion *) list->data;
				mreg->region_number = i++;
				}
			mf->n_regions = i;
			if (mf->selected_region >= mf->n_regions - 1)
				{
				mf->selected_region = mf->n_regions - 1;
				mf->prev_selected_region = 0;
				}
			pthread_mutex_unlock(&mf->region_list_mutex);
			break;

		case SET_LIMITS:		/* limits magnitude count */
			n = atoi(arg1);
			if (n < 3 || n > 120)
				n = 7;
			pikrellcam.motion_magnitude_limit = n;

			n = atoi(arg2);
			if (n < 3 || n > mf->width * mf->height / 2)
				n = 5;
			pikrellcam.motion_magnitude_limit_count = n;
			break;
		}
	}

void
motion_init(void)
	{
	MotionRegion	*mreg;
	SList			*list;
	static boolean	done_once = FALSE;

	if (!done_once)
		{
		done_once = TRUE;
		motion_frame.motion_enable = pikrellcam.motion_enable;
		}

	/* motion frames are from 16x16 macroblocks of the video frame
	*/
	motion_frame.width = (pikrellcam.camera_config.video_width / 16) + 1;
	motion_frame.height = (pikrellcam.camera_config.video_height / 16) + 1;
	motion_frame.vectors_size =
			motion_frame.width * motion_frame.height * sizeof(MotionVector);

	for (list = motion_frame.motion_region_list; list; list = list->next)
		{
		mreg = (MotionRegion *) list->data;
		mreg->x  = motion_frame.width * mreg->xf0;
		mreg->y  = motion_frame.height * mreg->yf0;
		mreg->dx = motion_frame.width * mreg->dxf;
		mreg->dy = motion_frame.height * mreg->dyf;
		}

	if (motion_frame.vectors)
		free(motion_frame.vectors);
	motion_frame.vectors = malloc(motion_frame.vectors_size);

	if (motion_frame.trigger)
		free(motion_frame.trigger);
	motion_frame.trigger = malloc(MF_TRIGGER_SIZE);
	motion_frame.motion_status = 0;
	}

void
motion_regions_config_save(char *config_file)
	{
	FILE			*f;
	SList			*list;
	MotionRegion	*mreg;

	if (   !config_file
	    || (f = fopen(config_file, "w")) == NULL
	   )
		{
		log_printf("Failed to save regions config file %s. %m\n", config_file);
		return;
		}

	for (list = motion_frame.motion_region_list; list; list = list->next)
		{
		mreg = (MotionRegion *) list->data;
		fprintf(f, "add_region %.3f %.3f %.3f %.3f\n",
				mreg->xf0, mreg->yf0, mreg->dxf, mreg->dyf);
		}
	fclose(f);
	}

boolean
motion_regions_config_load(char *config_file)
	{
	boolean	save_show;
	FILE	*f;
	char	buf[128];

	if (   !config_file
	    || (f = fopen(config_file, "r")) == NULL
	   )
		return FALSE;

	save_show = motion_frame.show_regions;

	motion_command("delete_regions all");
	while (fgets(buf, sizeof(buf), f) != NULL)
		motion_command(buf);
	fclose(f);

	motion_frame.show_regions = save_show;		/* can't double send_cmd()??*/
	return TRUE;
	}
