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
  \file src/plugins/ms2/ms2_menu_handlers.c
  \ingroup MS2Plugin,Plugins
  \brief MS2 Plugin specific menu handlers
  \author David Andruczyk
  */

#include <config.h>
#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <ms2_gui_handlers.h>
#include <ms2_menu_handlers.h>
#include <ms2_plugin.h>

extern gconstpointer *global_data;


/*!
  \brief Setups up MS2 specific menu entries on the main gui 
  \param xml is the pointer to the core gui XML
  */
G_MODULE_EXPORT void ecu_plugin_menu_setup(GladeXML *xml)
{
	Firmware_Details *firmware = NULL;
	GtkWidget *menu = NULL;
	GtkWidget *item = NULL;
	GtkWidget *image = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (firmware->capabilities & MS2)
	{
		menu = glade_xml_get_widget (xml, "tools_menu_menu");
		item = gtk_image_menu_item_new_with_mnemonic("Battery _Voltage Calibration");
		image = gtk_image_new_from_stock("gtk-preferences",GTK_ICON_SIZE_MENU);
		g_object_set(item,"image",image,NULL);
		if (gtk_minor_version >= 16)
			g_object_set(item,"always-show-image",TRUE,NULL);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(show_battery_calibrator_window),NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		
		item = gtk_image_menu_item_new_with_mnemonic("Sensor _Calibration (MAP/Baro)");
		image = gtk_image_new_from_stock("gtk-preferences",GTK_ICON_SIZE_MENU);
		g_object_set(item,"image",image,NULL);
		if (gtk_minor_version >= 16)
			g_object_set(item,"always-show-image",TRUE,NULL);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(show_sensor_calibrator_window),NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

		item = gtk_image_menu_item_new_with_mnemonic("Calibrate Ther_mistor Tables");
		image = gtk_image_new_from_stock("gtk-preferences",GTK_ICON_SIZE_MENU);
		g_object_set(item,"image",image,NULL);
		if (gtk_minor_version >= 16)
			g_object_set(item,"always-show-image",TRUE,NULL);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(show_ms2_therm_table_generator_window),NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

		/*
		item = gtk_image_menu_item_new_with_mnemonic("Calibrate _MAF Tables");
		image = gtk_image_new_from_stock("gtk-preferences",GTK_ICON_SIZE_MENU);
		g_object_set(item,"image",image,NULL);
		if (gtk_minor_version >= 16)
			g_object_set(item,"always-show-image",TRUE,NULL);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(show_ms2_maf_table_generator_window),NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		*/

		item = gtk_image_menu_item_new_with_mnemonic("MS-2 _AFR Calibrator");
		image = gtk_image_new_from_stock("gtk-preferences",GTK_ICON_SIZE_MENU);
		g_object_set(item,"image",image,NULL);
		if (gtk_minor_version >= 16)
			g_object_set(item,"always-show-image",TRUE,NULL);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(show_ms2_afr_calibrator_window),NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

		item = gtk_image_menu_item_new_with_mnemonic("MS-2 _TPS Calibrator");
		image = gtk_image_new_from_stock("gtk-preferences",GTK_ICON_SIZE_MENU);
		g_object_set(item,"image",image,NULL);
		if (gtk_minor_version >= 16)
			g_object_set(item,"always-show-image",TRUE,NULL);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(show_tps_calibrator_window),NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

		item = gtk_image_menu_item_new_with_mnemonic("MS-2 Re-Init (WarmBoot)");
		image = gtk_image_new_from_stock("gtk-redo",GTK_ICON_SIZE_MENU);
		g_object_set(item,"image",image,NULL);
		if (gtk_minor_version >= 16)
			g_object_set(item,"always-show-image",TRUE,NULL);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(ms2_reinit),NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

		item = gtk_image_menu_item_new_with_mnemonic("MS-2 ReBoot (ColdBoot)");
		image = gtk_image_new_from_stock("gtk-refresh",GTK_ICON_SIZE_MENU);
		g_object_set(item,"image",image,NULL);
		if (gtk_minor_version >= 16)
			g_object_set(item,"always-show-image",TRUE,NULL);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(ms2_reboot),NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		gtk_widget_show_all(menu);
	}
	EXIT();
	return;
}


