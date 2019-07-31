/* PiKrellCam
|
|  Copyright (C) 2015-2019 Bill Wilson    billw@gkrellm.net
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

#define DISPLAY_MENU_NONE	0
#define VIDEO_PRESET		10
#define STILL_PRESET		11
#define METERING			13
#define EXPOSURE			14
#define WHITE_BALANCE		15
#define IMAGE_EFFECT		16
#define MOTION_LIMIT		17
#define SETTINGS			18
#define MOTION_SETTINGS		19
#define SERVO_SETTINGS		20
#define LOOP_SETTINGS		21
#define AUDIO_SETTINGS		22

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
	{ "motion_settings",    MOTION_SETTINGS  },
	{ "motion_limit",   MOTION_LIMIT  },
	{ "video_presets",      VIDEO_PRESET		  },
	{ "still_presets",      STILL_PRESET	  },
	{ "settings",        SETTINGS },
	{ "loop_settings",  LOOP_SETTINGS },
	{ "servo_settings",  SERVO_SETTINGS },
	{ "audio_settings",  AUDIO_SETTINGS },
	{ "picture",  PICTURE		  },
	{ "metering", METERING		  },
	{ "exposure", EXPOSURE		  },
	{ "white_balance", WHITE_BALANCE	  },
	{ "image_effect", IMAGE_EFFECT	  }
	};

#define N_DISPLAY_COMMANDS    (sizeof(display_commands) / sizeof(DisplayCommand))

typedef struct
	{
	char		*string;
	GlcdFont	*font;
	int			row,
				justify,
				xs,
				ys;
	}
	InformLine;

#define	N_INFORM_LINES	20

InformLine		inform_line[N_INFORM_LINES];
static int		inform_line_index;
static boolean	inform_shown;

static int	display_state;
static int	display_menu;
static int	display_action;

static DrawArea	top_status_area,
				inform_area,
				adj_control_area,
				bottom_status_area;

static boolean	adjustment_is_servo_limits(void);
static int		pan_save, tilt_save;

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
				Ydim, Ytrig, Ys;
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
			if (Ytrig < 225)
				Ytrig += 30;
			else if (Ytrig < 235)
				Ytrig += 20;
			else if (Ytrig < 245)
				Ytrig += 10;
			if (*ptrig > 2)			/* passing vector */
				*pY = Ytrig;
			else if (*ptrig == 2)	/* direction reject */
				*pY = Ydim + (Ytrig - Ydim) / 4;
			else if (*ptrig == 1)	/* sparkle */
				{
				Ys = (Ytrig - Ydim) / 2;
				if (Ys <= Ydim)
					*pY = Ydim - Ys;
				else
					*pY = 0;
				}
			else
				*pY = Ydim;
			}
		}
	}

#define SERVO_BAR_WIDTH      7
#define SERVO_TICK_WIDTH     5
#define SERVO_CORNER_MARGIN  35
#define Y_MAP(da, y)    (da->height - (y) - 1 - SERVO_CORNER_MARGIN)
#define X_MAP(da, x)    (da->width  - (x) - 1 - SERVO_CORNER_MARGIN)

static void
display_preset_setting(void)
	{
	PresetPosition	*pos;
	char			buf[32];

	if (pikrellcam.on_preset)
		{
		pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list, pikrellcam.preset_position_index);
		snprintf(buf, sizeof(buf), "%d-%d", pos->settings_index + 1, pos->n_settings);
		}
	else
		strcpy(buf, "---");
	i420_print(&bottom_status_area, normal_font, 0xff, -1, 1, 2,
				JUSTIFY_RIGHT(0), buf);
	}

static int
display_servo_pan(void)
	{
	PresetPosition	*pos;
	MotionFrame		*mf = &motion_frame;
	SList			*list;
	DrawArea		*da = draw_area;
	int				x0, y0, x, y, dx, i, pan, tilt;
	char			buf[16];

	servo_get_position(&pan, &tilt);

	dx = 2 * (pikrellcam.servo_pan_max - pikrellcam.servo_pan_min);
	x0 = X_MAP(da, dx);
	if (pikrellcam.servo_pan_max < 200)
		x0 -= 2 * (200 - pikrellcam.servo_pan_max);
	y0 = da->height - SERVO_BAR_WIDTH;
	glcd_draw_rectangle(glcd, da, 0, x0, y0, dx, SERVO_BAR_WIDTH);
	glcd_fill_rectangle(glcd, da, 0xf0, x0 + 1, y0 + 1,
					dx - 2, SERVO_BAR_WIDTH - 2);

	if (mf->show_preset || adjustment_is_servo_limits())
		{
		snprintf(buf, sizeof(buf), "%d", pan);
		i420_draw_string(da, normal_font, 0xff,
				x0 - strlen(buf) * normal_font->char_width - 4,
				da->height - normal_font->char_height + 1, buf);
		}

	y = y0 + SERVO_BAR_WIDTH - 1;
	if (pikrellcam.servo_pan_min > 101)
		{
		glcd_draw_h_line(glcd, da, 0, x0, y,
					-2 * (pikrellcam.servo_pan_min - 100));
		glcd_draw_h_line(glcd, da, 0xf0, x0, y - 1,
					-2 * (pikrellcam.servo_pan_min - 100));
		glcd_draw_h_line(glcd, da, 0, x0, y - 2,
					-2 * (pikrellcam.servo_pan_min - 100));
		}
	if (pikrellcam.servo_pan_max < 199)
		{
		glcd_draw_h_line(glcd, da, 0, x0 + dx, y,
					2 * (200 - pikrellcam.servo_pan_max));
		glcd_draw_h_line(glcd, da, 0xf0, x0 + dx, y - 1,
					2 * (200 - pikrellcam.servo_pan_max));
		glcd_draw_h_line(glcd, da, 0, x0 + dx, y - 2,
					2 * (200 - pikrellcam.servo_pan_max));
		}
	for (i = 30; i < 270; i += 10)
		{
		x = x0 + 2 * (i - pikrellcam.servo_pan_min);
		if (i == 100 || i == 150 || i == 200)
			{
			glcd_draw_v_line(glcd, da, 0,    x - 2, y0 + SERVO_BAR_WIDTH / 2, SERVO_BAR_WIDTH / 2);
			glcd_draw_v_line(glcd, da, 0,    x + 2, y0 + SERVO_BAR_WIDTH / 2, SERVO_BAR_WIDTH / 2);
			glcd_draw_h_line(glcd, da, 0,    x - 1, y0 + SERVO_BAR_WIDTH / 2, 3);
			glcd_draw_v_line(glcd, da, 0,    x,     y0, SERVO_BAR_WIDTH / 2);
			glcd_draw_h_line(glcd, da, 0xe0, x - 1, y0 + SERVO_BAR_WIDTH - 1, 3);
			glcd_draw_h_line(glcd, da, 0xe0, x - 1, y0 + SERVO_BAR_WIDTH - 2, 3);
			}
		else if (i <= pikrellcam.servo_pan_min || i >= pikrellcam.servo_pan_max)
			continue;
		else
			glcd_draw_v_line(glcd, da, 0, x, y0 + SERVO_BAR_WIDTH / 2, SERVO_BAR_WIDTH / 2);
		}

	/* position pan markers for all positions */
	for (list = pikrellcam.preset_position_list; list; list = list->next)
		{
		pos = (PresetPosition *) list->data;
		x = x0 + 2 * (pos->pan - pikrellcam.servo_pan_min);
		if (   (pos->pan == pan && pos->tilt == tilt)
		    || pos == pikrellcam.preset_last_on
		   )
			glcd_fill_rectangle(glcd, da, 0x0, x - 2, y0 + 1,
						5, SERVO_BAR_WIDTH - 1);
		else
			glcd_fill_rectangle(glcd, da, 0x0, x - 2, y0 + 1,
						5, SERVO_BAR_WIDTH / 2);
		glcd_draw_h_line(glcd, da, 0xe0, x - 2, y0 - 1, 5);
		glcd_draw_pixel(glcd, da,  0xe0, x - 2, y0);
		glcd_draw_pixel(glcd, da,  0xe0, x + 2, y0);
		}

	/* Pointer */
	x = x0 + 2 * (pan - pikrellcam.servo_pan_min);
	glcd_draw_v_line(glcd, da, 0,    x - 2, y0, SERVO_BAR_WIDTH / 2);
	glcd_draw_v_line(glcd, da, 0,    x + 2, y0, SERVO_BAR_WIDTH / 2);
	glcd_draw_h_line(glcd, da, 0,    x - 2, y0 + SERVO_BAR_WIDTH / 2, 5);
	glcd_draw_pixel(glcd, da,  0,    x,     y0 + SERVO_BAR_WIDTH / 2 - 1);
	glcd_draw_h_line(glcd, da, 0xff, x - 1, y0, 3);

	glcd_draw_h_line(glcd, da, 0xff, x - 4, y0 - 1, 9);
	glcd_draw_pixel(glcd, da,  0,    x - 5, y0 - 1);
	glcd_draw_pixel(glcd, da,  0,    x + 5, y0 - 1);
	glcd_draw_h_line(glcd, da, 0xff, x - 4, y0 - 2, 9);
	glcd_draw_pixel(glcd, da,  0,    x - 5, y0 - 2);
	glcd_draw_pixel(glcd, da,  0,    x + 5, y0 - 2);


	glcd_draw_h_line(glcd, da, 0xff, x - 3, y0 - 3, 7);
	glcd_draw_pixel(glcd, da,  0,    x - 4, y0 - 3);
	glcd_draw_pixel(glcd, da,  0,    x + 4, y0 - 3);

	glcd_draw_h_line(glcd, da, 0,    x - 3, y0 - 4, 7);
	return x0;
	}

