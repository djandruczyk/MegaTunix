/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/plugins/null/null_gui_handlers.c
  \ingroup NullPlugin,Plugins
  \brief Null plugin gui handler stubs
  \author David Andruczyk
  */

#include <null_plugin.h>
#include <null_gui_handlers.h>


extern gconstpointer *global_data;

/*!
  \brief This is used for this plugin to initialize stuff on the main Gui
  */
G_MODULE_EXPORT void ecu_gui_init(void)
{
	ENTER();
	/* We don't need anything specific to this ecu initialized */
	EXIT();
	return;
}


/*!
  \brief ECU specific plugin handler for toggle buttons
  \param widget is the pointer to the toggle button
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_toggle_button_handler(GtkWidget *widget, gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific plugin handler for standard buttons
  \param widget is the pointer to the standard button
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_std_button_handler(GtkWidget *widget, gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific plugin handler for radio/check buttons
  \param widget is the pointer to the radio/check button
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_bitmask_button_handler(GtkWidget *widget, gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific plugin handler for spin buttons
  \param widget is the pointer to the spin button
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_spin_button_handler(GtkWidget *widget, gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific plugin handler for text entries
  \param widget is the pointer to the text entry
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_entry_handler(GtkWidget *widget, gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}



/*!
  \brief ECU specific plugin handler for combo boxes
  \param widget is the pointer to the combo box 
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_combo_handler(GtkWidget *widget, gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}