/*!
   \brief General purpose handler to hide/show Sensor calibrate window
   \param widget is a pointer to the widget clicked
   \param data is unused
   \returns TRUE
    */
G_MODULE_EXPORT gboolean show_ms2_therm_table_generator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	static GtkWidget *chooser = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	g_return_val_if_fail(firmware,FALSE);
	g_return_val_if_fail(main_xml,FALSE);
	
	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return TRUE;
	}

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"table_generator_window",NULL);
		window = glade_xml_get_widget(xml,"table_generator_window");
		glade_xml_signal_autoconnect(xml);

		/* Default to params not a file */
		chooser = glade_xml_get_widget(xml,"import_filechooser_button");
		register_widget_f("import_filechooser_button",chooser);
		item = glade_xml_get_widget(xml,"use_params_rbutton");
		register_widget_f("use_params_rbutton",item);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(item),TRUE);
		g_signal_emit_by_name(item,"toggled",NULL);

		item = glade_xml_get_widget(xml,"sensor_combo");
		gtk_combo_box_set_active(GTK_COMBO_BOX(item),0);
		register_widget_f("thermister_sensor_combo",item);
		item = glade_xml_get_widget(xml,"temp_label");
		register_widget_f("temp_label",item);
		if (firmware->capabilities & PIS)
			gtk_widget_destroy(glade_xml_get_widget(xml,"bias_resistor_table"));
		else
		{
			item = glade_xml_get_widget(xml,"bias_entry");
			register_widget_f("bias_entry",item);
			OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
			OBJ_SET_FULL(item,"raw_upper",g_strdup("100000"),g_free);
			OBJ_SET(item,"precision",GINT_TO_POINTER(1));
		}

		item = glade_xml_get_widget(xml,"temp1_entry");
		register_widget_f("temp1_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("-40"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("300"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"temp2_entry");
		register_widget_f("temp2_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("-40"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("300"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"temp3_entry");
		register_widget_f("temp3_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("-40"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("300"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"resistance1_entry");
		register_widget_f("resistance1_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("500000"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"resistance2_entry");
		register_widget_f("resistance2_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("500000"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"resistance3_entry");
		register_widget_f("resistance3_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("500000"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"celsius_rbutton");
		OBJ_SET_FULL(item,"temp_label",g_strdup("Temperature(\302\260 C)"),g_free);
		register_widget_f("thermister_celsius_rbutton",item);
		item = glade_xml_get_widget(xml,"fahrenheit_rbutton");
		OBJ_SET_FULL(item,"temp_label",g_strdup("Temperature(\302\260 F)"),g_free);
		register_widget_f("thermister_fahrenheit_rbutton",item);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(item),TRUE);
		g_signal_emit_by_name(item,"toggled",NULL);
		item = glade_xml_get_widget(xml,"kelvin_rbutton");
		OBJ_SET_FULL(item,"temp_label",g_strdup("Temperature(\302\260 K)"),g_free);
		register_widget_f("thermister_kelvin_rbutton",item);
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget_f("main_window")));
		gtk_widget_show(GTK_WIDGET(window));
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
		{
			gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(chooser));
			gtk_widget_show(GTK_WIDGET(window));
		}
	EXIT();
	return TRUE;
}


/*!
 \brief General purpose handler to hide/show Sensor calibrate window
 \param widget is a pointer to the widget clicked
 \param data is unused
 \returns TRUE
 */
G_MODULE_EXPORT gboolean show_ms2_afr_calibrator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GtkWidget *item2 = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	Firmware_Details *firmware = NULL;
	gboolean (*populate_afr_calibrator_combo_f)(GtkWidget *) = NULL;

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
		xml = glade_xml_new(main_xml->filename,"ms2_afr_calibrator_window",NULL);
		window = glade_xml_get_widget(xml,"ms2_afr_calibrator_window");
		glade_xml_signal_autoconnect(xml);

		item = glade_xml_get_widget(xml,"ego_sensor_combo");
		register_widget_f("afr_calibrate_ego_sensor_combo",item);
		if (get_symbol_f("populate_afr_calibrator_combo",(void **)&populate_afr_calibrator_combo_f))
			populate_afr_calibrator_combo_f(item);
		item2 = glade_xml_get_widget(xml,"generic_wideband_frame");
		OBJ_SET(item,"generic_controls",item2);

		item = glade_xml_get_widget(xml,"voltage1_entry");
		register_widget_f("voltage1_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("5"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"voltage2_entry");
		register_widget_f("voltage2_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("5"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"afr1_entry");
		register_widget_f("afr1_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("99"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"afr2_entry");
		register_widget_f("afr2_entry",item);
		OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("99"),g_free);
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget_f("main_window")));
		gtk_widget_show(GTK_WIDGET(window));
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
		gtk_widget_show(GTK_WIDGET(window));
	EXIT();
	return TRUE;
}


