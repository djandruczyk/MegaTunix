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
#include <comms_gui.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <getfiles.h>
#include <gui_handlers.h>
#include <init.h>
#include <listmgmt.h>
#include <offline.h>
#include <runtime_gui.h>
#include <logviewer_gui.h>
#include <notifications.h>
#include <rtv_processor.h>
#include <serialio.h>
#include <stringmatch.h>
#include <timeout_handlers.h>
#include <threads.h>
#include <widgetmgmt.h>

GThread *realtime_id = NULL;
gint playback_id = 0;
gint toothmon_id = 0;
gint statuscounts_id = 0;
static gint trigmon_id = 0;
static gboolean restart_realtime = FALSE;

static gboolean check_for_files(const gchar * path, const gchar *ext);

/*!
 \brief start_tickler() starts up a GTK+ timeout function based on the
 enum passed to it.
 \param type (TicklerType enum) is an enum passed which is used to know 
 which timeout to fire up.
 \see signal_read_rtvars_thread signal_read_rtvars
 */
void start_tickler(TicklerType type)
{
	extern volatile gboolean offline;
	extern gboolean rtvars_loaded;
	extern gboolean connected;
	extern gboolean interrogated;
	extern GCond *rtv_thread_cond;
	extern gconstpointer *global_data;
	switch (type)
	{
		case RTV_TICKLER:
			if (offline)
				break;
			if (!rtvars_loaded)
				break;
			if (restart_realtime)
			{
				update_logbar("comms_view",NULL,_("TTM is active, Realtime Reader suspended\n"),FALSE,FALSE,FALSE);
				break;
			}
			if (!realtime_id)
			{
				flush_rt_arrays();

				realtime_id = g_thread_create(signal_read_rtvars_thread,
						NULL, /* Thread args */
						TRUE, /* Joinable */
						NULL); /*GError Pointer */
				update_logbar("comms_view",NULL,_("Realtime Reader started\n"),FALSE,FALSE,FALSE);
			}
			else
				update_logbar("comms_view","warning",_("Realtime Reader ALREADY started\n"),FALSE,FALSE,FALSE);
			break;
		case LV_PLAYBACK_TICKLER:
			if (playback_id == 0)
				playback_id = gdk_threads_add_timeout((GINT)DATA_GET(global_data,"lv_scroll_delay"),(GSourceFunc)pb_update_logview_traces,GINT_TO_POINTER(FALSE));
			else
				dbg_func(CRITICAL,g_strdup(__FILE__": start_tickler()\n\tPlayback already running \n"));
			break;
		case TOOTHMON_TICKLER:
			if (offline)
				break;
			if (realtime_id)
			{
				/* TTM and Realtime are mutulally exclusive,
				 * and TTM takes precedence,  so disabled 
				 * realtime, and manually fire it once per
				 * TTM read so the gauges will still update
				 */
				g_cond_signal(rtv_thread_cond);
				restart_realtime = TRUE;
				realtime_id = NULL;
			}
			if (toothmon_id == 0)
			{
				signal_toothtrig_read(TOOTHMON_TICKLER);
				toothmon_id = g_timeout_add(3000,(GSourceFunc)signal_toothtrig_read,GINT_TO_POINTER(TOOTHMON_TICKLER));
			}
			else
				dbg_func(CRITICAL,g_strdup(__FILE__": start_tickler()\n\tTrigmon tickler already active \n"));
			break;
		case TRIGMON_TICKLER:
			if (offline)
				break;
			if (realtime_id)
			{
				/* TTM and Realtime are mutually exclusive,
				 * and TTM takes precedence,  so disabled 
				 * realtime, and manually fire it once per
				 * TTM read so the gauges will still update
				 */
				g_cond_signal(rtv_thread_cond);
				restart_realtime = TRUE;
				realtime_id = NULL;
			}
			if (trigmon_id == 0)
			{
				signal_toothtrig_read(TRIGMON_TICKLER);
				trigmon_id = g_timeout_add(750,(GSourceFunc)signal_toothtrig_read,GINT_TO_POINTER(TRIGMON_TICKLER));
			}
			else
				dbg_func(CRITICAL,g_strdup(__FILE__": start_tickler()\n\tTrigmon tickler already active \n"));
			break;
		case SCOUNTS_TICKLER:
			if (offline)
				break;
			if (!((connected) && (interrogated)))
				break;
			if (statuscounts_id == 0)
				statuscounts_id = g_timeout_add(100,(GSourceFunc)update_errcounts,NULL);
			else
				dbg_func(CRITICAL,g_strdup(__FILE__": start_tickler()\n\tStatuscounts tickler already active \n"));
			break;
		default:
			break;

	}
}