static void
display_servo_tilt(void)
	{
	PresetPosition	*pos;
	MotionFrame		*mf = &motion_frame;
	DrawArea		*da = draw_area;
	char			buf[16];
	int             x0, y0, x, y, y1, dy, i, pan, tilt;

	servo_get_position(&pan, &tilt);

	x0 = da->width - SERVO_BAR_WIDTH;
	y0 = (pikrellcam.servo_tilt_min > 100) ?
				pikrellcam.servo_tilt_min - 100 : 0;
	dy = 2 * (pikrellcam.servo_tilt_max - pikrellcam.servo_tilt_min);

	y = Y_MAP(da, 2 * y0 + dy);
	glcd_draw_rectangle(glcd, da, 0, x0, y,
				SERVO_BAR_WIDTH, dy);
	glcd_fill_rectangle(glcd, da, 0xf0, x0 + 1, y + 1,
				SERVO_BAR_WIDTH - 2, dy - 2);

	if (mf->show_preset || adjustment_is_servo_limits())
		{
		snprintf(buf, sizeof(buf), "%d", tilt);
		y1 = y;
		if (pikrellcam.servo_tilt_max < 199)
			y1 -= 2 * (200 - pikrellcam.servo_tilt_max);
		i420_draw_string(da, normal_font, 0xff,
				da->width - strlen(buf) * normal_font->char_width - 2,
				y1 - normal_font->char_height - 2, buf);
		}

	x = x0 + SERVO_BAR_WIDTH - 1;
	if (pikrellcam.servo_tilt_min > 101)
		{
		glcd_draw_v_line(glcd, da, 0, x, y + dy,
					2 * (pikrellcam.servo_tilt_min - 100));
		glcd_draw_v_line(glcd, da, 0xf0, x - 1, y + dy,
					2 * (pikrellcam.servo_tilt_min - 100));
		glcd_draw_v_line(glcd, da, 0, x - 2, y + dy,
					2 * (pikrellcam.servo_tilt_min - 100));
		}
	if (pikrellcam.servo_tilt_max < 199)
		{
		glcd_draw_v_line(glcd, da, 0, x, y,
					-2 * (200 - pikrellcam.servo_tilt_max));
		glcd_draw_v_line(glcd, da, 0xf0, x - 1, y,
					-2 * (200 - pikrellcam.servo_tilt_max));
		glcd_draw_v_line(glcd, da, 0, x - 2, y,
					-2 * (200 - pikrellcam.servo_tilt_max));
		}
	for (i = 30; i < 270; i += 10)
		{
		if (pikrellcam.servo_tilt_min > 100)
			y = i - 100;
		else
			y = i - pikrellcam.servo_tilt_min;
		y = Y_MAP(da, 2 * y);

		if (i == 100 || i == 150 || i == 200)
			{
			glcd_draw_h_line(glcd, da, 0,    x0 + SERVO_BAR_WIDTH / 2, y - 2, SERVO_BAR_WIDTH / 2);
			glcd_draw_h_line(glcd, da, 0,    x0 + SERVO_BAR_WIDTH / 2, y + 2, SERVO_BAR_WIDTH / 2);
			glcd_draw_v_line(glcd, da, 0,    x0 + SERVO_BAR_WIDTH / 2, y - 1, 3);
			glcd_draw_h_line(glcd, da, 0,    x0,                       y, SERVO_BAR_WIDTH / 2);

			glcd_draw_v_line(glcd, da, 0xe0, x0 + SERVO_BAR_WIDTH - 1, y - 1, 3);
			glcd_draw_v_line(glcd, da, 0xe0, x0 + SERVO_BAR_WIDTH - 2, y - 1, 3);
			}
		else if (i <= pikrellcam.servo_tilt_min || i >= pikrellcam.servo_tilt_max)
			continue;
		else
			glcd_draw_h_line(glcd, da, 0, x0 + SERVO_BAR_WIDTH / 2,    y, SERVO_BAR_WIDTH / 2);
		}

	/* Current position tilt marker */
	pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list,
				pikrellcam.preset_position_index);
	if (pos)
		{
		if (pikrellcam.servo_tilt_min > 100)
			y = pos->tilt - 100;
		else
			y = pos->tilt - pikrellcam.servo_tilt_min;
		y = Y_MAP(da, 2 * y);
		glcd_fill_rectangle(glcd, da, 0x0, x0 + 1, y - 2,
					SERVO_BAR_WIDTH - 1, 5);
		glcd_draw_v_line(glcd, da, 0xe0, x0 - 1, y - 2, 5);
		glcd_draw_pixel(glcd, da,  0xe0, x0, y - 2);
		glcd_draw_pixel(glcd, da,  0xe0, x0, y + 2);
		}

	/* Pointer */
	if (pikrellcam.servo_tilt_min > 100)
		y = tilt - 100;
	else
		y = tilt - pikrellcam.servo_tilt_min;
	y = Y_MAP(da, 2 * y);

	glcd_draw_h_line(glcd, da, 0,    x0,    y - 2, SERVO_BAR_WIDTH / 2);
	glcd_draw_h_line(glcd, da, 0,    x0,    y + 2, SERVO_BAR_WIDTH / 2);
	glcd_draw_v_line(glcd, da, 0,    x0 + SERVO_BAR_WIDTH / 2, y - 2, 5);
	glcd_draw_pixel(glcd, da,  0,    x0 + SERVO_BAR_WIDTH / 2 - 1, y);
	glcd_draw_v_line(glcd, da, 0xff, x - 1, y0, 3);

	glcd_draw_v_line(glcd, da, 0xff, x0 - 1, y - 4, 9);
	glcd_draw_pixel(glcd, da,  0,    x0 - 1, y - 5);
	glcd_draw_pixel(glcd, da,  0,    x0 - 1, y + 5);
	glcd_draw_v_line(glcd, da, 0xff, x0 - 2, y - 4, 9);
	glcd_draw_pixel(glcd, da,  0,    x0 - 2, y - 5);
	glcd_draw_pixel(glcd, da,  0,    x0 - 2, y + 5);


	glcd_draw_v_line(glcd, da, 0xff, x0 - 3, y - 3, 7);
	glcd_draw_pixel(glcd, da,  0,    x0 - 3, y - 4);
	glcd_draw_pixel(glcd, da,  0,    x0 - 3, y + 4);

	glcd_draw_v_line(glcd, da, 0,    x0 - 4, y - 3, 7);
	}

