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

#include <inttypes.h>
#include <unistd.h>

#include "glcd-widgets.h"
#include "utils.h"


extern SList	*glcd_widget_list;


/* ====================================================== */
/* make static after debug */
GlcdEventTouch	event_touch;

void
glcd_event_handler(Glcd *glcd)
	{
	TouchCalibratePoint	display, raw;

	if (glcd_touch_data_available(glcd))
		{
		glcd_touch_read_conversion(glcd, TRUE);

		event_touch.x_raw = raw.x = glcd_touch_get_raw_x(glcd);
		event_touch.y_raw = raw.y = glcd_touch_get_raw_y(glcd);
		touch_calibrate_point(&display, &raw, &glcd->touch.calibrate_matrix);
		event_touch.x = display.x;
		event_touch.y = display.y;

//		event_touch.x = glcd_touch_get_display_x(glcd);
//		event_touch.y = glcd_touch_get_display_y(glcd);
//		event_touch.x_raw = glcd_touch_get_raw_x(glcd);
//		event_touch.y_raw = glcd_touch_get_raw_y(glcd);
//		event_touch.time = millis();
		event_touch.valid = TRUE;
		}

	}


GlcdEventTouch *
glcd_event_get_touch(Glcd *glcd)
	{
	GlcdEventTouch	*te = NULL;

	glcd_event_handler(glcd);
	if (event_touch.valid)
		te = &event_touch;
	event_touch.valid = FALSE;
	return te;
	}

void
glcd_event_wait_for_touch_release(Glcd *glcd)
	{
	while (1)
		{
		usleep(10000);
		if (!glcd_touch_data_available(glcd))
			break;
		glcd_touch_read_conversion(glcd, FALSE);
		}
	}


GlcdButton *
glcd_event_touch_in_button(GlcdEventTouch *te)
	{
	SList		*list;
	GlcdWidget	*pWid;
	int			x, y;

	for (list = glcd_widget_list; list; list = list->next)
		{
		pWid = (GlcdWidget *) list->data;
		if (pWid->type != WIDGET_TYPE_BUTTON)
			continue;
		x = pWid->x + pWid->draw_area->x0;
		y = pWid->y + pWid->draw_area->y0;
		if (   te->x >= x && te->x <= x + pWid->dx
		    && te->y >= y && te->y <= y + pWid->dy
		   )
			return (GlcdButton *) pWid;
		}
	return NULL;
	}

GlcdSlider *
glcd_event_touch_in_slider(GlcdEventTouch *te)
	{
	GlcdWidget	*pWid;
	GlcdSlider	*pSld;
	SList		*list;
	int			x, y;

	for (list = glcd_widget_list; list; list = list->next)
		{
		pWid = (GlcdWidget *) list->data;
		if (pWid->type != WIDGET_TYPE_SLIDER)
			continue;
		pSld = (GlcdSlider *) pWid;

		x = pWid->x + pWid->draw_area->x0;
		y = pWid->y + pWid->draw_area->y0;
		if (   te->x >= x && te->x < x + pWid->dx
		    && te->y >= y && te->y < y + pWid->dy
		   )
			{
			te->x -= pWid->draw_area->x0;
			te->y -= pWid->draw_area->y0;
			return pSld;
			}
		}
	return NULL;
	}


boolean
glcd_event_check(Glcd *glcd)
	{
	GlcdEventTouch	*pEv;
	GlcdWidget		*pWid;
	GlcdButton		*pBut;
	GlcdSlider		*pSld;
	GlcdRegion		*pBR;
	int				dv, dt, value;

	if ((pEv = glcd_event_get_touch(glcd)) == NULL)
		return FALSE;
	if ((pBut = glcd_event_touch_in_button(pEv)) != NULL)
		{
		glcd_button_draw(pBut, BUTTON_DOWN);
		glcd->frame_buffer_update(glcd);
		glcd_event_wait_for_touch_release(glcd);
		glcd_button_draw(pBut, BUTTON_UP);
		glcd->frame_buffer_update(glcd);
		if (pBut->callback)
			(*pBut->callback)(pBut);
		}
	else if ((pSld = glcd_event_touch_in_slider(pEv)) != NULL)
		{
		pWid = (GlcdWidget *) pSld;
		pBR = &pSld->bar_region;

		/* A slider touch event region is the vertical bar region, but the
		|  knob can only be adjusted by the slide_length (bar length minus
		|  the knob length). So, scale touches within this region from
		|  value_min to value_max.
		*/
		if (pWid->flags & WIDGET_FLAG_H_PACK)
			dv = pBR->y + (pSld->knob_length /2) + pSld->slide_length - pEv->y;
		else
			dv = pEv->x - (pBR->x + pSld->knob_length / 2);

		if (dv < 0)
			dv = 0;
		else if (dv > pSld->slide_length)
			dv = pSld->slide_length;

		value = pSld->value_min + (pSld->value_max - pSld->value_min) * dv
										/ pSld->slide_length;

		/* Update low changes in slider value at a slower rate than
		|  higher value changes to try to smooth the slider action.
		*/
		dv = abs(value - pSld->value);
		dt = pEv->time - pSld->time;
		if (    dv > 5					/* full speed slider */
		    || (dv > 3 && dt > 200)		/* 5 updates/sec	*/
		    || (dv > 0 && dt > 333)		/* 3 updates/sec	*/
		   )
			{
			glcd_slider_draw(pSld);
			pSld->value = value;
			pSld->time = pEv->time;
			if (pSld->callback)
				(*pSld->callback)(pSld);
			}
		}
	return TRUE;
	}

#endif  /* WIRINGPI */
