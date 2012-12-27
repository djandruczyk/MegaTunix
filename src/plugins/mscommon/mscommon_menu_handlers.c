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
  \file src/plugins/mscommon/mscommon_menu_handlers.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS Common menu init/handler functions
  \author David Andruczyk
  */

#include <config.h>
#include <datamgmt.h>
#include <firmware.h>
#include <gtk/gtk.h>
#include <mscommon_gui_handlers.h>
#include <mscommon_menu_handlers.h>
#include <mscommon_plugin.h>

extern gconstpointer *global_data;


/*!
  \brief Sets up the Gui menu and calls the ECU specific menu init function
  \param xml is the core gui XML
  */
G_MODULE_EXPORT void common_plugin_menu_setup(GladeXML *xml)
{
	void (*ecu_plugin_menu_setup)(GladeXML *) = NULL;
	GtkWidget *menu = NULL;
	GtkWidget *item = NULL;
	GtkWidget *image = NULL;

	ENTER();
	/* View->Tabs Menu */
	menu = glade_xml_get_widget (xml, "goto_tab1_menu");
	item = gtk_menu_item_new_with_mnemonic("_Boost Tables");
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(jump_to_tab_f),NULL);
	OBJ_SET(item,"target_tab",GINT_TO_POINTER(BOOSTTABLES_TAB));
	if (!check_tab_existance_f(BOOSTTABLES_TAB))
		gtk_widget_set_sensitive(item,FALSE);
	else
		gtk_widget_set_sensitive(item,TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_menu_item_new_with_mnemonic("_Staging Tables");
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(jump_to_tab_f),NULL);
	OBJ_SET(item,"target_tab",GINT_TO_POINTER(STAGING_TAB));
	if (!check_tab_existance_f(STAGING_TAB))
		gtk_widget_set_sensitive(item,FALSE);
	else
		gtk_widget_set_sensitive(item,TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_menu_item_new_with_mnemonic("_Rotary Tables");
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(jump_to_tab_f),NULL);
	OBJ_SET(item,"target_tab",GINT_TO_POINTER(ROTARYTABLES_TAB));
	if (!check_tab_existance_f(ROTARYTABLES_TAB))
		gtk_widget_set_sensitive(item,FALSE);
	else
		gtk_widget_set_sensitive(item,TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	gtk_widget_show_all(menu);

	/* View Menu */
	menu = glade_xml_get_widget (xml, "view_menu_menu");
	item = gtk_image_menu_item_new_with_mnemonic("ECU _Errors");
	image = gtk_image_new_from_stock("gtk-stop",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(jump_to_tab_f),NULL);
	OBJ_SET(item,"target_tab",GINT_TO_POINTER(ERROR_STATUS_TAB));
	if (!check_tab_existance_f(ERROR_STATUS_TAB))
		gtk_widget_set_sensitive(item,FALSE);
	else
		gtk_widget_set_sensitive(item,TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	gtk_widget_show_all(menu);

	/* Tuning Menu */
	menu = glade_xml_get_widget (xml, "generate1_menu");
	item = gtk_menu_item_new_with_mnemonic("_Ignition Map");
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(show_create_ignition_map_window),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	gtk_widget_show_all(menu);

	/* Tools Menu */
	menu = glade_xml_get_widget (xml, "tools_menu_menu");
	item = gtk_image_menu_item_new_with_mnemonic("Trigger _Offset Tool");
	image = gtk_image_new_from_stock("gtk-justify-fill",GTK_ICON_SIZE_MENU);
	g_object_set(item,"image",image,NULL);
	if (gtk_minor_version >= 16)
		g_object_set(item,"always-show-image",TRUE,NULL);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(show_trigger_offset_window),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	gtk_widget_show_all(menu);

	if (get_symbol_f("ecu_plugin_menu_setup",(void **)&ecu_plugin_menu_setup))
		ecu_plugin_menu_setup(xml);
	EXIT();
	return;
}


/*!
  \brief Creates an ignition map
  \param widget is the buttonthe user clicked on to create the map
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean create_ignition_map(GtkWidget *widget, gpointer data)
{
	GladeXML* xml = NULL;
	GtkWidget* item = NULL;
	gint i = 0;
	gint x = 0;
	gint y = 0;
	gint table = 0;
	gchar *table_title = NULL;
	Firmware_Details *firmware = NULL;
	gint canID = 0;
	gdouble x_bin[64], y_bin[64];
	gint page = 0;
	gint base = 0;
	DataSize size = MTX_U08;
	gint mult = 0;
	gint raw = 0;
	gdouble light_advance = 0.0;
	gdouble idle_rpm = 0.0;
	gdouble idle_load = 0.0;
	gdouble idle_advance = 0.0;
	gdouble peak_torque_rpm = 0.0;
	gdouble peak_torque_load = 0.0;
	gdouble peak_torque_advance = 0.0;
	gdouble advance = 0.0;
	gdouble maximum_rpm_advance = 0.0;
	gdouble maximum_retard = 0.0;
	gdouble retard_start_load = 0.0;
	gfloat *multiplier = NULL;
	gfloat *adder = NULL;
	GList ***ecu_widgets = NULL;
	GList *list = NULL;
	gchar * tmpbuf = NULL;
	gint precision = 0;
	GtkTreeIter iter;
	GtkTreeModel *model = NULL;

	ENTER();
	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	canID = firmware->canID;

	xml = glade_get_widget_tree (GTK_WIDGET (widget));

	/* find the combo box */
	item = glade_xml_get_widget (xml, "spark_table_combo");
	if (!item)
	{
		MTXDBG(CRITICAL,_("Unable to find spark table combo! where am I?\n"));
		EXIT();
		return TRUE;
	}

	/* the selected text in the combo box */
	if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(item),&iter))
	{
		MTXDBG(CRITICAL,_("Table was not found!\n"));
		EXIT();
		return TRUE;
	}
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(item));
	gtk_tree_model_get(model,&iter,1,&table,-1);


	/* light load */
	item = glade_xml_get_widget (xml, "light_advance");
	light_advance = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	/* idle */
	item = glade_xml_get_widget (xml, "idle_rpm");
	idle_rpm = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	item = glade_xml_get_widget (xml, "idle_advance");
	idle_advance = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	item = glade_xml_get_widget (xml, "idle_load");
	idle_load = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	/* peak torque */
	item = glade_xml_get_widget (xml, "peak_torque_rpm");
	peak_torque_rpm = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	item = glade_xml_get_widget (xml, "peak_torque_advance");
	peak_torque_advance = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	item = glade_xml_get_widget (xml, "peak_torque_load");
	peak_torque_load = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	/* extra advance at tables maximum RPM */
	item = glade_xml_get_widget (xml, "maximum_rpm_advance");
	maximum_rpm_advance = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	/* retard */
	item = glade_xml_get_widget (xml, "maximum_retard");
	maximum_retard = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	item = glade_xml_get_widget (xml, "retard_start_load");
	retard_start_load = strtod(gtk_entry_get_text(GTK_ENTRY(item)), NULL);

	/* build a copy of the x and y bins */
	/* Find bin corresponding to rpm  */
	page = firmware->table_params[table]->x_page;
	base = firmware->table_params[table]->x_base;
	size = firmware->table_params[table]->x_size;
	mult = get_multiplier_f(size);

	multiplier = firmware->table_params[table]->x_fromecu_mult;
	adder = firmware->table_params[table]->x_fromecu_add;

	/* fetch us a copy of the x bins */
	for (i=0; i != firmware->table_params[table]->x_bincount; i++)
	{
		raw = ms_get_ecu_data(canID, page, base+(i*mult), size);
		if ((multiplier) && (adder))
			x_bin[i] = (raw * (*multiplier)) + (*adder);
		else if (multiplier)
			x_bin[i] = (raw * (*multiplier));
		else
			x_bin[i] = raw;
	}

	/* Find bin corresponding to load  */
	page = firmware->table_params[table]->y_page;
	base = firmware->table_params[table]->y_base;
	size = firmware->table_params[table]->y_size;
	mult = get_multiplier_f(size);

	multiplier = firmware->table_params[table]->y_fromecu_mult;
	adder = firmware->table_params[table]->y_fromecu_add;

	/* fetch us a copy of the y bins */
	for (i=0; i != firmware->table_params[table]->y_bincount; i++)
	{
		raw = ms_get_ecu_data(canID, page, base+(i*mult), size);
		if ((multiplier) && (adder))
			y_bin[i] = (raw * (*multiplier)) + (*adder);
		else if (multiplier)
			y_bin[i] = (raw * (*multiplier));
		else
			y_bin[i] = raw;
	}

	page = firmware->table_params[table]->z_page;
	base = firmware->table_params[table]->z_base;
	size = firmware->table_params[table]->z_size;
	mult = get_multiplier_f(size);


	/* generate a spark table :) */
	for (y=0; y != firmware->table_params[table]->y_bincount; y++)
	{
		for (x=0; x != firmware->table_params[table]->x_bincount; x++)
		{

			/* idle -> peak torque rpm, advance interpolate */
			advance = linear_interpolate(x_bin[x], idle_rpm, peak_torque_rpm, idle_advance, peak_torque_advance);

			/* maximum extra advance? */
			if (x_bin[x] > peak_torque_rpm)
				advance += linear_interpolate(x_bin[x], peak_torque_rpm, x_bin[firmware->table_params[table]->x_bincount-1], 0, maximum_rpm_advance);

			/* low load advance */
			/* high load retard */
			if (y_bin[y] > retard_start_load)
				advance -= linear_interpolate(y_bin[y], retard_start_load, y_bin[firmware->table_params[table]->y_bincount-1], 0, maximum_retard);

			/* HACK ALERT,  this assumes the list at
			 * ecu_widgets[page][offset], contains the VEtable widget at
			 * offset 0 of that list.  (assumptions are bad practice!)
			 */
			if (firmware->capabilities & PIS)
				list = ecu_widgets[firmware->table_params[table]->z_page][firmware->table_params[table]->z_base + ((x * firmware->table_params[table]->y_bincount) * mult) + (y * mult)];
			else
				list = ecu_widgets[firmware->table_params[table]->z_page][firmware->table_params[table]->z_base + ((y * firmware->table_params[table]->y_bincount) * mult) + (x * mult)];
			widget = (GtkWidget *)g_list_nth_data(list,0);
			precision = (GINT)OBJ_GET(widget, "precision");
			tmpbuf = g_strdup_printf("%1$.*2$f", advance, precision);
			gtk_entry_set_text(GTK_ENTRY(widget), tmpbuf);
			g_free(tmpbuf);

			std_entry_handler_f(GTK_WIDGET(widget), NULL);
		}
	}

	gtk_widget_hide(glade_xml_get_widget (xml, "create_ignition_window"));
	EXIT();
	return TRUE;
}