/*!
 \brief stop_tickler() kills off the GTK+ timeout for the specified handler 
 passed across in the ENUM
 /param TicklerType an enumeration used to determine which handler to stop.
 \see start_tickler
 */
void stop_tickler(TicklerType type)
{
	extern volatile gboolean leaving;
	extern GCond *rtv_thread_cond;
	switch (type)
	{
		case RTV_TICKLER:
			if (realtime_id)
			{
				g_cond_signal(rtv_thread_cond);
				update_logbar("comms_view",NULL,_("Realtime Reader stopped\n"),FALSE,FALSE,FALSE);
				realtime_id = NULL;
			}
			else
				update_logbar("comms_view","warning",_("Realtime Reader ALREADY stopped\n"),FALSE,FALSE,FALSE);

			if (!leaving)
				reset_runtime_status();
			break;

		case LV_PLAYBACK_TICKLER:
			if (playback_id)
			{
				g_source_remove(playback_id);
				playback_id = 0;
			}
			break;
		case TOOTHMON_TICKLER:
			if (toothmon_id)
			{
				g_source_remove(toothmon_id);
				toothmon_id = 0;
			}
			if (restart_realtime)
			{
				restart_realtime = FALSE;
				start_tickler(RTV_TICKLER);
			}
			break;
		case TRIGMON_TICKLER:
			if (trigmon_id)
			{
				g_source_remove(trigmon_id);
				trigmon_id = 0;
			}
			if (restart_realtime)
			{
				restart_realtime = FALSE;
				start_tickler(RTV_TICKLER);
			}
			break;
		case SCOUNTS_TICKLER:
			if (statuscounts_id)
				g_source_remove(statuscounts_id);
			statuscounts_id = 0;
			break;
		default:
			break;
	}
}



/*!
 \brief signal_read_rtvars_thread() is thread which fires off the read msg
 to get a new set of realtiem variables.  It does so by queing messages to
 a thread which handles I/O.  This function will check the queue depth and 
 if the queue is backed up it will skip sending a request for data, as that 
 will only aggravate the queue roadblock.
 \returns 0 on signal to exit
 */
void * signal_read_rtvars_thread(gpointer data)
{
	extern Serial_Params *serial_params;
	extern GAsyncQueue *io_data_queue;
	extern GAsyncQueue *pf_dispatch_queue;
	extern GCond *rtv_thread_cond;
	GMutex * mutex = g_mutex_new();
	GTimeVal time;

	g_mutex_lock(mutex);
	while (TRUE)
	{

		/* Auto-throttling if gui gets sluggish */
		while (( g_async_queue_length(io_data_queue) > 2) || 
				(g_async_queue_length(pf_dispatch_queue) > 8))
			g_usleep(5000);


		dbg_func(IO_MSG|THREADS,g_strdup(__FILE__": signal_read_rtvars_thread()\n\tsending message to thread to read RT vars\n"));
		signal_read_rtvars();

		g_get_current_time(&time);
		g_time_val_add(&time,serial_params->read_wait*1000);
		if (g_cond_timed_wait(rtv_thread_cond,mutex,&time))
		{
			g_mutex_unlock(mutex);
			g_mutex_free(mutex);
			g_thread_exit(0);
		}
	}
}


/*!
 \brief signal_read_rtvars() sends io message to I/O core to tell ms to send 
 back runtime vars
 */
void signal_read_rtvars(void)
{
	OutputData *output = NULL;
	extern Firmware_Details *firmware;

	if (firmware->capabilities & MS2)
	{
		output = initialize_outputdata();
		DATA_SET(output->data,"canID", GINT_TO_POINTER(firmware->canID));
		DATA_SET(output->data,"page", GINT_TO_POINTER(firmware->ms2_rt_page));
		DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->ms2_rt_page));
		DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
		io_cmd(firmware->rt_command,output);			
	}
	else
		io_cmd(firmware->rt_command,NULL);			
}


/*!
 \brief signal_toothtrig_read() is called by a GTK+ timeout on a periodic basis
 to get a new set of toother or ignition trigger data.  It does so by queing 
 messages to a thread which handles I/O.  
 \returns TRUE
 */
gboolean signal_toothtrig_read(TicklerType type)
{
	extern Firmware_Details *firmware;
	dbg_func(IO_MSG,g_strdup(__FILE__": signal_toothtrig_read()\n\tsending message to thread to read ToothTrigger data\n"));

	/* Make the gauges stay up to date,  even if rather slowly 
	 * Also gets us access to current RPM and other vars for calculating 
	 * data from the TTM results
	 */
	signal_read_rtvars();
	switch (type)
	{
		case TOOTHMON_TICKLER:
			if (firmware->capabilities & MSNS_E)
				io_cmd("ms1_e_read_toothmon",NULL);
			break;
		case TRIGMON_TICKLER:
			if (firmware->capabilities & MSNS_E)
				io_cmd("ms1_e_read_trigmon",NULL);
			break;
		default:
			break;
	}
	return TRUE;	/* Keep going.... */
}


