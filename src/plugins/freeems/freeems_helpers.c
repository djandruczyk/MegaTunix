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
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	gint seq = 6;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	guint8 pkt[DATALOG_REQ_PKT_LEN]; 
	gint res = 0;
	gint tmit_len = 0;
	gint len = 0;
	guint8 sum = 0;
	gint i = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_if_fail(serial_params);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_SET_ASYNC_DATALOG_TYPE & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_SET_ASYNC_DATALOG_TYPE & 0x00ff );
	pkt[L_PAYLOAD_IDX+1] = 0;
	for (i=0;i<DATALOG_REQ_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[DATALOG_REQ_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,DATALOG_REQ_PKT_LEN,&tmit_len);

	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_SET_ASYNC_DATALOG_TYPE);
	if (!write_wrapper_f(serial_params->fd, buf, tmit_len, &len))
	{
		g_free(buf);
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_SET_ASYNC_DATALOG_TYPE);
		g_async_queue_unref(queue);
		return;
	}
	g_free(buf);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_SET_ASYNC_DATALOG_TYPE);
	g_async_queue_unref(queue);
	if (packet)
		freeems_packet_cleanup(packet);
	return;
}


void warm_boot_ecu(void)
{
	GTimeVal tval;
	gint seq = 6;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	guint8 pkt[SOFT_SYSTEM_RESET_PKT_LEN]; 
	gint res = 0;
	gint tmit_len = 0;
	gint len = 0;
	guint8 sum = 0;
	gint i = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_if_fail(serial_params);
	printf("warm boot ecu?\n");

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_SOFT_SYSTEM_RESET & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_SOFT_SYSTEM_RESET & 0x00ff );
	for (i=0;i<REQUEST_SOFT_SYSTEM_RESET-1;i++)
		sum += pkt[i];
	pkt[REQUEST_SOFT_SYSTEM_RESET-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,REQUEST_SOFT_SYSTEM_RESET,&tmit_len);
	if (!write_wrapper_f(serial_params->fd, buf, tmit_len, &len))
	{
		g_free(buf);
		return;
	}
	g_free(buf);
	return;
}