static void
display_preset_settings(void)
	{
	MotionFrame	*mf = &motion_frame;
	char		info1[100];

	if (pikrellcam.preset_modified_warning)
		{
		i420_print(&top_status_area, normal_font, 0xff, 0, 1, 0,
				JUSTIFY_LEFT, "Settings changed off preset");
		i420_print(&top_status_area, normal_font, 0xff, 1, 1, 0,
				JUSTIFY_LEFT, "will not be saved unless:");
		i420_print(&top_status_area, normal_font, 0xff, 2, 1, 0,
				JUSTIFY_LEFT, "Setup->Preset->New");
		}

	snprintf(info1, sizeof(info1), "Mag %d  Cnt %d   Burst %d,%d",
				(int) sqrt(mf->mag2_limit), mf->mag2_limit_count,
				pikrellcam.motion_burst_count, pikrellcam.motion_burst_frames);
	i420_print(&bottom_status_area, normal_font, 0xff,
				pikrellcam.have_servos ? 1 : 2,
				-6 * normal_font->char_width, 0,
				JUSTIFY_RIGHT(0), info1);
	}

#define VU_HEIGHT	80

static void
display_audio(void)
	{
	AudioCircularBuffer *acb = &audio_circular_buffer;
	DrawArea			*da = &inform_area;
	char				buf[16];
	int					y0, y, dy;
	static int			x_stream;

	if (!acb->pcm)
		return;

	y0 = da->height - normal_font->char_height - 1;
	snprintf(buf, sizeof(buf), "%ddB", pikrellcam.audio_gain_dB);
	i420_draw_string(da, normal_font, 0xff, 1, y0, buf);

	dy = acb->vu_meter0 * VU_HEIGHT / INT16_MAX;
	acb->vu_meter0 = 0;
	y = y0 - dy - 1;
	if (dy > 0)
		{
		glcd_draw_v_line(glcd, da, 0xc0, 3, y, dy);
		glcd_draw_v_line(glcd, da, 0xff, 4, y, dy);
		glcd_draw_v_line(glcd, da, 0xff, 5, y, dy);
		glcd_draw_v_line(glcd, da, 0xc0, 6, y, dy);
		}
	glcd_draw_h_line(glcd, da, 0, 3, y, 4);
	glcd_draw_rectangle(glcd, da, 0xff, 1,
				y0 - (VU_HEIGHT + 2), 8, VU_HEIGHT + 2);
	glcd_draw_rectangle(glcd, da, 0, 2,
				y0 - (VU_HEIGHT + 1), 6, VU_HEIGHT);

	dy = acb->vu_meter1 * VU_HEIGHT / INT16_MAX;
	acb->vu_meter1 = 0;
	if (acb->channels == 2)
		{
		y = y0 - dy - 1;
		if (dy > 0)
			{
			glcd_draw_v_line(glcd, da, 0xc0, 10, y, dy);
			glcd_draw_v_line(glcd, da, 0xff, 11, y, dy);
			glcd_draw_v_line(glcd, da, 0xff, 12, y, dy);
			glcd_draw_v_line(glcd, da, 0xc0, 13, y, dy);
			}
		glcd_draw_h_line(glcd, da, 0, 10, y, 4);
		glcd_draw_rectangle(glcd, da, 0xff, 8,
					y0 - (VU_HEIGHT + 2), 8, VU_HEIGHT + 2);
		glcd_draw_rectangle(glcd, da, 0, 9,
					y0 - (VU_HEIGHT + 1), 6, VU_HEIGHT);
		}
	if (pikrellcam.audio_trigger_video)
		{
		dy = pikrellcam.audio_trigger_level * VU_HEIGHT / 100;
		glcd_draw_h_line(glcd, da, 0, 3, y0 - dy,
					acb->channels == 2 ? 11 : 4);
		glcd_draw_h_line(glcd, da, 0xff, 3, y0 - dy - 1,
					acb->channels == 2 ? 11 : 4);
		}

	if (acb->mp3_stream_fd > 0)
		{
		glcd_draw_h_line(glcd, da, 0xff, x_stream, da->height - 3, 5);
		glcd_draw_h_line(glcd, da, 0xff, x_stream, da->height - 2, 5);
		glcd_draw_h_line(glcd, da, 0xff, x_stream, da->height - 1, 5);
		x_stream = (x_stream + 2) % (4 * normal_font->char_width - 5);
		}
	}

static void
inform_draw(void)
	{
	int i;

	inform_shown = FALSE;
	for (i = 0; i < N_INFORM_LINES; ++i)
		{
		if (!inform_line[i].string)
			continue;
		inform_shown = TRUE;
		i420_print(&inform_area, inform_line[i].font, 0xff,
				inform_line[i].row, inform_line[i].xs, inform_line[i].ys,
				inform_line[i].justify, inform_line[i].string);
		}
	}

