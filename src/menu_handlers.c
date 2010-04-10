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
#include <afr_calibrate.h>
#include <enums.h>
#include <firmware.h>
#include <fileio.h>
#include <gui_handlers.h>
#include <math.h>
#include <menu_handlers.h>
#include <runtime_text.h>
#include <stdlib.h>
#include <tabloader.h>
#include <threads.h>
#include <vex_support.h>
#include <widgetmgmt.h>


extern GObject *global_data;
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
	{"lookuptables_setup_menuitem",-1},
	{"import_tables_menuitem",VEX_IMPORT},
	{"export_tables_menuitem",VEX_EXPORT},
	{"restore_ecu_menuitem",ECU_RESTORE},
	{"backup_ecu_menuitem",ECU_BACKUP},
};

EXPORT void setup_menu_handlers_pf()
{
	GtkWidget *item = NULL;
	guint i = 0;
	GladeXML *xml = NULL;
	extern Firmware_Details *firmware;
	extern volatile gboolean leaving;


	xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	if ((!xml) || (leaving))
		return;
	item = glade_xml_get_widget(xml,"show_tab_visibility_menuitem");
	gtk_widget_set_sensitive(item,TRUE);
	
	if (firmware->capabilities & MS2)
	{
		item = glade_xml_get_widget(xml,"show_table_generator_menuitem");
		gtk_widget_set_sensitive(item,TRUE);

		item = glade_xml_get_widget(xml,"show_tps_calibrator_menuitem");
		gtk_widget_set_sensitive(item,TRUE);

		item = glade_xml_get_widget(xml,"show_ms2_afr_calibrator_menuitem");
		gtk_widget_set_sensitive(item,TRUE);
		item = glade_xml_get_widget(xml,"show_sensor_calibrator_menuitem");
		gtk_widget_set_sensitive(item,TRUE);
		item = glade_xml_get_widget(xml,"show_trigger_offset_menuitem");
		gtk_widget_set_sensitive(item,TRUE);
		item = glade_xml_get_widget(xml,"ms2_reinit_menuitem");
		gtk_widget_set_sensitive(item,TRUE);
		item = glade_xml_get_widget(xml,"ms2_reboot_menuitem");
		gtk_widget_set_sensitive(item,TRUE);
	}
	
	for (i=0;i< (sizeof(items)/sizeof(items[0]));i++)
	{
		item = glade_xml_get_widget(xml,items[i].item);
		if (GTK_IS_WIDGET(item))
			OBJ_SET(item,"target_tab",
					GINT_TO_POINTER(items[i].tab));
		if (!check_tab_existance(items[i].tab))
			gtk_widget_set_sensitive(item,FALSE);
		else
			gtk_widget_set_sensitive(item,TRUE);
	}
	for (i=0;i< (sizeof(fio_items)/sizeof(fio_items[0]));i++)
	{
		item = glade_xml_get_widget(xml,fio_items[i].item);
		if (GTK_IS_WIDGET(item))
		{
			OBJ_SET(item,"fio_action",
					GINT_TO_POINTER(fio_items[i].action));
			gtk_widget_set_sensitive(item,TRUE);
		}
	}
}

/*!
 \brief switches to tab encoded into the widget
 */
