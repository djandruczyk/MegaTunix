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

#ifndef __MS2_PLUGIN_H__
#define __MS2_PLUGIN_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <enums.h>
#include <enums.h>

#ifdef __MS2_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif

/* Function pointers */
EXTERN void (*error_msg_f)(const gchar *);
EXTERN gboolean (*get_symbol_f)(const gchar *, void **);
EXTERN void (*io_cmd_f)(const gchar *, void *);
EXTERN void (*dbg_func_f)(int,gchar *);
EXTERN void (*start_tickler_f)(gint);
EXTERN void (*stop_tickler_f)(gint);
EXTERN void (*signal_read_rtvars_f)(void);
EXTERN gint (*ms_get_ecu_data_f)(gint, gint, gint, DataSize);
EXTERN GtkWidget *(*lookup_widget_f)(const gchar *);
EXTERN gboolean (*lookup_current_value_f)(const gchar * internal_name, gfloat *value);
EXTERN GdkGC *(*initialize_gc_f)(GdkDrawable *, GcType);
EXTERN void (*create_single_bit_state_watch_f)(const gchar *, gint, gboolean, gboolean, const gchar *, gpointer);
EXTERN void *(*evaluator_create_f)(char *);
EXTERN void *(*evaluator_destroy_f)(void *);
EXTERN double (*evaluator_evaluate_x_f)(void *, double);
EXTERN guint (*get_bitshift_f)(guint);
EXTERN void (*ms_send_to_ecu_f)(gint, gint, gint, DataSize, gint, gboolean);
EXTERN void (*update_widget_f)(gpointer, gpointer);
EXTERN gint (*convert_before_download_f)(GtkWidget *, gfloat);
EXTERN gboolean (*search_model_f)(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
EXTERN GtkWidget *(*mask_entry_new_with_mask_wrapper_f)(gchar *);
EXTERN void (*register_widget_f)(gchar *, GtkWidget *);
EXTERN void (*bind_to_lists_f)(GtkWidget * , gchar * );
/* Function pointers */

/* Prototypes */
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_ecu_enums(void);
/* Prototypes */


#endif
