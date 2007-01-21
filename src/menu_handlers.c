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
#include <enums.h>
#include <menu_handlers.h>


static struct 
{
	const gchar *item;
	TabIdent tab;
}items[] = {
	{"vetable_tuning_menuitem",VETABLES_TAB},
	{"spark_tuning_menuitem",SPARKTABLES_TAB},
	{"afr_tuning_menuitem",AFRTABLES_TAB},
	{"boost_tuning_menuitem",BOOSTTABLES_TAB},
	{"rotary_tuning_menuitem",ROTARYTABLES_TAB},
	{"runtime_vars_menuitem",RUNTIME_TAB},
	{"ecu_errors_menuitem",ERROR_STATUS_TAB},
};

static struct 
{
	const gchar *item;
	FioAction action;
}fio_items[] = {
	{"import_tables_menuitem",VEX_IMPORT},
	{"export_tables_menuitem",VEX_EXPORT},
	{"restore_ecu_menuitem",ECU_RESTORE},
	{"backup_ecu_menuitem",ECU_BACKUP},
};

void setup_menu_handlers()
{
	GtkWidget *item = NULL;
	gint i = 0;
	GladeXML *xml = NULL;
	extern GtkWidget *main_window;

	xml = glade_get_widget_tree(main_window);
	
	for (i=0;i< (sizeof(items)/sizeof(items[0]));i++)
	{
		item = glade_xml_get_widget(xml,items[i].item);
		if (GTK_IS_WIDGET(item))
			g_object_set_data(G_OBJECT(item),"target_tab",
					GINT_TO_POINTER(items[i].tab));
		if (!check_tab_existance(items[i].tab))
			gtk_widget_set_sensitive(item,FALSE);
	}
	for (i=0;i< (sizeof(fio_items)/sizeof(fio_items[0]));i++)
	{
		item = glade_xml_get_widget(xml,fio_items[i].item);
		if (GTK_IS_WIDGET(item))
			g_object_set_data(G_OBJECT(item),"fio_action",
					GINT_TO_POINTER(fio_items[i].action));
	}
}

/*!
 \brief switches to tab encoded into the widget
 */
EXPORT gboolean jump_to_tab(GtkWidget *widget, gpointer data)
{
	extern GHashTable *dynamic_widgets;
	GtkWidget *notebook = NULL;
	TabIdent target = -1;
	TabIdent c_tab = 0;
	gint total = 0;
	GtkWidget * child = NULL;
	gint i = 0;
	
	notebook = g_hash_table_lookup(dynamic_widgets, "toplevel_notebook");
	if (!GTK_IS_NOTEBOOK(notebook))
		return FALSE;
	if (!g_object_get_data(G_OBJECT(widget),"target_tab"))
		return FALSE;
	target = (TabIdent)g_object_get_data(G_OBJECT(widget),"target_tab");
	total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	for (i=0;i<total;i++)
	{
		child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
		if (!g_object_get_data(G_OBJECT(child),"tab_ident"))
			continue;
		c_tab = (TabIdent)g_object_get_data(G_OBJECT(child),"tab_ident");
		if (c_tab == target)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),i);
			return TRUE;
		}
	}

	return FALSE;
}

/*!
 \brief General purpose handler to take care of menu initiated settings 
 transfers like VEX import/export and ECU backup/restore
 */
EXPORT gboolean settings_transfer(GtkWidget *widget, gpointer data)
{
	FioAction action = -1;
	action = (FioAction)g_object_get_data(G_OBJECT(widget),"fio_action");

	switch (action)
	{
		case VEX_IMPORT:
			printf("vex_import\n");
			break;
		case VEX_EXPORT:
			printf("vex_export\n");
			break;
		case ECU_BACKUP:
			printf("ecu_backup\n");
			break;
		case ECU_RESTORE:
			printf("ecu_restore\n");
			break;
	}
	return TRUE;
}

/*!
 \brief General purpose handler to take care of menu initiated settings 
 transfers like VEX import/export and ECU backup/restore
 */
gboolean check_tab_existance(TabIdent target)
{
	extern GHashTable *dynamic_widgets;
	GtkWidget *notebook = NULL;
	TabIdent c_tab = 0;
	gint total = 0;
	GtkWidget * child = NULL;
	gint i = 0;
	
	notebook = g_hash_table_lookup(dynamic_widgets, "toplevel_notebook");
	if (!GTK_IS_NOTEBOOK(notebook))
		return FALSE;
	total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	for (i=0;i<total;i++)
	{
		child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
		if (!g_object_get_data(G_OBJECT(child),"tab_ident"))
			continue;
		c_tab = (TabIdent)g_object_get_data(G_OBJECT(child),"tab_ident");
		if (c_tab == target)
		{
			return TRUE;
		}
	}
	return FALSE;
}

