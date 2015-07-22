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
#include "glcd-widgets.h"

CameraAdjust	camera_adjust_temp;
MotionTimes		motion_times_temp;

#define	DISPLAY_DEFAULT			0
#define	DISPLAY_MENU			1
#define	DISPLAY_ADJUSTMENT		2
#define	DISPLAY_INFORM			3
#define	DISPLAY_QUIT			4


#define ACTION_NONE			0
#define ACTION_START		1
#define REPEAT_LEFT_ARROW	2
#define LEFT_ARROW			3
#define SELECT				4
#define RIGHT_ARROW			5
#define REPEAT_RIGHT_ARROW	6
#define BACK				8
#define PICTURE				9
#define VIDEO_PRESET		10
#define STILL_PRESET		11
#define CAMERA_ADJUSTMENTS	12
#define METERING			13
#define EXPOSURE			14
#define WHITE_BALANCE		15
#define IMAGE_EFFECT		16
#define MOTION_LIMIT		17
#define MOTION_SETTING		18
#define MOTION_TIME			19

typedef struct
	{
	char *name;
	int	 action;
	}
DisplayCommand;

static DisplayCommand display_commands[] =
	{
	{ "<<",		  REPEAT_LEFT_ARROW  },
	{ "<",		  LEFT_ARROW		  },
	{ "sel",	  SELECT			  },
	{ ">",		  RIGHT_ARROW	  },
	{ ">>",		  REPEAT_RIGHT_ARROW },
	{ "back",	  BACK			  },
	{ "motion_time",    MOTION_TIME  },
	{ "motion_limit",   MOTION_LIMIT  },
	{ "motion_setting", MOTION_SETTING },
	{ "video_presets",      VIDEO_PRESET		  },
	{ "still_presets",      STILL_PRESET	  },
	{ "camera_adjustments", CAMERA_ADJUSTMENTS  },
	{ "picture",  PICTURE		  },
	{ "metering", METERING		  },
	{ "exposure", EXPOSURE		  },
	{ "white_balance", WHITE_BALANCE	  },
	{ "image_effect", IMAGE_EFFECT	  }
	};

#define N_DISPLAY_COMMANDS    (sizeof(display_commands) / sizeof(DisplayCommand))

static int	display_state;
static int	display_menu;
static int	display_action;

static DrawArea	top_status_area,
				inform_area,
				adj_control_area,
				bottom_status_area;


  /* Draw on the I420 frame with my libglcd graphics library functions.
  |  They have a glcd namespace because I wrote the lib for a TFT LCD
  |  screen, but I have added a I420 framebuffer driver (no color yet)
  |  to it for pikrellcam.
  */
#define	JUSTIFY_LEFT			4
#define	JUSTIFY_CENTER			3
#define	JUSTIFY_LEFT_AT_X		2
#define	JUSTIFY_CENTER_AT_X		1

  /* If field is zero, right align, otherwise left align in a field width
  |  width.
  */
#define	JUSTIFY_RIGHT(field)	(-field)

static GlcdFont	*large_font = &font_12x24;
static GlcdFont	*normal_font = &font_9x15;

static DrawArea	*draw_area;
static Glcd		*glcd;

static void
i420_print(DrawArea *da, GlcdFont *font, int16_t color, int row,
				int xs, int ys, int justify, char *str)
	{
	int		x, y;

	if (row >= 0)
		y = ys + row * (font->char_height) + 1;
	else
		y = ys + da->height + row * (font->char_height) - 1;

	if (justify == JUSTIFY_LEFT)
		x = xs;
	else if (justify == JUSTIFY_CENTER)
		x = xs + (da->width - strlen(str) * font->char_width) / 2;
	else if (justify == JUSTIFY_LEFT_AT_X)
		x = xs - strlen(str) * font->char_width;
	else if (justify == JUSTIFY_CENTER_AT_X)
		x = xs - strlen(str) * font->char_width / 2;
	else
		/* It's justify right in a possible field width */
		{
		x = xs + da->width - 2;
		if (justify < 0)
			x -= -justify * font->char_width;
		else
			x -= strlen(str) * font->char_width;
		}

	/* Video frame can have large intensity variation, so draw shadow text
	|  and avoid black background.
	*/
	glcd_draw_string(glcd, da, font, 0, x + 1, y + 1, str);
	glcd_draw_string(glcd, da, font, color, x, y, str);
	}

static void
i420_draw_string(DrawArea *da, GlcdFont *font, int16_t color,
			int x, int y, char *string)
	{
	glcd_draw_string(glcd, da, font, 0, x + 1, y + 1, string);
	glcd_draw_string(glcd, da, font, color, x, y, string);
	}

static void
i420_dim_frame(uint8_t *i420)
	{
	MotionFrame	*mf = &motion_frame;
	int16_t		*ptrig;			/* ptr to motion frame trigger data */
	int			x, y, x_mv, y_mv;
	uint8_t		*pY,		/* ptr to I420 intensity (Y) data */
				Ydim, Ytrig;
//	uint8_t		*pU, *pV;
	
	for (y = 0; y < pikrellcam.mjpeg_height; ++y)
		{
		y_mv = MJPEG_TO_MOTION_VECTOR_Y(y);
		for (x = 0; x < pikrellcam.mjpeg_width; ++x)
			{
			x_mv = MJPEG_TO_MOTION_VECTOR_X(x);
			ptrig = mf->trigger + x_mv + mf->width * y_mv;

			pY = i420 + x + y * pikrellcam.mjpeg_width;

//			pV = pY + (((pkrellcam.mjpeg_height + 15) & ~15) * pikrellcam.mjpeg_width);
//			pV += (y / 2) * pikrellcam.mjpeg_width + x / 2;

//			pU = pY + (((((pikrellcam.mjpeg_height * 3) / 2) + 15) & ~15) * pikrellcam.mjpeg_width);
//			pU += (y / 2) * pikrellcam.mjpeg_width + x / 2;

			Ydim = *pY * pikrellcam.motion_vectors_dimming / 100;
			Ytrig = *pY;
			if (Ytrig < 245)
				Ytrig += 10;
			if (*ptrig > 2)			/* passing vector */
				*pY = Ytrig;
			else if (*ptrig == 2)	/* reject */
				*pY = Ydim + (Ytrig - Ydim) / 3;
			else if (*ptrig == 1)	/* sparkle */
				*pY = Ydim + 2 * (Ytrig - Ydim) / 3;
			else
				*pY = Ydim;
			}
		}
	}

