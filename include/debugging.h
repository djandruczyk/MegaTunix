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

#ifndef __DEBUG_GUI_H__
#define __DEBUG_GUI_H__

#include <gtk/gtk.h>
#include <enums.h>


typedef struct _DebugLevel DebugLevel;
/*! 
 \brief _DebugLevel stores the debugging name, handler, class (bitmask) and 
 shift (forgot why this is here) and a enable/disable flag. Used to make the
 debugging core a little more configurable
 */


struct _DebugLevel
{
	gchar * name;		/*! Debugging name */
	gint	handler;	/*! Signal handler name */
	Dbg_Class dclass;	/*! Bit mask for this level (0-31) */
	Dbg_Shift dshift;	/*! Bit shift amount */
	gboolean enabled;	/*! Enabled or not? */
};


/* Prototypes */
void close_debug(void);
void open_debug(void);
void dbg_func(gchar *);
void populate_debugging(GtkWidget *);
/* Prototypes */

#endif
