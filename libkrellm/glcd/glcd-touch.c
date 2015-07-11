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

#include <wiringPi.h>

#include "glcd.h"
#include "utils.h"

static void		touch_read_conversion_soft(GlcdTouch *touch, boolean save);
static boolean	touch_data_available(GlcdTouch *touch);


static void
touch_read_conversion_hard(GlcdTouch *touch, boolean save)
	{
	uint8_t		buf[3];
	uint32_t	x = 0,
				y = 0;

	if (save)
		{
		pinMode(touch->IRQ_pin, OUTPUT);
		digitalWrite(touch->IRQ_pin, LOW);
		}
	buf[0] = 0xD0;
	spi_rw(touch->spi_device, touch->spi_demux_BA, buf, 3);
	x = (buf[1] << 2) | (buf[2] >> 6);

	buf[0] = 0x90;
	spi_rw(touch->spi_device, touch->spi_demux_BA, buf, 3);
	y = (buf[1] << 2) | (buf[2] >> 6);

	if (save)
		{
		touch->x_raw = x;
		touch->y_raw = y;

		pinMode(touch->IRQ_pin, INPUT);
		pullUpDnControl(touch->IRQ_pin, PUD_UP);
		}

	/* Write a PD1,PD0 = 0 control byte to put ADS/XPT in power down mode.
	|  For some reason, having the power down bits 0 in the above x,y
	|  conversion reads is not enough and the PENIRQ line will not respond
	|  to touches if there is a delay before the next read conversion.
	*/
	buf[0] = 0x0;
	spi_rw(touch->spi_device, touch->spi_demux_BA, buf, 3);
	}

void
glcd_touch_spi_soft_init(Glcd *glcd, int DIN_pin, int DOUT_pin,
			int CS_pin, int CLK_pin, int IRQ_pin)
	{
	GlcdTouch	*touch		= &glcd->touch;

	touch->spi_soft.DIN_pin = DIN_pin;
	pinMode(touch->spi_soft.DIN_pin, OUTPUT);
	digitalWrite(touch->spi_soft.DIN_pin, HIGH);

	touch->spi_soft.DOUT_pin = DOUT_pin;
	pinMode(touch->spi_soft.DOUT_pin, INPUT);

	touch->spi_soft.CS_pin = CS_pin;
	pinMode(touch->spi_soft.CS_pin, OUTPUT);
	digitalWrite(touch->spi_soft.CS_pin, HIGH);

	touch->spi_soft.CLK_pin = CLK_pin;
	pinMode(touch->spi_soft.CLK_pin, OUTPUT);
	digitalWrite(touch->spi_soft.CLK_pin, LOW);

	touch->IRQ_pin = IRQ_pin;
	pinMode(touch->IRQ_pin, INPUT);

	touch->convert_count = 1;
	touch->x_max = 3750;
	touch->x_min = 300;
	touch->y_max = 3440;
	touch->y_min = 215;

	touch->read_conversion = touch_read_conversion_soft;
	touch->data_available  = touch_data_available;
	}


void
glcd_touch_spi_hard_init(Glcd *glcd, SpiDevice *spidev, uint32_t speed,
				int demux_BA, int IRQ_pin)
	{
	GlcdTouch	*touch		= &glcd->touch;
	uint8_t		buf[3];

	touch->spi_device = spidev;
	touch->spi_demux_BA = demux_BA;
	spi_device_speed_set(spidev, speed, demux_BA);

	touch->IRQ_pin = IRQ_pin;
	pinMode(touch->IRQ_pin, INPUT);
	pullUpDnControl(touch->IRQ_pin, PUD_UP);

	touch->convert_count = 1;

	touch->calibrate_matrix.An = 5376;
	touch->calibrate_matrix.Bn = -192;
	touch->calibrate_matrix.Cn = -306336;
	touch->calibrate_matrix.Dn = -256;
	touch->calibrate_matrix.En = -8192;
	touch->calibrate_matrix.Fn = 56960;
	touch->calibrate_matrix.divider = -1196;

	touch->read_conversion = touch_read_conversion_hard;
	touch->data_available  = touch_data_available;

	buf[0] = 0x0;
	spi_rw(touch->spi_device, touch->spi_demux_BA, buf, 3);
	}