/*!
 \brief early interrogation() is called from a one shot timeout from main
 in order to start the interrogation process as soon as the gui is up and 
 running.
 */
gboolean early_interrogation()
{
	set_title(g_strdup(_("Initiating background ECU interrogation...")));
	update_logbar("interr_view","warning",_("Initiating background ECU interrogation...\n"),FALSE,FALSE,FALSE);
	io_cmd("interrogation",NULL);
	return FALSE;
}

/*!
 \brief personality_choice() is called from a one shot timeout from main
 in order to open the window to ask the user what ECU family to deal with
 running.
 */
gboolean personality_choice()
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
	ListElement *element = NULL;
	gchar *tmpbuf = NULL;
	gchar *name = NULL;
	GArray *classes = NULL;
	GSList *group = NULL;
	GList *p_list = NULL;
	GList *s_list = NULL;
	ConfigFile *cfgfile = NULL;
	guint i = 0;
	gint result = 0;
	extern gconstpointer *global_data;

	dirs = get_dirs(g_strconcat(INTERROGATOR_DATA_DIR,PSEP,"Profiles",PSEP,NULL),&classes);
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
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": personality_choice()\n\t \"details.cfg\" file missing!, was MegaTunix installed properly?\n\n"));
			i++;
			continue;

		}
		element = g_new0(ListElement, 1);
		cfg_read_string(cfgfile,"Family","friendly_name",&element->name);
		cfg_read_int(cfgfile,"Family","baud",&element->baud);
		element->dirname = g_strdup(dirs[i]);
		element->filename = g_path_get_basename(dirs[i]);
		if (g_strcasecmp(element->filename,(gchar *)DATA_GET(global_data,"previous_ecu_family")) == 0)
			element->def = TRUE;

		if (g_array_index(classes,FileClass,i) == PERSONAL)
			p_list = g_list_append(p_list,(gpointer)element);
		if (g_array_index(classes,FileClass,i) == SYSTEM)
			s_list = g_list_append(s_list,(gpointer)element);
		g_free(name);
		i++;
		cfg_free(cfgfile);	
	}
	p_list = g_list_sort(p_list,list_sort);
	s_list = g_list_sort(s_list,list_sort);

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
			OBJ_SET(button,"ecu_persona",element);
			OBJ_SET(button,"handler",
					GINT_TO_POINTER(ECU_PERSONA));
			g_signal_connect(button,
					"toggled",
					G_CALLBACK(toggle_button_handler),
					NULL);
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
		label = gtk_label_new(g_strdup(element->name));
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
		OBJ_SET(button,"ecu_persona",element);
		OBJ_SET(button,"handler",
				GINT_TO_POINTER(ECU_PERSONA));
		g_signal_connect(button,
				"toggled",
				G_CALLBACK(toggle_button_handler),
				NULL);
		gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,TRUE,0);
		if (element->def)
		{
			gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),TRUE);
			gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(button));
		}
	}

	g_strfreev(dirs);
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
		case GTK_RESPONSE_CLOSE:
			leave(NULL,NULL);
			break;
		case GTK_RESPONSE_ACCEPT:
		case GTK_RESPONSE_OK:
			filename = get_file(g_build_filename(INTERROGATOR_DATA_DIR,"Profiles",DATA_GET(global_data,"ecu_family"),"comm.xml",NULL),NULL);
			printf("filename to load is %s\n",filename);
			load_comm_xml(filename);
			g_free(filename);
			io_cmd("interrogation",NULL);
			break;
		default:
			filename = get_file(g_build_filename(INTERROGATOR_DATA_DIR,"Profiles",DATA_GET(global_data,"ecu_family"),"comm.xml",NULL),NULL);
			load_comm_xml(filename);
			g_free(filename);
                        g_timeout_add(100,(GSourceFunc)set_offline_mode,NULL);
			return FALSE;
	}

	return FALSE;
}


gboolean check_for_files(const gchar * path, const gchar *ext)
{
	GDir * dir = NULL;
	const gchar * file = NULL;

	dir=g_dir_open(path,0,NULL);
	if (!dir)
		return FALSE;
	while ((file = g_dir_read_name(dir)))
	{
		if (g_str_has_suffix(file,ext))
		{
			g_dir_close(dir);
			return TRUE;
		}
	}
	g_dir_close(dir);
	return FALSE;
}
