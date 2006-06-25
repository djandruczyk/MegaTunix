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
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <runtime_gui.h>
#include <logviewer_gui.h>
#include <notifications.h>
#include <rtv_processor.h>
#include <structures.h>
#include <timeout_handlers.h>
#include <threads.h>

gint realtime_id = 0;
gint playback_id = 0;


/*!
 \brief start_realtime_tickler() starts up a GTK+ timeout function to run
 the signal_read_rtvars function on a periodic time schedule
 \see signal_read_rtvars
 */
void start_realtime_tickler()
{
	extern struct Serial_Params *serial_params;

	if (realtime_id == 0)
	{
		flush_rt_arrays();
		realtime_id = g_timeout_add(serial_params->read_wait,
				(GtkFunction)signal_read_rtvars,NULL);
		update_logbar("comms_view",NULL,g_strdup("Realtime Reader started\n"),TRUE,FALSE);
	}
	else
		update_logbar("comms_view","warning",g_strdup("Realtime Reader ALREADY started\n"),TRUE,FALSE);
}


/*!
 \brief start_logviewer_playback() kicks off a GTK+ timeout to update the
 logviewer display on a periodic basis.
 \see update_logview_traces
 \see stop_logviewer_playback
 */
void start_logviewer_playback()
{

	if (playback_id == 0)
		playback_id = g_timeout_add(33,(GtkFunction)pb_update_logview_traces,GINT_TO_POINTER(FALSE));
	else
		dbg_func(g_strdup(__FILE__": start_logviewer_playback()\n\tPlayback already running \n"),CRITICAL);
}


/*!
 \brief stop_logviewer_playback() kills off the GTK+ timeout that updates the
 logviewer display on a periodic basis.
 \see update_logview_traces
 \see start_logviewer_playback
 */
void stop_logviewer_playback()
{
	if (playback_id)
	{
		g_source_remove(playback_id);
		playback_id = 0;
	}
}


/*!
 \brief stop_realtime_tickler() kills off the GTK+ timeout that gets the
 realtime variables on a periodic basis.
 \see start_realtime_tickler
 */
void stop_realtime_tickler()
{
	extern GAsyncQueue *io_queue;
	extern gint dispatcher_id;
	extern gboolean leaving;

	if (realtime_id)
	{
		g_source_remove(realtime_id);
		update_logbar("comms_view",NULL,g_strdup("Realtime Reader stopped\n"),TRUE,FALSE);
		realtime_id = 0;
		while (leaving && ((g_async_queue_length(io_queue) > 0) || (dispatcher_id != 0)))
		{
		//	printf("waiting for queue to finish\n");
			g_usleep(10000);
		}
	}
	else
		update_logbar("comms_view","warning",g_strdup("Realtime Reader ALREADY stopped\n"),TRUE,FALSE);

	reset_runtime_status();
}


/*!
 \brief signal_read_rtvars() is called by a GTK+ timeout on a periodic basis
 to get a new set of realtiem variables.  It does so by queing messages to
 a thread which handles I/O.  This function will check the queue depth and 
 if the queue is backed up it will skip sending a request for data, as that 
 will only aggravate the queue roadblock.
 \returns TRUE
 */
gboolean signal_read_rtvars()
{
	gint length = 0;
	extern GAsyncQueue *io_queue;
	extern gboolean rtvars_loaded;
	static gint errcount = 0;

	length = g_async_queue_length(io_queue);
	/* IF queue depth is too great we should not make the problem worse
	 * so we skip a call as we're probably trying to go faster than the 
	 * MS and/or serail port can go....
	 */
	if (length > 2)
		return TRUE;

	dbg_func(g_strdup(__FILE__": signal_read_rtvars()\n\tsending message to thread to read RT vars\n"),SERIAL_RD|SERIAL_WR);

	if (errcount > 10)
	{
		stop_realtime_tickler();
		errcount = 0;
	}
	if (!rtvars_loaded)
		return TRUE;
//	if (connected)
		io_cmd(IO_REALTIME_READ,NULL);			
		/*
	else
	{	
		errcount++;
		dbg_func(g_strdup(__FILE__": signal_read_rtvars()\n\tNOT connected, not queing message to thread handler....\n"),CRITICAL);

	}
	*/
	return TRUE;	/* Keep going.... */
}


/*!
 \brief early interrogation() is called from a one shot timeout from main
 in order to start the interrogation process as soon as the gui is up and 
 running.
 */
gboolean early_interrogation()
{
	set_title(g_strdup("Initiating background ECU interrogation..."));
	update_logbar("interr_view","warning",g_strdup("Initiating background ECU interrogation...\n"),FALSE,FALSE);
	io_cmd(IO_INTERROGATE_ECU,NULL);
	return FALSE;
}

