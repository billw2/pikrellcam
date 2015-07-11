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
#include <stdlib.h>
#include <math.h>

#include "glcd.h"
#include "ili9341.h"

#include <endian.h>



#define SWAP(a, b)	{ int t; t = a; a = b; b = t; }

#define	IS_CLIPPED(da, x, y)	(   ((x) < 0) || ((x) >= (da)->width) \
							     || ((y) < 0) || ((y) >= (da)->height))



void
glcd_led(Glcd *glcd, int state)
	{
	if (glcd->led_control)
		(*glcd->led_control)(glcd, state);
	}

int
glcd_get_screen_height(Glcd *glcd)
	{
	return glcd->screen_height;
	}

int
glcd_get_screen_width(Glcd *glcd)
	{
	return glcd->screen_width;
	}

int
glcd_get_display_height(Glcd *glcd)
	{
	return glcd->display.height;
	}

int
glcd_get_display_width(Glcd *glcd)
	{
	return glcd->display.width;
	}

DrawArea *
glcd_get_display_area(Glcd *glcd)
	{
	return &glcd->display;
	}

  /* For serial interfaces where 16 bit color values will be clocked
  |  out a byte at a time, endianess matters.  Raspberry Pi defaults
  |  to little endian.
  */
uint16_t
glcd_map_color(uint8_t r, uint8_t g, uint8_t b)
	{
	uint16_t	color;

	color = (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3;

#if __BYTE_ORDER == __LITTLE_ENDIAN
	color = (color >> 8) | (color << 8);
#endif

	return color;
	}


uint16_t
glcd_map_color_percent(int r_percent, int g_percent, int b_percent)
	{
	uint8_t		r,		/* 5 bits */
				g,		/* 6 bits */
				b;		/* 5 bits */
	uint16_t	color;

	r = r_percent * 0x1f / 100;
	g = g_percent * 0x3f / 100;
	b = b_percent * 0x1f / 100;

	color = (r << 11) | (g << 5) | b;

#if __BYTE_ORDER == __LITTLE_ENDIAN
	color = (color >> 8) | (color << 8);
#endif

	return color;
	}

int
glcd_get_rotation(Glcd *glcd)
	{
	return glcd->rotation;
	}

void
glcd_set_rotation(Glcd *glcd, int rotation)
	{
	glcd->rotation = rotation;

	glcd->display.x0 = 0;
	glcd->display.y0 = 0;
	if (rotation == 0 || rotation == 180)
		{
		glcd->display.width = glcd->screen_width;
		glcd->display.height = glcd->screen_height;
		}
	else if (rotation == 90 || rotation == 270)
		{
		glcd->display.width = glcd->screen_height;
		glcd->display.height = glcd->screen_width;
		}
	else
		printf("Bad glcd_set_rotation value: %d\n", rotation);

	if (glcd->set_rotation)
		(*glcd->set_rotation)(glcd, rotation);
	}

void
glcd_set_frame_buffer(Glcd *glcd, void *fb, int width, int height)
	{
	if (glcd->set_frame_buffer)
		(*glcd->set_frame_buffer)(glcd, fb, width, height);
	}

void
glcd_draw_pixel(Glcd *glcd, DrawArea *da, uint16_t color, int x, int y)
	{
	if (!da || IS_CLIPPED(da, x, y))
		return;
	if (glcd->set_pixel)
		glcd->set_pixel(glcd, color, da->x0 + x, da->y0 + y);
	}


  /* Bresenham's algorithm from wikipedia
  */
void
glcd_draw_line(Glcd *glcd, DrawArea *da, uint16_t color,
				int x0, int y0, int x1, int y1)
	{
	int			x, y;
	int			deltax, deltay, ystep, error;
	boolean		steep;

	if (!da || !glcd->set_pixel)
		return;
	if (y0 == y1)
		{
		if (x0 > x1)
			SWAP(x0, x1);
		glcd_draw_h_line(glcd, da, color, x0, y0, x1 - x0);
		return;		
		}
	if (x0 == x1)
		{
		if (y0 > y1)
			SWAP(y0, y1);
		glcd_draw_v_line(glcd, da, color, x0, y0, y1 - y0);
		return;		
		}
	steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep)
		{
		SWAP(x0, y0);
		SWAP(x1, y1);
		}
	if (x0 > x1)
		{
		SWAP(x0, x1);
		SWAP(y0, y1);
		}
	deltax = x1 - x0;
	deltay = abs(y1 - y0);
	error = deltax / 2;
	y = y0;
	ystep = (y0 < y1) ? 1 : -1;

	for (x = x0; x <= x1; ++x)
		{
		if (   (steep && !IS_CLIPPED(da, y, x))
		    || (!steep && !IS_CLIPPED(da, x, y))
		   )
			{
			if (steep)
				glcd->set_pixel(glcd, color, da->x0 + y, da->y0 + x);
			else
				glcd->set_pixel(glcd, color, da->x0 + x, da->y0 + y);
			}
		error = error - deltay;
		if (error < 0)
			{
			y += ystep;
			error += deltax;
			}
		}
	}

