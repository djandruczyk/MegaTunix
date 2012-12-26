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
  \file src/visibility.c
  \ingroup CoreMtx
  \brief Handles hiding of tabs the user deems are not applicable to their
  needs
  \author David Andruczyk
  */

#include <config.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <glade/glade.h>
#include <stdlib.h>
#include <visibility.h>
#include <widgetmgmt.h>

static GtkWidget *vis_window = NULL;
extern gconstpointer *global_data;

/*!
  \brief shows the tabe visibility chooser window
  \param widget is the widget clicked to get this to come up
  \param data is unused
  \return TRUE on success
  */
G_MODULE_EXPORT gboolean show_tab_visibility_window(GtkWidget * widget, gpointer data)
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

	ENTER();
	if (!(GTK_IS_WIDGET(vis_window)))
	{
		main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
		if (!main_xml)
		{
			EXIT();
			return FALSE;
		}

		notebook = glade_xml_get_widget(main_xml,"toplevel_notebook");
		hidden_list = (gboolean *)DATA_GET(global_data,"hidden_list");

		xml = glade_xml_new(main_xml->filename,"tab_visibility_top_vbox",NULL);

		vis_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_transient_for(GTK_WINDOW(vis_window),GTK_WINDOW(lookup_widget("main_window")));
		gtk_window_set_title(GTK_WINDOW(vis_window),_("Tab Visibility"));
		gtk_window_set_default_size(GTK_WINDOW(vis_window),200,300);
		g_signal_connect(G_OBJECT(vis_window),"delete_event",
				G_CALLBACK(gtk_widget_hide),vis_window);

		vbox = glade_xml_get_widget(xml,"tab_visibility_top_vbox");
		if (GTK_IS_WIDGET(vbox))
			gtk_container_add(GTK_CONTAINER(vis_window),vbox);
		else
			printf(_("ERROR, glade element not found!\n"));

		gint rows = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
		DATA_SET(global_data,"notebook_rows",GINT_TO_POINTER(rows));
		table = glade_xml_get_widget(xml,"tab_visibility_table");
		gtk_table_resize(GTK_TABLE(table),rows,2);

		for (gint i=0;i<rows;i++)
		{
			child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
			button = gtk_check_button_new();
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hidden_list[i]);
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
	EXIT();
	return TRUE;
}


/*!
  \brief hides a tab
  \param widget is the widget clicked to get this to come up
  \param data is the index to the tab to hide
  \return TRUE on success
  */
G_MODULE_EXPORT gboolean hide_tab(GtkWidget *widget, gpointer data)
{
	GtkWidget *child;
	GtkWidget *label;
	GtkWidget *notebook;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	extern GdkColor red;
	extern GdkColor black;
	gint index = (GINT)data;
	gint total = (GINT)DATA_GET(global_data,"notebook_rows");
	gint i = 0;
	gboolean hidden = FALSE;
	gint *hidden_list;

	ENTER();
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	hidden_list = (gboolean *)DATA_GET(global_data,"hidden_list");
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
	item = lookup_widget("show_tab_visibility_menuitem");
	if (hidden)
		 gtk_widget_modify_text(gtk_bin_get_child(GTK_BIN(item)),GTK_STATE_NORMAL,&red);
	else
		 gtk_widget_modify_text(gtk_bin_get_child(GTK_BIN(item)),GTK_STATE_NORMAL,&black);
	EXIT();
	return TRUE;
}

