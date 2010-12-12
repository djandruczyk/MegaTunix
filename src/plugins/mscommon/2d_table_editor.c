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
#include <config.h>
#include <curve.h>
#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <gauge.h>
#include <getfiles.h>
#include <glade/glade.h>
#include <mscommon_gui_handlers.h>
#include <stdlib.h>
#include <mtxmatheval.h>

extern gconstpointer *global_data;

typedef struct
{
	GtkWidget *curve;
	Axis axis;
	gchar * source;
}CurveData;

G_MODULE_EXPORT gboolean create_2d_table_editor_group(GtkWidget *button)
{
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	GtkWidget *widget = NULL;
	GtkWidget *window = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *curve = NULL;
	GtkWidget *x_parent = NULL;
	GtkWidget *y_parent = NULL;
	GtkWidget *x_table = NULL;
	GtkWidget *y_table = NULL;
	GtkWidget *label = NULL;
	GtkWidget *entry = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *gauge = NULL;
	GtkWidget *parent = NULL;
	GtkWidget *curve_parent = NULL;
	CurveData *cdata = NULL;
	GArray *x_entries = NULL;
	GArray *y_entries = NULL;
	GList *widget_list = NULL;
	GList *curve_list = NULL;
	GList *gauge_list = NULL;
	gchar * tmpbuf = NULL;
	gchar * filename = NULL;
	gchar **vector = NULL;
	GList ***ve_widgets = NULL;
	gint num_tabs = 0;
	gint x_mult = 0;
	gint y_mult = 0;
	gint page = 0;
	gint offset = 0;
	gint i = 0;
	gint j = 0;
	gfloat tmpf = 0.0;
	guint32 id = 0;
	gint rows = 0;
	gint table_num = 0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	ve_widgets = DATA_GET(global_data,"ve_widgets");

	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if (!main_xml)
		return FALSE;

	xml = glade_xml_new(main_xml->filename,"table_editor_window",NULL);
	window = glade_xml_get_widget(xml,"table_editor_window");

	glade_xml_signal_autoconnect(xml);

	g_signal_connect(G_OBJECT(window),"destroy_event",
			G_CALLBACK(close_2d_editor),window);
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(close_2d_editor),window);
	gtk_window_set_title(GTK_WINDOW(window),_("2D Table Group Editor"));
	gtk_window_resize(GTK_WINDOW(window),800,530);

	widget = glade_xml_get_widget(xml,"2d_close_button");
	g_signal_connect_swapped(G_OBJECT(widget),"clicked",
			G_CALLBACK(close_2d_editor),window);

	widget = glade_xml_get_widget(xml,"get_data_button");
	OBJ_SET(widget,"handler",GINT_TO_POINTER(READ_VE_CONST));
	OBJ_SET(widget,"bind_to_list",g_strdup("get_data_buttons"));
	bind_to_lists_f(widget,"get_data_buttons");
	widget_list = g_list_prepend(widget_list,(gpointer)widget);

	widget = glade_xml_get_widget(xml,"burn_data_button");
	OBJ_SET(widget,"handler",GINT_TO_POINTER(BURN_MS_FLASH));
	OBJ_SET(widget,"bind_to_list",g_strdup("burners"));
	bind_to_lists_f(widget,"burners");
	widget_list = g_list_prepend(widget_list,(gpointer)widget);

	widget = glade_xml_get_widget(xml,"curve_editor_menuitem");
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);

	widget = glade_xml_get_widget(xml,"close_menuitem");
	OBJ_SET(widget,"window",(gpointer)window);

	widget = glade_xml_get_widget(xml,"te_layout_hbox1");
	gtk_widget_destroy(widget);

	widget = glade_xml_get_widget(xml,"te_layout_vbox");

	tmpbuf = OBJ_GET(button,"te_tables");
	vector = parse_keys_f(tmpbuf,&num_tabs,",");
	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(widget),notebook,TRUE,TRUE,0);
	for (j = 0;j < num_tabs;j++)
	{
		table_num = (gint)strtod(vector[j],NULL);
		if (table_num >= firmware->total_te_tables)
		{
			warn_user_f("Requested to create 2D table editor window for an undefined (out of range) table ID");
			return FALSE;
		}
		if (!firmware->te_params)
		{
			warn_user_f("No 2D table Editor tables (te_tables) defined in interrogation profile, yet told to create a graph for a table... BUG detected!");
			continue;
		}
		if (!firmware->te_params[table_num])
		{
			warn_user_f("Requested to create a 2D table editor window for an undefined table!");
			continue;
		}
		xml = glade_xml_new(main_xml->filename,"te_layout_hbox1",NULL);
		widget = glade_xml_get_widget(xml,"te_layout_hbox1");
		label = gtk_label_new(firmware->te_params[table_num]->title);
		gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
		if (firmware->te_params[table_num]->bind_to_list)
		{
			OBJ_SET(widget,"bind_to_list", g_strdup(firmware->te_params[table_num]->bind_to_list));
			OBJ_SET(widget,"match_type", GINT_TO_POINTER(firmware->te_params[table_num]->match_type));
			bind_to_lists_f(widget,firmware->te_params[table_num]->bind_to_list);
			widget_list = g_list_prepend(widget_list,(gpointer)widget);
			OBJ_SET(label,"bind_to_list", g_strdup(firmware->te_params[table_num]->bind_to_list));
			OBJ_SET(label,"match_type", GINT_TO_POINTER(firmware->te_params[table_num]->match_type));
			bind_to_lists_f(label,firmware->te_params[table_num]->bind_to_list);
			widget_list = g_list_prepend(widget_list,(gpointer)label);
		}

		if (firmware->te_params[table_num]->gauge ||
				firmware->te_params[table_num]->c_gauge ||
				firmware->te_params[table_num]->f_gauge)
		{
			parent = glade_xml_get_widget(xml,"te_gaugeframe");
			gauge = mtx_gauge_face_new();
			gauge_list = g_list_prepend(gauge_list,(gpointer)gauge);

			OBJ_SET(window,"gauge",gauge);
			if (firmware->te_params[table_num]->gauge_temp_dep)
			{
				if ((GINT)DATA_GET(global_data,"temp_units") == CELSIUS)
					tmpbuf = g_strdelimit(firmware->te_params[table_num]->c_gauge,"\\",'/');
				else
					tmpbuf = g_strdelimit(firmware->te_params[table_num]->f_gauge,"\\",'/');
			}
			else
				tmpbuf = g_strdelimit(firmware->te_params[table_num]->gauge,"\\",'/');
			filename = get_file(g_strconcat(GAUGES_DATA_DIR,PSEP,tmpbuf,NULL),NULL);
			mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
			lookup_current_value_f(firmware->te_params[table_num]->gauge_datasource, &tmpf);
			mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),tmpf);
			g_free(filename);
			id = create_value_change_watch_f(firmware->te_params[table_num]->gauge_datasource,FALSE,"update_misc_gauge",(gpointer)gauge);
			OBJ_SET(gauge,"gauge_id",GINT_TO_POINTER(id));
			gtk_container_add(GTK_CONTAINER(parent),gauge);
		}
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),widget,label);
		curve_parent = glade_xml_get_widget(xml,"te_right_frame");
		curve = mtx_curve_new();
		curve_list = g_list_prepend(curve_list,(gpointer)curve);
		mtx_curve_set_title(MTX_CURVE(curve),_(firmware->te_params[table_num]->title));
		mtx_curve_set_x_axis_label(MTX_CURVE(curve),_(firmware->te_params[table_num]->x_axis_label));
		mtx_curve_set_y_axis_label(MTX_CURVE(curve),_(firmware->te_params[table_num]->y_axis_label));
		cdata = g_new0(CurveData, 1);
		cdata->curve = curve;
		cdata->axis = _X_;
		cdata->source = firmware->te_params[table_num]->x_source;
		id = create_value_change_watch_f(cdata->source,FALSE,"update_curve_marker",(gpointer)cdata);
		mtx_curve_set_show_x_marker(MTX_CURVE(curve),TRUE);
		OBJ_SET(curve,"cdata",(gpointer)cdata);
		OBJ_SET(curve,"marker_id",GINT_TO_POINTER(id));
		mtx_curve_set_auto_hide_vertexes(MTX_CURVE(curve),TRUE);
		g_signal_connect(G_OBJECT(curve),"coords-changed",
				G_CALLBACK(coords_changed), NULL);
		g_signal_connect(G_OBJECT(curve),"vertex-proximity",
				G_CALLBACK(vertex_proximity), NULL);
		g_signal_connect(G_OBJECT(curve),"marker-proximity",
				G_CALLBACK(marker_proximity), NULL);

		label = glade_xml_get_widget(xml,"x_units");
		gtk_label_set_markup(GTK_LABEL(label),firmware->te_params[table_num]->x_units);
		label = glade_xml_get_widget(xml,"y_units");
		gtk_label_set_markup(GTK_LABEL(label),firmware->te_params[table_num]->y_units);
		label = glade_xml_get_widget(xml,"x_title");
		gtk_label_set_markup(GTK_LABEL(label),firmware->te_params[table_num]->x_name);
		label = glade_xml_get_widget(xml,"y_title");
		gtk_label_set_markup(GTK_LABEL(label),firmware->te_params[table_num]->y_name);
		rows = firmware->te_params[table_num]->bincount;
		mtx_curve_set_empty_array(MTX_CURVE(curve),rows);
		x_table = gtk_table_new(rows+1,1,FALSE);
		y_table = gtk_table_new(rows+1,1,FALSE);

		x_parent = glade_xml_get_widget(xml,"te_x_frame");
		y_parent = glade_xml_get_widget(xml,"te_y_frame");
		gtk_container_set_border_width(GTK_CONTAINER(x_table),5);
		gtk_container_set_border_width(GTK_CONTAINER(y_table),5);
		gtk_container_add(GTK_CONTAINER(x_parent),x_table);
		gtk_container_add(GTK_CONTAINER(y_parent),y_table);

		x_mult = get_multiplier_f(firmware->te_params[table_num]->x_size);
		y_mult = get_multiplier_f(firmware->te_params[table_num]->y_size);
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
			OBJ_SET(entry,"raw_lower",g_strdup_printf("%i",(firmware->te_params[table_num]->x_raw_lower)));
			OBJ_SET(entry,"raw_upper",g_strdup_printf("%i",(firmware->te_params[table_num]->x_raw_upper)));
			OBJ_SET(entry,"dl_conv_expr",g_strdup(firmware->te_params[table_num]->x_dl_conv_expr));
			OBJ_SET(entry,"ul_conv_expr",g_strdup(firmware->te_params[table_num]->x_ul_conv_expr));
			OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->x_precision));
			OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->x_size));
			OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->x_page));
			OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->x_temp_dep));
			OBJ_SET(entry,"use_color",GINT_TO_POINTER(firmware->te_params[table_num]->x_use_color));
			OBJ_SET(entry,"force_color_update",GINT_TO_POINTER(firmware->te_params[table_num]->x_use_color));
			if(firmware->te_params[table_num]->x_temp_dep)
			{
				OBJ_SET(entry,"widget_temp",DATA_GET(global_data,"temp_units"));
				OBJ_SET(entry,"bind_to_list", g_strdup("temperature"));
				bind_to_lists_f(entry,"temperature");
			}

			offset = (i*x_mult) + firmware->te_params[table_num]->x_base;
			OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(update_2d_curve),curve);
			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(entry_changed_handler_f),NULL);
			g_signal_connect(G_OBJECT(entry),"key_press_event",
					G_CALLBACK(key_event_f),NULL);
			g_signal_connect(G_OBJECT(entry),"key_release_event",
					G_CALLBACK(key_event_f),NULL);
			g_signal_connect(G_OBJECT(entry),"focus_out_event",
					G_CALLBACK(focus_out_handler_f),NULL);
			g_signal_connect(G_OBJECT(entry),"activate",
					G_CALLBACK(std_entry_handler_f),NULL);

			if (firmware->te_params[table_num]->reversed)
				gtk_table_attach(GTK_TABLE(x_table),entry,
						0,1,rows-i-1,rows-i, GTK_SHRINK,GTK_SHRINK,0,0);
			else
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
			OBJ_SET(entry,"raw_lower",g_strdup_printf("%i",(firmware->te_params[table_num]->y_raw_lower)));
			OBJ_SET(entry,"raw_upper",g_strdup_printf("%i",(firmware->te_params[table_num]->y_raw_upper)));
			OBJ_SET(entry,"dl_conv_expr",firmware->te_params[table_num]->y_dl_conv_expr);
			OBJ_SET(entry,"ul_conv_expr",firmware->te_params[table_num]->y_ul_conv_expr);
			OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->y_precision));
			OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->y_size));
			OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->y_page));
			OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->y_temp_dep));
			OBJ_SET(entry,"use_color",GINT_TO_POINTER(firmware->te_params[table_num]->y_use_color));
			OBJ_SET(entry,"force_color_update",GINT_TO_POINTER(firmware->te_params[table_num]->y_use_color));
			if(firmware->te_params[table_num]->y_temp_dep)
			{
				OBJ_SET(entry,"widget_temp",DATA_GET(global_data,"temp_units"));
				OBJ_SET(entry,"bind_to_list", g_strdup("temperature"));
				bind_to_lists_f(entry,"temperature");
			}
			offset = (i*y_mult) + firmware->te_params[table_num]->y_base;
			OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(update_2d_curve),curve);
			g_signal_connect(G_OBJECT(entry),"changed",
					G_CALLBACK(entry_changed_handler_f),NULL);
			g_signal_connect(G_OBJECT(entry),"key_press_event",
					G_CALLBACK(key_event_f),NULL);
			g_signal_connect(G_OBJECT(entry),"key_release_event",
					G_CALLBACK(key_event_f),NULL);
			g_signal_connect(G_OBJECT(entry),"focus_out_event",
					G_CALLBACK(focus_out_handler_f),NULL);
			g_signal_connect(G_OBJECT(entry),"activate",
					G_CALLBACK(std_entry_handler_f),NULL);

			if (firmware->te_params[table_num]->reversed)
				gtk_table_attach(GTK_TABLE(y_table),entry,
						0,1,rows-i-1,rows-i, GTK_SHRINK,GTK_SHRINK,0,0);
			else
				gtk_table_attach(GTK_TABLE(y_table),entry,
						0,1,i,i+1, GTK_SHRINK,GTK_SHRINK,0,0);
			page = firmware->te_params[table_num]->y_page;
			ve_widgets[page][offset] = g_list_prepend(ve_widgets[page][offset],(gpointer)entry);
			widget_list = g_list_prepend(widget_list,(gpointer)entry);
			update_widget(G_OBJECT(entry),NULL);
		}
		/* Create the "LOCK" buttons */
		dummy = gtk_toggle_button_new_with_label("Unlocked");
		OBJ_SET(dummy,"axis",GINT_TO_POINTER(_X_));
		g_signal_connect(G_OBJECT(dummy),"toggled",
				G_CALLBACK(set_axis_locking),curve);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dummy),firmware->te_params[table_num]->x_lock);
		gtk_table_attach(GTK_TABLE(x_table),dummy,
				0,1,i,i+1, GTK_EXPAND|GTK_FILL,0,0,0);
		dummy = gtk_toggle_button_new_with_label("Unlocked");
		OBJ_SET(dummy,"axis",GINT_TO_POINTER(_Y_));
		g_signal_connect(G_OBJECT(dummy),"toggled",
				G_CALLBACK(set_axis_locking),curve);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dummy),firmware->te_params[table_num]->y_lock);
		gtk_table_attach(GTK_TABLE(y_table),dummy,
				0,1,i,i+1, GTK_EXPAND|GTK_FILL,0,0,0);

		mtx_curve_set_x_precision(MTX_CURVE(curve),firmware->te_params[table_num]->x_precision);
		mtx_curve_set_y_precision(MTX_CURVE(curve),firmware->te_params[table_num]->y_precision);
		mtx_curve_set_hard_limits(MTX_CURVE(curve),
				(gfloat)firmware->te_params[table_num]->x_raw_lower,
				(gfloat)firmware->te_params[table_num]->x_raw_upper,
				(gfloat)firmware->te_params[table_num]->y_raw_lower,
				(gfloat)firmware->te_params[table_num]->y_raw_upper);
		OBJ_SET(curve,"x_entries",x_entries);
		OBJ_SET(curve,"y_entries",y_entries);
		if (firmware->te_params[table_num]->bind_to_list)
			g_list_foreach(get_list_f(firmware->te_params[table_num]->bind_to_list),alter_widget_state_f,NULL);
		create_value_change_watch_f(cdata->source,TRUE,"update_curve_marker",(gpointer)cdata);
		gtk_container_add(GTK_CONTAINER(curve_parent),curve);
	}
	OBJ_SET(window,"widget_list",widget_list);
	OBJ_SET(window,"curve_list",curve_list);
	OBJ_SET(window,"gauge_list",gauge_list);
	gtk_widget_show_all(window);
	return TRUE;

}


