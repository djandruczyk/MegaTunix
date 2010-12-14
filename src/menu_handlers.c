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
#include <debugging.h>
#include <enums.h>
#include <firmware.h>
#include <gui_handlers.h>
#include <math.h>
#include <menu_handlers.h>
#include <plugin.h>
#include <runtime_text.h>
#include <stdlib.h>
#include <tabloader.h>
#include <threads.h>
#include <widgetmgmt.h>
#include <mtxmatheval.h>

extern gconstpointer *global_data;
static struct 
{
	const gchar *item;
	TabIdent tab;
}items[] = {
	{"vetable_tuning_menuitem",VETABLES_TAB},
	{"spark_tuning_menuitem",SPARKTABLES_TAB},
	{"afr_tuning_menuitem",AFRTABLES_TAB},
	{"boost_tuning_menuitem",BOOSTTABLES_TAB},
	{"staging_tuning_menuitem",STAGING_TAB},
	{"rotary_tuning_menuitem",ROTARYTABLES_TAB},
	{"runtime_vars_menuitem",RUNTIME_TAB},
	{"ecu_errors_menuitem",ERROR_STATUS_TAB},
};

static struct 
{
	const gchar *item;
	FioAction action;
}fio_items[] = {
	{"import_tables_menuitem",VEX_IMPORT},
	{"export_tables_menuitem",VEX_EXPORT},
	{"restore_ecu_menuitem",ECU_RESTORE},
	{"backup_ecu_menuitem",ECU_BACKUP},
};

G_MODULE_EXPORT void setup_menu_handlers_pf(void)
{
	void (*common_plugin_menu_setup)(GladeXML *);
	GtkWidget *item = NULL;
	guint i = 0;
	GladeXML *xml = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!xml) || (DATA_GET(global_data,"leaving")))
		return;

	if (get_symbol ("common_plugin_menu_setup",(void *)&common_plugin_menu_setup))
		common_plugin_menu_setup(xml);

	gdk_threads_enter();

	item = glade_xml_get_widget(xml,"show_tab_visibility_menuitem");
	gtk_widget_set_sensitive(item,TRUE);
	
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
	gdk_threads_leave();
	return;
}

/*!
 \brief switches to tab encoded into the widget
 */
G_MODULE_EXPORT gboolean jump_to_tab(GtkWidget *widget, gpointer data)
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
G_MODULE_EXPORT gboolean settings_transfer(GtkWidget *widget, gpointer data)
{
	FioAction action = -1;
	action = (FioAction)OBJ_GET(widget,"fio_action");
	void (*do_backup)(GtkWidget *, gpointer) = NULL;
	void (*do_restore)(GtkWidget *, gpointer) = NULL;
	void (*vex_import)(GtkWidget *, gpointer) = NULL;
	void (*vex_export)(GtkWidget *, gpointer) = NULL;

	switch (action)
	{
		case VEX_IMPORT:
			if (get_symbol("select_vex_for_import",(void*)&vex_import))
				vex_import(NULL,NULL);
			break;
		case VEX_EXPORT:
			if (get_symbol("select_vex_for_export",(void*)&vex_export))
				vex_export(NULL,NULL);
			break;
		case ECU_BACKUP:
			if (get_symbol("select_file_for_ecu_backup",(void*)&do_backup))
				do_backup(NULL,NULL);
			break;
		case ECU_RESTORE:
			if (get_symbol("select_file_for_ecu_restore",(void*)&do_restore))
				do_restore(NULL,NULL);
			break;
	}
	return TRUE;
}

/*!
 \brief General purpose handler to take care of menu initiated settings 
 transfers like VEX import/export and ECU backup/restore
 */
