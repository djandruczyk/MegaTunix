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
#include <curve.h>
#include <defines.h>
#include <enums.h>
#include <fileio.h>
#include <firmware.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <keyparser.h>
#include <stdlib.h>
#include <tabloader.h>
#include <widgetmgmt.h>


extern GObject *global_data;
extern Firmware_Details *firmware;

EXPORT gboolean create_2d_table_editor_group(GtkWidget *button)
{
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	GtkWidget *widget = NULL;
	GtkWidget *window = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *parent = NULL;
	GtkWidget *curve = NULL;
	GtkWidget *x_parent = NULL;
	GtkWidget *y_parent = NULL;
	GtkWidget *x_table = NULL;
	GtkWidget *y_table = NULL;
	GtkWidget *label = NULL;
	GtkWidget *entry = NULL;
	GArray *x_entries = NULL;
	GArray *y_entries = NULL;
	GList *widget_list = NULL;
	GList *curve_list = NULL;
	gchar * tmpbuf = NULL;
	gchar **vector = NULL;
	extern GList ***ve_widgets;
	gint num_tabs = 0;
	gint x_mult = 0;
	gint y_mult = 0;
	gint page = 0;
	gint offset = 0;
	gint i = 0;
	gint j = 0;
	gint rows = 0;
	gint table_num = 0;

	main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	if (!main_xml)
		return FALSE;

	xml = glade_xml_new(main_xml->filename,"table_editor_window",NULL);
	window = glade_xml_get_widget(xml,"table_editor_window");

	glade_xml_signal_autoconnect(xml);
	
	g_signal_connect(G_OBJECT(window),"destroy_event",
			G_CALLBACK(close_2d_editor),window);
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(close_2d_editor),window);
	tmpbuf = g_strdup_printf("2D Table Group Editor");
	gtk_window_set_title(GTK_WINDOW(window),tmpbuf);
	g_free(tmpbuf);
	gtk_window_resize(GTK_WINDOW(window),640,400);

	widget = glade_xml_get_widget(xml,"curve_editor_menuitem");
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);

	widget = glade_xml_get_widget(xml,"close_menuitem");
	OBJ_SET(widget,"window",(gpointer)window);

	widget = glade_xml_get_widget(xml,"te_layout_hbox1");
	gtk_widget_destroy(widget);

	widget = glade_xml_get_widget(xml,"te_layout_vbox");

	tmpbuf = OBJ_GET(button,"te_tables");
	vector = parse_keys(tmpbuf,&num_tabs,",");
	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(widget),notebook,TRUE,TRUE,0);
	for (j = 0;j < num_tabs;j++)
	{
		xml = glade_xml_new(main_xml->filename,"te_layout_hbox1",NULL);
		widget = glade_xml_get_widget(xml,"te_layout_hbox1");
		table_num = (gint)strtod(vector[j],NULL);
		label = gtk_label_new(firmware->te_params[table_num]->title);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),widget,label);
		parent = glade_xml_get_widget(xml,"te_right_frame");
		curve = mtx_curve_new();
		curve_list = g_list_prepend(curve_list,(gpointer)curve);
		gtk_container_add(GTK_CONTAINER(parent),curve);
		gtk_widget_realize(curve);
		mtx_curve_set_title(MTX_CURVE(curve),firmware->te_params[table_num]->title);
		mtx_curve_set_auto_hide_vertexes(MTX_CURVE(curve),TRUE);
		g_signal_connect(G_OBJECT(curve),"coords-changed",
				G_CALLBACK(coords_changed), NULL);

		label = glade_xml_get_widget(xml,"x_units");
		gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->x_units);
		label = glade_xml_get_widget(xml,"y_units");
		gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->y_units);
		label = glade_xml_get_widget(xml,"x_title");
		gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->x_name);
		label = glade_xml_get_widget(xml,"y_title");
		gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->y_name);
		rows = firmware->te_params[table_num]->bincount;
		mtx_curve_set_empty_array(MTX_CURVE(curve),rows);
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
		x_entries = g_array_new(FALSE,TRUE,sizeof(GtkWidget *));
		y_entries = g_array_new(FALSE,TRUE,sizeof(GtkWidget *));
		for (i=0;i<rows;i++)
		{
			/* X Column */
			entry = gtk_entry_new();
			gtk_entry_set_width_chars(GTK_ENTRY(entry),6);
			OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
			g_array_insert_val(x_entries,i,entry);
			OBJ_SET(entry,"curve_axis",GINT_TO_POINTER(_X_));
			OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET(entry,"raw_lower",GINT_TO_POINTER(firmware->te_params[table_num]->x_raw_lower));
			OBJ_SET(entry,"raw_upper",GINT_TO_POINTER(firmware->te_params[table_num]->x_raw_upper));
			OBJ_SET(entry,"dl_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->x_dl_conv_expr));
			OBJ_SET(entry,"ul_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->x_ul_conv_expr));
			OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->x_precision));
			OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->x_size));
			OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->x_page));
			OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->x_temp_dep));
			if(firmware->te_params[table_num]->x_temp_dep)
			{
				OBJ_SET(entry,"widget_temp",OBJ_GET(global_data,"temp_units"));
				bind_to_lists(entry,"temperature");
			}

			offset = (i*x_mult) + firmware->te_params[table_num]->x_base;
			OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(update_2d_curve),curve);
			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(entry_changed_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"key_press_event",
					G_CALLBACK(key_event),NULL);
			g_signal_connect(G_OBJECT(entry),"key_release_event",
					G_CALLBACK(key_event),NULL);
			g_signal_connect(G_OBJECT(entry),"focus_out_event",
					G_CALLBACK(focus_out_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"activate",
					G_CALLBACK(std_entry_handler),NULL);

			gtk_table_attach(GTK_TABLE(x_table),entry,
					0,1,i,i+1, GTK_SHRINK,GTK_SHRINK,0,0);
			page = firmware->te_params[table_num]->x_page;
			ve_widgets[page][offset] = g_list_prepend(ve_widgets[page][offset],(gpointer)entry);
			widget_list = g_list_prepend(widget_list,(gpointer)entry);

			update_widget(G_OBJECT(entry),NULL);


			/* Y Column */
			entry = gtk_entry_new();
			gtk_entry_set_width_chars(GTK_ENTRY(entry),6);
			OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
			g_array_insert_val(y_entries,i,entry);
			OBJ_SET(entry,"curve_axis",GINT_TO_POINTER(_Y_));
			OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET(entry,"raw_lower",GINT_TO_POINTER(firmware->te_params[table_num]->y_raw_lower));
			OBJ_SET(entry,"raw_upper",GINT_TO_POINTER(firmware->te_params[table_num]->y_raw_upper));
			OBJ_SET(entry,"dl_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->y_dl_conv_expr));
			OBJ_SET(entry,"ul_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->y_ul_conv_expr));
			OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->y_precision));
			OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->y_size));
			OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->y_page));
			OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->y_temp_dep));
			if(firmware->te_params[table_num]->y_temp_dep)
			{
				OBJ_SET(entry,"widget_temp",OBJ_GET(global_data,"temp_units"));
				bind_to_lists(entry,"temperature");
			}
			offset = (i*y_mult) + firmware->te_params[table_num]->y_base;
			OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(update_2d_curve),curve);
			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(entry_changed_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"key_press_event",
					G_CALLBACK(key_event),NULL);
			g_signal_connect(G_OBJECT(entry),"key_release_event",
					G_CALLBACK(key_event),NULL);
			g_signal_connect(G_OBJECT(entry),"focus_out_event",
					G_CALLBACK(focus_out_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"activate",
					G_CALLBACK(std_entry_handler),NULL);

			gtk_table_attach(GTK_TABLE(y_table),entry,
					0,1,i,i+1, GTK_SHRINK,GTK_SHRINK,0,0);
			page = firmware->te_params[table_num]->y_page;
			ve_widgets[page][offset] = g_list_prepend(ve_widgets[page][offset],(gpointer)entry);
			widget_list = g_list_prepend(widget_list,(gpointer)entry);

			update_widget(G_OBJECT(entry),NULL);
		}
		mtx_curve_set_x_precision(MTX_CURVE(curve),firmware->te_params[table_num]->x_precision);
		mtx_curve_set_y_precision(MTX_CURVE(curve),firmware->te_params[table_num]->y_precision);
		OBJ_SET(curve,"x_entries",x_entries);
		OBJ_SET(curve,"y_entries",y_entries);
	}
	OBJ_SET(window,"widget_list",widget_list);
	OBJ_SET(window,"curve_list",curve_list);
	gtk_widget_show_all(window);
	return TRUE;

}


