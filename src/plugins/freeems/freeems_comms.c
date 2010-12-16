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

	/* HACK ALERT, just turn on serial for now */
	printf("serial_repair_thread, enabling freems comms!\n");
	freeems_serial_enable();
	return 0;
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

	printf("Able to write, going to sleep for 10...\n");
	g_usleep(10000000);
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