static void
motion_draw(uint8_t *i420)
	{
	GlcdFont        *font;
	VideoCircularBuffer *vcb = &video_circular_buffer;
	DrawArea        *da;
	MotionFrame     *mf = &motion_frame;
	MotionRegion    *mreg;
	CompositeVector *vec;
	SList           *mrlist;
	char            *msg, *fifo_msg, info[100], status[100];
	int16_t         color;			/* just B&W */
	int             i, x, y, dx, dy, r, r_unit;
	int             t_record, t_hold;

	if (!glcd)
		return;
	da = draw_area;

	if (mf->show_vectors)
		i420_dim_frame(i420);

	if (mf->show_vectors)
		{
		i = pikrellcam.have_servos ? 0 : 1;
		x = pikrellcam.have_servos ? -12 : 0;
		i420_print(&bottom_status_area, normal_font, 0xff, i, x, 0,
					JUSTIFY_RIGHT(0), "Vectors: ON");
		}

	fifo_msg = "";
	if (!inform_shown && (mf->show_preset || pikrellcam.preset_notify))
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

			if (   pikrellcam.on_preset && !pikrellcam.servo_moving
			    && mreg->vector.mag2_count >= mf->mag2_limit_count
				&& !pikrellcam.preset_notify
			   )
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
				if (mf->motion_status & MOTION_BURST)
					{
					vec = &mf->frame_vector;
					x  = MOTION_VECTOR_TO_MJPEG_X(vec->x);
					y  = MOTION_VECTOR_TO_MJPEG_Y(vec->y);
					dx = -vec->vx * r_unit;
					dy = -vec->vy * r_unit;
					glcd_draw_line(glcd, da, color, x, y, x + dx, y + dy);
					glcd_fill_circle(glcd, da, color, x + dx, y + dy, 12);
					r = (int) sqrt(vec->mag2_count) * r_unit;
					i = MOTION_VECTOR_TO_MJPEG_Y(mf->height);
					if (r > i / 2)
						r = i / 2;
					glcd_draw_circle(glcd, da, color, x, y, r);
					}
				}
			}
		display_preset_settings();

		if (pikrellcam.on_preset && !pikrellcam.preset_notify)
			{
			if (pikrellcam.motion_detects_fifo_enable)
				fifo_msg = "Motion FIFO ON   ";

			if (mf->frame_window > 0 && mf->motion_status == MOTION_NONE)
				{
				snprintf(status, sizeof(status), "confirming[%d]", mf->frame_window);
				msg = status;
				}
			else if (mf->motion_status & MOTION_DETECTED)
				{
				if (mf->motion_status & MOTION_BURST)
					msg = "burst motion";
				else if (mf->motion_status & MOTION_FIFO)
					msg = "ext motion";
				else
					msg = "motion";
				}
			else if (mf->frame_vector.mag2_count > 0)
				msg = "counts";
			else if (mf->sparkle_count > 0 || mf->reject_count > 0)
				msg = "noise";
			else
				msg = "quiet";
			if (mf->frame_vector.mag2_count > 0)
				snprintf(status, sizeof(status), "%s:%-3d", msg, mf->frame_vector.mag2_count);
			else
				snprintf(status, sizeof(status), "%s", msg);

			if (pikrellcam.motion_show_counts)
				{
				snprintf(info, sizeof(info),
					"%sany:%-3d(%.1f) rej:%-3d spkl:%-3d(%.1f)  %s",
					fifo_msg, mf->any_count, mf->any_count_expma,
					mf->reject_count, mf->sparkle_count, mf->sparkle_expma,
					status);
				i420_print(&bottom_status_area, normal_font, 0xff, 0, 0, 0,
					JUSTIFY_LEFT, info);
				}
			else
				{
				snprintf(info, sizeof(info), "%s%s", fifo_msg, status);
				i420_print(&bottom_status_area, normal_font, 0xff, 0, 0, 0,
					JUSTIFY_LEFT, info);
				}
			}
		}
	else
		mf->selected_region = -1;

	t_record = vcb->record_elapsed_time;

	if (vcb->state & VCB_STATE_LOOP_RECORD)
		{
		msg = "";
		if (vcb->state & VCB_STATE_MOTION_RECORD)
			msg = pikrellcam.external_motion_record_event ?
				(mf->fifo_detects ? mf->fifo_trigger_code : "Audio") : "Motion";
		t_record = vcb->max_record_time - t_record;
		if (t_record < 0)
			t_record = 0;
		snprintf(info, sizeof(info), "REC (Loop %s) %d:%02d",
					msg, t_record / 60, t_record % 60);
		}
	else if (   (vcb->state & VCB_STATE_MOTION_RECORD)
	         || pikrellcam.motion_stills_record
	        )
		{
		if (pikrellcam.t_now > pikrellcam.motion_sync_time)
			t_record -= pikrellcam.t_now - pikrellcam.motion_sync_time;
		if (mf->fifo_trigger_time_limit > 0)
			{
			t_hold = vcb->max_record_time -
					(pikrellcam.t_now - vcb->record_start_time);
			snprintf(info, sizeof(info), "REC (Extern) %d:%02d  end %d:%02d",
						t_record / 60, t_record % 60,
						t_hold / 60, t_hold % 60);
			}
		else
			{
			t_hold = pikrellcam.motion_times.event_gap -
					(pikrellcam.t_now - pikrellcam.motion_last_detect_time);
			if (pikrellcam.motion_stills_record)
				snprintf(info, sizeof(info), "REC (%s) Stills: %d  hold %d:%02d",
					pikrellcam.external_motion_record_event ?
						(mf->fifo_detects ? mf->fifo_trigger_code : "Audio")
									: "Motion",
					pikrellcam.motion_stills_count,
					t_hold / 60, t_hold % 60);
			else
				snprintf(info, sizeof(info), "REC (%s) %d:%02d  hold %d:%02d",
					pikrellcam.external_motion_record_event ?
						(mf->fifo_detects ? mf->fifo_trigger_code : "Audio")
									: "Motion",
					t_record / 60, t_record % 60,
					t_hold / 60, t_hold % 60);
			}
		}
	else if (vcb->state == VCB_STATE_MANUAL_RECORD)
		snprintf(info, sizeof(info), "REC (%s) %d:%02d",
					vcb->pause ? "Pause" : "Manual",
					t_record / 60, t_record % 60);
	else
		snprintf(info, sizeof(info), "REC (Stop)");

	x = 1;
	i420_print(&bottom_status_area, normal_font, 0xff, 2, x, 0,
					JUSTIFY_LEFT, info);

	if (mf->motion_enable)
		{
		if (vcb->state == VCB_STATE_MANUAL_RECORD)
			msg = "Motion ---";
		else if (pikrellcam.servo_moving)
			msg = "Motion hold (moving)";
		else if (!pikrellcam.on_preset)
			{
			if (pikrellcam.motion_off_preset)
				msg = "Motion ON (off preset)";
			else
				msg = "Motion hold (off preset)";
			}
		else if (pikrellcam.motion_stills_enable)
			msg = "Motion Stills  ON";
		else
			msg = "Motion  ON";
		}
	else
		{
		if (pikrellcam.on_preset)
			msg = "Motion OFF";
		else
			msg = "Motion OFF (off preset)";
		}
	i420_print(&bottom_status_area, normal_font, 0xff, 1, 1, 0,
					JUSTIFY_LEFT, msg);

	if (pikrellcam.motion_detects_fifo_enable && !*fifo_msg)
		i420_print(&bottom_status_area, normal_font, 0xff, 0, 1, 0,
						JUSTIFY_LEFT, "Motion FIFO ON");

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
			snprintf(tbuf, sizeof(tbuf), "   Period:  %-3d ", p);
			}
		else if (time_lapse.on_hold)
			{
			snprintf(info, sizeof(info), "Timelapse:  hold ");
			snprintf(tbuf, sizeof(tbuf), "   Period:  %-4d ", p);
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
		snprintf(info, sizeof(info), "Converting: %s.mp4 %.3fM ",
				time_lapse.convert_name,
				(float) time_lapse.convert_size / 1000000.0);
		i420_print(&top_status_area, normal_font, 0xff, 2, 0, 0,
					JUSTIFY_RIGHT(0), info);
			}
	if (pikrellcam.video_notify)
		i420_print(&inform_area, normal_font, 0xff, 6, 0, 0, JUSTIFY_CENTER,
					fname_base(pikrellcam.video_last));
	if (pikrellcam.still_notify)
		i420_print(&inform_area, normal_font, 0xff, 5, 0, 0, JUSTIFY_CENTER,
					fname_base(pikrellcam.still_last));
	if (pikrellcam.timelapse_notify && time_lapse.show_status)
		i420_print(&inform_area, normal_font, 0xff, 4, 0, 0, JUSTIFY_CENTER,
					fname_base(pikrellcam.timelapse_jpeg_last));
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

