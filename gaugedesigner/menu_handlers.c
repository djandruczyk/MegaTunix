/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
#include <gtk/gtk.h>
#include <menu_handlers.h>
#include <events.h>
#include <loadsave.h>
#include <handlers.h>


void menu_setup(GtkBuilder *toplevel)
{
	GtkWidget *bar = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *menubar = NULL;
	GtkWidget *menu = NULL;
	GtkWidget *item = NULL;
	GtkWidget *image = NULL;

	vbox = GTK_WIDGET (gtk_builder_get_object(toplevel,"menu_vbox"));
	menubar = gtk_menu_bar_new();
	gtk_box_pack_end(GTK_BOX(vbox),menubar,FALSE,FALSE,0);
	item = gtk_menu_item_new_with_label("File");
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),item);

	/* File Menu */
	item = gtk_image_menu_item_new_with_mnemonic("_New Gauge");
	OBJ_SET(toplevel,"new_gauge_menuitem",item);
	image = gtk_image_new_from_stock("gtk-new",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(create_new_gauge),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Load Gauge");
	OBJ_SET(toplevel,"load_gauge_menuitem",item);
	image = gtk_image_new_from_stock("gtk-open",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(load_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Close");
	OBJ_SET(toplevel,"close_gauge_menuitem",item);
	image = gtk_image_new_from_stock("gtk-close",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(close_current_gauge),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("S_ave Gauge");
	OBJ_SET(toplevel,"save_gauge_menuitem",item);
	image = gtk_image_new_from_stock("gtk-save",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(save_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("S_ave Gauge As");
	OBJ_SET(toplevel,"save_as_menuitem",item);
	image = gtk_image_new_from_stock("gtk-save-as",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(save_as_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Quit");
	image = gtk_image_new_from_stock("gtk-quit",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(quit_gaugedesigner),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	gtk_widget_show_all(menu);

	/* Edit Menu */
	item = gtk_menu_item_new_with_label("Edit");
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),item);

	item = gtk_image_menu_item_new_with_mnemonic("_General Attributes");
	image = gtk_image_new_from_stock("gtk-select-color",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(general_attributes_menu_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("Gauge Te_xt");
	image = gtk_image_new_from_stock("gtk-italic",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(text_attributes_menu_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("Gauge _Tickmarks");
	image = gtk_image_new_from_stock("gtk-preferences",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(tick_groups_menu_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("Gauge _Warning Thresholds");
	image = gtk_image_new_from_stock("gtk-properties",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(warning_ranges_menu_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("Gauge _Alert Thresholds");
	image = gtk_image_new_from_stock("gtk-dialog-warning",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(alert_ranges_menu_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("Gauge _Advanced Graphics");
	image = gtk_image_new_from_stock("gtk-zoom-in",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(polygon_menu_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	gtk_widget_show_all(menu);

	/* Help Menu */
	item = gtk_menu_item_new_with_label("Help");
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),item);

	item = gtk_image_menu_item_new_with_mnemonic("_About GaugeDesigner");
	image = gtk_image_new_from_stock("gtk-about",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(about_menu_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	return;
}