G_MODULE_EXPORT gboolean create_2d_table_editor(gint table_num, GtkWidget *parent)
{
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	GtkWidget *widget = NULL;
	GtkWidget *window = NULL;
	GtkWidget *curve = NULL;
	GtkWidget *x_parent = NULL;
	GtkWidget *y_parent = NULL;
	GtkWidget *x_table = NULL;
	GtkWidget *y_table = NULL;
	GtkWidget *label = NULL;
	GtkWidget *entry = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *gauge = NULL;
	GtkWidget *curve_parent = NULL;
	CurveData *cdata = NULL;
	GArray *x_entries = NULL;
	GArray *y_entries = NULL;
	GList *widget_list = NULL;
	GList *curve_list = NULL;
	GList *gauge_list = NULL;
	gchar * tmpbuf = NULL;
	gchar * filename = NULL;
	GList ***ve_widgets = NULL;
	gint x_mult = 0;
	gint y_mult = 0;
	gint page = 0;
	gint offset = 0;
	gint i = 0;
	guint32 id = 0;
	gfloat tmpf = 0.0;
	gint rows = 0;
	gboolean embedded = FALSE;
	void *evaluator = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	ve_widgets = DATA_GET(global_data,"ve_widgets");

	if (table_num >= firmware->total_te_tables)
	{
		warn_user_f("Requested to create 2D table editor window for an undefined (out of range) table ID");
		return FALSE;
	}
	if (!firmware->te_params)
	{
		warn_user_f("No 2D table Editor tables (te_tables) defined in interrogation profile, yet told to create a graph for a table... BUG detected!");
		return FALSE;
	}
	if (!firmware->te_params[table_num])
	{
		warn_user_f("Requested to create a 2D table editor window for an undefined table!");
		return FALSE;
	}
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if (!main_xml)
		return FALSE;

	if (GTK_IS_WIDGET(parent)) /* Embedded mode */
		embedded = TRUE;

	if (!embedded)
	{
		xml = glade_xml_new(main_xml->filename,"table_editor_window",NULL);
		window = glade_xml_get_widget(xml,"table_editor_window");
		if (firmware->te_params[table_num]->bind_to_list)
		{
			OBJ_SET(window,"bind_to_list", g_strdup(firmware->te_params[table_num]->bind_to_list));
			OBJ_SET(window,"match_type", GINT_TO_POINTER(firmware->te_params[table_num]->match_type));
			bind_to_lists_f(window,firmware->te_params[table_num]->bind_to_list);
			widget_list = g_list_prepend(widget_list,(gpointer)window);
		}
		glade_xml_signal_autoconnect(xml);

		g_signal_connect(G_OBJECT(window),"destroy_event",
				G_CALLBACK(close_2d_editor),window);
		g_signal_connect(G_OBJECT(window),"delete_event",
				G_CALLBACK(close_2d_editor),window);
		tmpbuf = g_strdup_printf("%s (%s)",_("2D Table Editor"),firmware->te_params[table_num]->title);
		gtk_window_set_title(GTK_WINDOW(window),tmpbuf);
		g_free(tmpbuf);
		gtk_window_set_default_size(GTK_WINDOW(window),640,480);

		widget = glade_xml_get_widget(xml,"2d_close_button");
		g_signal_connect_swapped(G_OBJECT(widget),"clicked",
				G_CALLBACK(close_2d_editor),window);

		widget = glade_xml_get_widget(xml,"get_data_button");
		OBJ_SET(widget,"handler",GINT_TO_POINTER(READ_VE_CONST));
		OBJ_SET(widget,"bind_to_list",g_strdup("get_data_buttons"));
		bind_to_lists_f(widget,"get_data_buttons");
		widget_list = g_list_prepend(widget_list,(gpointer)widget);

		widget = glade_xml_get_widget(xml,"burn_data_button");
		OBJ_SET(widget,"handler",GINT_TO_POINTER(BURN_MS_FLASH));
		OBJ_SET(widget,"bind_to_list",g_strdup("burners"));
		bind_to_lists_f(widget,"burners");
		widget_list = g_list_prepend(widget_list,(gpointer)widget);

		widget = glade_xml_get_widget(xml,"curve_editor_menuitem");
		gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);

		widget = glade_xml_get_widget(xml,"close_menuitem");
		OBJ_SET(widget,"window",(gpointer)window);

		curve_parent = glade_xml_get_widget(xml,"te_right_frame");
	}
	else
		curve_parent = parent;
	curve = mtx_curve_new();
	curve_list = g_list_prepend(curve_list,(gpointer)curve);
	mtx_curve_set_title(MTX_CURVE(curve),(gchar *)_(firmware->te_params[table_num]->title));
	mtx_curve_set_x_axis_label(MTX_CURVE(curve),_(firmware->te_params[table_num]->x_axis_label));
	mtx_curve_set_y_axis_label(MTX_CURVE(curve),_(firmware->te_params[table_num]->y_axis_label));

	if ((firmware->te_params[table_num]->gauge ||
				firmware->te_params[table_num]->c_gauge ||
				firmware->te_params[table_num]->f_gauge) && (!embedded))
	{
		parent = glade_xml_get_widget(xml,"te_gaugeframe");
		gauge = mtx_gauge_face_new();
		gauge_list = g_list_prepend(gauge_list,(gpointer)gauge);
		if (firmware->te_params[table_num]->gauge_temp_dep)
		{
			if ((GINT)DATA_GET(global_data,"temp_units") == CELSIUS)
				tmpbuf = g_strdelimit(firmware->te_params[table_num]->c_gauge,"\\",'/');
			else
				tmpbuf = g_strdelimit(firmware->te_params[table_num]->f_gauge,"\\",'/');
		}
		else
			tmpbuf = g_strdelimit(firmware->te_params[table_num]->gauge,"\\",'/');
		filename = get_file(g_strconcat(GAUGES_DATA_DIR,PSEP,tmpbuf,NULL),NULL);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		lookup_current_value_f(firmware->te_params[table_num]->gauge_datasource, &tmpf);
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),tmpf);

		g_free(filename);
		id = create_value_change_watch_f(firmware->te_params[table_num]->gauge_datasource,FALSE,"update_misc_gauge",(gpointer)gauge);
		OBJ_SET(gauge,"gauge_id",GINT_TO_POINTER(id));
		gtk_container_add(GTK_CONTAINER(parent),gauge);
	}
	cdata = g_new0(CurveData, 1);
	cdata->curve = curve;
	cdata->axis = _X_;
	cdata->source = firmware->te_params[table_num]->x_source;
	mtx_curve_set_show_x_marker(MTX_CURVE(curve),TRUE);
	OBJ_SET(curve,"cdata",(gpointer)cdata);
	mtx_curve_set_auto_hide_vertexes(MTX_CURVE(curve),TRUE);
	g_signal_connect(G_OBJECT(curve),"coords-changed",
			G_CALLBACK(coords_changed), NULL);
	g_signal_connect(G_OBJECT(curve),"vertex-proximity",
			G_CALLBACK(vertex_proximity), NULL);
	g_signal_connect(G_OBJECT(curve),"marker-proximity",
			G_CALLBACK(marker_proximity), NULL);

	if (!embedded)
	{
		label = glade_xml_get_widget(xml,"x_units");
		gtk_label_set_markup(GTK_LABEL(label),firmware->te_params[table_num]->x_units);
		label = glade_xml_get_widget(xml,"y_units");
		gtk_label_set_markup(GTK_LABEL(label),firmware->te_params[table_num]->y_units);
		label = glade_xml_get_widget(xml,"x_title");
		gtk_label_set_markup(GTK_LABEL(label),firmware->te_params[table_num]->x_name);
		label = glade_xml_get_widget(xml,"y_title");
		gtk_label_set_markup(GTK_LABEL(label),firmware->te_params[table_num]->y_name);
	}
	rows = firmware->te_params[table_num]->bincount;
	mtx_curve_set_empty_array(MTX_CURVE(curve),rows);
	x_table = gtk_table_new(rows,1,FALSE);
	y_table = gtk_table_new(rows,1,FALSE);

	if (embedded)
	{
		x_parent = gtk_frame_new(NULL);
		y_parent = gtk_frame_new(NULL);
	}
	else
	{
		x_parent = glade_xml_get_widget(xml,"te_x_frame");
		y_parent = glade_xml_get_widget(xml,"te_y_frame");
	}
	gtk_container_set_border_width(GTK_CONTAINER(x_table),5);
	gtk_container_set_border_width(GTK_CONTAINER(y_table),5);
	gtk_container_add(GTK_CONTAINER(x_parent),x_table);
	gtk_container_add(GTK_CONTAINER(y_parent),y_table);

	x_mult = get_multiplier_f(firmware->te_params[table_num]->x_size);
	y_mult = get_multiplier_f(firmware->te_params[table_num]->y_size);
	x_entries = g_array_new(FALSE,TRUE,sizeof(GtkWidget *));
	y_entries = g_array_new(FALSE,TRUE,sizeof(GtkWidget *));
	for (i=0;i<rows;i++)
	{
		/* X Column */
		entry = gtk_entry_new();
		OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
		gtk_entry_set_width_chars(GTK_ENTRY(entry),6);
		OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
		g_array_insert_val(x_entries,i,entry);
		OBJ_SET(entry,"curve_axis",GINT_TO_POINTER(_X_));
		OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(entry,"raw_lower",g_strdup_printf("%i",(firmware->te_params[table_num]->x_raw_lower)));
		OBJ_SET(entry,"raw_upper",g_strdup_printf("%i",(firmware->te_params[table_num]->x_raw_upper)));
		OBJ_SET(entry,"dl_conv_expr",g_strdup(firmware->te_params[table_num]->x_dl_conv_expr));
		OBJ_SET(entry,"ul_conv_expr",g_strdup(firmware->te_params[table_num]->x_ul_conv_expr));
		OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->x_precision));
		OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->x_size));
		OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->x_page));
		OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->x_temp_dep));
		OBJ_SET(entry,"use_color",GINT_TO_POINTER(firmware->te_params[table_num]->x_use_color));
		OBJ_SET(entry,"force_color_update",GINT_TO_POINTER(firmware->te_params[table_num]->x_use_color));

		if(firmware->te_params[table_num]->x_temp_dep)
		{
			OBJ_SET(entry,"widget_temp",DATA_GET(global_data,"temp_units"));
			OBJ_SET(entry,"bind_to_list",g_strdup("temperature"));
			bind_to_lists_f(entry,"temperature");
		}

		offset = (i*x_mult) + firmware->te_params[table_num]->x_base;
		OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

		g_signal_connect(G_OBJECT(entry),"changed",
				G_CALLBACK(update_2d_curve),curve);
		g_signal_connect(G_OBJECT(entry),"changed",
				G_CALLBACK(entry_changed_handler_f),NULL);
		g_signal_connect(G_OBJECT(entry),"key_press_event",
				G_CALLBACK(key_event_f),NULL);
		g_signal_connect(G_OBJECT(entry),"key_release_event",
				G_CALLBACK(key_event_f),NULL);
		g_signal_connect(G_OBJECT(entry),"focus_out_event",
				G_CALLBACK(focus_out_handler_f),NULL);
		g_signal_connect(G_OBJECT(entry),"activate",
				G_CALLBACK(std_entry_handler_f),NULL);

		if (firmware->te_params[table_num]->reversed)
			gtk_table_attach(GTK_TABLE(x_table),entry,
					0,1,rows-i-1,rows-i, GTK_SHRINK,GTK_SHRINK,0,0);
		else
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
		OBJ_SET(entry,"raw_lower",g_strdup_printf("%i",(firmware->te_params[table_num]->y_raw_lower)));
		OBJ_SET(entry,"raw_upper",g_strdup_printf("%i",(firmware->te_params[table_num]->y_raw_upper)));
		OBJ_SET(entry,"dl_conv_expr",firmware->te_params[table_num]->y_dl_conv_expr);
		OBJ_SET(entry,"ul_conv_expr",firmware->te_params[table_num]->y_ul_conv_expr);
		OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->y_precision));
		OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->y_size));
		OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->y_page));
		OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->y_temp_dep));
		OBJ_SET(entry,"use_color",GINT_TO_POINTER(firmware->te_params[table_num]->y_use_color));
		OBJ_SET(entry,"force_color_update",GINT_TO_POINTER(firmware->te_params[table_num]->y_use_color));

		if(firmware->te_params[table_num]->y_temp_dep)
		{
			OBJ_SET(entry,"widget_temp",DATA_GET(global_data,"temp_units"));
			OBJ_SET(entry,"bind_to_list",g_strdup("temperature"));
			bind_to_lists_f(entry,"temperature");
		}
		offset = (i*y_mult) + firmware->te_params[table_num]->y_base;
		OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));

		g_signal_connect(G_OBJECT(entry),"changed",
				G_CALLBACK(update_2d_curve),curve);
		g_signal_connect(G_OBJECT(entry),"changed",
				G_CALLBACK(entry_changed_handler_f),NULL);
		g_signal_connect(G_OBJECT(entry),"key_press_event",
				G_CALLBACK(key_event_f),NULL);
		g_signal_connect(G_OBJECT(entry),"key_release_event",
				G_CALLBACK(key_event_f),NULL);
		g_signal_connect(G_OBJECT(entry),"focus_out_event",
				G_CALLBACK(focus_out_handler_f),NULL);
		g_signal_connect(G_OBJECT(entry),"activate",
				G_CALLBACK(std_entry_handler_f),NULL);

		if (firmware->te_params[table_num]->reversed)
			gtk_table_attach(GTK_TABLE(y_table),entry,
					0,1,rows-i-1,rows-i, GTK_SHRINK,GTK_SHRINK,0,0);
		else
			gtk_table_attach(GTK_TABLE(y_table),entry,
					0,1,i,i+1, GTK_SHRINK,GTK_SHRINK,0,0);
		page = firmware->te_params[table_num]->y_page;
		ve_widgets[page][offset] = g_list_prepend(ve_widgets[page][offset],(gpointer)entry);
		widget_list = g_list_prepend(widget_list,(gpointer)entry);

		update_widget(G_OBJECT(entry),NULL);
	}
	/* Create the "LOCK" buttons */
	dummy = gtk_toggle_button_new_with_label("Unlocked");
	OBJ_SET(dummy,"axis",GINT_TO_POINTER(_X_));
	g_signal_connect(G_OBJECT(dummy),"toggled",
			G_CALLBACK(set_axis_locking),curve);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dummy),firmware->te_params[table_num]->x_lock);
	gtk_table_attach(GTK_TABLE(x_table),dummy,
			0,1,i,i+1, GTK_EXPAND|GTK_FILL,0,0,0);
	dummy = gtk_toggle_button_new_with_label("Unlocked");
	OBJ_SET(dummy,"axis",GINT_TO_POINTER(_Y_));
	g_signal_connect(G_OBJECT(dummy),"toggled",
			G_CALLBACK(set_axis_locking),curve);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dummy),firmware->te_params[table_num]->y_lock);
	gtk_table_attach(GTK_TABLE(y_table),dummy,
			0,1,i,i+1, GTK_EXPAND|GTK_FILL,0,0,0);

	mtx_curve_set_x_precision(MTX_CURVE(curve),firmware->te_params[table_num]->x_precision);
	mtx_curve_set_y_precision(MTX_CURVE(curve),firmware->te_params[table_num]->y_precision);

	evaluator = eval_create_f(firmware->te_params[table_num]->x_ul_conv_expr);
	firmware->te_params[table_num]->x_2d_lower_limit = eval_x_f(evaluator,firmware->te_params[table_num]->x_raw_lower);
	firmware->te_params[table_num]->x_2d_upper_limit = eval_x_f(evaluator,firmware->te_params[table_num]->x_raw_upper);
	eval_destroy_f(evaluator);

	evaluator = eval_create_f(firmware->te_params[table_num]->y_ul_conv_expr);
	firmware->te_params[table_num]->y_2d_lower_limit = eval_x_f(evaluator,firmware->te_params[table_num]->y_raw_lower);
	firmware->te_params[table_num]->y_2d_upper_limit = eval_x_f(evaluator,firmware->te_params[table_num]->y_raw_upper);
	eval_destroy_f(evaluator);

	mtx_curve_set_hard_limits(MTX_CURVE(curve),
			firmware->te_params[table_num]->x_2d_lower_limit,
			firmware->te_params[table_num]->x_2d_upper_limit,
			firmware->te_params[table_num]->y_2d_lower_limit,
			firmware->te_params[table_num]->y_2d_upper_limit);

	/* One shot to get marker drawn. */
	create_value_change_watch_f(cdata->source,TRUE,"update_curve_marker",(gpointer)cdata);
	/* continuous to catch changes. */
	id = create_value_change_watch_f(cdata->source,FALSE,"update_curve_marker",(gpointer)cdata);
	OBJ_SET(curve,"marker_id",GINT_TO_POINTER(id));
	if (!embedded)
	{
		OBJ_SET(window,"widget_list",widget_list);
		OBJ_SET(window,"curve_list",curve_list);
		OBJ_SET(window,"x_entries",x_entries);
		OBJ_SET(window,"y_entries",y_entries);
	}
	OBJ_SET(curve,"x_entries",x_entries);
	OBJ_SET(curve,"y_entries",y_entries);
	gtk_container_add(GTK_CONTAINER(curve_parent),curve);

	if (embedded)
		gtk_widget_show_all(curve);
	else
		gtk_widget_show_all(window);
	return TRUE;
}