void
glcd_draw_h_line(Glcd *glcd, DrawArea *da, uint16_t color,
			int x0, int y0, int length)
	{
	if (!da || !glcd->v_line || length == 0)
		return;
	if (y0 < 0 || y0 >= da->height || length == 0)
		return;

	if (length < 0)
		{
		x0 += length;
		length = -length;
		}
	if (x0 >= da->width || x0 + length <= 0)
		return;

	if (x0 < 0)
		{
		length += x0;
		x0 = 0;
		}
	if (x0 + length > da->width)
		length = da->width - x0;

	x0 += da->x0;
	y0 += da->y0;

	glcd->h_line(glcd, color, x0, y0, length);
	}


void
glcd_draw_v_line(Glcd *glcd, DrawArea *da, uint16_t color,
			int x0, int y0, int length)
	{
	if (!da || !glcd->v_line || length == 0)
		return;
	if (x0 < 0 || x0 >= da->width || length == 0)
		return;

	if (length < 0)
		{
		y0 += length;
		length = -length;
		}
	if (y0 >= da->height || y0 + length <= 0)
		return;

	if (y0 < 0)
		{
		length += y0;
		y0 = 0;
		}
	if (y0 + length > da->height)
		length = da->height - y0;

	x0 += da->x0;
	y0 += da->y0;

	glcd->v_line(glcd, color, x0, y0, length);
	}


void
glcd_draw_rectangle(Glcd *glcd, DrawArea *da, uint16_t color,
				int x0, int y0, int width, int height)
	{
	if (!da)
		return;
	glcd_draw_h_line(glcd, da, color, x0, y0, width);
	glcd_draw_h_line(glcd, da, color, x0, y0 + height - 1, width);
	glcd_draw_v_line(glcd, da, color, x0, y0, height);
	glcd_draw_v_line(glcd, da, color, x0 + width - 1, y0, height);
	}

void
glcd_fill_rectangle(Glcd *glcd, DrawArea *da, uint16_t color,
				int x0, int y0, int width, int height)
	{
	int		x1, y1, x, y, dx, dy;

	x1 = x0 + width - 1;
	y1 = y0 + height - 1;
	if (width < 0)
		SWAP(x0, x1);
	if (height < 0)
		SWAP(y0, y1);

	if (   !da || width == 0 || height == 0
	    || x0 >= da->width || y0 >= da->height
		|| x1 < 0 || y1 < 0
	   )
		return;

	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 >= da->width)
		x1 = da->width - 1;
	if (y1 >= da->height)
		y1 = da->height - 1;

	x0 += da->x0;
	y0 += da->y0;
	x1 += da->x0;
	y1 += da->y0;
	dx = x1 - x0 + 1;
	dy = y1 - y0 + 1;

	if (dx < dy)
		{
		for (x = 0; x < dx; ++x)
			glcd->v_line(glcd, color, x0 + x, y0, dy);
		}
	else
		{
		for (y = 0; y < dy; ++y)
			glcd->h_line(glcd, color, x0, y0 + y, dx);
		}
	}

