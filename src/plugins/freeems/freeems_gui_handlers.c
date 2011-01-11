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

#include <config.h>
#include <debugging.h>
#include <defines.h>
#include <firmware.h>
#include <freeems_gui_handlers.h>
#include <freeems_helpers.h>
#include <freeems_plugin.h>
#include <glade/glade.h>
#include <gtk/gtk.h>


extern gconstpointer *global_data;

G_MODULE_EXPORT void common_gui_init(void)
{
	void (*ecu_gui_init_f)(void) = NULL;
	/* This function is for doing any gui finalization on the CORE gui
	   for stuff specific to this firmware family.
	   */
	/* If ECU lib has a call run it */
	if (get_symbol_f("ecu_gui_init",(void *)&ecu_gui_init_f))
		ecu_gui_init_f();
}



G_MODULE_EXPORT gboolean common_button_handler(GtkWidget *widget, gpointer data)
{
	FreeEMSStdButton handler;

	handler = (FreeEMSStdButton)OBJ_GET(widget,"handler");

	switch (handler)
	{
		case SOFT_BOOT_ECU:
			soft_boot_ecu();
			break;
		case HARD_BOOT_ECU:
			hard_boot_ecu();
			break;
	}
	return TRUE;
}



G_MODULE_EXPORT gboolean common_bitmask_button_handler(GtkWidget *widget, gpointer data)
{
	static gboolean (*ecu_handler)(GtkWidget *, gpointer) = NULL;
	gint handler = 0;

	handler = (GINT)OBJ_GET(widget,"handler");

	/* No handlers yet, try ecu specific plugin */
	switch (handler)
	{
		default:
			if (!ecu_handler)
			{
				if (get_symbol_f("ecu_bitmask_button_handler",(void *)&ecu_handler))
					return ecu_handler(widget,data);
				else
					dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": common_bitmask_button_handler()\n\tDefault case, but there is NO ecu_bitmask_button_handler available, unhandled case for widget %s, BUG!\n",glade_get_widget_name(widget)));
			}
			else
				return ecu_handler(widget,data);
			break;
	}


	return TRUE;
}
