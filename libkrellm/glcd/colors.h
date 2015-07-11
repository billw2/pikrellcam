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
/* 16 bit rgb colors (5/6/5) derived from:
|  http://en.wikipedia.org/wiki/X11_color_names
*/

#include <endian.h>

/* Raspberry Pi is by default little endian */

#if __BYTE_ORDER == __LITTLE_ENDIAN

#define AliceBlue			0xbfef
#define AntiqueWhite		0x3af7
#define Aqua				0xff07
#define Aquamarine			0xf97f
#define Azure				0xffef
#define Beige				0x9aef
#define Bisque				0x17ff
#define Black				0x0000
#define BlanchedAlmond		0x38ff
#define Blue				0x1f00
#define BlueViolet			0x5b81
#define Brown				0x44a1
#define Burlywood			0xb0d5
#define CadetBlue			0xf35c
#define Chartreuse			0xe07f
#define Chocolate			0x23cb
#define Coral				0xe9fb
#define Cornflower			0x9c64
#define Cornsilk			0xbaff
#define Crimson				0xa7d0
#define Cyan				0xff07
#define DarkBlue			0x1100
#define DarkCyan			0x5104
#define DarkGoldenrod		0x21b4
#define DarkGray			0x34a5
#define DarkGreen			0x0003
#define DarkKhaki			0xadb5
#define DarkMagenta			0x1188
#define DarkOliveGreen		0x4553
#define DarkOrange			0x40fc
#define DarkOrchid			0x9891
#define DarkRed				0x0088
#define DarkSalmon			0xaee4
#define DarkSeaGreen		0xd18d
#define DarkSlateBlue		0xf141
#define DarkSlateGray		0x692a
#define DarkTurquoise		0x7906
#define DarkViolet			0x1988
#define DeepPink			0xb1f8
#define DeepSkyBlue			0xff05
#define DimGray				0x2c63
#define DodgerBlue			0x7f1c
#define Firebrick			0x04a9
#define FloralWhite			0xbdff
#define ForestGreen			0x4424
#define Fuchsia				0x1ff8
#define Gainsboro			0xdad6
#define GhostWhite			0xbff7
#define Gold				0x80fe
#define Goldenrod			0x04d5
#define Gray				0xef7b
#define Green				0xe003
#define GreenYellow			0xe5af
#define Honeydew			0xfdef
#define HotPink				0x36fb
#define IndianRed			0xcbc2
#define Indigo				0x0f40
#define Ivory				0xfdff
#define Khaki				0x11ef
#define Lavender			0x1edf
#define LavenderBlush		0x7dff
#define LawnGreen			0xc07f
#define LemonChiffon		0xb8ff
#define LightBlue			0xbbae
#define LightCoral			0xefeb
#define LightCyan			0xffdf
#define LightGoldenrod		0xb9f7
#define LightGray			0x99ce
#define LightGreen			0x518f
#define LightPink			0x97fd
#define LightSalmon			0xeefc
#define LightSeaGreen		0x9425
#define LightSkyBlue		0x7e86
#define LightSlateGray		0x3274
#define LightSteelBlue		0x1aae
#define LightYellow			0xfbff
#define Lime				0xe007
#define LimeGreen			0x4636
#define Linen				0x7bf7
#define Magenta				0x1ff8
#define Maroon				0x0078
#define MediumAquamarine	0x5466
#define MediumBlue			0x1800
#define MediumOrchid		0x99b2
#define MediumPurple		0x7a8b
#define MediumSeaGreen		0x8d3d
#define MediumSlateBlue		0x3c73
#define MediumSpringGreen	0xb207
#define MediumTurquoise		0x7846
#define MediumVioletRed		0xb0c0
#define MidnightBlue		0xcd18
#define MintCream			0xfeef
#define MistyRose			0x1bff
#define Moccasin			0x16ff
#define NavajoWhite			0xd5fe
#define Navy				0x0f00
#define OldLace				0x9bf7
#define Olive				0xe07b
#define OliveDrab			0x646c
#define Orange				0x00fd
#define OrangeRed			0x20fa
#define Orchid				0x7ad3
#define PaleGoldenrod		0x34e7
#define PaleGreen			0xb297
#define PaleTurquoise		0x5caf
#define PaleVioletRed		0x71d3
#define PapayaWhip			0x7aff
#define PeachPuff			0xb6fe
#define Peru				0x07c4
#define Pink				0xf8fd
#define Plum				0xfad4
#define PowderBlue			0xfbae
#define Purple				0x0f78
#define Red					0x00f8
#define RosyBrown			0x71b4
#define RoyalBlue			0x3b3b
#define SaddleBrown			0x228a
#define Salmon				0xedf3
#define SandyBrown			0x0bed
#define SeaGreen			0x4a2c
#define Seashell			0x9cff
#define Sienna				0x859a
#define Silver				0xf7bd
#define SkyBlue				0x7c86
#define SlateBlue			0xd86a
#define SlateGray			0xf16b
#define Snow				0xbeff
#define SpringGreen			0xef07
#define SteelBlue			0x1644
#define Tan					0x91cd
#define Teal				0xef03
#define Thistle				0xfad5
#define Tomato				0x08fb
#define Turquoise			0xf93e
#define Violet				0x1ce4
#define Wheat				0xd5ee
#define White				0xffff
#define WhiteSmoke			0x9def
#define Yellow				0xe0ff
#define YellowGreen			0x4696

