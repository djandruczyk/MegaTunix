/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <configfile.h>
#include <config.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <getfiles.h>
#include <gui_handlers.h>
#include <init.h>
#include <listmgmt.h>
#include <notifications.h>
#include <offline.h>
#include <personalities.h>
#include <plugin.h>
#include <threads.h>
#include <widgetmgmt.h>


extern gconstpointer *global_data;


/*!
 \brief personality_choice() is called from a one shot timeout from main
 in order to open the window to ask the user what ECU family to deal with
 running.
 */
G_MODULE_EXPORT gboolean personality_choice(void)
{
	GtkWidget *dialog = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *ebox = NULL;
	GtkWidget *sep = NULL;
	GtkWidget *button = NULL;
	GtkWidget *label = NULL;
	gchar ** dirs = NULL;
	gchar * filename = NULL;
	PersonaElement *element = NULL;
	gchar *tmpbuf = NULL;
	gboolean shouldjump = FALSE;
	gchar *name = NULL;
	GArray *classes = NULL;
	GSList *group = NULL;
	GList *p_list = NULL;
	GList *s_list = NULL;
	ConfigFile *cfgfile = NULL;
	guint i = 0;
	gint result = 0;
	extern gconstpointer *global_data;

	dirs = get_dirs(g_build_path(PSEP,INTERROGATOR_DATA_DIR,"Profiles",NULL),&classes);
	if (!dirs)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": personality_choice()\n\t NO Interrogation profiles found, was MegaTunix installed properly?\n\n"));
		return FALSE;
	}
	i = 0;
	while (dirs[i])
	{
		tmpbuf = g_strdup_printf("%s%s%s",dirs[i],PSEP,"details.cfg");
		cfgfile = cfg_open_file(tmpbuf);
		g_free(tmpbuf);
		if (!cfgfile)
		{
	//		dbg_func(CRITICAL,g_strdup_printf(__FILE__": personality_choice()\n\t \"details.cfg\" file missing!, was MegaTunix installed properly?\n\n"));
			i++;
			continue;

		}
		element = g_new0(PersonaElement, 1);
		cfg_read_string(cfgfile,"Family","sequence",&element->sequence);
		cfg_read_string(cfgfile,"Family","friendly_name",&element->name);
		cfg_read_string(cfgfile,"Family","persona",&element->persona);
		cfg_read_string(cfgfile,"Family","ecu_lib",&element->ecu_lib);
		cfg_read_string(cfgfile,"Family","common_lib",&element->common_lib);
		if (!cfg_read_string(cfgfile,"Family","baud",&element->baud_str))
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": personality_choice()\n\t \"details.cfg\" baud string undefined!, was MegaTunix installed properly?\n\n"));
		element->dirname = g_strdup(dirs[i]);
		element->filename = g_path_get_basename(dirs[i]);
		if (g_strcasecmp(element->filename,(gchar *)DATA_GET(global_data,"last_ecu_family")) == 0)
			element->def = TRUE;
		if ((DATA_GET(global_data,"cli_persona")) && (element->persona))
		{
			if (g_strcasecmp(element->persona, (gchar *)DATA_GET(global_data,"cli_persona")) == 0)
			{
				button = gtk_toggle_button_new();
				gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),TRUE);
				persona_selection(button,(gpointer)element);
				shouldjump = TRUE;
			}
		}

		if (g_array_index(classes,FileClass,i) == PERSONAL)
			p_list = g_list_append(p_list,(gpointer)element);
		if (g_array_index(classes,FileClass,i) == SYSTEM)
			s_list = g_list_append(s_list,(gpointer)element);
		g_free(name);
		i++;
		cfg_free(cfgfile);	
	}
	p_list = g_list_sort(p_list,persona_list_sort);
	s_list = g_list_sort(s_list,persona_list_sort);
	g_strfreev(dirs);
	g_array_free(classes,TRUE);
	if (shouldjump)
	{
		g_list_foreach(p_list,free_persona_element,NULL);
		g_list_foreach(s_list,free_persona_element,NULL);
		DATA_SET(global_data,"cli_persona",NULL);
		if (DATA_GET(global_data,"offline"))
			goto jumpahead_offline;
		else
			goto jumpahead;
	}

	set_title(g_strdup(_("Choose an ECU family?")));
	update_logbar("interr_view","warning",_("Prompting user for ECU family to interrogate...\n"),FALSE,FALSE,FALSE);

	dialog = gtk_dialog_new_with_buttons("Select ECU Personality",
			GTK_WINDOW(lookup_widget("main_window")),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			"Exit MegaTunix",
			GTK_RESPONSE_CLOSE,
			"Go Offline",
			GTK_RESPONSE_CANCEL,
			"Find my ECU",
			GTK_RESPONSE_OK,
			NULL);
	vbox = gtk_vbox_new(TRUE,2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),vbox,TRUE,TRUE,0);
	if (g_list_length(p_list) > 0)
	{
		label = gtk_label_new("Custom (personal) Profiles");
		gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,0);

		group = NULL;
		/* Cycle list for PERSONAL profile files */
		for (i=0;i<g_list_length(p_list);i++)
		{
			element = g_list_nth_data(p_list,i);

			ebox = gtk_event_box_new();
			gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
			hbox = gtk_hbox_new(FALSE,10);
			gtk_container_add(GTK_CONTAINER(ebox),hbox);
			label = gtk_label_new(element->name);
			gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,TRUE,0);
			if (!check_for_files (element->dirname,"prof"))
			{
				gtk_widget_set_sensitive(ebox,FALSE);
				button = gtk_radio_button_new(NULL);
			}
			else
			{
				button = gtk_radio_button_new(group);
				group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
			}
			g_signal_connect(button,
					"toggled",
					G_CALLBACK(persona_selection),
					element);
			gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,TRUE,0);
			if (element->def)
			{
				gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),TRUE);
				gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(button));
			}
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
		label = gtk_label_new(element->name);
		gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,TRUE,0);
		if (!check_for_files (element->dirname,"prof"))
		{
			gtk_widget_set_sensitive(ebox,FALSE);
			button = gtk_radio_button_new(NULL);
		}
		else
		{
			button = gtk_radio_button_new(group);
			group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
		}
		g_signal_connect(button,
				"toggled",
				G_CALLBACK(persona_selection),
				element);
		gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,TRUE,0);
		if (element->def)
		{
			gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),TRUE);
			gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(button));
		}
	}

	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	g_list_foreach(p_list,free_persona_element,NULL);
	g_list_foreach(s_list,free_persona_element,NULL);
	g_list_free(p_list);
	g_list_free(s_list);
	switch (result)
	{
		case GTK_RESPONSE_CLOSE:
			leave(NULL,NULL);
			break;
		case GTK_RESPONSE_ACCEPT:
		case GTK_RESPONSE_OK: /* Normal mode */
jumpahead:
			plugins_init();
			filename = get_file(g_build_filename(INTERROGATOR_DATA_DIR,"Profiles",DATA_GET(global_data,"ecu_family"),"comm.xml",NULL),NULL);
			load_comm_xml(filename);
			g_free(filename);
			io_cmd("interrogation",NULL);
			break;
		default: /* Offline */
jumpahead_offline:
			plugins_init();
			filename = get_file(g_build_filename(INTERROGATOR_DATA_DIR,"Profiles",DATA_GET(global_data,"ecu_family"),"comm.xml",NULL),NULL);
			load_comm_xml(filename);
			g_free(filename);
			g_timeout_add(100,(GSourceFunc)set_offline_mode,NULL);
			return FALSE;
	}
	return FALSE;
}


