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
  \file src/plugins/mscommon/2d_table_editor.c
  \ingroup MSCommonPlugin,Plugins
  \brief MSCommon 2d table editor, NOTE this should move to global with MS
  specific stubs here
  \author David Andruczyk
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
#include <mscommon_plugin.h>
#include <mscommon_gui_handlers.h>
#include <stdlib.h>

extern gconstpointer *global_data;

/*!
  \brief CurveData holds basic info on each curve including the widget itself
  its axis and its source
  */
typedef struct
{
	GtkWidget *curve;		/*!< Widget Pointer */
	Axis axis;			/*!< Axis */
	gchar *source;			/*!< data source */
}CurveData;


/*!
  \brief Creates a group of 2D Table Editors packed into a GtkNotebook
  \param button is a pointer to the widget the user click on, which has bound
  to is a list of the TE Table ID's which we need to create on-screen 
  representations for
  \returns TRUE on success, FALSE otherwise
  */
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
	GList ***ecu_widgets = NULL;
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
	gchar *pathstub = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");

	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if (!main_xml)
	{
		EXIT();
		return FALSE;
	}

	xml = glade_xml_new(main_xml->filename,"table_editor_window",NULL);
	window = glade_xml_get_widget(xml,"table_editor_window");

	glade_xml_signal_autoconnect(xml);

	g_signal_connect(G_OBJECT(window),"destroy_event",
			G_CALLBACK(close_2d_editor),NULL);
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(close_2d_editor),NULL);
	gtk_window_set_title(GTK_WINDOW(window),_("2D Table Group Editor"));
	gtk_window_resize(GTK_WINDOW(window),800,530);

	widget = glade_xml_get_widget(xml,"2d_close_button");
	g_signal_connect_swapped(G_OBJECT(widget),"clicked",
			G_CALLBACK(close_2d_editor),window);

	widget = glade_xml_get_widget(xml,"get_data_button");
	OBJ_SET(widget,"handler",GINT_TO_POINTER(READ_VE_CONST));
	OBJ_SET_FULL(widget,"bind_to_list",g_strdup("get_data_buttons"),g_free);
	bind_to_lists_f(widget,"get_data_buttons");
	widget_list = g_list_prepend(widget_list,(gpointer)widget);

	widget = glade_xml_get_widget(xml,"burn_data_button");
	OBJ_SET(widget,"handler",GINT_TO_POINTER(BURN_FLASH));
	OBJ_SET_FULL(widget,"bind_to_list",g_strdup("burners"),g_free);
	bind_to_lists_f(widget,"burners");
	widget_list = g_list_prepend(widget_list,(gpointer)widget);

	/*
	widget = glade_xml_get_widget(xml,"curve_editor_menuitem");
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);
	*/

	widget = glade_xml_get_widget(xml,"close_menuitem");
	OBJ_SET(widget,"window",(gpointer)window);

	widget = glade_xml_get_widget(xml,"te_layout_hbox1");
	gtk_widget_destroy(widget);

	widget = glade_xml_get_widget(xml,"te_layout_vbox");

	tmpbuf = (gchar *)OBJ_GET(button,"te_tables");
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
			EXIT();
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
			OBJ_SET_FULL(widget,"bind_to_list", g_strdup(firmware->te_params[table_num]->bind_to_list),g_free);
			OBJ_SET(widget,"match_type", GINT_TO_POINTER(firmware->te_params[table_num]->match_type));
			bind_to_lists_f(widget,firmware->te_params[table_num]->bind_to_list);
			widget_list = g_list_prepend(widget_list,(gpointer)widget);
			OBJ_SET_FULL(label,"bind_to_list", g_strdup(firmware->te_params[table_num]->bind_to_list),g_free);
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
				if ((GINT)DATA_GET(global_data,"mtx_temp_units") == CELSIUS)
					tmpbuf = g_strdelimit(firmware->te_params[table_num]->c_gauge,"\\",'/');
				else if ((GINT)DATA_GET(global_data,"mtx_temp_units") == KELVIN)
					tmpbuf = g_strdelimit(firmware->te_params[table_num]->c_gauge,"\\",'/');
				else
					tmpbuf = g_strdelimit(firmware->te_params[table_num]->f_gauge,"\\",'/');
			}
			else
				tmpbuf = g_strdelimit(firmware->te_params[table_num]->gauge,"\\",'/');
			pathstub = g_build_filename(GAUGES_DATA_DIR,tmpbuf,NULL);
			filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,NULL);
			g_free(pathstub);
			mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
			lookup_current_value_f(firmware->te_params[table_num]->gauge_datasource, &tmpf);
			mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),tmpf);
			g_free(filename);
			id = create_rtv_value_change_watch_f(firmware->te_params[table_num]->gauge_datasource,FALSE,"update_misc_gauge",(gpointer)gauge);
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
		id = create_rtv_value_change_watch_f(cdata->source,FALSE,"update_curve_marker",(gpointer)cdata);
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
			OBJ_SET(entry,"last_value",GINT_TO_POINTER(-G_MAXINT));
			gtk_entry_set_width_chars(GTK_ENTRY(entry),7);
			OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
			g_array_insert_val(x_entries,i,entry);
			OBJ_SET(entry,"curve_axis",GINT_TO_POINTER(_X_));
			OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET_FULL(entry,"raw_lower",g_strdup_printf("%i",(firmware->te_params[table_num]->x_raw_lower)),g_free);
			OBJ_SET_FULL(entry,"raw_upper",g_strdup_printf("%i",(firmware->te_params[table_num]->x_raw_upper)),g_free);
			OBJ_SET(entry,"fromecu_mult",firmware->te_params[table_num]->x_fromecu_mult);
			OBJ_SET(entry,"fromecu_add",firmware->te_params[table_num]->x_fromecu_add);
			OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->x_precision));
			OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->x_size));
			OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->x_page));
			OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->x_temp_dep));
			OBJ_SET(entry,"use_color",GINT_TO_POINTER(firmware->te_params[table_num]->x_use_color));
			if(firmware->te_params[table_num]->x_temp_dep)
			{
				OBJ_SET(entry,"widget_temp",DATA_GET(global_data,"mtx_temp_units"));
				OBJ_SET_FULL(entry,"bind_to_list", g_strdup("temperature"),g_free);
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
			ecu_widgets[page][offset] = g_list_prepend(ecu_widgets[page][offset],(gpointer)entry);
			widget_list = g_list_prepend(widget_list,(gpointer)entry);
			update_widget(G_OBJECT(entry),NULL);

			/* Y Column */
			entry = gtk_entry_new();
			OBJ_SET(entry,"last_value",GINT_TO_POINTER(-G_MAXINT));
			gtk_entry_set_width_chars(GTK_ENTRY(entry),7);
			OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
			g_array_insert_val(y_entries,i,entry);
			OBJ_SET(entry,"curve_axis",GINT_TO_POINTER(_Y_));
			OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET_FULL(entry,"raw_lower",g_strdup_printf("%i",(firmware->te_params[table_num]->y_raw_lower)),g_free);
			OBJ_SET_FULL(entry,"raw_upper",g_strdup_printf("%i",(firmware->te_params[table_num]->y_raw_upper)),g_free);
			OBJ_SET(entry,"fromecu_mult",firmware->te_params[table_num]->y_fromecu_mult);
			OBJ_SET(entry,"fromecu_add",firmware->te_params[table_num]->y_fromecu_add);
			OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->y_precision));
			OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->y_size));
			OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->y_page));
			OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->y_temp_dep));
			OBJ_SET(entry,"use_color",GINT_TO_POINTER(firmware->te_params[table_num]->y_use_color));
			if(firmware->te_params[table_num]->y_temp_dep)
			{
				OBJ_SET(entry,"widget_temp",DATA_GET(global_data,"mtx_temp_units"));
				OBJ_SET_FULL(entry,"bind_to_list", g_strdup("temperature"),g_free);
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
			ecu_widgets[page][offset] = g_list_prepend(ecu_widgets[page][offset],(gpointer)entry);
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
				0,1,i,i+1, (GtkAttachOptions)(GTK_EXPAND|GTK_FILL),(GtkAttachOptions)0,0,0);
		dummy = gtk_toggle_button_new_with_label("Unlocked");
		OBJ_SET(dummy,"axis",GINT_TO_POINTER(_Y_));
		g_signal_connect(G_OBJECT(dummy),"toggled",
				G_CALLBACK(set_axis_locking),curve);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dummy),firmware->te_params[table_num]->y_lock);
		gtk_table_attach(GTK_TABLE(y_table),dummy,
				0,1,i,i+1,(GtkAttachOptions)(GTK_EXPAND|GTK_FILL),(GtkAttachOptions)0,0,0);

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
		create_rtv_value_change_watch_f(cdata->source,TRUE,"update_curve_marker",(gpointer)cdata);
		gtk_container_add(GTK_CONTAINER(curve_parent),curve);
	}
	OBJ_SET(window,"widget_list",widget_list);
	OBJ_SET(window,"curve_list",curve_list);
	OBJ_SET(window,"gauge_list",gauge_list);
	gtk_widget_show_all(window);
	EXIT();
	return TRUE;
}