EXPORT gboolean jump_to_tab(GtkWidget *widget, gpointer data)
{
	GtkWidget *notebook = NULL;
	TabIdent target = -1;
	TabIdent c_tab = 0;
	gint total = 0;
	GtkWidget * child = NULL;
	gint i = 0;
	
	notebook = lookup_widget( "toplevel_notebook");
	if (!GTK_IS_NOTEBOOK(notebook))
		return FALSE;
	if (!OBJ_GET(widget,"target_tab"))
		return FALSE;
	target = (TabIdent)OBJ_GET(widget,"target_tab");
	total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	for (i=0;i<total;i++)
	{
		child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
		if (!OBJ_GET(child,"tab_ident"))
			continue;
		c_tab = (TabIdent)OBJ_GET(child,"tab_ident");
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
	action = (FioAction)OBJ_GET(widget,"fio_action");

	switch (action)
	{
		case VEX_IMPORT:
			select_vex_for_import(NULL,NULL);
			break;
		case VEX_EXPORT:
			select_vex_for_export(NULL,NULL);
			break;
		case ECU_BACKUP:
			select_file_for_ecu_backup(NULL,NULL);
			break;
		case ECU_RESTORE:
			select_file_for_ecu_restore(NULL,NULL);
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
	GtkWidget *notebook = NULL;
	TabIdent c_tab = 0;
	gint total = 0;
	GtkWidget * child = NULL;
	gint i = 0;
	
	notebook = lookup_widget( "toplevel_notebook");
	if (!GTK_IS_NOTEBOOK(notebook))
		return FALSE;
	total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	for (i=0;i<total;i++)
	{
		child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
		if (!OBJ_GET(child,"tab_ident"))
			continue;
		c_tab = (TabIdent)OBJ_GET(child,"tab_ident");
		if (c_tab == target)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*!
 \brief General purpose handler to hide/show tps calibrate window
 */
EXPORT gboolean show_tps_calibrator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	extern volatile gboolean leaving;
	extern GList ***ve_widgets;

	main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	if ((!main_xml) || (leaving))
		return TRUE;

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"calibrate_tps_window",NULL);
		window = glade_xml_get_widget(xml,"calibrate_tps_window");
		glade_xml_signal_autoconnect(xml);

		item = glade_xml_get_widget(xml,"tpsMin_entry");
		register_widget("tpsMin_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(518));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("2047"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(0));
		ve_widgets[0][518] = g_list_prepend(
				ve_widgets[0][518],
				(gpointer)item);

		item = glade_xml_get_widget(xml,"tpsMax_entry");
		register_widget("tpsMax_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(520));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("2047"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(0));
		ve_widgets[0][520] = g_list_prepend(
				ve_widgets[0][520],
				(gpointer)item);

		/* Force them to update */
		g_list_foreach(ve_widgets[0][518],update_widget,NULL);
		g_list_foreach(ve_widgets[0][520],update_widget,NULL);

		item = glade_xml_get_widget(xml,"get_tps_button_min");
		OBJ_SET(item,"handler",GINT_TO_POINTER(GET_CURR_TPS));
		OBJ_SET(item,"source",g_strdup("tpsADC"));
		OBJ_SET(item,"dest_widget",g_strdup("tpsMin_entry"));
		item = glade_xml_get_widget(xml,"get_tps_button_max");
		OBJ_SET(item,"handler",GINT_TO_POINTER(GET_CURR_TPS));
		OBJ_SET(item,"source",g_strdup("tpsADC"));
		OBJ_SET(item,"dest_widget",g_strdup("tpsMax_entry"));
		gtk_widget_show_all(GTK_WIDGET(window));
		return TRUE;
	}
	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(window)))
		gtk_widget_hide_all(GTK_WIDGET(window));
	else
		gtk_widget_show_all(GTK_WIDGET(window));
	return TRUE;
}

/*!
 \brief General purpose handler to hide/show Sensor calibrate window
 */
EXPORT gboolean show_table_generator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	extern volatile gboolean leaving;
	extern GList ***ve_widgets;

	main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	if ((!main_xml) || (leaving))
		return TRUE;

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"table_generator_window",NULL);
		window = glade_xml_get_widget(xml,"table_generator_window");
		glade_xml_signal_autoconnect(xml);

		item = glade_xml_get_widget(xml,"sensor_combo");
		gtk_combo_box_set_active(GTK_COMBO_BOX(item),0);
		register_widget("thermister_sensor_combo",item);
		item = glade_xml_get_widget(xml,"temp_label");
		OBJ_SET(item,"c_label",g_strdup("Temperature(\302\260 C)"));
		OBJ_SET(item,"f_label",g_strdup("Temperature(\302\260 F)"));
		register_widget("temp_label",item);
		item = glade_xml_get_widget(xml,"bias_entry");
		register_widget("bias_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("100000"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"temp1_entry");
		register_widget("temp1_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("-40"));
		OBJ_SET(item,"raw_upper",g_strdup("300"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"temp2_entry");
		register_widget("temp2_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("-40"));
		OBJ_SET(item,"raw_upper",g_strdup("300"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"temp3_entry");
		register_widget("temp3_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("-40"));
		OBJ_SET(item,"raw_upper",g_strdup("300"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"resistance1_entry");
		register_widget("resistance1_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("500000"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"resistance2_entry");
		register_widget("resistance2_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("500000"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"resistance3_entry");
		register_widget("resistance3_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("500000"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"celsius_radiobutton");
		register_widget("thermister_celsius_radiobutton",item);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(item),FALSE);
		g_signal_emit_by_name(item,"toggled",NULL);
		gtk_widget_show_all(GTK_WIDGET(window));
		return TRUE;
	}
	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(window)))
		gtk_widget_hide_all(GTK_WIDGET(window));
	else
		gtk_widget_show_all(GTK_WIDGET(window));
	return TRUE;
}


/*!
 \brief General purpose handler to hide/show Sensor calibrate window
 */
EXPORT gboolean show_ms2_afr_calibrator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GtkWidget *item2 = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	extern volatile gboolean leaving;
	extern GList ***ve_widgets;

	main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	if ((!main_xml) || (leaving))
		return TRUE;

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"ms2_afr_calibrator_window",NULL);
		window = glade_xml_get_widget(xml,"ms2_afr_calibrator_window");
		glade_xml_signal_autoconnect(xml);

		item = glade_xml_get_widget(xml,"ego_sensor_combo");
		register_widget("afr_calibrate_ego_sensor_combo",item);
		populate_afr_calibrator_combo(item);
		item2 = glade_xml_get_widget(xml,"generic_wideband_frame");
		OBJ_SET(item,"generic_controls",item2);

		item = glade_xml_get_widget(xml,"voltage1_entry");
		register_widget("voltage1_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("5"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"voltage2_entry");
		register_widget("voltage2_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("5"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"afr1_entry");
		register_widget("afr1_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("99"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"afr2_entry");
		register_widget("afr2_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		OBJ_SET(item,"raw_upper",g_strdup("99"));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		gtk_widget_show_all(GTK_WIDGET(window));
		return TRUE;
	}
	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(window)))
		gtk_widget_hide_all(GTK_WIDGET(window));
	else
		gtk_widget_show_all(GTK_WIDGET(window));
	return TRUE;
}


/*!
 \brief General purpose handler to hide/show Sensor calibrate window
 */
EXPORT gboolean show_sensor_calibrator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	extern volatile gboolean leaving;
	extern GList ***ve_widgets;

	main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	if ((!main_xml) || (leaving))
		return TRUE;

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"sensor_calibration_window",NULL);
		window = glade_xml_get_widget(xml,"sensor_calibration_window");
		glade_xml_signal_autoconnect(xml);

		item = glade_xml_get_widget(xml,"map0_entry");
		register_widget("map0_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("-1"));
		OBJ_SET(item,"raw_upper",g_strdup("327"));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(506));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET(item,"dl_conv_expr",g_strdup("x*10"));
		OBJ_SET(item,"ul_conv_expr",g_strdup("x/10"));
		ve_widgets[0][506] = g_list_prepend(
				ve_widgets[0][506],
				(gpointer)item);

		item = glade_xml_get_widget(xml,"map5_entry");
		register_widget("map5_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("-1"));
		OBJ_SET(item,"raw_upper",g_strdup("327"));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(508));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET(item,"dl_conv_expr",g_strdup("x*10"));
		OBJ_SET(item,"ul_conv_expr",g_strdup("x/10"));
		ve_widgets[0][508] = g_list_prepend(
				ve_widgets[0][508],
				(gpointer)item);

		item = glade_xml_get_widget(xml,"baro0_entry");
		register_widget("baro0_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("-1"));
		OBJ_SET(item,"raw_upper",g_strdup("327"));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(530));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET(item,"dl_conv_expr",g_strdup("x*10"));
		OBJ_SET(item,"ul_conv_expr",g_strdup("x/10"));
		ve_widgets[0][530] = g_list_prepend(
				ve_widgets[0][530],
				(gpointer)item);

		item = glade_xml_get_widget(xml,"baro5_entry");
		register_widget("baro5_entry",item);
		OBJ_SET(item,"raw_lower",g_strdup("-1"));
		OBJ_SET(item,"raw_upper",g_strdup("327"));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(532));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET(item,"dl_conv_expr",g_strdup("x*10"));
		OBJ_SET(item,"ul_conv_expr",g_strdup("x/10"));
		ve_widgets[0][532] = g_list_prepend(
				ve_widgets[0][532],
				(gpointer)item);

		/* Force them to update */
		g_list_foreach(ve_widgets[0][506],update_widget,NULL);
		g_list_foreach(ve_widgets[0][508],update_widget,NULL);
		g_list_foreach(ve_widgets[0][530],update_widget,NULL);
		g_list_foreach(ve_widgets[0][532],update_widget,NULL);

		item = glade_xml_get_widget(xml,"get_data_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(READ_VE_CONST));
		OBJ_SET(item,"bind_to_list",g_strdup("get_data_buttons"));

		item = glade_xml_get_widget(xml,"burn_data_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(BURN_MS_FLASH));
		OBJ_SET(item,"bind_to_list",g_strdup("burners"));
		bind_to_lists(item,"burners");
		gtk_widget_show_all(GTK_WIDGET(window));

		return TRUE;
	}
	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(window)))
		gtk_widget_hide_all(GTK_WIDGET(window));
	else
		gtk_widget_show_all(GTK_WIDGET(window));
	return TRUE;
}


