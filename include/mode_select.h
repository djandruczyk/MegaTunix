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

#ifndef __MODE_SELECT_H__
#define __MODE_SELECT_H__

#include <gtk/gtk.h>

/* Prototypes */
void parse_ecu_capabilities(gint);
void set_raw_memory_mode(gboolean);
void set_widget_sensitive(gpointer, gpointer);
void set_widget_active(gpointer, gpointer);
/* Prototypes */

#endif
