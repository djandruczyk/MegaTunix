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

#ifndef __RUNTIME_GUI_H__
#define __RUNTIME_GUI_H__

#include <gtk/gtk.h>

/* Prototypes */
void build_runtime(GtkWidget *);
gboolean update_runtime_vars(void);
void reset_runtime_status(void);
void rt_update_values(gpointer,gpointer,gpointer);
/* Prototypes */

#endif
