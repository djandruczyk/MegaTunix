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
	{"runtime_vars_menuitem",RUNTIME_TAB},
	{"ecu_errors_menuitem",ERROR_STATUS_TAB},
};

void setup_menu_handlers(GladeXML *xml)
{
	GtkWidget *item = NULL;
	gint i = 0;
	
	for (i=0;i< (sizeof(items)/sizeof(items[0]));i++)
	{
		item = glade_xml_get_widget(xml,items[i].item);
		if (GTK_IS_WIDGET(item))
			g_object_set_data(G_OBJECT(item),"target_tab",
					GINT_TO_POINTER(items[i].tab));
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
	target = (TabIdent)g_object_get_data(G_OBJECT(widget),"target_tab");
	total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	for (i=0;i<total;i++)
	{
		child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
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
 \brief General purpose handler to take car of menu initiated settings 
 transfers like VEX import/export and ECU backup/restore
 */
EXPORT gboolean settings_transfer(GtkWidget *widget, gpointer data)
{
	printf("settings_transfer needs a little work\n");
	return TRUE;
}

