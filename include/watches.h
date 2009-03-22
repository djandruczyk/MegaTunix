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
	SINGLE_BIT_STATE,	/*! Single bit state (level) watch */
	SINGLE_BIT_CHANGE,	/*! Single bit change (change) watch */
	MULTI_BIT,		/*! Multi-bit watch (not implemented yet) */
	EXACT,			/*! Exact val watch (not implemented yet) */
	THRESHOLD,		/*! Threshold val watch (not implemented yet) */
	RANGE,			/*! Range val watch (not implemented yet) */
	VALUE_CHANGE,		/*! Watch that triggers only on valud changes */
	WATCH_COUNT
}WatchStyle;


typedef struct _DataWatch DataWatch;


struct _DataWatch
{
	guint32 id;		/*! Watch ID */
	gint bit;		/*! Bit to watch */
	gfloat low;		/*! Low point (range watch) */
	gfloat high;		/*! Highpoint (range watch) */
	gfloat exact;		/*! Exact value watch */
	gint threshold;		/*! threshold watch */
	gint i_val;		/*! Integer value */
	gfloat f_val;		/*! Integer value */
	gpointer user_data;	/*! user data to pass to user function */
	WatchStyle style;	/*! type of watch */
	gboolean state;		/*! state for bit watches */
	gboolean only_once;	/*! Run only once then evaporate */
	gchar * function;	/*! function to call when watch strikes */
	void (*func) (gpointer,gint,gfloat);/*! Function pointer */
	gchar * varname;	/*! Variable name (rtv internal name) to check */
};
/* Prototypes */
EXPORT void fire_off_rtv_watches_pf();
guint32 create_single_bit_state_watch(gchar *, gint, gboolean, gboolean, gchar *, gpointer);
guint32 create_single_bit_change_watch(gchar *, gint, gboolean, gchar *, gpointer);
guint32 create_value_change_watch(gchar *, gboolean,gchar *, gpointer);
void watch_destroy(gpointer);
void remove_watch(guint32);
void process_watches(gpointer, gpointer, gpointer);
gboolean watch_active(guint32 );
/* Prototypes */

#endif
