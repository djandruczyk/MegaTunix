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

#ifndef __MSCOMMON_GUI_HANDLERS_H__
#define __MSCOMMON_GUI_HANDLERS_H__

#include <configfile.h>
#include <enums.h>
#include <gtk/gtk.h>

/* Externs */
extern gboolean (*get_symbol_f)(const gchar *, void **);
extern void (*thread_update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean);
extern GtkWidget *(*lookup_widget_f)(const gchar *);
extern gboolean (*set_file_api_f)(ConfigFile *, gint , gint );
extern gboolean (*get_file_api_f)(ConfigFile *, gint *, gint *);
extern void (*stop_tickler_f)(gint);
extern void (*start_tickler_f)(gint);
extern gchar **(*parse_keys_f)(gchar *, gint *, gchar * );
extern void(*set_title_f)(const gchar *);
extern glong (*get_extreme_from_size_f)(DataSize, Extreme);
extern gfloat (*convert_after_upload_f)(GtkWidget *);
extern gint (*convert_before_download_f)(GtkWidget *, gfloat);
extern gboolean (*std_entry_handler_f)(GtkWidget *, gpointer);
extern gboolean (*entry_changed_handler_f)(GtkWidget *, gpointer);
extern GdkColor (*get_colors_from_hue_f)(gfloat, gfloat, gfloat);
/* Externs */

/* Prototypes */
gboolean common_entry_handler(GtkWidget *, gpointer);
gboolean common_spin_handler(GtkWidget *, gpointer);
gboolean common_combo_handler(GtkWidget *, gpointer);
/* Prototypes */

/* Enums */

#endif
