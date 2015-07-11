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

#include <stdio.h>
#include <inttypes.h>

#include "glcd-widgets.h"
#include "utils.h"


SList	*glcd_widget_list;

uint16_t	display_bg_color = Black;


/* GUI construction using this glcd-widgets library has a few simplifying
|  constraints in order to keep the code base manageably small as appropriate
|  for a microcontroller.
|
|	1) The GlcdWindow pointer passed in to glcd_window_init() should be a
|	pointer to a static window structure so that widgets created inside
|	drawing areas owned by the window can be managed (Ie. old widgets
|	automatically destroyed when glcd_window_init() is called again on
|	the same static window).
|
|	2) A GlcdDrawArea may be successively horizontaly or vertically split to
|	create subdivided drawing areas.  Each area may then have widgets packed
|	in horizontally or vertically regardless of how it was split.  However,
|	within each area you should not have both horizontal and vertical packed
|	widgets.
|
*/


  /* A window is an optional title draw area above a body draw area.
  */
GlcdWindow *
glcd_window_new(Glcd *glcd,
		char *title, GlcdFont *title_font,
		uint16_t title_color, uint16_t title_bg_color,
		uint16_t title_border_color, uint8_t title_border,
		uint16_t body_bg_color, uint16_t body_border_color, uint8_t body_border,
		int	x0, int y0, int width, int height)
	{
	GlcdWindow	*pW = calloc(sizeof(GlcdWindow), 1);
	DrawArea	*dt,
				*da;

	pW->glcd = glcd;
	pW->title = title;
	pW->title_font = title_font;
	pW->title_color = title_color;

	dt = &pW->title_area;
	dt->border = title_border;
	dt->bg_color = title_bg_color;
	dt->border_color = title_border_color;

	da = &pW->body_area;
	da->border = body_border;
	da->bg_color = body_bg_color;
	da->border_color = body_border_color;

	if (title && title_font)
		{
		dt->x0 = x0 + dt->border;
		dt->y0 = y0 + dt->border;
		dt->width = width - 2 * dt->border;
		dt->height = title_font->char_height + 2 * dt->border + 1;
		height -= dt->height;
		y0 += dt->height;
		}
	da->x0 = x0 + da->border;
	da->y0 = y0 + da->border;
	da->width = width - 2 * da->border;
	da->height = height - 2 * da->border;

//	glcd_widget_destroy_all(pW);
	return pW;
	}

/* Clear a draw area to its background color and redraw its border, if any.
*/
void
glcd_area_clear(Glcd *glcd, DrawArea *da)
	{
	DrawArea	*disp_area = glcd_get_display_area(glcd);
	int			i;

	if (!da)
		return;
	glcd_fill_rectangle(glcd, da, da->bg_color, 0, 0, da->width, da->height);
	for (i = 1; i <= da->border; ++i)
		glcd_draw_rectangle(glcd, disp_area, da->border_color,
						da->x0 - i, da->y0 - i,
						da->width + 2 * i, da->height + 2 * i);
	}

/* Clear the body draw area of a window and optionally redraw its title, if any
*/
void
glcd_window_clear(GlcdWindow *pW, boolean draw_title)
	{
	int			dx;
	DrawArea	*dt, *da;

	if (!pW)
		return;

	dt = &pW->title_area;
	da = &pW->body_area;

	if (draw_title && pW->title)
		{
		glcd_area_clear(pW->glcd, dt);
		dx = strlen(pW->title) * pW->title_font->char_width;
		glcd_draw_string(pW->glcd, dt, pW->title_font, pW->title_color,
				(dt->width - dx) / 2, 1, pW->title);
		}
	glcd_area_clear(pW->glcd, da);
	}


