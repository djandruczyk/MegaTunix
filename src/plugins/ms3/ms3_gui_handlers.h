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
  \file src/plugins/ms3/ms3_gui_handlers.h
  \ingroup MS3Plugin,Headers
  \brief MS3 Plugin specific gui handlers
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif
	
#ifndef __NULL_GUI_HANDLERS_H__
#define __NULL_GUI_HANDLERS_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>

/* Enumerations */
/* Enumerations */

/* Prototypes */
void ecu_gui_init(void);
gboolean ecu_toggle_button_handler(GtkWidget *, gpointer);
gboolean ecu_button_handler(GtkWidget *, gpointer);
gboolean ecu_bitmask_button_handler(GtkWidget *, gpointer);
gboolean ecu_spin_button_handler(GtkWidget *, gpointer);
gboolean ecu_entry_handler(GtkWidget *, gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