/*!
 \brief General purpose handler to hide/show Sensor calibrate window
 \param widget is a pointer to the widget clicked
 \param data is unused
 \returns TRUE
 */
G_MODULE_EXPORT gboolean show_sensor_calibrator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	gfloat *tmpf = NULL;
	Firmware_Details *firmware = NULL;
	GList ***ecu_widgets;
	void (*update_widget_f)(gpointer, gpointer) = NULL;

	ENTER();
	if (!update_widget_f)
		get_symbol_f("update_widget",(void **)&update_widget_f);

	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!main_xml) || (DATA_GET(global_data,"leaving")))
	{
		EXIT();
		return TRUE;
	}

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"sensor_calibration_window",NULL);
		window = glade_xml_get_widget(xml,"sensor_calibration_window");

		item = glade_xml_get_widget(xml,"map0_entry");
		register_widget_f("map0_entry",item);
		if (firmware->capabilities & PIS)
		{
			OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
			OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
			OBJ_SET_FULL(item,"raw_upper",g_strdup("600"),g_free);
			OBJ_SET(item,"page",GINT_TO_POINTER(0));
			OBJ_SET(item,"offset",GINT_TO_POINTER(2702));
			OBJ_SET(item,"size",GINT_TO_POINTER(MTX_U16));
			ecu_widgets[0][2702] = g_list_prepend(
					ecu_widgets[0][2702],
					(gpointer)item);
		}
		else
		{
			OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
			OBJ_SET_FULL(item,"raw_lower",g_strdup("-1000"),g_free);
			OBJ_SET_FULL(item,"raw_upper",g_strdup("32767"),g_free);
			OBJ_SET(item,"page",GINT_TO_POINTER(0));
			OBJ_SET(item,"offset",GINT_TO_POINTER(506));
			OBJ_SET(item,"precision",GINT_TO_POINTER(1));
			OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
			tmpf = g_new0(gfloat, 1);
			*tmpf = 0.1;
			OBJ_SET_FULL(item,"fromecu_mult",tmpf,g_free);
			ecu_widgets[0][506] = g_list_prepend(
					ecu_widgets[0][506],
					(gpointer)item);
		}

		item = glade_xml_get_widget(xml,"map5_entry");
		register_widget_f("map5_entry",item);
		if (firmware->capabilities & PIS)
		{
			OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
			OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
			OBJ_SET_FULL(item,"raw_upper",g_strdup("600"),g_free);
			OBJ_SET(item,"page",GINT_TO_POINTER(0));
			OBJ_SET(item,"offset",GINT_TO_POINTER(2704));
			OBJ_SET(item,"size",GINT_TO_POINTER(MTX_U16));
			ecu_widgets[0][2704] = g_list_prepend(
					ecu_widgets[0][2704],
					(gpointer)item);
		}
		else
		{
			OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
			OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
			OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
			OBJ_SET_FULL(item,"raw_lower",g_strdup("-1000"),g_free);
			OBJ_SET_FULL(item,"raw_upper",g_strdup("32767"),g_free);
			OBJ_SET(item,"page",GINT_TO_POINTER(0));
			OBJ_SET(item,"offset",GINT_TO_POINTER(508));
			OBJ_SET(item,"precision",GINT_TO_POINTER(1));
			OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
			tmpf = g_new0(gfloat, 1);
			*tmpf = 0.1;
			OBJ_SET_FULL(item,"fromecu_mult",tmpf,g_free);
			ecu_widgets[0][508] = g_list_prepend(
					ecu_widgets[0][508],
					(gpointer)item);
		}

		item = glade_xml_get_widget(xml,"baro0_entry");
		register_widget_f("baro0_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET_FULL(item,"raw_lower",g_strdup("-32767"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("32767"),g_free);
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(530));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		tmpf = g_new0(gfloat, 1);
		*tmpf = 0.1;
		OBJ_SET_FULL(item,"fromecu_mult",tmpf,g_free);
		ecu_widgets[0][530] = g_list_prepend(
				ecu_widgets[0][530],
				(gpointer)item);

		if (firmware->capabilities & PIS)
			gtk_widget_destroy(item);

		item = glade_xml_get_widget(xml,"baro5_entry");
		register_widget_f("baro5_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET_FULL(item,"raw_lower",g_strdup("-32767"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("32767"),g_free);
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(532));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		tmpf = g_new0(gfloat, 1);
		*tmpf = 0.1;
		OBJ_SET_FULL(item,"fromecu_mult",tmpf,g_free);
		ecu_widgets[0][532] = g_list_prepend(
				ecu_widgets[0][532],
				(gpointer)item);

		item = glade_xml_get_widget(xml,"bcor0_entry");
		register_widget_f("bcor0_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET_FULL(item,"raw_lower",g_strdup("-3276"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("3276"),g_free);
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(534));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		ecu_widgets[0][534] = g_list_prepend(
				ecu_widgets[0][534],
				(gpointer)item);

		item = glade_xml_get_widget(xml,"bcormult_entry");
		register_widget_f("bcormult_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET_FULL(item,"raw_lower",g_strdup("-200"),g_free);
		OBJ_SET_FULL(item,"raw_upper",g_strdup("200"),g_free);
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(536));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		ecu_widgets[0][536] = g_list_prepend(
				ecu_widgets[0][536],
				(gpointer)item);

		if (firmware->capabilities & PIS)
		{
			gtk_widget_destroy(item);
			g_list_foreach(ecu_widgets[0][2702],update_widget_f,NULL);
			g_list_foreach(ecu_widgets[0][2704],update_widget_f,NULL);
		}
		else
		{
			/* Force them to update */
			g_list_foreach(ecu_widgets[0][506],update_widget_f,NULL);
			g_list_foreach(ecu_widgets[0][508],update_widget_f,NULL);
			g_list_foreach(ecu_widgets[0][530],update_widget_f,NULL);
			g_list_foreach(ecu_widgets[0][532],update_widget_f,NULL);
			g_list_foreach(ecu_widgets[0][534],update_widget_f,NULL);
			g_list_foreach(ecu_widgets[0][536],update_widget_f,NULL);
		}

		item = glade_xml_get_widget(xml,"get_data_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(READ_VE_CONST));
		OBJ_SET_FULL(item,"bind_to_list",g_strdup("get_data_buttons"),g_free);

		item = glade_xml_get_widget(xml,"burn_data_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(BURN_FLASH));
		OBJ_SET_FULL(item,"bind_to_list",g_strdup("burners"),g_free);
		bind_to_lists_f(item,"burners");
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget_f("main_window")));
		glade_xml_signal_autoconnect(xml);
		gtk_widget_show(GTK_WIDGET(window));

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
			gtk_widget_show(GTK_WIDGET(window));
	EXIT();
	return TRUE;
}


