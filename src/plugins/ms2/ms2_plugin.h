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

/* Prototypes */
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_ecu_enums(void);
/* Prototypes */

/* Function pointers */
void (*error_msg_f)(const gchar *);
gboolean (*get_symbol_f)(const gchar *, void **);
void (*io_cmd_f)(const gchar *, void *);
void (*dbg_func_f)(int,gchar *);
void (*start_tickler_f)(gint);
void (*stop_tickler_f)(gint);
void (*signal_read_rtvars_f)(void);
gint (*ms_get_ecu_data_f)(gint, gint, gint, DataSize);
GtkWidget *(*lookup_widget_f)(const gchar *);
gboolean (*lookup_current_value_f)(const gchar * internal_name, gfloat *value);
GdkGC *(*initialize_gc_f)(GdkDrawable *, GcType);
void (*create_single_bit_state_watch_f)(const gchar *, gint, gboolean, gboolean, const gchar *, gpointer);
void *(*evaluator_create_f)(char *);
void *(*evaluator_destroy_f)(void *);
double (*evaluator_evaluate_x_f)(void *, double);
guint (*get_bitshift_f)(guint);
void (*ms_send_to_ecu_f)(gint, gint, gint, DataSize, gint, gboolean);
void (*update_widget_f)(gpointer, gpointer);
gint (*convert_before_download_f)(GtkWidget *, gfloat);
gboolean (*search_model_f)(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
GtkWidget *(*mask_entry_new_with_mask_wrapper_f)(gchar *);

/* Function pointers */

#endif