G_MODULE_EXPORT gboolean close_2d_editor(GtkWidget * widget, gpointer data)
{
	GList *list = NULL;

	list = OBJ_GET(widget, "widget_list");
	if (list)
	{
		g_list_foreach(list,remove_widget,(gpointer)list);
		g_list_free(list);
		list = NULL;
	}
	list = OBJ_GET(widget, "curve_list");
	if (list)
	{
		g_list_foreach(list,clean_curve,NULL);
		g_list_free(list);
		list = NULL;
	}
	list = OBJ_GET(widget, "gauge_list");
	if (list)
	{
		g_list_foreach(list,gauge_cleanup,NULL);
		g_list_free(list);
		list = NULL;
	}
	gtk_widget_destroy(widget);
	return FALSE;
}


G_MODULE_EXPORT void remove_widget(gpointer widget_ptr, gpointer data)
{
	GList ***ve_widgets = NULL;
	gint page = -1;
	gint offset = -1;

	ve_widgets = DATA_GET(global_data,"ve_widgets");
	remove_from_lists_f(OBJ_GET(widget_ptr,"bind_to_list"),widget_ptr);
	if (OBJ_GET(widget_ptr,"page"))
		page = (GINT)OBJ_GET(widget_ptr,"page");
	else
		page = -1;
	if (OBJ_GET(widget_ptr,"offset"))
		offset = (GINT)OBJ_GET(widget_ptr,"offset");
	else
		offset = -1;
	if (( page >= 0 ) && (offset >= 0))
		ve_widgets[page][offset] = g_list_remove(ve_widgets[page][offset],widget_ptr);
	/*dealloc_widget(widget_ptr,NULL);*/
}