/*!
  \brief shows/hides  the ignition map window populates the combo box with 
  available spark maps that could be filled out by the generator
  \param widget is unused
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean show_create_ignition_map_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	Firmware_Details *firmware;
	GtkListStore *store = NULL;
	GtkTreeIter iter;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!main_xml) || (DATA_GET(global_data,"leaving")))
	{
		EXIT();
		return TRUE;
	}

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"create_ignition_window",NULL);
		window = glade_xml_get_widget(xml,"create_ignition_window");
		item = glade_xml_get_widget(xml,"spark_table_combo");
		glade_xml_signal_autoconnect(xml);
		store = gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_INT);

		for (gint t=0;t<firmware->total_tables;t++)
		{
			if (firmware->table_params[t]->is_spark)
			{
				gtk_list_store_append(store,&iter);
				gtk_list_store_set(store,&iter,0,g_strdup(firmware->table_params[t]->table_name),1,t,-1);
			}
		}
		gtk_combo_box_set_model(GTK_COMBO_BOX(item),GTK_TREE_MODEL(store));
#if GTK_MINOR_VERSION < 24
		gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(item),0);
#else
		gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(item),0);
#endif

		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget_f("main_window")));
		gtk_widget_show_all(GTK_WIDGET(window));
		EXIT();
		return TRUE;
	}

#if GTK_MINOR_VERSION >=18
	if (gtk_widget_get_visible(GTK_WIDGET(window)))
#else
	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(window)))
#endif
		gtk_widget_hide(GTK_WIDGET(window));
	else
		gtk_widget_show_all(GTK_WIDGET(window));
	EXIT();
	return TRUE;
}


/*
  \brief non extrapolating linear dual line interpolation (3 times fast 
  after 6 beers :-) 
  \param offset is used to prevent extrapolation
  \param slope1_a
  \param slope1_b
  \param slope2_a
  \param slope2_b
  \returns the linear interpolation between the points
  */
