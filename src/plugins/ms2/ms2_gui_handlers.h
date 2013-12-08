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
  \file src/plugins/ms2/ms2_gui_handlers.h
  \ingroup MS2Plugin,Headers
  \brief MS2 Gui handlers
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MS2_GUI_HANDLERS_H__
#define __MS2_GUI_HANDLERS_H__

#include <../mscommon/mscommon_gui_handlers.h>
#include <gtk/gtk.h>

typedef enum
{
	START_TOOTHMON_LOGGER = LAST_COMMON_TOGGLE_HANDLER_ENUM + 1,
	START_TRIGMON_LOGGER,
	STOP_TOOTHMON_LOGGER,
	STOP_TRIGMON_LOGGER,
	START_COMPOSITEMON_LOGGER,
	STOP_COMPOSITEMON_LOGGER
}MS2ToggleHandler;

typedef enum
{
	GET_CURR_TPS = LAST_COMMON_STD_HANDLER_ENUM + 1,
	MS2_USER_OUTPUTS
}MS2StdHandler;

/* Prototypes */
gboolean ecu_combo_handler(GtkWidget *, gpointer);
gboolean ecu_entry_handler(GtkWidget *, gpointer);
void ecu_gui_init();
gboolean ecu_std_button_handler(GtkWidget *, gpointer);
gboolean ecu_toggle_button_handler(GtkWidget *, gpointer);
gboolean ecu_update_combo(GtkWidget *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