void
glcd_draw_rounded_rectangle(Glcd *glcd, DrawArea *da, uint16_t color,
				int x0, int y0, int width, int height, int radius)
	{
	int	e;
	int	x, y;

	if (!da || radius < 0)
		return;
	if (radius > width / 2)
		radius = width / 2;
	if (radius > height / 2)
		radius = height / 2;

	glcd_draw_h_line(glcd, da, color, x0 + radius, y0, width - 2 * radius);
	glcd_draw_h_line(glcd, da, color, x0 + radius, y0 + height - 1, width - 2*radius);
	glcd_draw_v_line(glcd, da, color, x0, y0 + radius, height - 2 * radius);
	glcd_draw_v_line(glcd, da, color, x0 + width - 1, y0 + radius, height - 2 * radius);

	e = (radius * radius) + radius;

	x = radius;
	y = 0;
	while (x > y)
		{
		y++;
		if (y * y + x * x >= e)
			x--;

		/* Top left arc
		*/
		glcd_draw_pixel(glcd, da, color, x0 + radius - x, y0 + radius - y);
		glcd_draw_pixel(glcd, da, color, x0 + radius - y, y0 + radius - x);

		/* Top right arc
		*/
		glcd_draw_pixel(glcd, da, color, x0 + width - 1 - radius + x, y0 + radius - y);
		glcd_draw_pixel(glcd, da, color, x0 + width - 1 - radius + y, y0 + radius - x);

		/* Bottom left arc
		*/
		glcd_draw_pixel(glcd, da, color, x0 + radius - x, y0 + height - 1 - radius + y);
		glcd_draw_pixel(glcd, da, color, x0 + radius - y, y0 + height - 1 - radius + x);

		/* Bottom right arc
		*/
		glcd_draw_pixel(glcd, da, color, x0 + width - 1 - radius + x,
						y0 + height - 1 - radius + y);
		glcd_draw_pixel(glcd, da, color, x0 + width - 1 - radius + y,
						y0 + height - 1 - radius + x);
		}
	}


void
glcd_fill_rounded_rectangle(Glcd *glcd, DrawArea *da, uint16_t color,
				int x0, int y0, int width, int height, int radius)
	{
	int		e;
	int		x, y;

	if (!da || radius < 0)
		return;
	if (radius > width / 2)
		radius = width / 2;
	if (radius > height / 2)
		radius = height / 2;

	if (height - 2 * radius > 0)
		glcd_fill_rectangle(glcd, da, color, x0, y0 + radius,
					width, height - 2 * radius);
	glcd_draw_h_line(glcd, da, color, x0 + radius, y0, width - 2 * radius);
	glcd_draw_h_line(glcd, da, color, x0 + radius, y0 + height - 1, width - 2*radius);

	e = (radius * radius) + radius;

	x = radius;
	y = 0;
	while (x > y)
		{
		y++;
		if (y * y + x * x >= e)
			x--;
		glcd_draw_h_line(glcd, da, color, x0 + radius - x, y0 + radius - y,
							width - 2 * radius + 2 * x);
		glcd_draw_h_line(glcd, da, color, x0 + radius - y, y0 + radius - x,
							width - 2 * radius + 2 * y);
		glcd_draw_h_line(glcd, da, color, x0 + radius - x, y0 + height - 1 - radius +y,
							width - 2 * radius + 2 * x);
		glcd_draw_h_line(glcd, da, color, x0 + radius - y, y0 + height - 1 - radius +x,
							width - 2 * radius + 2 * y);
		}
	}


  /* Draw a circle using wikipedia midpoint circle (Bresenham) algorithm
  */
void
glcd_draw_circle(Glcd *glcd, DrawArea *da, uint16_t color, int x0, int y0, int radius)
	{
	int	x		= 0,
		y		= radius,
		f		= 1 - radius;
	int	ddF_x	= 1,
		ddF_y	= -2 * radius;

	if (!da || radius <= 0)
		return;
	glcd_draw_pixel(glcd, da, color, x0, y0 + radius);
	glcd_draw_pixel(glcd, da, color, x0, y0 - radius);
	glcd_draw_pixel(glcd, da, color, x0 + radius, y0);
	glcd_draw_pixel(glcd, da, color, x0 - radius, y0);
 
	while(x < y)
		{
		if (f >= 0) 
			{
			y--;
			ddF_y += 2;
			f += ddF_y;
			}
		x++;
		ddF_x += 2;
		f += ddF_x;    
		glcd_draw_pixel(glcd, da, color, x0 + x, y0 + y);
		glcd_draw_pixel(glcd, da, color, x0 - x, y0 + y);
		glcd_draw_pixel(glcd, da, color, x0 + x, y0 - y);
		glcd_draw_pixel(glcd, da, color, x0 - x, y0 - y);
		glcd_draw_pixel(glcd, da, color, x0 + y, y0 + x);
		glcd_draw_pixel(glcd, da, color, x0 - y, y0 + x);
		glcd_draw_pixel(glcd, da, color, x0 + y, y0 - x);
		glcd_draw_pixel(glcd, da, color, x0 - y, y0 - x);
		}
	}


  /* Draw a filled circle using midpoint circle algorithm but add
  |  optimising out drawing over already drawn pixels when y value doesn't
  |  change (Ie, don't draw the whole line again, just the endpoint pixels).
  */
