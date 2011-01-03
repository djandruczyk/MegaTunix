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
#include <interrogate.h>
#include <freeems_plugin.h>
#include <freeems_helpers.h>
#include <packet_handlers.h>
#include <serialio.h>

extern gconstpointer *global_data;


/*!
 \brief interrogate_ecu() interrogates the target ECU to determine what
 firmware it is running.  It does this by reading a list of tests, sending
 those tests in turn, reading the responses and them comparing the group of
 responses against a list of interrogation profiles until it finds a match.
 */
G_MODULE_EXPORT gboolean interrogate_ecu(void)
{
	GAsyncQueue *queue = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	/* ECU has already been detected via comms test
	   Now we need to figure out its variant and adapt to it
	   */
	/* Send stream disable command */
	stop_streaming();

	/* Request firmware version */
	request_firmware_version();

	/* FreeEMS Interrogator NOT WRITTEN YET */
	return TRUE;
}


void request_firmware_version(void)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	Serial_Params *serial_params = NULL;
	/* Raw packet */
	guint8 *buf = NULL;
	guint8 pkt[FIRM_REQ_PKT_LEN];
	gint res = 0;
	gint len = 0;
	gint i = 0;
	guint8 sum = 0;
	gint tmit_len = 0;
	gchar *version = NULL;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_if_fail(serial_params);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_FIRMWARE_VERSION & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_FIRMWARE_VERSION & 0x00ff );
	for (i=0;i<FIRM_REQ_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[FIRM_REQ_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,FIRM_REQ_PKT_LEN,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_FIRMWARE_VERSION);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, &len))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_FIRMWARE_VERSION);
		g_free(buf);
		g_async_queue_unref(queue);
		return;
	}
	g_free(buf);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_FIRMWARE_VERSION);
	g_async_queue_unref(queue);
	if (packet)
		printf("Firmware version PACKET ARRIVED!\n");
	else
		printf("TIMEOUT\n");


	version = g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_len);
	printf("Version \"%s\"\n",version);
	thread_update_widget_f("text_version_entry",MTX_ENTRY,g_strdup(version));
	g_free(version);
	
	freeems_packet_cleanup(packet);
	return;
}
