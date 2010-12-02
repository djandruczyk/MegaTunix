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

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <gtk/gtk.h>

/* Prototypes */
gboolean plugin_function(GtkWidget *, gpointer);
void plugins_init(void);
void plugins_shutdown(void);
gboolean get_symbol(const gchar *, void **);
/* Prototypes */

#endif
