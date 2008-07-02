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

#include <2d_table_editor.h>
#include <3d_vetable.h>
#include <config.h>
#include <defines.h>
#include <enums.h>
#include <fileio.h>
#include <firmware.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <stdlib.h>


extern GtkWidget **te_windows;
extern GObject *global_data;
extern Firmware_Details *firmware;

EXPORT gboolean create_2d_table_editor(GtkWidget *widget,gpointer data)
{
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	GtkWidget *window = NULL;
	GtkWidget *x_parent = NULL;
	GtkWidget *y_parent = NULL;
	GtkWidget *x_table = NULL;
	GtkWidget *y_table = NULL;
	GtkWidget *label = NULL;
	GtkWidget *entry = NULL;
	gchar * tmpbuf = NULL;
	extern GList ***ve_widgets;
	gint x_mult = 0;
	gint y_mult = 0;
	gint page = 0;
	gint offset = 0;
	gint i = 0;
	gint rows = 0;
	gint table_num = 0;

	table_num = (gint)g_ascii_strtod(OBJ_GET(widget,"table_num"),NULL);
	if (!(GTK_IS_WIDGET(te_windows[table_num])))
	{
		main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
		if (!main_xml)
			return FALSE;

		xml = glade_xml_new(main_xml->filename,"table_editor_window",NULL);
		window = glade_xml_get_widget(xml,"table_editor_window");
		tmpbuf = g_strdup_printf("2D Table Editor (%s)",firmware->te_params[table_num]->title);
		gtk_window_set_title(GTK_WINDOW(window),tmpbuf);
		g_free(tmpbuf);
		gtk_window_set_default_size(GTK_WINDOW(window),500,400);
		//g_signal_connect(G_OBJECT(window),"delete_event",
		//		G_CALLBACK(close_2d_editor),window);

		label = glade_xml_get_widget(xml,"x_units");
		gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->x_units);
		label = glade_xml_get_widget(xml,"y_units");
		gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->y_units);
		label = glade_xml_get_widget(xml,"x_title");
		gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->x_name);
		label = glade_xml_get_widget(xml,"y_title");
		gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->y_name);
		rows = firmware->te_params[table_num]->bincount;
		x_table = gtk_table_new(rows,1,TRUE);
		y_table = gtk_table_new(rows,1,TRUE);

		x_parent = glade_xml_get_widget(xml,"te_x_frame");
		y_parent = glade_xml_get_widget(xml,"te_y_frame");
		gtk_container_set_border_width(GTK_CONTAINER(x_table),5);
		gtk_container_set_border_width(GTK_CONTAINER(y_table),5);
		gtk_container_add(GTK_CONTAINER(x_parent),x_table);
		gtk_container_add(GTK_CONTAINER(y_parent),y_table);

		
		x_mult = get_multiplier(firmware->te_params[table_num]->x_size);
		y_mult = get_multiplier(firmware->te_params[table_num]->y_size);
		for (i=0;i<rows;i++)
		{
			/* X Column */
			entry = gtk_entry_new();
			OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET(entry,"raw_lower",GINT_TO_POINTER(firmware->te_params[table_num]->x_raw_lower));
			OBJ_SET(entry,"raw_upper",GINT_TO_POINTER(firmware->te_params[table_num]->x_raw_upper));
			OBJ_SET(entry,"dl_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->x_dl_conv_expr));
			OBJ_SET(entry,"ul_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->x_ul_conv_expr));
			OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->x_precision));
			OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->x_size));
			OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->x_page));
			offset = (i*x_mult) + firmware->te_params[table_num]->x_base;
			OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(entry_changed_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"activate",
					G_CALLBACK(std_entry_handler),NULL);
					
			gtk_table_attach_defaults(GTK_TABLE(x_table),entry,
					0,1,i,i+1);
			page = firmware->te_params[table_num]->x_page;
			ve_widgets[page][offset] = g_list_prepend(ve_widgets[page][offset],(gpointer)entry);
			firmware->te_params[table_num]->entries = g_list_prepend(firmware->te_params[table_num]->entries,(gpointer)entry);

		//	update_widget(G_OBJECT(entry),NULL);


			/* Y Column */
			entry = gtk_entry_new();
			OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET(entry,"raw_lower",GINT_TO_POINTER(firmware->te_params[table_num]->y_raw_lower));
			OBJ_SET(entry,"raw_upper",GINT_TO_POINTER(firmware->te_params[table_num]->y_raw_upper));
			OBJ_SET(entry,"dl_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->y_dl_conv_expr));
			OBJ_SET(entry,"ul_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->y_ul_conv_expr));
			OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->y_precision));
			OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->y_size));
			OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->y_page));
			offset = (i*y_mult) + firmware->te_params[table_num]->y_base;
			OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(entry_changed_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"activate",
					G_CALLBACK(std_entry_handler),NULL);
					
			gtk_table_attach_defaults(GTK_TABLE(y_table),entry,
					0,1,i,i+1);
			page = firmware->te_params[table_num]->y_page;
			ve_widgets[page][offset] = g_list_prepend(ve_widgets[page][offset],(gpointer)entry);
			firmware->te_params[table_num]->entries = g_list_prepend(firmware->te_params[table_num]->entries,(gpointer)entry);

		//	update_widget(G_OBJECT(entry),NULL);
		}
	
	}

	gtk_widget_show_all(window);
	return TRUE;
}


gboolean close_2d_editor(GtkWidget * widget, gpointer data);
