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

#ifndef __JIMSTIM_PLUGIN_H__
#define __JIMSTIM_PLUGIN_H__

#include <gtk/gtk.h>
#include <threads.h>


/* Function Pointers */
void (*error_msg_f)(const gchar *) = NULL;
gboolean (*get_symbol_f)(const gchar *, void **) = NULL;
GtkWidget *(*lookup_widget_f)(const gchar *) = NULL;
void (*io_cmd_f)(const gchar *,void *) = NULL;
OutputData *(*initialize_outputdata_f)(void) = NULL;
void *(*dbg_func_f)(int,gchar *) = NULL;
void (*start_tickler_f)(gint) = NULL;
void (*stop_tickler_f)(gint) = NULL;
GList *(*get_list_f)(gchar *) = NULL;
void (*set_widget_sensitive_f)(gpointer, gpointer) = NULL;
void (*update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean, gboolean) = NULL;
/* Function Pointers */


/* Prototypes */
void plugin_init(gconstpointer *data);
void plugin_shutdown(void);
void register_ecu_enums(void);
void deregister_ecu_enums(void);
/* Prototypes */

#endif