G_MODULE_EXPORT void gauge_cleanup(gpointer gauge_ptr, gpointer data)
{
	gint id = 0;
	GtkWidget *widget = (GtkWidget *)gauge_ptr;
	if (OBJ_GET(widget, "gauge_id"))
	{
		id = (GINT)OBJ_GET(widget, "gauge_id");
		remove_watch_f(id);
	}
}


G_MODULE_EXPORT void clean_curve(gpointer curve_ptr, gpointer data)
{
	GArray *array = NULL;
	guint32 id = 0;
	CurveData *cdata = NULL;
	GtkWidget *widget = (GtkWidget *)curve_ptr;

	array = OBJ_GET(widget, "x_entries");
	if (array)
		g_array_free(array,TRUE);
	array = OBJ_GET(widget, "y_entries");
	if (array)
		g_array_free(array,TRUE);
	cdata = OBJ_GET(widget, "cdata");
	if (cdata)
		g_free(cdata);
	id = (guint32)(GINT)OBJ_GET(widget, "marker_id");
	if (id > 0)
		remove_watch_f(id);
}

G_MODULE_EXPORT gboolean update_2d_curve(GtkWidget *widget, gpointer data)
{
	GtkWidget *curve = (GtkWidget *)data;
	MtxCurveCoord point;
	Axis axis;
	gint index = 0;
	gchar * text = NULL;
	gfloat tmpf = 0.0;

	index = (GINT) OBJ_GET(widget,"curve_index");
	axis = (Axis) OBJ_GET(widget,"curve_axis");
	mtx_curve_get_coords_at_index(MTX_CURVE(curve),index,&point);
	text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	tmpf = (gfloat)strtod(g_strdelimit(text,",.",'.'),NULL);
	if (axis == _X_)
		point.x = tmpf;
	else if (axis == _Y_)
		point.y = tmpf;
	else
		printf(_("ERROR in update_2d_curve()!!!\n"));
	mtx_curve_set_coords_at_index(MTX_CURVE(curve),index,point);
	return FALSE;

}


