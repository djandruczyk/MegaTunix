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
  \file src/plugins/ms1/ms1_plugin.h
  \ingroup MS1Plugin,Headers
  \brief MS1 plugin init/shutdown code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MS1_PLUGIN_H__
#define __MS1_PLUGIN_H__

#include <plugindefines.h>
#include <gtk/gtk.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>

#ifdef __MS1_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif


/* Function Pointers */
EXTERN void (*alter_widget_state_f)(gpointer, gpointer);
EXTERN void (*error_msg_f)(const gchar *);
EXTERN void (*dbg_func_f)(int,const gchar *, const gchar *, gint ,const gchar *, ...);
EXTERN gfloat (*convert_after_upload_f)(GtkWidget *);
EXTERN gint (*convert_before_download_f)(GtkWidget *, gfloat);
EXTERN gboolean (*entry_changed_handler_f)(GtkWidget *, gpointer);
EXTERN guint (*get_bitshift_f)(guint);
EXTERN GdkColor (*get_colors_from_hue_f)(gfloat, gfloat, gfloat);
EXTERN gint (*get_ecu_data_f)(GtkWidget *);
EXTERN void (*get_essential_bits_f)(GtkWidget *, gint *, gint *, gint *, gint *, gint *, gint *);
EXTERN void (*get_essentials_f)(GtkWidget *, gint *, gint *, gint *, DataSize *, gint *);
EXTERN glong (*get_extreme_from_size_f)(DataSize, Extreme);
EXTERN gboolean (*get_symbol_f)(const gchar *,void **);
EXTERN GdkGC *(*initialize_gc_f)(GdkDrawable *, GcType);
EXTERN void (*io_cmd_f)(const gchar *,void *);
EXTERN gboolean (*lookup_current_value_f)(const gchar * internal_name, gfloat *value);
EXTERN GtkWidget *(*lookup_widget_f)(const gchar *);
EXTERN gboolean (*lookuptables_configurator_f)(GtkWidget *, gpointer );
EXTERN gint (*ms_get_ecu_data_f)(gint, gint, gint, DataSize);
EXTERN void (*ms_send_to_ecu_f)(gint, gint, gint, DataSize, gint, gboolean);
EXTERN void (*recalc_table_limits_f)(gint, gint);
EXTERN void (*signal_read_rtvars_f)(void);
EXTERN void (*start_tickler_f)(gint);
EXTERN gboolean (*std_entry_handler_f)(GtkWidget *, gpointer);
EXTERN void (*stop_tickler_f)(gint);
/* Function Pointers */

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