/*!
  \brief Creates a 2D Table Editors for visualizing a simpel 2D table
  \param table_num is the TE table ID we need to create the display for.
  \param parent if set it's the parent widget to embed this editor into
  \returns TRUE on success, FALSE otherwise
  */
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
	GList ***ecu_widgets = NULL;
	gfloat *mult = NULL;
	gfloat *adder = NULL;
	gfloat raw_lower = 0;
	gfloat raw_upper = 0;
	gint x_mult = 0;
	gint y_mult = 0;
	gint page = 0;
	gint offset = 0;
	gint i = 0;
	guint32 id = 0;
	gfloat tmpf = 0.0;
	gint rows = 0;
	gboolean embedded = FALSE;
	Firmware_Details *firmware = NULL;
	gchar *pathstub = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");

	if (table_num >= firmware->total_te_tables)
	{
		warn_user_f("Requested to create 2D table editor window for an undefined (out of range) table ID");
		EXIT();
		return FALSE;
	}
	if (!firmware->te_params)
	{
		warn_user_f("No 2D table Editor tables (te_tables) defined in interrogation profile, yet told to create a graph for a table... BUG detected!");
		EXIT();
		return FALSE;
	}
	if (!firmware->te_params[table_num])
	{
		warn_user_f("Requested to create a 2D table editor window for an undefined table!");
		EXIT();
		return FALSE;
	}
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if (!main_xml)
	{
		EXIT();
		return FALSE;
	}

	if (GTK_IS_WIDGET(parent)) /* Embedded mode */
		embedded = TRUE;

	if (!embedded)
	{
		xml = glade_xml_new(main_xml->filename,"table_editor_window",NULL);
		window = glade_xml_get_widget(xml,"table_editor_window");
		if (firmware->te_params[table_num]->bind_to_list)
		{
			OBJ_SET_FULL(window,"bind_to_list", g_strdup(firmware->te_params[table_num]->bind_to_list),g_free);
			OBJ_SET(window,"match_type", GINT_TO_POINTER(firmware->te_params[table_num]->match_type));
			bind_to_lists_f(window,firmware->te_params[table_num]->bind_to_list);
			widget_list = g_list_prepend(widget_list,(gpointer)window);
		}
		glade_xml_signal_autoconnect(xml);

		g_signal_connect(G_OBJECT(window),"destroy_event",
				G_CALLBACK(close_2d_editor),NULL);
		g_signal_connect(G_OBJECT(window),"delete_event",
				G_CALLBACK(close_2d_editor),NULL);
		tmpbuf = g_strdup_printf("%s (%s)",_("2D Table Editor"),firmware->te_params[table_num]->title);
		gtk_window_set_title(GTK_WINDOW(window),tmpbuf);
		g_free(tmpbuf);
		gtk_window_set_default_size(GTK_WINDOW(window),640,480);

		widget = glade_xml_get_widget(xml,"2d_close_button");
		g_signal_connect_swapped(G_OBJECT(widget),"clicked",
				G_CALLBACK(close_2d_editor),window);

		widget = glade_xml_get_widget(xml,"get_data_button");
		OBJ_SET(widget,"handler",GINT_TO_POINTER(READ_VE_CONST));
		OBJ_SET_FULL(widget,"bind_to_list",g_strdup("get_data_buttons"),g_free);
		bind_to_lists_f(widget,"get_data_buttons");
		widget_list = g_list_prepend(widget_list,(gpointer)widget);

		widget = glade_xml_get_widget(xml,"burn_data_button");
		OBJ_SET(widget,"handler",GINT_TO_POINTER(BURN_FLASH));
		OBJ_SET_FULL(widget,"bind_to_list",g_strdup("burners"),g_free);
		bind_to_lists_f(widget,"burners");
		widget_list = g_list_prepend(widget_list,(gpointer)widget);

		/*
		widget = glade_xml_get_widget(xml,"curve_editor_menuitem");
		gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);
		*/

		widget = glade_xml_get_widget(xml,"close_menuitem");
		OBJ_SET(widget,"window",(gpointer)window);

		curve_parent = glade_xml_get_widget(xml,"te_right_frame");
	}
	else
		curve_parent = parent;
	curve = mtx_curve_new();
	if (!embedded)
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
			if ((GINT)DATA_GET(global_data,"mtx_temp_units") == CELSIUS)
				tmpbuf = g_strdelimit(firmware->te_params[table_num]->c_gauge,"\\",'/');
			else if ((GINT)DATA_GET(global_data,"mtx_temp_units") == KELVIN)
				tmpbuf = g_strdelimit(firmware->te_params[table_num]->c_gauge,"\\",'/');
			else
				tmpbuf = g_strdelimit(firmware->te_params[table_num]->f_gauge,"\\",'/');
		}
		else
			tmpbuf = g_strdelimit(firmware->te_params[table_num]->gauge,"\\",'/');
		pathstub = g_build_filename(GAUGES_DATA_DIR,tmpbuf,NULL);
		filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,NULL);
		g_free(pathstub);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		lookup_current_value_f(firmware->te_params[table_num]->gauge_datasource, &tmpf);
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),tmpf);

		g_free(filename);
		id = create_rtv_value_change_watch_f(firmware->te_params[table_num]->gauge_datasource,FALSE,"update_misc_gauge",(gpointer)gauge);
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
		OBJ_SET(entry,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
		gtk_entry_set_width_chars(GTK_ENTRY(entry),7);
		OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
		g_array_insert_val(x_entries,i,entry);
		OBJ_SET(entry,"curve_axis",GINT_TO_POINTER(_X_));
		OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET_FULL(entry,"raw_lower",g_strdup_printf("%i",(firmware->te_params[table_num]->x_raw_lower)),g_free);
		OBJ_SET_FULL(entry,"raw_upper",g_strdup_printf("%i",(firmware->te_params[table_num]->x_raw_upper)),g_free);
		OBJ_SET(entry,"fromecu_mult",firmware->te_params[table_num]->x_fromecu_mult);
		OBJ_SET(entry,"fromecu_add",firmware->te_params[table_num]->x_fromecu_add);
		OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->x_precision));
		OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->x_size));
		OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->x_page));
		OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->x_temp_dep));
		OBJ_SET(entry,"use_color",GINT_TO_POINTER(firmware->te_params[table_num]->x_use_color));

		if(firmware->te_params[table_num]->x_temp_dep)
		{
			OBJ_SET(entry,"widget_temp",DATA_GET(global_data,"mtx_temp_units"));
			OBJ_SET_FULL(entry,"bind_to_list",g_strdup("temperature"),g_free);
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
		ecu_widgets[page][offset] = g_list_prepend(ecu_widgets[page][offset],(gpointer)entry);
		if (!embedded)
			widget_list = g_list_prepend(widget_list,(gpointer)entry);

		update_widget(G_OBJECT(entry),NULL);

		/* Y Column */
		entry = gtk_entry_new();
		OBJ_SET(entry,"last_value",GINT_TO_POINTER(-G_MAXINT));
		gtk_entry_set_width_chars(GTK_ENTRY(entry),7);
		OBJ_SET(entry,"curve_index",GINT_TO_POINTER(i));
		g_array_insert_val(y_entries,i,entry);
		OBJ_SET(entry,"curve_axis",GINT_TO_POINTER(_Y_));
		OBJ_SET(entry,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(entry,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET_FULL(entry,"raw_lower",g_strdup_printf("%i",(firmware->te_params[table_num]->y_raw_lower)),g_free);
		OBJ_SET_FULL(entry,"raw_upper",g_strdup_printf("%i",(firmware->te_params[table_num]->y_raw_upper)),g_free);
		OBJ_SET(entry,"fromecu_mult",firmware->te_params[table_num]->y_fromecu_mult);
		OBJ_SET(entry,"fromecu_add",firmware->te_params[table_num]->y_fromecu_add);
		OBJ_SET(entry,"precision",GINT_TO_POINTER(firmware->te_params[table_num]->y_precision));
		OBJ_SET(entry,"size",GINT_TO_POINTER(firmware->te_params[table_num]->y_size));
		OBJ_SET(entry,"page",GINT_TO_POINTER(firmware->te_params[table_num]->y_page));
		OBJ_SET(entry,"temp_dep",GINT_TO_POINTER(firmware->te_params[table_num]->y_temp_dep));
		OBJ_SET(entry,"use_color",GINT_TO_POINTER(firmware->te_params[table_num]->y_use_color));

		if(firmware->te_params[table_num]->y_temp_dep)
		{
			OBJ_SET(entry,"widget_temp",DATA_GET(global_data,"mtx_temp_units"));
			OBJ_SET_FULL(entry,"bind_to_list",g_strdup("temperature"),g_free);
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
		ecu_widgets[page][offset] = g_list_prepend(ecu_widgets[page][offset],(gpointer)entry);
		if (!embedded)
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
			0,1,i,i+1,(GtkAttachOptions)(GTK_EXPAND|GTK_FILL),(GtkAttachOptions)0,0,0);
	dummy = gtk_toggle_button_new_with_label("Unlocked");
	OBJ_SET(dummy,"axis",GINT_TO_POINTER(_Y_));
	g_signal_connect(G_OBJECT(dummy),"toggled",
			G_CALLBACK(set_axis_locking),curve);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dummy),firmware->te_params[table_num]->y_lock);
	gtk_table_attach(GTK_TABLE(y_table),dummy,
			0,1,i,i+1,(GtkAttachOptions)(GTK_EXPAND|GTK_FILL),(GtkAttachOptions)0,0,0);

	mtx_curve_set_x_precision(MTX_CURVE(curve),firmware->te_params[table_num]->x_precision);
	mtx_curve_set_y_precision(MTX_CURVE(curve),firmware->te_params[table_num]->y_precision);

	mult = firmware->te_params[table_num]->x_fromecu_mult;
	adder = firmware->te_params[table_num]->x_fromecu_add;
	raw_lower = firmware->te_params[table_num]->x_raw_lower;
	raw_upper = firmware->te_params[table_num]->x_raw_upper;

	if ((mult) && (adder))
	{
		firmware->te_params[table_num]->x_2d_lower_limit = 
			(raw_lower * (*mult)) + (*adder);
		firmware->te_params[table_num]->x_2d_upper_limit = 
			(raw_upper * (*mult)) + (*adder);
	}
	else if (mult)
	{
		firmware->te_params[table_num]->x_2d_lower_limit = 
			raw_lower * (*mult);
		firmware->te_params[table_num]->x_2d_upper_limit = 
			raw_upper * (*mult);
	}
	else
	{
		firmware->te_params[table_num]->x_2d_lower_limit = raw_lower;
		firmware->te_params[table_num]->x_2d_upper_limit = raw_upper;
	}

	/* Y */
	mult = firmware->te_params[table_num]->y_fromecu_mult;
	adder = firmware->te_params[table_num]->y_fromecu_add;
	raw_lower = firmware->te_params[table_num]->y_raw_lower;
	raw_upper = firmware->te_params[table_num]->y_raw_upper;

	if ((mult) && (adder))
	{
		firmware->te_params[table_num]->y_2d_lower_limit = 
			(raw_lower * (*mult)) + (*adder);
		firmware->te_params[table_num]->y_2d_upper_limit = 
			(raw_upper * (*mult)) + (*adder);
	}
	else if (mult)
	{
		firmware->te_params[table_num]->y_2d_lower_limit = 
			raw_lower * (*mult);
		firmware->te_params[table_num]->y_2d_upper_limit = 
			raw_upper * (*mult);
	}
	else
	{
		firmware->te_params[table_num]->y_2d_lower_limit = raw_lower;
		firmware->te_params[table_num]->y_2d_upper_limit = raw_upper;
	}

	mtx_curve_set_hard_limits(MTX_CURVE(curve),
			firmware->te_params[table_num]->x_2d_lower_limit,
			firmware->te_params[table_num]->x_2d_upper_limit,
			firmware->te_params[table_num]->y_2d_lower_limit,
			firmware->te_params[table_num]->y_2d_upper_limit);

	/* One shot to get marker drawn. */
	create_rtv_value_change_watch_f(cdata->source,TRUE,"update_curve_marker",(gpointer)cdata);
	/* continuous to catch changes. */
	id = create_rtv_value_change_watch_f(cdata->source,FALSE,"update_curve_marker",(gpointer)cdata);
	OBJ_SET(curve,"marker_id",GINT_TO_POINTER(id));
	if (!embedded)
	{
		OBJ_SET(window,"widget_list",widget_list);
		OBJ_SET(window,"curve_list",curve_list);
		OBJ_SET(window,"x_entries",x_entries);
		OBJ_SET(window,"y_entries",y_entries);
	}
	/*
	else
	{
		OBJ_SET(parent,"widget_list",widget_list);
		OBJ_SET(parent,"curve_list",curve_list);
		OBJ_SET(parent,"x_entries",x_entries);
		OBJ_SET(parent,"y_entries",y_entries);
	}
	*/
	OBJ_SET(curve,"x_entries",x_entries);
	OBJ_SET(curve,"y_entries",y_entries);
	gtk_container_add(GTK_CONTAINER(curve_parent),curve);

	if (embedded)
		gtk_widget_show_all(curve);
	else
		gtk_widget_show_all(window);
	EXIT();
	return TRUE;
}