G_MODULE_EXPORT void coords_changed(GtkWidget *curve, gpointer data)
{
	MtxCurveCoord point;
	gint index = 0;
	GArray *array;
	gint precision = 0;
	GtkWidget *entry = NULL;
	gchar * tmpbuf = NULL;

	index = mtx_curve_get_active_coord_index(MTX_CURVE(curve));
	mtx_curve_get_coords_at_index(MTX_CURVE(curve),index,&point);

	if(!mtx_curve_get_x_axis_lock_state(MTX_CURVE(curve)))
	{
		/* X Coord */
		array = OBJ_GET(curve,"x_entries");
		entry = g_array_index(array,GtkWidget *,index);
		precision = (GINT)OBJ_GET(entry, "precision");
		tmpbuf = g_strdup_printf("%1$.*2$f",point.x,precision);
		gtk_entry_set_text(GTK_ENTRY(entry),tmpbuf);
		g_signal_emit_by_name(entry, "activate");
		g_free(tmpbuf);
	}

	if(!mtx_curve_get_y_axis_lock_state(MTX_CURVE(curve)))
	{
		/* Y Coord */
		array = OBJ_GET(curve,"y_entries");
		entry  = g_array_index(array,GtkWidget *,index);
		precision = (GINT)OBJ_GET(entry, "precision");
		tmpbuf = g_strdup_printf("%1$.*2$f",point.y,precision);
		gtk_entry_set_text(GTK_ENTRY(entry),tmpbuf);
		g_signal_emit_by_name(entry, "activate");
		g_free(tmpbuf);
	}
}


