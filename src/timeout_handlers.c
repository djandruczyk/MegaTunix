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
#include <gui_handlers.h>
#include <init.h>
#include <runtime_gui.h>
#include <logviewer_gui.h>
#include <notifications.h>
#include <rtv_processor.h>
#include <serialio.h>
#include <timeout_handlers.h>
#include <threads.h>

gint realtime_id = 0;
gint playback_id = 0;
gint toothmon_id = 0;
gint statuscounts_id = 0;
static gint trigmon_id = 0;
static gboolean restart_realtime = FALSE;

extern gint dbg_lvl;
extern GObject *global_data;

/*!
 \brief start_tickler() starts up a GTK+ timeout function based on the
 enum passed to it.
 \param type (TicklerType enum) is an enum passed which is used to know 
 which timeout to fire up.
 \see signal_read_rtvars
 */
void start_tickler(TicklerType type)
{
	extern Serial_Params *serial_params;
	extern volatile gboolean offline;
	extern gboolean rtvars_loaded;
	extern gboolean connected;
	extern gboolean interrogated;
	switch (type)
	{
		case RTV_TICKLER:
			if (offline)
				break;
			if (!rtvars_loaded)
				break;
			if (restart_realtime)
			{
				update_logbar("comms_view",NULL,g_strdup("TTM is active, Realtime Reader suspended\n"),FALSE,FALSE);
				break;
			}
			if (realtime_id == 0)
			{
				flush_rt_arrays();
				realtime_id = g_timeout_add(serial_params->read_wait,(GtkFunction)signal_read_rtvars,NULL);
				update_logbar("comms_view",NULL,g_strdup("Realtime Reader started\n"),FALSE,FALSE);
			}
			else
				update_logbar("comms_view","warning",g_strdup("Realtime Reader ALREADY started\n"),FALSE,FALSE);
			break;
		case LV_PLAYBACK_TICKLER:
			if (playback_id == 0)
				playback_id = g_timeout_add((gint)OBJ_GET(global_data,"lv_scroll_delay"),(GtkFunction)pb_update_logview_traces,GINT_TO_POINTER(FALSE));
			else
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": start_tickler()\n\tPlayback already running \n"));
			}
			break;
		case TOOTHMON_TICKLER:
			if (offline)
				break;
			if (realtime_id != 0)
			{
				/* TTM and Realtime are mutulally exclusive,
				 * and TTM takes precedence,  so disabled 
				 * realtime, and manually fire it once per
				 * TTM read so the gauges will still update
				 */
				g_source_remove(realtime_id);
				restart_realtime = TRUE;
				realtime_id = 0;
			}
			if (toothmon_id == 0)
			{
				signal_toothtrig_read(TOOTHMON_TICKLER);
				toothmon_id = g_timeout_add(3000,(GtkFunction)signal_toothtrig_read,GINT_TO_POINTER(TOOTHMON_TICKLER));
			}
			else
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": start_tickler()\n\tTrigmon tickler already active \n"));
			}
			break;
		case TRIGMON_TICKLER:
			if (offline)
				break;
			if (realtime_id != 0)
			{
				/* TTM and Realtime are mutulally exclusive,
				 * and TTM takes precedence,  so disabled 
				 * realtime, and manually fire it once per
				 * TTM read so the gauges will still update
				 */
				g_source_remove(realtime_id);
				restart_realtime = TRUE;
				realtime_id = 0;
			}
			if (trigmon_id == 0)
			{
				signal_toothtrig_read(TRIGMON_TICKLER);
				trigmon_id = g_timeout_add(750,(GtkFunction)signal_toothtrig_read,GINT_TO_POINTER(TRIGMON_TICKLER));
			}
			else
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": start_tickler()\n\tTrigmon tickler already active \n"));
			}
			break;
		case SCOUNTS_TICKLER:
			if (offline)
				break;
			if (!((connected) && (interrogated)))
				break;
			if (statuscounts_id == 0)
				statuscounts_id = g_timeout_add(100,(GtkFunction)update_errcounts,NULL);
			else
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": start_tickler()\n\tStatuscounts tickler already active \n"));
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
	extern Serial_Params *serial_params;
	extern volatile gboolean leaving;
	switch (type)
	{
		case RTV_TICKLER:
			if (realtime_id)
			{
				g_source_remove(realtime_id);
				update_logbar("comms_view",NULL,g_strdup("Realtime Reader stopped\n"),FALSE,FALSE);
				realtime_id = 0;
			}
			else
				update_logbar("comms_view","warning",g_strdup("Realtime Reader ALREADY stopped\n"),FALSE,FALSE);

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

	}
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
	OutputData *output = NULL;
	extern Firmware_Details *firmware;
	extern GAsyncQueue *io_queue;
	//extern GAsyncQueue *pf_dispatch_queue;
	//extern GAsyncQueue *gui_dispatch_queue;
	extern gboolean rtvars_loaded;

	if (!rtvars_loaded)
		return TRUE;

	length = g_async_queue_length(io_queue);

	/* If queue depth is too great we should not make the problem worse
	 * so we skip a call as we're probably trying to go faster than the 
	 * MS and/or serial port can go....
	 */
	//printf("Queue length is %i requests long\n",length);
	//printf("PF Dispatch queue length is %i requests long\n", g_async_queue_length(pf_dispatch_queue));
	//printf("Gui Dispatch queue length is %i requests long\n", g_async_queue_length(gui_dispatch_queue));

	if (length > 2)
		return TRUE;

	if (dbg_lvl & (SERIAL_RD|SERIAL_WR))
		dbg_func(g_strdup(__FILE__": signal_read_rtvars()\n\tsending message to thread to read RT vars\n"));

	if (firmware->capabilities & MS2)
	{
		output = initialize_outputdata();
		OBJ_SET(output->object,"canID", GINT_TO_POINTER(firmware->canID));
		OBJ_SET(output->object,"page", GINT_TO_POINTER(firmware->ms2_rt_page));
		OBJ_SET(output->object,"truepgnum", GINT_TO_POINTER(firmware->ms2_rt_page));
		OBJ_SET(output->object,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
		io_cmd(firmware->rt_command,output);			
	}
	else
		io_cmd(firmware->rt_command,NULL);			
	return TRUE;	/* Keep going.... */
}


/*!
 \brief signal_toothtrig_read() is called by a GTK+ timeout on a periodic basis
 to get a new set of toother or ignition trigger data.  It does so by queing 
 messages to a thread which handles I/O.  
 \returns TRUE
 */
gboolean signal_toothtrig_read(TicklerType type)
{
	if (dbg_lvl & (SERIAL_RD|SERIAL_WR))
		dbg_func(g_strdup(__FILE__": signal_toothtrig_read()\n\tsending message to thread to read ToothTrigger data\n"));

	/* Make the gauges stay up to date,  even if rather slowly 
	 * Also gets us access to current RPM and other vars for calculating 
	 * data from the TTM results
	 */
	signal_read_rtvars();
	switch (type)
	{
		case TOOTHMON_TICKLER:
			io_cmd("ms1_e_read_toothmon",NULL);
			break;
		case TRIGMON_TICKLER:
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
	set_title(g_strdup("Initiating background ECU interrogation..."));
	update_logbar("interr_view","warning",g_strdup("Initiating background ECU interrogation...\n"),FALSE,FALSE);
	io_cmd("interrogation",NULL);
	return FALSE;
}