G_MODULE_EXPORT gboolean check_tab_existance(TabIdent target)
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
G_MODULE_EXPORT gboolean show_tps_calibrator_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	Firmware_Details *firmware = NULL;
	GList ***ecu_widgets = NULL;
	void (*update_widget_f)(gpointer, gpointer) = NULL;

	if (!update_widget_f)
		get_symbol("update_widget",(void *)&update_widget_f);

	ecu_widgets = DATA_GET(global_data,"ecu_widgets");
	firmware = DATA_GET(global_data,"firmware");

	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!main_xml) || (DATA_GET(global_data,"leaving")))
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
		if (firmware->capabilities & PIS)
			OBJ_SET(item,"offset",GINT_TO_POINTER(2676));
		else
			OBJ_SET(item,"offset",GINT_TO_POINTER(518));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		if (firmware->capabilities & PIS)
		{
			OBJ_SET(item,"raw_upper",g_strdup("255"));
			OBJ_SET(item,"precision",GINT_TO_POINTER(0));
			ecu_widgets[0][2676] = g_list_prepend(
					ecu_widgets[0][2676],
					(gpointer)item);
		}
		else
		{
			OBJ_SET(item,"raw_upper",g_strdup("2047"));
			OBJ_SET(item,"precision",GINT_TO_POINTER(0));
			ecu_widgets[0][518] = g_list_prepend(
					ecu_widgets[0][518],
					(gpointer)item);
		}

		item = glade_xml_get_widget(xml,"tpsMax_entry");
		register_widget("tpsMax_entry",item);
		OBJ_SET(item,"handler",GINT_TO_POINTER(GENERIC));
		OBJ_SET(item,"dl_type",GINT_TO_POINTER(IMMEDIATE));
		OBJ_SET(item,"page",GINT_TO_POINTER(0));
		if (firmware->capabilities & PIS)
			OBJ_SET(item,"offset",GINT_TO_POINTER(2678));
		else
			OBJ_SET(item,"offset",GINT_TO_POINTER(520));
		OBJ_SET(item,"size",GINT_TO_POINTER(MTX_S16));
		OBJ_SET(item,"raw_lower",g_strdup("0"));
		if (firmware->capabilities & PIS)
		{
			OBJ_SET(item,"raw_upper",g_strdup("255"));
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
			OBJ_SET(item,"raw_upper",g_strdup("2047"));
			OBJ_SET(item,"precision",GINT_TO_POINTER(0));
			ecu_widgets[0][520] = g_list_prepend(
					ecu_widgets[0][520],
					(gpointer)item);

			/* Force them to update */
			g_list_foreach(ecu_widgets[0][518],update_widget_f,NULL);
			g_list_foreach(ecu_widgets[0][520],update_widget_f,NULL);

		}

		item = glade_xml_get_widget(xml,"get_tps_button_min");
		OBJ_SET(item,"handler",GINT_TO_POINTER(GET_CURR_TPS));
		if (firmware->capabilities & PIS)
			OBJ_SET(item,"source",g_strdup("status_adc_tps"));
		else
			OBJ_SET(item,"source",g_strdup("tpsADC"));
		OBJ_SET(item,"dest_widget",g_strdup("tpsMin_entry"));

		item = glade_xml_get_widget(xml,"get_tps_button_max");
		OBJ_SET(item,"handler",GINT_TO_POINTER(GET_CURR_TPS));
		if (firmware->capabilities & PIS)
			OBJ_SET(item,"source",g_strdup("status_adc_tps"));
		else
			OBJ_SET(item,"source",g_strdup("tpsADC"));
		OBJ_SET(item,"dest_widget",g_strdup("tpsMax_entry"));
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget("main_window")));
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
 \ show / hide window
 \ populates the combo box with available spark maps that could be filled out by the generator
  */
G_MODULE_EXPORT gboolean show_create_ignition_map_window(GtkWidget *widget, gpointer data)
{
	static GtkWidget *window = NULL;
	GtkWidget *item = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	Firmware_Details *firmware = NULL;
	GList *spark_tables = NULL;
	gint t;

	firmware = DATA_GET(global_data,"firmware");
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!main_xml) || (DATA_GET(global_data,"leaving")))
		return TRUE;

	if (!GTK_IS_WIDGET(window))
	{
		xml = glade_xml_new(main_xml->filename,"create_ignition_window",NULL);
		window = glade_xml_get_widget(xml,"create_ignition_window");
		glade_xml_signal_autoconnect(xml);

		for (t=0; t != firmware->total_tables; t++)
		{
			if (firmware->table_params[t]->is_spark)
				spark_tables = g_list_append(spark_tables, firmware->table_params[t]->table_name);
		}

		if (spark_tables)
		{
			item = glade_xml_get_widget(xml,"spark_table_combo");
			gtk_combo_set_popdown_strings(GTK_COMBO(item), spark_tables);
		}
		else
		{
			dbg_func(CRITICAL, g_strdup_printf(__FILE__ ": show_create_ignition_map_window():\n\tUnable to find or allocate any table titles!\n"));
			return TRUE;	// panic
		}

		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget("main_window")));
		gtk_widget_show_all(GTK_WIDGET(window));
		return TRUE;
	}

	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(window)))
		gtk_widget_hide_all(GTK_WIDGET(window));
	else
		gtk_widget_show_all(GTK_WIDGET(window));
	return TRUE;
}
