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

#include <comms_gui.h>
#include <config.h>
#include <conversions.h>
#include <dataio.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <interrogate.h>
#include <logviewer_gui.h>
#include <notifications.h>
#include <post_process.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <string.h>
#include <structures.h>
#include <sys/poll.h>
#include <tabloader.h>
#include <threads.h>
#include <unistd.h>


GThread *raw_input_thread;			/* thread handle */
GAsyncQueue *io_queue = NULL;
gboolean raw_reader_running;			/* flag for thread */
extern gboolean connected;			/* valid connection with MS */
extern GtkWidget * comms_view;
extern struct DynamicMisc misc;
extern struct Serial_Params *serial_params;
static gchar *handler_types[]={"Realtime Vars","VE/Constants(1)","VE/Constants(2)","Ignition Vars","Raw Memory Dump","Comms Test"};


void io_cmd(IoCommands cmd, gpointer data)
{
	struct Io_Message *message = NULL;
	extern gint ecu_caps;
	extern struct IoCmds *cmds;
	extern gboolean tabs_loaded;
	gint tmp = -1;

	/* This function is the bridge from the main GTK thread (gui) to
	 * the Serial I/O Handler (GThread). Communication is achieved through
	 * Asynchronous Queues. (preferred method in GLib and should be more 
	 * portable to more arch's)
	 */
	switch (cmd)
	{
		case IO_REALTIME_READ:
			message = g_new0(struct Io_Message,1);
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
			if (!connected)	// Attempt reconnect with comms test
				io_cmd(IO_COMMS_TEST,NULL);
			message = g_new0(struct Io_Message,1);
			message->command = INTERROGATION;
			message->funcs = g_array_new(TRUE,TRUE,sizeof(gint));
			if (!tabs_loaded)
			{
				tmp = UPD_LOAD_GUI_TABS;
				g_array_append_val(message->funcs,tmp);
			}
			tmp = UPD_READ_VE_CONST;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			break;
		case IO_COMMS_TEST:
			message = g_new0(struct Io_Message,1);
			message->command = COMMS_TEST;
			message->page = 0;
			message->out_str = g_strdup("Q");
			message->out_len = 1;
			message->handler = C_TEST;
			g_async_queue_push(io_queue,(gpointer)message);
			break;
		case IO_READ_VE_CONST:
			message = g_new0(struct Io_Message,1);
			message->command = READ_CMD;
			message->page = 0;
			message->out_str = g_strdup(cmds->veconst_cmd);
			message->out_len = cmds->ve_cmd_len;
			message->handler = VE_AND_CONSTANTS_1;
			message->funcs = g_array_new(TRUE,TRUE,sizeof(gint));
			tmp = UPD_STORE_CONVERSIONS;
			g_array_append_val(message->funcs,tmp);
			tmp = UPD_VE_CONST;
			g_array_append_val(message->funcs,tmp);
			tmp = UPD_STORE_BLACK;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			if (ecu_caps & DUALTABLE)
			{
				message = g_new0(struct Io_Message,1);
				message->command = READ_CMD;
				message->page = 1;
				message->out_str = g_strdup(cmds->veconst_cmd);
				message->out_len = cmds->ve_cmd_len;
				message->handler = VE_AND_CONSTANTS_2;
				message->funcs = g_array_new(FALSE,TRUE,sizeof(gint));
				tmp = UPD_VE_CONST;
				g_array_append_val(message->funcs,tmp);
				g_async_queue_push(io_queue,(gpointer)message);
			}
			if (ecu_caps & (S_N_SPARK|S_N_EDIS))
			{
				message = g_new0(struct Io_Message,1);
				message->command = READ_CMD;
				message->page = 0;
				message->out_str = g_strdup(cmds->ignition_cmd);
				message->out_len = cmds->ign_cmd_len;
				message->handler = IGNITION_VARS;
				message->funcs = g_array_new(FALSE,TRUE,sizeof(gint));
				tmp = UPD_VE_CONST;
				g_array_append_val(message->funcs,tmp);
				g_async_queue_push(io_queue,(gpointer)message);
			}
			break;
		case IO_READ_RAW_MEMORY:
			message = g_new0(struct Io_Message,1);
			message->command = READ_CMD;
			message->page = 0;
			message->out_str = g_strdup(cmds->raw_mem_cmd);
			message->out_len = cmds->raw_mem_cmd_len;
			message->handler = RAW_MEMORY_DUMP;
			message->offset = (gint)data;
			message->funcs = g_array_new(FALSE,TRUE,sizeof(gint));
			tmp = UPD_RAW_MEMORY;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			break;
		case IO_BURN_MS_FLASH:
			message = g_new0(struct Io_Message,1);
			message->command = BURN_CMD;
			message->funcs = g_array_new(FALSE,TRUE,sizeof(gint));
			tmp = UPD_STORE_BLACK;
			g_array_append_val(message->funcs,tmp);
			g_async_queue_push(io_queue,(gpointer)message);
			break;
		case IO_WRITE_DATA:
			message = g_new0(struct Io_Message,1);
			message->command = WRITE_CMD;
			message->payload = data;
			g_async_queue_push(io_queue,(gpointer)message);
			break;
	}
}

