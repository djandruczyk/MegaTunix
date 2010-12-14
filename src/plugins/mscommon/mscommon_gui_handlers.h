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

#ifndef __MSCOMMON_GUI_HANDLERS_H__
#define __MSCOMMON_GUI_HANDLERS_H__

#include <configfile.h>
#include <enums.h>
#include <gtk/gtk.h>

/* Externs */
extern gboolean (*get_symbol_f)(const gchar *, void **);
extern void (*thread_update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean);
extern GtkWidget *(*lookup_widget_f)(const gchar *);
extern gboolean (*set_file_api_f)(ConfigFile *, gint , gint );
extern gboolean (*get_file_api_f)(ConfigFile *, gint *, gint *);
extern void (*stop_tickler_f)(gint);
extern void (*start_tickler_f)(gint);
extern gchar **(*parse_keys_f)(const gchar *, gint *, const gchar * );
extern void(*set_title_f)(const gchar *);
extern glong (*get_extreme_from_size_f)(DataSize, Extreme);
extern gfloat (*convert_after_upload_f)(GtkWidget *);
extern gint (*convert_before_download_f)(GtkWidget *, gfloat);
extern gboolean (*std_entry_handler_f)(GtkWidget *, gpointer);
extern gboolean (*entry_changed_handler_f)(GtkWidget *, gpointer);
extern GdkColor (*get_colors_from_hue_f)(gfloat, gfloat, gfloat);
extern guint (*get_bitshift_f)(guint);
extern GList *(*get_list_f)(gchar *);
extern gboolean (*lookup_current_value_f)(const gchar *, gfloat *);
extern gboolean (*search_model_f)(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
extern void (*set_reqfuel_color_f)(GuiColor, gint);
extern gint (*translate_string_f)(const gchar *);
extern void (*alter_widget_state_f)(gpointer, gpointer);
extern void (*update_ve3d_if_necessary_f)(gint, gint);
extern void (*thread_refresh_widget_f)(GtkWidget *);
/* Externs */

/* Prototypes */
gboolean common_entry_handler(GtkWidget *, gpointer);
gboolean common_bitmask_button_handler(GtkWidget *, gpointer);
gboolean common_slider_handler(GtkWidget *, gpointer);
gboolean common_std_button_handler(GtkWidget *, gpointer);
gboolean common_toggle_button_handler(GtkWidget *, gpointer);
gboolean common_combo_handler(GtkWidget *, gpointer);
gboolean common_spin_button_handler(GtkWidget *, gpointer);

void set_widget_labels(const gchar *);
void swap_labels(GtkWidget *, gboolean);
void switch_labels(gpointer, gpointer);
void update_combo(GtkWidget *);
void update_entry(GtkWidget *);
void update_checkbutton(GtkWidget *);
gboolean force_update_table(gpointer);
gboolean trigger_group_update(gpointer);
gboolean update_multi_expression(gpointer);
void combo_handle_group_2_update(GtkWidget *);
void combo_handle_algorithms(GtkWidget *);
void handle_group_2_update(GtkWidget *);
void handle_algorithm(GtkWidget *);
gint get_choice_count(GtkTreeModel *);
void combo_set_labels(GtkWidget *, GtkTreeModel *);
gboolean search_model(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
void toggle_groups_linked(GtkWidget *, gboolean);
void combo_toggle_groups_linked(GtkWidget *,gint);
void combo_toggle_labels_linked(GtkWidget *,gint);
void set_widget_label_from_array(gpointer, gpointer);
void get_essential_bits(GtkWidget *, gint *, gint *, gint *, gint *, gint *, gint *);
void get_essentials(GtkWidget *, gint *, gint *, gint *, DataSize *, gint *);
void update_widget(gpointer, gpointer);
void recalc_table_limits(gint, gint);
void thread_refresh_widgets_at_offset(gint, gint);
/* Prototypes */


typedef enum
{
        NUM_SQUIRTS_1 = LAST_BUTTON_ENUM + 3,
        NUM_SQUIRTS_2,
        NUM_CYLINDERS_1,
        NUM_CYLINDERS_2,
        NUM_INJECTORS_1,
        NUM_INJECTORS_2,
	LOCKED_REQ_FUEL,
	REQ_FUEL_1,
	REQ_FUEL_2,
	MULTI_EXPRESSION,
	ALT_SIMUL


}CommonMtxButton;

#endif
