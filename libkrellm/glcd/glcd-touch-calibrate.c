/* libkrellm/glcd
|
|  Copyright (C) 2013-2015 Bill Wilson   billw@gkrellm.net
|
|  libkrellm/glcd is free software: you can redistribute it and/or modify
|  it under the terms of the GNU General Public License as published by
|  the Free Software Foundation, either version 3 of the License, or
|  (at your option) any later version.
|
|  libkrellm/glcd is distributed in the hope that it will be useful,
|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|  GNU General Public License for more details.
|
|  You should have received a copy of the GNU General Public License
|  along with the libkrellm.  If not, see <http://www.gnu.org/licenses/>.
|
*/

#ifdef HAVE_WIRINGPI

#include <stdio.h>
#include <unistd.h>

#include <wiringPi.h>
#include "glcd-widgets.h"


/* Touch screen calibrate functions are based on the public domain code
|  written by by Carlos E. Vidales (copyright (c) 2001).  See:
|
|  http://www.embedded.com/design/system-integration/4023968/How-To-Calibrate-Touch-Screens
|
*/

static void
touch_calibrate_target(GlcdWindow *pW,
			TouchCalibratePoint *raw, TouchCalibratePoint *display,
			int radius, int text_rotation)
	{
	Glcd			*glcd = pW->glcd;
	GlcdEventTouch	*te;

	glcd_window_clear(pW, FALSE);
	glcd_draw_circle(pW->glcd, &pW->body_area, Red,
				display->x, display->y, radius);
	glcd->frame_buffer_update(glcd);
	usleep(500000);

	do
		{
		glcd_touch_read_conversion(glcd, FALSE);
		usleep(10000);
		te = glcd_event_get_touch(glcd);
		}
	while (!te);

	raw->x = te->x_raw;
	raw->y = te->y_raw;
// printf("x_raw = %d  y_raw = %d\n", raw->x, raw->y);
	}

void
touch_calibrate_screen(Glcd *glcd)
	{
	GlcdWindow			*pW;
	DrawArea			*screen_area;
	TouchCalibratePoint	raw[3],
						display[3];
	TouchCalibrateMatrix *matrix;
	int					save_rotation;

	save_rotation = glcd_get_rotation(glcd);
	glcd_set_rotation(glcd, 0);
	pW = glcd_window_new(glcd, NULL, NULL, 0, 0, 0, 0,	/* no title */
				glcd_map_color_percent(18, 18, 28),		/* body bg_color */
				0, 0,									/* no body border */
				0, 0,									/* window size */
				glcd_get_display_width(glcd), glcd_get_display_height(glcd));
	screen_area = &pW->body_area;

	display[0].x = screen_area->width / 10;
	display[0].y = screen_area->height / 10;
	touch_calibrate_target(pW, &raw[0], &display[0], 5, save_rotation);

	display[1].x = screen_area->width -  screen_area->width / 10;
	display[1].y = screen_area->height / 2;
	touch_calibrate_target(pW, &raw[1], &display[1], 5, save_rotation);

	display[2].x = screen_area->width / 2;
	display[2].y = screen_area->height - screen_area->height / 10;
	touch_calibrate_target(pW, &raw[2], &display[2], 5, save_rotation);

	matrix = &glcd->touch.calibrate_matrix;
	touch_set_calibrate_matrix(&display[0], &raw[0], matrix);

	printf("Calibrate Matrix:\n");
	printf( "	An = %d\n"
			"	Bn = %d\n"
			"	Cn = %d\n"
			"	Dn = %d\n"
			"	En = %d\n"
			"	Fn = %d\n"
			"	dividor = %d\n",

		matrix->An, matrix->Bn, matrix->Cn, matrix->Dn, matrix->En, matrix->Fn,
		matrix->divider);
	
	glcd_set_rotation(glcd, save_rotation);
	}

/* 
*/
boolean
touch_set_calibrate_matrix(
		TouchCalibratePoint *display, TouchCalibratePoint *raw,
		TouchCalibrateMatrix *matrix)
	{
	matrix->divider = ((raw[0].x - raw[2].x) * (raw[1].y - raw[2].y)) -
	                     ((raw[1].x - raw[2].x) * (raw[0].y - raw[2].y));

	if (matrix->divider == 0)
		return FALSE;

	matrix->An =
			((display[0].x - display[2].x) * (raw[1].y - raw[2].y)) -
			((display[1].x - display[2].x) * (raw[0].y - raw[2].y));

	matrix->Bn =
			((raw[0].x - raw[2].x) * (display[1].x - display[2].x)) -
			((display[0].x - display[2].x) * (raw[1].x - raw[2].x));

	matrix->Cn =
			(raw[2].x * display[1].x - raw[1].x * display[2].x) * raw[0].y +
			(raw[0].x * display[2].x - raw[2].x * display[0].x) * raw[1].y +
			(raw[1].x * display[0].x - raw[0].x * display[1].x) * raw[2].y;

	matrix->Dn =
			((display[0].y - display[2].y) * (raw[1].y - raw[2].y)) -
			((display[1].y - display[2].y) * (raw[0].y - raw[2].y));

	matrix->En =
			((raw[0].x - raw[2].x) * (display[1].y - display[2].y)) -
			((display[0].y - display[2].y) * (raw[1].x - raw[2].x));

	matrix->Fn =
			(raw[2].x * display[1].y - raw[1].x * display[2].y) * raw[0].y +
			(raw[0].x * display[2].y - raw[2].x * display[0].y) * raw[1].y +
			(raw[1].x * display[0].y - raw[0].x * display[1].y) * raw[2].y;

	return TRUE;
	}


  /* Convert touch screen raw point coordinates into calibrated display
  |  point coordinates using a calibrate matrix.
  */
int
touch_calibrate_point(
			TouchCalibratePoint *display, TouchCalibratePoint *raw,
			TouchCalibrateMatrix *matrix)
	{
	if (matrix->divider == 0)
		return FALSE;

	display->x = ((matrix->An * raw->x) + (matrix->Bn * raw->y) + matrix->Cn)
						/ matrix->divider;

	display->y = ((matrix->Dn * raw->x) + (matrix->En * raw->y) + matrix->Fn)
						/ matrix->divider;

	return TRUE;
	}

#endif /* HAVE_WIRINGPI */