static void
motion_draw(uint8_t *i420)
	{
	GlcdFont		*font;
	VideoCircularBuffer *vcb = &video_circular_buffer;
	DrawArea		*da;
	MotionFrame		*mf = &motion_frame;
	MotionRegion	*mreg;
	CompositeVector	*vec;
	SList 			*mrlist;
	char			*msg, info[100], status[100];
	int16_t			color;			/* just B&W */
	int				x, y, dx, dy, r_unit;
	int				t_record, t_hold;

	if (!glcd)
		return;
	da = draw_area;

	if (mf->show_vectors)
		{
		i420_dim_frame(i420);
		i420_print(&bottom_status_area, normal_font, 0xff, 0, 0, 0,
					JUSTIFY_RIGHT(0), "Vectors ON");
		}

	if (mf->show_regions)
		{
		for (mrlist = mf->motion_region_list; mrlist; mrlist = mrlist->next)
			{
			mreg = (MotionRegion *) mrlist->data;

			x  = MOTION_VECTOR_TO_MJPEG_X(mreg->x);
			y  = MOTION_VECTOR_TO_MJPEG_Y(mreg->y);
			dx = MOTION_VECTOR_TO_MJPEG_X(mreg->dx);
			dy = MOTION_VECTOR_TO_MJPEG_Y(mreg->dy);

			glcd_draw_rectangle(glcd, da, 0, x + 1, y + 1, dx, dy);
			glcd_draw_rectangle(glcd, da, 0xf0, x, y, dx, dy);

			if (mreg->region_number == mf->selected_region)
				{
				snprintf(info, sizeof(info), "[%d]", mreg->region_number);
				font = large_font;
				}
			else
				{
				snprintf(info, sizeof(info), "%d", mreg->region_number);
				font = normal_font;
				}
			i420_draw_string(draw_area, font, 0xff, x + 2, y + 1, info);

			if (mreg->vector.mag2_count >= mf->mag2_limit_count)
				{
				color = (mreg->motion > 0) ? 0xff : 0xb0;
				snprintf(info, sizeof(info), "cnt: %d",
						mreg->vector.mag2_count);
				i420_draw_string(draw_area, normal_font, color,
						x + 2, y + dy - normal_font->char_height - 1, info);

				snprintf(info, sizeof(info), "mag: %d",
						(int) sqrt(mreg->vector.mag2));
				i420_draw_string(draw_area, normal_font, color,
						x + 2, y + dy - 2 * normal_font->char_height - 1, info);

				vec = &mreg->vector;
				x  = MOTION_VECTOR_TO_MJPEG_X(vec->x - vec->box_w / 2);
				y  = MOTION_VECTOR_TO_MJPEG_Y(vec->y - vec->box_h / 2);
				dx = MOTION_VECTOR_TO_MJPEG_X(vec->box_w);
				dy = MOTION_VECTOR_TO_MJPEG_X(vec->box_h);
				glcd_draw_rectangle(glcd, da, color, x, y, dx, dy);

				x  = MOTION_VECTOR_TO_MJPEG_X(vec->x);
				y  = MOTION_VECTOR_TO_MJPEG_Y(vec->y);
				r_unit = MOTION_VECTOR_TO_MJPEG_X(1);
				dx = -vec->vx * r_unit;
				dy = -vec->vy * r_unit;
				glcd_draw_line(glcd, da, color, x, y, x + dx, y + dy);
				if (mreg->motion > 0)
					glcd_fill_circle(glcd, da, color, x + dx, y + dy, 6);
				else
					glcd_draw_circle(glcd, da, color, x + dx, y + dy, 6);
				}
			}
		snprintf(info, sizeof(info), "Magnitude limit: %d",
								(int) sqrt(mf->mag2_limit));
		i420_print(&top_status_area, normal_font, 0xff, 0, 1, 0,
					JUSTIFY_LEFT, info);

		snprintf(info, sizeof(info), "    Limit count: %d",
								mf->mag2_limit_count);
		i420_print(&top_status_area, normal_font, 0xff, 1, 1, 0,
					JUSTIFY_LEFT, info);

		if (mf->frame_window > 0 && mf->motion_status == MOTION_NONE)
			{
			snprintf(status, sizeof(status), "%d", mf->frame_window);
			msg = status;
			}
		else if (mf->motion_status == MOTION_DETECTED)
			msg = "motion";
		else if (mf->trigger_count > 0)
			msg = "counts";
		else if (mf->sparkle_count > 0 || mf->reject_count > 0)
			msg = "noise";
		else
			msg = "quiet";

		snprintf(info, sizeof(info), "cnt:%d rej:%d spkl:%d %.1f",
					mf->trigger_count, mf->reject_count,
					mf->sparkle_count, mf->sparkle_expma);
		i420_print(&bottom_status_area, normal_font, 0xff, 1, 40, 0,
					JUSTIFY_CENTER, msg);
		i420_print(&bottom_status_area, normal_font, 0xff, 0, 40, 0,
					JUSTIFY_CENTER, info);
		i420_print(&bottom_status_area, normal_font, 0xff, 1, 0, 0,
					JUSTIFY_RIGHT(0), "Regions ON");
		}
	else
		mf->selected_region = -1;

	if (vcb->state & VCB_STATE_MOTION_RECORD)
		{
		if (pikrellcam.t_now < vcb->motion_sync_time)
			t_record = pikrellcam.t_now - vcb->record_start;
		else
			t_record = vcb->motion_sync_time - vcb->record_start;
		t_hold = pikrellcam.motion_times.event_gap -
				(pikrellcam.t_now - vcb->motion_last_detect_time);
		snprintf(info, sizeof(info), "REC (Motion) %d:%02d  hold %d:%02d",
					t_record / 60, t_record % 60,
					t_hold / 60, t_hold % 60);
		}
	else if (vcb->state & VCB_STATE_MANUAL_RECORD)
		{
		t_record = pikrellcam.t_now - vcb->record_start;
		snprintf(info, sizeof(info), "REC (%s) %d:%02d",
					vcb->pause ? "Pause" : "Manual",
					t_record / 60, t_record % 60);
		}
	else
		snprintf(info, sizeof(info), "REC (Stop)");
	i420_print(&bottom_status_area, normal_font, 0xff, 1, 1, 0,
					JUSTIFY_LEFT, info);

	if (mf->motion_enable)
		{
		if (vcb->state & VCB_STATE_MANUAL_RECORD)
			msg = "Motion ---";
		else
			msg = "Motion  ON";
		}
	else
		msg = "Motion OFF";
	i420_print(&bottom_status_area, normal_font, 0xff, 0, 1, 0,
					JUSTIFY_LEFT, msg);

	if (time_lapse.show_status)
		{
		int		p = time_lapse.period,
		        t = 0;
		char	tbuf[32];

		if (time_lapse.event)
			t = (int) (time_lapse.event->time - pikrellcam.t_now);
		if (!time_lapse.activated)
			{
			snprintf(info, sizeof(info), "Timelapse:  off ");
			}
		else if (time_lapse.on_hold)
			{
			snprintf(info, sizeof(info), "Timelapse: hold ");
			}
		else if (p >= 3600)
			{
			snprintf(info, sizeof(info), "Timelapse:  %d:%2.2d:%2.2d ",
					p / 3600, (p % 3600) / 60, (p % 3600) % 60);
			snprintf(tbuf, sizeof(tbuf),     "%d.%05d [%d:%2.2d:%2.2d]",
					time_lapse.series, time_lapse.sequence,
					t / 3600, (t % 3600) / 60, (t % 3600) % 60);
			}
		else if (p >= 60)
			{
			snprintf(info, sizeof(info),
					"Timelapse:  %2.2d:%2.2d ",
								p / 60, p % 60);
			snprintf(tbuf, sizeof(tbuf),
					    "%d.%05d [%2.2d:%2.2d]",
					time_lapse.series, time_lapse.sequence, t / 60, t % 60);
			}
		else
			{
			snprintf(info, sizeof(info), "Timelapse:  %2d ", p);
			snprintf(tbuf, sizeof(tbuf),     "%d.%05d [%2d]",
						time_lapse.series, time_lapse.sequence, t);
			}
		i420_print(&top_status_area, normal_font, 0xff, 0, 0, 0,
					JUSTIFY_RIGHT(0), info);
		i420_print(&top_status_area, normal_font, 0xff, 1, 0, 0,
					JUSTIFY_RIGHT(0), tbuf);
		}
	if (time_lapse.convert_name && *time_lapse.convert_name)
		{
		snprintf(info, sizeof(info), "Converting: %s %.3fM ",
				time_lapse.convert_name,
				(float) time_lapse.convert_size / 1000000.0);
		i420_print(&top_status_area, normal_font, 0xff, 2, 0, 0,
					JUSTIFY_RIGHT(0), info);
			}
	if (pikrellcam.video_notify)
		i420_print(&inform_area, normal_font, 0xff, 6, 0, 0, JUSTIFY_CENTER,
					fname_base(pikrellcam.video_last_save));
	if (pikrellcam.still_notify)
		i420_print(&inform_area, normal_font, 0xff, 5, 0, 0, JUSTIFY_CENTER,
					fname_base(pikrellcam.still_last_save));
	if (pikrellcam.timelapse_notify && time_lapse.show_status)
		i420_print(&inform_area, normal_font, 0xff, 4, 0, 0, JUSTIFY_CENTER,
					fname_base(pikrellcam.timelapse_last_save));
	}