/*!
  \brief Closes a 2-D table editor and deallocates any required resources
  \param widget is the 2D table editor window containing the widget list to
  be deallocated
  \param data is unused
  \returns FALSE
  */
G_MODULE_EXPORT gboolean close_2d_editor(GtkWidget * widget, gpointer data)
{
	GList *list = NULL;

	ENTER();
	list = (GList *)OBJ_GET(widget, "widget_list");
	if (list)
	{
		g_list_foreach(list,remove_widget,NULL);
		g_list_free(list);
		list = NULL;
	}
	list = (GList *)OBJ_GET(widget, "curve_list");
	if (list)
	{
		g_list_foreach(list,clean_curve,NULL);
		g_list_free(list);
		list = NULL;
	}
	list = (GList *)OBJ_GET(widget, "gauge_list");
	if (list)
	{
		g_list_foreach(list,gauge_cleanup,NULL);
		g_list_free(list);
		list = NULL;
	}
	gtk_widget_destroy(widget);
	EXIT();
	return FALSE;
}


/*!
  \brief removes a widget pointer from the ecu_widgets list
  \param widget_ptr is the pointer to the widget to remove
  \param data is unused
  */
G_MODULE_EXPORT void remove_widget(gpointer widget_ptr, gpointer data)
{
	GList ***ecu_widgets = NULL;
	gint page = -1;
	gint offset = -1;

	ENTER();
	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	remove_from_lists_f((const gchar *)OBJ_GET(widget_ptr,"bind_to_list"),widget_ptr);
	if (!GTK_IS_ENTRY(widget_ptr))
	{
		EXIT();
		return;
	}
	page = (GINT)OBJ_GET(widget_ptr,"page");
	offset = (GINT)OBJ_GET(widget_ptr,"offset");
	if (( page >= 0 ) && (offset >= 0))
		ecu_widgets[page][offset] = g_list_remove(ecu_widgets[page][offset],widget_ptr);
	/*dealloc_widget(widget_ptr,NULL);*/
	EXIT();
	return;
}


