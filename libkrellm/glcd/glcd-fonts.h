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
#ifndef GLCD_FONTS_H
#define GLCD_FONTS_H

/* XXX */
//#include "../../pikrellm.h"

#include <inttypes.h>

typedef struct GlcdFont
	{
	uint8_t	char_width,
			char_height,
			first_char,
			n_chars;

	const unsigned char *bitmap;
	}
	GlcdFont;


extern GlcdFont	font_9x15;
extern GlcdFont	font_12x24;
extern GlcdFont	font_shadow_bold;

#endif