typedef struct
	{
	char	*name;
	int		length;
	int		line_position;
	}
	MenuEntry;

static SList 	*display_menu_list;
static int		*display_menu_index;

static SList 	*menu_video_presets_list;
static int		menu_video_presets_index;

static SList 	*menu_camera_adjustments_list;
static int		menu_camera_adjustments_index;

static SList 	*menu_still_presets_list;
static int		menu_still_presets_index;

static SList 	*menu_picture_list;
static int		menu_picture_index;

static SList 	*menu_metering_list;
static int		menu_metering_index;

static SList 	*menu_exposure_list;
static int		menu_exposure_index;

static SList 	*menu_white_balance_list;
static int		menu_white_balance_index;

static SList 	*menu_image_effect_list;
static int		menu_image_effect_index;

static SList 	*menu_motion_time_list;
static int		menu_motion_time_index;

static SList 	*menu_motion_limit_list;
static int		menu_motion_limit_index;

static SList 	*menu_motion_setting_list;
static int		menu_motion_setting_index;


static char	*video_presets_entry[] =
	{
	"1080p",
	"720p",
	"1296x972",
	"1024x768",
	"1024x576"
	};

#define N_VIDEO_PRESET_ENTRIES \
		(sizeof(video_presets_entry) / sizeof(char *))

static char	*still_presets_entry[] =
	{
	"1920x1080",
	"1280x720",
	"2592x1944",
	"2592x1458",
	"1920x1440",
	"1296x972",
	"1024x768",
	"1024x576"
	};

#define N_STILL_PRESET_ENTRIES \
		(sizeof(still_presets_entry) / sizeof(char *))