/*!
  \brief  removes the watch that drives the gauge on the 2D editor winddow
  \param gauge_ptr is the pointer to the Gauge object
  \param data is unused
  */
G_MODULE_EXPORT void gauge_cleanup(gpointer gauge_ptr, gpointer data)
{
	GtkWidget *widget = (GtkWidget *)gauge_ptr;
	ENTER();
	if (OBJ_GET(widget, "gauge_id"))
	{
		guint id = (GINT)OBJ_GET(widget, "gauge_id");
		remove_rtv_watch_f(id);
	}
	EXIT();
	return;
}


/*!
  \brief deallocates the data that was stored within the curve object including
  the X and Y entries, the cdata and gets rid of the marker_id watch
  \param curve_ptr is the pointer to the 2D curve widget
  \param data is unused
  */
G_MODULE_EXPORT void clean_curve(gpointer curve_ptr, gpointer data)
{
	GArray *array = NULL;
	guint32 id = 0;
	CurveData *cdata = NULL;
	GtkWidget *widget = (GtkWidget *)curve_ptr;

	ENTER();
	array = (GArray *)OBJ_GET(widget, "x_entries");
	if (array)
		g_array_free(array,TRUE);
	array = (GArray *)OBJ_GET(widget, "y_entries");
	if (array)
		g_array_free(array,TRUE);
	cdata = (CurveData *)OBJ_GET(widget, "cdata");
	if (cdata)
		g_free(cdata);
	id = (guint32)(GINT)OBJ_GET(widget, "marker_id");
	if (id > 0)
		remove_rtv_watch_f(id);
	EXIT();
	return;
}


