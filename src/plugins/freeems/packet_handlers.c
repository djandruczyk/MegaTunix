/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <packet_handlers.h>
#include <serialio.h>

extern GtkWidget *interr_view;


gboolean find_any_packet(void *data, gint len, gint *start, gint *end)
{
	guchar *ptr = NULL;
	gint i = 0;

	g_assert(start);
	g_assert(end);
	*start = 0;
	*end = 0;
	ptr = data;
	for (i=0;i<len;i++)
	{
		printf("offset %i,  \"%0X\" \n",i,(gint)ptr[i]);
		if (ptr[i] == ESCAPE)
			printf("ESCAPE byte found at offset %i\n",i);
		if ((ptr[i] == START) && (*start == 0))
		{
			printf("Found start marker at %i\n",i);
			*start = i;
		}
		if ((ptr[i] == END) && (*end == 0))
		{
			printf("Found end marker at %i\n",i);
			*end = i;
		}
	}
	if ((*start > 0) && (*end > 0))
		return TRUE;
	else
		return FALSE;

}