void
glcd_fill_circle(Glcd *glcd, DrawArea *da, uint16_t color,
			int x0, int y0, int radius)
	{
	int	f		= 1 - radius;
	int	ddF_x	= 1;
	int	ddF_y	= -2 * radius;
	int	x		= 0;
	int	y		= radius;
	int	y_prev	= radius;

	if (!da || radius <= 0)
		return;
	glcd_draw_pixel(glcd, da, color, x0, y0 + radius);
	glcd_draw_pixel(glcd, da, color, x0, y0 - radius);
	glcd_draw_h_line(glcd, da, color, x0 - radius, y0, radius + radius + 1);

	while(x < y)
		{
		if (f >= 0) 
			{
			y--;
			ddF_y += 2;
			f += ddF_y;
			}
		x++;
		ddF_x += 2;
		f += ddF_x;

		if (y < y_prev)
			{
			glcd_draw_h_line(glcd, da, color, x0 - x, y0 + y, x + x + 1);
			glcd_draw_h_line(glcd, da, color, x0 - x, y0 - y, x + x + 1);
			}
		else
			{
			glcd_draw_pixel(glcd, da, color, x0 - x, y0 + y);
    		glcd_draw_pixel(glcd, da, color, x0 + x, y0 + y);
			glcd_draw_pixel(glcd, da, color, x0 - x, y0 - y);
    		glcd_draw_pixel(glcd, da, color, x0 + x, y0 - y);
			}
		glcd_draw_h_line(glcd, da, color, x0 - y, y0 + x, y + y + 1);
		glcd_draw_h_line(glcd, da, color, x0 - y, y0 - x, y + y + 1);

		y_prev = y;
		}
	}

void
glcd_fill_screen(Glcd *glcd, uint16_t color)
	{
	int		y;

	for (y = 0; y < glcd->display.height - 1; ++y)
		glcd->h_line(glcd, color, 0, y, glcd->display.width);
	}


void
glcd_print_string(Glcd *glcd, DrawArea *da, GlcdFont *font, uint16_t color,
			boolean clear, int row, char *string)
	{
	int	y;

	if (!da || !font || !string)
		return;
	y = row * font->char_height + 1;
	if (clear)
		glcd_fill_rectangle(glcd, da, da->bg_color,
						0, y, da->width, font->char_height);
	glcd_draw_string(glcd, da, font, color, 1, y, string);
	}


int
glcd_draw_string(Glcd *glcd, DrawArea *da, GlcdFont *font, uint16_t color,
		int x0, int y0, char *string)
	{
	char	*s;
	int		x, y, x1, count, ib;
	uint8_t	data, mask, *pBitmap;


	if (!da || !font || !string)
		return 0;

	for (count = 0, s = string; *s; ++s, ++count)
		{
		ib = (*s - font->first_char) * font->char_height;
		ib *= 1 + (font->char_width - 1) / 8;
		pBitmap = (uint8_t *) &font->bitmap[ib];

		x1 = x0 + font->char_width * count;

		for(y = 0; y < font->char_height; ++y)
			{
			mask = 0x80;
			data = *pBitmap++;
			for(x = 0; x < font->char_width; ++x)
				{   
				if (!mask)
					{
					data = *pBitmap++;
					mask = 0x80;
					}
				if (   (data & mask)
				    && !IS_CLIPPED(da, x1 + x, y0 + y)
				   )
					{
					glcd->set_pixel(glcd, color,
								da->x0 + x1 + x, da->y0 + y0 + y);
					}
				mask >>= 1;
				}
			}
		}
	return count * font->char_width;
	}