/*!
  \brief updates the curve with new data from the corresponding text entry
  This extracts the curve axis and index from the widget in order to know 
  which vertex of the curve to change
  \param widget is hte entry that was changed via the user
  \param data is a pointer to the curve to update
  \returns FALSE
  */
G_MODULE_EXPORT gboolean update_2d_curve(GtkWidget *widget, gpointer data)
{
	GtkWidget *curve = (GtkWidget *)data;
	MtxCurveCoord point;
	Axis axis;
	gint index = 0;
	gchar * text = NULL;
	gfloat tmpf = 0.0;

	ENTER();
	index = (GINT) OBJ_GET(widget,"curve_index");
	axis = (Axis)(GINT)OBJ_GET(widget,"curve_axis");
	mtx_curve_get_coords_at_index(MTX_CURVE(curve),index,&point);
	text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	tmpf = (gfloat)strtod(g_strdelimit(text,",.",'.'),NULL);
	g_free(text);
	if (axis == _X_)
		point.x = tmpf;
	else if (axis == _Y_)
		point.y = tmpf;
	else
		printf(_("ERROR in update_2d_curve()!!!\n"));
	mtx_curve_set_coords_at_index(MTX_CURVE(curve),index,point);
	EXIT();
	return FALSE;
}


/*! 
  \brief signal handler to deal with the user clicking on the curve and moving
  the vertex, which thus updates the corresponding entry
  \param curve is the pointer to the curve object
  \param data is unused
  */
