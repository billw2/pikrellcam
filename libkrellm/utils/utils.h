/* libkrellm/utils
|
|  Copyright (C) 2013-2015 Bill Wilson   billw@gkrellm.net
|
|  libkrellm/utils is free software: you can redistribute it and/or modify
|  it under the terms of the GNU General Public License as published by
|  the Free Software Foundation, either version 3 of the License, or
|  (at your option) any later version.
|
|  libkrellm/utils is distributed in the hope that it will be useful,
|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|  GNU General Public License for more details.
|
|  You should have received a copy of the GNU General Public License
|  along with the libkrellm.  If not, see <http://www.gnu.org/licenses/>.
|
*/

#ifndef _UTILS_H_
#define _UTILS_H_

#include <inttypes.h>

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifndef TRUE
#define	TRUE	1
#define FALSE	0
#endif

#ifndef MAX
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif


typedef struct _SList   SList;
typedef int				boolean;

struct _SList
	{
	void  *data;
	SList *next;
	};


typedef struct
	{
	int			fd;
	uint32_t	speed[4];
	boolean		demux_enabled;
	int			current_demux_BA;
	int			demux_B_pin,
				demux_A_pin;
	}
	SpiDevice;

#ifdef __cplusplus
extern "C" {
#endif

int			micro_elapsed_time(struct timeval *timer);
boolean		dup_string(char **dst, char *src);

boolean		isfifo(char *fifo);
boolean		isdir(char *dir);
boolean		make_directory(char *dir);

SpiDevice	*spi_device_open(char *devname, uint32_t speed,
						int B_pin, int A_pin);
void		spi_device_speed_set(SpiDevice *spidev, uint32_t speed,
						int demux_BA);
boolean		spi_read(SpiDevice *spidev, int demux_BA,
						uint8_t *data, int length);
boolean		spi_write(SpiDevice *spidev, int demux_BA,
						uint8_t *data, int length);
boolean		spi_rw(SpiDevice *spidev, int demux_BA,
						uint8_t *data, int length);


int		i2c_open(int i2c_address);


void	slist_free(SList *list);
void	slist_and_data_free(SList *list);
SList	*slist_append(SList *list, void *data);
SList	*slist_prepend(SList *list, void *data);
SList	*slist_remove(SList *list, void *data);
SList	*slist_remove_link(SList *list, SList *link);
SList	*slist_nth(SList *list, int  n);
void	*slist_nth_data(SList *list, int n);
SList	*slist_find(SList *list, void *data);
int		slist_length(SList *list);

#ifdef __cplusplus
}
#endif

#endif  /* _UTILS_H_ */
