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

#include <comms_gui.h>
#include <debugging.h>
#include <logviewer_gui.h>
#include <notifications.h>
#include <plugin.h>
#include <rtv_processor.h>
#include <runtime_status.h>
#include <serialio.h>
#include <threads.h>
#include <timeout_handlers.h>

extern gconstpointer *global_data;

/*!
 \brief start_tickler() starts up a GTK+ timeout function based on the
 enum passed to it.
 \param type, is an enum passed which is used to know 
 which timeout to fire up.
 \see signal_read_rtvars_thread signal_read_rtvars
 */
G_MODULE_EXPORT void start_tickler(TicklerType type)
{
	gint id = 0;
	GThread *realtime_id = NULL;
	switch (type)
	{
		case RTV_TICKLER:
			if (DATA_GET(global_data,"offline"))
				break;
			if (!DATA_GET(global_data,"rtvars_loaded"))
				break;
			if (DATA_GET(global_data,"restart_realtime"))
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
			if (!((DATA_GET(global_data,"connected")) && 
						(DATA_GET(global_data,"interrogated"))))
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
 /param type, an enumeration used to determine which handler to stop.
 \see start_tickler
 */
G_MODULE_EXPORT void stop_tickler(TicklerType type)
{
	GCond *rtv_thread_cond = NULL;
	GMutex *rtv_thread_mutex = NULL;
	rtv_thread_cond = DATA_GET(global_data,"rtv_thread_cond");
	rtv_thread_mutex = DATA_GET(global_data,"rtv_thread_mutex");
	g_return_if_fail(rtv_thread_mutex);

	switch (type)
	{
		case RTV_TICKLER:
			if (DATA_GET(global_data,"realtime_id"))
			{
				DATA_SET(global_data,"realtime_id",NULL);
				g_mutex_lock(rtv_thread_mutex);
				g_cond_signal(rtv_thread_cond);
				g_mutex_unlock(rtv_thread_mutex);
				update_logbar("comms_view",NULL,_("Realtime Reader stopped\n"),FALSE,FALSE,FALSE);
				DATA_SET(global_data,"realtime_id",NULL);
			}
			else
				update_logbar("comms_view",NULL,_("Realtime Reader ALREADY stopped\n"),FALSE,FALSE,FALSE);

			if (!DATA_GET(global_data,"leaving"))
				reset_runtime_status();
			break;

		case LV_PLAYBACK_TICKLER:
			if (DATA_GET(global_data,"playback_id"))
			{
				g_source_remove((GINT)DATA_GET(global_data,"playback_id"));
				DATA_SET(global_data,"playback_id",GINT_TO_POINTER(0));
			}
			break;
		case SCOUNTS_TICKLER:
			if (DATA_GET(global_data,"statuscounts_id"))
			{
				g_source_remove((GINT)DATA_GET(global_data,"statuscounts_id"));
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
 \param data, unused
 \returns 0 on signal to exit
 */
G_MODULE_EXPORT void * signal_read_rtvars_thread(gpointer data)
{
	static void (*signal_read_rtvars)(void);
	static gboolean (*setup_rtv)(void);
	static gboolean (*teardown_rtv)(void);
	Serial_Params *serial_params;
	GMutex * mutex = g_mutex_new();
	GTimeVal time;
	GAsyncQueue *io_data_queue = NULL;
	GAsyncQueue *pf_dispatch_queue = NULL;
	GCond *rtv_thread_cond = NULL;
	GMutex *rtv_thread_mutex = NULL;

	g_mutex_lock(mutex);
	serial_params = DATA_GET(global_data,"serial_params");
	io_data_queue = DATA_GET(global_data,"io_data_queue");
	pf_dispatch_queue = DATA_GET(global_data,"pf_dispatch_queue");
	rtv_thread_cond = DATA_GET(global_data,"rtv_thread_cond");
	rtv_thread_mutex = DATA_GET(global_data,"rtv_thread_mutex");
	get_symbol("signal_read_rtvars",(void *)&signal_read_rtvars);
	get_symbol("setup_rtv",(void *)&setup_rtv);
	get_symbol("teardown_rtv",(void *)&teardown_rtv);

	g_return_val_if_fail(serial_params,NULL);
	g_return_val_if_fail(signal_read_rtvars,NULL);
	g_return_val_if_fail(io_data_queue,NULL);
	g_return_val_if_fail(pf_dispatch_queue,NULL);
	g_return_val_if_fail(rtv_thread_cond,NULL);
	g_return_val_if_fail(rtv_thread_mutex,NULL);
	g_return_val_if_fail(setup_rtv,NULL);
	g_return_val_if_fail(teardown_rtv,NULL);

	if (!setup_rtv())
	{
		g_mutex_unlock(mutex);
		g_mutex_free(mutex);
		g_thread_exit(NULL);
	}
	g_async_queue_ref(io_data_queue);
	g_async_queue_ref(pf_dispatch_queue);
	g_mutex_lock(rtv_thread_mutex);
	while (TRUE)
	{
		dbg_func(IO_MSG|THREADS,g_strdup(__FILE__": signal_read_rtvars_thread()\n\tsending message to thread to read RT vars\n"));

		signal_read_rtvars();

		/* Auto-throttling if gui gets sluggish */
		while (( g_async_queue_length(io_data_queue) > 2) || 
				(g_async_queue_length(pf_dispatch_queue) > 3))
		{
			g_get_current_time(&time);
			g_time_val_add(&time,1000*g_async_queue_length(pf_dispatch_queue));
			if (g_cond_timed_wait(rtv_thread_cond,rtv_thread_mutex,&time))
				goto breakout;
		}
		g_get_current_time(&time);
		g_time_val_add(&time,serial_params->read_wait*1000);
		if (g_cond_timed_wait(rtv_thread_cond,rtv_thread_mutex,&time))
			goto breakout;
	}
breakout:
	g_async_queue_unref(io_data_queue);
	g_async_queue_unref(pf_dispatch_queue);
	g_mutex_unlock(mutex);
	g_mutex_free(mutex);
	g_mutex_unlock(rtv_thread_mutex);
	teardown_rtv();
	g_thread_exit(0);
	return NULL;
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


/*!
  \brief Handler to fire off if this is the first time MtX has run
  \returns FALSE
  */
G_MODULE_EXPORT gboolean check_for_first_time(void)
{
	if (DATA_GET(global_data,"first_time"))
		printf("should run first_time_wizard\n");
        gdk_threads_add_timeout(100,(GSourceFunc)personality_choice,NULL);
	return FALSE;

}
