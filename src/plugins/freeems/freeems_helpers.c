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
#include <threads.h>



G_MODULE_EXPORT void stop_streaming(void)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	gint res = 0;
	gint seq = 6;

	queue = g_async_queue_new();
	output = initialize_outputdata_f();
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_ASYNC_DATALOG_TYPE));
	DATA_SET(output->data,"databyte",GINT_TO_POINTER(0));
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	register_packet_queue(PAYLOAD_ID,queue,405);
	io_cmd("datalog_mgmt",output);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	printf("going to wait on queue %p\n",(gpointer)queue);
	packet = g_async_queue_timed_pop(queue,&tval);
	if (packet)
		printf("PACKET ARRIVED!\n");
	else
		printf("TIMEOUT\n");
	deregister_packet_queue(PAYLOAD_ID,queue,405);
	freeems_packet_cleanup(packet);
	g_async_queue_unref(queue);
	return;
}
