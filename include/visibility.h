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

#ifndef __VISIBILITY_H__
#define __VISIBILITY_H__

#include <config.h>
#include <defines.h>
#include <gtk/gtk.h>

/* Prototypes */
EXPORT gboolean show_tab_visibility_window(GtkWidget *, gpointer);
gboolean hide_tab(GtkWidget *, gpointer );
/* Prototypes */

#endif
