/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file include/widgetmgmt.h
  \ingroup Headers
  \brief Header for the widget management utility functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __WIDGETMGMT_H__
#define __WIDGETMGMT_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
void alter_widget_state(gpointer, gpointer);
gboolean deregister_widget(const gchar *);
void dump_datalist(GQuark, gpointer, gpointer);
void get_geo(GtkWidget *, const char *, PangoRectangle *);
gint get_multiplier(DataSize );
gboolean get_state(gchar *, gint );
void lock_entry(GtkWidget *);
GtkWidget * lookup_widget(const gchar *);
void populate_master(GtkWidget *, gpointer );
void register_widget(const gchar *, GtkWidget *);
void set_fixed_size(GtkWidget *, int);
void set_widget_active(gpointer, gpointer);
void set_widget_sensitive(gpointer, gpointer);
void store_widget_data(gpointer , gpointer );
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
