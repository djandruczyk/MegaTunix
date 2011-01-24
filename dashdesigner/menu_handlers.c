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
#include <gtk/gtk.h>
#include <menu_handlers.h>
#include <events.h>
#include <loadsave.h>


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
	item = gtk_menu_item_new_with_mnemonic("_Dashboard");
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),item);

	/* Dashboard Menu */
	item = gtk_image_menu_item_new_with_mnemonic("_Add Gauge");
	OBJ_SET(toplevel,"new_dash_menuitem",item);
	image = gtk_image_new_from_stock("gtk-add",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(create_preview_list),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Load Dash");
	OBJ_SET(toplevel,"load_dash_menuitem",item);
	image = gtk_image_new_from_stock("gtk-open",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(load_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Close Dash");
	OBJ_SET(toplevel,"close_dash_menuitem",item);
	image = gtk_image_new_from_stock("gtk-close",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(close_current_dash),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Save Dash");
	OBJ_SET(toplevel,"save_dash_menuitem",item);
	image = gtk_image_new_from_stock("gtk-save",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(save_handler),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_image_menu_item_new_with_mnemonic("Save Dash _As");
	OBJ_SET(toplevel,"save_dash_as_menuitem",item);
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
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(dashdesigner_quit),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	gtk_widget_show_all(menu);

	/* Edit Menu */
	item = gtk_menu_item_new_with_mnemonic("_Edit");
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),item);

	item = gtk_image_menu_item_new_with_mnemonic("_Optimize Dash Size");
	image = gtk_image_new_from_stock("gtk-zoom-in",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(optimize_dash_size),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	/* Help Menu */
	item = gtk_menu_item_new_with_mnemonic("_Help");
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),item);

	item = gtk_image_menu_item_new_with_mnemonic("_About DashDesigner");
	image = gtk_image_new_from_stock("gtk-about",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(dashdesigner_about),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	return;
}
