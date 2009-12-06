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

#ifndef __DASH_EVENTS_H__
#define __DASH_EVENTS_H__

#include <defines.h>
#include <glade/glade.h>
#include <gtk/gtk.h>


/* Enums */
typedef enum 
{
	GAUGE_ADD=1,
	GAUGE_REMOVE
}Choice;

/* Prototypes */

EXPORT gboolean dashdesigner_about(GtkWidget *, gpointer );
EXPORT gboolean add_gauge(GtkWidget *, gpointer );
EXPORT gboolean dashdesigner_quit(GtkWidget *, gpointer );
EXPORT gboolean create_preview_list(GtkWidget *, gpointer );
EXPORT gboolean gauge_choice_button_event(GtkWidget *, GdkEventButton *,gpointer );
EXPORT gboolean close_current_dash(GtkWidget *, gchar * );
void raise_fixed_child (GtkWidget * );
void update_properties(GtkWidget *,Choice);
void set_combo_to_source(GtkWidget *, gchar * );
void free_element(gpointer ,gpointer );
gint list_sort(gconstpointer , gconstpointer );

gboolean dummy(GtkWidget *, gpointer );
EXPORT gboolean optimize_dash_size(GtkWidget *, gpointer );
			 
/* Prototypes */

#endif
