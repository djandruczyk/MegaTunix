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

#ifndef __T_LOGGER_H__
#define __T_LOGGER_H__

#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
void setup_logger_display(GtkWidget *);
gboolean logger_display_config_event(GtkWidget *, GdkEventConfigure *, gpointer);
gboolean logger_display_expose_event(GtkWidget *, GdkEventExpose *, gpointer);
void crunch_trigtooth_data(gint);
void update_trigtooth_display(gint);
void gdk_update_trigtooth_display(gint);
void cairo_update_trigtooth_display(gint);
void bind_ttm_to_page(gint page);

/* Prototypes */

#endif