static char	*metering_entry[] =
	{
	"average",
	"spot",
	"backlit",
	"matrix"
	};

#define N_METERING_ENTRIES \
		(sizeof(metering_entry) / sizeof(char *))

static char	*exposure_entry[] =
	{
	"off",
	"auto",
	"night",
	"nightpreview",
	"backlight",
	"spotlight",
	"sports",
	"snow",
	"beach",
	"verylong",
	"fixedfps",
	"antishake",
	"fireworks"
	};

#define N_EXPOSURE_ENTRIES \
		(sizeof(exposure_entry) / sizeof(char *))

static char	*white_balance_entry[] =
	{
	"off",
	"auto",
	"sunlight",
	"cloudy",
	"shade",
	"tungsten",
	"flourescent",
	"incandescent",
	"flash",
	"horizon"
	};

#define N_WHITE_BALANCE_ENTRIES \
		(sizeof(white_balance_entry) / sizeof(char *))


static char	*image_effect_entry[] =
	{
	"none",
	"negative",
	"solarize",
	"posterize",
	"whiteboard",
	"blackboard",
	"sketch",
	"denoise",
	"emboss",
	"oilpaint",
	"hatch",
	"gpen",
	"pastel",
	"watercolor",
	"film",
	"blur",
	"saturation",
	"colorswap",
	"washedout",
	"posterise",
	"colorpoint",
	"colourbalance",
	"cartoon"
	};

#define N_IMAGE_EFFECT_ENTRIES \
		(sizeof(image_effect_entry) / sizeof(char *))


typedef struct
	{
	char	*name;
	int		min,
			max,
			increment,
			value,
			prev_value,
			revert_value;
	char	*units;
	CameraParameter
			*cam_param;
	int		*config_value;
	}
	Adjustment;


  /* Adjustment changes made to a temp struct to avoid thrashing camera
  |  destroys/creates.  Final change is applied if/when SEL is clicked.
  */
static Adjustment	camera_adjustment[] =
	{
	{ "video bitrate",   2000000, 18000000, 100000, 0, 0, 0, "", NULL, &camera_adjust_temp.video_bitrate },
	{ "video fps",       1,    30,    1,   0, 0, 0, "", NULL, &camera_adjust_temp.video_fps },
	{ "mp4 boxing fps",  1,    30,    1,   0, 0, 0, "", NULL, &camera_adjust_temp.mp4_box_fps },
	{ "still quality",  5,    100,   1,   0, 0, 0, "", NULL, &camera_adjust_temp.still_quality },
	};

#define N_CAMERA_ADJUSTMENTS \
		(sizeof(camera_adjustment) / sizeof(Adjustment))

static Adjustment	picture_adjustment[] =
	{
	{ "sharpness",  -100, 100, 1,   0, 0, 0, "", NULL, NULL },
	{ "contrast",   -100, 100, 1,   0, 0, 0, "", NULL, NULL },
	{ "brightness",    0, 100, 1,   0, 0, 0, "", NULL, NULL },
	{ "saturation", -100, 100, 1,   0, 0, 0, "", NULL, NULL },
	{ "iso",           0, 800, 100, 0, 0, 0, "", NULL, NULL },
	{ "exposure_compensation", -25, 25, 1, 0, 0, 0, "", NULL, NULL },
	{ "video_stabilisation", 0, 1, 1, 0, 0, 0, "", NULL, NULL },
	{ "rotation",      0, 270, 90, 0, 0, 0, "", NULL, NULL },
	{ "shutter_speed", 0, 6000000, 100, 0, 0, 0, "usec", NULL, NULL }
	};

#define N_PICTURE_ADJUSTMENTS \
		(sizeof(picture_adjustment) / sizeof(Adjustment))


  /* Adjustment changes made to a temp struct to avoid thrashing malloc/free
  |  of huge circular buffer.  Final change is applied if/when SEL is clicked.
  */
Adjustment	motion_time_adjustment[] =
	{
	{ "Confirm_Gap",  0, 30,  1, 0, 0, 0, "", NULL, &motion_times_temp.confirm_gap },
	{ "Pre_Capture",  1, 180, 1, 0, 0, 0, "", NULL, &motion_times_temp.pre_capture },
	{ "Event_Gap",    5, 300, 1, 0, 0, 0, "", NULL, &motion_times_temp.event_gap },
	{ "Post_Capture", 1, 180, 1, 0, 0, 0, "", NULL, &motion_times_temp.post_capture }
	};

#define N_MOTION_TIME_ADJUSTMENTS \
		(sizeof(motion_time_adjustment) / sizeof(Adjustment))

Adjustment	motion_limit_adjustment[] =
	{
	{ "Vector_Magnitude",  3, 40, 1, 0, 0, 0, "", NULL, &pikrellcam.motion_magnitude_limit },
	{ "Vector_Count",      2, 40, 1, 0, 0, 0, "", NULL, &pikrellcam.motion_magnitude_limit_count }
	};

#define N_MOTION_LIMIT_ADJUSTMENTS \
		(sizeof(motion_limit_adjustment) / sizeof(Adjustment))

Adjustment	motion_setting_adjustment[] =
	{
	{ "Startup_Motion",   0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.motion_enable },
	{ "Vector_Dimming",   30, 60, 1, 0, 0, 0, "", NULL, &pikrellcam.motion_vectors_dimming },
//	{ "Vertical_Filter",  0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.motion_vertical_filter },
	{ "Preview_Clean",    0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.motion_preview_clean },
	};

#define N_MOTION_SETTING_ADJUSTMENTS \
		(sizeof(motion_setting_adjustment) / sizeof(Adjustment))


#define VECTOR_DIMMING_INDEX	1  /* Must track above Vector_Dimming entry */


static Adjustment	*cur_adj,
					*adjustments;

static boolean		cur_adj_start;

void
restart_camera(void)
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

  /* Apply adjustments that were not done live.
  */
