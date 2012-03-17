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
  \file include/gui_handlers.h
  \ingroup Headers
  \brief Header for the global gui handlers
  \author David Andruczyk
  */

#ifndef __GUI_HANDLERS_H__
#define __GUI_HANDLERS_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <watches.h>

/* Regular Buttons */
typedef enum
{
	GO_LEFT = 0x990,
	GO_RIGHT,
	GO_UP,
	GO_DOWN
}Direction;

/* Toggle/Radio global button handlers */
typedef enum
{
	TOOLTIPS_STATE=0x500,
	LOG_RAW_DATASTREAM,
	TRACKING_FOCUS,
	COMMA,
	TAB,
	REALTIME_VIEW,
	PLAYBACK_VIEW,
	OFFLINE_FIRMWARE_CHOICE,
	ECU_PERSONA,
	COMM_AUTODETECT,
	TOGGLE_NETMODE,
	TOGGLE_KELVIN,
	TOGGLE_CELSIUS,
	TOGGLE_FAHRENHEIT,
	TOGGLE_FIXED_COLOR_SCALE,
	TOGGLE_AUTO_COLOR_SCALE,
	LAST_GLOBAL_TOGGLE_HANDLER_ENUM
}ToggleHandler;


/* Standard global button handlers */
typedef enum
{
        START_REALTIME = 0x520,
        STOP_REALTIME,
        START_PLAYBACK,
        STOP_PLAYBACK,
        READ_VE_CONST,
        READ_RAW_MEMORY,
        BURN_FLASH,
        INTERROGATE_ECU,
        OFFLINE_MODE,
        SELECT_DLOG_EXP,
        SELECT_DLOG_IMP,
        DLOG_SELECT_DEFAULTS,
        DLOG_SELECT_ALL,
        DLOG_DESELECT_ALL,
        DLOG_DUMP_INTERNAL,
        CLOSE_LOGFILE,
        START_DATALOGGING,
        STOP_DATALOGGING,
        EXPORT_VETABLE,
        IMPORT_VETABLE,
        REVERT_TO_BACKUP,
        BACKUP_ALL,
        RESTORE_ALL,
        SELECT_PARAMS,
        RESCALE_TABLE,
        EXPORT_SINGLE_TABLE,
        IMPORT_SINGLE_TABLE,
        TE_TABLE,
        TE_TABLE_GROUP,
        PHONE_HOME,
	SER_INTERVAL_DELAY,
        SER_READ_TIMEOUT,
        RTSLIDER_FPS,
        RTTEXT_FPS,
        DASHBOARD_FPS,
        VE3D_FPS,
        SET_SER_PORT,
        LOGVIEW_ZOOM,
        BAUD_CHANGE,
        DEBUG_LEVEL,
        LAST_GLOBAL_STD_HANDLER_ENUM
}StdHandler;


/* Prototypes */
gboolean prevent_close(GtkWidget *, gpointer );
gboolean leave(GtkWidget *, gpointer);
gboolean comm_port_override(GtkEditable *);
gboolean std_button_handler(GtkWidget *, gpointer);
gboolean std_entry_handler(GtkWidget *, gpointer);
gboolean std_combo_handler(GtkWidget *, gpointer);
gboolean entry_changed_handler(GtkWidget *, gpointer );
gboolean toggle_button_handler(GtkWidget *, gpointer);
gboolean bitmask_button_handler(GtkWidget *, gpointer);
gboolean spin_button_handler(GtkWidget *, gpointer);
gboolean widget_grab(GtkWidget *, GdkEventButton *, gpointer );
gboolean key_event(GtkWidget *, GdkEventKey *, gpointer );
gboolean set_algorithm(GtkWidget *, gpointer );
void notebook_page_changed(GtkNotebook *, GtkWidget *, guint, gpointer);
void cancel_visible_function(GtkNotebook *, GtkWidget *, guint, gpointer);
void subtab_changed(GtkNotebook *, GtkWidget *, guint, gpointer);
gboolean focus_out_handler(GtkWidget *, GdkEventFocus *, gpointer );
gboolean slider_value_changed(GtkWidget *, gpointer );
gboolean force_update_table(gpointer);
gboolean trigger_group_update(gpointer );
void prompt_to_save(void);
gboolean prompt_r_u_sure(void);
guint get_bitshift(guint );
void update_misc_gauge(RtvWatch *);
glong get_extreme_from_size(DataSize, Extreme);
gboolean clamp_value(GtkWidget *, gpointer);
void refocus_cell(GtkWidget *, Direction);
void set_widget_label_from_array(gpointer, gpointer);
void insert_text_handler(GtkEntry *, const gchar *, gint, gint *, gpointer);
void set_widget_labels(const gchar *);
void swap_labels(GtkWidget *, gboolean);
void switch_labels(gpointer, gpointer);
void combo_toggle_groups_linked(GtkWidget *,gint);
void combo_toggle_labels_linked(GtkWidget *,gint);
void set_widget_label_from_array(gpointer, gpointer);
void combo_set_labels(GtkWidget *, GtkTreeModel *);
gint get_choice_count(GtkTreeModel *);
void recalc_table_limits(gint, gint);
void update_groups_pf(void);
void update_sources_pf(void);
void process_group(gpointer, gpointer);
void process_source(gpointer, gpointer);
void update_current_notebook_page(void);
void update_entry_color(GtkWidget *, gint, gboolean, gboolean);
void update_entry_color_wrapper(gpointer, gpointer);
gboolean table_color_refresh(gpointer);
/* Prototypes */

#endif