G_MODULE_EXPORT void coords_changed(GtkWidget *curve, gpointer data)
{
	MtxCurveCoord point;
	gint index = 0;
	GArray *array;
	gint precision = 0;
	GtkWidget *entry = NULL;
	gchar * tmpbuf = NULL;

	ENTER();
	index = mtx_curve_get_active_coord_index(MTX_CURVE(curve));
	mtx_curve_get_coords_at_index(MTX_CURVE(curve),index,&point);

	if(!mtx_curve_get_x_axis_lock_state(MTX_CURVE(curve)))
	{
		/* X Coord */
		array = (GArray *)OBJ_GET(curve,"x_entries");
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
		array = (GArray *)OBJ_GET(curve,"y_entries");
		entry  = g_array_index(array,GtkWidget *,index);
		precision = (GINT)OBJ_GET(entry, "precision");
		tmpbuf = g_strdup_printf("%1$.*2$f",point.y,precision);
		gtk_entry_set_text(GTK_ENTRY(entry),tmpbuf);
		g_signal_emit_by_name(entry, "activate");
		g_free(tmpbuf);
	}
	EXIT();
	return;
}

 
/*!
  \brief  This handler fires when the mouse gets close to a vertex. This
  will make the point change color 
  \param curve is a pointer the the curve  object
  \param data is unused
  */
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

	ENTER();
	index = mtx_curve_get_vertex_proximity_index(MTX_CURVE(curve));
	x_array = (GArray *)OBJ_GET(curve,"x_entries");
	y_array = (GArray *)OBJ_GET(curve,"y_entries");
	if ((!x_array) || (!y_array))
	{
		EXIT();
		return;
	}
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
			EXIT();
			return;
		}
		else	/* Last IS defined, thus check for polarity */
			last = (GINT)OBJ_GET(curve,"last_proximity_vertex");
		if (last < 0)
		{
			OBJ_SET(curve, "last_proximity_vertex",GINT_TO_POINTER(index+1));
			EXIT();
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
			EXIT();
			return;
		}
		else	/* Last IS defined, thus check for polarity */
			last = (GINT)OBJ_GET(curve,"last_proximity_vertex");
		if (last < 0)
		{
			OBJ_SET(curve, "last_proximity_vertex",GINT_TO_POINTER(index+1));
			EXIT();
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

	EXIT();
	return;
}