EXPORT gboolean show_sensor_calibration_help(GtkWidget *widhet, gpointer data)
{
	GtkWidget *window;
	GtkWidget *view;
	gchar * text = NULL;
	GtkTextBuffer *buffer;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	view = gtk_text_view_new ();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view),FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view),FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(view),5);

	gtk_container_add(GTK_CONTAINER(window),view);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	text = g_strdup("MAP Sensor Calibration\n\nFor the	MPX4115   use	10.6 and 	121.7\nMPX4250		10.0	260.0\nMPXH6300	1.1	315.5\nGM 3-BAR	1.1	315.5\nMPXH6400	3.5	416.5\n\n(GM 3-bar data from Dave Hartnell, http://www.not2fast.com/electronics/component_docs/MAP_12223861.pdf)\n\n	Sensor type	vLo	pLo	vHi	pHi	vRef\n	MPX4115 	0.204 v	15 kPa	4.794 v	115 kPa	5.100 v\n	MPX4250 	0.204 v	20 kPa	4.896 v	250 kPa	5.100 v\n	MPXH6300	0.306 v	20 kPa	4.913 v	304 kPa	5.100 v\n	GM 3-BAR	0.631 v	40 kPa	4.914 v	304 kPa	5.100 v\n	MPXH6400	0.200 v	20 kPa	4.800 v	400 kPa	5.000 v\n\nIn general, use values derived from these equations:\n\n	m = (pHi-pLo)/(vHi-vLo)\n	pv1 = pLo - m * vLo\n	pv2 = pv1 + m * vRef\n\nReferences:\n	http://www.freescale.com/files/sensors/doc/data_sheet/MPX4115A.pdf\n	http://www.freescale.com/files/sensors/doc/data_sheet/MPX4250A.pdf\n	http://www.freescale.com/files/sensors/doc/data_sheet/MPXH6300A.pdf\n	http://www.freescale.com/files/sensors/doc/data_sheet/MPXH6400A.pdf\n\nBarometer Sensor Calibration\n\nIf your system has an external barometer sensor, separate from the MAP sensor,\nthen use these values to calibrate it properly.  If you have a standard MS installation, then copy your MAP sensor values here.\n\nBarometric Correction Calibration\n\nCorrection for barometric effects is performed using the linear function below.\n\n	correction = correction_0 + (rate * barometer) / 100\n'At total vacuum' contains the total correction at a barometer reading of 0 kPa (you are on the moon).\nThe 'Rate' contains the percentage per 100 kPa to scale the barometer value.\nUsing the default values of 147 and -47, we see that for a barometer of 100 kPa,\nwe have 100% correction.\ncorrection = 147 + (-47*100) / 100 = 100%\n ");
	gtk_text_buffer_set_text (buffer, text, -1);


	g_free(text);
	gtk_widget_show_all(window);


	return TRUE;
}