void
glcd_window_delete(GlcdWindow *pW)
	{
	DrawArea	*disp_area = glcd_get_display_area(pW->glcd);
	DrawArea	*da;

	da = &pW->title_area;
	glcd_fill_rectangle(pW->glcd, disp_area, display_bg_color,
					-da->border, -da->border,
					da->width + 2 * da->border, da->height + 2 * da->border);
	da = &pW->body_area;
	glcd_fill_rectangle(pW->glcd, disp_area, display_bg_color,
					-da->border, -da->border,
					da->width + 2 * da->border, da->height + 2 * da->border);
	glcd_widget_destroy_all(pW);
	}

  /* Split a drawing area into two areas along the x horizontal axis.
  */
void
glcd_area_h_split(DrawArea *src, DrawArea *dst1, DrawArea *dst2,
				uint8_t amount, uint8_t how, uint8_t border)
	{
	int	dx, dx1, dx2;

	dst1->y0 = dst2->y0 = src->y0 + border;
	dst1->height = dst2->height = src->height - 2 * border;
	dst1->border_color = dst2->border_color = src->border_color;
	dst1->bg_color = dst2->bg_color = src->bg_color;
	dst1->border = dst2->border = border;

	dx = (how & SPLIT_PIXELS) ? amount : src->width * amount / 100;
	dx1 = (how & SPLIT_1ST) ? dx : src->width - dx;
	dx2 = src->width - dx1;

	dst1->x0 = src->x0 + border;
	dst1->width = dx1 - 2 * border;

	dst2->x0 = src->x0 + dx1 + border;
	dst2->width = dx2 - 2 * border;;
	}

  /* Split a drawing area into two areas along the y vertical axis.
  */
void
glcd_area_v_split(DrawArea *src, DrawArea *dst1, DrawArea *dst2,
				uint8_t amount, uint8_t how, uint8_t border)
	{
	int	dy, dy1, dy2;

	dst1->x0 = dst2->x0 = src->x0 + border;
	dst1->width = dst2->width = src->width - 2 * border;
	dst1->border_color = dst2->border_color = src->border_color;
	dst1->bg_color = dst2->bg_color = src->bg_color;
	dst1->border = dst2->border = border;

	dy = (how & SPLIT_PIXELS) ? amount : src->height * amount / 100;
	dy1 = (how & SPLIT_1ST) ? dy : src->height - dy;
	dy2 = src->height - dy1;

	dst1->y0 = src->y0 + border;
	dst1->height = dy1 - 2 * border;

	dst2->y0 = src->y0 + dy1 + border;
	dst2->height = dy2 - 2 * border;
	}

void
glcd_widget_destroy_all(GlcdWindow *pW)
	{
	SList		*list;
	GlcdWidget	*pWid;
	boolean		done = FALSE;

	while (!done)
		{
		done = TRUE;	/* Assume won't find any */
		for (list = glcd_widget_list; list; list = list->next)
			{
			pWid = (GlcdWidget *) list->data;
			if (pWid->window != pW)
				continue;
			glcd_widget_list = slist_remove(glcd_widget_list, (void *) pWid);
			free(pWid);
			done = FALSE;		/* Keep looking until don't find one */
			break;
			}
		}
	free(pW);
	}

void
glcd_widget_draw_all(GlcdWindow *pW)
	{
	GlcdWidget	*pWid;
	SList		*list;

	for (list = glcd_widget_list; list; list = list->next)
		{
		pWid = (GlcdWidget *) list->data;
		if (pWid->window != pW)
			continue;
		if (pWid->type == WIDGET_TYPE_BUTTON)
			glcd_button_draw((GlcdButton *) pWid, BUTTON_UP);
		else if (pWid->type == WIDGET_TYPE_SPINBUTTON)
			glcd_spin_button_draw((GlcdSpinButton *) pWid, SPINBUTTON_UP);
		else if (pWid->type == WIDGET_TYPE_SLIDER)
			glcd_slider_draw((GlcdSlider *) pWid);
		}
	}


static void
get_value_extents(GlcdFont *font, int value, int *width, int *height)
	{
	char buf[32];

	sprintf(buf, "%d", value);

	if (width)
		*width = font->char_width * strlen(buf);
	if (height)
		*height = font->char_height;
	}