G_MODULE_EXPORT gboolean persona_selection(GtkWidget *widget, gpointer data)
{
	PersonaElement *element = (PersonaElement *)data;

	if (!element)
		return FALSE;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{
		if (element->baud_str)
			DATA_SET_FULL(global_data,"ecu_baud_str", g_strdup(element->baud_str),g_free);
		else
			DATA_SET(global_data,"ecu_baud_str", NULL);
		if (element->ecu_lib)
			DATA_SET_FULL(global_data,"ecu_lib", g_strdup(element->ecu_lib),g_free);
		else
			DATA_SET(global_data,"ecu_lib", NULL);
		if (element->common_lib)
			DATA_SET_FULL(global_data,"common_lib", g_strdup(element->common_lib),g_free);
		else
			DATA_SET(global_data,"common_lib", NULL);
		if (element->dirname)
			DATA_SET_FULL(global_data,"ecu_dirname", g_strdup(element->dirname),g_free);
		else
			DATA_SET(global_data,"ecu_dirname", NULL);
		if (element->filename)
			DATA_SET_FULL(global_data,"ecu_family", g_strdup(element->filename),g_free);
		else
			DATA_SET(global_data,"ecu_family", NULL);
	}
	return TRUE;
}


G_MODULE_EXPORT void free_persona_element(gpointer data, gpointer user_data)
{
	PersonaElement *e = (PersonaElement *)data;
	cleanup(e->filename);
	cleanup(e->dirname);
	cleanup(e->name);
	cleanup(e->common_lib);
	cleanup(e->ecu_lib);
	cleanup(e->baud_str);
	cleanup(e);
}


G_MODULE_EXPORT gint persona_list_sort(gconstpointer a, gconstpointer b)
{
	PersonaElement *a1 = (PersonaElement *)a;
	PersonaElement *b1 = (PersonaElement *)b;
	return g_ascii_strcasecmp(a1->sequence,b1->sequence);
}