void
glcd_touch_read_conversion(Glcd *glcd, boolean save)
	{
	GlcdTouch	*touch = &glcd->touch;

	if (touch->read_conversion)
		(*touch->read_conversion)(touch, save);
	}

boolean
glcd_touch_data_available(Glcd *glcd)
	{
	GlcdTouch	*touch = &glcd->touch;

	if (touch->data_available)
		return (*touch->data_available)(touch);
	return FALSE;
	}

void
glcd_touch_irq_connect(Glcd *glcd, void (*handler)())
	{
	wiringPiISR(glcd->touch.IRQ_pin, INT_EDGE_FALLING, handler);
//	attachInterrupt(glcd->touch.IRQ_pin, handler, FALLING);
	}

void
glcd_touch_irq_disconnect(Glcd *glcd)
	{
//	detachInterrupt(glcd->touch.IRQ_pin);
	}

void
glcd_touch_set_calibrate(GlcdTouch *touch,
			int xmin, int xmax, int ymin, int ymax)
	{
	touch->x_min = xmin;
	touch->x_max = xmax;
	touch->y_min = ymin;
	touch->y_max = ymax;
	}

/* ADS7843 Conversion: Write a control 8 bit byte which selects 12 bit mode
|  conversion and then read a 12 bit result.
|  Control byte:
|		S A2 A1 A0 MODE SER/DFR PD1 PD0
|	    1  0  0  1  0     0       0   0		= 0x90
|	    1  1  0  1  0     0       0   0		= 0xD0
|
|		S is start bit
|		SER/DFR low selects ratiometric mode.
|		0x90 command connects:	(data sheet fig 4)
|			Y+ to +REF
|			X+ to +IN
|			Y- to -IN and -REF
|		So voltage at X+ is converted as value between Y+ and Y-.
|
|		0xD0 command connects:
|			X+ to +REF
|			Y+ to +IN
|			X- to -IN and -REF
|		So voltage at Y+ is converted as value between X+ and X-.
|
|		MODE 0 selects 12 bit conversion
|		PD1 PD0 zero selects power down mode with IRQ enabled.
|
|		| S | A2| A1| A0| M | SD|PD1|PD0|       | 11| 10| 9|...
|		  _   _   _   _   _   _   _  a_       b_c d_   _
|		_| |_| |_| |_| |_| |_| |_| |_| |______| |_| |_| |_...
|        :   :   :   :   :   :   :   :        : : :   :   : ...
| DIN    ^   ^   ^   ^   ^   ^   ^   ^        : : :   :   : ...
| DOUT                                        : : ^   ^   ^ ...
|                                       ________:
| BUSY                          _______|        |______________
|
| A hardware SPI interface would latch the final control write bit at rising
| clock edge 'a', skip rising clock edge 'b' where BUSY is asserted and then
| start latching read bits at rising clock edge 'd' (first read bit data is
| asserted after falling clock edge 'c' along with deassertion of BUSY).
| Thus the interface is BUSY for one cycle and this is the extra clock cycle
| the spec sheet (FIXME: provide link?) talks about.
|
| This software interface may seem to not account for this, but it actually
| does because a cycle is effectively inserted when we transition from
| writing control data before a rising clock to then reading result data
| after a falling clock. Our first read bit happens at a "fuzzy" but after
| DCLK Falling to DOUT Valid time after falling clock edge 'c'.  So the
| final clock rising edge a hardware interface would present is not
| necessary in software.
|
| We also don't finish clocking out the zero fill 4 least significant bits
| that a hardware interface would clock..
| Just raise CS after 12 bits to abort the cycle and then we can start a
| new transfer.
*/


  /* ADS7843 write cycle timing:			MIN		MAX
  |		DIN Valid Prior to DCLK Rising		100ns
  |		DIN Hold After DCLK HIGH			10ns
  |		DCLK HIGH							200ns
  |		DCLK LOW							200ns
  |
  */
