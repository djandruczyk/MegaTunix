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

#ifndef __MSCOMMON_PLUGIN_H__
#define __MSCOMMON_PLUGIN_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <configfile.h>
#include <enums.h>
#include <threads.h>

#ifdef __MSCOMMON_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif

/* Function Pointers */
EXTERN void (*error_msg_f)(const gchar *);
EXTERN gboolean (*get_symbol_f)(const gchar *,void **);
EXTERN void (*cleanup_f)(void *);
EXTERN void (*io_cmd_f)(const gchar *,void *);
EXTERN void (*dbg_func_f)(gint,gchar *);
EXTERN GList *(*get_list_f)(gchar *);
EXTERN OutputData *(*initialize_outputdata_f)(void);
EXTERN void (*set_title_f)(const gchar *);
EXTERN void (*set_widget_labels_f)(const gchar *);
EXTERN void (*set_widget_sensitive_f)(gpointer, gpointer);
EXTERN void (*thread_update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean);
EXTERN void (*update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean, gboolean);
EXTERN void (*process_rt_vars_f)(void *, gint );
EXTERN void (*thread_update_widget_f)(const gchar *, WidgetType, gchar *);
EXTERN gboolean (*queue_function_f)(const gchar * );
EXTERN gboolean (*lookup_precision_f)(const gchar *, gint *);
EXTERN gboolean (*lookup_current_value_f)(const gchar *, gfloat *);
EXTERN gfloat (*direct_lookup_data_f)(gchar *, gint );
EXTERN gint (*direct_reverse_lookup_f)(gchar *, gint );
EXTERN gint (*direct_reverse_lookup_f)(gchar *, gint );
EXTERN gint (*reverse_lookup_f)(gconstpointer *, gint );
EXTERN gfloat (*lookup_data_f)(gconstpointer *, gint );
EXTERN gint (*translate_string_f)(const gchar *);
EXTERN GtkWidget *(*lookup_widget_f)(const gchar *);
EXTERN void (*set_group_color_f)(GuiColor, const gchar * );
EXTERN gint (*get_multiplier_f)(DataSize);
EXTERN guint (*get_bitshift_f)(guint);
EXTERN GtkWidget *(*spin_button_handler_f)(GtkWidget *, gpointer);
EXTERN gboolean (*set_file_api_f)(ConfigFile *, gint, gint );
EXTERN gboolean (*get_file_api_f)(ConfigFile *, gint *, gint * );
EXTERN void (*stop_tickler_f)(gint);
EXTERN void (*start_tickler_f)(gint);
EXTERN gchar **(*parse_keys_f)(const gchar *, gint *, const gchar * );
EXTERN gint (*get_multiplier_f)(DataSize );
EXTERN glong (*get_extreme_from_size_f)(DataSize, Extreme);
EXTERN gfloat (*convert_after_upload_f)(GtkWidget *);
EXTERN gint (*convert_before_download_f)(GtkWidget *, gfloat);
EXTERN GdkColor (*get_colors_from_hue_f)(gfloat, gfloat, gfloat);
EXTERN void (*mem_alloc_f)(void);
EXTERN void *(*evaluator_create_f)(char *);
EXTERN void (*evaluator_destroy_f)( void *);
EXTERN double (*evaluator_evaluate_x_f)(void *, double);
EXTERN void (*thread_widget_set_sensitive_f)(const gchar *, gboolean);
EXTERN void (*get_table_f)(gpointer, gpointer, gpointer);
EXTERN void (*free_multi_source_f)(gpointer);
EXTERN void (*flush_serial_f)(gint, gint);
EXTERN void (*_set_sized_data_f)(guint8 *, gint, DataSize, gint, gboolean);
EXTERN gint (*_get_sized_data_f)(guint8 *, gint, DataSize, gboolean);
EXTERN gboolean (*std_entry_handler_f)(GtkWidget *, gpointer);
EXTERN gboolean (*entry_changed_handler_f)(GtkWidget *, gpointer);
EXTERN gboolean (*search_model_f)(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
EXTERN void (*set_reqfuel_color_f)(GuiColor, gint);
EXTERN void (*alter_widget_state_f)(gpointer, gpointer);
EXTERN void (*bind_to_lists_f)(GtkWidget *, gchar * );
EXTERN void (*warn_user_f)(const gchar *);
EXTERN gboolean (*key_event_f)(GtkWidget *, GdkEventKey *, gpointer );
EXTERN gboolean (*focus_out_handler_f)(GtkWidget *, GdkEventFocus *, gpointer );
EXTERN guint32 (*create_value_change_watch_f)(const gchar *, gboolean, const gchar *, gpointer);
EXTERN void (*remove_from_lists_f)(gchar *, gpointer);
EXTERN void (*remove_watch_f)(guint32);
EXTERN void (*thread_refresh_widget_range_f)(gint, gint, gint);
EXTERN void (*thread_refresh_widgets_at_offset_f)(gint, gint);
EXTERN void (*update_ve3d_if_necessary_f)(gint, gint);
EXTERN void (*thread_refresh_widget_f)(GtkWidget *);
EXTERN void (*register_widget_f)(gchar *, GtkWidget *);
EXTERN gboolean (*write_wrapper_f)(gint, const void *, size_t, gint *);
EXTERN gboolean (*read_wrapper_f)(gint, void *, size_t, gint *);
EXTERN gint (*read_data_f)(gint , void **, gboolean);
EXTERN gboolean (*check_tab_existance_f)(gint);
EXTERN gboolean (*jump_to_tab_f)(GtkWidget *, gpointer );
EXTERN void (*add_additional_rtt_f)(GtkWidget *);
EXTERN gdouble (*temp_to_ecu_f)(gdouble);
EXTERN gdouble (*temp_to_host_f)(gdouble);
EXTERN gdouble (*c_to_k_f)(gdouble);
EXTERN gdouble (*f_to_k_f)(gdouble);
EXTERN void (*swap_labels_f)(GtkWidget *, gboolean);
EXTERN void (*combo_toggle_groups_linked_f)(GtkWidget *,gint);
EXTERN void (*combo_toggle_labels_linked_f)(GtkWidget *,gint);
EXTERN void (*combo_set_labels_f)(GtkWidget *, GtkTreeModel *);
EXTERN void (*recalc_table_limits_f)(gint, gint);
EXTERN gint (*get_choice_count_f)(GtkTreeModel *);


/* Function Pointers */

/* Prototypes */
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_common_enums(void);
void deregister_common_enums(void);
/* Prototypes */

#endif
