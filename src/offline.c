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

#include <apicheck.h>
#include <api-versions.h>
#include <config.h>
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <fileio.h>
#include <getfiles.h>
#include <gui_handlers.h>
#include <init.h>
#include <interrogate.h>
#include <listmgmt.h>
#include <lookuptables.h>
#include <mtxmatheval.h>
#include <mode_select.h>
#include <notifications.h>
#include <multi_expr_loader.h>
#include <offline.h>
#include <plugin.h>
#include <rtv_map_loader.h>
#include <string.h>
#include <stdlib.h>
#include <tabloader.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <widgetmgmt.h>


extern gconstpointer *global_data;


/*!
 \brief set_offline_mode() is called when the "Offline Mode" button is clicked
 in the general tab and is used to present the user with list of firmware 
 choices to select one for loading to work in offline mode (no connection to
 an ECU)
 */
G_MODULE_EXPORT gboolean set_offline_mode(void)
{
	GtkWidget * widget = NULL;
	gchar * filename = NULL;
	GArray *tests = NULL;
	GHashTable *tests_hash = NULL;
	gboolean tmp = TRUE;
	GModule *module = NULL;
	GArray *pfuncs = NULL;
	PostFunction *pf = NULL;
	extern Firmware_Details *firmware;
	extern gboolean interrogated;
        extern GAsyncQueue *io_repair_queue;


	/* Cause Serial Searcher thread to abort.... */
	if (io_repair_queue)
		g_async_queue_push(io_repair_queue,&tmp);

	gdk_threads_enter();
	filename = present_firmware_choices();
	if (!filename)
	{
		DATA_SET(global_data,"offline",GINT_TO_POINTER(FALSE));
		interrogated = FALSE;
		widget = lookup_widget("interrogate_button");
		if (GTK_IS_WIDGET(widget))
			gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);
		widget = lookup_widget("offline_button");
		if (GTK_IS_WIDGET(widget))
			gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);
		gdk_threads_leave();
		plugins_shutdown();
		gdk_threads_add_timeout(500,(GSourceFunc)personality_choice,NULL);

		return FALSE;
	}

	DATA_SET_FULL(global_data,"last_offline_profile",g_strdup(filename),g_free);
	DATA_SET(global_data,"offline",GINT_TO_POINTER(TRUE));
	interrogated = TRUE;

	/* Disable interrogation button */
	widget = lookup_widget("interrogate_button");
	if (GTK_IS_WIDGET(widget))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);

	queue_function("kill_conn_warning");

	tests = validate_and_load_tests(&tests_hash);
	if (!firmware)
	{
		firmware = g_new0(Firmware_Details,1);
		DATA_SET(global_data,"firmware",firmware);
	}
	load_firmware_details(firmware,filename);

	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	pfuncs = g_array_new(FALSE,TRUE,sizeof(PostFunction *));

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("update_interrogation_gui_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("load_realtime_map_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("load_status_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("load_rt_text_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("load_gui_tabs_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("load_sliders_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("start_statuscounts_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("disable_burner_buttons_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("offline_ecu_restore_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("setup_menu_handlers_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("enable_3d_buttons_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("ready_msg_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	
	g_module_close(module);

	io_cmd(NULL,pfuncs);

	/*
	io_cmd(firmware->get_all_command,NULL);
	*/

	widget = lookup_widget("interrogate_button");
	if (GTK_IS_WIDGET(widget))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
	widget = lookup_widget("offline_button");
	if (GTK_IS_WIDGET(widget))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
	g_list_foreach(get_list("get_data_buttons"),set_widget_sensitive,GINT_TO_POINTER(FALSE));

	free_tests_array(tests);

	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	pfuncs = g_array_new(FALSE,TRUE,sizeof(PostFunction *));

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("reset_temps_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	g_module_close(module);

	io_cmd(NULL,pfuncs);
	gdk_threads_leave();
	return FALSE;
}


/*!
 \brief present_firmware_choices() presents a dialog box with the firmware
 choices.
 \returns the name of the chosen firmware
 */
G_MODULE_EXPORT gchar * present_firmware_choices(void)
{
	gchar ** filenames = NULL;
	GtkWidget *dialog = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *ebox = NULL;
	GtkWidget *sep = NULL;
	GtkWidget *button = NULL;
	GtkWidget *label = NULL;
	ListElement *element = NULL;
	gchar *tmpbuf = NULL;
	GArray *classes = NULL;
	GSList *group = NULL;
	GList *p_list = NULL;
	GList *s_list = NULL;
	ConfigFile *cfgfile = NULL;
	const gchar * last_file = NULL;
	gint major = 0;
	gint minor = 0;
	guint i = 0;
	gint result = 0;
	extern gconstpointer *global_data;



	filenames = get_files(g_build_filename(INTERROGATOR_DATA_DIR,"Profiles",DATA_GET(global_data,"ecu_family"),NULL),g_strdup("prof"),&classes);
	if (!filenames)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": present_firmware_choices()\n\t NO Interrogation profiles found, was MegaTunix installed properly?\n\n"));
		return NULL;
	}
	i = 0;
	while (filenames[i]) 
	{
		cfgfile = cfg_open_file(filenames[i]);
		if (!cfgfile)
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": present_firmware_choices()\n\t Interrogation profile damaged!, was MegaTunix installed properly?\n\n"));
			i++;
			continue;
		}
		get_file_api(cfgfile,&major,&minor);
		if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
		{
			thread_update_logbar("interr_view","warning",g_strdup_printf(_("Interrogation profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,cfgfile->filename),FALSE,FALSE);
			i++;
			continue;
		}
		cfg_read_string(cfgfile,"interrogation_profile","name",&tmpbuf);
		cfg_free(cfgfile);
		last_file = DATA_GET(global_data,"last_offline_profile");

		if (g_array_index(classes,FileClass,i) == PERSONAL)
		{
			element = g_new0(ListElement, 1);
			element->filename = g_strdup(filenames[i]);
			if (g_strcasecmp(element->filename,last_file) == 0)
				element->def = TRUE;

			element->name = g_strdup(tmpbuf);
			p_list = g_list_append(p_list,(gpointer)element);
		}
		if (g_array_index(classes,FileClass,i) == SYSTEM)
		{
			element = g_new0(ListElement, 1);
			element->filename = g_strdup(filenames[i]);
			if (g_strcasecmp(element->filename,last_file) == 0)
				element->def = TRUE;
			element->name = g_strdup(tmpbuf);
			s_list = g_list_append(s_list,(gpointer)element);
		}
		g_free(tmpbuf);
		i++;
	}
	p_list = g_list_sort(p_list,list_sort);
	s_list = g_list_sort(s_list,list_sort);


	dialog = gtk_dialog_new_with_buttons("Select Firmware",
			GTK_WINDOW(lookup_widget("main_window")),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			"Abort",
			GTK_RESPONSE_CANCEL,
			"Load",
			GTK_RESPONSE_OK,
			NULL);

	vbox = gtk_vbox_new(TRUE,2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),vbox,TRUE,TRUE,0);
	group = NULL;
	/* Dummies */
	button = gtk_radio_button_new(group);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
	if (g_list_length(p_list) > 0)
	{
		label = gtk_label_new("Custom (personal) Profiles");
		gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,0);

		/* Cycle list for PERSONAL interogation files */
		for (i=0;i<g_list_length(p_list);i++)
		{
			element = g_list_nth_data(p_list,i);

			ebox = gtk_event_box_new();
			gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
			hbox = gtk_hbox_new(FALSE,10);
			gtk_container_add(GTK_CONTAINER(ebox),hbox);
			label = gtk_label_new(g_strdup(element->name));
			gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,TRUE,0);
			button = gtk_radio_button_new(group);
			g_free(OBJ_GET(button,"filename"));
			OBJ_SET(button,"filename",g_strdup(element->filename));
			OBJ_SET(button,"handler",
					GINT_TO_POINTER(OFFLINE_FIRMWARE_CHOICE));
			g_signal_connect(button,
					"toggled",
					G_CALLBACK(toggle_button_handler),
					NULL);
			gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,TRUE,0);
			if (element->def)
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),FALSE);
			group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
		}

		sep = gtk_hseparator_new();
		gtk_box_pack_start(GTK_BOX(vbox),sep,TRUE,TRUE,0);
	}
	label = gtk_label_new("System Wide ECU Profiles");
	gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,0);
	/* Cycle list for System interogation files */
	for (i=0;i<g_list_length(s_list);i++)
	{
		element = g_list_nth_data(s_list,i);
		ebox = gtk_event_box_new();
		gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
		hbox = gtk_hbox_new(FALSE,10);
		gtk_container_add(GTK_CONTAINER(ebox),hbox);
		label = gtk_label_new(g_strdup(element->name));
		gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,TRUE,0);
		button = gtk_radio_button_new(group);
		g_free(OBJ_GET(button,"filename"));
		OBJ_SET(button,"filename",g_strdup(element->filename));
		OBJ_SET(button,"handler",
				GINT_TO_POINTER(OFFLINE_FIRMWARE_CHOICE));
		g_signal_connect(button,
				"toggled",
				G_CALLBACK(toggle_button_handler),
				NULL);
		gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,TRUE,0);
		if (element->def)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		else
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),FALSE);
		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
	}
	/*
	   if (i==1)
	   gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(button));
	   else
	   gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),TRUE);
	 */

	g_strfreev(filenames);
	g_array_free(classes,TRUE);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	g_list_foreach(p_list,free_element,NULL);
	g_list_foreach(s_list,free_element,NULL);
	g_list_free(p_list);
	g_list_free(s_list);
	switch (result)
	{
		case GTK_RESPONSE_ACCEPT:
		case GTK_RESPONSE_OK:
			return DATA_GET(global_data,"offline_firmware_choice");
			break;
		case GTK_RESPONSE_CANCEL:
		default:
			return NULL;
	}
	return NULL;
}

