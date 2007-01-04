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

#ifndef __GUI_HANDLERS_H__
#define __GUI_HANDLERS_H__

#include <gtk/gtk.h>

/* Prototypes */
void leave(GtkWidget *, gpointer);
EXPORT gboolean comm_port_change(GtkEditable *);
EXPORT gboolean std_button_handler(GtkWidget *, gpointer);
EXPORT gboolean std_entry_handler(GtkWidget *, gpointer);
EXPORT gboolean entry_changed_handler(GtkWidget *, gpointer );
EXPORT gboolean toggle_button_handler(GtkWidget *, gpointer);
EXPORT gboolean bitmask_button_handler(GtkWidget *, gpointer);
EXPORT gboolean spin_button_handler(GtkWidget *, gpointer);
EXPORT gboolean widget_grab(GtkWidget *, GdkEventButton *, gpointer );
EXPORT gboolean key_event(GtkWidget *, GdkEventKey *, gpointer );
EXPORT gboolean set_algorithm(GtkWidget *, gpointer );
void page_changed(GtkNotebook *, GtkNotebookPage *, guint, gpointer);
void update_ve_const(void);
void update_widget(gpointer, gpointer );
void switch_labels(gpointer , gpointer );
void swap_labels(gchar *, gboolean );
/* Prototypes */

#endif
