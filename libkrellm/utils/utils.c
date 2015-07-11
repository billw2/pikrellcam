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

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"


/* Return elapsed time in usec since last call on this timer.
|  Return 0 if first call.
*/
int
micro_elapsed_time(struct timeval *timer)
	{
	static struct timeval	lap;
	struct timeval			dif;

	gettimeofday(&lap, NULL);
	if (timer->tv_sec > 0 || timer->tv_usec > 0)
		timersub(&lap, timer, &dif);
	else
		dif.tv_sec = dif.tv_usec = 0;
	*timer = lap;
	return (dif.tv_sec * 1000000 + dif.tv_usec);
	}

boolean
dup_string(char **dst, char *src)
	{
	if (!dst)
		return FALSE;
	if (!src)
		src = "";		
	if (*dst)
		{
		if (!strcmp(*dst, src))
			return FALSE;
		free(*dst);
		}
	*dst = strdup(src);
	return TRUE;
	}

boolean
isfifo(char *fifo)
	{
	struct stat st;
	boolean		result = FALSE;

	if (stat(fifo, &st) == 0 && S_ISFIFO(st.st_mode))
		result = TRUE;
	return result;
	}

boolean
isdir(char *dir)
	{
	struct stat st;
	boolean		result = FALSE;

	if (stat(dir, &st) == 0 && S_ISDIR(st.st_mode))
		result = TRUE;
	return result;
	}

boolean
make_directory(char *dir)
	{
	boolean dir_exists;

	if ((dir_exists = isdir(dir)) == FALSE)
		{
		if (mkdir(dir, 0755) < 0)
			printf("Make directory %s failed: %s\n", dir, strerror(errno));
		else
			dir_exists = TRUE;
		}
	return dir_exists;
	}

