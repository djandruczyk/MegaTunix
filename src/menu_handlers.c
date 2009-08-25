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

#include <afr_calibrate.h>
#include <config.h>
#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <fileio.h>
#include <gui_handlers.h>
#include <math.h>
#include <menu_handlers.h>
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
	gint i = 0;
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
		g_signal_connect_swapped(G_OBJECT(window),"destroy_event",
				G_CALLBACK(gtk_widget_hide),window);
		g_signal_connect_swapped(G_OBJECT(window),"delete_event",
				G_CALLBACK(gtk_widget_hide),window);

		item = glade_xml_get_widget(xml,"tpsMin_entry");
		register_widget("tpsMin_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		OBJ_SET(item,"offset",GINT_TO_POINTER(518));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(2047));
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
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(2047));
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
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(100000));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"temp1_entry");
		register_widget("temp1_entry",item);
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(-40));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(300));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"temp2_entry");
		register_widget("temp2_entry",item);
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(-40));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(300));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"temp3_entry");
		register_widget("temp3_entry",item);
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(-40));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(300));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"resistance1_entry");
		register_widget("resistance1_entry",item);
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(500000));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"resistance2_entry");
		register_widget("resistance2_entry",item);
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(500000));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"resistance3_entry");
		register_widget("resistance3_entry",item);
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(500000));
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
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(5));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"voltage2_entry");
		register_widget("voltage2_entry",item);
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(5));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"afr1_entry");
		register_widget("afr1_entry",item);
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(99));
		OBJ_SET(item,"precision",GINT_TO_POINTER(1));

		item = glade_xml_get_widget(xml,"afr2_entry");
		register_widget("afr2_entry",item);
		OBJ_SET(item,"raw_lower",GINT_TO_POINTER(0));
		OBJ_SET(item,"raw_upper",GINT_TO_POINTER(99));
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