static void
get_value_region_extents(GlcdSlider *pSld)
	{
	char	sbuf[16];

	sprintf(sbuf, "%d", pSld->value_max);
	pSld->value_region.dx = pSld->font->char_width * strlen(sbuf);
	pSld->value_region.dy = pSld->font->char_height;
	}

/* Get max widget extents for all widgets in a window or, if window is NULL,
|  for all widgets in a draw area.  Use this to be able to evenly pack
|  widgets into a draw area and to be able to size buttons the same.
*/
void
glcd_widget_extents(GlcdWindow *pW, DrawArea *da, int *dxmax, int *dymax)
	{
	GlcdWidget		*pWid;
	GlcdButton		*pB;
	GlcdSpinButton	*pSB;
	GlcdSlider		*pS;
	SList			*list;
	int				t, dx = 0, dy = 0;

	for (list = glcd_widget_list; list; list = list->next)
		{
		pWid = (GlcdWidget *) list->data;
		if (pW && (pW != pWid->window))
			continue;
		else if (da && (da != pWid->draw_area))
			continue;
		if (pWid->type == WIDGET_TYPE_BUTTON)
			{
			pB = (GlcdButton *) pWid;
			if ((t = pB->font->char_width * strlen(pB->text) + 2 * pB->pad)
							> dx)
				dx = t;
			if ((t = pB->font->char_height + 2 * pB->pad) > dy)
				dy = t;
			}
		else if (pWid->type == WIDGET_TYPE_SPINBUTTON)
			{
			pSB = (GlcdSpinButton *) pWid;
			if ((t = pSB->text_width + 2 * pSB->pad + pSB->arrow_width) > dx)
				dx = t;
			if ((t = pSB->text_height + 2 * pSB->pad) > dy)
				dy = t;
			}
		else if (pWid->type == WIDGET_TYPE_SLIDER)
			{
			pS = (GlcdSlider *) pWid;
			t = MAX(pS->knob_width, pS->bar_width);
			if ((pWid->flags & WIDGET_FLAG_H_PACK) && t > dx)
				dx = t;
			else if ((pWid->flags & WIDGET_FLAG_V_PACK) && t > dy)
				dy = t;
			}
		}
	if (dxmax)
		*dxmax = dx;
	if (dymax)
		*dymax = dy;
	}

static int
glcd_draw_area_n_widgets(DrawArea *da)
	{
	GlcdWidget	*pWid;
	SList		*list;
	int			n_widgets = 0;

	for (list = glcd_widget_list; list; list = list->next)
		{
		pWid = (GlcdWidget *) list->data;
		if (pWid->draw_area == da)
			++n_widgets;
		}
	return n_widgets;
	}


