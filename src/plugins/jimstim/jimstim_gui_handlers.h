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

/*!
  \file src/plugins/jimstim/jimstim_gui_handlers.h
  \ingroup JimStimPlugin,Headers
  \brief JimStim gui handlers
  \author David Andruczyk
  */
 
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __JIMSTIM_GUI_HANDLERS_H__
#define __JIMSTIM_GUI_HANDLERS_H__

#include <gtk/gtk.h>
#include <../mscommon/mscommon_gui_handlers.h>
#include <glade/glade.h>

/* Enumerations */
typedef enum
{
	SWEEP_START = LAST_COMMON_STD_HANDLER_ENUM + 1,
	SWEEP_STOP,
	RPM_MODE
}JimStimStdHandler;

/* Prototypes */
gboolean ecu_bitmask_button_handler(GtkWidget *, gpointer);
gboolean ecu_button_handler(GtkWidget *, gpointer);
gboolean ecu_entry_handler(GtkWidget *, gpointer);
void ecu_gui_init(void);
gboolean ecu_spin_button_handler(GtkWidget *, gpointer);
gboolean ecu_toggle_button_handler(GtkWidget *, gpointer);
gboolean ecu_update_combo(GtkWidget *, gpointer);
gboolean jimstim_rpm_value_changed(GtkWidget *, gpointer);
/* Prototypes */
#endif

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