void *serial_io_handler(gpointer data)
{
	struct Io_Message *message = NULL;	
	gint len=0;
	gint i=0;
	gint val=-1;
	extern gboolean paused_handlers;
	extern gint mem_view_style[];
	/* Create Queue to listen for commands */
	io_queue = g_async_queue_new();

	/* Endless Loop, wiat for message, processs and repeat... */
	while (1)
	{
		message = g_async_queue_pop(io_queue);
		switch ((CmdType)message->command)
		{
			case INTERROGATION:
				dbg_func(__FILE__": serial_io_handler() Interrogate_ecu requested\n",SERIAL_GEN);
				interrogate_ecu();
				break;
			case COMMS_TEST:
				dbg_func(__FILE__": serial_io_handler() comms_test requested \n",SERIAL_GEN);
				comms_test();
				break;
			case READ_CMD:
				dbg_func(g_strdup_printf(__FILE__": serial_io_handler() read_command requested (%s)\n",handler_types[message->handler]),SERIAL_GEN);
				readfrom_ecu(message);
				break;
			case WRITE_CMD:
				dbg_func(__FILE__": serial_io_handler() write_command requested\n",SERIAL_GEN);
				writeto_ecu(message);
				break;
			case BURN_CMD:
				dbg_func(__FILE__": serial_io_handler() burn_command requested\n",SERIAL_GEN);
				burn_ms_flash();
				break;

		}
		/* If dispatch funcs are defined, run them from the thread
		 * context. NOTE wrapping with gdk_threads_enter/leave, as 
		 * that is NECESSARY to to do when making gtk calls inside
		 * the thread....
		 */
		if (message->funcs != NULL)
		{
			len = message->funcs->len;
			for (i=0;i<len;i++)
			{
				val = g_array_index(message->funcs,
						UpdateFunctions, i);
				switch ((UpdateFunctions)val)
				{
					case UPD_LOAD_GUI_TABS:
						gdk_threads_enter();
						load_gui_tabs();
						gdk_threads_leave();
						break;
					case UPD_READ_VE_CONST:
						io_cmd(IO_READ_VE_CONST,NULL);
						break;
					case UPD_STORE_CONVERSIONS:
						store_conversions();
						break;
					case UPD_REALTIME:
						update_runtime_vars();
						break;
					case UPD_VE_CONST:
						gdk_threads_enter();
						paused_handlers = TRUE;
						update_ve_const();
						paused_handlers = FALSE;
						gdk_threads_leave();
						break;
					case UPD_STORE_BLACK:
						gdk_threads_enter();
						set_store_buttons_state(BLACK);
						gdk_threads_leave();
						break;
					case UPD_LOGVIEWER:
						gdk_threads_enter();
						update_logview_traces();
						gdk_threads_leave();
						break;
					case UPD_RAW_MEMORY:
						gdk_threads_enter();
						update_raw_memory_view(mem_view_style[message->offset],message->offset);
						gdk_threads_leave();
						break;
					case UPD_DATALOGGER:
						run_datalog();
						break;
				}
			}
		}
		dealloc_message(message);
	}
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
void readfrom_ecu(void *ptr)
{
	struct pollfd ufds;
	struct Io_Message *message = (struct Io_Message *)ptr;
	gint result;
	extern gint ecu_caps;

	if(serial_params->open == FALSE)
		return;

	ufds.fd = serial_params->fd;
        ufds.events = POLLIN;

        /* Flush serial port... */
        tcflush(serial_params->fd, TCIOFLUSH);

        if (ecu_caps & DUALTABLE)
                set_ms_page(message->page);
        result = write(serial_params->fd,
			message->out_str,
			message->out_len);
	if (result != message->out_len)	
		dbg_func(__FILE__": readfrom_ecu() write command to ECU failed\n",CRITICAL);

	dbg_func(g_strdup_printf(__FILE__": readfrom_ecu() Sent %s to the ECU\n",message->out_str),SERIAL_WR);
	// If reading raw_memory, need a second arg for the offset... 
	if (message->handler == RAW_MEMORY_DUMP)
		result = write(serial_params->fd,&message->offset,1);

	/* check for data,,,, */
        result = poll (&ufds,1,5*serial_params->poll_timeout);
        if (result == 0)   /* Error */
        {
                dbg_func(__FILE__": readfrom_ecu(), failure reading Data from ECU\n",CRITICAL);
                serial_params->errcount++;
                connected = FALSE;
        }
        else            /* Data arrived */
        {
                connected = TRUE;
                dbg_func(g_strdup_printf(__FILE__": reading %s\n",
				handler_types[message->handler]),SERIAL_RD);
		if (message->handler != -1)
                	handle_ms_data(message->handler,message->offset);

        }	
}

/* Called from a THREAD ONLY so gdk_threads_enter/leave are REQUIRED on any
 * gtk function inside.... 
 */
void comms_test()
{
	struct pollfd ufds;
	gint result = -1;
	gchar * tmpbuf = NULL;


	if (serial_params->open == FALSE)
	{
                connected = FALSE;
		dbg_func(__FILE__": comms_test(), Serial Port is NOT opened can NOT check ecu comms...\n",CRITICAL);
		gdk_threads_enter();
		no_ms_connection();
		gdk_threads_leave();
		return;
	}

	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;
	/* Flush the toilet.... */
	tcflush(serial_params->fd, TCIOFLUSH);	
	while (write(serial_params->fd,"C",1) != 1)
	{
		usleep(10000);
		dbg_func(__FILE__": Error writing \"C\" to the ecu in comms_test()\n",CRITICAL);
	}
	dbg_func(__FILE__": check_ecu_comms() Requesting MS Clock (\"C\" cmd)\n",SERIAL_RD);
	result = poll (&ufds,1,serial_params->poll_timeout*5);
	if (result)
	{
		handle_ms_data(C_TEST,-1);
		connected = TRUE;

		tmpbuf = g_strdup_printf("ECU Comms Test Successfull\n");
		/* COMMS test succeeded */
		dbg_func(__FILE__": comms_test(), ECU Comms Test Successfull\n",SERIAL_RD);
		gdk_threads_enter();
		update_logbar(comms_view,NULL,tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
		gtk_widget_set_sensitive(misc.status[STAT_CONNECTED],
				connected);
		gtk_widget_set_sensitive(misc.ww_status[STAT_CONNECTED],
				connected);
		gdk_threads_leave();

	}
	else
	{
		connected = FALSE;
		tmpbuf = g_strdup_printf("I/O with MegaSquirt Timeout\n");
		/* An I/O Error occurred with the MegaSquirt ECU */
		dbg_func(__FILE__": comms_test(), I/O with ECU Timeout\n",CRITICAL);
		gdk_threads_enter();
		update_logbar(comms_view,"warning",tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
		gtk_widget_set_sensitive(misc.status[STAT_CONNECTED],
				connected);
		gtk_widget_set_sensitive(misc.ww_status[STAT_CONNECTED],
				connected);
		gdk_threads_leave();
	}
	/* Flush the toilet again.... */
	tcflush(serial_params->fd, TCIOFLUSH);
	return;
}

void write_ve_const(gint value, gint offset, gboolean ign_parm)
{
        struct OutputData *output = NULL;
        output = g_new0(struct OutputData,1);
        output->value = value;
        output->offset = offset;
        output->ign_parm = ign_parm;
        io_cmd(IO_WRITE_DATA,output);
	return;
}

void writeto_ecu(void *ptr)
{
	struct Io_Message *message = (struct Io_Message *)ptr;
	struct OutputData *data = message->payload;

	gint value = data->value;
	gint offset = data->offset;
	gboolean ign_parm = data->ign_parm;
	gint highbyte = 0;
	gint lowbyte = 0;
	gboolean twopart = 0;
	gint res = 0;
	gint count = 0;
	char lbuff[3] = {0, 0, 0};
	extern unsigned char *ms_data;
	extern unsigned char *ms_data_last;
	extern gint ecu_caps;
	gchar * write_cmd = NULL;;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	if (!connected)
	{
		no_ms_connection();
		g_static_mutex_unlock(&mutex);
		return;		/* can't write anything if disconnected */
	}
	dbg_func(g_strdup_printf("MS Serial Write, Value %i, Mem Offset %i\n",value,offset),SERIAL_WR);

	if (value > 255)
	{
		dbg_func(g_strdup_printf(__FILE__": Large value being sent: %i, to offset %i\n",value,offset),SERIAL_WR);

		highbyte = (value & 0xff00) >> 8;
		lowbyte = value & 0x00ff;
		twopart = TRUE;
		dbg_func(g_strdup_printf(__FILE__": Highbyte: %i, Lowbyte %i\n",highbyte,lowbyte),SERIAL_WR);
	}
	if (value < 0)
	{
		dbg_func(__FILE__": WARNING!!, value sent is below 0\n",CRITICAL);
		g_static_mutex_unlock(&mutex);
		return;
	}

	/* Handles variants and dualtable... */
	if (offset > MS_PAGE_SIZE)
	{
		offset -= MS_PAGE_SIZE;
		if (ecu_caps & DUALTABLE)
			set_ms_page(1);
		else
			dbg_func(g_strdup_printf(__FILE__": High offset (%i), but no DT flag\n",offset+MS_PAGE_SIZE),CRITICAL);

	}
	/* NOT high offset, but if using DT switch page back to 0 */
	else if (ecu_caps & DUALTABLE)
		set_ms_page(0);

	if (ign_parm)
		write_cmd = g_strdup("J");
	else
		write_cmd = g_strdup("W");

	lbuff[0]=offset;
	if (twopart)
	{
		lbuff[1]=highbyte;
		lbuff[2]=lowbyte;
		count = 3;
		dbg_func(__FILE__": Sending 16 bit value to ECU\n",SERIAL_WR);
	}
	else
	{
		lbuff[1]=value;
		count = 2;
		dbg_func(__FILE__": Sending 8 bit value to ECU\n",SERIAL_WR);
	}


	res = write (serial_params->fd,write_cmd,1);	/* Send write command */
	if (res != 1 )
		dbg_func("Sending write command FAILED!!!\n",CRITICAL);
	res = write (serial_params->fd,lbuff,count);	/* Send offset+data */
	if (res != count )
		dbg_func("Sending offset+data FAILED!!!\n",CRITICAL);
	if (ecu_caps & DUALTABLE)
		set_ms_page(0);
	g_free(write_cmd);

	/* We check to see if the last burn copy of the MS VE/constants matches 
	 * the currently set, if so take away the "burn now" notification.
	 * avoid unnecessary burns to the FLASH 
	 */
	res = memcmp(ms_data_last,ms_data,2*MS_PAGE_SIZE);
	gdk_threads_enter();
	if (res == 0)
		set_store_buttons_state(BLACK);
	else
		set_store_buttons_state(RED);
	gdk_threads_leave();

	g_static_mutex_unlock(&mutex);
	return;
}

void burn_ms_flash()
{
        extern unsigned char *ms_data;
        extern unsigned char *ms_data_last;
        gint res = 0;
        extern gint ecu_caps;
        static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

        g_static_mutex_lock(&mutex);

        tcflush(serial_params->fd, TCIOFLUSH);

        /* doing this may NOT be necessary,  but who knows... */
        if (ecu_caps & DUALTABLE)
                set_ms_page(0);

        res = write (serial_params->fd,"B",1);  /* Send Burn command */
        if (res != 1)
        {
                dbg_func(g_strdup_printf(__FILE__": Burn Failure, write command failed %i\n",res),CRITICAL);
        }

        dbg_func(__FILE__": Burn to Flash\n",SERIAL_WR);

        /* sync temp buffer with current burned settings */
        memcpy(ms_data_last,ms_data,2*MS_PAGE_SIZE);

        tcflush(serial_params->fd, TCIOFLUSH);

        g_static_mutex_unlock(&mutex);
        return;
}