/* Widget packing.
|  Widgets may be horizontally (glcd_widget_h_pack()) or
|  vertically (glcd_widget_v_pack()) packed into a draw area, but do not mix
|  horizontal and vertical packing into the same draw area.
|		Add the widget to the widget list.
|		Get the extents of all the widgets now in the draw area.
|		Reposition the widgets to space them evenly horizontally or vertically
|			in the area.
|  But, if pWid is NULL:
|		Just get the extents and reposition all the widgets that are there.
|		This allows repacking widgets after a manual resize of one of them.
*/
void
glcd_widget_h_pack(DrawArea *da, GlcdWidget *pWid, uint8_t flags)
	{
	GlcdWidget	*pw;
	GlcdSlider	*pSld;
	GlcdRegion	*pVR, *pKR, *pBR;
	SList		*list;
	int			dxmax, dymax;
	int			i, n_widgets, spacing;

	if (pWid)
		{
		pWid->draw_area = da;
		pWid->flags = flags;
		pWid->flags |= WIDGET_FLAG_H_PACK;
		if (flags & WIDGET_FLAG_PACK_START)
			glcd_widget_list = slist_prepend(glcd_widget_list, (void *) pWid);
		else
			glcd_widget_list = slist_append(glcd_widget_list, (void *) pWid);
		}

	glcd_widget_extents(NULL, da, &dxmax, &dymax);
	n_widgets = glcd_draw_area_n_widgets(da);
	spacing = (da->width - n_widgets * dxmax) / (n_widgets + 1);

	for (i = 0, list = glcd_widget_list; list; list = list->next)
		{
		pw = (GlcdWidget *) list->data;
		if (pw->draw_area != da)
			continue;
		pw->x = spacing + i * (spacing + dxmax);
		if (   pw->type == WIDGET_TYPE_BUTTON
		    || pw->type == WIDGET_TYPE_SPINBUTTON
		   )
			{
			pw->y = (da->height - dymax) / 2;
			pw->dx = dxmax;
			pw->dy = dymax;
			}
		else if (pw->type == WIDGET_TYPE_SLIDER)
			{
			pSld = (GlcdSlider *) pw;
			pw->dx = MAX(pSld->knob_width, pSld->bar_width);
			pw->x += (dxmax - pw->dx) / 2;

			pKR = &pSld->knob_region;
			pKR->x = pw->x + (pw->dx - pSld->knob_width) / 2;
			pKR->dx = pSld->knob_width;
			pKR->dy = pSld->knob_length;

			get_value_region_extents(pSld);
			pVR = &pSld->value_region;

			pSld->bar_length = da->height - 2 * pSld->pad - pVR->dy;
			pSld->slide_length = pSld->bar_length - pSld->knob_length;
			pw->dy = pSld->bar_length;

			pVR->x = pw->x + pw->dx / 2 - pVR->dx / 2;
			if (pSld->value_placement == GLCDSLIDER_VALUE_PLACEMENT_MIN)
				{
				pw->y = pSld->pad;
				pVR->y = pw->y + pw->dy;
				}
			else		/* GLCDSLIDER_VALUE_PLACEMENT_MAX	*/
				{
				pw->y = pSld->pad + pVR->dy;
				pVR->y = pSld->pad;
				}
			pKR->y = pw->y;				/* Sane knob pos for first draw */

			pBR = &pSld->bar_region;
			pBR->x = pw->x + (pw->dx - pSld->bar_width) / 2;
			pBR->y = pw->y;
			pBR->dx = pSld->bar_width;
			pBR->dy = pw->dy;
			}
		++i;
		}
	}