G_MODULE_EXPORT void vertex_proximity(GtkWidget *curve, gpointer data)
{
	gint index = 0;
	gint last = 0;
	gint last_marker = 0;
	GArray *x_array;
	GArray *y_array;
	GtkWidget *entry = NULL;
	GdkColor blue = { 0, 0, 0, 65535};
	GdkColor green = { 0, 0, 65535, 0};

	index = mtx_curve_get_vertex_proximity_index(MTX_CURVE(curve));
	x_array = (GArray *)OBJ_GET(curve,"x_entries");
	y_array = (GArray *)OBJ_GET(curve,"y_entries");
	if ((!x_array) || (!y_array))
		return;
	if (index >= 0)	/* we are on a vertex for sure */
	{
		/* Turn the current one red */
		entry = g_array_index(x_array,GtkWidget *,index);
		highlight_entry(entry,&blue);
		entry = g_array_index(y_array,GtkWidget *,index);
		highlight_entry(entry,&blue);
		if (!OBJ_GET(curve,"last_proximity_vertex"))
		{
			/* Last undefined, must be first run, 
			 * nothing to reset, thus store last 
			 * and break out
			 */
			OBJ_SET(curve, "last_proximity_vertex",GINT_TO_POINTER(index+1));
			return;
		}
		else	/* Last IS defined, thus check for polarity */
			last = (GINT)OBJ_GET(curve,"last_proximity_vertex");
		if (last < 0)
		{
			OBJ_SET(curve, "last_proximity_vertex",GINT_TO_POINTER(index+1));
			return;	/* No vertex to undo */
		}
		else
		{	/* Need to reset previous vertex back to defaults */
			last_marker = (GINT)OBJ_GET(curve,"last_marker_vertex");
			if (last_marker != last) /* Set to marker color instead */
			{
				entry = g_array_index(x_array,GtkWidget *,last-1);
				highlight_entry(entry,NULL);
				entry = g_array_index(y_array,GtkWidget *,last-1);
				highlight_entry(entry,NULL);
			}
			else
			{
				entry = g_array_index(x_array,GtkWidget *,last-1);
				highlight_entry(entry,&green);
				entry = g_array_index(y_array,GtkWidget *,last-1);
				highlight_entry(entry,&green);
			}
		}
	}
	else
	{	/* we are NOT on a vertex, so check last status */
		if (!OBJ_GET(curve,"last_proximity_vertex"))
		{
			/* Last undefined, must be first run, 
			 * nothing to reset, thus store last 
			 * and break out
			 */
			OBJ_SET(curve, "last_proximity_vertex",GINT_TO_POINTER(index+1));
			return;
		}
		else	/* Last IS defined, thus check for polarity */
			last = (GINT)OBJ_GET(curve,"last_proximity_vertex");
		if (last < 0)
		{
			OBJ_SET(curve, "last_proximity_vertex",GINT_TO_POINTER(index+1));
			return;	/* No vertex to undo */
		}
		else
		{	/* Need to reset previous vertex back to defaults */
			last_marker = (GINT)OBJ_GET(curve,"last_marker_vertex");
			if (last_marker != last) /* Set to marker color instead */
			{
				entry = g_array_index(x_array,GtkWidget *,last-1);
				highlight_entry(entry,NULL);
				entry = g_array_index(y_array,GtkWidget *,last-1);
				highlight_entry(entry,NULL);
			}
			else
			{
				entry = g_array_index(x_array,GtkWidget *,last-1);
				highlight_entry(entry,&green);
				entry = g_array_index(y_array,GtkWidget *,last-1);
				highlight_entry(entry,&green);
			}
		}
	}
	OBJ_SET(curve, "last_proximity_vertex",GINT_TO_POINTER(index+1));

}


