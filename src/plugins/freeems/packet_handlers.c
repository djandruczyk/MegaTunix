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
#include <freeems_plugin.h>
#include <packet_handlers.h>


gboolean find_any_packet(guchar *buf, gint len, gint *start, gint *end)
{
	guchar *p = NULL;
	gint i = 0;

	g_assert(start);
	g_assert(end);
	*start = -1;
	*end = -1;
	p = buf;
	for (i=0;i<len;i++)
	{
		if ((p[i] == START) && (*start == -1))
			*start = i;
		if ((p[i] == END) && (*end == -1))
			*end = i;
	}
	if ((*start >= 0) && (*end >= 0))
		return TRUE;
	else
		return FALSE;
}
