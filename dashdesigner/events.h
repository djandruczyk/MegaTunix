/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux EFI tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef __DASH_EVENTS_H__
#define __DASH_EVENTS_H__

#include <defines.h>
#include <gtk/gtk.h>


/* Enums */
typedef enum 
{
	GAUGE_ADD=1,
	GAUGE_REMOVE
}Choice;

/* Prototypes */

gboolean add_gauge(GtkWidget *, gpointer );
gboolean close_current_dash(GtkWidget *, gchar * );
gboolean create_preview_list(GtkWidget *, gpointer );
gboolean dashdesigner_about(GtkWidget *, gpointer );
gboolean dashdesigner_quit(GtkWidget *, gpointer );
gboolean dummy(GtkWidget *, gpointer );
void free_element(gpointer ,gpointer );
gboolean gauge_choice_button_event(GtkWidget *, GdkEventButton *,gpointer );
gint list_sort(gconstpointer , gconstpointer );
gboolean optimize_dash_size(GtkWidget *, gpointer );
void raise_fixed_child (GtkWidget * );
void scan_for_gauges(gpointer, gpointer);
void set_combo_to_source(GtkWidget *, gchar * );
void update_properties(GtkWidget *,Choice);
			 
/* Prototypes */

#endif