/*!
 \brief General purpose handler to hide/show Battery calibrate window
 \param widget is a pointer to the widget clicked
 \param data is unused
 \returns TRUE
 */
G_MODULE_EXPORT gboolean show_battery_calibrator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	gfloat *tmpf = NULL;
	Firmware_Details *firmware = NULL;
	GList ***ecu_widgets;
	void (*update_widget_f)(gpointer, gpointer) = NULL;

	ENTER();
	if (!update_widget_f)
		get_symbol_f("update_widget",(void **)&update_widget_f);

	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!main_xml) || (DATA_GET(global_data,"leaving")))
	{
		EXIT();
		return TRUE;
	}

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"battery_calibration_window",NULL);
		window = glade_xml_get_widget(xml,"battery_calibration_window");
		glade_xml_signal_autoconnect(xml);

		item = glade_xml_get_widget(xml,"batt0_entry");
		register_widget_f("batt0_entry",item);
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(522));
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		tmpf = g_new0(gfloat, 1);
		*tmpf = 0.1;
		OBJ_SET_FULL(item,"fromecu_mult",tmpf,g_free);
		ecu_widgets[0][522] = g_list_prepend(
				ecu_widgets[0][522],
				(gpointer)item);

		item = glade_xml_get_widget(xml,"battmax_entry");
		register_widget_f("battmax_entry",item);
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(524));
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		tmpf = g_new0(gfloat, 1);
		*tmpf = 0.1;
		OBJ_SET_FULL(item,"fromecu_mult",tmpf,g_free);
		ecu_widgets[0][524] = g_list_prepend(
				ecu_widgets[0][524],
				(gpointer)item);

			/* Force them to update */
		g_list_foreach(ecu_widgets[0][522],update_widget_f,NULL);
		g_list_foreach(ecu_widgets[0][524],update_widget_f,NULL);

		item = glade_xml_get_widget(xml,"get_data_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(READ_VE_CONST));
		OBJ_SET_FULL(item,"bind_to_list",g_strdup("get_data_buttons"),g_free);

		item = glade_xml_get_widget(xml,"burn_data_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(BURN_FLASH));
		OBJ_SET_FULL(item,"bind_to_list",g_strdup("burners"),g_free);
		bind_to_lists_f(item,"burners");
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget_f("main_window")));
		gtk_widget_show(GTK_WIDGET(window));

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
			gtk_widget_show(GTK_WIDGET(window));
	EXIT();
	return TRUE;
}


