/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <freeems_helpers.h>
#include <freeems_plugin.h>
#include <packet_handlers.h>
#include <serialio.h>
#include <threads.h>

extern gconstpointer *global_data;


G_MODULE_EXPORT void stop_streaming(void)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	gint seq = 6;
	Serial_Params *serial_params = NULL;
	unsigned char pkt[7] = {0xAA,0x00,0x01,0x94,0x00,0x95,0xCC};
	gint res = 0;
	gint len = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,405);
	if (!write_wrapper_f(serial_params->fd,&pkt, 7, &len))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,405);
		g_async_queue_unref(queue);
		return;
	}
	g_get_current_time(&tval);
	g_time_val_add(&tval,5000000);
	printf("going to wait on queue %p\n",(gpointer)queue);
	packet = g_async_queue_timed_pop(queue,&tval);
	printf("after timed pop in helper\n");
	if (packet)
		printf("PACKET ARRIVED!\n");
	else
		printf("TIMEOUT\n");
	deregister_packet_queue(PAYLOAD_ID,queue,405);
	freeems_packet_cleanup(packet);
	g_async_queue_unref(queue);
	return;
}