static SList 	*menu_motion_settings_list;
static int		menu_motion_settings_index;

static SList 	*menu_motion_limit_list;
static int		menu_motion_limit_index;

static SList 	*menu_settings_list;
static int		menu_settings_index;

static SList 	*menu_servo_settings_list;
static int		menu_servo_settings_index;

static SList 	*menu_loop_settings_list;
static int		menu_loop_settings_index;

static SList 	*menu_audio_settings_list;
static int		menu_audio_settings_index;


static char	*video_presets_entry[] =
	{
	"1080p",
	"720p",			// V2 1280x720 / v1 1296x730
	"1640x1232",	// V2 4:3
	"1640x922",		// V2 16:9
	"1296x972",		// V1 4:3
	"1024x768",
	"1024x576"
	};

#define N_VIDEO_PRESET_ENTRIES \
		(sizeof(video_presets_entry) / sizeof(char *))

static char	*still_presets_entry[] =
	{
	"1920x1080",
	"1280x720",
	"3280x2464",	// v2
	"2592x1944",
	"2592x1458",
	"1920x1440",
	"1640x1232",	// V2 4:3
	"1640x922",		// V2 16:9
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
//	"whiteboard",
//	"blackboard",
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
//	"posterise",
//	"colorpoint",
//	"colourbalance",
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
	{ "hflip",      0, 1, 1, 0, 0, 0, "", NULL, NULL },
	{ "vflip",      0, 1, 1, 0, 0, 0, "", NULL, NULL },
	{ "shutter_speed", 0, 6000000, 100, 0, 0, 0, "usec", NULL, NULL }
	};

#define N_PICTURE_ADJUSTMENTS \
		(sizeof(picture_adjustment) / sizeof(Adjustment))


  /* Adjustment changes made to a temp struct to avoid thrashing malloc/free
  |  of huge circular buffer.  Final change is applied if/when SEL is clicked.
  */
Adjustment	motion_settings_adjustment[] =
	{
	{ "Startup_Motion",   0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.startup_motion_enable },
	{ "Confirm_Gap",  0, 30,  1, 0, 0, 0, "", NULL, &motion_times_temp.confirm_gap },
	{ "Pre_Capture",  1, 180, 1, 0, 0, 0, "", NULL, &motion_times_temp.pre_capture },
	{ "Event_Gap",    1, 300, 1, 0, 0, 0, "", NULL, &motion_times_temp.event_gap },
	{ "Post_Capture", 1, 180, 1, 0, 0, 0, "", NULL, &motion_times_temp.post_capture },
	{ "Video_Time_Limit",   0, 1800, 10, 0, 0, 0, "sec", NULL, &pikrellcam.motion_record_time_limit },
	{ "Motion_Stills_(no_videos)",   0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.motion_stills_enable },
	{ "Max_Stills_per_Minute", 1, 60, 1, 0, 0, 0, "", NULL, &pikrellcam.motion_stills_per_minute}
	};

#define N_MOTION_SETTINGS_ADJUSTMENTS \
		(sizeof(motion_settings_adjustment) / sizeof(Adjustment))

Adjustment	motion_limit_adjustment[] =
	{
	{ "Vector_Magnitude",  2, 50, 1, 0, 0, 0, "", NULL, &pikrellcam.motion_magnitude_limit },
	{ "Vector_Count",      2, 50, 1, 0, 0, 0, "", NULL, &pikrellcam.motion_magnitude_limit_count },
	{ "Burst_Count",      50, 2000, 10, 0, 0, 0, "", NULL, &pikrellcam.motion_burst_count },
	{ "Burst_Frames",      2, 20, 1, 0, 0, 0, "", NULL, &pikrellcam.motion_burst_frames }
	};

#define N_MOTION_LIMIT_ADJUSTMENTS \
		(sizeof(motion_limit_adjustment) / sizeof(Adjustment))


  /* Some settings djustment changes are made to a temp struct to avoid thrashing
  |  camera destroys/creates.  Final change is applied if/when SEL is clicked.
  */
Adjustment	settings_adjustment[] =
	{
//	{ "Vertical_Filter",  0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.motion_vertical_filter },
	{ "Check_Media_Diskfree",   0, 1,  1, 0, 0, 0, "", NULL, &pikrellcam.check_media_diskfree },
	{ "Check_Archive_Diskfree", 0, 1,  1, 0, 0, 0, "", NULL, &pikrellcam.check_archive_diskfree },
	{ "Diskfree_Percent",  5,  90, 1, 0, 0, 0, "", NULL, &pikrellcam.diskfree_percent },
	{ "video_bitrate",   1000000, 25000000, 100000, 0, 0, 0, "", NULL, &camera_adjust_temp.video_bitrate },
	{ "video_fps",       1,    30,    1,   0, 0, 0, "", NULL, &camera_adjust_temp.video_fps },
	{ "video_mp4box_fps",  0,    30,    1,   0, 0, 0, "", NULL, &camera_adjust_temp.video_mp4box_fps },
	{ "mjpeg_divider",  1,  12,   1,   0, 0, 0, "", NULL, &pikrellcam.mjpeg_divider },
	{ "still_quality",  5,    100,   1,   0, 0, 0, "", NULL, &camera_adjust_temp.still_quality },
	{ "Vector_Counts",  0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.motion_show_counts },
	{ "Vector_Dimming", 30, 60, 1, 0, 0, 0, "", NULL, &pikrellcam.motion_vectors_dimming },
	{ "Preview_Clean",  0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.motion_preview_clean },
	};


