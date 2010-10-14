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

#ifndef __GUI_HANDLERS_H__
#define __GUI_HANDLERS_H__

#include <defines.h>
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

typedef enum
{
	START_REALTIME = 0x20,
	STOP_REALTIME,
	START_PLAYBACK,
	STOP_PLAYBACK,
	READ_VE_CONST,
	READ_RAW_MEMORY,
	BURN_MS_FLASH,
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
	REBOOT_GETERR,
	REVERT_TO_BACKUP,
	BACKUP_ALL,
	RESTORE_ALL,
	SELECT_PARAMS,
	REQ_FUEL_POPUP,
	RESCALE_TABLE,
	REQFUEL_RESCALE_TABLE,
	EXPORT_SINGLE_TABLE,
	IMPORT_SINGLE_TABLE,
	TE_TABLE,
	TE_TABLE_GROUP,
	GET_CURR_TPS,
	INCREMENT_VALUE,
	DECREMENT_VALUE,
	PHONE_HOME
}StdButton;

/* Toggle/Radio buttons */
typedef enum
{
	TOOLTIPS_STATE=0x50,
	TRACKING_FOCUS,
	FAHRENHEIT,
	CELSIUS,
	COMMA,
	TAB,
	REALTIME_VIEW,
	PLAYBACK_VIEW,
	HEX_VIEW,
	BINARY_VIEW,
	DECIMAL_VIEW,
	OFFLINE_FIRMWARE_CHOICE,
	START_TOOTHMON_LOGGER,
	START_TRIGMON_LOGGER,
	STOP_TOOTHMON_LOGGER,
	STOP_TRIGMON_LOGGER,
	START_COMPOSITEMON_LOGGER,
	STOP_COMPOSITEMON_LOGGER,
	COMM_AUTODETECT,
	TOGGLE_NETMODE
}ToggleButton;

/* spinbuttons... */
typedef enum
{
	REQ_FUEL_DISP=0x80,
	REQ_FUEL_CYLS,
	REQ_FUEL_RATED_INJ_FLOW,
	REQ_FUEL_RATED_PRESSURE,
	REQ_FUEL_ACTUAL_PRESSURE,
	REQ_FUEL_AFR,
	LOCKED_REQ_FUEL,
	REQ_FUEL_1,
	REQ_FUEL_2,
	SER_INTERVAL_DELAY,
	SER_READ_TIMEOUT,
	RTSLIDER_FPS,
	RTTEXT_FPS,
	DASHBOARD_FPS,
	VE3D_FPS,
	SET_SER_PORT,
	NUM_SQUIRTS_1,
	NUM_SQUIRTS_2,
	NUM_CYLINDERS_1,
	NUM_CYLINDERS_2,
	NUM_INJECTORS_1,
	NUM_INJECTORS_2,
	TRIGGER_ANGLE,
	ODDFIRE_ANGLE,
	LOGVIEW_ZOOM,
	DEBUG_LEVEL,
	GENERIC,
	BAUD_CHANGE,
	MULTI_EXPRESSION,
	MS2_USER_OUTPUTS,
	ALT_SIMUL
}MtxButton;
/* Prototypes */
EXPORT gboolean prevent_close(GtkWidget *, gpointer );
EXPORT gboolean leave(GtkWidget *, gpointer);
EXPORT gboolean comm_port_override(GtkEditable *);
EXPORT gboolean std_button_handler(GtkWidget *, gpointer);
EXPORT gboolean std_entry_handler(GtkWidget *, gpointer);
EXPORT gboolean entry_changed_handler(GtkWidget *, gpointer );
EXPORT gboolean toggle_button_handler(GtkWidget *, gpointer);
EXPORT gboolean bitmask_button_handler(GtkWidget *, gpointer);
EXPORT gboolean spin_button_handler(GtkWidget *, gpointer);
EXPORT gboolean widget_grab(GtkWidget *, GdkEventButton *, gpointer );
EXPORT gboolean key_event(GtkWidget *, GdkEventKey *, gpointer );
EXPORT gboolean set_algorithm(GtkWidget *, gpointer );
EXPORT void notebook_page_changed(GtkNotebook *, GtkNotebookPage *, guint, gpointer);
EXPORT void subtab_changed(GtkNotebook *, GtkNotebookPage *, guint, gpointer);
EXPORT gboolean focus_out_handler(GtkWidget *, GdkEventFocus *, gpointer );
EXPORT gboolean slider_value_changed(GtkWidget *, gpointer );
void update_ve_const_pf(void);
gboolean force_update_table(gpointer);
gboolean trigger_group_update(gpointer );
void update_widget(gpointer, gpointer );
void switch_labels(gpointer , gpointer );
void swap_labels(gchar *, gboolean );
void toggle_groups_linked(GtkWidget *, gboolean);
void prompt_to_save(void);
gboolean prompt_r_u_sure(void);
void combo_toggle_groups_linked(GtkWidget *,gint);
void combo_toggle_labels_linked(GtkWidget *,gint);
gint get_choice_count(GtkTreeModel *);
guint get_bitshift(guint );
EXPORT void update_misc_gauge(DataWatch *);
void refresh_widgets_at_offset(gint, gint);
glong get_extreme_from_size(DataSize, Extreme);
EXPORT gboolean clamp_value(GtkWidget *, gpointer);
void recalc_table_limits(gint, gint);
gboolean update_multi_expression(gpointer);
void refocus_cell(GtkWidget *, Direction);
void set_widget_label_from_array(gpointer, gpointer);
void insert_text_handler(GtkEntry *, const gchar *, gint, gint *, gpointer);



/* Prototypes */

#endif
