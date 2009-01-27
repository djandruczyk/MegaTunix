/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/* Runtime Gui Structures */

#ifndef __WATCHES_H__
#define __WATCHES_H__

#include <defines.h>
#include <gtk/gtk.h>

typedef enum
{
	SINGLE_BIT,
	MULTI_BIT,
	EXACT,
	THRESHOLD,
	RANGE,
	WATCH_COUNT
}WatchStyle;

typedef struct _DataWatch DataWatch;

struct _DataWatch
{
	guint32 id;
	gint bit;
	gint low;
	gint high;
	gint exact;
	gint threshold;
	gpointer user_data;
	WatchStyle style;
	gboolean state;
	gchar * function;
	gchar * varname;
};
/* Prototypes */
EXPORT void fire_off_rtv_watches_pf();
guint32 create_single_bit_watch(gchar *, gint, gint, gchar *, gpointer);
void watch_destroy(gpointer);
void remove_watch(guint32);
void process_watches(gpointer, gpointer, gpointer);
gboolean watch_active(guint32 );
/* Prototypes */

#endif