#define VECTOR_DIMMING_INDEX	1  /* Must track above Vector_Dimming entry */
#define N_SETTINGS_ADJUSTMENTS \
		(sizeof(settings_adjustment) / sizeof(Adjustment))

Adjustment	loop_settings_adjustment[] =
	{
	{ "Startup_Loop",     0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.loop_startup_enable },
	{ "Time_Limit",   10, 900, 10, 0, 0, 0, "sec", NULL, &pikrellcam.loop_record_time_limit },
	{ "Diskusage_Percent", 5, 95, 1,   0, 0, 0, "", NULL, &pikrellcam.loop_diskusage_percent }
	};

#define N_LOOP_SETTINGS_ADJUSTMENTS \
		(sizeof(loop_settings_adjustment) / sizeof(Adjustment))

Adjustment	audio_settings_adjustment[] =
	{
	{ "Audio_Trigger_Video", 0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.audio_trigger_video },
	{ "Audio_Trigger_Level", 2, 100, 1, 0, 0, 0, "", NULL, &pikrellcam.audio_trigger_level },
	{ "Box_MP3_Only",  0, 1, 1, 0, 0, 0, "", NULL, &pikrellcam.audio_box_MP3_only }
	};

#define N_AUDIO_SETTINGS_ADJUSTMENTS \
		(sizeof(audio_settings_adjustment) / sizeof(Adjustment))

Adjustment	servo_settings_adjustment[] =
	{
	{ "Motion_Off_Preset",   0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.motion_off_preset },
	{ "Move_Step_msec",    0,   200,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_move_step_msec },
	{ "Preset_Step_msec",  0,   100,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_preset_step_msec },
	{ "Servo_Settle_msec", 200,  1000,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_settle_msec },
	{ "Move_Steps",        5,    30,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_move_steps },
	{ "Pan_Left_Limit",    SERVO_MIN_WIDTH,  SERVO_MAX_WIDTH,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_pan_min },
	{ "Pan_Right_Limit",   SERVO_MIN_WIDTH,  SERVO_MAX_WIDTH,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_pan_max },
	{ "Tilt_Up_Limit",     SERVO_MIN_WIDTH,  SERVO_MAX_WIDTH,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_tilt_max },
	{ "Tilt_Down_Limit",   SERVO_MIN_WIDTH,  SERVO_MAX_WIDTH,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_tilt_min },
	{ "Servo_Pan_Invert",   0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_pan_invert },
	{ "Servo_Tilt_Invert",  0,  1,  1, 0, 0, 0, "", NULL, &pikrellcam.servo_tilt_invert }
	};

#define PAN_LEFT_LIMIT_INDEX	5
#define PAN_RIGHT_LIMIT_INDEX	6
#define TILT_UP_LIMIT_INDEX		7
#define TILT_DOWN_LIMIT_INDEX	8

#define N_SERVO_SETTINGS_ADJUSTMENTS \
		(sizeof(servo_settings_adjustment) / sizeof(Adjustment))


static Adjustment	*cur_adj,
					*adjustments;
static boolean		cur_adj_start;


static boolean
adjustment_is_servo_limits(void)
	{
	if (   display_state == DISPLAY_ADJUSTMENT
	    && pikrellcam.have_servos
	    && adjustments == &servo_settings_adjustment[0]
	    && *display_menu_index >= PAN_LEFT_LIMIT_INDEX
	    && *display_menu_index <= TILT_DOWN_LIMIT_INDEX
	   )
		return TRUE;
	return FALSE;
	}

static boolean
adjustment_is_servo_pan_limits(void)
	{
	if (   adjustments == &servo_settings_adjustment[0]
	    && pikrellcam.have_servos
	    && (   *display_menu_index == PAN_LEFT_LIMIT_INDEX
	        || *display_menu_index == PAN_RIGHT_LIMIT_INDEX
	       )
	   )
		return TRUE;
	return FALSE;
	}

static boolean
adjustment_is_servo_tilt_limits(void)
	{
	if (   adjustments == &servo_settings_adjustment[0]
	    && pikrellcam.have_servos
	    && (   *display_menu_index == TILT_DOWN_LIMIT_INDEX
	        || *display_menu_index == TILT_UP_LIMIT_INDEX
	       )
	   )
		return TRUE;
	return FALSE;
	}

  /* Apply adjustments that were not done live.
  */
