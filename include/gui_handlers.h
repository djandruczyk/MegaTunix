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
gboolean comm_port_change(GtkEditable *);
gboolean std_button_handler(GtkWidget *, gpointer);
gboolean std_entry_handler(GtkWidget *, gpointer);
gboolean entry_changed_handler(GtkWidget *, gpointer );
gboolean toggle_button_handler(GtkWidget *, gpointer);
gboolean bitmask_button_handler(GtkWidget *, gpointer);
gboolean spin_button_handler(GtkWidget *, gpointer);
gboolean spin_button_grab(GtkWidget *, GdkEventButton *, gpointer );
gboolean key_press_event(GtkWidget *, GdkEventKey *, gpointer );
void page_changed(GtkNotebook *, GtkNotebookPage *, guint, gpointer);
void update_ve_const(void);
void update_widget(gpointer, gpointer );
void switch_labels(gchar * ,gboolean );
/* Prototypes */

#endif
