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

static gboolean restart_realtime = FALSE;


/*!
 \brief start_tickler() starts up a GTK+ timeout function based on the
 enum passed to it.
 \param type (TicklerType enum) is an enum passed which is used to know 
 which timeout to fire up.
 \see signal_read_rtvars_thread signal_read_rtvars
 */
G_MODULE_EXPORT void start_tickler(TicklerType type)
{
	extern gboolean rtvars_loaded;
	extern gboolean connected;
	extern gboolean interrogated;
	extern gconstpointer *global_data;
	gint id = 0;
	GThread *realtime_id = NULL;
	switch (type)
	{
		case RTV_TICKLER:
			if (DATA_GET(global_data,"offline"))
				break;
			if (!rtvars_loaded)
				break;
			if (restart_realtime)
			{
				update_logbar("comms_view",NULL,_("TTM is active, Realtime Reader suspended\n"),FALSE,FALSE,FALSE);
				break;
			}
			if (!DATA_GET(global_data,"realtime_id"))
			{
				flush_rt_arrays();

				realtime_id = g_thread_create(signal_read_rtvars_thread,
						NULL, /* Thread args */
						TRUE, /* Joinable */
						NULL); /*GError Pointer */
				DATA_SET(global_data,"realtime_id",realtime_id);
				update_logbar("comms_view",NULL,_("Realtime Reader started\n"),FALSE,FALSE,FALSE);
			}
			else
				update_logbar("comms_view","warning",_("Realtime Reader ALREADY started\n"),FALSE,FALSE,FALSE);
			break;
		case LV_PLAYBACK_TICKLER:
			if (!DATA_GET(global_data,"playback_id"))
			{
				id = gdk_threads_add_timeout((GINT)DATA_GET(global_data,"lv_scroll_delay"),(GSourceFunc)pb_update_logview_traces,GINT_TO_POINTER(FALSE));
				DATA_SET(global_data,"playback_id",GINT_TO_POINTER(id));
			}
			else
				dbg_func(CRITICAL,g_strdup(__FILE__": start_tickler()\n\tPlayback already running \n"));
			break;
		case SCOUNTS_TICKLER:
			if (DATA_GET(global_data,"offline"))
				break;
			if (!((connected) && (interrogated)))
				break;
			if (!DATA_GET(global_data,"statuscounts_id"))
			{
				id = g_timeout_add(100,(GSourceFunc)update_errcounts,NULL);
				DATA_SET(global_data,"statuscounts_id",GINT_TO_POINTER(id));
			}
			else
				dbg_func(CRITICAL,g_strdup(__FILE__": start_tickler()\n\tStatuscounts tickler already active \n"));
			break;
		default:
			/* Search for registered handlers from plugins */
			break;

	}
}


/*!
 \brief stop_tickler() kills off the GTK+ timeout for the specified handler 
 passed across in the ENUM
 /param TicklerType an enumeration used to determine which handler to stop.
 \see start_tickler
 */
G_MODULE_EXPORT void stop_tickler(TicklerType type)
{
	extern volatile gboolean leaving;
	extern GCond *rtv_thread_cond;
	extern gconstpointer *global_data;
	switch (type)
	{
		case RTV_TICKLER:
			if (DATA_GET(global_data,"realtime_id"))
			{
				g_cond_signal(rtv_thread_cond);
				update_logbar("comms_view",NULL,_("Realtime Reader stopped\n"),FALSE,FALSE,FALSE);
				DATA_SET(global_data,"realtime_id",NULL);
			}
			else
				update_logbar("comms_view","warning",_("Realtime Reader ALREADY stopped\n"),FALSE,FALSE,FALSE);

			if (!leaving)
				reset_runtime_status();
			break;

		case LV_PLAYBACK_TICKLER:
			if (DATA_GET(global_data,"playback_id"))
			{
				g_source_remove((gint)DATA_GET(global_data,"playback_id"));
				DATA_SET(global_data,"playback_id",GINT_TO_POINTER(0));
			}
			break;
		case SCOUNTS_TICKLER:
			if (DATA_GET(global_data,"statuscounts_id"))
			{
				g_source_remove((gint)DATA_GET(global_data,"statuscounts_id"));
				DATA_SET(global_data,"statuscounts_id",GINT_TO_POINTER(0));
			}
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
G_MODULE_EXPORT void * signal_read_rtvars_thread(gpointer data)
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
G_MODULE_EXPORT void signal_read_rtvars(void)
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
 \brief early interrogation() is called from a one shot timeout from main
 in order to start the interrogation process as soon as the gui is up and 
 running.
 */
G_MODULE_EXPORT gboolean early_interrogation(void)
{
	set_title(g_strdup(_("Initiating background ECU interrogation...")));
	update_logbar("interr_view","warning",_("Initiating background ECU interrogation...\n"),FALSE,FALSE,FALSE);
	io_cmd("interrogation",NULL);
	return FALSE;
}

