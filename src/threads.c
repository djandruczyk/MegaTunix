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
#include <comms_gui.h>
#include <config.h>
#include <conversions.h>
#include <dataio.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <gui_handlers.h>
#include <interrogate.h>
#include <logviewer_gui.h>
#include <notifications.h>
#include <post_process.h>
#include <runtime_controls.h>
#include <runtime_gui.h>
#include <rtv_map_loader.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <structures.h>
#include <threads.h>
#include <tabloader.h>
#include <unistd.h>


GThread *raw_input_thread;			/* thread handle */
GAsyncQueue *io_queue = NULL;
GAsyncQueue *dispatch_queue = NULL;
gboolean raw_reader_running;			/* flag for thread */
extern gboolean connected;			/* valid connection with MS */
extern gboolean interrogated;			/* valid connection with MS */
extern GtkWidget * comms_view;
extern struct Serial_Params *serial_params;
gchar *handler_types[]={"Realtime Vars","VE-Block","Raw Memory Dump","Comms Test"};


void io_cmd(Io_Command cmd, gpointer data)
{
	struct Io_Message *message = NULL;
	extern struct Io_Cmds *cmds;
	extern gboolean tabs_loaded;
	extern struct Firmware_Details * firmware;
	gint tmp = -1;
	gint i = 0;

	/* This function is the bridge from the main GTK thread (gui) to
	 * the Serial I/O Handler (GThread). Communication is achieved through
	 * Asynchronous Queues. (preferred method in GLib and should be more 
	 * portable to more arch's)
	 */
	switch (cmd)
	{
		case IO_REALTIME_READ:
			message = g_new0(struct Io_Message,1);
			message->cmd = cmd;
			message->command = READ_CMD;
			message->out_str = g_strdup(cmds->realtime_cmd);
			message->out_len = cmds->rt_cmd_len;
			message->handler = REALTIME_VARS;
			message->funcs = g_array_new(FALSE,TRUE,sizeof(gint));
			tmp = UPD_REALTIME;
			g_array_append_val(message->funcs,tmp);
			tmp = UPD_LOGVIEWER;
			g_array_append_val(message->funcs,tmp);
			tmp = UPD_DATALOGGER;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			break;

		case IO_INTERROGATE_ECU:
			message = g_new0(struct Io_Message,1);
			message->cmd = cmd;
			message->command = INTERROGATION;
			message->funcs = g_array_new(TRUE,TRUE,sizeof(gint));
			tmp = UPD_COMMS_STATUS;
			g_array_append_val(message->funcs,tmp);
			if (!tabs_loaded)
			{
				tmp = UPD_LOAD_REALTIME_MAP;
				g_array_append_val(message->funcs,tmp);
				tmp = UPD_LOAD_GUI_TABS;
				g_array_append_val(message->funcs,tmp);
				tmp = UPD_POPULATE_DLOGGER;
				g_array_append_val(message->funcs,tmp);
				tmp = UPD_LOAD_RT_SLIDERS;
				g_array_append_val(message->funcs,tmp);
			}
			tmp = UPD_READ_VE_CONST;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			break;
		case IO_COMMS_TEST:
			message = g_new0(struct Io_Message,1);
			message->cmd = cmd;
			message->command = COMMS_TEST;
			message->page = 0;
			message->out_str = g_strdup("Q");
			message->out_len = 1;
			message->handler = C_TEST;
			message->funcs = g_array_new(TRUE,TRUE,sizeof(gint));
			tmp = UPD_COMMS_STATUS;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			break;
		case IO_READ_VE_CONST:
			if (!firmware)
				break;
			for (i=0;i<firmware->total_pages;i++)
			{
				message = g_new0(struct Io_Message,1);
				message->command = READ_CMD;
				message->page = i;
				if (firmware->page_params[i]->is_spark)
				{
					message->out_str = g_strdup(cmds->ignition_cmd);
					message->out_len = cmds->ign_cmd_len;
				}
				else
				{
					message->out_str = g_strdup(cmds->veconst_cmd);
					message->out_len = cmds->ve_cmd_len;
				}
				message->handler = VE_BLOCK;
				message->funcs = g_array_new(TRUE,TRUE,sizeof(gint));
				if (i == (firmware->total_pages-1))
				{
					tmp = UPD_VE_CONST;
					g_array_append_val(message->funcs,tmp);
					tmp = UPD_STORE_BLACK;
					g_array_append_val(message->funcs,tmp);
				}
				g_async_queue_push(io_queue,(gpointer)message);
			}
			break;
		case IO_READ_RAW_MEMORY:
			message = g_new0(struct Io_Message,1);
			message->cmd = cmd;
			message->command = READ_CMD;
			message->page = 0;
			message->offset = (gint)data;
			message->out_str = g_strdup(cmds->raw_mem_cmd);
			message->out_len = cmds->raw_mem_cmd_len;
			message->handler = RAW_MEMORY_DUMP;
			message->funcs = g_array_new(FALSE,TRUE,sizeof(gint));
			tmp = UPD_RAW_MEMORY;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			break;
		case IO_BURN_MS_FLASH:
			message = g_new0(struct Io_Message,1);
			message->cmd = cmd;
			message->command = BURN_CMD;
			message->funcs = g_array_new(FALSE,TRUE,sizeof(gint));
			tmp = UPD_STORE_BLACK;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			break;
		case IO_WRITE_DATA:
			message = g_new0(struct Io_Message,1);
			message->cmd = cmd;
			message->command = WRITE_CMD;
			message->payload = data;
			message->funcs = g_array_new(FALSE,TRUE,sizeof(gint));
			tmp = UPD_WRITE_STATUS;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			break;
	}
}