/*!
  \brief shows the sensor calibration help text
  \param widget is the button the user clicked on
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean show_sensor_calibration_help(GtkWidget *widget, gpointer data)
{
	GtkWidget *window;
	GtkWidget *view;
	gchar * text = NULL;
	GtkTextBuffer *buffer;

	ENTER();
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	view = gtk_text_view_new ();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view),FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view),FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(view),5);

	gtk_container_add(GTK_CONTAINER(window),view);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	text = g_strdup("MAP Sensor Calibration\n\nFor the      MPX4115   use   10.6 and        121.7\nMPX4250          10.0    260.0\nMPXH6300 1.1     315.5\nGM 3-BAR 1.1     315.5\nMPXH6400 3.5     416.5\n\n(GM 3-bar data from Dave Hartnell, http://www.not2fast.com/electronics/component_docs/MAP_12223861.pdf)\n\n    Sensor type     vLo     pLo     vHi     pHi     vRef\n  MPX4115         0.204 v 15 kPa  4.794 v 115 kPa 5.100 v\n       MPX4250         0.204 v 20 kPa  4.896 v 250 kPa 5.100 v\n       MPXH6300        0.306 v 20 kPa  4.913 v 304 kPa 5.100 v\n       GM 3-BAR        0.631 v 40 kPa  4.914 v 304 kPa 5.100 v\n       MPXH6400        0.200 v 20 kPa  4.800 v 400 kPa 5.000 v\n\nIn general, use values derived from these equations:\n\n     m = (pHi-pLo)/(vHi-vLo)\n       pv1 = pLo - m * vLo\n   pv2 = pv1 + m * vRef\n\nReferences:\n   http://www.freescale.com/files/sensors/doc/data_sheet/MPX4115A.pdf\n    http://www.freescale.com/files/sensors/doc/data_sheet/MPX4250A.pdf\n    http://www.freescale.com/files/sensors/doc/data_sheet/MPXH6300A.pdf\n   http://www.freescale.com/files/sensors/doc/data_sheet/MPXH6400A.pdf\n\nBarometer Sensor Calibration\n\nIf your system has an external barometer sensor, separate from the MAP sensor,\nthen use these values to calibrate it properly.  If you have a standard MS installation, then copy your MAP sensor values here.\n\nBarometric Correction Calibration\n\nCorrection for barometric effects is performed using the linear function below.\n\n      correction = correction_0 + (rate * barometer) / 100\n'At total vacuum' contains the total correction at a barometer reading of 0 kPa (you are on the moon).\nThe 'Rate' contains the percentage per 100 kPa to scale the barometer value.\nUsing the default values of 147 and -47, we see that for a barometer of 100 kPa,\nwe have 100% correction.\ncorrection = 147 + (-47*100) / 100 = 100%\n ");
	gtk_text_buffer_set_text (buffer, text, -1);


	g_free(text);
	gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget_f("main_window")));
	gtk_widget_show(window);


	EXIT();
	return TRUE;
}



/*! 
  \brief tells ms2 to reinitialize 
  \param widget is unused
  \param data is unused
 */
