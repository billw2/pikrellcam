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
#include "glcd.h"


static void
i420_set_pixel(Glcd *glcd, uint16_t color, int x, int y)
	{
	uint8_t		*p = (uint8_t *) glcd->frame_buffer;
	int			offset;

	offset = y * glcd->display.width + x;
	*(p + offset) = (uint8_t) color;
	}

static void
i420_h_line(Glcd *glcd, uint16_t color, int x, int y, int dx)
	{
	uint8_t		*p = (uint8_t *) glcd->frame_buffer;
	int			offset;

	offset = y * glcd->display.width + x;
	while (dx--)
		*(p + offset++) = (uint8_t) color;
	}

static void
i420_v_line(Glcd *glcd, uint16_t color, int x, int y, int dy)
	{
	uint8_t		*p = (uint8_t *) glcd->frame_buffer;
	int			offset;

	offset = y * glcd->display.width + x;
	while (dy--)
		{
		*(p + offset) = (uint8_t) color;
		offset += glcd->display.width;
		}
	}

static void
i420_set_frame_buffer(Glcd *glcd, uint16_t *fb, int width, int height)
	{
	glcd->frame_buffer = fb;
	glcd->screen_width = glcd->display.width = width;
	glcd->screen_height = glcd->display.height = height;
	}

/* 
|  
*/
Glcd *
glcd_i420_init(void)
	{
	Glcd	*glcd;

	glcd = calloc(sizeof(Glcd), 1);

	glcd->set_pixel = i420_set_pixel;
	glcd->h_line = i420_h_line;
	glcd->v_line = i420_v_line;
	glcd->set_frame_buffer = i420_set_frame_buffer;

	return glcd;
	}
