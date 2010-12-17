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
#include <defines.h>
#include <freeems_comms.h>
#include <freeems_plugin.h>
#include <gtk/gtk.h>
#include <serialio.h>
#include <template.h>


extern gconstpointer *global_data;


G_MODULE_EXPORT void *serial_repair_thread(gpointer data)
{
	/* We got sent here because of one of the following occurred:
	 * Serial port isn't opened yet (app just fired up)
	 * Serial I/O errors (missing data, or failures reading/writing)
	 *  - This includes things like pulling the RS232 cable out of the ECU
	 * Serial port disappeared (i.e. device hot unplugged)
	 *  - This includes unplugging the USB side of a USB->Serial adapter
	 *    or going out of bluetooth range, for a BT serial device
	 *
	 * Thus we need to handle all possible conditions if possible
	 */
	static gboolean serial_is_open = FALSE; /* Assume never opened */
	static GAsyncQueue *io_repair_queue = NULL;
	gint baud = 0;
	gchar * potential_ports;
	gint len = 0;
	gboolean autodetect = FALSE;
	guchar buf [1024];
	gchar ** vector = NULL;
	guint i = 0;
	Serial_Params *serial_params = NULL;
	void (*unlock_serial_f)(void) = NULL;
	void (*close_serial_f)(void) = NULL;
	gboolean (*open_serial_f)(const gchar *) = NULL;
	gboolean (*lock_serial_f)(const gchar *) = NULL;
	void (*setup_serial_params_f)(void) = NULL;

	serial_params = DATA_GET(global_data,"serial_params");

	get_symbol_f("setup_serial_params",(void *)&setup_serial_params_f);
	get_symbol_f("open_serial",(void *)&open_serial_f);
	get_symbol_f("close_serial",(void *)&close_serial_f);
	get_symbol_f("lock_serial",(void *)&lock_serial_f);
	get_symbol_f("unlock_serial",(void *)&unlock_serial_f);
	dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread created!\n"));

	if (DATA_GET(global_data,"offline"))
	{
		g_timeout_add(100,(GSourceFunc)queue_function_f,"kill_conn_warning");
		dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, offline mode!\n"));
		g_thread_exit(0);
	}
	if (!io_repair_queue)
		io_repair_queue = DATA_GET(global_data,"io_repair_queue");
	/* IF serial_is_open is true, then the port was ALREADY opened 
	 * previously but some error occurred that sent us down here. Thus
	 * first do a simple comms test, if that succeeds, then just cleanup 
	 * and return,  if not, close the port and essentially start over.
	 */
	if (serial_is_open == TRUE)
	{
		dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port considered open, but throwing errors\n"));
		i = 0;
		while (i <= 5)
		{
			dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Calling comms_test, attempt %i\n",i));
			if (comms_test())
			{
				dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, successfull comms test!\n"));
				g_thread_exit(0);
			}
			i++;
		}
		close_serial_f();
		unlock_serial_f();
		serial_is_open = FALSE;
		/* Fall through */
	}
	while (!serial_is_open)
	{
		/* If "leaving" flag set, EXIT now */
		if (DATA_GET(global_data,"leaving"))
			g_thread_exit(0);
		dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port NOT considered open yet.\n"));
		autodetect = (GBOOLEAN) DATA_GET(global_data,"autodetect_port");
		if (!autodetect) /* User thinks he/she is S M A R T */
		{
			potential_ports = (gchar *)DATA_GET(global_data, "override_port");
			if (potential_ports == NULL)
				potential_ports = (gchar *)DATA_GET(global_data,"potential_ports");
		}
		else    /* Auto mode */
			potential_ports = (gchar *)DATA_GET(global_data,"potential_ports");
		vector = g_strsplit(potential_ports,",",-1);
		for (i=0;i<g_strv_length(vector);i++)
		{
			/* Message queue used to exit immediately */
			if (g_async_queue_try_pop(io_repair_queue))
			{
				g_timeout_add(300,(GSourceFunc)queue_function_f,"kill_conn_warning");
				dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, told to!\n"));
				g_thread_exit(0);
			}
			if (!g_file_test(vector[i],G_FILE_TEST_EXISTS))
			{
				dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port %s does NOT exist\n",vector[i]));

				/* Wait 200 ms to avoid deadlocking */
				g_usleep(200000);
				continue;
			}
			g_usleep(100000);
			dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Attempting to open port %s\n",vector[i]));
			thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Attempting to open port %s\n"),vector[i]),FALSE,FALSE);
			if (lock_serial_f(vector[i]))
			{
				if (open_serial_f(vector[i]))
				{
					if (autodetect)
						thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
					dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port %s opened, setting baud to %i for comms test\n",vector[i],baud));
					setup_serial_params_f();
					/* read out any junk in buffer and toss it */
					//read_wrapper_f(serial_params->fd,&buf,1024,&len);

					thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Searching for ECU\n")),FALSE,FALSE);
					dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Performing ECU comms test via port %s.\n",vector[i]));
					if (comms_test())
					{       /* We have a winner !!  Abort loop */
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Search successfull\n")),FALSE,FALSE);
						serial_is_open = TRUE;
						break;
					}
					else
					{
						dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t COMMS test failed, no ECU found, closing port %s.\n",vector[i]));
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("No ECU found...\n")),FALSE,FALSE);
						close_serial_f();
						unlock_serial_f();
						/*g_usleep(100000);*/
					}
				}
				g_usleep(100000);
			}
			else
			{
				dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port %s is open by another application\n",vector[i]));
				thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("Port %s is open by another application\n"),vector[i]),FALSE,FALSE);
			}
		}
		queue_function_f("conn_warning");
	}

	if (serial_is_open)
	{
		freeems_serial_enable();
		queue_function_f("kill_conn_warning");
		thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
	}
	if (vector)
		g_strfreev(vector);
	dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, device found!\n"));
	g_thread_exit(0);
	return NULL;

}
		