/*!
  \brief  This handler fires when the marker gets close to a vertex. This
  will make the point change color 
  \param curve is a pointer the the curve object
  \param data is unused
  */
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

	ENTER();
	index = mtx_curve_get_marker_proximity_index(MTX_CURVE(curve));
	m_index = mtx_curve_get_vertex_proximity_index(MTX_CURVE(curve));
	x_array = (GArray *)OBJ_GET(curve,"x_entries");
	y_array = (GArray *)OBJ_GET(curve,"y_entries");
	if ((!x_array) || (!y_array))
	{
		EXIT();
		return;
	}
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
			EXIT();
			return;
		}
		else	/* Last IS defined, thus check for polarity */
			last = (GINT)OBJ_GET(curve,"last_marker_vertex");
		if (last < 0)
		{
			OBJ_SET(curve, "last_marker_vertex",GINT_TO_POINTER(index+1));
			EXIT();
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
			EXIT();
			return;
		}
		else	/* Last IS defined, thus check for polarity */
			last = (GINT)OBJ_GET(curve,"last_marker_vertex");
		if (last < 0)
		{
			OBJ_SET(curve, "last_marker_vertex",GINT_TO_POINTER(index+1));
			EXIT();
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

	EXIT();
	return;
}


/*!
  \brief handler attached te the 2d table editor window to close the window
  \param widget is the menu handler item
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean close_menu_handler(GtkWidget * widget, gpointer data)
{
	ENTER();
	close_2d_editor((GtkWidget *)OBJ_GET(widget,"window"),NULL);
	EXIT();
	return TRUE;
}


/*!
  \brief This is the watch triggered function to update the curve markers
  when their source value changes
  \param watch is a pointer to the DateWatch structure
  \see RtvWatch
  */