/*!
 \brief General purpose handler to hide/show trigger offset window
 */
EXPORT gboolean show_trigger_offset_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GtkWidget *partner = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	extern volatile gboolean leaving;
	extern GList ***ve_widgets;

	main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	if ((!main_xml) || (leaving))
		return TRUE;

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"trigger_offset_window",NULL);
		window = glade_xml_get_widget(xml,"trigger_offset_window");
		glade_xml_signal_autoconnect(xml);

		item = glade_xml_get_widget(xml,"plus_button");
		register_widget("plus_button",item);
		OBJ_SET(item,"partner_widget",lookup_widget("IGN_trigger_offset_entry"));
		OBJ_SET(item,"handler",GINT_TO_POINTER(INCREMENT_VALUE));
		OBJ_SET(item,"amount",GINT_TO_POINTER(5));

		item = glade_xml_get_widget(xml,"minus_button");
		register_widget("minus_button",item);
		OBJ_SET(item,"partner_widget",lookup_widget("IGN_trigger_offset_entry"));
		OBJ_SET(item,"handler",GINT_TO_POINTER(DECREMENT_VALUE));
		OBJ_SET(item,"amount",GINT_TO_POINTER(5));

		item = glade_xml_get_widget(xml,"advance_parent_box");
		OBJ_SET(item,"ctrl_name",g_strdup("trigger_offset_tool_advance_rtt"));
		OBJ_SET(item,"source",g_strdup("sparkangle"));
		OBJ_SET(item,"label_prefix",g_strdup("<span font_desc=\"Sans 64\">"));
		OBJ_SET(item,"label_suffix",g_strdup("</span>"));
		OBJ_SET(item,"markup",GINT_TO_POINTER(TRUE));
		add_additional_rtt(item);

		item = glade_xml_get_widget(xml,"offset_entry");
		register_widget("offset_entry",item);
		partner = lookup_widget("IGN_trigger_offset_entry");
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"page",OBJ_GET(partner,"page"));
		OBJ_SET(item,"offset",OBJ_GET(partner,"offset"));
		OBJ_SET(item,"size",OBJ_GET(partner,"size"));
		OBJ_SET(item,"raw_lower",OBJ_GET(partner,"raw_lower"));
		OBJ_SET(item,"raw_upper",OBJ_GET(partner,"raw_upper"));
		OBJ_SET(item,"dl_conv_expr",OBJ_GET(partner,"dl_conv_expr"));
		OBJ_SET(item,"ul_conv_expr",OBJ_GET(partner,"ul_conv_expr"));
		OBJ_SET(item,"precision",OBJ_GET(partner,"precision"));
		ve_widgets[(GINT)OBJ_GET(partner,"page")][(GINT)OBJ_GET(partner,"offset")] = g_list_prepend(ve_widgets[(GINT)OBJ_GET(partner,"page")][(GINT)OBJ_GET(partner,"offset")],(gpointer)item);

		g_list_foreach(ve_widgets[(GINT)OBJ_GET(partner,"page")][(GINT)OBJ_GET(partner,"offset")],update_widget,NULL);

		item = glade_xml_get_widget(xml,"burn_data_button");
		OBJ_SET(item,"handler",GINT_TO_POINTER(BURN_MS_FLASH));
		OBJ_SET(item,"bind_to_list",g_strdup("burners"));
		bind_to_lists(item,"burners");
		/* Force them to update */
		gtk_widget_show_all(GTK_WIDGET(window));
		return TRUE;
	}
	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(window)))
		gtk_widget_hide_all(GTK_WIDGET(window));
	else
		gtk_widget_show_all(GTK_WIDGET(window));
	return TRUE;
}


/*! \brief tell ms2 to reinitialize */
EXPORT gboolean ms2_reinit(GtkWidget *widget, gpointer data)
{
	io_cmd("ms2_reinit",NULL);
	return TRUE;
}


/*! \brief tell ms2 to fully reboot */
EXPORT gboolean ms2_reboot(GtkWidget *widget, gpointer data)
{
	io_cmd("ms2_reboot",NULL);
	return TRUE;
}
