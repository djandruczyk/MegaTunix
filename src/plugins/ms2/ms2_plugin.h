/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \file src/plugins/ms2/ms2_plugin.h
  \ingroup MS2Plugin,Headers
  \brief MS2 plugin init/shutdown code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MS2_PLUGIN_H__
#define __MS2_PLUGIN_H__

#include <plugindefines.h>
#include <gtk/gtk.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>

#ifdef __MS2_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif

/* Function pointers */
EXTERN void (*alter_widget_state_f)(gpointer, gpointer);
EXTERN void (*bind_to_lists_f)(GtkWidget * , const gchar * );
EXTERN gfloat (*calc_value_f)(gfloat, gfloat *, gfloat *, ConvDir);
EXTERN gfloat (*convert_after_upload_f)(GtkWidget *);
EXTERN gint (*convert_before_download_f)(GtkWidget *, gfloat);
EXTERN void (*convert_temps_f)(gpointer, gpointer);
EXTERN void (*create_rtv_single_bit_state_watch_f)(const gchar *, gint, gboolean, gboolean, const gchar *, gpointer);
EXTERN void (*dbg_func_f)(int,const gchar *, const gchar *, gint ,const gchar *, ...);
EXTERN void *(*evaluator_create_f)(char *);
EXTERN void *(*evaluator_destroy_f)(void *);
EXTERN double (*evaluator_evaluate_x_f)(void *, double);
EXTERN gdouble (*f_to_c_f)(gdouble);
EXTERN gdouble (*f_to_k_f)(gdouble);
EXTERN void (*get_essential_bits_f)(GtkWidget *, gint *, gint *, gint *, gint *, gint *, gint *);
EXTERN guint (*get_bitshift_f)(guint);
EXTERN void (*error_msg_f)(const gchar *);
EXTERN glong (*get_extreme_from_size_f)(DataSize, Extreme);
EXTERN gboolean (*get_symbol_f)(const gchar *, void **);
EXTERN GdkGC *(*initialize_gc_f)(GdkDrawable *, GcType);
EXTERN void (*io_cmd_f)(const gchar *, void *);
EXTERN gboolean (*lookup_current_value_f)(const gchar * internal_name, gfloat *value);
EXTERN GtkWidget *(*lookup_widget_f)(const gchar *);
EXTERN GtkWidget *(*mask_entry_new_with_mask_f)(gchar *);
EXTERN gint (*ms_get_ecu_data_f)(gint, gint, gint, DataSize);
EXTERN void (*ms_send_to_ecu_f)(gint, gint, gint, DataSize, gint, gboolean);
EXTERN void (*register_widget_f)(const gchar *, GtkWidget *);
EXTERN void (*remove_from_lists_f)(const gchar *, gpointer);
EXTERN gboolean (*search_model_f)(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
EXTERN void (*signal_read_rtvars_f)(void);
EXTERN void (*start_tickler_f)(gint);
EXTERN void (*stop_tickler_f)(gint);
EXTERN gdouble (*temp_to_host_f)(gdouble);
EXTERN void (*update_widget_f)(gpointer, gpointer);
/* Function pointers */

/* Prototypes */
void deregister_ecu_enums(void);
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_ecu_enums(void);
/* Prototypes */


#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
