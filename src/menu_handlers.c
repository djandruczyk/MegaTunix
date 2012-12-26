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
  \file src/menu_handlers.c
  \ingroup CoreMtx
  \brief Sets up the global Mtx menu's applicable to all firmware variants

  This checks for and calls the ECU plugin function that enables the ECU
  specific menu handlers as appropriate
  \author David Andruczyk
  */

#include <debugging.h>
#include <firmware.h>
#include <menu_handlers.h>
#include <plugin.h>
#include <tableio.h>
#include <widgetmgmt.h>

extern gconstpointer *global_data;
static struct 
{
	const gchar *item;
	TabIdent tab;
}items[] = {
	{"vetable_tuning_menuitem",VETABLES_TAB},
	{"spark_tuning_menuitem",SPARKTABLES_TAB},
	{"afr_tuning_menuitem",AFRTABLES_TAB},
	{"runtime_vars_menuitem",RUNTIME_TAB},
};

static struct 
{
	const gchar *item;
	FioAction action;
	gboolean sensitivity;
}fio_items[] = {
	{"import_tables_menuitem",ALL_TABLE_IMPORT,FALSE},
	{"export_tables_menuitem",ALL_TABLE_EXPORT,FALSE},
	{"restore_ecu_menuitem",ECU_RESTORE,TRUE},
	{"backup_ecu_menuitem",ECU_BACKUP,TRUE},
};

/*!
  \brief Sets up the menu handlers and additional menu options as well as 
  calling the common plugin's menu setup handler. (which in turn calls the
  ecu specific plugin handler)
  */
G_MODULE_EXPORT void setup_menu_handlers_pf(void)
{
	void (*common_plugin_menu_setup)(GladeXML *);
	GtkWidget *item = NULL;
	guint i = 0;
	GladeXML *xml = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!xml) || (DATA_GET(global_data,"leaving")))
	{
		EXIT();
		return;
	}

	if (get_symbol ("common_plugin_menu_setup",(void **)&common_plugin_menu_setup))
		common_plugin_menu_setup(xml);

	item = glade_xml_get_widget(xml,"show_tab_visibility_menuitem");
	gtk_widget_set_sensitive(item,TRUE);
	
	for (i=0;i< (sizeof(items)/sizeof(items[0]));i++)
	{
		item = glade_xml_get_widget(xml,items[i].item);
		if (GTK_IS_WIDGET(item))
			OBJ_SET(item,"target_tab",
					GINT_TO_POINTER(items[i].tab));
		if (check_tab_existance(items[i].tab))
			gtk_widget_set_sensitive(item,TRUE);
		else
			gtk_widget_set_sensitive(item,FALSE);
	}
	for (i=0;i< (sizeof(fio_items)/sizeof(fio_items[0]));i++)
	{
		item = glade_xml_get_widget(xml,fio_items[i].item);
		if (GTK_IS_WIDGET(item))
		{
			OBJ_SET(item,"fio_action",
					GINT_TO_POINTER(fio_items[i].action));
			gtk_widget_set_sensitive(item,fio_items[i].sensitivity);
		}
	}
	EXIT();
	return;
}

/*!
  \brief switches to tab encoded into the widget
  \param widget is the the menuitem the user clicked upon
  \param data is unused
  \returns TRUE if handled, FALSE otherwise
  */
G_MODULE_EXPORT gboolean jump_to_tab(GtkWidget *widget, gpointer data)
{
	GtkWidget *notebook = NULL;
	TabIdent target;
	TabIdent c_tab;
	gint total = 0;
	GtkWidget * child = NULL;
	gint i = 0;
	
	ENTER();
	notebook = lookup_widget( "toplevel_notebook");
	if (!GTK_IS_NOTEBOOK(notebook))
	{
		EXIT();
		return FALSE;
	}
	if (!OBJ_GET(widget,"target_tab"))
	{
		EXIT();
		return FALSE;
	}
	target = (TabIdent)(GINT)OBJ_GET(widget,"target_tab");
	total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	for (i=0;i<total;i++)
	{
		child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
		if (!OBJ_GET(child,"tab_ident"))
			continue;
		c_tab = (TabIdent)(GINT)OBJ_GET(child,"tab_ident");
		if (c_tab == target)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),i);
			EXIT();
			return TRUE;
		}
	}
	EXIT();
	return FALSE;
}

/*!
  \brief General purpose handler to take care of menu initiated settings 
  transfers like Table import/export and ECU backup/restore
  \param widget is the pointer to widget the user clicket on
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean settings_transfer(GtkWidget *widget, gpointer data)
{
	FioAction action;
	action = (FioAction)(GINT)OBJ_GET(widget,"fio_action");
	void (*do_backup)(GtkWidget *, gpointer) = NULL;
	void (*do_restore)(GtkWidget *, gpointer) = NULL;

	ENTER();
	switch (action)
	{
		case ALL_TABLE_IMPORT:
			//select_all_tables_for_import();
			break;
		case ALL_TABLE_EXPORT:
			//select_all_tables_for_export();
			break;
		case ECU_BACKUP:
			if (get_symbol("select_file_for_ecu_backup",(void **)&do_backup))
				do_backup(NULL,NULL);
			break;
		case ECU_RESTORE:
			if (get_symbol("select_file_for_ecu_restore",(void **)&do_restore))
				do_restore(NULL,NULL);
			break;
	}
	EXIT();
	return TRUE;
}

/*!
  \brief General purpose handler to take care of menu initiated settings 
  transfers like table import/export and ECU backup/restore
  \param target is the enumeration for a TAB to be checked if it exists
  \returns TRUE if tab found, FALSE if not
  */
G_MODULE_EXPORT gboolean check_tab_existance(TabIdent target)
{
	GtkWidget *notebook = NULL;
	TabIdent c_tab;
	gint total = 0;
	GtkWidget * child = NULL;
	gint i = 0;
	
	ENTER();
	notebook = lookup_widget( "toplevel_notebook");
	if (!GTK_IS_NOTEBOOK(notebook))
	{
		EXIT();
		return FALSE;
	}
	total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	for (i=0;i<total;i++)
	{
		child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
		if (!OBJ_GET(child,"tab_ident"))
			continue;
		c_tab = (TabIdent)(GINT)OBJ_GET(child,"tab_ident");
		if (c_tab == target)
		{
			EXIT();
			return TRUE;
		}
	}
	EXIT();
	return FALSE;
}


