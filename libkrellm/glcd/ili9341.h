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

#ifndef _ILI9341_H
#define _ILI9341_H

#ifdef	HAVE_WIRINGPI

#define	ILI9341_WIDTH	240
#define	ILI9341_HEIGHT	320


/* ILI9341 commands
*/

#define	ILI9341_SOFTWARE_RESET				0x01
#define	ILI9341_DISPLAY_OFF					0x28
#define	ILI9341_POWER_CONTROL_B				0xCF
#define ILI9341_POWER_ON_SEQUENCE			0xED
#define ILI9341_DRIVER_TIMING_A				0xE8
#define ILI9341_POWER_CONTROL_A				0xCB
#define ILI9341_PUMP_RATIO_CONTROL			0xF7
#define ILI9341_DRIVER_TIMING_B				0xEA
#define ILI9341_POWER_CONTROL_1				0xC0
#define ILI9341_POWER_CONTROL_2				0xC1
#define ILI9341_VCOM_CONTROL_1				0xC5
#define	ILI9341_VCOM_CONTROL_2				0xC7
#define ILI9341_PIXEL_FORMAT				0x3A
#define ILI9341_FRAME_RATE_CONTROL			0xB1
#define ILI9341_GAMMA_SET					0x26
#define ILI9341_ENTRY_MODE_SET				0xB7
#define ILI9341_DISPLAY_FUNCTION_CONTROL	0xB6
#define	ILI9341_SLEEP_OUT					0x11
#define ILI9341_DISPLAY_ON					0x29
#define ILI9341_COLUMN_ADDRESS_SET			0x2A
#define ILI9341_PAGE_ADDRESS_SET			0x2B
#define ILI9341_MEMORY_WRITE				0x2C
#define ILI9341_MEMORY_ACCESS_CONTROL		0x36
#define ILI9341_POSITIVE_GAMMA_CORRECTION	0xE0
#define ILI9341_NEGATIVE_GAMMA_CORRECTION	0xE1

#define ILI9341_MY	0x80
#define ILI9341_MX	0x40
#define ILI9341_MV	0x20
#define ILI9341_ML	0x10
#define ILI9341_BGR	0x08
#define ILI9341_MH	0x04

#define	LCD_LED_ON	1
#define	LCD_LED_OFF	0

typedef struct
	{
	int		speed,
			dc_pin,
			reset_pin,
			led_pin;
	}
	ili9341_LCD;


Glcd	*glcd_ili9341_spi_init(SpiDevice *spidev, uint32_t speed, int demux_BA,
								int dc_pin, int reset_pin, int led_pin);

#endif			/* HAVE_WIRINGPI  */
#endif			/* _ILI9341_H */
