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

#ifndef __MS1_PLUGIN_H__
#define __MS1_PLUGIN_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <enums.h>

/* Prototypes */
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_enums(void);
/* Prototypes */

/* Function Pointers */
void (*error_msg_f)(const gchar *);
gboolean (*get_symbol_f)(const gchar *,void **);
void (*io_cmd_f)(const gchar *,void *);
void *(*dbg_func_f)(int,gchar *);
void (*start_tickler_f)(gint);
void (*stop_tickler_f)(gint);
void (*signal_read_rtvars_f)(void);
gint (*get_ecu_data_f)(gint,gint,gint,DataSize);
GtkWidget *(*lookup_widget_f)(const gchar *);
gboolean (*lookup_current_value_f)(const gchar * internal_name, gfloat *value);
GdkGC *(*initialize_gc_f)(GdkDrawable *, GcType);
/* Function Pointers */

#endif
