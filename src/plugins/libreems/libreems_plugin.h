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
  \file src/plugins/libreems/libreems_plugin.h
  \ingroup LibreEMSPlugin,Headers
  \brief LibreEMS Plugin init/shutdown functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LIBREEMS_PLUGIN_H__
#define __LIBREEMS_PLUGIN_H__

#include <plugindefines.h>
#include <gtk/gtk.h>
#include <debugging.h>
#include <defines.h>
#include <configfile.h>
#include <enums.h>
#include <threads.h>

#ifdef __LIBREEMS_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif


/* Function Pointers */
EXTERN gint (*_get_sized_data_f)(guint8 *, gint, DataSize, gboolean);
EXTERN void (*_set_sized_data_f)(guint8 *, gint, DataSize, gint, gboolean);
EXTERN void (*alter_widget_state_f)(gpointer, gpointer);
EXTERN void (*bind_to_lists_f)(GtkWidget *, gchar * );
EXTERN gboolean (*check_tab_existance_f)(gint);
EXTERN void (*cleanup_f)(void *);
EXTERN void (*close_binary_logs_f)(void);
EXTERN void (*combo_set_labels_f)(GtkWidget *, GtkTreeModel *);
EXTERN void (*combo_toggle_groups_linked_f)(GtkWidget *,gint);
EXTERN void (*combo_toggle_labels_linked_f)(GtkWidget *,gint);
EXTERN gfloat (*convert_after_upload_f)(GtkWidget *);
EXTERN gint (*convert_before_download_f)(GtkWidget *, gfloat);
EXTERN void (*dbg_func_f)(gint, const gchar *, const gchar *, gint, const gchar *, ...);
EXTERN gfloat (*direct_lookup_data_f)(gchar *, gint );
EXTERN gint (*direct_reverse_lookup_f)(gchar *, gint );
EXTERN void (*dump_output_f)(gint, guchar *);
EXTERN gboolean (*entry_changed_handler_f)(GtkWidget *, gpointer);
EXTERN void (*error_msg_f)(const gchar *);
EXTERN void *(*evaluator_create_f)(char *);
EXTERN void (*evaluator_destroy_f)( void *);
EXTERN double (*evaluator_evaluate_x_f)(void *, double);
EXTERN gboolean (*flush_binary_logs_f)(gpointer);
EXTERN void (*flush_serial_f)(gint, gint);
EXTERN gboolean (*focus_out_handler_f)(GtkWidget *, GdkEventFocus *, gpointer );
EXTERN guint (*get_bitshift_f)(guint);
EXTERN GdkColor (*get_colors_from_hue_f)(gfloat, gfloat, gfloat);
EXTERN glong (*get_extreme_from_size_f)(DataSize, Extreme);
EXTERN gboolean (*get_file_api_f)(ConfigFile *, gint *, gint * );
EXTERN GList *(*get_list_f)(const gchar *);
EXTERN gint (*get_multiplier_f)(DataSize);
EXTERN gboolean (*get_symbol_f)(const gchar *,void **);
EXTERN void (*get_table_f)(gpointer, gpointer, gpointer);
EXTERN OutputData *(*initialize_outputdata_f)(void);
EXTERN gboolean (*insert_text_handler_f)(GtkWidget *, gpointer);
EXTERN void (*io_cmd_f)(const gchar *,void *);
EXTERN gboolean (*jump_to_tab_f)(GtkWidget *, gpointer );
EXTERN gboolean (*key_event_f)(GtkWidget *, GdkEventKey *, gpointer );
EXTERN void (*log_inbound_data_f)(const void *, size_t);
EXTERN void (*log_outbound_data_f)(const void *, size_t);
EXTERN gboolean (*lookup_current_value_f)(const gchar *, gfloat *);
EXTERN gfloat (*lookup_data_f)(gconstpointer *, gint );
EXTERN gboolean (*lookup_precision_f)(const gchar *, gint *);
EXTERN GtkWidget *(*lookup_widget_f)(const gchar *);
EXTERN void (*mem_alloc_f)(void);
EXTERN void (*mtx_gauge_face_import_xml_wrapper_f)(GtkWidget *, gchar *);
EXTERN GtkWidget *(*mtx_gauge_face_new_wrapper_f)(void);
EXTERN void (*mtx_gauge_face_set_value_wrapper_f)(GtkWidget *, gfloat);
EXTERN void (*open_binary_logs_f)(void);
EXTERN gchar **(*parse_keys_f)(const gchar *, gint *, const gchar * );
EXTERN void (*process_rt_vars_f)(void *,gint );
EXTERN gboolean (*queue_function_f)(const gchar *);
EXTERN gint (*read_data_f)(gint , guint8 **, gboolean);
EXTERN gboolean (*read_wrapper_f)(gint, guint8 *, size_t, gint *);
EXTERN void (*recalc_table_limits_f)(gint, gint);
EXTERN char *(*regex_wrapper_f)(char *, char *, int *);
EXTERN void (*register_widget_f)(const gchar *, GtkWidget *);
EXTERN void (*remove_from_lists_f)(gchar *, gpointer);
EXTERN gint (*reverse_lookup_f)(gconstpointer *, gint );
EXTERN gboolean (*search_model_f)(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
EXTERN gboolean (*set_file_api_f)(ConfigFile *, gint, gint );
EXTERN void (*set_reqfuel_color_f)(GuiColor, gint);
EXTERN void (*set_title_f)(const gchar *);
EXTERN void (*set_widget_labels_f)(const gchar *);
EXTERN void (*set_widget_sensitive_f)(gpointer, gpointer);
EXTERN gboolean (*spin_button_handler_f)(GtkWidget *, gpointer);
EXTERN void (*start_restore_monitor_f)(void);
EXTERN void (*start_tickler_f)(gint);
EXTERN gboolean (*std_entry_handler_f)(GtkWidget *, gpointer);
EXTERN void (*stop_tickler_f)(gint);
EXTERN void (*swap_labels_f)(GtkWidget *, gboolean);
EXTERN gboolean (*table_color_refresh_wrapper_f)(gpointer);
EXTERN gdouble (*temp_to_ecu_f)(gdouble);
EXTERN gdouble (*temp_to_host_f)(gdouble);
EXTERN void (*thread_refresh_widget_f)(GtkWidget *);
EXTERN void (*thread_refresh_widget_range_f)(gint, gint, gint);
EXTERN void (*thread_refresh_widgets_at_offset_f)(gint, gint);
EXTERN void (*thread_set_group_color_f)(GuiColor, const gchar * );
EXTERN void (*thread_widget_set_sensitive_f)(const gchar *, gboolean);
EXTERN void (*thread_update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean);
EXTERN void (*thread_update_widget_f)(const gchar *, WidgetType, gchar *);
EXTERN gint (*translate_string_f)(const gchar *);
EXTERN void (*update_current_notebook_page_f)(void);
EXTERN void (*update_entry_color_f)(GtkWidget *, gint, gboolean, gboolean);
EXTERN void (*update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean, gboolean);
EXTERN void (*update_ve3d_if_necessary_f)(gint, gint);
EXTERN void (*warn_user_f)(const gchar *);
EXTERN gboolean (*write_wrapper_f)(gint, const void *, size_t, gint *);
/* Function Pointers */

/* Prototypes */
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_common_enums(void);
void deregister_common_enums(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