static boolean
apply_adjustment(void)
	{
	VideoCircularBuffer	*vcb = &video_circular_buffer;
	PresetPosition		*pos;
	PresetSettings		*settings = NULL;
	boolean				config_modified = TRUE;

	if (!adjustments || !cur_adj)
		return FALSE;
	if (adjustments == &motion_settings_adjustment[0])
		{
		pikrellcam.motion_times.confirm_gap = motion_times_temp.confirm_gap;
		pikrellcam.motion_times.post_capture = motion_times_temp.post_capture;
		if (pikrellcam.motion_times.post_capture > motion_times_temp.event_gap)
			motion_times_temp.event_gap = pikrellcam.motion_times.post_capture;

		if (   motion_times_temp.pre_capture != pikrellcam.motion_times.pre_capture
		    || motion_times_temp.event_gap != pikrellcam.motion_times.event_gap
		   )
			{
			pthread_mutex_lock(&vcb->mutex);
			if (vcb->state == VCB_STATE_NONE)
				{
				pikrellcam.motion_times.pre_capture = motion_times_temp.pre_capture;
				pikrellcam.motion_times.event_gap = motion_times_temp.event_gap;
				video_circular_buffer_init();
				audio_circular_buffer_init();
				}
			else
				{
				display_inform("\"Cannot change pre_capture or event_gap\" 3 3 1");
				display_inform("\"while video is recording.\" 4 3 1");
				display_inform("timeout 2");
				}
			pthread_mutex_unlock(&vcb->mutex);
			}
		if (pikrellcam.motion_stills_enable && pikrellcam.loop_enable)
			{
			pikrellcam.motion_stills_enable = FALSE;
			display_inform("\"Cannot enable motion stills\" 3 3 1");
			display_inform("\"while loop videos are enabled.\" 4 3 1");
			display_inform("timeout 3");
			}
		}
	else if (adjustments == &motion_limit_adjustment[0])
		{
		if (pikrellcam.on_preset)
			{
			pos = (PresetPosition *) slist_nth_data(pikrellcam.preset_position_list,
							pikrellcam.preset_position_index);
			settings = (PresetSettings *) slist_nth_data(pos->settings_list,
							pos->settings_index);
			settings->mag_limit = pikrellcam.motion_magnitude_limit;
			settings->mag_limit_count = pikrellcam.motion_magnitude_limit_count;
			settings->burst_count = pikrellcam.motion_burst_count;
			settings->burst_frames = pikrellcam.motion_burst_frames;
			pikrellcam.preset_modified = TRUE;
			}
		else
			pikrellcam.preset_modified_warning = TRUE;
		config_modified = FALSE;
		}
	else if (adjustments == &settings_adjustment[0])
		{
		if (   camera_adjust_temp.video_fps != pikrellcam.camera_adjust.video_fps
		    || camera_adjust_temp.still_quality != pikrellcam.camera_adjust.still_quality
		    || camera_adjust_temp.video_bitrate != pikrellcam.camera_adjust.video_bitrate
		   )
			camera_restart();
		else	/* Pick up changes not requiring new buffer or camera restart */
			pikrellcam.camera_adjust = camera_adjust_temp;

		/* All other adjustments have been done live. */
		}
	else if (adjustment_is_servo_limits())
		{
		if (pan_save < pikrellcam.servo_pan_min)
			pan_save = pikrellcam.servo_pan_min;
		if (pan_save > pikrellcam.servo_pan_max)
			pan_save = pikrellcam.servo_pan_max;
		if (tilt_save < pikrellcam.servo_tilt_min)
			tilt_save = pikrellcam.servo_tilt_min;
		if (tilt_save > pikrellcam.servo_tilt_max)
			tilt_save = pikrellcam.servo_tilt_max;
		servo_move(pan_save, tilt_save, pikrellcam.servo_move_step_msec);
		}
	return config_modified;
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
	if (adjustment_is_servo_limits())
		servo_move(pan_save, tilt_save, pikrellcam.servo_move_step_msec);
	}