EXPORT gboolean create_2d_table_editor(gint table_num)
{
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	GtkWidget *widget = NULL;
	GtkWidget *window = NULL;
	GtkWidget *parent = NULL;
	GtkWidget *curve = NULL;
	GtkWidget *x_parent = NULL;
	GtkWidget *y_parent = NULL;
	GtkWidget *x_table = NULL;
	GtkWidget *y_table = NULL;
	GtkWidget *label = NULL;
	GtkWidget *entry = NULL;
	GArray *x_entries = NULL;
	GArray *y_entries = NULL;
	GList *widget_list = NULL;
	gchar * tmpbuf = NULL;
	extern GList ***ve_widgets;
	gint x_mult = 0;
	gint y_mult = 0;
	gint page = 0;
	gint offset = 0;
	gint i = 0;
	gint rows = 0;

	main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	if (!main_xml)
		return FALSE;

	xml = glade_xml_new(main_xml->filename,"table_editor_window",NULL);
	window = glade_xml_get_widget(xml,"table_editor_window");

	glade_xml_signal_autoconnect(xml);
	
	g_signal_connect(G_OBJECT(window),"destroy_event",
			G_CALLBACK(close_2d_editor),window);
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(close_2d_editor),window);
	tmpbuf = g_strdup_printf("2D Table Editor (%s)",firmware->te_params[table_num]->title);
	gtk_window_set_title(GTK_WINDOW(window),tmpbuf);
	g_free(tmpbuf);
	gtk_window_set_default_size(GTK_WINDOW(window),500,400);

	widget = glade_xml_get_widget(xml,"curve_editor_menuitem");
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);

	widget = glade_xml_get_widget(xml,"close_menuitem");
	OBJ_SET(widget,"window",(gpointer)window);

	parent = glade_xml_get_widget(xml,"te_right_frame");
	curve = mtx_curve_new();
	gtk_container_add(GTK_CONTAINER(parent),curve);
	mtx_curve_set_title(MTX_CURVE(curve),firmware->te_params[table_num]->title);
	mtx_curve_set_auto_hide_vertexes(MTX_CURVE(curve),TRUE);
	g_signal_connect(G_OBJECT(curve),"coords-changed",
			G_CALLBACK(coords_changed), NULL);

	label = glade_xml_get_widget(xml,"x_units");
	gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->x_units);
	label = glade_xml_get_widget(xml,"y_units");
	gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->y_units);
	label = glade_xml_get_widget(xml,"x_title");
	gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->x_name);
	label = glade_xml_get_widget(xml,"y_title");
	gtk_label_set_text(GTK_LABEL(label),firmware->te_params[table_num]->y_name);
	rows = firmware->te_params[table_num]->bincount;
	mtx_curve_set_empty_array(MTX_CURVE(curve),rows);
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
	x_entries = g_array_new(FALSE,TRUE,sizeof(GtkWidget *));
	y_entries = g_array_new(FALSE,TRUE,sizeof(GtkWidget *));
	for (i=0;i<rows;i++)
	{
		/* X Column */
		entry = gtk_entry_new();
		gtk_entry_set_width_chars(GTK_ENTRY(entry),6);
		OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
		g_array_insert_val(x_entries,i,entry);
		OBJ_SET(entry,"curve_axis",GINT_TO_POINTER(_X_));
		OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(entry,"raw_lower",GINT_TO_POINTER(firmware->te_params[table_num]->x_raw_lower));
		OBJ_SET(entry,"raw_upper",GINT_TO_POINTER(firmware->te_params[table_num]->x_raw_upper));
		OBJ_SET(entry,"dl_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->x_dl_conv_expr));
		OBJ_SET(entry,"ul_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->x_ul_conv_expr));
		OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->x_precision));
		OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->x_size));
		OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->x_page));
		OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->x_temp_dep));
		if(firmware->te_params[table_num]->x_temp_dep)
		{
			OBJ_SET(entry,"widget_temp",OBJ_GET(global_data,"temp_units"));
			bind_to_lists(entry,"temperature");
		}

		offset = (i*x_mult) + firmware->te_params[table_num]->x_base;
		OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

		g_signal_connect(G_OBJECT(entry),"changed",
				G_CALLBACK(update_2d_curve),curve);
		g_signal_connect(G_OBJECT(entry),"changed",
				G_CALLBACK(entry_changed_handler),NULL);
		g_signal_connect(G_OBJECT(entry),"key_press_event",
				G_CALLBACK(key_event),NULL);
		g_signal_connect(G_OBJECT(entry),"key_release_event",
				G_CALLBACK(key_event),NULL);
		g_signal_connect(G_OBJECT(entry),"focus_out_event",
				G_CALLBACK(focus_out_handler),NULL);
		g_signal_connect(G_OBJECT(entry),"activate",
				G_CALLBACK(std_entry_handler),NULL);

		gtk_table_attach(GTK_TABLE(x_table),entry,
				0,1,i,i+1, GTK_SHRINK,GTK_SHRINK,0,0);
		page = firmware->te_params[table_num]->x_page;
		ve_widgets[page][offset] = g_list_prepend(ve_widgets[page][offset],(gpointer)entry);
		widget_list = g_list_prepend(widget_list,(gpointer)entry);

		update_widget(G_OBJECT(entry),NULL);


		/* Y Column */
		entry = gtk_entry_new();
		gtk_entry_set_width_chars(GTK_ENTRY(entry),6);
		OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
		g_array_insert_val(y_entries,i,entry);
		OBJ_SET(entry,"curve_axis",GINT_TO_POINTER(_Y_));
		OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(entry,"raw_lower",GINT_TO_POINTER(firmware->te_params[table_num]->y_raw_lower));
		OBJ_SET(entry,"raw_upper",GINT_TO_POINTER(firmware->te_params[table_num]->y_raw_upper));
		OBJ_SET(entry,"dl_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->y_dl_conv_expr));
		OBJ_SET(entry,"ul_conv_expr",GINT_TO_POINTER(firmware->te_params[table_num]->y_ul_conv_expr));
		OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->y_precision));
		OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->y_size));
		OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->y_page));
		OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->y_temp_dep));
		if(firmware->te_params[table_num]->y_temp_dep)
		{
			OBJ_SET(entry,"widget_temp",OBJ_GET(global_data,"temp_units"));
			bind_to_lists(entry,"temperature");
		}
		offset = (i*y_mult) + firmware->te_params[table_num]->y_base;
		OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

		g_signal_connect(G_OBJECT(entry),"changed",
				G_CALLBACK(update_2d_curve),curve);
		g_signal_connect(G_OBJECT(entry),"changed",
				G_CALLBACK(entry_changed_handler),NULL);
		g_signal_connect(G_OBJECT(entry),"key_press_event",
				G_CALLBACK(key_event),NULL);
		g_signal_connect(G_OBJECT(entry),"key_release_event",
				G_CALLBACK(key_event),NULL);
		g_signal_connect(G_OBJECT(entry),"focus_out_event",
				G_CALLBACK(focus_out_handler),NULL);
		g_signal_connect(G_OBJECT(entry),"activate",
				G_CALLBACK(std_entry_handler),NULL);

		gtk_table_attach(GTK_TABLE(y_table),entry,
				0,1,i,i+1, GTK_SHRINK,GTK_SHRINK,0,0);
		page = firmware->te_params[table_num]->y_page;
		ve_widgets[page][offset] = g_list_prepend(ve_widgets[page][offset],(gpointer)entry);
		widget_list = g_list_prepend(widget_list,(gpointer)entry);

		update_widget(G_OBJECT(entry),NULL);
	}
	mtx_curve_set_x_precision(MTX_CURVE(curve),firmware->te_params[table_num]->x_precision);
	mtx_curve_set_y_precision(MTX_CURVE(curve),firmware->te_params[table_num]->y_precision);
	OBJ_SET(window,"widget_list",widget_list);
	OBJ_SET(curve,"x_entries",x_entries);
	OBJ_SET(curve,"y_entries",y_entries);
	OBJ_SET(window,"x_entries",x_entries);
	OBJ_SET(window,"y_entries",y_entries);

	gtk_widget_show_all(window);
	return TRUE;
}


