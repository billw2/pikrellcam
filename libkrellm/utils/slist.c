/*
|  slist functions:
|    The slist functions are from glib-2.0.0 which are:
|	 Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
|    and are repackaged here for my libkrellm utils library.
|
| This Library is free software: you can redistribute it and/or modify
| it under the terms of the GNU General Public License as published by
| the Free Software Foundation, either version 3 of the License, or
| (at your option) any later version.
|
| This Library is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with the libukrellm.  If not, see
| <http://www.gnu.org/licenses/>.
|
*/

#include <stdio.h>

#include "utils.h"

/* ==== SList functions ==== */

void
slist_free(SList *list)
	{
	SList *last;

	while (list)
		{
		last = list;
		list = list->next;
		free(last);
		}
	}

void
slist_and_data_free(SList *list)
	{
	SList *last;

	while (list)
		{
		if (list->data)
			free(list->data);
		last = list;
		list = list->next;
		free(last);
		}
	}


SList *
slist_last(SList *list)
	{
	if (list)
		{
		while (list->next)
			list = list->next;
		}
	return list;
	}

SList *
slist_append(SList *list, void *data)
	{
	SList *new_list;
	SList *last;

	if ((new_list = calloc(1, sizeof(SList))) != NULL)
		new_list->data = data;

	if (list)
		{
		last = slist_last(list);
		last->next = new_list;
		}
	else
		list = new_list;
	return list;
	}

SList *
slist_prepend(SList *list, void *data)
	{
	SList *new_list;

	if ((new_list = calloc(1, sizeof(SList))) != NULL)
		{
		new_list->data = data;
		new_list->next = list;
		}
	else
		new_list = list;    /* If can't alloc, leave list unchanged */
	return new_list;
	}

SList *
slist_remove(SList *list, void *data)
	{
	SList *tmp, *prev = NULL;

	tmp = list;
	while (tmp)
		{
		if (tmp->data == data)
			{
			if (prev)
				prev->next = tmp->next;
			else
				list = tmp->next;

			free(tmp);
			break;
			}
		prev = tmp;
		tmp = prev->next;
		}
	return list;
	}

SList *
slist_nth(SList *list, int  n)
	{
	while (n-- > 0 && list)
		list = list->next;
	return list;
	}

void *
slist_nth_data(SList *list, int n)
	{
	while (n-- > 0 && list)
		list = list->next;
	return list ? list->data : NULL;
	}

SList *
slist_find(SList *list, void *data)
	{
	while (list)
		{
		if (list->data == data)
			break;
		list = list->next;
		}
	return list;
	}

int
slist_length(SList *list)
	{
	int length;

	length = 0;
	while (list)
		{
		length++;
		list = list->next;
		}
	return length;
	}

SList *
slist_remove_link(SList *list, SList *link)
	{
	SList	*tmp,
	        *prev = NULL;

	tmp = list;
	while (tmp)
		{
		if (tmp == link)
			{
			if (prev)
				prev->next = tmp->next;
			if (list == tmp)
				list = list->next;
			tmp->next = NULL;
			break;
			}
		prev = tmp;
		tmp = tmp->next;
		}
	return list;
	}