G_MODULE_EXPORT void marker_proximity(GtkWidget *curve, gpointer data)
{
	gint index = 0;
	gint m_index = 0;
	gint last = 0;
	gint last_vertex = 0;
	GArray *x_array;
	GArray *y_array;
	GtkWidget *entry = NULL;
	GdkColor blue = { 0, 0, 0, 65535};
	GdkColor green = { 0, 0, 65535, 0};

	index = mtx_curve_get_marker_proximity_index(MTX_CURVE(curve));
	m_index = mtx_curve_get_vertex_proximity_index(MTX_CURVE(curve));
	x_array = (GArray *)OBJ_GET(curve,"x_entries");
	y_array = (GArray *)OBJ_GET(curve,"y_entries");
	if ((!x_array) || (!y_array))
		return;
	if ((index >= 0) && (m_index != index))	/* we are on a vertex for sure */
	{
		/* Turn the current one green */
		entry = g_array_index(x_array,GtkWidget *,index);
		highlight_entry(entry,&green);
		entry = g_array_index(y_array,GtkWidget *,index);
		highlight_entry(entry,&green);
		if (!OBJ_GET(curve,"last_marker_vertex"))
		{
			/* Last undefined, must be first run, 
			 * nothing to reset, thus store last 
			 * and break out
			 */
			OBJ_SET(curve, "last_marker_vertex",GINT_TO_POINTER(index+1));
			return;
		}
		else	/* Last IS defined, thus check for polarity */
			last = (GINT)OBJ_GET(curve,"last_marker_vertex");
		if (last < 0)
		{
			OBJ_SET(curve, "last_marker_vertex",GINT_TO_POINTER(index+1));
			return;	/* No vertex to undo */
		}
		else
		{	/* Need to reset previous vertex back to defaults */
			last_vertex = (GINT)OBJ_GET(curve,"last_proximity_vertex");
			if (last != last_vertex)
			{
				entry = g_array_index(x_array,GtkWidget *,last-1);
				highlight_entry(entry,NULL);
				entry = g_array_index(y_array,GtkWidget *,last-1);
				highlight_entry(entry,NULL);
			}
			else
			{
				entry = g_array_index(x_array,GtkWidget *,last-1);
				highlight_entry(entry,&blue);
				entry = g_array_index(y_array,GtkWidget *,last-1);
				highlight_entry(entry,&blue);
			}
		}
	}
	else
	{	/* we are NOT on a vertex, so check last status */
		if (!OBJ_GET(curve,"last_marker_vertex"))
		{
			/* Last undefined, must be first run, 
			 * nothing to reset, thus store last 
			 * and break out
			 */
			OBJ_SET(curve, "last_marker_vertex",GINT_TO_POINTER(index+1));
			return;
		}
		else	/* Last IS defined, thus check for polarity */
			last = (GINT)OBJ_GET(curve,"last_marker_vertex");
		if (last < 0)
		{
			OBJ_SET(curve, "last_marker_vertex",GINT_TO_POINTER(index+1));
			return;	/* No vertex to undo */
		}
		else
		{	/* Need to reset previous vertex back to defaults */
			last_vertex = (GINT)OBJ_GET(curve,"last_proximity_vertex");
			if (last != last_vertex)
			{
				entry = g_array_index(x_array,GtkWidget *,last-1);
				highlight_entry(entry,NULL);
				entry = g_array_index(y_array,GtkWidget *,last-1);
				highlight_entry(entry,NULL);
			}
			else
			{
				entry = g_array_index(x_array,GtkWidget *,last-1);
				highlight_entry(entry,&blue);
				entry = g_array_index(y_array,GtkWidget *,last-1);
				highlight_entry(entry,&blue);
			}
		}
	}
	OBJ_SET(curve, "last_marker_vertex",GINT_TO_POINTER(index+1));

}


