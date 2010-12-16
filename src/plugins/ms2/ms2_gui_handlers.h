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
