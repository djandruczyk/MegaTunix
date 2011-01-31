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
void (*error_msg_f)(const gchar *);
gboolean (*get_symbol_f)(const gchar *, void **);
GtkWidget *(*lookup_widget_f)(const gchar *);
void (*io_cmd_f)(const gchar *,void *);
OutputData *(*initialize_outputdata_f)(void);
void *(*dbg_func_f)(int,gchar *);
void (*start_tickler_f)(gint);
void (*stop_tickler_f)(gint);
GList *(*get_list_f)(gchar *);
void (*set_widget_sensitive_f)(gpointer, gpointer);
void (*update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean, gboolean);
/* Function Pointers */

/* Prototypes */
void plugin_init(gconstpointer *data);
void plugin_shutdown(void);
void register_ecu_enums(void);
void deregister_ecu_enums(void);
/* Prototypes */

#endif
