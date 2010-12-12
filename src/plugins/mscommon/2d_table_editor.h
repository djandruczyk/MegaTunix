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

#ifndef __2D_TABLE_EDITOR_H__
#define __2D_TABLE_EDITOR_H__

#include <config.h>
#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <watches.h>

/*Externs */
extern void (*bind_to_lists_f)(GtkWidget *, gchar *);
extern gchar **(*parse_keys_f)(const gchar *, gint *, const gchar * );
extern void (*warn_user_f)(const gchar *);
extern gboolean (*lookup_current_value_f)(const gchar *, gfloat *);
extern gint (*get_multiplier_f)(DataSize);
extern gboolean (*entry_changed_handler_f)(GtkWidget *, gpointer);
extern gboolean (*std_entry_handler_f)(GtkWidget *, gpointer);
extern gboolean (*key_event_f)(GtkWidget *, GdkEventKey *, gpointer );
extern gboolean (*focus_out_handler_f)(GtkWidget *, GdkEventFocus *, gpointer );
extern guint32 (*create_value_change_watch_f)(const gchar *, gboolean, const gchar *, gpointer);
extern void (*alter_widget_state_f)(gpointer, gpointer);
extern void (*remove_from_lists_f)(gchar *, gpointer);
extern void (*remove_watch_f)(guint32);
extern void *(*eval_create_f)(char *);
extern void (*eval_destroy_f)(void *);
extern double (*eval_x_f)(void *, double);

/*Externs */
	
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
