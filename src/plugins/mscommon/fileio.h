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

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Externs */
extern void (*thread_update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean);
extern GtkWidget *(*lookup_widget_f)(const gchar *);
extern gboolean (*set_file_api_f)(ConfigFile *, gint , gint );
extern gboolean (*get_file_api_f)(ConfigFile *, gint *, gint *);
extern void (*stop_tickler_f)(gint);
extern void (*start_tickler_f)(gint);
extern gchar **(*parse_keys_f)(const gchar *, gint *, const gchar * );
extern void(*set_title_f)(const gchar *);

/* Externs */

/* Prototypes */
 gboolean select_file_for_ecu_backup(GtkWidget *, gpointer );
 gboolean select_file_for_ecu_restore(GtkWidget *, gpointer );
void backup_all_ecu_settings(gchar  *);
void restore_all_ecu_settings(gchar  *);
/* Prototypes */

#endif