static void
apply_adjustment(void)
	{
	VideoCircularBuffer	*vcb = &video_circular_buffer;

	if (!adjustments || !cur_adj)
		return;
	if (adjustments == &motion_time_adjustment[0])
		{
		if (   motion_times_temp.pre_capture != pikrellcam.motion_times.pre_capture
		    || motion_times_temp.event_gap != pikrellcam.motion_times.event_gap
		   )
			{
			pthread_mutex_lock(&vcb->mutex);
			pikrellcam.motion_times  = motion_times_temp;
			circular_buffer_init();
			pthread_mutex_unlock(&vcb->mutex);
			}		
		}
	else if (adjustments == &camera_adjustment[0])
		{
		if (   camera_adjust_temp.video_fps != pikrellcam.camera_adjust.video_fps
		    || camera_adjust_temp.still_quality != pikrellcam.camera_adjust.still_quality
		    || camera_adjust_temp.video_bitrate != pikrellcam.camera_adjust.video_bitrate
		   )
			restart_camera();
		else	/* Pick up changes not requiring new buffer or camera restart */
			pikrellcam.camera_adjust = camera_adjust_temp;

		/* All other adjustments have been done live. */
		}
	}

static void
revert_adjustment(void)
	{
	char	buf[50];

	if (!adjustments || !cur_adj)
		return;

	if (cur_adj->value != cur_adj->revert_value)
		{
		if (adjustments == &picture_adjustment[0])
			{
			snprintf(buf, sizeof(buf), "%d", cur_adj->revert_value);
			mmalcam_config_parameter_set(cur_adj->name, buf, TRUE);
			}
		else
			*(cur_adj->config_value) = cur_adj->revert_value;
		}
	}

static void
display_adjustment(uint8_t *i420)
	{
	DrawArea	*da = &adj_control_area;
	GlcdFont	*font = normal_font;
	char		buf[50];
	int			bar_x0, bar_y0, bar_dy, bar_width, adj_x, fast_factor;
	static int	prev_action;
	boolean		boolean_flag = FALSE;

	cur_adj = adjustments + *display_menu_index;
	if (cur_adj_start)
		{
		if (adjustments == &picture_adjustment[0])
			{
			cur_adj->cam_param = mmalcam_config_parameter_get(cur_adj->name);
			if (!cur_adj->cam_param)
				{
				log_printf("??? no camera parameter %s\n", cur_adj->name);
				return;
				}
			cur_adj->value = atoi(cur_adj->cam_param->arg);;
			}
		else
			cur_adj->value = *(cur_adj->config_value);

		cur_adj->revert_value = cur_adj->value;
		cur_adj_start = FALSE;
		}
	else if (   display_action == ACTION_NONE
	         && (   prev_action == REPEAT_LEFT_ARROW
	             || prev_action == REPEAT_RIGHT_ARROW
	            )
	        )
		display_action = prev_action;

	cur_adj->prev_value = cur_adj->value;

	fast_factor = 2;
	while ((cur_adj->max - cur_adj->min) / (fast_factor * cur_adj->increment)
				> 50)
		fast_factor += 1;

	if (fast_factor > 20)	/* XXX for shutter_speed, this needs work */
		fast_factor = 20;

	if (display_action == LEFT_ARROW)
		cur_adj->value -= cur_adj->increment;
	else if (display_action == RIGHT_ARROW)
		cur_adj->value += cur_adj->increment;
	else if (display_action == REPEAT_LEFT_ARROW)
		cur_adj->value -= fast_factor * cur_adj->increment;
	else if (display_action == REPEAT_RIGHT_ARROW)
		cur_adj->value += fast_factor * cur_adj->increment;

	if (cur_adj->value > cur_adj->max)
		{
		cur_adj->value = cur_adj->max;
		prev_action = ACTION_NONE;		/* Turn off possible REPEAT */
		}
	else if (cur_adj->value < cur_adj->min)
		{
		cur_adj->value = cur_adj->min;
		prev_action = ACTION_NONE;		/* Turn off possible REPEAT */
		}
	else
		prev_action = display_action;

	if (cur_adj->value != cur_adj->prev_value)
		{
		if (adjustments == &picture_adjustment[0])
			{
			snprintf(buf, sizeof(buf), "%d", cur_adj->value);
			mmalcam_config_parameter_set(cur_adj->name, buf, TRUE);
			}
		else
			*(cur_adj->config_value) = cur_adj->value;
		}

	bar_y0 = font->char_height * 3 / 2;
	bar_dy = font->char_height / 2;

	if (cur_adj->min == 0 && cur_adj->max == 1)
		{
		boolean_flag = TRUE;
		bar_width = 16 * font->char_width;
		bar_x0 = (da->width - bar_width) / 2;
		}
	else
		{
		bar_x0 = 12 * font->char_width;
		bar_width = da->width - 2 * bar_x0;
		}
	adj_x = bar_x0 + (int64_t) bar_width * (int64_t) (cur_adj->value - cur_adj->min)
									/ (int64_t) (cur_adj->max - cur_adj->min);

	if (   adjustments == &motion_setting_adjustment[0]
	    && *display_menu_index == VECTOR_DIMMING_INDEX
	   )
		i420_dim_frame(i420);

	glcd_draw_rectangle(glcd, da, 0, bar_x0, bar_y0, bar_width, bar_dy);
	glcd_fill_rectangle(glcd, da, 0xe0, bar_x0 + 1, bar_y0 + 1,
						bar_width  - 2, bar_dy - 2);
	glcd_fill_rectangle(glcd, da, 0,
				adj_x - 2, bar_y0 - bar_dy, 5, bar_dy);
	glcd_fill_rectangle(glcd, da, 0xe0,
				adj_x - 1, bar_y0 - bar_dy + 1, 3, bar_dy - 1);

	if (boolean_flag)
		snprintf(buf, sizeof(buf), "%s", "");
	else
		snprintf(buf, sizeof(buf), "%d", cur_adj->min);
	i420_print(da, font, 0xff, 1, bar_x0 - 2, 0, JUSTIFY_LEFT_AT_X, buf);

	if (boolean_flag)
		snprintf(buf, sizeof(buf), "%s", "");
	else
		snprintf(buf, sizeof(buf), "%d", cur_adj->max);
	i420_print(da, font, 0xff, 1, bar_x0 + bar_width + 2, 0, JUSTIFY_LEFT, buf);

	if (boolean_flag)
		snprintf(buf, sizeof(buf), "%s", cur_adj->value ? "ON" : "OFF");
	else
		snprintf(buf, sizeof(buf), "%d", cur_adj->value);
	i420_print(da, font, 0xff, 0, adj_x, 0, JUSTIFY_CENTER_AT_X, buf);

	i420_print(da, font, 0xff, 2, 0, 0, JUSTIFY_CENTER, cur_adj->name);
	}

