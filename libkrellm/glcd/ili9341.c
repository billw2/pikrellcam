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

#ifdef	HAVE_WIRINGPI

#include <stdio.h>
#include <stdarg.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "glcd.h"
#include "utils.h"
#include "ili9341.h"


uint16_t	frame_buffer[240 * 320];



/* Number of varargs passed, works only for int args.
|  See:  http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
|        http://smackerelofopinion.blogspot.com/2011/10/determining-number-of-arguments-in-c.html
*/
#define NUM_VARARGS(...) (int)(sizeof((int[]){0, ##__VA_ARGS__})/sizeof(int)-1)

#define ili9341_command(glcd, command, ...) \
	_ili9341_command(glcd, command, NUM_VARARGS(__VA_ARGS__), ##__VA_ARGS__)


static void
_ili9341_command(Glcd *glcd, int command, int n_params, ...)
	{
	va_list		args;
    int			i;
	uint8_t		cbuf[2], buf[32];

//	printf("0x%x:  ", command);

	cbuf[0] = (uint8_t) command;

	va_start(args, n_params);
	for (i = 0; i < n_params; ++i)
		{
		buf[i] = (uint8_t) va_arg(args, unsigned int);
//		printf("%02x ", buf[i]);
		}
	va_end(args);
//	printf("\n");

	digitalWrite(glcd->dc_pin, LOW);
	spi_write(glcd->spi_device, glcd->spi_demux_BA, cbuf, 1);

	digitalWrite(glcd->dc_pin, HIGH);
	if (n_params > 0)
		spi_write(glcd->spi_device, glcd->spi_demux_BA, buf, n_params);
	}

static void
ili9341_set_address_window(Glcd *glcd, int x0, int y0, int x1, int y1)
	{
	ili9341_command(glcd, 0x2A /* ILI9341_COLUMN_ADDRESS_SET */,
			(x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF);
	ili9341_command(glcd, 0x2B /* ILI9341_PAGE_ADDRESS_SET */,
			(y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF);

	ili9341_command(glcd, 0x2C /* ILI9341_MEMORY_WRITE */);
	}


static void
ili9341_set_pixel(Glcd *glcd, uint16_t color, int x, int y)
	{
	uint16_t	*p = glcd->frame_buffer;
	int			offset;

	offset = y * glcd->display.width + x;
	*(p + offset) = color;
	}

static void
ili9341_h_line(Glcd *glcd, uint16_t color, int x, int y, int dx)
	{
	uint16_t	*p = glcd->frame_buffer;
	int			offset;

	offset = y * glcd->display.width + x;
	while (dx--)
		*(p + offset++) = color;
	}

static void
ili9341_v_line(Glcd *glcd, uint16_t color, int x, int y, int dy)
	{
	uint16_t	*p = glcd->frame_buffer;
	int			offset;

	offset = y * glcd->display.width + x;
	while (dy--)
		{
		*(p + offset) = color;
		offset += glcd->display.width;
		}
	}

static void
ili9341_write_data(uint16_t color)
	{
	}

static void
ili9341_frame_buffer_update(Glcd *glcd)
	{
	uint8_t	*fb8 = (uint8_t *) glcd->frame_buffer;
	uint8_t	*p, *plast;
	int		pixels = glcd->display.width * glcd->display.height;

	ili9341_set_address_window(glcd, 0, 0,
				glcd->display.width - 1, glcd->display.height - 1);

	/* SPI max IO transfer size defaults to one page size (4096)
	*/
	plast = fb8 + 2 * pixels;
	for (p = fb8; p < plast - 4096; p += 4096)
		spi_write(glcd->spi_device, glcd->spi_demux_BA, p, 4096);
	if (p < plast)
		spi_write(glcd->spi_device, glcd->spi_demux_BA, p, plast - p);
	}

static void
ili9341_set_rotation(Glcd *glcd, int rotation)
	{
	unsigned int	param = ILI9341_MX;		/* Default zero rotation */

	if (rotation == 90)
		param = ILI9341_MY | ILI9341_MX | ILI9341_MV;
	else if (rotation == 180)
		param = ILI9341_MY;
	else if (rotation == 270)
		param = ILI9341_MV | ILI9341_ML;

	param |= ILI9341_BGR;

	ili9341_command(glcd, 0x36 /* ILI9341_MEMORY_ACCESS_CONTROL */,
			param);
	}

static void
ili9341_led(Glcd *glcd, int state)
	{
	if (state == LCD_LED_ON)
		digitalWrite(glcd->led_pin, LOW);
	else
		digitalWrite(glcd->led_pin, HIGH);
	}


/* ILI9341 SCL read cycle min is 150 nsec or 6.66 MHz.  SPI clock on the
|  RPI is settable in steps of 2, 4, 8, 16, and 32 MHz, so use 4 MHz.
*/
Glcd *
glcd_ili9341_spi_init(SpiDevice *spidev, uint32_t speed, int demux_BA,
								int dc_pin, int reset_pin, int led_pin)
	{
	Glcd	*glcd;
	int		i, j, t0, t1;

	glcd = calloc(sizeof(Glcd), 1);

	if (!spidev)
		{
		printf("ili9341_spi_init() NULL spidev\n");
		return NULL;
		}
	glcd->spi_device = spidev;
	glcd->spi_demux_BA = demux_BA;
	spi_device_speed_set(spidev, speed, demux_BA);

	glcd->screen_width = glcd->display.width = ILI9341_WIDTH;
	glcd->screen_height = glcd->display.height = ILI9341_HEIGHT;

	glcd->led_control = ili9341_led;
	glcd->set_rotation = ili9341_set_rotation;
	glcd->set_address_window = ili9341_set_address_window;
	glcd->set_pixel = ili9341_set_pixel;
	glcd->h_line = ili9341_h_line;
	glcd->v_line = ili9341_v_line;
	glcd->write_data = ili9341_write_data;
	glcd->frame_buffer_update = ili9341_frame_buffer_update;

	glcd->frame_buffer =
				calloc(ILI9341_WIDTH * ILI9341_HEIGHT, sizeof(uint16_t));

	pinMode(dc_pin, OUTPUT);
	if (reset_pin >= 0)
		{
		pinMode(reset_pin, OUTPUT);
		digitalWrite(reset_pin, HIGH);
		}
	pinMode(led_pin, OUTPUT);

	glcd->dc_pin = dc_pin;
	glcd->reset_pin = reset_pin;
	glcd->led_pin = led_pin;


	ili9341_command(glcd, 0x01 /* ILI9341_SOFTWARE_RESET */);
	delay(5);

	ili9341_command(glcd, 0x28 /* ILI9341_DISPLAY_OFF */);

	ili9341_command(glcd, 0xCF /* ILI9341_POWER_CONTROL_B */,
				0x00, 0x83, 0x30);

	ili9341_command(glcd, 0xED /* ILI9341_POWER_ON_SEQUENCE */,
				0x64, 0x03, 0x12, 0x81);

	ili9341_command(glcd, 0xE8 /* ILI9341_DRIVER_TIMING_A */,
				0x85, 0x01, 0x79);

	ili9341_command(glcd, 0xCB /* ILI9341_POWER_CONTROL_A */,
				0x39, 0x2C, 0x00, 0x34, 0x02);

	ili9341_command(glcd, 0xF7 /* ILI9341_PUMP_RATIO_CONTROL */,
				0x20);

	ili9341_command(glcd, 0xEA /* ILI9341_DRIVER_TIMING_B */,
				0x00, 0x00);

	ili9341_command(glcd, 0xC0 /* ILI9341_POWER_CONTROL_1 */,
				0x26);

	ili9341_command(glcd, 0xC1 /* ILI9341_POWER_CONTROL_2 */,
				0x11);

	ili9341_command(glcd, 0xC5 /* ILI9341_VCOM_CONTROL_1 */,
				0x35, 0x3E);

	ili9341_command(glcd, 0xC7 /* ILI9341_VCOM_CONTROL_2 */,
				0xBE);

	ili9341_command(glcd, 0x3A /* ILI9341_PIXEL_FORMAT */,
				0x55);     /* 16bit pixel */

	ili9341_command(glcd, 0xB1 /* ILI9341_FRAME_RATE_CONTROL */,
				0x00, 0x1B);

	ili9341_command(glcd, 0x26 /* ILI9341_GAMMA_SET */,
				0x01);	/* gamma curve 1 */

	ili9341_command(glcd, 0xE0 /* ILI9341_POSITIVE_GAMMA_CORRECTION */,
//			0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
//			0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00);
			0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87,
			0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00);

	ili9341_command(glcd, 0xE1 /* ILI9341_NEGATIVE_GAMMA_CORRECTION */,
//			0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
//			0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F);
			0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78,
			0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F);

	ili9341_command(glcd, 0xB7 /* ILI9341_ENTRY_MODE_SET */,
			0x07);

	ili9341_command(glcd, 0xB6 /* ILI9341_DISPLAY_FUNCTION_CONTROL */,
			0x0A, 0x82, 0x27, 0x00);

	ili9341_command(glcd, 0x11 /* ILI9341_SLEEP_OUT */);
	delay(100);

	ili9341_command(glcd, 0x29 /* ILI9341_DISPLAY_ON */);
	delay(20);



	/* ********** start temporary SPI speed test */

	ili9341_set_address_window(glcd, 0, 0, 239, 319);
	t0 = millis();

	/* SPI max IO transfer size defaults to one page size (4096)
	*/
	for (i = 0; i < 16; ++i)
		{
		for (j = 0; j < 240 * 320; ++j)
			glcd->frame_buffer[j] = glcd_map_color_percent(5, 5, 5);
		ili9341_frame_buffer_update(glcd);
		}
	t1 = millis();
	printf("frame rate = %.2f\n", 1.0 / (float) (t1 - t0) * 16.0 * 1000);
	/* ********** end temporary SPI speed test */


	return glcd;
	}

#endif /* WIRINGPI */
