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

#ifdef __JIMSTIM_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif

/* Function Pointers */
EXTERN void (*error_msg_f)(const gchar *) = NULL;
EXTERN gboolean (*get_symbol_f)(const gchar *, void **) = NULL;
EXTERN GtkWidget *(*lookup_widget_f)(const gchar *) = NULL;
EXTERN void (*io_cmd_f)(const gchar *,void *) = NULL;
EXTERN OutputData *(*initialize_outputdata_f)(void) = NULL;
EXTERN void *(*dbg_func_f)(int,gchar *) = NULL;
EXTERN void (*start_tickler_f)(gint) = NULL;
EXTERN void (*stop_tickler_f)(gint) = NULL;
EXTERN GList *(*get_list_f)(gchar *) = NULL;
EXTERN void (*set_widget_sensitive_f)(gpointer, gpointer) = NULL;
EXTERN void (*update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean, gboolean) = NULL;
/* Function Pointers */


/* Prototypes */
void plugin_init(gconstpointer *data);
void plugin_shutdown(void);
void register_ecu_enums(void);
void deregister_ecu_enums(void);
/* Prototypes */

#endif