G_MODULE_EXPORT gboolean close_menu_handler(GtkWidget * widget, gpointer data)
{
	close_2d_editor(OBJ_GET(widget,"window"),NULL);
	return TRUE;
}


G_MODULE_EXPORT void update_curve_marker(DataWatch *watch)
{
	CurveData *cdata = (CurveData *)watch->user_data;
	if (!MTX_IS_CURVE(cdata->curve))
	{
	        remove_watch_f(watch->id);
		return;
	}
	if (cdata->axis == _X_)
		mtx_curve_set_x_marker_value(MTX_CURVE(cdata->curve),watch->val);
	if (cdata->axis == _Y_)
		mtx_curve_set_y_marker_value(MTX_CURVE(cdata->curve),watch->val);
}


G_MODULE_EXPORT gboolean set_axis_locking(GtkWidget *widget, gpointer data)
{
	Axis axis = (Axis)OBJ_GET(widget,"axis");
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if (state)
		gtk_button_set_label(GTK_BUTTON(widget)," Locked ");
	else
		gtk_button_set_label(GTK_BUTTON(widget),"Unlocked");
	if (axis == _X_)
		mtx_curve_set_x_axis_lock_state(MTX_CURVE(data),state);
	if (axis == _Y_)
		mtx_curve_set_y_axis_lock_state(MTX_CURVE(data),state);
	return TRUE;
}


G_MODULE_EXPORT gboolean add_2d_table(GtkWidget *widget)
{
	gint table_num = 0;

	if (!GTK_IS_WIDGET(widget))
		return FALSE;

	table_num = (gint)g_ascii_strtod(OBJ_GET(widget,"te_table_num"),NULL);
	create_2d_table_editor(table_num,widget);
	return TRUE;
}


G_MODULE_EXPORT void highlight_entry(GtkWidget *widget, GdkColor *color)
{
#ifdef __WIN32__
	if ((GTK_WIDGET_VISIBLE(widget)) && (GTK_WIDGET_SENSITIVE(widget)))
	{
		if (!color) 
		{
			if (OBJ_GET(widget,"use_color")) 	/* Color reset */
				update_widget((GObject *)widget,NULL);
			else
				gtk_widget_modify_base(widget,GTK_STATE_NORMAL,color);
		}
		else
			gtk_widget_modify_base(widget,GTK_STATE_NORMAL,color);
	}
	return;
#else
	GdkGC *gc = OBJ_GET(widget,"hl_gc");
	extern GdkColor white;
	if (!GDK_IS_DRAWABLE(widget->window))
		return;
	if (!gc)
	{
		gc = gdk_gc_new(widget->window);
		gdk_gc_set_subwindow(gc,GDK_INCLUDE_INFERIORS);
		OBJ_SET(widget,"hl_gc",(gpointer)gc);
	}
	if (color)
		gdk_gc_set_rgb_fg_color(gc,color);
	else
	{
		if (OBJ_GET(widget, "use_color"))
			gdk_gc_set_rgb_fg_color(gc,&widget->style->base[GTK_STATE_NORMAL]);
		else
			gdk_gc_set_rgb_fg_color(gc,&white);
	}
	/* Top */
	gdk_draw_rectangle(widget->window,gc,TRUE,2,2,widget->allocation.width-4,2);
	/* Bottom */
	gdk_draw_rectangle(widget->window,gc,TRUE,2,widget->allocation.height-5,widget->allocation.width-4,2);
	/* Left */
	gdk_draw_rectangle(widget->window,gc,TRUE,2,2,2,widget->allocation.height-6);
	/* Right */
	gdk_draw_rectangle(widget->window,gc,TRUE,widget->allocation.width-4,2,2,widget->allocation.height-6);
#endif
}