void
glcd_widget_v_pack(DrawArea *da, GlcdWidget *pWid, uint8_t flags)
	{
	GlcdWidget	*pw;
	GlcdSlider	*pSld;
	GlcdRegion	*pVR, *pKR, *pBR;
	SList		*list;
	int			dxmax, dymax;
	int			i, n_widgets, spacing;

	if (pWid)
		{
		pWid->draw_area = da;
		pWid->flags = flags;
		pWid->flags |= WIDGET_FLAG_V_PACK;
		if (flags & WIDGET_FLAG_PACK_START)
			glcd_widget_list = slist_prepend(glcd_widget_list, (void *) pWid);
		else
			glcd_widget_list = slist_append(glcd_widget_list, (void *) pWid);
		}

	glcd_widget_extents(NULL, da, &dxmax, &dymax);
	n_widgets = glcd_draw_area_n_widgets(da);
	spacing = (da->height - n_widgets * dymax) / (n_widgets + 1);

	for (i = 0, list = glcd_widget_list; list; list = list->next)
		{
		pw = (GlcdWidget *) list->data;
		if (pw->draw_area != da)
			continue;
		pw->y = spacing + i * (spacing + dymax);
		if (   pw->type == WIDGET_TYPE_BUTTON
		    || pw->type == WIDGET_TYPE_SPINBUTTON
		   )
			{
			pw->x = (da->width - dxmax) / 2;
			pw->dx = dxmax;
			pw->dy = dymax;
			}
		else if (pw->type == WIDGET_TYPE_SLIDER)
			{
			pSld = (GlcdSlider *) pw;
			pw->dy = MAX(pSld->knob_width, pSld->bar_width);
			pw->y += (dymax - pw->dy) / 2;

			pKR = &pSld->knob_region;
			pKR->y = pw->y + (pw->dy - pSld->knob_width) / 2;
			pKR->dx = pSld->knob_length;
			pKR->dy = pSld->knob_width;

			get_value_region_extents(pSld);
			pVR = &pSld->value_region;

			pSld->bar_length  = da->width - 2 * pSld->pad - pVR->dx;
			pSld->slide_length = pSld->bar_length - pSld->knob_length;
			pw->dx = pSld->bar_length;

			pVR->y  = pw->y + pw->dy / 2 - pVR->dy / 2;
			if (pSld->value_placement == GLCDSLIDER_VALUE_PLACEMENT_MIN)
				{
				pw->x = pSld->pad + pVR->dx;
				pVR->x = pSld->pad;
				}
			else		/* GLCDSLIDER_VALUE_PLACEMENT_MAX	*/
				{
				pw->x = pSld->pad;
				pVR->x = pw->x + pw->dx;
				}
			pKR->x = pw->x;		/* Sane knob pos for first draw */

			pBR = &pSld->bar_region;
			pBR->y = pw->y + (pw->dy - pSld->bar_width) / 2;
			pBR->x = pw->x;
			pBR->dx = pw->dx;
			pBR->dy = pSld->bar_width;
			}
		++i;
		}
	}


/* ====================================================== */

GlcdButton *
glcd_button_new(GlcdWindow *pW, GlcdFont *font, char *text,
				uint16_t text_color, uint16_t bg_color, uint8_t pad,
				void (*callback)(GlcdButton *), int data)
	{
	GlcdButton	*pBut;

	if ((pBut = (GlcdButton *) calloc(1, sizeof(GlcdButton))) == NULL)
		return NULL;

	((GlcdWidget *)pBut)->window    = pW;
	((GlcdWidget *)pBut)->type      = WIDGET_TYPE_BUTTON;

	pBut->font = font;
	pBut->text = text;
	pBut->text_color = text_color;
	pBut->bg_color = bg_color;
	pBut->pad = pad;
	pBut->callback = callback;
	pBut->data = data;

	return pBut;
	}

void
glcd_button_connect(GlcdButton *pBut, void (*callback)(GlcdButton *), int data)
	{
	pBut->callback = callback;
	pBut->data = data;
	}

void
glcd_button_draw(GlcdButton *pBut, boolean up)
	{
	GlcdWidget	*pWid = (GlcdWidget *)pBut;
	GlcdWindow	*pW;
	uint16_t	color;

	pW   = pWid->window;
	color = up ? Black : Gold;

	glcd_fill_rounded_rectangle(pW->glcd, pWid->draw_area, pBut->bg_color,
						pWid->x, pWid->y, pWid->dx, pWid->dy, 8);
	glcd_draw_rounded_rectangle(pW->glcd, pWid->draw_area, color,
						pWid->x, pWid->y, pWid->dx, pWid->dy, 8);

	glcd_draw_string(pW->glcd, pWid->draw_area, pBut->font, pBut->text_color,
					pWid->x + pWid->dx / 2
						- strlen(pBut->text) * pBut->font->char_width / 2,
					pWid->y + pWid->dy / 2 - pBut->font->char_height / 2,
					(char *) pBut->text);
	}


/* ====================================================== */

