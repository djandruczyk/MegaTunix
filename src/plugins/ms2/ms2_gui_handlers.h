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

#ifndef __MS2_GUI_HANDLERS_H__
#define __MS2_GUI_HANDLERS_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Externs */
extern void (*dbg_func_f)(int, gchar *);
extern gboolean (*std_entry_handler_f)(GtkWidget *, gpointer);
extern gboolean (*entry_changed_handler_f)(GtkWidget *, gpointer);
extern gint (*ms_get_ecu_data_f)(gint, gint, gint, DataSize);
extern void (*ms_send_to_ecu_f)(gint, gint, gint, DataSize, gint, gboolean);
extern void (*recalc_table_limits_f)(gint, gint);
extern glong (*get_extreme_from_size_f)(DataSize, Extreme);
extern GdkColor (*get_colors_from_hue_f)(gfloat, gfloat, gfloat);
extern gint (*convert_before_download_f)(GtkWidget *, gfloat);
extern void (*start_tickler_f)(gint);
extern void (*stop_tickler_f)(gint);
extern void (*io_cmd_f)(const gchar *, void *);
extern guint (*get_bitshift_f)(guint);
extern void *(*evaluator_create_f)(char *);
extern void *(*evaluator_destroy_f)(void *);
extern double (*evaluator_evaluate_x_f)(void *, double);
extern void ms_send_to_ecu(gint, gint, gint, DataSize, gint, gboolean);
extern void (*update_widget_f)(gpointer, gpointer);
extern gint (*convert_before_download_f)(GtkWidget *, gfloat);
extern gboolean (*search_model_f)(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
/* Externs */

/* Prototypes */
gboolean ecu_entry_handler(GtkWidget *, gpointer);
gboolean ecu_std_button_handler(GtkWidget *, gpointer);
gboolean ecu_toggle_button_handler(GtkWidget *, gpointer);
gboolean ecu_combo_handler(GtkWidget *, gpointer);
/* Prototypes */

typedef enum
{
	START_TOOTHMON_LOGGER = LAST_TOGGLE_ENUM + 1,
	START_TRIGMON_LOGGER,
	STOP_TOOTHMON_LOGGER,
	STOP_TRIGMON_LOGGER,
	START_COMPOSITEMON_LOGGER,
	STOP_COMPOSITEMON_LOGGER
}MS2ToggleButton;
#endif