G_MODULE_EXPORT void freeems_serial_enable(void)
{
	GIOChannel *channel = NULL;
	GAsyncQueue *queue = NULL;
	Serial_Params *serial_params = NULL;
	gint tmpi = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	printf("serial enable!\n");
	if ((!serial_params->open) || (!serial_params->fd))
	{
		dbg_func(CRITICAL,g_strdup(_(__FILE__": freeems_serial_setup, serial port is NOT open, or filedescriptor is invalid!\n")));
		return;
	}
	queue = g_async_queue_new();
	DATA_SET(global_data,"ack_queue",(gpointer)queue);

#ifdef __WIN32__
	channel = g_io_channel_win32_new_fd(serial_params->fd);
#else
	channel = g_io_channel_unix_new(serial_params->fd);
#endif
	printf("channel open!\n");
	/* Set to raw mode */
	g_io_channel_set_encoding(channel, NULL, NULL);
	/* Reader */
	tmpi = g_io_add_watch(channel,G_IO_IN|G_IO_PRI, able_to_read,(gpointer)queue);
	DATA_SET(global_data,"read_watch",GINT_TO_POINTER(tmpi));

	/* Writer */
	tmpi = g_io_add_watch(channel,G_IO_OUT, able_to_write,(gpointer)queue);
	DATA_SET(global_data,"write_watch",GINT_TO_POINTER(tmpi));

	/* Error Catcher */
	tmpi = g_io_add_watch(channel,G_IO_ERR|G_IO_HUP|G_IO_NVAL, serial_error,NULL);
	DATA_SET(global_data,"error_watch",GINT_TO_POINTER(tmpi));

}


G_MODULE_EXPORT gboolean able_to_read(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	gchar buf[2048];
	gchar *ptr = NULL;
	gsize count = 2048;
	gsize bytes_read = 0;
	GIOStatus status;
	GError *err = NULL;

	printf("Data waiting to be read!\n");
	status = g_io_channel_read_chars(channel, &buf[0], count, &bytes_read, &err);
	if (err)
	{
		printf("error reported: \"%s\"\n",err->message);
		g_error_free(err);
	}
	switch (status)
	{
		case G_IO_STATUS_ERROR:
			printf("IO ERROR!\n");
			break;
		case G_IO_STATUS_NORMAL:
			printf("SUCCESS!\n");
			break;
		case G_IO_STATUS_EOF:
			printf("EOF!\n");
			break;
		case G_IO_STATUS_AGAIN:
			printf("TEMP UNAVAIL!\n");
			break;
	}
	printf("read %i bytes\n",bytes_read);
	return TRUE;
}


G_MODULE_EXPORT gboolean able_to_write(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	GAsyncQueue *ack_queue = NULL;
	GAsyncQueue *out_queue = NULL;
	ack_queue = (GAsyncQueue *)data;
	out_queue = DATA_GET(global_data,"out_packet_queue");

	g_usleep(100000);
	printf("Able to write, sleeping 100 ms\n");
	return TRUE;
}


G_MODULE_EXPORT gboolean serial_error(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	switch (cond)
	{
		case G_IO_IN: 
			printf("data waiting to be read!\n");
			break;
		case G_IO_OUT: 
			printf("channel ready to be written to!\n");
			break;
		case G_IO_PRI: 
			printf("Priority data waiting to be read!\n");
			break;
		case G_IO_ERR: 
			printf("ERROR condition!\n");
			break;
		case G_IO_HUP: 
			printf("Hungup/connection broken!\n");
			break;
		case G_IO_NVAL: 
			printf("Invalid req, descriptor not open!\n");
			break;
	}
	return TRUE;
}


G_MODULE_EXPORT gboolean comms_test(void)
{
	static gint errcount = 0;
	gboolean result = FALSE;
	gchar * err_text = NULL;
	gint len = 0;
	Serial_Params *serial_params = NULL;
	extern gconstpointer *global_data;

	serial_params = DATA_GET(global_data,"serial_params");

	dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\t Entered...\n"));
	if (!serial_params)
		return FALSE;

	dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tRequesting ECU Clock (\"C\" cmd)\n"));

	result = read_data_f(2024,NULL,FALSE);
	if (result > 0)     /* Success */
	{
		DATA_SET(global_data,"connected",GINT_TO_POINTER(TRUE));
		errcount=0;
		dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tECU Comms Test Successful\n"));
		queue_function_f("kill_conn_warning");
		thread_update_widget_f("titlebar",MTX_TITLE,g_strdup(_("ECU Connected...")));
		thread_update_logbar_f("comms_view","info",g_strdup_printf(_("ECU Comms Test Successful\n")),FALSE,FALSE);
		return TRUE;

	}
	else
	{
		/* An I/O Error occurred with the MegaSquirt ECU  */
		DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
		errcount++;
		if (errcount > 5 )
			queue_function_f("conn_warning");
		thread_update_widget_f("titlebar",MTX_TITLE,g_strdup_printf(_("COMMS ISSUES: Check COMMS tab")));
		dbg_func_f(SERIAL_RD|IO_PROCESS,g_strdup(__FILE__": comms_test()\n\tI/O with ECU Timeout\n"));
		thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("I/O with ECU Timeout\n")),FALSE,FALSE);
		return FALSE;
	}
	return FALSE;
}