GlcdSpinButton *
glcd_spin_button_new(GlcdWindow *pW, GlcdFont *font,
				uint16_t text_color, uint16_t bg_color, uint16_t arrow_color,
				uint8_t pad,
				int value, int value_min, int value_max, int increment,
				void (*callback)(GlcdSpinButton *), int data)
	{
	GlcdSpinButton	*pSB;
	int				ref_value;

	if ((pSB = (GlcdSpinButton *) calloc(1, sizeof(GlcdSpinButton))) == NULL)
		return NULL;

	((GlcdWidget *)pSB)->window    = pW;
	((GlcdWidget *)pSB)->type      = WIDGET_TYPE_SPINBUTTON;

	pSB->font = font;
	pSB->text_color = text_color;
	pSB->bg_color = bg_color;
	pSB->arrow_color = arrow_color;
	pSB->callback = callback;
	pSB->data = data;

	pSB->value_max = value_max;
	pSB->value_min = value_min;
	pSB->increment = increment;
	pSB->pad = pad;

	if (value < value_min)
		value = value_min;
	else if (value > value_max)
		value = value_max;
	pSB->value = value;
	sprintf(pSB->text, "%d", value);

	if (value_min < 0 && abs(value_min) * 10 > abs(value_max))
		ref_value = value_min;
	else
		ref_value = value_max;
	get_value_extents(font, ref_value, &pSB->text_width, &pSB->text_height);

	pSB->arrow_width = pSB->text_height / 2 + pSB->pad;

	return pSB;
	}

void
glcd_spin_button_connect(GlcdSpinButton *pBut,
					void (*callback)(GlcdSpinButton *), int data)
	{
	pBut->callback = callback;
	pBut->data = data;
	}

void
glcd_spin_button_draw(GlcdSpinButton *pSB, int how)
	{
	GlcdWidget	*pWid = (GlcdWidget *)pSB;
	GlcdRegion	*pVR		= &pSB->value_region;
//	GlcdRegion	*pAR		= &pSB->arrow_region;
	GlcdWindow	*pW;
	char		buf[32];
	uint16_t	color;
	int			x;

	pW   = pWid->window;
	color = (how > 0) ? Black : Gold;

	/* Value region
	*/
	glcd_fill_rounded_rectangle(pW->glcd, pWid->draw_area, pSB->bg_color,
					pVR->x, pVR->y, pVR->dx, pVR->dy, 8);
	glcd_draw_rounded_rectangle(pW->glcd, pWid->draw_area, Black,
					pVR->x, pVR->y, pVR->dx, pVR->dy, 8);

	/* Draw current value right justified in value region.
	*/
	sprintf(buf, "%d", pSB->value);
	x = pVR->dx - pSB->font->char_width * strlen(buf) - pSB->pad;
	glcd_draw_string(pW->glcd, pWid->draw_area, pSB->font, pSB->text_color,
				pVR->x + x, pVR->y + pSB->pad, buf);


	glcd_fill_rounded_rectangle(pW->glcd, pWid->draw_area, pSB->bg_color,
						pWid->x, pWid->y, pWid->dx, pWid->dy, 8);
	glcd_draw_rounded_rectangle(pW->glcd, pWid->draw_area, color,
						pWid->x, pWid->y, pWid->dx, pWid->dy, 8);

	}


/* ====================================================== */

GlcdSlider *
glcd_slider_new(GlcdWindow *pW, GlcdFont *font,
				uint16_t text_color, uint16_t body_color, uint16_t knob_color,
				int value, int value_placement, int value_min, int value_max,
				int bar_width, int knob_width, int knob_length, int pad)
	{
	GlcdSlider	*pSld;

	if ((pSld = (GlcdSlider *) calloc(1, sizeof(GlcdSlider))) == NULL)
		return NULL;

	((GlcdWidget *)pSld)->window = pW;
	((GlcdWidget *)pSld)->type = WIDGET_TYPE_SLIDER;

	pSld->font = font;
	pSld->text_color = text_color;
	pSld->body_color = body_color;
	pSld->knob_color = knob_color;

	pSld->value_placement = value_placement;
	pSld->value_min = value_min;
	pSld->value_max = value_max;
	if (value >= value_min && value <= value_max)
		pSld->value = value;
	else
		pSld->value = value_min;

	pSld->knob_length = knob_length;
	pSld->knob_width  = knob_width;
	pSld->bar_width   = bar_width;
	pSld->pad         = pad;

	return pSld;
	}

