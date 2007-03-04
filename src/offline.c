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
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <getfiles.h>
#include <gui_handlers.h>
#include <init.h>
#include <interrogate.h>
#include <listmgmt.h>
#include <lookuptables.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <mode_select.h>
#include <multi_expr_loader.h>
#include <offline.h>
#include <rtv_map_loader.h>
#include <string.h>
#include <stdlib.h>
#include <tabloader.h>
#include <threads.h>


gchar * offline_firmware_choice = NULL;
gboolean offline = FALSE;
extern gint dbg_lvl;


/*!
 \brief set_offline_mode() is called when the "Offline Mode" button is clicked
 in the general tab and is used to present the user with list of firmware 
 choices to select one for loading to work in offline mode (no connection to
 an ECU)
 */
void set_offline_mode(void)
{
	extern GHashTable *dynamic_widgets;
	GtkWidget * widget = NULL;
	gchar * filename = NULL;
	GArray *cmd_array = NULL;
	GHashTable *cmd_details = NULL;
	struct Canidate *canidate = NULL;
	extern struct Firmware_Details *firmware;
	extern gint ecu_caps;
	extern gboolean interrogated;
	gchar **sources = NULL;
	gchar **suffixes = NULL;
	gchar **conv_exprs = NULL;
	gchar **precisions = NULL;
	gchar **expr_keys = NULL;
	gint len1 = 0;
	gint len2 = 0;
	gint len3 = 0;
	gint len4 = 0;
	gint len5 = 0;
	struct MultiSource *multi = NULL;
	gint j = 0;

	gint i = 0;

	offline = TRUE;
	/* Disable interrogation button */
	widget = g_hash_table_lookup(dynamic_widgets,"interrogate_button");
	if (GTK_IS_WIDGET(widget))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);

	queue_function(g_strdup("kill_conn_warning"));

	cmd_details = g_hash_table_new(g_str_hash,g_str_equal);
	cmd_array = validate_and_load_tests(cmd_details);

	filename = present_firmware_choices(cmd_array,cmd_details);
	if (!filename)
	{
		offline = FALSE;
		interrogated = FALSE;
		g_hash_table_destroy(cmd_details);
		g_array_free(cmd_array,TRUE);
		widget = g_hash_table_lookup(dynamic_widgets,"interrogate_button");
		if (GTK_IS_WIDGET(widget))
			gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);
		widget = g_hash_table_lookup(dynamic_widgets,"offline_button");
		if (GTK_IS_WIDGET(widget))
			gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);
		return;

	}
	canidate = load_potential_match(cmd_array,filename);

	load_profile_details(canidate);
	load_lookuptables(canidate);
	/* Set flags */
	ecu_caps = canidate->capabilities;

	/* Set expected sizes for commands */
	if (!firmware)
		firmware = g_new0(struct Firmware_Details,1);

	firmware->name = g_strdup(canidate->name);
	firmware->tab_list = g_strsplit(canidate->load_tabs,",",0);
	firmware->tab_confs = g_strsplit(canidate->tab_confs,",",0);
	firmware->rtv_map_file = g_strdup(canidate->rtv_map_file);
	firmware->sliders_map_file = g_strdup(canidate->sliders_map_file);
	firmware->status_map_file = g_strdup(canidate->status_map_file);
	firmware->multi_page = canidate->multi_page;
	firmware->total_tables = canidate->total_tables;
	firmware->total_pages = canidate->total_pages;
	firmware->write_cmd = g_strdup(canidate->write_cmd);
	firmware->burn_cmd = g_strdup(canidate->burn_cmd);
	firmware->page_cmd = g_strdup(canidate->page_cmd);

	/* Allocate RAM for the Req_Fuel_Params structures. */
	firmware->rf_params = g_new0(struct Req_Fuel_Params *,firmware->total_tables);

	/* Allocate ram for the necessary structures... */
	firmware->table_params = g_new0(struct Table_Params *,firmware->total_tables);
	for (i=0;i<firmware->total_tables;i++)
	{
		firmware->rf_params[i] = g_new0(struct Req_Fuel_Params, 1);
		firmware->table_params[i] = initialize_table_params();
		memcpy(firmware->table_params[i],canidate->table_params[i],sizeof(struct Table_Params));
		firmware->rf_params[i]->num_cyls=1;
		firmware->rf_params[i]->num_squirts=1;
		firmware->rf_params[i]->num_inj=1;
		/* Check for multi source table handling */
		if (firmware->table_params[i]->x_multi_source)
		{
			firmware->table_params[i]->x_multi_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,free_multi_source);
			expr_keys = g_strsplit(firmware->table_params[i]->x_multi_expr_keys,",",-1);
			sources = g_strsplit(firmware->table_params[i]->x_sources,",",-1);
			suffixes = g_strsplit(firmware->table_params[i]->x_suffixes,",",-1);
			conv_exprs = g_strsplit(firmware->table_params[i]->x_conv_exprs,",",-1);
			precisions = g_strsplit(firmware->table_params[i]->x_precisions,",",-1);
			len1 = g_strv_length(expr_keys);
			len2 = g_strv_length(sources);
			len3 = g_strv_length(suffixes);
			len4 = g_strv_length(conv_exprs);
			len5 = g_strv_length(precisions);
			if ((len1 != len2) || (len1 != len3) || (len1 != len4) || (len1 != len5))
				printf("length mismatch!\n");
			for (j=0;j<len1;j++)
			{
				multi = g_new0(struct MultiSource,1);
				multi->source = g_strdup(sources[j]);
				multi->conv_expr = g_strdup(conv_exprs[j]);
				multi->evaluator = evaluator_create(multi->conv_expr);
				multi->suffix = g_strdup(suffixes[j]);
				multi->precision = (gint)strtol(precisions[j],NULL,10);
				g_hash_table_insert(firmware->table_params[i]->x_multi_hash,g_strdup(expr_keys[j]),(gpointer)multi);
			}
			g_strfreev(expr_keys);
			g_strfreev(sources);
			g_strfreev(suffixes);
			g_strfreev(conv_exprs);
			g_strfreev(precisions);
		}
		else
			firmware->table_params[i]->x_eval = evaluator_create(firmware->table_params[i]->x_conv_expr);
		/* Check for multi source table handling */
		if (firmware->table_params[i]->y_multi_source)
		{
			firmware->table_params[i]->y_multi_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,free_multi_source);
			expr_keys = g_strsplit(firmware->table_params[i]->y_multi_expr_keys,",",-1);
			sources = g_strsplit(firmware->table_params[i]->y_sources,",",-1);
			suffixes = g_strsplit(firmware->table_params[i]->y_suffixes,",",-1);
			conv_exprs = g_strsplit(firmware->table_params[i]->y_conv_exprs,",",-1);
			precisions = g_strsplit(firmware->table_params[i]->y_precisions,",",-1);
			len1 = g_strv_length(expr_keys);
			len2 = g_strv_length(sources);
			len3 = g_strv_length(suffixes);
			len4 = g_strv_length(conv_exprs);
			len5 = g_strv_length(precisions);
			if ((len1 != len2) || (len1 != len3) || (len1 != len4) || (len1 != len5))
				printf("length mismatch!\n");
			for (j=0;j<len1;j++)
			{
				multi = g_new0(struct MultiSource,1);
				multi->source = g_strdup(sources[j]);
				multi->conv_expr = g_strdup(conv_exprs[j]);
				multi->evaluator = evaluator_create(multi->conv_expr);
				multi->suffix = g_strdup(suffixes[j]);
				multi->precision = (gint)strtol(precisions[j],NULL,10);
				g_hash_table_insert(firmware->table_params[i]->y_multi_hash,g_strdup(expr_keys[j]),(gpointer)multi);
			}
			g_strfreev(expr_keys);
			g_strfreev(sources);
			g_strfreev(suffixes);
			g_strfreev(conv_exprs);
			g_strfreev(precisions);
		}
		else
			firmware->table_params[i]->y_eval = evaluator_create(firmware->table_params[i]->y_conv_expr);

		/* Check for multi source table handling */
		if (firmware->table_params[i]->z_multi_source)
		{
			firmware->table_params[i]->z_multi_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,free_multi_source);
			expr_keys = g_strsplit(firmware->table_params[i]->z_multi_expr_keys,",",-1);
			sources = g_strsplit(firmware->table_params[i]->z_sources,",",-1);
			suffixes = g_strsplit(firmware->table_params[i]->z_suffixes,",",-1);
			conv_exprs = g_strsplit(firmware->table_params[i]->z_conv_exprs,",",-1);
			precisions = g_strsplit(firmware->table_params[i]->z_precisions,",",-1);
			len1 = g_strv_length(expr_keys);
			len2 = g_strv_length(sources);
			len3 = g_strv_length(suffixes);
			len4 = g_strv_length(conv_exprs);
			len5 = g_strv_length(precisions);
			if ((len1 != len2) || (len1 != len3) || (len1 != len4) || (len1 != len5))
				printf("length mismatch!\n");
			for (j=0;j<len1;j++)
			{
				multi = g_new0(struct MultiSource,1);
				multi->source = g_strdup(sources[j]);
				multi->conv_expr = g_strdup(conv_exprs[j]);
				multi->evaluator = evaluator_create(multi->conv_expr);
				multi->suffix = g_strdup(suffixes[j]);
				multi->precision = (gint)strtol(precisions[j],NULL,10);
				g_hash_table_insert(firmware->table_params[i]->z_multi_hash,g_strdup(expr_keys[j]),(gpointer)multi);
			}
			g_strfreev(expr_keys);
			g_strfreev(sources);
			g_strfreev(suffixes);
			g_strfreev(conv_exprs);
			g_strfreev(precisions);
		}
		else
			firmware->table_params[i]->z_eval = evaluator_create(firmware->table_params[i]->z_conv_expr);


	}

	firmware->page_params = g_new0(struct Page_Params *,firmware->total_pages);
	for (i=0;i<firmware->total_pages;i++)
	{
		firmware->page_params[i] = initialize_page_params();
		memcpy(firmware->page_params[i],canidate->page_params[i],sizeof(struct Page_Params));
	}

	mem_alloc();

	g_hash_table_destroy(cmd_details);
	g_array_free(cmd_array,TRUE);

	interrogated = TRUE;

	io_cmd(IO_LOAD_REALTIME_MAP,NULL);
	io_cmd(IO_LOAD_GUI_TABS,NULL);
	io_cmd(IO_UPDATE_VE_CONST,NULL);

	widget = g_hash_table_lookup(dynamic_widgets,"interrogate_button");
	if (GTK_IS_WIDGET(widget))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
	widget = g_hash_table_lookup(dynamic_widgets,"offline_button");
	if (GTK_IS_WIDGET(widget))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
	g_list_foreach(get_list("get_data_buttons"),set_widget_sensitive,GINT_TO_POINTER(FALSE));

}


