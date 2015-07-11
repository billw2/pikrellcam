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

#ifdef HAVE_WIRINGPI

#include <stdio.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>

#include "wiringPi.h"

#include "utils.h"


static uint8_t	spi_mode	= 0,
				spi_bpw		= 8;
static uint16_t	spi_delay	= 0;



SpiDevice *
spi_device_open(char *devname, uint32_t speed, int B_pin, int A_pin)
	{
	SpiDevice	*spidev;
	int			i;

	spidev = (SpiDevice *) calloc(1, sizeof(SpiDevice));

	if ((spidev->fd = open(devname, O_RDWR)) < 0)
		{
		printf("%s open failed: %s\n", devname, strerror(errno));
		free(spidev);
		return NULL;
		}
	for (i = 0; i < 4; ++i)
		spidev->speed[i] = speed;
	spidev->demux_B_pin = B_pin;
	spidev->demux_A_pin = A_pin;
	spidev->demux_enabled = FALSE;

	if (A_pin >= 0)
		{
		spidev->demux_enabled = TRUE;
		pinMode(A_pin, OUTPUT);
		digitalWrite(A_pin, LOW);
		}
	if (B_pin >= 0)
		{
		spidev->demux_enabled = TRUE;
		pinMode(B_pin, OUTPUT);
		digitalWrite(B_pin, LOW);
		}
	spidev->current_demux_BA = 0;

    if (ioctl(spidev->fd, SPI_IOC_WR_MODE, &spi_mode) < 0)
		printf("%s set mode failed: %s\n", devname, strerror(errno));

    if (ioctl(spidev->fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bpw) < 0)
		printf("%s set bpw failed: %s\n", devname, strerror(errno));

    if (ioctl(spidev->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
		printf("%s set speed failed: %s\n", devname, strerror(errno));

    return spidev;
	}

void
spi_device_speed_set(SpiDevice *spidev, uint32_t speed, int demux_BA)
	{
	if (spidev && spidev->demux_enabled && demux_BA >= 0 && demux_BA < 4)
		{
		spidev->speed[demux_BA] = speed;
		}
	}

static boolean
spi_demux_select(SpiDevice *spidev, int demux_BA)
	{
	if (!spidev || demux_BA < 0 || demux_BA >= 4)
		return FALSE;
	if (   spidev->demux_enabled
	    && spidev->current_demux_BA != demux_BA
	   )
		{
		if (spidev->demux_B_pin >= 0)
			digitalWrite(spidev->demux_B_pin, (demux_BA & 0x2) ? HIGH : LOW);
		if (spidev->demux_A_pin >= 0)
			digitalWrite(spidev->demux_A_pin, (demux_BA & 0x1) ? HIGH : LOW);
		spidev->current_demux_BA = demux_BA;
		}
	return TRUE;
	}

boolean
spi_read(SpiDevice *spidev, int demux_BA, uint8_t *data, int length)
	{
	struct spi_ioc_transfer		spi;

	if (!spi_demux_select(spidev, demux_BA))
		return FALSE;

	memset (&spi, 0, sizeof(spi));	/* clear padding bytes?? */
	spi.tx_buf		= 0;
	spi.rx_buf		= (unsigned long) data;
	spi.len			= length;
	spi.delay_usecs	= spi_delay;
	spi.speed_hz	= spidev->speed[demux_BA];
	spi.bits_per_word = spi_bpw;

	if (ioctl(spidev->fd, SPI_IOC_MESSAGE(1), &spi) < 0)
		{
		printf("spi_read() ioctl() failed: %s\n", strerror(errno));
		return FALSE;
		}
	return TRUE;
	}

boolean
spi_write(SpiDevice *spidev, int demux_BA, uint8_t *data, int length)
	{
	struct spi_ioc_transfer		spi;

	if (!spi_demux_select(spidev, demux_BA))
		return FALSE;

	memset (&spi, 0, sizeof(spi));	/* clear padding bytes?? */
	spi.tx_buf		= (unsigned long) data;
	spi.rx_buf		= 0;
	spi.len			= length;
	spi.delay_usecs	= spi_delay;
	spi.speed_hz	= spidev->speed[demux_BA];
	spi.bits_per_word = spi_bpw;

	if (ioctl(spidev->fd, SPI_IOC_MESSAGE(1), &spi) < 0)
		{
		printf("spi_write() ioctl() failed: %s\n", strerror(errno));
		return FALSE;
		}
	return TRUE;
	}

boolean
spi_rw(SpiDevice *spidev, int demux_BA, uint8_t *data, int length)
	{
	struct spi_ioc_transfer		spi;

	if (!spi_demux_select(spidev, demux_BA))
		return FALSE;

	memset (&spi, 0, sizeof(spi));	/* clear padding bytes?? */
	spi.tx_buf		= (unsigned long) data;
	spi.rx_buf		= (unsigned long) data;
	spi.len			= length;
	spi.delay_usecs	= spi_delay;
	spi.speed_hz	= spidev->speed[demux_BA];
	spi.bits_per_word = spi_bpw;

	if (ioctl(spidev->fd, SPI_IOC_MESSAGE(1), &spi) < 0)
		{
		printf("spi_rw() ioctl() failed: %s\n", strerror(errno));
		return FALSE;
		}
	return TRUE;
	}



int
i2c_open(int i2c_address)
	{
	int 	fd;
	char	*device;

	device = (pi_board_rev() == 1) ? "/dev/i2c-0" : "/dev/i2c-1";

	if ((fd = open(device, O_RDWR)) < 0)
		{
		printf("I2C device %s open failed: %s\n", device, strerror(errno));
		exit(EXIT_FAILURE);
		}
	if (ioctl(fd, I2C_SLAVE, i2c_address) < 0)
		{
		printf("%s I2C slave address 0x%x failed: %s\n",
			device, i2c_address, strerror(errno));
		exit(EXIT_FAILURE);
		}
	return fd;
	}


#endif /*  HAVE_WIRINGPI */
