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
#include <string.h>

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
	gchar *version = NULL;
	gchar *fw_version = NULL;
	guint8 major = 0;
	guint8 minor = 0;
	guint8 micro = 0;
	gint i = 0;
	GList *locations = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	/* ECU has already been detected via comms test
	   Now we need to figure out its variant and adapt to it
	   */
	/* Send stream disable command */
	stop_streaming();

	/* Request firmware version */
	request_firmware_version(&fw_version);
	update_logbar_f("interr_view",NULL,g_strdup_printf(_("Firmware Version request returned %i bytes (%s)\n"),(gint)strlen(fw_version),fw_version),FALSE,FALSE,TRUE);
	thread_update_widget_f("ecu_signature_entry",MTX_ENTRY,g_strdup(fw_version));
	g_free(fw_version);

	/* Request interface version */
	request_interface_version(&version, &major, &minor, &micro);
	update_logbar_f("interr_view",NULL,g_strdup_printf(_("Interface Version request returned %i bytes ( %i.%i.%i %s)\n"),(gint)strlen(version)+3,major,minor,micro,version),FALSE,FALSE,TRUE);

	thread_update_widget_f("text_version_entry",MTX_ENTRY,g_strdup(version));
	thread_update_widget_f("ecu_revision_entry",MTX_ENTRY,g_strdup_printf("%i.%i.%i",major,minor,micro));
	g_free(version);

	/* Request List of location ID's */
	locations = request_location_ids();
	for (i=0;i<g_list_length(locations);i++)
	{
		printf("Locations ID %i\n",(gint)g_list_nth_data(locations,i));
	}
	/* FreeEMS Interrogator NOT WRITTEN YET */
	return TRUE;
}


void request_firmware_version(gchar **version)
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
	/*
	if (packet)
		printf("Firmware version PACKET ARRIVED!\n");
	else
		printf("TIMEOUT\n");
	*/

	if (packet)
	{
		*version = g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_length);

		freeems_packet_cleanup(packet);
	}
	return;
}


void request_interface_version(gchar **version, guint8 *major, guint8 *minor, guint8 *micro)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	/* Raw packet */
	guint8 pkt[INTVER_REQ_PKT_LEN];
	gint res = 0;
	gint len = 0;
	gint i = 0;
	guint8 sum = 0;
	gint tmit_len = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_if_fail(serial_params);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0x00ff );
	for (i=0;i<INTVER_REQ_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[INTVER_REQ_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,INTVER_REQ_PKT_LEN,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, &len))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
		g_free(buf);
		g_async_queue_unref(queue);
		return;
	}
	g_free(buf);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
	g_async_queue_unref(queue);
	/*
	   if (packet)
	   printf("Firmware version PACKET ARRIVED!\n");
	   else
	   printf("TIMEOUT\n");
	 */

	if (packet)
	{
		*version = g_strndup((const gchar *)(packet->data+packet->payload_base_offset+3),packet->payload_length-3);
		*major = (guint8)(packet->data[packet->payload_base_offset]);
		*minor = (guint8)(packet->data[packet->payload_base_offset+1]);
		*micro = (guint8)(packet->data[packet->payload_base_offset+2]);
		freeems_packet_cleanup(packet);
	}
	return;
}


/*
 \brief Queries the ECU for a location ID list
 */
GList *request_location_ids(void)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	GList *list = NULL;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	/* Raw packet */
	guint8 pkt[LOC_ID_LIST_REQ_PKT_LEN];
	gint res = 0;
	gint len = 0;
	gint i = 0;
	gint h = 0;
	gint l = 0;
	gint tmpi = 0;
	guint8 sum = 0;
	gint tmit_len = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,NULL);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_RETRIEVE_LIST_OF_LOCATION_IDS & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_RETRIEVE_LIST_OF_LOCATION_IDS & 0x00ff );
	pkt[L_PAYLOAD_IDX+1] = 0;
	for (i=0;i<LOC_ID_LIST_REQ_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[LOC_ID_LIST_REQ_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,LOC_ID_LIST_REQ_PKT_LEN,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_RETRIEVE_LIST_OF_LOCATION_IDS);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, &len))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_RETRIEVE_LIST_OF_LOCATION_IDS);
		g_free(buf);
		g_async_queue_unref(queue);
		return NULL;
	}
	g_free(buf);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_RETRIEVE_LIST_OF_LOCATION_IDS);
	g_async_queue_unref(queue);
	if (packet)
	{
		for (i=0;i<packet->payload_length;i++)
		{
			tmpi = 0;
			h = packet->data[packet->payload_base_offset+i];
			i++;
			l = packet->data[packet->payload_base_offset+i];
			tmpi = (h << 8) + l;
			list = g_list_append(list,GINT_TO_POINTER(tmpi));
		}
		freeems_packet_cleanup(packet);
	}
	return list;
}