G_MODULE_EXPORT gboolean ms2_reinit(GtkWidget *widget, gpointer data)
{
	ENTER();
	io_cmd_f("ms2_reinit",NULL);
	EXIT();
	return TRUE;
}


/*! 
  \brief tells ms2 to fully reboot 
  \param widget is unused
  \param data is unused
 */
G_MODULE_EXPORT gboolean ms2_reboot(GtkWidget *widget, gpointer data)
{
	ENTER();
	io_cmd_f("ms2_reboot",NULL);
	EXIT();
	return TRUE;
}


/*!
  \brief General purpose handler to hide/show tps calibrate window
  \param widget is the pointer to the TPS caliobration window
  \param data is unused
 */
G_MODULE_EXPORT gboolean show_tps_calibrator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	Firmware_Details *firmware = NULL;
	GList ***ecu_widgets = NULL;
	void (*update_widget_f)(gpointer, gpointer) = NULL;

	ENTER();
	if (!update_widget_f)
		get_symbol_f("update_widget",(void **)&update_widget_f);


	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!main_xml) || (DATA_GET(global_data,"leaving")))
	{
		EXIT();
		return TRUE;
	}

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"calibrate_tps_window",NULL);
		window = glade_xml_get_widget(xml,"calibrate_tps_window");
		glade_xml_signal_autoconnect(xml);

		item = glade_xml_get_widget(xml,"tpsMin_entry");
		register_widget_f("tpsMin_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		if (firmware->capabilities & PIS)
			OBJ_SET(item,"offset",GINT_TO_POINTER(2676));
		else
			OBJ_SET(item,"offset",GINT_TO_POINTER(518));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
		if (firmware->capabilities & PIS)
		{
			OBJ_SET_FULL(item,"raw_upper",g_strdup("255"),g_free);
			OBJ_SET(item,"precision",GINT_TO_POINTER(0));
			ecu_widgets[0][2676] = g_list_prepend(
					ecu_widgets[0][2676],
					(gpointer)item);
		}
		else
		{
			OBJ_SET_FULL(item,"raw_upper",g_strdup("2047"),g_free);
			OBJ_SET(item,"precision",GINT_TO_POINTER(0));
			ecu_widgets[0][518] = g_list_prepend(
					ecu_widgets[0][518],
					(gpointer)item);
		}

		item = glade_xml_get_widget(xml,"tpsMax_entry");
		register_widget_f("tpsMax_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"last_value",GINT_TO_POINTER(-G_MAXINT));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		if (firmware->capabilities & PIS)
			OBJ_SET(item,"offset",GINT_TO_POINTER(2678));
		else
			OBJ_SET(item,"offset",GINT_TO_POINTER(520));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET_FULL(item,"raw_lower",g_strdup("0"),g_free);
		if (firmware->capabilities & PIS)
		{
			OBJ_SET_FULL(item,"raw_upper",g_strdup("255"),g_free);
			OBJ_SET(item,"precision",GINT_TO_POINTER(0));
			ecu_widgets[0][2678] = g_list_prepend(
					ecu_widgets[0][2678],
					(gpointer)item);

			/* Force them to update */
			g_list_foreach(ecu_widgets[0][2676],update_widget_f,NULL);
			g_list_foreach(ecu_widgets[0][2678],update_widget_f,NULL);

		}
		else
		{
			OBJ_SET_FULL(item,"raw_upper",g_strdup("2047"),g_free);
			OBJ_SET(item,"precision",GINT_TO_POINTER(0));
			ecu_widgets[0][520] = g_list_prepend(
					ecu_widgets[0][520],
					(gpointer)item);

			/* Force them to update */
			g_list_foreach(ecu_widgets[0][518],update_widget_f,NULL);
			g_list_foreach(ecu_widgets[0][520],update_widget_f,NULL);

		}


		item = glade_xml_get_widget(xml,"calibrate_tps_cancel_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(BURN_FLASH));

		item = glade_xml_get_widget(xml,"tps_calibrate_ok_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(BURN_FLASH));

		item = glade_xml_get_widget(xml,"get_tps_button_min");
		OBJ_SET(item,"handler",GINT_TO_POINTER(GET_CURR_TPS));
		if (firmware->capabilities & PIS)
			OBJ_SET_FULL(item,"source",g_strdup("status_adc_tps"),g_free);
		else
			OBJ_SET_FULL(item,"source",g_strdup("tpsADC"),g_free);
		OBJ_SET_FULL(item,"dest_widget",g_strdup("tpsMin_entry"),g_free);

		item = glade_xml_get_widget(xml,"get_tps_button_max");
		OBJ_SET(item,"handler",GINT_TO_POINTER(GET_CURR_TPS));
		if (firmware->capabilities & PIS)
			OBJ_SET_FULL(item,"source",g_strdup("status_adc_tps"),g_free);
		else
			OBJ_SET_FULL(item,"source",g_strdup("tpsADC"),g_free);
		OBJ_SET_FULL(item,"dest_widget",g_strdup("tpsMax_entry"),g_free);
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget_f("main_window")));
		gtk_widget_show(GTK_WIDGET(window));
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
			gtk_widget_show(GTK_WIDGET(window));
	EXIT();
	return TRUE;
}


/*!
  \brief toggle button handler for the therm calibrator
  \param widget is the pointer to the togglebutton flipped
  \param data is the pointer to the destination widget we want to enable or
  disable its sensitivity
  */
G_MODULE_EXPORT gboolean therm_set_state(gpointer data, GtkWidget *widget)
{
	GtkWidget *dest = (GtkWidget *)data;
	
	ENTER();
	g_return_val_if_fail(GTK_IS_WIDGET(dest),FALSE);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		gtk_widget_set_sensitive(dest,TRUE);
	else
		gtk_widget_set_sensitive(dest,FALSE);
	EXIT();
	return TRUE;
}