static void
display_draw_menu(uint8_t *i420)
	{
	MenuEntry	*current, *entry;
	DrawArea	*da = &adj_control_area;
	GlcdFont	*font = normal_font;
	SList		*list;
	char		*param = NULL;
	int			x0, x, y, dx, len;

	if (!display_menu_list)
		return;
	len = slist_length(display_menu_list);

	if (display_action == SELECT)
		{
		switch (display_menu)
			{
			case CAMERA_ADJUSTMENTS:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &camera_adjustment[0];
				cur_adj_start = TRUE;
				break;
			case MOTION_LIMIT:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &motion_limit_adjustment[0];
				cur_adj_start = TRUE;
				break;
			case MOTION_TIME:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &motion_time_adjustment[0];
				cur_adj_start = TRUE;
				break;
			case MOTION_SETTING:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &motion_setting_adjustment[0];
				cur_adj_start = TRUE;
				break;
			case PICTURE:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &picture_adjustment[0];
				cur_adj_start = TRUE;
				break;

			case VIDEO_PRESET:
				break;
			case STILL_PRESET:
				break;

			case METERING:
				param = "metering_mode";
				break;
			case EXPOSURE:
				param = "exposure_mode";
				break;
			case WHITE_BALANCE:
				param = "white_balance";
				break;
			case IMAGE_EFFECT:
				param = "image_effect";
				break;
			}
		if (   param
		    && (entry = (MenuEntry *) slist_nth_data(display_menu_list,
							*display_menu_index)) != NULL
		   )
			{
			mmalcam_config_parameter_set(param, entry->name, TRUE);
			pikrellcam.config_modified = TRUE;
			}

		return;
		}
	if (display_action == LEFT_ARROW)
		*display_menu_index -= 1;
	else if (display_action == RIGHT_ARROW)
		*display_menu_index += 1;
	else if (display_action == REPEAT_LEFT_ARROW)
		*display_menu_index -= 3;
	else if (display_action == REPEAT_RIGHT_ARROW)
		*display_menu_index += 3;
	if (*display_menu_index < 0)
		*display_menu_index = 0;
	if (*display_menu_index > len - 1)
		*display_menu_index = len - 1;

	current = slist_nth_data(display_menu_list, *display_menu_index);
	x0 = da->width / 2;
	x0 -= current->length * large_font->char_width / 2;
	x0 -= current->line_position * normal_font->char_width;
	for (x = x0, list = display_menu_list; list; list = list->next)
		{
		entry = (MenuEntry *) list->data;
		if (entry == current)
			{
			font = large_font;
			y = 0;
			}
		else
			{
			font = normal_font;
			y = large_font->char_height - normal_font->char_height;
			}
		i420_print(da, font,
				(entry == current) ? 0xff : 0xe0,
				0, x, y, JUSTIFY_LEFT, entry->name);
		x += (entry->length + 1) * font->char_width;
		}
	dx = current->length * large_font->char_width;
	x0 = (da->width - dx) / 2;
	y = large_font->char_height;
	glcd_draw_h_line(glcd, da, 0xe0, x0, y, dx);
	glcd_draw_h_line(glcd, da, 0, x0, y + 1, dx);
	}