gboolean close_2d_editor(GtkWidget * widget, gpointer data)
{
	GArray *array = NULL;
	GList *list = NULL;
	
	list = OBJ_GET(widget, "widget_list");
	if (list)
	{
		g_list_foreach(list,remove_widget,(gpointer)list);
		g_list_free(list);
	}
	list = OBJ_GET(widget, "curve_list");
	if (list)
	{
		g_list_foreach(list,clean_curve,NULL);
		g_list_free(list);
	}
	array = OBJ_GET(widget, "x_entries");
	if (array)
		g_array_free(array,TRUE);
	array = OBJ_GET(widget, "y_entries");
	if (array)
		g_array_free(array,TRUE);
	gtk_widget_destroy(widget);
	return FALSE;
}


void remove_widget(gpointer widget_ptr, gpointer data)
{
	extern GList ***ve_widgets;
	gint page = 0;
	gint offset = 0;
	remove_from_list("temperature",widget_ptr);
	page = (gint)OBJ_GET(widget_ptr,"page");
	offset = (gint)OBJ_GET(widget_ptr,"offset");
	ve_widgets[page][offset] = g_list_remove(ve_widgets[page][offset],widget_ptr);
}


void clean_curve(gpointer curve_ptr, gpointer data)
{
	GArray *array = NULL;
	GtkWidget *widget = (GtkWidget *)curve_ptr;

	array = OBJ_GET(widget, "x_entries");
	if (array)
		g_array_free(array,TRUE);
	array = OBJ_GET(widget, "y_entries");
	if (array)
		g_array_free(array,TRUE);
}

