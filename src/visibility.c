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
#include <fileio.h>
#include <glade/glade.h>
#include <stdlib.h>
#include <visibility.h>


static GtkWidget *vis_window = NULL;
extern GObject *global_data;

EXPORT gboolean show_tab_visibility_window(GtkWidget * widget, gpointer data)
{
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *table = NULL;
	GtkWidget *child = NULL;
	GtkWidget *label = NULL;
	GtkWidget *button = NULL;
	gboolean *hidden_list = NULL;
	gint rows = 0;
	gint i = 0;

	if (!(GTK_IS_WIDGET(vis_window)))
	{
		main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
		if (!main_xml)
			return FALSE;

		notebook = glade_xml_get_widget(main_xml,"toplevel_notebook");
		hidden_list = (gboolean *)OBJ_GET(global_data,"hidden_list");

		xml = glade_xml_new(main_xml->filename,"tab_visibility_top_vbox",NULL);

		vis_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(vis_window),"Tab Visibility");
		gtk_window_set_default_size(GTK_WINDOW(vis_window),200,300);
		g_signal_connect(G_OBJECT(vis_window),"delete_event",
				G_CALLBACK(gtk_widget_hide),vis_window);

		vbox = glade_xml_get_widget(xml,"tab_visibility_top_vbox");
		if (GTK_IS_WIDGET(vbox))
			gtk_container_add(GTK_CONTAINER(vis_window),vbox);
		else
			printf("ERROR, glade element not found!\n");

		rows = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
		OBJ_SET(global_data,"notebook_rows",GINT_TO_POINTER(rows));
		table = glade_xml_get_widget(xml,"tab_visibility_table");
		gtk_table_resize(GTK_TABLE(table),rows,2);

		for (i=0;i<rows;i++)
		{
			child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
			button = gtk_check_button_new();
			gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),hidden_list[i]);
			g_signal_connect(G_OBJECT(button),"toggled",
					G_CALLBACK(hide_tab),
					GINT_TO_POINTER(i));
			gtk_table_attach_defaults(GTK_TABLE(table),button,
					0,1,i+1,i+2);
			label = gtk_label_new(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook),child));
			gtk_table_attach_defaults(GTK_TABLE(table),label,
					1,2,i+1,i+2);
			
		}
	}

	gtk_widget_show_all(vis_window);
	return TRUE;
}


gboolean hide_tab(GtkWidget *widget, gpointer data)
{
	GtkWidget *child;
	GtkWidget *label;
	GtkWidget *notebook;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	extern GdkColor red;
	extern GdkColor black;
	extern GHashTable *dynamic_widgets;
	gint index = (gint)data;
	gint total = (gint)OBJ_GET(global_data,"notebook_rows");
	gint i = 0;
	gboolean hidden = FALSE;
	gint *hidden_list;

	main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	hidden_list = (gboolean *)OBJ_GET(global_data,"hidden_list");
	notebook = glade_xml_get_widget(main_xml,"toplevel_notebook");

	child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),index);
	label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook),child);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		gtk_widget_hide(child);
		gtk_widget_hide(label);
		hidden_list[index] = TRUE;
	}
	else
	{
		gtk_widget_show(child);
		gtk_widget_show(label);
		hidden_list[index] = FALSE;
	}

	for (i=0;i<total;i++)
	{
		if (hidden_list[i])
			hidden = TRUE;
	}
	item = g_hash_table_lookup(dynamic_widgets,"show_tab_visibility_menuitem");
	if (hidden)
		 gtk_widget_modify_text(GTK_BIN(item)->child,GTK_STATE_NORMAL,&red);
	else
		 gtk_widget_modify_text(GTK_BIN(item)->child,GTK_STATE_NORMAL,&black);
	return TRUE;
}

