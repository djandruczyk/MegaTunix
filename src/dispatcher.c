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

#include <comms.h>
#include <config.h>
#include <conversions.h>
#include <dataio.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <dispatcher.h>
#include <enums.h>
#include <gui_handlers.h>
#include <interrogate.h>
#include <logviewer_gui.h>
#include <notifications.h>
#include <post_process.h>
#include <runtime_sliders.h>
#include <runtime_gui.h>
#include <rtv_map_loader.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <structures.h>
#include <tabloader.h>
#include <threads.h>
#include <unistd.h>


extern GAsyncQueue *dispatch_queue;
extern gboolean connected;			/* valid connection with MS */
extern gboolean offline;			/* Offline mode */
extern gboolean interrogated;			/* valid connection with MS */

gboolean dispatcher(gpointer data)
{
	extern struct Firmware_Details * firmware;
	gint len=0;
	gint i=0;
	gint val=-1;
	gint count = 0;
	struct Io_Message *message = NULL;
	extern gint temp_units;
	extern gboolean paused_handlers;
	extern gint mem_view_style[];

	if (!dispatch_queue) /*queue not built yet... */
		return TRUE;
	/* Endless Loop, wiat for message, processs and repeat... */
trypop:
	//printf("dispatch queue length %i\n",g_async_queue_length(dispatch_queue));
	message = g_async_queue_try_pop(dispatch_queue);
	if (!message)
		return TRUE;

	/* NOTE if !connected we ABORT All dispatchers as they all
	 * depend on a "connected" status.
	 */
	if ((!connected) && (!offline)) // Raise error window.... 
		no_ms_connection();

	if (message->funcs != NULL)
	{
		len = message->funcs->len;
		for (i=0;i<len;i++)
		{
			val = g_array_index(message->funcs,UpdateFunction, i);

			switch ((UpdateFunction)val)
			{
				case UPD_POPULATE_DLOGGER:
					if (connected)
						populate_dlog_choices();
					break;
				case UPD_LOAD_RT_SLIDERS:
					if (connected)
					{
						load_sliders();
						reset_temps(GINT_TO_POINTER(temp_units));
					}
					break;
				case UPD_LOAD_REALTIME_MAP:
					if (connected)
						load_realtime_map();
					break;
				case UPD_LOAD_GUI_TABS:
					if (connected)
					{
						load_gui_tabs();
						reset_temps(GINT_TO_POINTER(temp_units));
					}
					break;
				case UPD_READ_VE_CONST:
					if (connected)
						io_cmd(IO_READ_VE_CONST,NULL);
					break;
				case UPD_REALTIME:
					if (connected)
						update_runtime_vars();
					break;
				case UPD_VE_CONST:
					paused_handlers = TRUE;
					if (connected)
						update_ve_const();
					paused_handlers = FALSE;
					break;
				case UPD_STORE_BLACK:
					set_store_buttons_state(BLACK);
					for (i=0;i<firmware->total_pages;i++)
						set_reqfuel_state(BLACK,i);
					break;
				case UPD_LOGVIEWER:
					if (connected)
						update_logview_traces();
					break;
				case UPD_RAW_MEMORY:
					if (connected)
						update_raw_memory_view(mem_view_style[message->offset],message->offset);
					break;
				case UPD_DATALOGGER:
					if (connected)
						run_datalog();
					break;
				case UPD_COMMS_STATUS:
					update_comms_status();
					break;
				case UPD_WRITE_STATUS:
					update_write_status();
					break;

			}
		}
	}
	dealloc_message(message);
	count++;
	/* try to handle up to 4 messages at a time.  If this is 
	 * set too high, we can cause the timeout to hog the gui if it's
	 * too low, things can fall behind. (GL redraw ;( )
	 * */
	if(count < 3)
		goto trypop;
	return TRUE;
}

void dealloc_message(void * ptr)
{
        struct Io_Message *message = (struct Io_Message *)ptr;
        if (message->out_str)
                g_free(message->out_str);
        if (message->funcs)
                g_array_free(message->funcs,TRUE);
        if (message->payload)
                g_free(message->payload);
        g_free(message);

}