G_MODULE_EXPORT void update_curve_marker(RtvWatch *watch)
{
	CurveData *cdata = (CurveData *)watch->user_data;
	ENTER();
	if (!MTX_IS_CURVE(cdata->curve))
	{
		remove_rtv_watch_f(watch->id);
		EXIT();
		return;
	}
	if (cdata->axis == _X_)
		mtx_curve_set_x_marker_value(MTX_CURVE(cdata->curve),watch->val);
	if (cdata->axis == _Y_)
		mtx_curve_set_y_marker_value(MTX_CURVE(cdata->curve),watch->val);
	EXIT();
	return;
}


/*!
  \brief Handler to lock/unlock the X or Y axis of the curve from being edited
  \param widget is the pointer to the togglebutton the user flipped
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean set_axis_locking(GtkWidget *widget, gpointer data)
{
	Axis axis = (Axis)(GINT)OBJ_GET(widget,"axis");
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	ENTER();
	if (state)
		gtk_button_set_label(GTK_BUTTON(widget)," Locked ");
	else
		gtk_button_set_label(GTK_BUTTON(widget),"Unlocked");
	if (axis == _X_)
		mtx_curve_set_x_axis_lock_state(MTX_CURVE(data),state);
	if (axis == _Y_)
		mtx_curve_set_y_axis_lock_state(MTX_CURVE(data),state);
	EXIT();
	return TRUE;
}


/*!
  \brief handler to create a 2D table, extracts the table_number from the  
  widget and calls the function to create the 2d table  editor
  \param widget is the pointer to the button the user clicked on
  \returns TRUE on success, FALSE on failure
  */
G_MODULE_EXPORT gboolean add_2d_table(GtkWidget *widget)
{
	gint table_num = 0;
	ENTER();
	g_return_val_if_fail(GTK_IS_WIDGET(widget),FALSE);

	table_num = (gint)g_ascii_strtod((gchar *)OBJ_GET(widget,"te_table_num"),NULL);
	create_2d_table_editor(table_num,widget);
	EXIT();
	return TRUE;
}


/*!
  \brief Highlights the corresponding text entry related to the curve
  vertex
  \param widget is the pointer to the text entry in question
  \param color is the pointer to the color to set the entry to
  */
G_MODULE_EXPORT void highlight_entry(GtkWidget *widget, GdkColor *color)
{
	cairo_t *cr = NULL;
	gint w = 0;
	gint h = 0;
	GdkWindow *window = NULL;
	GdkColor white = {0,65535,65535,65535};
	window = gtk_entry_get_text_window(GTK_ENTRY(widget));
	ENTER();
	if (GDK_IS_DRAWABLE(window))
	{
		cr = gdk_cairo_create(window);
		if (color)
			gdk_cairo_set_source_color(cr,color);
		else
		{
			if (OBJ_GET(widget, "use_color"))
				gdk_cairo_set_source_color(cr,&gtk_widget_get_style(widget)->base[GTK_STATE_NORMAL]);
			else
				gdk_cairo_set_source_color(cr,&white);
		}
		cairo_set_line_width(cr,2);
#if GTK_MINOR_VERSION < 24
		gdk_drawable_get_size(window, &w, &h);
#else
		w = gdk_window_get_width(window);
		h = gdk_window_get_height(window);
#endif
		cairo_rectangle(cr,1,1,w-2,h-2);
		cairo_stroke(cr);
		cairo_destroy(cr);
	}
	EXIT();
	return;
}
