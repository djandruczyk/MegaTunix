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

#ifndef __WIDGETMGMT_H__
#define __WIDGETMGMT_H__

#include <gtk/gtk.h>

/* Prototypes */
void populate_master(GtkWidget *, gpointer );
void register_widget(gchar *, GtkWidget *);
gboolean deregister_widget(gchar *);
GtkWidget * get_raw_widget(gint , gint );
/* Prototypes */

#endif
