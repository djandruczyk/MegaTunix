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

#ifndef __MSCOMMON_MENU_HANDLERS_H__
#define __MSCOMMON_MENU_HANDLERS_H__

#include <defines.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <threads.h>

/* Externs */
extern gboolean (*get_symbol_f)(const gchar *, void **);
extern void (*dbg_func_f)(gint, gchar *);
extern gint (*get_multiplier_f)(DataSize);
extern void *(*eval_create_f)(char *);
extern void (*eval_destroy_f)( void *);
extern double (*eval_x_f)(void *, double);
extern gboolean (*std_entry_handler_f)(GtkWidget *, gpointer);
extern void (*register_widget_f)(gchar *, GtkWidget *);
/* Externs */

/* Prototypes */
void common_plugin_menu_setup(GladeXML *);
gboolean show_tps_calibrator_window(GtkWidget *, gpointer);
gboolean show_create_ignition_map_window(GtkWidget *, gpointer);
gboolean create_ignition_map(GtkWidget *, gpointer);
gdouble linear_interpolate(gdouble, gdouble, gdouble, gdouble, gdouble);
/* Prototypes */

#endif
