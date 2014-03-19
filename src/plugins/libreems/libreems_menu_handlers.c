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
  \file src/plugins/libreems/libreems_menu_handlers.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS Menu handlers for ECU specific menu hooks
  \author David Andruczyk
  */

#include <libreems_helpers.h>
#include <libreems_menu_handlers.h>
#include <libreems_plugin.h>

extern gconstpointer *global_data;


/*!
  \brief Sets up the main gui menu with stuff common to this ECU persona.
  It checks for the presence of an ecu/firmware specific handler and runs that
  as well
  \param xml is a pointer to the GladeXML structure for the core Gui
  */
G_MODULE_EXPORT void common_plugin_menu_setup(GladeXML *xml)
{
	void (*ecu_plugin_menu_setup)(GladeXML *) = NULL;
	GtkWidget *menu = NULL;
	GtkWidget *item = NULL;
	GtkWidget *image = NULL;

	ENTER();
	/* View->Tabs Menu */
	/*
	   menu = glade_xml_get_widget (xml, "goto_tab1_menu");
	   item = gtk_menu_item_new_with_mnemonic("_Boost Tables");
	   g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(jump_to_tab_f),NULL);
	   OBJ_SET(item,"target_tab",GINT_TO_POINTER(BOOSTTABLES_TAB));
	   if (!check_tab_existance_f(BOOSTTABLES_TAB))
	   gtk_widget_set_sensitive(item,FALSE);
	   else
	   gtk_widget_set_sensitive(item,TRUE);
	   gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	   item = gtk_menu_item_new_with_mnemonic("_Staging Tables");
	   g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(jump_to_tab_f),NULL);
	   OBJ_SET(item,"target_tab",GINT_TO_POINTER(STAGING_TAB));
	   if (!check_tab_existance_f(STAGING_TAB))
	   gtk_widget_set_sensitive(item,FALSE);
	   else
	   gtk_widget_set_sensitive(item,TRUE);
	   gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	   item = gtk_menu_item_new_with_mnemonic("_Rotary Tables");
	   g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(jump_to_tab_f),NULL);
	   OBJ_SET(item,"target_tab",GINT_TO_POINTER(ROTARYTABLES_TAB));
	   if (!check_tab_existance_f(ROTARYTABLES_TAB))
	   gtk_widget_set_sensitive(item,FALSE);
	   else
	   gtk_widget_set_sensitive(item,TRUE);
	   gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	   gtk_widget_show_all(menu);
	   */

	/* View Menu */
	menu = glade_xml_get_widget (xml, "view_menu_menu");
	item = gtk_image_menu_item_new_with_mnemonic("ECU _Errors");
	image = gtk_image_new_from_stock("gtk-stop",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(jump_to_tab_f),NULL);
	OBJ_SET(item,"target_tab",GINT_TO_POINTER(ERROR_STATUS_TAB));
	if (!check_tab_existance_f(ERROR_STATUS_TAB))
		gtk_widget_set_sensitive(item,FALSE);
	else
		gtk_widget_set_sensitive(item,TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	gtk_widget_show_all(menu);

	/* Tuning Menu */
	/*
	   menu = glade_xml_get_widget (xml, "generate1_menu");
	   item = gtk_menu_item_new_with_mnemonic("_Ignition Map");
	   g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(show_create_ignition_map_window),NULL);
	   gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	   gtk_widget_show_all(menu);
	   */

	/* Tools Menu */
	menu = glade_xml_get_widget (xml, "tools_menu_menu");

	item = gtk_image_menu_item_new_with_mnemonic("_Reset ALL Counters");
	image = gtk_image_new_from_stock("gtk-execute",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(reset_counters),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Stop HiSpeed Streaming");
	image = gtk_image_new_from_stock("gtk-stop",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(stop_streaming),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Start HiSpeed Streaming");
	image = gtk_image_new_from_stock("gtk-go-forward",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(start_streaming),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Soft Boot ECU");
	image = gtk_image_new_from_stock("gtk-refresh",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(soft_boot_ecu),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Hard Boot ECU");
	image = gtk_image_new_from_stock("gtk-help",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(hard_boot_ecu),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	gtk_widget_show_all(menu);

	if (get_symbol_f("ecu_plugin_menu_setup",(void **)&ecu_plugin_menu_setup))
		ecu_plugin_menu_setup(xml);
	EXIT();
	return;
}

/*!
 *\brief Handler to setup any ecu specific menu objects on the core Gui
 *\param xml is a pointer to the core Gui xml
 */
G_MODULE_EXPORT void ecu_plugin_menu_setup(GladeXML *xml)
{
	ENTER();
	EXIT();
	return;
	/* Don't need to do anything yet */
}