int
glcd_draw_string_rotated(Glcd *glcd, DrawArea *pA, GlcdFont *font,
			uint16_t color, int degree, int x0, int y0, char *string)
	{
	char	*s;
	uint8_t	*pBitmap;
	int		x, y;
	uint16_t	data, mask;
	int		ib, count, x1, x_rot, y_rot;
	float	sinT, cosT, ysinT, ycosT, radians;

	if (!pA || !font || !string)
		return 0;

	radians = degree * (2 * 3.141592654 / 360);
	sinT = (float) sin(radians);
	cosT = (float) cos(radians);

	for (count = 0, s = string; *s; ++count, ++s)
		{
		x1 = font->char_width * count;

		ib = (*s - font->first_char) * font->char_height;
		ib *= 1 + (font->char_width - 1) / 8;
		pBitmap = (uint8_t *) &font->bitmap[ib];

		for(y = 0; y < font->char_height; ++y)
			{
			mask = 0x80;
			data = *pBitmap++;
			ysinT = y * sinT;
			ycosT = y * cosT;
			for (x = 0; x < font->char_width; ++x)
				{
				if (!mask)
					{
					data = *pBitmap++;
					mask = 0x80;
					}
				x_rot = x0 + (((x1 + x) * cosT) - ysinT);
				y_rot = y0 + (((x1 + x) * sinT) + ycosT);

				if (   (data & mask)
				    && !IS_CLIPPED(pA, x_rot, y_rot)
				   )
					{
					x_rot += pA->x0;
					y_rot += pA->y0;
					glcd->set_pixel(glcd, color, x_rot, y_rot);
					}
				mask >>= 1;
				}
			}
		}
	return count * font->char_width;
	}

void
glcd_draw_image(Glcd *glcd, DrawArea *da, GlcdImage *im, int x0, int y0)
	{
	uint16_t	*pixel, *row_pixel;
	int		x, y, x1, y1, dx;

	if (!da || !im || im->width <= 0 || im->height <= 0)
		return;
	x1 = x0 + im->width - 1;
	y1 = y0 + im->height - 1;
	if (   x0 >= da->width || y0 >= da->height
	    || x1 < 0 || y1 < 0
	   )
		return;

	row_pixel = &im->data[0];
	if (x0 < 0)
		{
		row_pixel += -x0;
		x0 = 0;
		}
	if (y0 < 0)
		{
		row_pixel += (-y0 * im->width);
		y0 = 0;
		}
	if (x1 >= da->width)
		x1 = da->width - 1;
	if (y1 >= da->height)
		y1 = da->height - 1;

	x0 += da->x0;
	y0 += da->y0;
	x1 += da->x0;
	y1 += da->y0;
	dx = x1 - x0 + 1;

	for (y = y0; y <= y1; ++y)
		{
		pixel = row_pixel;
		for (x = 0; x < dx; ++x)
			glcd->set_pixel(glcd, *pixel++, x, y);
		row_pixel += im->width;
		}
	}

void
glcd_draw_pixmap(Glcd *glcd, DrawArea *da, uint16_t *pixmap,
			int x0, int y0, int pixmap_width, int pixmap_height)
	{
	uint16_t	*pixel, *row_pixel;
	int		x, y, x1, y1, dx;

	if (!da || !pixmap || pixmap_width <= 0 || pixmap_height <= 0)
		return;
	x1 = x0 + pixmap_width - 1;
	y1 = y0 + pixmap_height - 1;
	if (   x0 >= da->width || y0 >= da->height
	    || x1 < 0 || y1 < 0
	   )
		return;

	row_pixel = &pixmap[0];
	if (x0 < 0)
		{
		row_pixel += -x0;
		x0 = 0;
		}
	if (y0 < 0)
		{
		row_pixel += (-y0 * pixmap_width);
		y0 = 0;
		}
	if (x1 >= da->width)
		x1 = da->width - 1;
	if (y1 >= da->height)
		y1 = da->height - 1;

	x0 += da->x0;
	y0 += da->y0;
	x1 += da->x0;
	y1 += da->y0;
	dx = x1 - x0 + 1;

	for (y = y0; y <= y1; ++y)
		{
		pixel = row_pixel;
		for (x = 0; x < dx; ++x)
			glcd->set_pixel(glcd, *pixel++, x, y);
		row_pixel += pixmap_width;
		}
	}