static void
display_adjustment(uint8_t *i420)
	{
	DrawArea	*da = &adj_control_area;
	GlcdFont	*font = normal_font;
	char		buf[50];
	int			bar_x0, bar_y0, bar_dy, bar_width, adj_x, fast_factor;
	int			pan, tilt, max, min;
	static int	prev_action;
	boolean		boolean_flag = FALSE;

	if (adjustments == &motion_limit_adjustment[0] && !pikrellcam.on_preset)
		pikrellcam.preset_modified_warning = TRUE;
	else
		pikrellcam.preset_modified_warning = FALSE;

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
			cur_adj->value = atoi(cur_adj->cam_param->arg);
			}
		else if (adjustment_is_servo_limits())
			{
			servo_get_position(&pan_save, &tilt_save);
			if (*display_menu_index == PAN_LEFT_LIMIT_INDEX)
				{
				cur_adj->value = pikrellcam.servo_pan_min;
				servo_move(pikrellcam.servo_pan_min, tilt_save, pikrellcam.servo_move_step_msec);
				}
			else if (*display_menu_index == PAN_RIGHT_LIMIT_INDEX)
				{
				cur_adj->value = pikrellcam.servo_pan_max;
				servo_move(pikrellcam.servo_pan_max, tilt_save, pikrellcam.servo_move_step_msec);
				}
			else if (*display_menu_index == TILT_UP_LIMIT_INDEX)
				{
				cur_adj->value = pikrellcam.servo_tilt_max;
				servo_move(pan_save, pikrellcam.servo_tilt_max, pikrellcam.servo_move_step_msec);
				}
			else if (*display_menu_index == TILT_DOWN_LIMIT_INDEX)
				{
				cur_adj->value = pikrellcam.servo_tilt_min;
				servo_move(pan_save, pikrellcam.servo_tilt_min, pikrellcam.servo_move_step_msec);
				}
			cur_adj->prev_value = cur_adj->value;
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
	else if (display_action == REPEAT_LEFT_ARROW && prev_action == REPEAT_LEFT_ARROW)
		display_action = ACTION_NONE;
	else if (display_action == REPEAT_RIGHT_ARROW && prev_action == REPEAT_RIGHT_ARROW)
		display_action = ACTION_NONE;

	/* Set cur adjust to config value if it changed while adjusting.
	|  For if a FIFO command sets it while we are adjusting.
	*/
#if 0
	if (adjustments == &picture_adjustment[0])
		{
		if (atoi(cur_adj->cam_param->arg) != cur_adj->revert_value)
			{
			cur_adj->value = atoi(cur_adj->cam_param->arg);
			cur_adj->revert_value = cur_adj->value;
			}
		}

	if (adjustments != &picture_adjustment[0])
		{
		if (*(cur_adj->config_value) != cur_adj->revert_value)
			{
			cur_adj->value = *(cur_adj->config_value);
			cur_adj->revert_value = cur_adj->value;
			}
		}
#endif

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

		if (adjustment_is_servo_pan_limits())
			{
			preset_pan_range(&max, &min);
			if (cur_adj->value <= min || cur_adj->value >= max)
				{
				servo_get_position(NULL, &tilt);
				servo_move(cur_adj->value, tilt, pikrellcam.servo_move_step_msec);
				}
			else
				{
				cur_adj->value = cur_adj->prev_value;
				display_inform("\"Cannot move limit past a preset.\" 5 3 1");
				display_inform("timeout 2");
				}
//printf("max:%d min:%d cur:%d prev:%d tilt:%d\n",
//max, min, cur_adj->value, cur_adj->prev_value, tilt);
			}
		else if (adjustment_is_servo_tilt_limits())
			{
			preset_tilt_range(&max, &min);
			if (cur_adj->value <= min || cur_adj->value >= max)
				{
				servo_get_position(&pan, &tilt);
				servo_move(pan, cur_adj->value, pikrellcam.servo_move_step_msec);
				}
			else
				{
				cur_adj->value = cur_adj->prev_value;
				display_inform("\"Cannot move limit past a preset.\" 5 3 1");
				display_inform("timeout 2");
				}
			}
		}
	cur_adj->prev_value = cur_adj->value;

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

	if (   adjustments == &settings_adjustment[0]
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
	else if (   adjustments == &motion_settings_adjustment[0]
	         && cur_adj->value == 0
	        )
		snprintf(buf, sizeof(buf), "OFF");
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
			case MOTION_LIMIT:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &motion_limit_adjustment[0];
				cur_adj_start = TRUE;
				break;
			case MOTION_SETTINGS:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &motion_settings_adjustment[0];
				cur_adj_start = TRUE;
				break;
			case SETTINGS:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &settings_adjustment[0];
				cur_adj_start = TRUE;
				break;
			case LOOP_SETTINGS:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &loop_settings_adjustment[0];
				cur_adj_start = TRUE;
				break;
			case AUDIO_SETTINGS:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &audio_settings_adjustment[0];
				cur_adj_start = TRUE;
				break;
			case SERVO_SETTINGS:
				display_state = DISPLAY_ADJUSTMENT;
				adjustments = &servo_settings_adjustment[0];
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

  /* This runs in an Arm thread
  */
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
				pikrellcam.config_modified = apply_adjustment();
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
					camera_restart();
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
					camera_restart();
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
				{
				display_state = DISPLAY_DEFAULT;
				display_menu = DISPLAY_MENU_NONE;
				}
			break;
		case MOTION_SETTINGS:
			display_menu_list = menu_motion_settings_list;
			display_menu_index = &menu_motion_settings_index;
			display_state = DISPLAY_MENU;
			display_menu = MOTION_SETTINGS;
			break;
		case MOTION_LIMIT:
			display_menu_list = menu_motion_limit_list;
			display_menu_index = &menu_motion_limit_index;
			display_state = DISPLAY_MENU;
			display_menu = MOTION_LIMIT;
			break;
		case SETTINGS:
			display_menu_list = menu_settings_list;
			display_menu_index = &menu_settings_index;
			display_state = DISPLAY_MENU;
			display_menu = SETTINGS;
			break;
		case LOOP_SETTINGS:
			display_menu_list = menu_loop_settings_list;
			display_menu_index = &menu_loop_settings_index;
			display_state = DISPLAY_MENU;
			display_menu = LOOP_SETTINGS;
			break;
		case AUDIO_SETTINGS:
			display_menu_list = menu_audio_settings_list;
			display_menu_index = &menu_audio_settings_index;
			display_state = DISPLAY_MENU;
			display_menu = AUDIO_SETTINGS;
			break;
		case SERVO_SETTINGS:
			display_menu_list = menu_servo_settings_list;
			display_menu_index = &menu_servo_settings_index;
			display_state = DISPLAY_MENU;
			display_menu = SERVO_SETTINGS;
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

static void
display_inform_expire(void)
	{
	int		i;

	for (i = 0; i < N_INFORM_LINES; ++i)
		{
		if (!inform_line[i].string)
			continue;
		free(inform_line[i].string);
		inform_line[i].string = NULL;
		}
	inform_line_index = 0;
	}

void
display_inform(char *args)
	{
	InformLine *iline;
	char       str[128];
	int        i, n = 0, font = 0, row = 0, xs = 0, ys = 0, justify = 4;

	if (sscanf(args, "timeout %d\n", &n) == 1)
		{
		n = (n < 0 || n > 30) ? pikrellcam.notify_duration : n;
		if (n == 0)
			{
			event_remove_name("display inform expire");
			display_inform_expire();
			}
		else
			event_count_down_add("display inform expire",
					n * EVENT_LOOP_FREQUENCY, display_inform_expire, NULL);
		}
	else if (sscanf(args, "clear %d\n", &n) == 1)
		{
		for (i = 0; i < N_INFORM_LINES; ++i)
			{
			if (inform_line[i].row == n && inform_line[i].string)
				{
				free(inform_line[i].string);
				inform_line[i].string = NULL;
				}
			}
		}
	else if (inform_line_index < N_INFORM_LINES)
		{
		n = sscanf(args, "\"%127[^\"]\" %d %d %d %d %d",
				str, &row, &justify, &font, &xs, &ys);
		if (n > 0 && row >= 0 && inform_line_index < N_INFORM_LINES)
			{
			iline = &inform_line[inform_line_index++];
			dup_string(&iline->string, str);
			iline->row = row;
			iline->justify = justify;
			iline->font = (font == 0) ? normal_font : large_font;
			iline->xs = xs;
			iline->ys = ys;
			}
		}
	}

void
display_inform_clear(void)
	{
	if (event_remove_name("display inform expire"))
		display_inform_expire();
	}

static int quit_flag;

  /* Called from I420_video_callback() so this runs in a GPU thread
  */
void
display_draw(uint8_t *i420)
	{
	static int	clean_count;

	/* If this frame will be a preview save jpeg and the user wants it clean,
	|  do not draw.
	*/
	if (pikrellcam.do_preview_save && pikrellcam.motion_preview_clean)
		clean_count = 2;	/* Failsafe maybe.  Try hard to get a clean. */

	if (clean_count > 0)
		{
		--clean_count;
		return;
		}

	glcd_set_frame_buffer(glcd, (uint16_t *) i420,
				pikrellcam.mjpeg_width, pikrellcam.mjpeg_height);

	if (display_state != DISPLAY_QUIT)
		inform_draw();

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
	if (display_menu == MOTION_LIMIT)
		display_preset_settings();

	if (pikrellcam.have_servos)
		{
		display_servo_pan();
		display_servo_tilt();
		}
	display_audio();
	display_preset_setting();

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
	display_menu = DISPLAY_MENU_NONE;
	}

void
display_quit(void)
	{
	int	i;

	display_state = DISPLAY_QUIT;
	display_menu = DISPLAY_MENU_NONE;
	quit_flag = TRUE;
	for (i = 0; i < 10; ++i)
		{
		if (!quit_flag)
			break;
		else
			usleep(100000);
		}
	usleep(200000);
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

	/* split a bottom_status_area off the screen draw area for three lines
	|  of status text
	*/
	glcd_area_v_split(draw_area, &da, &bottom_status_area,
			3 * normal_font->char_height + 2, (SPLIT_PIXELS | SPLIT_2ND), 0);

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
	if (!menu_motion_settings_list)
		{
		for (i = 0, position = 0; i < N_MOTION_SETTINGS_ADJUSTMENTS; ++i)
			{
			adj = &motion_settings_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_motion_settings_list = slist_append(menu_motion_settings_list, entry);
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
	if (!menu_settings_list)
		{
		for (i = 0, position = 0; i < N_SETTINGS_ADJUSTMENTS; ++i)
			{
			adj = &settings_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_settings_list = slist_append(menu_settings_list, entry);
			}
		}
	if (!menu_loop_settings_list)
		{
		for (i = 0, position = 0; i < N_LOOP_SETTINGS_ADJUSTMENTS; ++i)
			{
			adj = &loop_settings_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_loop_settings_list = slist_append(menu_loop_settings_list, entry);
			}
		}
	if (!menu_audio_settings_list)
		{
		for (i = 0, position = 0; i < N_AUDIO_SETTINGS_ADJUSTMENTS; ++i)
			{
			adj = &audio_settings_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_audio_settings_list = slist_append(menu_audio_settings_list, entry);
			}
		}
	if (!menu_servo_settings_list)
		{
		for (i = 0, position = 0; i < N_SERVO_SETTINGS_ADJUSTMENTS; ++i)
			{
			adj = &servo_settings_adjustment[i];
			entry = calloc(1, sizeof(MenuEntry));
			entry->name = adj->name;
			entry->length = strlen(entry->name);
			entry->line_position = position;
			position += entry->length + 1;
			menu_servo_settings_list = slist_append(menu_servo_settings_list, entry);
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
