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
#include <defines.h>
#include <firmware.h>
#include <ms1_plugin.h>
#include <ms1_menu_handlers.h>

extern gconstpointer *global_data;

G_MODULE_EXPORT void ecu_plugin_menu_setup(GladeXML *xml)
{
	Firmware_Details *firmware = NULL;
	GtkWidget *item = NULL;

	firmware = DATA_GET(global_data,"firmware");

	gdk_threads_enter();
	if (firmware->capabilities & MS1)
	{
		item = glade_xml_get_widget(xml,"lookuptables_setup_menuitem");
		gtk_widget_set_sensitive(item,TRUE);
	}
	gdk_threads_leave();
	return;
}