#else

#define AliceBlue			0xefbf
#define AntiqueWhite		0xf73a
#define Aqua				0x07ff
#define Aquamarine			0x7ff9
#define Azure				0xefff
#define Beige				0xef9a
#define Bisque				0xff17
#define Black				0x0000
#define BlanchedAlmond		0xff38
#define Blue				0x001f
#define BlueViolet			0x815b
#define Brown				0xa144
#define Burlywood			0xd5b0
#define CadetBlue			0x5cf3
#define Chartreuse			0x7fe0
#define Chocolate			0xcb23
#define Coral				0xfbe9
#define Cornflower			0x649c
#define Cornsilk			0xffba
#define Crimson				0xd0a7
#define Cyan				0x07ff
#define DarkBlue			0x0011
#define DarkCyan			0x0451
#define DarkGoldenrod		0xb421
#define DarkGray			0xa534
#define DarkGreen			0x0300
#define DarkKhaki			0xb5ad
#define DarkMagenta			0x8811
#define DarkOliveGreen		0x5345
#define DarkOrange			0xfc40
#define DarkOrchid			0x9198
#define DarkRed				0x8800
#define DarkSalmon			0xe4ae
#define DarkSeaGreen		0x8dd1
#define DarkSlateBlue		0x41f1
#define DarkSlateGray		0x2a69
#define DarkTurquoise		0x0679
#define DarkViolet			0x8819
#define DeepPink			0xf8b1
#define DeepSkyBlue			0x05ff
#define DimGray				0x632c
#define DodgerBlue			0x1c7f
#define Firebrick			0xa904
#define FloralWhite			0xffbd
#define ForestGreen			0x2444
#define Fuchsia				0xf81f
#define Gainsboro			0xd6da
#define GhostWhite			0xf7bf
#define Gold				0xfe80
#define Goldenrod			0xd504
#define Gray				0x7bef
#define Green				0x03e0
#define GreenYellow			0xafe5
#define Honeydew			0xeffd
#define HotPink				0xfb36
#define IndianRed			0xc2cb
#define Indigo				0x400f
#define Ivory				0xfffd
#define Khaki				0xef11
#define Lavender			0xdf1e
#define LavenderBlush		0xff7d
#define LawnGreen			0x7fc0
#define LemonChiffon		0xffb8
#define LightBlue			0xaebb
#define LightCoral			0xebef
#define LightCyan			0xdfff
#define LightGoldenrod		0xf7b9
#define LightGray			0xce99
#define LightGreen			0x8f51
#define LightPink			0xfd97
#define LightSalmon			0xfcee
#define LightSeaGreen		0x2594
#define LightSkyBlue		0x867e
#define LightSlateGray		0x7432
#define LightSteelBlue		0xae1a
#define LightYellow			0xfffb
#define Lime				0x07e0
#define LimeGreen			0x3646
#define Linen				0xf77b
#define Magenta				0xf81f
#define Maroon				0x7800
#define MediumAquamarine	0x6654
#define MediumBlue			0x0018
#define MediumOrchid		0xb299
#define MediumPurple		0x8b7a
#define MediumSeaGreen		0x3d8d
#define MediumSlateBlue		0x733c
#define MediumSpringGreen	0x07b2
#define MediumTurquoise		0x4678
#define MediumVioletRed		0xc0b0
#define MidnightBlue		0x18cd
#define MintCream			0xeffe
#define MistyRose			0xff1b
#define Moccasin			0xff16
#define NavajoWhite			0xfed5
#define Navy				0x000f
#define OldLace				0xf79b
#define Olive				0x7be0
#define OliveDrab			0x6c64
#define Orange				0xfd00
#define OrangeRed			0xfa20
#define Orchid				0xd37a
#define PaleGoldenrod		0xe734
#define PaleGreen			0x97b2
#define PaleTurquoise		0xaf5c
#define PaleVioletRed		0xd371
#define PapayaWhip			0xff7a
#define PeachPuff			0xfeb6
#define Peru				0xc407
#define Pink				0xfdf8
#define Plum				0xd4fa
#define PowderBlue			0xaefb
#define Purple				0x780f
#define Red					0xf800
#define RosyBrown			0xb471
#define RoyalBlue			0x3b3b
#define SaddleBrown			0x8a22
#define Salmon				0xf3ed
#define SandyBrown			0xed0b
#define SeaGreen			0x2c4a
#define Seashell			0xff9c
#define Sienna				0x9a85
#define Silver				0xbdf7
#define SkyBlue				0x867c
#define SlateBlue			0x6ad8
#define SlateGray			0x6bf1
#define Snow				0xffbe
#define SpringGreen			0x07ef
#define SteelBlue			0x4416
#define Tan					0xcd91
#define Teal				0x03ef
#define Thistle				0xd5fa
#define Tomato				0xfb08
#define Turquoise			0x3ef9
#define Violet				0xe41c
#define Wheat				0xeed5
#define White				0xffff
#define WhiteSmoke			0xef9d
#define Yellow				0xffe0
#define YellowGreen			0x9646

#endif
