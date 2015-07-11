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


#ifndef _GLCD_WIDGETS_H
#define _GLCD_WIDGETS_H

#include "glcd.h"

#define	BUTTON_UP	1
#define	BUTTON_DOWN	0

#define SPINBUTTON_UP		0
#define SPINBUTTON_INC_DOWN	1
#define SPINBUTTON_DEC_DOWN	2


#define SPLIT_1ST			0x1
#define SPLIT_2ND			0x2
#define	SPLIT_PERCENT		0x4
#define SPLIT_PIXELS		0x8


typedef struct
	{
	Glcd		*glcd;
	char		*title;
	GlcdFont	*title_font;
	uint16_t	title_color;
	DrawArea	title_area;
	DrawArea	body_area;
	}
	GlcdWindow;



#define BUTTON_EXPAND_NONE		0x1
#define BUTTON_EXPAND_EQUAL		0x2
#define BUTTON_EXPAND_FILL		0x4

#define	WIDGET_TYPE_BUTTON		1
#define	WIDGET_TYPE_SLIDER		2
#define	WIDGET_TYPE_SPINBUTTON	3

#define	WIDGET_FLAG_JUSTIFY_START	0x1
#define	WIDGET_FLAG_JUSTIFY_END		0x2
#define	WIDGET_FLAG_JUSTIFY_CENTER	0x4
#define	WIDGET_FLAG_PACK_START		0x8
#define	WIDGET_FLAG_PACK_END		0x10
#define	WIDGET_FLAG_H_PACK			0x20
#define	WIDGET_FLAG_V_PACK			0x40

typedef struct
	{
	GlcdWindow	*window;
	DrawArea	*draw_area;
	uint8_t		type,
				flags;
	int			x, y,
				dx, dy;
	}
	GlcdWidget;

typedef struct _GlcdButton
	{
	GlcdWidget	widget;
	GlcdFont	*font;
	char		*text;
	uint16_t	text_color,
				bg_color;
	uint8_t		pad;

	void		(*callback)(struct _GlcdButton *);
	int			data;
	}
	GlcdButton;

typedef struct
	{
	uint16_t	x, y,
				dx, dy;
	}
	GlcdRegion;


typedef struct _GlcdSpinButton
	{
	GlcdWidget	widget;
	GlcdFont	*font;
	char		text[32];
	uint16_t	text_color,
				bg_color,
				arrow_color;

	int			value,
				value_min,
				value_max,
				increment;

	int			text_width,
				text_height;
	int			arrow_width;;
	int			pad;

	GlcdRegion	value_region,
				arrow_region;

	void		(*callback)(struct _GlcdSpinButton *);
	int			data;
	}
	GlcdSpinButton;



#define	GLCDSLIDER_VALUE_PLACEMENT_MIN	1
#define	GLCDSLIDER_VALUE_PLACEMENT_MAX	2

#define	GLCDSLIDER_BAR_RADIUS	4
#define	GLCDSLIDER_KNOB_RADIUS	4

typedef struct _GlcdSlider
	{
	GlcdWidget	widget;
	GlcdFont	*font;
	uint16_t		text_color,
				body_color,
				knob_color;
	uint8_t		pad,
				value_placement;		/* Next to value_min or value_max */
	int			value,
				value_min,
				value_max;

	uint16_t		knob_width,
				knob_length,
				bar_width,
				bar_length,
				slide_length;
	GlcdRegion	value_region,
				knob_region,
				bar_region;

	unsigned int time;
	void		(*callback)(struct _GlcdSlider *);
	int			data;
	}
	GlcdSlider;


typedef struct
	{
	TouchCalibratePoint	display,
						raw;
	boolean				valid;
	unsigned int 		time;

	int			x,
				y,
				x_raw,
				y_raw;
	}
	GlcdEventTouch;


GlcdWindow 	*glcd_window_new(Glcd *glcd,
					char *title, GlcdFont *title_font,
					uint16_t title_color, uint16_t title_bg_color,
					uint16_t title_border_color, uint8_t title_border,
					uint16_t body_bg_color, uint16_t body_border_color,
					uint8_t body_border,
					int	x0, int y0, int width, int height);
void		glcd_window_clear(GlcdWindow *pW, boolean draw_title);
void		glcd_window_delete(GlcdWindow *pW);
void		glcd_widget_destroy_all(GlcdWindow *pW);
void		glcd_widget_draw_all(GlcdWindow *pW);
void		glcd_widget_h_pack(DrawArea *da, GlcdWidget *pWid, uint8_t flags);
void		glcd_widget_v_pack(DrawArea *da, GlcdWidget *pWid, uint8_t flags);
void		glcd_widget_extents(GlcdWindow *pW, DrawArea *da,
					int *dxmax, int *dymax);


void		glcd_area_clear(Glcd *glcd, DrawArea *da);

void		glcd_area_h_split(DrawArea *src, DrawArea *dst1, DrawArea *dst2,
					uint8_t amount, uint8_t how, uint8_t border);
void		glcd_area_v_split(DrawArea *src, DrawArea *dst1, DrawArea *dst2,
					uint8_t amount, uint8_t how, uint8_t border);


GlcdButton	*glcd_button_new(GlcdWindow *pW,
					GlcdFont *font, char *text,
					uint16_t text_color, uint16_t bg_color, uint8_t pad,
					void (*callback)(GlcdButton *), int data);
void		glcd_button_connect(GlcdButton *pBut,
					void (*callback)(GlcdButton *), int data);
void		glcd_button_draw(GlcdButton *but, boolean up);


GlcdSpinButton
			*glcd_spin_button_new(GlcdWindow *pW, GlcdFont *font,
					uint16_t text_color, uint16_t bg_color,
					uint16_t arrow_color, uint8_t pad,
					int value, int value_min, int value_max, int increment,
					void (*callback)(GlcdSpinButton *), int data);
void		glcd_spin_button_connect(GlcdSpinButton *pBut,
					void (*callback)(GlcdSpinButton *), int data);
void		glcd_spin_button_draw(GlcdSpinButton *pBut, int how);


GlcdSlider	*glcd_slider_new(GlcdWindow *pW, GlcdFont *font,
					uint16_t text_color, uint16_t body_color, uint16_t knob_color,
					int value, int value_pos, int value_min, int value_max,
					int bar_width, int knob_width, int knob_length, int pad);
void		glcd_slider_connect(GlcdSlider *pSld,
					void (*callback)(GlcdSlider *), int data);
void		glcd_slider_draw(GlcdSlider *sld);



void		glcd_event_handler(Glcd *glcd);
boolean		glcd_event_check(Glcd *glcd);
void		glcd_event_wait_for_touch_release(Glcd *glcd);
GlcdButton	*glcd_event_touch_in_button(GlcdEventTouch *te);
GlcdSlider	*glcd_event_touch_in_slider(GlcdEventTouch *te);
GlcdEventTouch *glcd_event_get_touch(Glcd *glcd);

#endif	/* _GLCD_WIDGETS_H */
