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
#include <dirent.h>

#define EXPMA_SMOOTHING	0.01

#define SMALL_OBJECT_COUNT  15
#define IN_BOX_COUNT_MIN    (2 * cvec->mag2_count)


MotionFrame		motion_frame;

static CompositeVector	zero_cvec;


static boolean
composite_vector_better(CompositeVector *cvec1, CompositeVector *cvec2)
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
	CompositeVector *cvec, tvec;
	MotionVector    *mv;
	Area            *area;
	char            *mt, *mtb;
	int             x, y, mag2, mb_index, mvmag2, dot, cos2,
	                x0, y0, x1, y1;
	int16_t         *pm, *pp, *pn;

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
	if ((y1 = mreg->y + mreg->dy) >= mf->height)
		y1 = mf->height - 1;
	if ((x0 = mreg->x) == 0)
		x0 = 1;
	if ((x1 = mreg->x + mreg->dx) >= mf->width)
		x1 = mf->width - 1;

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
				mf->any_count += 1;
				}
			}
		}

	/* If sparkle noise, override configured limit_count (for dusk/dawn times).
	|  In regions with no motion, this reduces chances of a spurious reject.
	|  In regions with motion, this tries to raise limit_count above the noise
	|  background, but sensitivity is reduced.
	*/
	x = SMALL_OBJECT_COUNT;
	if (mf->mag2_limit_count < x)
		{
		mf->mag2_limit_count += 2 * mreg->sparkle_count / 3;
		if (mf->mag2_limit_count > x)
			mf->mag2_limit_count = x;
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
	if (tvec.mag2_count >= mf->mag2_limit_count)
		{
		tvec.vx /= tvec.mag2_count;
		tvec.vy /= tvec.mag2_count;
		tvec.mag2 = tvec.vx * tvec.vx + tvec.vy * tvec.vy;

		for (y = y0; y < y1; ++y)
			{
			for (x = x0; x < x1; ++x)
				{
				mb_index = mf->width * y + x;
				mvmag2 = *(mf->trigger + mb_index);
				if (mvmag2 <= 1)
					continue;

				mv = &mf->vectors[mb_index];
				dot = tvec.vx * mv->vx + tvec.vy * mv->vy;
				if (   dot > 0		/* angle at least < 90 deg */
				    && tvec.mag2 > 0
					&& (cos2 = 100 * dot * dot / (tvec.mag2 * mvmag2)) >= 82
				   )
					{
					cvec->mag2_count += 1;
					cvec->vx += mv->vx;
					cvec->vy += mv->vy;
					cvec->x  += x;
					cvec->y  += y;

					area = &mf->motion_area;
					if (area->x0 == 0 || area->x0 > x)
						area->x0 = x;
					if (area->x1 < x)
						area->x1 = x;
					if (area->y0 == 0 || area->y0 > y)
						area->y0 = y;
					if (area->y1 < y)
						area->y1 = y;
					}
				else
					{
					*(mf->trigger + mf->width * y + x) = 2;	/* reject flag */
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
	mreg->motion = MOTION_NONE;
	if (cvec->mag2_count >= mf->mag2_limit_count)
		{
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

		/* Get a box that will hold at least 2x mag2_count vectors
		|  and count the vectors within the box.
		*/
		for (cvec->box_w = 4, cvec->box_h = 4;
					cvec->box_w * cvec->box_h <= IN_BOX_COUNT_MIN; )
			{
			if (cvec->box_h <= cvec->box_w)
				cvec->box_h += 2;
			else
				cvec->box_w += 2;
			}

		cvec->in_box_count = 0;
		cvec->in_box_rejects = 0;
		for (y = cvec->y - cvec->box_h / 2; y <= cvec->y + cvec->box_h / 2; ++y)
			{
			for (x = cvec->x - cvec->box_w / 2; x <= cvec->x + cvec->box_w / 2; ++x)
				{
				int  trig = *(mf->trigger + mf->width * y + x);

				if (trig > 2)
					cvec->in_box_count += 1;
				else if (trig == 2)
					cvec->in_box_rejects += 1;
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
			if (cvec->mag2_count < SMALL_OBJECT_COUNT)
				{
				if (   cvec->in_box_count > cvec->mag2_count * 8 / 10
				    && cvec->mag2 < 5 * mf->mag2_limit
				    && mreg->reject_count < cvec->mag2_count / 2
				   )
					mreg->motion = MOTION_TYPE_DIR_SMALL;
				}
			else
				if (   cvec->in_box_count > cvec->mag2_count * 7 / 10
			        || (   cvec->in_box_count > cvec->mag2_count * 5 / 10
			            && mreg->reject_count < cvec->mag2_count * 4 / 10
			           )
			       )
				mreg->motion = MOTION_TYPE_DIR_NORMAL;
			}
		/* Flag it if a density of count+rejects is sufficient for a burst
		|  detect (which still has to pass a total count check).
		*/
		if (mf->any_count_expma < 100.0)
			{
			if (cvec->in_box_count + cvec->in_box_rejects > 4 * IN_BOX_COUNT_MIN / 10)
				mreg->motion |= MOTION_TYPE_BURST_DENSITY;
			}
		else if (mf->any_count_expma < 300.0)
			{
			if (cvec->in_box_count + cvec->in_box_rejects > 7 * IN_BOX_COUNT_MIN / 10)
				mreg->motion |= MOTION_TYPE_BURST_DENSITY;
			}
		else
			if (cvec->in_box_count + cvec->in_box_rejects > 9 * IN_BOX_COUNT_MIN / 10)
				mreg->motion |= MOTION_TYPE_BURST_DENSITY;

		if (pikrellcam.verbose_motion)
			{
			printf(
"cvec[%d]: x,y(%d,%d) dx,dy(%d,%d) mag2,count,lim(%d,%d,%d) rej,spkl,vert(%d,%d,%d)\n",
				mreg->region_number,
				cvec->x, cvec->y, cvec->vx, cvec->vy, cvec->mag2,
				cvec->mag2_count, mf->mag2_limit_count, mreg->reject_count,
				mreg->sparkle_count, cvec->vertical);
			mt = "----";
			mtb = "------";
			if (mreg->motion & MOTION_TYPE_DIR_SMALL)
				mt = "dirS";
			else if (mreg->motion & MOTION_TYPE_DIR_NORMAL)
				mt = "dirN";
			if (mreg->motion & MOTION_TYPE_BURST_DENSITY)
				mtb = "burstD";
			/* in_box density is what counts are compared to, not in_size
			*/
			printf(
"   box:%dx%d in_box[cnt:%d rej:%d] density[dir:%.1f burst:%.1f]  **motion[%s,%s]\n",
				cvec->box_w, cvec->box_h,
				cvec->in_box_count, cvec->in_box_rejects,
				(float) cvec->in_box_count / (float) cvec->mag2_count,
				(float) (cvec->in_box_count + cvec->in_box_rejects)
						/ (float) IN_BOX_COUNT_MIN,
				mt, mtb);
			}
		}
	else
		*cvec = zero_cvec;
	}

static void
motion_stats_write(VideoCircularBuffer *vcb, MotionFrame *mf)
	{
	CompositeVector *frame_vec = &mf->frame_vector;

	if (!vcb->motion_stats_file)
		return;
	if (vcb->motion_stats_do_header)
		fprintf(vcb->motion_stats_file,
			"time, x, y, dx, dy, magnitude, count\n"
	        "# width %d height %d\n",
	        mf->width, mf->height);
	vcb->motion_stats_do_header = FALSE;

	fprintf(vcb->motion_stats_file,
		"%6.3f, %3d, %3d, %3d, %3d, %3.0f, %4d\n",
		(float) vcb->frame_count / (float) pikrellcam.camera_adjust.video_fps,
		frame_vec->x, frame_vec->y, -frame_vec->vx, -frame_vec->vy,
		sqrt((float)frame_vec->mag2), frame_vec->mag2_count);
	}

void
motion_event_write(VideoCircularBuffer *vcb, MotionFrame *mf)
	{
	static FILE		*f;
	CompositeVector *cvec, *frame_vec = &mf->frame_vector;
	MotionRegion	*mreg;
	PresetPosition	*pos;
	PresetSettings	*settings = NULL;
	SList			*mrlist;
	int				burst, i, pan, tilt;
	boolean			dir_motion;

	if (vcb->state == VCB_STATE_MOTION_RECORD_START)
		{
		f = fopen(pikrellcam.motion_events_filename, "w");
		fprintf(f, "<header>\n");
		fprintf(f, "video %s\n", pikrellcam.video_pathname);
		fprintf(f, "frame %d %d\n", mf->width, mf->height);
		servo_get_position(&pan, &tilt);
		pos = preset_find_at_position(pan, tilt);
		if (pos)
			{
			fprintf(f, "preset %d %d\n", pikrellcam.preset_position_index + 1,
			        pos ? pos->settings_index + 1 : 1);
			settings = (PresetSettings *)slist_nth_data(pos->settings_list,
							pos->settings_index);
			if (settings)
				{
				fprintf(f, "magnitude_limit %d\n", settings->mag_limit);
				fprintf(f, "magnitude_count %d\n", settings->mag_limit_count);
				fprintf(f, "burst_count %d\n", settings->burst_count);
				fprintf(f, "burst_frames %d\n", settings->burst_frames);
				}
			}
		if (pikrellcam.have_servos)
			{
			fprintf(f, "pan %d\n", pan);
			fprintf(f, "tilt %d\n", tilt);
			}
		fprintf(f, "</header>\n");
		}
	if (f && (   vcb->state == VCB_STATE_MOTION_RECORD
	          || vcb->state == VCB_STATE_MOTION_RECORD_START
	         )
	   )
		{
		burst = frame_vec->mag2_count + mf->reject_count;
		fprintf(f, "<motion %6.3f>\n",
			(float) vcb->frame_count / (float) pikrellcam.camera_adjust.video_fps);
		fprintf(f, "b %3d\n", (mf->motion_status & MOTION_BURST) ? burst : 0);
		fprintf(f, "f %3d %3d %3d %3d %3.0f %4d\n",
				frame_vec->x, frame_vec->y, -frame_vec->vx, -frame_vec->vy,
				sqrt((float)frame_vec->mag2), frame_vec->mag2_count);
		if (mf->motion_status & MOTION_DIRECTION)
			{
			for (i = 0, mrlist = mf->motion_region_list; mrlist;
						mrlist = mrlist->next, ++i)
				{
				mreg = (MotionRegion *) mrlist->data;
				dir_motion = mreg->motion
						& (MOTION_TYPE_DIR_SMALL | MOTION_TYPE_DIR_NORMAL);
				if (!dir_motion)
					continue;
				cvec = &mreg->vector;
				fprintf(f, "%d %3d %3d %3d %3d %3.0f %4d\n",
						i, cvec->x, cvec->y, -cvec->vx, -cvec->vy,
						sqrt((float)cvec->mag2), cvec->mag2_count);
				}
			}
		fprintf(f, "</motion>\n");
		fflush(f);
		}
	else if (f && (vcb->state == VCB_STATE_NONE))
		{
		fprintf(f, "<end>\n");
		fclose(f);
		f = NULL;
		}
	/* else a MANUAL record state */
	}

void
motion_frame_process(VideoCircularBuffer *vcb, MotionFrame *mf)
	{
	MotionRegion    *mreg;
	CompositeVector *cvec, *frame_vec;
	SList           *mrlist;
	int             motion_count, fail_count, direction_motion;
	boolean         burst_density_pass, motion_enabled;
	char            tbuf[50], *msg;
	int             x0, y0, x1, y1, t;
	static int      mfp_number, motion_burst_frame;

	/* Allow some startup camera settle time before motion detecting.
	*/
	if (pikrellcam.t_now < pikrellcam.t_start + 3)
		return;

	mf->motion_status = MOTION_NONE;

	mf->sparkle_count  = 0;
	mf->reject_count   = 0;
	mf->any_count      = 0;
	mf->vertical_count = 0;

	memset(mf->trigger, 0, MF_TRIGGER_SIZE);
	mf->motion_area.x0 = mf->motion_area.y0 = mf->motion_area.x1 = mf->motion_area.y1 = 0;
	mf->mag2_limit  = pikrellcam.motion_magnitude_limit * pikrellcam.motion_magnitude_limit;
	mf->mag2_limit_count = pikrellcam.motion_magnitude_limit_count;

	pthread_mutex_lock(&mf->region_list_mutex);
	for (mrlist = mf->motion_region_list; mrlist; mrlist = mrlist->next)
		{
		mreg = (MotionRegion *) mrlist->data;

		get_composite_vector(mf, mreg);

		/* Revert any dynamic mag2 adjust done in get_composite_vector().
		*/
		mf->mag2_limit  = pikrellcam.motion_magnitude_limit * pikrellcam.motion_magnitude_limit;
		mf->mag2_limit_count = pikrellcam.motion_magnitude_limit_count;
		}

	motion_count = 0;
	fail_count = 0;
	burst_density_pass = FALSE;
	mf->frame_vector = zero_cvec;
	frame_vec = &mf->frame_vector;
	mf->cvec_count = 0;

	for (mrlist = mf->motion_region_list; mrlist; mrlist = mrlist->next)
		{
		mreg = (MotionRegion *) mrlist->data;
		cvec = &mreg->vector;
		direction_motion = mreg->motion & (MOTION_TYPE_DIR_SMALL | MOTION_TYPE_DIR_NORMAL);

		if (   cvec->mag2_count > 0
		    && !direction_motion
		   )
			fail_count += 1;
		else if (direction_motion)
			motion_count += 1;

		if (mreg->motion & MOTION_TYPE_BURST_DENSITY)
			burst_density_pass = TRUE;

		if (cvec->mag2_count > 0)
			{
			frame_vec->x  += cvec->x;
			frame_vec->y  += cvec->y;
			frame_vec->vx += cvec->vx;
			frame_vec->vy += cvec->vy;
			frame_vec->mag2_count += cvec->mag2_count;
			mf->cvec_count += 1;
			}
		}

	/* The frame composite vector is the vector sum of all the region composite
	|  vectors and the box is formed from the max dx and dy of the region
	|  vector box extents from the frame vector center.  This vector box is
	|  likely smaller than the geometric box recorded in the motion_area values.
	*/
	if (mf->cvec_count > 0)
		{
		frame_vec->x /= mf->cvec_count;
		frame_vec->y /= mf->cvec_count;
		frame_vec->vx /= mf->cvec_count;
		frame_vec->vy /= mf->cvec_count;
		frame_vec->mag2 =
			frame_vec->vx * frame_vec->vx + frame_vec->vy * frame_vec->vy;

		x0 = y0 = x1 = y1 = 0;
		for (mrlist = mf->motion_region_list; mrlist; mrlist = mrlist->next)
			{
			mreg = (MotionRegion *) mrlist->data;
			cvec = &mreg->vector;
			if (cvec->mag2_count == 0)
				continue;

			t = cvec->x - cvec->box_w / 2;
			if (x0 == 0 || t < x0)
				x0 = t;
			t = cvec->x + cvec->box_w / 2;
			if (t > x1)
				x1 = t;
			t = cvec->y - cvec->box_h / 2;
			if (y0 == 0 || t < y0)
				y0 = t;
			t = cvec->y + cvec->box_h / 2;
			if (t > y1)
				y1 = t;
			}
		frame_vec->box_w = 2 * MAX(frame_vec->x - x0, x1 - frame_vec->x);
		frame_vec->box_h = 2 * MAX(frame_vec->y - y0, y1 - frame_vec->y);
		}
	pthread_mutex_unlock(&mf->region_list_mutex);

	/* To be used (large sparkle counts after sunset / before sunrise).
	*/
	mf->sparkle_expma = EXPMA_SMOOTHING * (float) mf->sparkle_count +
							(1.0 - EXPMA_SMOOTHING) * mf->sparkle_expma;

	mf->motion_status = MOTION_NONE;

	/* Burst motion triggers on counts above the any_count_expma
	|  background count noise.
	*/
	if (   burst_density_pass
	    && frame_vec->mag2_count + mf->reject_count >
				pikrellcam.motion_burst_count + (int) mf->any_count_expma
	   )
		{
		if (motion_burst_frame < pikrellcam.motion_burst_frames)
			++motion_burst_frame;
		mf->motion_status = MOTION_PENDING_BURST;
		}
	else
		{
		/* any_count_expma is background noise counts so exclude from it
		|  activity that caused any pass or fail cvecs.  It already
		|  excludes sparkles.
		*/
		if (motion_count == 0)
			mf->any_count_expma = 0.02 * (float) mf->any_count +
					(1.0 - 0.02) * mf->any_count_expma;
		if (motion_burst_frame > 0)
			--motion_burst_frame;
		}

	if (   motion_count > 0
	    && fail_count == 0
	   )
		{
		if (   vcb->state != VCB_STATE_MOTION_RECORD
		    && mf->frame_window == 0
		    && pikrellcam.motion_times.confirm_gap > 0
		   )
			{
			mf->frame_window = pikrellcam.camera_adjust.video_fps *
					pikrellcam.motion_times.confirm_gap / pikrellcam.mjpeg_divider;
			mf->motion_status |= MOTION_PENDING_DIR;
			}
		else
			mf->motion_status = (MOTION_DETECTED | MOTION_DIRECTION);
		}

	if (motion_burst_frame == pikrellcam.motion_burst_frames)
		{
		mf->motion_status &= ~(MOTION_PENDING_DIR | MOTION_PENDING_BURST);
		mf->motion_status |= (MOTION_DETECTED | MOTION_BURST);
		mf->frame_window = 0;
		}
	if (mf->external_trigger)
		{
		mf->external_trigger = FALSE;
		mf->motion_status &= ~(MOTION_PENDING_DIR | MOTION_PENDING_BURST);
		mf->motion_status |= (MOTION_DETECTED | MOTION_EXTERNAL);
		mf->frame_window = 0;
		}

	if (mf->frame_window > 0)
		mf->frame_window -= 1;

	if (   pikrellcam.verbose_motion
	    && frame_vec->mag2_count > 0
	   )
		{
		printf(
"frame:   x,y(%d,%d) dx,dy(%d,%d) mag2:%d count,rej(%d,%d) any[%d,%.1f] burst:%d\n",
			frame_vec->x, frame_vec->y, frame_vec->vx, frame_vec->vy,
			frame_vec->mag2, frame_vec->mag2_count, mf->reject_count,
			mf->any_count, mf->any_count_expma, motion_burst_frame);
		}
	if (pikrellcam.verbose_motion && (fail_count > 0 || motion_count > 0))
		{
		strftime(tbuf, sizeof(tbuf), "%T", &pikrellcam.tm_local);
		msg = "";
		if ((mf->motion_status & (MOTION_DIRECTION | MOTION_BURST))
				    == (MOTION_DIRECTION | MOTION_BURST))
			msg = "***MOTION BOTH***";
		else if (mf->motion_status & MOTION_DIRECTION)
			msg = "***MOTION DIRECTION***";
		else if (mf->motion_status & MOTION_BURST)
			msg = "***MOTION BURST***";
		else if (mf->motion_status & MOTION_EXTERNAL)
			msg = "***MOTION EXTERNAL***";
		else
			{
			if ((mf->motion_status & (MOTION_PENDING_DIR | MOTION_PENDING_BURST))
						== (MOTION_PENDING_DIR | MOTION_PENDING_BURST))
				msg = "***motion pending (dir|burst)***";
			else if (mf->motion_status & MOTION_PENDING_DIR)
				msg = "***motion pending (dir)***";
			else if (mf->motion_status & MOTION_PENDING_BURST)
				msg = "***motion pending (burst)***";
			}

		printf("%s [%d] regions[motion:%d fail:%d] spkl[%d,%.1f] window:%d  %s\n\n",
			tbuf, mfp_number,  motion_count, fail_count,
			mf->sparkle_count, mf->sparkle_expma,
			mf->frame_window, msg);
		}
	++mfp_number;
	if (motion_burst_frame == pikrellcam.motion_burst_frames)
		motion_burst_frame = 0;

	motion_enabled = (  (  mf->motion_enable
	                     || (mf->external_trigger_mode & EXT_TRIG_MODE_ENABLE)
	                    )
	                  && !pikrellcam.servo_moving
					  && (pikrellcam.on_preset || pikrellcam.motion_off_preset)
	                 );

	if (   (   (mf->motion_status & MOTION_DETECTED)
	        && motion_enabled
	        && (mf->external_trigger_mode & EXT_TRIG_MODE_TIMES) == 0
	       )
	    || ( (mf->motion_status & MOTION_EXTERNAL) && motion_enabled)
	   )
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
			mf->preview_frame_vector = mf->frame_vector;
			if (mf->motion_status & MOTION_EXTERNAL)
				{
				mf->preview_motion_area.x0 = mf->preview_motion_area.y0 = 0;
				mf->preview_motion_area.x1 = mf->width - 1;
				mf->preview_motion_area.y1 = mf->height - 1;
				}
			else
				mf->preview_motion_area = mf->motion_area;
			pikrellcam.video_notify = FALSE;
			pikrellcam.still_notify = FALSE;
			pikrellcam.timelapse_notify = FALSE;

			if (!strcmp(pikrellcam.motion_preview_save_mode, "first"))
				{
				motion_preview_area_fixup();
				mf->do_preview_save_cmd = TRUE;
				}
			mf->first_detect = mf->motion_status;
			mf->burst_detects = 0;
			mf->direction_detects = 0;
			mf->max_burst_count = 0;
			mf->first_burst_count = 0;
			if (mf->motion_status & MOTION_DIRECTION)
				{
				mf->direction_detects = 1;
				}
			if (mf->motion_status & MOTION_BURST)
				{
				mf->burst_detects = 1;
				mf->first_burst_count = frame_vec->mag2_count;
				mf->max_burst_count = frame_vec->mag2_count + mf->reject_count;
				}

			if (pikrellcam.verbose_motion && !pikrellcam.verbose)
				printf("***Motion record start: %s\n\n", pikrellcam.video_pathname);
			}
		else if (vcb->state == VCB_STATE_MOTION_RECORD)
			{
			/* Already recording, so each motion trigger bumps up the record
			|  time to now + post capture time.
			|  If mode "best" and better composite vector, save a new preview.
			*/
			vcb->motion_sync_time = pikrellcam.t_now + pikrellcam.motion_times.post_capture;

			if (   !(mf->motion_status & MOTION_EXTERNAL)
			    && !strcmp(pikrellcam.motion_preview_save_mode, "best")
			    && composite_vector_better(&mf->frame_vector, &mf->preview_frame_vector)
			   )
				{
				mf->preview_frame_vector = mf->frame_vector;
				mf->preview_motion_area = mf->motion_area;
				mf->do_preview_save = TRUE;
				/* on_preview_save_cmd for save mode "best" is handled
				|  in video_record_stop().
				*/
				}
			if (mf->motion_status & MOTION_DIRECTION)
				++mf->direction_detects;
			if (mf->motion_status & MOTION_BURST)
				{
				++mf->burst_detects;
				if (mf->max_burst_count < frame_vec->mag2_count + mf->reject_count)
					mf->max_burst_count = frame_vec->mag2_count + mf->reject_count;
				}
			motion_event_write(vcb, mf);

			if (pikrellcam.verbose_motion)
				printf("==>Motion record bump: %s\n\n", pikrellcam.video_pathname);
			}
		if (pikrellcam.motion_stats)
			motion_stats_write(vcb, mf);

		}
	}


void
motion_preview_area_fixup(void)
	{
	CompositeVector *frame_vec;
	Area            *area = &motion_frame.preview_motion_area;
	int             sign, d, dx, dy;

	motion_frame.final_preview_vector = motion_frame.preview_frame_vector;
	frame_vec = &motion_frame.final_preview_vector;

	/* frame_vec is built from boxes for vector density counts which do not
	|  try to accurately frame motion extents, but compared to motion_area
	|  it is more likely to not include spurious vectors outside of motion
	|  of interest.  But can get better framing by adjusting the frame_vec
	|  somewhat based on the motion_area.
	*/
	area->x0 = MOTION_VECTOR_TO_MJPEG_X(area->x0);
	area->y0 = MOTION_VECTOR_TO_MJPEG_Y(area->y0);
	area->x1 = MOTION_VECTOR_TO_MJPEG_X(area->x1);
	area->y1 = MOTION_VECTOR_TO_MJPEG_Y(area->y1);
	frame_vec->x     = MOTION_VECTOR_TO_MJPEG_X(frame_vec->x);
	frame_vec->y     = MOTION_VECTOR_TO_MJPEG_Y(frame_vec->y);
	frame_vec->box_w = MOTION_VECTOR_TO_MJPEG_X(frame_vec->box_w);
	frame_vec->box_h = MOTION_VECTOR_TO_MJPEG_Y(frame_vec->box_h);
	dx = frame_vec->box_w;
	dy = frame_vec->box_h;

	d = frame_vec->x + dx / 2;
	if (d < area->x1)
		dx += (area->x1 - d) / 3;
	d = frame_vec->x - dx / 2;
	if (d > area->x0)
		dx += (d - area->x1) / 3;
	d = frame_vec->y - dy / 2;
	if (d > area->y0)
		dy += (d - area->y0);
	d = frame_vec->y + dy / 2;
	if (d < area->y1)
		dy += (area->y1 - d);
	d = pikrellcam.motion_area_min_side;
	if (dx < d)
		dx = d;
	if (dy < d)
		dy = d;
	frame_vec->box_w = dx;
	frame_vec->box_h = dy;

	/* Adjust for camera video path skew so small faster objects have a chance
	|  of being framed.
	*/
	sign = frame_vec->vx >= 0 ? -1 : 1;
	d = abs(frame_vec->vx) - 5;
	if (d > 0)
		frame_vec->x += sign * 8 * d / 10;
	if (frame_vec->x + dx / 2 >= pikrellcam.mjpeg_width)
		frame_vec->x = pikrellcam.mjpeg_width - dx / 2 - 1;
	if (frame_vec->x - dx / 2 < 0)
		frame_vec->x = dx / 2 + 1;

	sign = frame_vec->vy >= 0 ? -1 : 1;
	d = abs(frame_vec->vy) - 5;
	if (d > 0)
		frame_vec->y += sign * 8 * d / 10;
	if (frame_vec->y + dy / 2 >= pikrellcam.mjpeg_height)
		frame_vec->y = pikrellcam.mjpeg_height - dy / 2 - 1;
	if (frame_vec->y - dy / 2 < 0)
		frame_vec->y = dy / 2 + 1;
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

#define NEW_REGION        0
#define ADD_REGION        1
#define MOVE_REGION       2
#define MOVE_COARSE       3
#define MOVE_FINE         4
#define ASSIGN_REGION     5
#define SAVE_REGIONS      6
#define LOAD_REGIONS      7
#define LOAD_REGIONS_SHOW 8
#define LIST_REGIONS      9
#define DELETE_REGIONS   10
#define SET_LIMITS       11
#define SET_BURST        12
#define SELECT_REGION    13
#define SHOW_PRESET      14
#define SHOW_VECTORS     15
#define TRIGGER          16
#define PRE_CAPTURE      17
#define CONFIRM_GAP      18
#define EVENT_GAP        19
#define POST_CAPTURE     20

typedef struct
	{
	char	*name;
	int		id,
			n_args;
	}
	MotionCommand;

static MotionCommand motion_commands[] =
	{
	{ "show_preset",  SHOW_PRESET,    1 },
	{ "show_regions", SHOW_PRESET,    1 },
	{ "show_vectors", SHOW_VECTORS,    1 },
	{ "new_region",    NEW_REGION,    4 },
	{ "add_region",    ADD_REGION,    4 },		// Not a regions modify
	{ "move_region",   MOVE_REGION,   5 },
	{ "move_coarse",   MOVE_COARSE, 2 },
	{ "move_fine",     MOVE_FINE,   2 },
	{ "assign_region", ASSIGN_REGION, 5 },
	{ "save_regions",   SAVE_REGIONS,  1 },
	{ "load_regions",   LOAD_REGIONS,  1 },
	{ "load_regions_show", LOAD_REGIONS_SHOW,  1 },
	{ "list_regions",   LIST_REGIONS,  0 },
	{ "delete_regions", DELETE_REGIONS, 1 },	// All not a regions modify
	{ "select_region", SELECT_REGION,    1 },
	{ "limits", SET_LIMITS,    2 },
	{ "burst", SET_BURST,    2 },
	{ "trigger", TRIGGER,    0 },
	{ "pre_capture", PRE_CAPTURE,    1 },
	{ "confirm_gap", CONFIRM_GAP,    1 },
	{ "event_gap", EVENT_GAP,    1 },
	{ "post_capture", POST_CAPTURE,    1 }
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

static char *
motion_regions_name(char *name)
	{
	char        *s, *base = fname_base(name);
	static char reg_name[50];

	if (strncmp(base, "motion-regions-", 15) == 0)
		stpncpy(reg_name, base + 15, 49);
	else
		strcpy(reg_name, "default");

	s = strrchr(reg_name, '.');
	if (s)
		*s = '\0';
	return reg_name;
	}

void
motion_command(char *cmd_line)
	{
	MotionCommand *mcmd;
	MotionFrame   *mf = &motion_frame;
	MotionRegion  *mreg, mrtmp;
	VideoCircularBuffer *vcb = &video_circular_buffer;
	SList         *list;
	char          buf[64], arg1[32], arg2[32], arg3[32], arg4[32], arg5[32];
	char          *path, *reg_name;
	int           i, n, id = -1;
	float         delta = 0.0;
	struct dirent *dp;
	DIR           *dfd;
	boolean       new_region = FALSE;

	arg1[0] = '\0';
	arg2[0] = '\0';
	arg3[0] = '\0';
	n = sscanf(cmd_line, "%63s %31s %31s %31s %31s %31s",
					buf, arg1, arg2, arg3, arg4, arg5);
	if (n < 1 || buf[0] == '#')
		return;

	for (i = 0; i < N_MOTION_COMMANDS; ++i)
		{
		mcmd = &motion_commands[i];
		if (!strcmp(mcmd->name, buf))
			{
			if (mcmd->n_args <= n - 1)
				id = mcmd->id;
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
		case SHOW_PRESET:
			config_set_boolean(&mf->show_preset, arg1);
			pikrellcam.state_modified = TRUE;
			break;
		case SHOW_VECTORS:
			config_set_boolean(&mf->show_vectors, arg1);
			pikrellcam.state_modified = TRUE;
			break;
		/* A new_region is an edit modify from the web page.
		|  An add_region is a load from a config file and is not a modify.
		*/
		case NEW_REGION:		/* new_region x y dx dy */
			new_region = TRUE;
		case ADD_REGION:		/* load_region x y dx dy */
			memset((char *) &mrtmp, 0, sizeof(MotionRegion));
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
				mf->show_preset = TRUE;
				pikrellcam.state_modified = TRUE;
				if (new_region)
					preset_regions_set_modified();
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
				mf->show_preset = TRUE;
				pikrellcam.state_modified = TRUE;
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
				preset_regions_set_modified();
				pthread_mutex_unlock(&mf->region_list_mutex);
				}
			else
				{
				display_inform("\"Select a region first.\" 3 3 1");
				display_inform("timeout 1");
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
			preset_regions_set_modified();
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
			preset_regions_set_modified();
			pthread_mutex_unlock(&mf->region_list_mutex);
			break;

		case SAVE_REGIONS:		/* save_regions config-name */
			path = regions_custom_config(arg1);
			motion_regions_config_save(path, TRUE);
			free(path);
			break;

		case LOAD_REGIONS_SHOW:		/* load_regions and show */
			mf->show_preset = TRUE;
			pikrellcam.state_modified = TRUE;
		case LOAD_REGIONS:			/* load_regions config-name */
			path = regions_custom_config(arg1);
			if (motion_regions_config_load(path, TRUE))
				preset_regions_set_modified();
			free(path);
			break;

		case LIST_REGIONS:
			if ((dfd = opendir(pikrellcam.config_dir)) != NULL)
				{
				i = 2;
				n = 10;
				display_inform("\"Motion regions list:\" 1 3 1");
				while ((dp = readdir(dfd)) != NULL)
					{
					if (strncmp(dp->d_name, "motion-regions", 14) != 0)
						continue;
					reg_name = motion_regions_name(dp->d_name);
					snprintf(buf, sizeof(buf), "\"%s\" %d 4 1 %d",
						reg_name, i++, n);
					display_inform(buf);
					if (i == 10)
						{
						n += pikrellcam.mjpeg_width / 3;
						i = 2;
						}
					}
				display_inform("timeout 4");
				closedir(dfd);
				}
			break;
		/* An all delete is a clear prior to a load from a config file and
		|  is not a modify.  A delete of a region number is a delete from the
		|  from the web page and is an edit modify of the regions.
		*/
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
				preset_regions_set_modified();
				}
			else
				{
				display_inform("\"Select a region first.\" 3 3 1");
				display_inform("timeout 1");
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
			/* Allow manual override settings higher than gui allows */
			n = atoi(arg1);
			if (n < 3)
				n = 3;
			if (n > 120)
				n = 120;
			pikrellcam.motion_magnitude_limit = n;

			n = atoi(arg2);
			if (n < 2)
				n = 2;
			if (n > mf->width * mf->height / 2)
				n = n > mf->width * mf->height / 2;
			pikrellcam.motion_magnitude_limit_count = n;

			preset_settings_set_modified();
			break;

		case SET_BURST:			/* burst count frames */
			n = atoi(arg1);
			if (n < 20)
				n = 20;
			if (n > mf->width * mf->height / 2)
				n = n > mf->width * mf->height / 2;
			pikrellcam.motion_burst_count = n;

			n = atoi(arg2);
			if (n < 2)
				n = 2;
			if (n > 20)
				n = 20;
			pikrellcam.motion_burst_frames = n;

			preset_settings_set_modified();
			break;

		case TRIGGER:
			mf->external_trigger = TRUE;
			mf->external_trigger_mode = EXT_TRIG_MODE_DEFAULT;
			mf->external_trigger_pre_capture = 0;
			mf->external_trigger_time_limit = 0;
			if (arg1[0] && atoi(arg1) > 0)
				mf->external_trigger_mode = EXT_TRIG_MODE_ENABLE;
			if (arg2[0])
				{
				mf->external_trigger_mode |= EXT_TRIG_MODE_TIMES;
				mf->external_trigger_pre_capture = atoi(arg2);
				if (arg3[0])
					mf->external_trigger_time_limit = atoi(arg3);
				else
					mf->external_trigger_time_limit = pikrellcam.motion_times.post_capture;
				}
			break;

		case CONFIRM_GAP:
			n = atoi(arg1);
			pikrellcam.motion_times.confirm_gap = n;
			motion_times_temp.confirm_gap = n;
			pikrellcam.config_modified = TRUE;
			log_printf("command process: motion %s\n", cmd_line);
			break;

		case PRE_CAPTURE:
			pthread_mutex_lock(&vcb->mutex);
			if (vcb->state == VCB_STATE_NONE)
				{
				n = atoi(arg1);
				pikrellcam.motion_times.pre_capture = n;
				motion_times_temp.pre_capture = n;
				video_circular_buffer_init();
				audio_circular_buffer_init();
				pikrellcam.config_modified = TRUE;
				}
			else
				{
				display_inform("\"Cannot change pre_capture or event_gap\" 3 3 1");
				display_inform("\"while video is recording.\" 4 3 1");
				display_inform("timeout 2");
				}
			pthread_mutex_unlock(&vcb->mutex);
			log_printf("command process: motion %s\n", cmd_line);
			break;

		case EVENT_GAP:
			pthread_mutex_lock(&vcb->mutex);
			if (vcb->state == VCB_STATE_NONE)
				{
				n = atoi(arg1);
				pikrellcam.motion_times.event_gap = n;
				motion_times_temp.event_gap = n;
				video_circular_buffer_init();
				audio_circular_buffer_init();
				pikrellcam.config_modified = TRUE;
				}
			else
				{
				display_inform("\"Cannot change pre_capture or event_gap\" 3 3 1");
				display_inform("\"while video is recording.\" 4 3 1");
				display_inform("timeout 2");
				}
			pthread_mutex_unlock(&vcb->mutex);
			log_printf("command process: motion %s\n", cmd_line);
			break;

		case POST_CAPTURE:
			n = atoi(arg1);
			pikrellcam.motion_times.post_capture = n;
			motion_times_temp.post_capture = n;
			pikrellcam.config_modified = TRUE;
			log_printf("command process: motion %s\n", cmd_line);
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
	motion_frame.width = ((pikrellcam.camera_config.video_width + 15) / 16) + 1;
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
	motion_frame.motion_status = MOTION_NONE;
	}


void
motion_regions_config_save(char *config_file, boolean inform)
	{
	FILE         *f;
	SList        *list;
	MotionRegion *mreg;
	char         buf[128];

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

	if (inform)
		{
		snprintf(buf, sizeof(buf), "\"%s\" 4 3 1", motion_regions_name(config_file));
		display_inform("\"Saved motion regions to:\" 3 3 1");
		display_inform(buf);
		display_inform("timeout 1");
		}
	}

boolean
motion_regions_config_load(char *config_file, boolean inform)
	{
	boolean	save_show;
	FILE	*f;
	char	*reg_name, buf[128], dbuf[128];

	reg_name = motion_regions_name(config_file);
	snprintf(dbuf, sizeof(dbuf), "\"%s\" 4 3 1", reg_name);
	if (   !config_file
	    || (f = fopen(config_file, "r")) == NULL
	   )
		{
		if (inform)
			{
			display_inform("\"Cannot open motion regions name:\" 3 3 1");
			display_inform(dbuf);
			display_inform("timeout 2");
			}
		log_printf("Failed to open motion regions file: %s\n", config_file);
		return FALSE;
		}

	save_show = motion_frame.show_preset;

	motion_command("delete_regions all");
	while (fgets(buf, sizeof(buf), f) != NULL)
		motion_command(buf);
	fclose(f);

	dup_string(&pikrellcam.motion_regions_name, reg_name);
	if (inform)
		{
		display_inform("\"Loaded motion regions name:\" 3 3 1");
		display_inform(dbuf);
		display_inform("timeout 1");
		}
	motion_frame.show_preset = save_show;		/* can't double send_cmd()??*/
	return TRUE;
	}