void *serial_io_handler(gpointer data)
{
	struct Io_Message *message = NULL;	
	/* Create Queue to listen for commands */
	io_queue = g_async_queue_new();
	dispatch_queue = g_async_queue_new();

	/* Endless Loop, wiat for message, processs and repeat... */
	while (1)
	{
		message = g_async_queue_pop(io_queue);
		switch ((CmdType)message->command)
		{
			case INTERROGATION:
				dbg_func(__FILE__": serial_io_handler()\n\tInterrogate_ecu requested\n",SERIAL_RD|SERIAL_WR|THREADS);
				if (!connected)
					comms_test();
				if (connected)
					interrogate_ecu();
				else
					dbg_func(__FILE__": serial_io_handler()\n\tInterrogate_ecu request denied, NOT Connected!!\n",CRITICAL);
				break;
			case COMMS_TEST:
				dbg_func(__FILE__": serial_io_handler()\n\tcomms_test requested \n",SERIAL_RD|SERIAL_WR|THREADS);
				comms_test();
				if (!connected)
					dbg_func(__FILE__": serial_io_handler()\n\tComms Test failed, NOT Connected!!\n",CRITICAL);
				break;
			case READ_CMD:
				dbg_func(g_strdup_printf(__FILE__": serial_io_handler()\n\tread_command requested (%s)\n",handler_types[message->handler]),SERIAL_RD|THREADS);
				if (connected)
					readfrom_ecu(message);
				else
					dbg_func(g_strdup_printf(__FILE__": serial_io_handler()\n\treadfrom_ecu skipped, NOT Connected initiator %i!!\n",message->cmd),CRITICAL);
				break;
			case WRITE_CMD:
				dbg_func(__FILE__": serial_io_handler()\n\twrite_command requested\n",SERIAL_WR|THREADS);
				if (connected)
					writeto_ecu(message);
				else
					dbg_func(__FILE__": serial_io_handler()\n\twriteto_ecu skipped, NOT Connected!!\n",CRITICAL);
				break;
			case BURN_CMD:
				dbg_func(__FILE__": serial_io_handler()\n\tburn_command requested\n",SERIAL_WR|THREADS);
				if (connected)
					burn_ms_flash();
				else
					dbg_func(__FILE__": serial_io_handler()\n\tburn_ms_flash skipped, NOT Connected!!\n",CRITICAL);

				break;

		}
		/* Send rest of message back up to main context for gui
		 * updates via asyncqueue
		 */
		g_async_queue_push(dispatch_queue,(gpointer)message);
	}
}


void write_ve_const(gint page, gint offset, gint value, gboolean ign_parm)
{
	struct OutputData *output = NULL;
	extern GList ***ve_widgets;
	extern gboolean paused_handlers;
	extern struct Firmware_Details *firmware;

	if ((g_list_length(ve_widgets[page][offset]) > 1) &&
	   (offset != firmware->page_params[page]->reqfuel_offset))
	{
		paused_handlers = TRUE;
		g_list_foreach(ve_widgets[page][offset],update_widget,NULL);
		paused_handlers = FALSE;
	}


	dbg_func(g_strdup_printf(__FILE__": write_ve_const()\n\t Sending page %i, offset %i, value %i, ign_parm %i\n",page,offset,value,ign_parm),SERIAL_WR);
	output = g_new0(struct OutputData,1);
	output->page = page;
	output->offset = offset;
	output->value = value;
	output->ign_parm = ign_parm;
	io_cmd(IO_WRITE_DATA,output);
	return;
}