G_MODULE_EXPORT gdouble linear_interpolate(gdouble offset, gdouble slope1_a, gdouble slope1_b, gdouble slope2_a, gdouble slope2_b)
{
	gdouble slope1, slope2, result;
	gdouble ratio;

	ENTER();
	/* prevent extrapolation */
	if (offset <= slope1_a) 
	{
		EXIT();
		return slope2_a;
	}
	if (offset >= slope1_b) 
	{
		EXIT();
		return slope2_b;
	}

	offset -= slope1_a;
	slope1 = slope1_b - slope1_a;

	if (slope2_a > slope2_b)
		slope2 = slope2_a - slope2_b;
	else 
		slope2 = slope2_b - slope2_a;

	ratio = (gdouble) offset / slope1;
	result = ((gdouble)slope2_a * (1-ratio) + (gdouble)slope2_b * ratio);
	EXIT();
	return result;
}



/*!
 \brief General purpose handler to hide/show trigger offset window
 \param widget is the button
 \param data is unused
 \returns TRUE on success, FALSE otherwise
 */
G_MODULE_EXPORT gboolean show_trigger_offset_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	static Firmware_Details *firmware = NULL;
	GtkWidget *item = NULL;
	GtkWidget *partner = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	GList ***ecu_widgets = NULL;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");

	g_return_val_if_fail(firmware,FALSE);
	g_return_val_if_fail(ecu_widgets,FALSE);
	g_return_val_if_fail(main_xml,FALSE);

	if ((DATA_GET(global_data,"leaving")))
	{
		EXIT();
		return TRUE;
	}

	if (!GTK_IS_WIDGET(window))
	{
		gint page = 0;
		gint offset = 0;
		xml = glade_xml_new(main_xml->filename,"trigger_offset_window",NULL);
		window = glade_xml_get_widget(xml,"trigger_offset_window");
		glade_xml_signal_autoconnect(xml);

		item = glade_xml_get_widget(xml,"plus_button");
		register_widget_f("plus_button",item);
		if (firmware->capabilities & MS1_E)
			OBJ_SET(item,"partner_widget",lookup_widget_f("WD_trim_angle_entry"));
		else /* MS2 */
			OBJ_SET(item,"partner_widget",lookup_widget_f("IGN_trigger_offset_entry"));
		OBJ_SET(item,"handler",GINT_TO_POINTER(INCREMENT_VALUE));
		OBJ_SET(item,"amount",GINT_TO_POINTER(5));

		item = glade_xml_get_widget(xml,"minus_button");
		register_widget_f("minus_button",item);
		if (firmware->capabilities & MS1_E)
			OBJ_SET(item,"partner_widget",lookup_widget_f("WD_trim_angle_entry"));
		else /* MS2 */
			OBJ_SET(item,"partner_widget",lookup_widget_f("IGN_trigger_offset_entry"));
		OBJ_SET(item,"handler",GINT_TO_POINTER(DECREMENT_VALUE));
		OBJ_SET(item,"amount",GINT_TO_POINTER(5));

		item = glade_xml_get_widget(xml,"advance_parent_box");
		OBJ_SET_FULL(item,"ctrl_name",g_strdup("trigger_offset_tool_advance_rtt"),g_free);
		OBJ_SET_FULL(item,"source",g_strdup("sparkangle"),g_free);
		OBJ_SET_FULL(item,"label_prefix",g_strdup("<span font_desc=\"Sans 64\">"),g_free);
		OBJ_SET_FULL(item,"label_suffix",g_strdup("</span>"),g_free);
		OBJ_SET(item,"markup",GINT_TO_POINTER(TRUE));
		add_additional_rtt_f(item);

		item = glade_xml_get_widget(xml,"offset_entry");
		g_return_val_if_fail(GTK_IS_WIDGET(item),FALSE);
		register_widget_f("offset_entry",item);
		if (firmware->capabilities & MS1_E)
			partner = lookup_widget_f("WD_trim_angle_entry");
		else /* MS2 */
			partner = lookup_widget_f("IGN_trigger_offset_entry");

		g_return_val_if_fail(GTK_IS_WIDGET(partner),FALSE);
		page = (GINT)OBJ_GET(partner,"page");
		offset = (GINT)OBJ_GET(partner,"offset");

		OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"page",GINT_TO_POINTER(page));
		OBJ_SET(item,"offset",GINT_TO_POINTER(offset));
		OBJ_SET(item,"precision",OBJ_GET(partner,"precision"));
		OBJ_SET(item,"size",OBJ_GET(partner,"size"));
		OBJ_SET(item,"raw_lower",OBJ_GET(partner,"raw_lower"));
		OBJ_SET(item,"raw_upper",OBJ_GET(partner,"raw_upper"));
		OBJ_SET(item,"fromecu_mult",OBJ_GET(partner,"fromecu_mult"));
		OBJ_SET(item,"fromecu_add",OBJ_GET(partner,"fromecu_add"));
		ecu_widgets[page][offset] = g_list_prepend(ecu_widgets[page][offset],(gpointer)item);
		/* Force them to update */
		g_list_foreach(ecu_widgets[page][offset],update_widget,NULL);

		item = glade_xml_get_widget(xml,"burn_data_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(BURN_FLASH));
		OBJ_SET_FULL(item,"bind_to_list",g_strdup("burners"),g_free);
		bind_to_lists_f(item,"burners");
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget_f("main_window")));
		gtk_widget_show_all(GTK_WIDGET(window));
		EXIT();
		return TRUE;
	}
#if GTK_MINOR_VERSION >= 18
	if (gtk_widget_get_visible(GTK_WIDGET(window)))
#else
	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(window)))
#endif
		gtk_widget_hide(GTK_WIDGET(window));
	else
		gtk_widget_show_all(GTK_WIDGET(window));


	EXIT();
	return TRUE;
}