static void
touch_write_control_soft(GlcdTouch *touch, uint8_t control)
	{
	GlcdSpiSoft	*spi_soft = &touch->spi_soft;;
	int			i;
	uint8_t		mask = 0x80;

	for (i = 0; i < 8; ++i)
		{
		digitalWrite(spi_soft->DIN_pin, (mask & control) ? HIGH : LOW);
		digitalWrite(spi_soft->CLK_pin, HIGH);	/* DIN clocked on rising edge	*/
		/* Might need a delay here for 168MHz STM32F4?? */
		digitalWrite(spi_soft->CLK_pin, LOW);
		mask >>= 1;
		}
	}


  /* ADS7843 read cycle timing:				MIN		MAX
  |		CS Falling to DOUT Enabled					200ns
  |		DCLK Falling to DOUT Valid                  200ns
  |		DCLK HIGH							200ns
  |		DCLK LOW							200ns
  */
static uint16_t
read_conversion_soft(GlcdTouch *touch, boolean save)
	{
	GlcdSpiSoft	*spi_soft = &touch->spi_soft;;
	int			i;
	uint16_t	data = 0;

	/* Clock out the 12 significant bits of the conversion result.
	*/
	for (i = 0; i < 12; ++i)
		{
		data <<= 1;
		digitalWrite(spi_soft->CLK_pin, HIGH);
		/* Might need a delay here for 168MHz STM32F4?? */
		digitalWrite(spi_soft->CLK_pin, LOW); /* DOUT valid after falling clock */
		if (digitalRead(spi_soft->DOUT_pin))
			data += 1;
		}
	return data;
	}

static void
touch_read_conversion_soft(GlcdTouch *touch, boolean save)
	{
	int			i, count;
	uint16_t	xt, yt;
	uint32_t	x = 0, y = 0;

//	if (!touch_initialized)
//		return;

	touch->x_raw = touch->y_raw = 0;
	digitalWrite(touch->spi_soft.CS_pin, LOW);

	for (count = 0, i = 0; i < touch->convert_count; i++)
		{
		touch_write_control_soft(touch, 0x90);
		yt = read_conversion_soft(touch, save);

		touch_write_control_soft(touch, 0xD0);
		xt = read_conversion_soft(touch, save);

		if (   xt > touch->x_min && xt < touch->x_max
		    && yt > touch->y_min && yt < touch->y_max
		   )
			{
			y += yt;
			x += xt;
			++count;
			}
		}
	digitalWrite(touch->spi_soft.CS_pin, HIGH);

	if (count > 0)
		{
		touch->x_raw = x / count;
		touch->y_raw = y / count;
		}
	}

static boolean
touch_data_available(GlcdTouch *touch)
	{
	return !digitalRead(touch->IRQ_pin);
	}

int
glcd_touch_get_display_x(Glcd *glcd)
	{
	GlcdTouch	*touch			= &glcd->touch;
	int			display_width	= glcd_get_display_width(glcd);
	int			rot				= glcd_get_rotation(glcd);
	int 		x;

	if (rot == 0)
		x = display_width *
				(touch->x_raw - touch->x_min) / (touch->x_max - touch->x_min);
	else
		x = display_width - display_width *
				(touch->y_raw - touch->y_min) / (touch->y_max - touch->y_min);

	if (x < 0)
		x = 0;
	if (x > display_width - 1)
		x = display_width - 1;
	return x;
	}

int
glcd_touch_get_display_y(Glcd *glcd)
	{
	GlcdTouch	*touch			= &glcd->touch;
	int			display_height	= glcd_get_display_height(glcd);
	int			rot				= glcd_get_rotation(glcd);
	int 		y;

	if (rot == 0)
		y = display_height *
				(touch->y_raw - touch->y_min) / (touch->y_max - touch->y_min);
	else
		y = display_height *
				(touch->x_raw - touch->x_min) / (touch->x_max - touch->x_min);

	if (y < 0)
		y = 0;
	if (y > display_height - 1)
		y = display_height - 1;

	return y;
	}


int
glcd_touch_get_raw_x(Glcd *glcd)
	{
	GlcdTouch	*touch			= &glcd->touch;

	return touch->x_raw;
	}


int
glcd_touch_get_raw_y(Glcd *glcd)
	{
	GlcdTouch	*touch			= &glcd->touch;

	return touch->y_raw;
	}

void
glcd_touch_set_convert_count(GlcdTouch *touch, int count)
	{
	if (count > 0 && count <= 50)
		touch->convert_count = count;
	}

#endif /* HAVE_WIRINGPI */