gboolean update_2d_curve(GtkWidget *widget, gpointer data)
{
	GtkWidget *curve = (GtkWidget *)data;
	MtxCurveCoord point;
	Axis axis;
	gint index = 0;
	gchar * text = NULL;
	gfloat tmpf = 0.0;
	
	index = (gint) OBJ_GET(widget,"curve_index");
	axis = (Axis) OBJ_GET(widget,"curve_axis");
	mtx_curve_get_coords_at_index(MTX_CURVE(curve),index,&point);
	text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
        tmpf = (gfloat)strtod(g_strdelimit(text,",.",'.'),NULL);
	if (axis == _X_)
		point.x = tmpf;
	else if (axis == _Y_)
		point.y = tmpf;
	else
		printf("ERROR in update_2d_curve()!!!\n");
	mtx_curve_set_coords_at_index(MTX_CURVE(curve),index,point);
	return FALSE;
	
}


void coords_changed(GtkWidget *curve, gpointer data)
{
	MtxCurveCoord point;
	gint index = 0;
	GArray *array;
	gint precision = 0;
	GtkWidget *entry = NULL;
	gchar * tmpbuf = NULL;
	
	index = mtx_curve_get_active_coord_index(MTX_CURVE(curve));
	mtx_curve_get_coords_at_index(MTX_CURVE(curve),index,&point);
	/* X Coord */
	array = OBJ_GET(curve,"x_entries");
	entry = g_array_index(array,GtkWidget *,index);
	precision = (gint)OBJ_GET(entry, "precision");
	tmpbuf = g_strdup_printf("%1$.*2$f",point.x,precision);
	gtk_entry_set_text(GTK_ENTRY(entry),tmpbuf);
	g_signal_emit_by_name(entry, "activate");
	g_free(tmpbuf);

	/* Y Coord */
	array = OBJ_GET(curve,"y_entries");
	entry  = g_array_index(array,GtkWidget *,index);
	precision = (gint)OBJ_GET(entry, "precision");
	tmpbuf = g_strdup_printf("%1$.*2$f",point.y,precision);
	gtk_entry_set_text(GTK_ENTRY(entry),tmpbuf);
	g_signal_emit_by_name(entry, "activate");
	g_free(tmpbuf);
}


EXPORT gboolean close_menu_handler(GtkWidget * widget, gpointer data)
{
	close_2d_editor(OBJ_GET(widget,"window"),NULL);
	return TRUE;
}