void
glcd_slider_connect(GlcdSlider *pSld, void (*callback)(GlcdSlider *), int data)
	{
	pSld->callback = callback;
	pSld->data = data;
	}


/* Slider draw detail:
|
|    ->:     :<-- Knob Length
| -     -----
| :    (     )
| :    |-----|------------------------------------    -
| K    |     |                                    |   B
| :    |-----|------------------------------------    -
| :    (  :  )                                 :  :
| -     -----                                  :  :
|      :  :<------- Slide Length-------------->:  :
|      :<------------Bar Length------------------>:
|
|   K = Knob Width
|   B = Bar Width
|   Slider value can be printed at the min or max end of the slider.
|
| The knob is drawn within the bounds of the bar ends, so the slide length
| will be the Bar Length minus the Knob Length.  Touch events occur for touches
| within the Bar Length, so touches between the min bar end and 1/2 the knob
| length will set the slider to its minimum value, and touches between the
| max bar end and 1/2 the Knob Length will set the slider to its max value.
*/
void
glcd_slider_draw(GlcdSlider *pSld)
	{
	GlcdWidget	*pWid		= (GlcdWidget *) pSld;
	GlcdRegion	*pKR		= &pSld->knob_region,
				*pVR		= &pSld->value_region,
				*pBR		= &pSld->bar_region;
	GlcdWindow	*pW;
	int			x, slide_position;
	char		sbuf[32];

	pW = pWid->window;

	/* Erase old knob
	*/
	glcd_fill_rounded_rectangle(pW->glcd, pWid->draw_area,
					pWid->draw_area->bg_color,
					pKR->x, pKR->y, pKR->dx, pKR->dy, GLCDSLIDER_KNOB_RADIUS);

	/* GlcdSlider body
	*/
	glcd_fill_rounded_rectangle(pW->glcd, pWid->draw_area, pSld->body_color,
					pBR->x, pBR->y, pBR->dx, pBR->dy, GLCDSLIDER_BAR_RADIUS);
	glcd_draw_rounded_rectangle(pW->glcd, pWid->draw_area, Black,
					pBR->x, pBR->y, pBR->dx, pBR->dy, GLCDSLIDER_BAR_RADIUS);

	/* Calculate new knob position, move the knob region appropriately, and
	|  draw the knob.
	*/
	slide_position = pSld->slide_length * (pSld->value - pSld->value_min)
										/ (pSld->value_max - pSld->value_min);

	if (((GlcdWidget *)pSld)->flags & WIDGET_FLAG_H_PACK)
		pKR->y = pBR->y + pBR->dy - pKR->dy - slide_position;
	else
		pKR->x = pBR->x + slide_position;

	glcd_fill_rounded_rectangle(pW->glcd, pWid->draw_area, pSld->knob_color,
					pKR->x, pKR->y, pKR->dx, pKR->dy, GLCDSLIDER_KNOB_RADIUS);
	glcd_draw_rounded_rectangle(pW->glcd, pWid->draw_area, Black,
					pKR->x, pKR->y, pKR->dx, pKR->dy, GLCDSLIDER_KNOB_RADIUS);


	/* Clear the value region and draw current value centered in value region.
	*/
	glcd_fill_rectangle(pW->glcd, pWid->draw_area, pWid->draw_area->bg_color,
				pVR->x, pVR->y, pVR->dx, pVR->dy);
	sprintf(sbuf, "%d", pSld->value);
	x = (pVR->dx - pSld->font->char_width * strlen(sbuf)) / 2;
	glcd_draw_string(pW->glcd, pWid->draw_area, pSld->font, pSld->text_color,
				pVR->x + x, pVR->y, sbuf);
	}
