/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __MS1_PLUGIN_H__
#define __MS1_PLUGIN_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <enums.h>

#ifdef __MS1_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif


/* Function Pointers */
EXTERN void (*error_msg_f)(const gchar *);
EXTERN gboolean (*get_symbol_f)(const gchar *,void **);
EXTERN void (*io_cmd_f)(const gchar *,void *);
EXTERN void (*dbg_func_f)(int,gchar *);
EXTERN void (*start_tickler_f)(gint);
EXTERN void (*stop_tickler_f)(gint);
EXTERN void (*signal_read_rtvars_f)(void);
EXTERN gint (*ms_get_ecu_data_f)(gint, gint, gint, DataSize);
EXTERN void (*ms_send_to_ecu_f)(gint, gint, gint, DataSize, gint, gboolean);
EXTERN GtkWidget *(*lookup_widget_f)(const gchar *);
EXTERN gboolean (*lookup_current_value_f)(const gchar * internal_name, gfloat *value);
EXTERN GdkGC *(*initialize_gc_f)(GdkDrawable *, GcType);
EXTERN gboolean (*std_entry_handler_f)(GtkWidget *, gpointer);
EXTERN gboolean (*entry_changed_handler_f)(GtkWidget *, gpointer);
EXTERN void (*recalc_table_limits_f)(gint, gint);
EXTERN glong (*get_extreme_from_size_f)(DataSize, Extreme);
EXTERN GdkColor (*get_colors_from_hue_f)(gfloat, gfloat, gfloat);
EXTERN gint (*convert_before_download_f)(GtkWidget *, gfloat);
EXTERN guint (*get_bitshift_f)(guint);
EXTERN void (*get_essential_bits_f)(GtkWidget *, gint *, gint *, gint *, gint *, gint *, gint *);
EXTERN void (*get_essentials_f)(GtkWidget *, gint *, gint *, gint *, DataSize *, gint *);
EXTERN gfloat (*convert_after_upload_f)(GtkWidget *);
EXTERN gint (*get_ecu_data_f)(GtkWidget *);
EXTERN gboolean (*lookuptables_configurator_f)(GtkWidget *, gpointer );
/* Function Pointers */

/* Prototypes */
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_ecu_enums(void);
void deregister_ecu_enums(void);
/* Prototypes */

#endif
