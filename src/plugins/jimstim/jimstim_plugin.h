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

/*!
  \file src/plugins/jimstim/jimstim_plugin.h
  \ingroup JimStimPlugin,Headers
  \brief JimStim plugin init/shutdown code
  \author David Andruczyk
  */
 
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __JIMSTIM_PLUGIN_H__
#define __JIMSTIM_PLUGIN_H__

#include <plugindefines.h>
#include <debugging.h>
#include <gtk/gtk.h>
#include <threads.h>

#ifdef __JIMSTIM_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif

/* Function Pointers */
EXTERN gint (*convert_before_download_f)(GtkWidget *, gfloat);
EXTERN gfloat (*convert_after_upload_f)(GtkWidget *);
EXTERN void *(*dbg_func_f)(int,const gchar *, const gchar *, gint, const gchar *, ...);
EXTERN void (*error_msg_f)(const gchar *);
EXTERN void (*get_essential_bits_f)(GtkWidget *, gint *, gint *, gint *, gint *,gint *, gint *);
EXTERN GList *(*get_list_f)(const gchar *);
EXTERN gboolean (*get_symbol_f)(const gchar *, void **);
EXTERN OutputData *(*initialize_outputdata_f)(void);
EXTERN void (*io_cmd_f)(const gchar *,void *);
EXTERN GtkWidget *(*lookup_widget_f)(const gchar *);
EXTERN void (*ms_send_to_ecu_f)(gint, gint, gint, DataSize, gint, gboolean);
EXTERN gboolean (*search_model_f)(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
EXTERN void (*set_widget_sensitive_f)(gpointer, gpointer);
EXTERN void (*start_tickler_f)(gint);
EXTERN gboolean (*std_combo_handler_f)(GtkWidget *, gpointer);
EXTERN void (*stop_tickler_f)(gint);
EXTERN void (*update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean, gboolean);
/* Function Pointers */


/* Prototypes */
void deregister_ecu_enums(void);
void plugin_init(gconstpointer *data);
void plugin_shutdown(void);
void register_ecu_enums(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