void
display_command(char *cmd)
	{
	MenuEntry		*entry;
	CameraParameter	*cam_param = NULL;
	DisplayCommand	*dcmd;
	SList			*list;
	int				i, width, height;

	for (i = 0; i < N_DISPLAY_COMMANDS; ++i)
		{
		dcmd = &display_commands[i];
		if (!strcmp(dcmd->name, cmd))
			break;
		}
	if (i >= N_DISPLAY_COMMANDS)
		{
		log_printf("Bad display command: %s\n", cmd);
		return;
		}
//	printf("%s: %d\n", dcmd->name, dcmd->action);

	display_action = dcmd->action;
	switch (dcmd->action)
		{
		case REPEAT_LEFT_ARROW:
			break;
		case LEFT_ARROW:
			break;
		case SELECT:
			if (display_state == DISPLAY_ADJUSTMENT)
				{
				apply_adjustment();
				pikrellcam.config_modified = TRUE;
				display_state = DISPLAY_MENU;
				display_action = ACTION_NONE;
				}
			else if (display_state == DISPLAY_MENU && display_menu == VIDEO_PRESET)
				{
				entry = (MenuEntry *) slist_nth_data(display_menu_list,
								*display_menu_index);
				if (!strcmp(entry->name, "1080p"))
					{
					width = 1920;
					height = 1080;
					}
				else if (!strcmp(entry->name, "720p"))
					{
					width = 1296;	/* This is camera 720p mode */
					height = 730;
					}
				else
					sscanf(entry->name, "%dx%d", &width, &height);

				if (   width != pikrellcam.camera_config.video_width
				    || height != pikrellcam.camera_config.video_height
				   )
					{
					pikrellcam.camera_config.video_width = width;
					pikrellcam.camera_config.video_height = height;
					pikrellcam.config_modified = TRUE;
					restart_camera();
					}
				display_state = DISPLAY_MENU;
				display_action = ACTION_NONE;
				}
			else if (display_state == DISPLAY_MENU && display_menu == STILL_PRESET)
				{
		    	entry = (MenuEntry *) slist_nth_data(display_menu_list,
										*display_menu_index);
				sscanf(entry->name, "%dx%d", &width, &height);
				if (   width != pikrellcam.camera_config.still_width
				    || height != pikrellcam.camera_config.still_height
				   )
					{
					pikrellcam.camera_config.still_width = width;
					pikrellcam.camera_config.still_height = height;
					pikrellcam.config_modified = TRUE;
					restart_camera();
					}
				display_state = DISPLAY_MENU;
				display_action = ACTION_NONE;
				}
			break;
		case RIGHT_ARROW:
			break;
		case REPEAT_RIGHT_ARROW:
			break;
		case BACK:
			if (display_state == DISPLAY_ADJUSTMENT)
				{
				revert_adjustment();
				display_state = DISPLAY_MENU;
				display_action = ACTION_NONE;
				}
			else
				display_state = DISPLAY_DEFAULT;
			break;
		case MOTION_TIME:
			display_menu_list = menu_motion_time_list;
			display_menu_index = &menu_motion_time_index;
			display_state = DISPLAY_MENU;
			display_menu = MOTION_TIME;
			break;
		case MOTION_LIMIT:
			display_menu_list = menu_motion_limit_list;
			display_menu_index = &menu_motion_limit_index;
			display_state = DISPLAY_MENU;
			display_menu = MOTION_LIMIT;
			break;
		case MOTION_SETTING:
			display_menu_list = menu_motion_setting_list;
			display_menu_index = &menu_motion_setting_index;
			display_state = DISPLAY_MENU;
			display_menu = MOTION_SETTING;
			break;
		case PICTURE:
			display_menu_list = menu_picture_list;
			display_menu_index = &menu_picture_index;
			display_state = DISPLAY_MENU;
			display_menu = PICTURE;
			break;

		case VIDEO_PRESET:
			display_menu_list = menu_video_presets_list;
			display_menu_index = &menu_video_presets_index;
			display_state = DISPLAY_MENU;
			display_menu = VIDEO_PRESET;
			for (i = N_VIDEO_PRESET_ENTRIES - 1; i > 0 ; --i)
				{
				if (!strcmp(video_presets_entry[i], "720p"))
					{
					width = 1296;
					height = 730;
					}
				else
					sscanf(video_presets_entry[i], "%dx%d", &width, &height);
				if (   pikrellcam.camera_config.video_width == width
				    && pikrellcam.camera_config.video_height == height
				   )
					break;
				}
			menu_video_presets_index = i;
			break;
		case CAMERA_ADJUSTMENTS:
			display_menu_list = menu_camera_adjustments_list;
			display_menu_index = &menu_camera_adjustments_index;
			display_state = DISPLAY_MENU;
			display_menu = CAMERA_ADJUSTMENTS;
			break;
		case STILL_PRESET:
			display_menu_list = menu_still_presets_list;
			display_menu_index = &menu_still_presets_index;
			display_state = DISPLAY_MENU;
			display_menu = STILL_PRESET;
			for (i = N_STILL_PRESET_ENTRIES - 1; i > 0 ; --i)
				{
				sscanf(still_presets_entry[i], "%dx%d", &width, &height);
				if (   pikrellcam.camera_config.still_width == width
				    && pikrellcam.camera_config.still_height == height
				   )
					break;
				}
			menu_still_presets_index = i;
			break;

		case METERING:
			cam_param = mmalcam_config_parameter_get("metering_mode");
			display_menu_list = menu_metering_list;
			display_menu_index = &menu_metering_index;
			display_state = DISPLAY_MENU;
			display_menu = METERING;
			break;
		case EXPOSURE:
			cam_param = mmalcam_config_parameter_get("exposure_mode");
			display_menu_list = menu_exposure_list;
			display_menu_index = &menu_exposure_index;
			display_state = DISPLAY_MENU;
			display_menu = EXPOSURE;
			break;
		case WHITE_BALANCE:
			cam_param = mmalcam_config_parameter_get("white_balance");
			display_menu_list = menu_white_balance_list;
			display_menu_index = &menu_white_balance_index;
			display_state = DISPLAY_MENU;
			display_menu = WHITE_BALANCE;
			break;
		case IMAGE_EFFECT:
			cam_param = mmalcam_config_parameter_get("image_effect");
			display_menu_list = menu_image_effect_list;
			display_menu_index = &menu_image_effect_index;
			display_state = DISPLAY_MENU;
			display_menu = IMAGE_EFFECT;
			break;
		}
	if (cam_param)
		{
		for (i = 0, list = display_menu_list; list; ++i, list = list->next)
			{
			if (!strcmp(((MenuEntry *)list->data)->name, cam_param->arg))
				{
				*display_menu_index = i;
				break;
				}
			}
		}
	}

static int quit_flag;

void
display_draw(uint8_t *i420)
	{
	/* If this frame will be a preview save jpeg and the user wants it clean,
	|  do not draw.
	*/
	if (   motion_frame.do_preview_save			/* This frame will be preview */
	    && pikrellcam.motion_preview_clean
	   )
		return;  /* User wants a clean preview jpeg */

	glcd_set_frame_buffer(glcd, (uint16_t *) i420,
				pikrellcam.mjpeg_width, pikrellcam.mjpeg_height);
	switch (display_state)
		{
		case DISPLAY_DEFAULT:
			motion_draw(i420);
			break;
		case DISPLAY_MENU:
			display_draw_menu(i420);
			break;
		case DISPLAY_ADJUSTMENT:
			display_adjustment(i420);
			break;
		case DISPLAY_INFORM:
			break; /* TODO */
		case DISPLAY_QUIT:
			i420_print(&bottom_status_area, large_font, 0xff, 0, 0, 0,
					JUSTIFY_CENTER, "PiKrellCam Stopped");
			quit_flag = FALSE;
			break;
		}
	display_action = ACTION_NONE;
	}