G_MODULE_EXPORT gint ptr_sort(gconstpointer a, gconstpointer b)
{
	return strcmp((gchar *)a, (gchar *) b);
}


G_MODULE_EXPORT void offline_ecu_restore_pf(void)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	extern gboolean interrogated;
	extern Firmware_Details *firmware;

	if (!interrogated)
		return;

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_ecu_snapshots");
	fileio->default_path = g_strdup("ecu_snapshots");
	fileio->parent = lookup_widget("main_window");
	fileio->on_top = TRUE;
	fileio->title = g_strdup("You should load an ECU backup from a file");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	fileio->shortcut_folders = g_strdup("ecu_snapshots,../MTX_ecu_snapshots");
	fileio->default_filename = g_strdup(DATA_GET(global_data,"last_offline_filename"));

	gdk_threads_enter();
	filename = choose_file(fileio);
	if (filename)
	{
		DATA_SET_FULL(global_data,"last_offline_filename",g_strdup(filename),g_free);
		update_logbar("tools_view",NULL,_("Full Restore of ECU Initiated\n"),FALSE,FALSE,FALSE);
		restore_all_ecu_settings(filename);
		g_free(filename);
	}
	else
		io_cmd(firmware->get_all_command,NULL);
	free_mtxfileio(fileio);
	gdk_threads_leave();
	return;
}
