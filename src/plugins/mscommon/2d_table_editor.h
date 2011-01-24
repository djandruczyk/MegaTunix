/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __2D_TABLE_EDITOR_H__
#define __2D_TABLE_EDITOR_H__

#include <config.h>
#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <watches.h>

/* Prototypes */
gboolean create_2d_table_editor_group(GtkWidget *);
gboolean create_2d_table_editor(gint, GtkWidget *);
gboolean update_2d_curve(GtkWidget *, gpointer);
gboolean close_2d_editor(GtkWidget *, gpointer);
void coords_changed(GtkWidget *, gpointer);
void remove_widget(gpointer, gpointer);
gboolean close_menu_handler(GtkWidget *, gpointer);
void clean_curve(gpointer, gpointer);
void update_curve_marker(DataWatch *);
void vertex_proximity(GtkWidget *, gpointer);
void marker_proximity(GtkWidget *, gpointer);
gboolean set_axis_locking(GtkWidget *, gpointer );
void highlight_entry(GtkWidget *, GdkColor *);
void gauge_cleanup(gpointer , gpointer );

/* Prototypes */

#endif
