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

#ifndef GLCD_H
#define GLCD_H

#include "utils.h"
#include "colors.h"
#include "glcd-fonts.h"

#define	TRUE	1
#define	FALSE	0

typedef		int	boolean;


/* Known graphics LCD models
*/
#define	GLCD_GENERIC		0
#define GLCD_TFT01_32		1	/* Elec Freaks LCDs */
#define GLCD_TFT01_32W		2
#define GLCD_TFT01_22S		3
#define GLCD_ITDB02_32S		4	/* Iteadstudio LCDs */
#define GLCD_ITDB02_32WC	5

/* And supported controllers
*/
#define	ILI9341_SPI		1

#define	SSD1289		2
#define	ILI9327		3
#define	ILI9325		4


#define GLCD_MODE_UNDEFINED	0
#define GLCD_MODE_SPI_SOFT		1
#define GLCD_MODE_SPI_HARD		2

typedef struct
	{
	int32_t	x,
			y;
	}
	TouchCalibratePoint;


/* From Vidales calibrate.c
*/

typedef struct
	{
	int32_t	An,
			Bn,
			Cn,
			Dn,
			En,
			Fn,
			divider;
	}
	TouchCalibrateMatrix;


typedef struct
	{
	int		DIN_pin,
			DOUT_pin,
			CS_pin,
			CLK_pin;
	}
	GlcdSpiSoft;

typedef struct _glcdtouch
	{
	SpiDevice	*spi_device;
	int			spi_demux_BA;
	GlcdSpiSoft	spi_soft;
	int			IRQ_pin;

	uint16_t	x_raw,		/* 12 bits precision */
				y_raw;
	int			x_min,
				x_max,
				y_min,
				y_max;
	int			convert_count;

	TouchCalibrateMatrix
				calibrate_matrix;

	void		(*read_conversion)(struct _glcdtouch *glcd_touch, boolean save);
	boolean		(*data_available)(struct _glcdtouch *glcd_touch);
	}
	GlcdTouch;

typedef struct
	{
	uint8_t		border;
	uint16_t	border_color;
	uint16_t	bg_color;
	int			x0, y0,
				width, height;
	}
	DrawArea;

typedef struct _glcd
	{
	SpiDevice	*spi_device;
	int			spi_demux_BA;
	int			dc_pin,
				reset_pin,
				led_pin;
	int			screen_width,
				screen_height;
	DrawArea	display;
	int			rotation;

	GlcdTouch	touch;

	void		(*led_control)(struct _glcd *glcd, int state);
	void		(*set_address_window)(struct _glcd *glcd,
								int x0, int y0, int x1, int y1);
	void		(*set_pixel)(struct _glcd *glcd, uint16_t color, int x, int y);
	void		(*h_line)(struct _glcd *glcd, uint16_t color,
								int x, int y, int dx);
	void		(*v_line)(struct _glcd *glcd, uint16_t color,
								int x, int y, int dy);
	void		(*write_data)(uint16_t data);
	void		(*set_rotation)(struct _glcd *glcd, int rotation);
	void		(*set_frame_buffer)(struct _glcd *glcd,
								uint16_t * fb, int width, int height);
	void		(*frame_buffer_update)(struct _glcd *glcd);

	uint16_t	*frame_buffer;
	}
	Glcd;

typedef struct
	{
	uint16_t	width,
				height;
	uint16_t	*data;
	}
	GlcdImage;


#define DRAW_AREA(cp)   ((DrawArea *) (cp))


//void	glcd_dbi_init(uint8_t model, uint8_t CS, uint8_t DC, uint8_t RD,
//				uint8_t WR, uint8_t RESET, gpio_dev *DATA_dev);

Glcd	*glcd_i420_init(void);

void	glcd_led(Glcd *glcd, int state);

int		glcd_get_screen_height(Glcd *glcd);
int		glcd_get_screen_width(Glcd *glcd);
int		glcd_get_display_height(Glcd *glcd);
int		glcd_get_display_width(Glcd *glcd);
DrawArea *glcd_get_display_area(Glcd *glcd);
int		glcd_get_rotation(Glcd *glcd);
void	glcd_set_rotation(Glcd *glcd, int rotation);
void	glcd_set_frame_buffer(Glcd *glcd, void *fb, int width, int height);

uint16_t glcd_map_color(uint8_t r, uint8_t g, uint8_t b);
uint16_t glcd_map_color_percent(int r_percent, int g_percent, int b_percent);

void	glcd_draw_pixel(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0);
void	glcd_draw_line(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0, int x1, int y1);
void	glcd_draw_h_line(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0, int len);
void	glcd_draw_v_line(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0, int len);


void	glcd_draw_rectangle(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0, int width, int height);
void	glcd_fill_rectangle(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0, int width, int height);
void	glcd_draw_rounded_rectangle(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0, int width, int height, int radius);
void	glcd_fill_rounded_rectangle(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0, int width, int height, int radius);
void	glcd_draw_circle(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0, int radius);
void	glcd_fill_circle(Glcd *glcd, DrawArea *da, uint16_t color,
					int x0, int y0, int radius);
void	glcd_fill_screen(Glcd *glcd, uint16_t color);


void	glcd_print_string(Glcd *glcd, DrawArea *da, GlcdFont *font,
					uint16_t color, boolean clear, int row, char *string);
int		glcd_draw_string(Glcd *glcd, DrawArea *da, GlcdFont *font,
					uint16_t color, int x0, int y0, char *string);
int		glcd_draw_string_rotated(Glcd *glcd, DrawArea *pA, GlcdFont *font,
					uint16_t color, int degree, int x0, int y0, char *string);

void	glcd_draw_pixmap(Glcd *glcd, DrawArea *pA, uint16_t *pixmap,
					int x0, int y0, int width, int height);
void	glcd_draw_image(Glcd *glcd, DrawArea *pA, GlcdImage *im,
					int x0, int y0);


void	glcd_touch_spi_soft_init(Glcd *glcd, int DIN_pin, int DOUT_pin,
					int CS_pin, int CLK_pin, int IRQ_pin);
void	glcd_touch_spi_hard_init(Glcd *glcd, SpiDevice *spidev, uint32_t speed,
					int demux_BA, int IRQ_pin);
void	glcd_touch_irq_connect(Glcd *glcd, void (*handler)());
void	glcd_touch_irq_disconnect(Glcd *glcd);
void	glcd_touch_set_calibrate(GlcdTouch *touch,
					int xmin, int xmax, int ymin, int ymax);
void	glcd_touch_read_conversion(Glcd *glcd, boolean save);
boolean	glcd_touch_data_available(Glcd *glcd);
int		glcd_touch_get_display_x(Glcd *glcd);
int		glcd_touch_get_display_y(Glcd *glcd);
int		glcd_touch_get_raw_x(Glcd *glcd);
int		glcd_touch_get_raw_y(Glcd *glcd);
void	glcd_touch_set_convert_count(GlcdTouch *touch, int count);

void	touch_calibrate_screen(Glcd *glcd);
boolean	touch_set_calibrate_matrix(
					TouchCalibratePoint *display, TouchCalibratePoint *raw,
					TouchCalibrateMatrix *matrix);
int		touch_calibrate_point(
					TouchCalibratePoint *display, TouchCalibratePoint *raw,
					TouchCalibrateMatrix *matrix);


#endif
