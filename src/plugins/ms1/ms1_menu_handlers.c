/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \file src/plugins/ms1/ms1_menu_handlers.c
  \ingroup MS1Plugin,Plugins
  \brief MS1 Plugin menu init handlers
  \author David Andruczyk
  */

#include <firmware.h>
#include <ms1_plugin.h>
#include <ms1_menu_handlers.h>

extern gconstpointer *global_data;

/*!
  \brief Handler to setup any ecu specific menu objects on the core Gui
  \param xml is a pointer to the core Gui xml
  */
G_MODULE_EXPORT void ecu_plugin_menu_setup(GladeXML *xml)
{
	Firmware_Details *firmware = NULL;
	GtkWidget *item = NULL;
	GtkWidget *menu = NULL;
	GtkWidget *image = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (firmware->capabilities & MS1)
	{
		menu = glade_xml_get_widget (xml, "tools_menu_menu");
		item = gtk_image_menu_item_new_with_mnemonic("MS-1 LookupTables Mgmt");
		image = gtk_image_new_from_stock("gtk-index",GTK_ICON_SIZE_MENU);
		g_object_set(item,"image",image,NULL);
		if (gtk_minor_version >= 16)
			g_object_set(item,"always-show-image",TRUE,NULL);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(lookuptables_configurator_f),NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		gtk_widget_show_all(menu);
	}
	EXIT();
	return;
}