/*!
 \brief present_firmware_choices() presents a dialog box with the firmware
 choices.
 \param cmd_array (GArray) array of commands to be passed to subfunctions
 \param cmd_details (GHashTable) hashtable of details re the commands in the
 cmd_array array.
 \returns the name of the chosen firmware
 */
gchar * present_firmware_choices(GArray *cmd_array, GHashTable *cmd_details)
{
	gchar ** filenames = NULL;
	struct Canidate *potential = NULL;
	GtkWidget *dialog_window = NULL;
	GtkWidget *dialog = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *button = NULL;
	GtkWidget *label = NULL;
	GSList *group = NULL;
	gint i = 0;
	gint result = 0;
	extern gchar * offline_firmware_choice;


	filenames = get_files(g_strconcat(INTERROGATOR_DATA_DIR,PSEP,"Profiles",PSEP,NULL),g_strdup("prof"));
	if (!filenames)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": present_firmware_choices()\n\t NO Interrogation profiles found, was MegaTunix installed properly?\n\n"));
		return NULL;
	}

	dialog_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	dialog = gtk_dialog_new_with_buttons("Select Firmware",
				GTK_WINDOW(dialog_window),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				"Load",
				GTK_RESPONSE_OK,
				"Abort",
				GTK_RESPONSE_CANCEL,
				NULL);

	vbox = gtk_vbox_new(TRUE,5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),vbox,TRUE,TRUE,0);

	group = NULL;
	i = 0;
	while (filenames[i]) 
	{
		potential = load_potential_match(cmd_array,filenames[i]);
		if (!potential)
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": present_firmware_choices()\n\t Interrogation profile damaged!, was MegaTunix installed properly?\n\n"));
			i++;
			continue;
		}
		hbox = gtk_hbox_new(FALSE,10);
		gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);
		label = gtk_label_new(g_strdup(potential->name));
		gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,TRUE,0);
		button = gtk_radio_button_new(group);
		g_object_set_data(G_OBJECT(button),"filename",g_strdup(filenames[i]));
		g_object_set_data(G_OBJECT(button),"handler",
				GINT_TO_POINTER(OFFLINE_FIRMWARE_CHOICE));
		g_signal_connect(button,
				"toggled",
				G_CALLBACK(toggle_button_handler),
				NULL);
		gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,TRUE,0);
		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
		close_profile(potential);
		i++;
	}
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),TRUE);
	g_strfreev(filenames);
	
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	gtk_widget_destroy(dialog_window);
	switch (result)
	{
		case GTK_RESPONSE_ACCEPT:
		case GTK_RESPONSE_OK:
			return offline_firmware_choice;
			break;
		default:
			return NULL;
	}

}

gint ptr_sort(gconstpointer a, gconstpointer b)
{
	return strcmp((gchar *)a, (gchar *) b);
}
