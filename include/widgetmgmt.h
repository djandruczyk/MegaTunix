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
void store_widget_data(gpointer , gpointer );
void register_widget(gchar *, GtkWidget *);
gboolean deregister_widget(gchar *);
gboolean get_state(gchar *, gint );
void alter_widget_state(gpointer, gpointer);
void get_geo(GtkWidget *, const char *, PangoRectangle *);
void set_fixed_size(GtkWidget *, int);
void set_widget_labels(gchar *);
EXPORT void lock_entry(GtkWidget *);
/* Prototypes */

#endif