boolean
display_is_default(void)
	{
	return (display_state == DISPLAY_DEFAULT) ? TRUE : FALSE;
	}

void
display_set_default(void)
	{
	revert_adjustment();
	display_state = DISPLAY_DEFAULT;
	}

void
display_quit(void)
	{
	int	i;

	display_state = DISPLAY_QUIT;
	quit_flag = TRUE;
	for (i = 0; i < 10; ++i)
		{
		if (!quit_flag)
			break;
		else
			usleep(100000);
		}
	}

  /* Init the glcd library.  Set rotation to get the display area initialized
  |  even though the i420 driver ignores rotation (the camera does its own
  |  rotation).  The frame buffer will be set at each display call.
  */
void
display_init(void)
	{
	DrawArea	da;
	MenuEntry	*entry;
	Adjustment	*adj;
	int			i, position;

	if (!glcd)
		glcd = glcd_i420_init();
	glcd_set_frame_buffer(glcd, NULL,	/* pointer to be set at display calls */
				pikrellcam.mjpeg_width, pikrellcam.mjpeg_height);

	glcd_set_rotation(glcd, 0);	  /* rotation ignored, but sets display area */
	draw_area = glcd_get_display_area(glcd);

	/* split a bottom_status_area off the screen draw area for two lines
	|  of status text
	*/
	glcd_area_v_split(draw_area, &da, &bottom_status_area,
			2 * normal_font->char_height + 2, (SPLIT_PIXELS | SPLIT_2ND), 0);

	/* Out of what's left, split out a top status are for three lines of text.
	*/
	glcd_area_v_split(&da, &top_status_area, &inform_area,
			3 * normal_font->char_height + 2, (SPLIT_PIXELS | SPLIT_1ST), 0);

	/* adj_control_area is inside of the inform_area.
	*/
	glcd_area_v_split(&inform_area, &da /*throw away */, &adj_control_area,
			3 * normal_font->char_height + 2, (SPLIT_PIXELS | SPLIT_2ND), 0);

	if (pikrellcam.verbose)
		{
		printf("top status area:    x:%d y:%d dx:%d dy:%d\n",
			top_status_area.x0, top_status_area.y0,
			top_status_area.width, top_status_area.height);
		printf("inform area:        x:%d y:%d dx:%d dy:%d\n",
			inform_area.x0, inform_area.y0,
			inform_area.width, inform_area.height);
		printf("adj_control area:   x:%d y:%d dx:%d dy:%d\n",
			adj_control_area.x0, adj_control_area.y0,
			adj_control_area.width, adj_control_area.height);
		printf("bottom status area: x:%d y:%d dx:%d dy:%d\n",
			bottom_status_area.x0, bottom_status_area.y0,
			bottom_status_area.width, bottom_status_area.height);
		}
	if (!menu_motion_time_list)
		{
		for (i = 0, position = 0; i < N_MOTION_TIME_ADJUSTMENTS; ++i)
			{
			adj = &motion_time_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_motion_time_list = slist_append(menu_motion_time_list, entry);
			}
		}
	if (!menu_motion_limit_list)
		{
		for (i = 0, position = 0; i < N_MOTION_LIMIT_ADJUSTMENTS; ++i)
			{
			adj = &motion_limit_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_motion_limit_list = slist_append(menu_motion_limit_list, entry);
			}
		}
	if (!menu_motion_setting_list)
		{
		for (i = 0, position = 0; i < N_MOTION_SETTING_ADJUSTMENTS; ++i)
			{
			adj = &motion_setting_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_motion_setting_list = slist_append(menu_motion_setting_list, entry);
			}
		}
	if (!menu_video_presets_list)
		{
		for (i = 0, position = 0; i < N_VIDEO_PRESET_ENTRIES; ++i)
			{
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = video_presets_entry[i];
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_video_presets_list = slist_append(menu_video_presets_list, entry);
			}
		}
	if (!menu_camera_adjustments_list)
		{
		for (i = 0, position = 0; i < N_CAMERA_ADJUSTMENTS; ++i)
			{
			adj = &camera_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_camera_adjustments_list = slist_append(menu_camera_adjustments_list, entry);
			}
		}
	if (!menu_still_presets_list)
		{
		for (i = 0, position = 0; i < N_STILL_PRESET_ENTRIES; ++i)
			{
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = still_presets_entry[i];
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_still_presets_list = slist_append(menu_still_presets_list, entry);
			}
		}
	if (!menu_picture_list)
		{
		for (i = 0, position = 0; i < N_PICTURE_ADJUSTMENTS; ++i)
			{
			adj = &picture_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_picture_list = slist_append(menu_picture_list, entry);
			}
		}
	if (!menu_metering_list)
		{
		for (i = 0, position = 0; i < N_METERING_ENTRIES; ++i)
			{
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = metering_entry[i];
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_metering_list = slist_append(menu_metering_list, entry);
			}
		}
	if (!menu_exposure_list)
		{
		for (i = 0, position = 0; i < N_EXPOSURE_ENTRIES; ++i)
			{
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = exposure_entry[i];
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_exposure_list = slist_append(menu_exposure_list, entry);
			}
		}
	if (!menu_white_balance_list)
		{
		for (i = 0, position = 0; i < N_WHITE_BALANCE_ENTRIES; ++i)
			{
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = white_balance_entry[i];
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_white_balance_list = slist_append(menu_white_balance_list, entry);
			}
		}
	if (!menu_image_effect_list)
		{
		for (i = 0, position = 0; i < N_IMAGE_EFFECT_ENTRIES; ++i)
			{
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = image_effect_entry[i];
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_image_effect_list = slist_append(menu_image_effect_list, entry);
			}
		}
	}
