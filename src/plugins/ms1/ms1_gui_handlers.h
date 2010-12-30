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

#ifndef __MS1_GUI_HANDLERS_H__
#define __MS1_GUI_HANDLERS_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

/* Prototypes */
gboolean ecu_entry_handler(GtkWidget *, gpointer);
gboolean ecu_spin_button_handler(GtkWidget *, gpointer);
gboolean ecu_std_button_handler(GtkWidget *, gpointer);
gboolean ecu_toggle_button_handler(GtkWidget *, gpointer);
gboolean ecu_combo_handler(GtkWidget *, gpointer);
void ecu_update_entry(GtkWidget *);
void ecu_gui_init(GladeXML *);
/* Prototypes */

typedef enum
{
        START_TOOTHMON_LOGGER = LAST_TOGGLE_ENUM + 1,
        STOP_TOOTHMON_LOGGER,
        START_TRIGMON_LOGGER,
        STOP_TRIGMON_LOGGER
}MS1ToggleButton;

typedef enum
{
        TRIGGER_ANGLE = LAST_BUTTON_ENUM + 1,
        ODDFIRE_ANGLE
}MS1MtxButton;

#endif

