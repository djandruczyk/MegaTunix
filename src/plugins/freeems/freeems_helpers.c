/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
#include <datamgmt.h>
#include <firmware.h>
#include <gtk/gtk.h>
#include <freeems_comms.h>
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


G_MODULE_EXPORT void start_streaming(void)
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
	/* Turn on normal streaming */
	pkt[L_PAYLOAD_IDX+1] = 1;
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


void soft_boot_ecu(void)
{
	GTimeVal tval;
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

	if (DATA_GET(global_data,"offline"))
		return;
	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_SOFT_SYSTEM_RESET & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_SOFT_SYSTEM_RESET & 0x00ff );
	for (i=0;i<SOFT_SYSTEM_RESET_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[SOFT_SYSTEM_RESET_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,SOFT_SYSTEM_RESET_PKT_LEN,&tmit_len);
	if (!write_wrapper_f(serial_params->fd, buf, tmit_len, &len))
	{
		g_free(buf);
		return;
	}
	g_free(buf);
	return;
}


void hard_boot_ecu(void)
{
	GTimeVal tval;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	guint8 pkt[HARD_SYSTEM_RESET_PKT_LEN]; 
	gint res = 0;
	gint tmit_len = 0;
	gint len = 0;
	guint8 sum = 0;
	gint i = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_if_fail(serial_params);

	if (DATA_GET(global_data,"offline"))
		return;
	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_HARD_SYSTEM_RESET & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_HARD_SYSTEM_RESET & 0x00ff );
	for (i=0;i<HARD_SYSTEM_RESET_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[HARD_SYSTEM_RESET_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,HARD_SYSTEM_RESET_PKT_LEN,&tmit_len);
	if (!write_wrapper_f(serial_params->fd, buf, tmit_len, &len))
	{
		g_free(buf);
		return;
	}
	g_free(buf);
	return;
}


G_MODULE_EXPORT void spawn_read_all_pf(void)
{
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	if (!firmware)
		return;

	gdk_threads_enter();
	set_title_f(g_strdup(_("Queuing read of all ECU data...")));
	gdk_threads_leave();
	io_cmd_f(firmware->get_all_command,NULL);
}


G_MODULE_EXPORT gboolean read_freeems_data(void *data, FuncCall type)
{
	static Firmware_Details *firmware = NULL;
	static GRand *rand = NULL;
	OutputData *output = NULL;
	Command *command = NULL;
	gint seq = 0;
	GAsyncQueue *queue = NULL;
	gint i = 0;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	if (!rand)
		rand = g_rand_new();
	g_return_val_if_fail(firmware,FALSE);

	switch (type)
	{
		case FREEEMS_ALL:
			if (!DATA_GET(global_data,"offline"))
			{
				g_list_foreach(get_list_f("get_data_buttons"),set_widget_sensitive_f,GINT_TO_POINTER(FALSE));
				for (i=0;i<firmware->total_pages;i++)
				{
					if (!firmware->page_params[i]->dl_by_default)
						continue;
					seq = g_rand_int_range(rand,2,255);

					output = initialize_outputdata_f();
					DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
					DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
					DATA_SET(output->data,"location_id",GINT_TO_POINTER(firmware->page_params[i]->phys_ecu_page));
					DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_RETRIEVE_BLOCK_FROM_RAM));
					DATA_SET(output->data,"offset", GINT_TO_POINTER(0));
					DATA_SET(output->data,"num_wanted", GINT_TO_POINTER(firmware->page_params[i]->length));
					DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
					queue = g_async_queue_new();
					register_packet_queue(SEQUENCE_NUM,queue,seq);
					DATA_SET(output->data,"queue",queue);
					io_cmd_f(firmware->read_command,output);
				}
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		default:
			printf("default case, nothing happening here\n");
			break;
	}
	return TRUE;
}


G_MODULE_EXPORT void handle_transaction(void * data, FuncCall type)
{
	static GRand *rand = NULL;
	static Firmware_Details *firmware = NULL;
	Io_Message *message = NULL;
	OutputData *output = NULL;
	OutputData *retry = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	gint seq = 0;
	gint tmpi = 0;
	gint canID = 0;
	gint data_length = 0;
	gint locID = 0;
	gint offset = 0;
	gint size = 0;
	gint page = 0;
	GTimeVal tval;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	message = (Io_Message *)data;
	output = (OutputData *)message->payload;
	g_return_if_fail(firmware);
	g_return_if_fail(message);
	g_return_if_fail(output);

	if (!rand)
		rand = g_rand_new();

	switch (type)
	{
		case GENERIC_READ:
			seq = (GINT)DATA_GET(output->data,"sequence_num");
			canID = (GINT)DATA_GET(output->data,"canID");
			locID = (GINT)DATA_GET(output->data,"location_id");
			offset = (GINT)DATA_GET(output->data,"offset");
			size = (GINT)DATA_GET(output->data,"num_wanted");
			packet = retrieve_packet(output->data,NULL);
			queue = DATA_GET(output->data,"queue");
			deregister_packet_queue(SEQUENCE_NUM,queue,seq);
			g_async_queue_unref(queue);
			DATA_SET(output->data,"queue",NULL);
			if (packet)
			{
				if (packet->is_nack)
					printf("packet ACK FAILURE!\n");
				else
				{

					/*printf("Packet arrived for GENERIC_READ case with sequence %i (%.2X), locID %i\n",seq,seq,locID);
					printf("store new block locid %i, offset %i, data %p raw pkt len %i, payload len %i, num_wanted %i\n",locID,offset,packet->data+packet->payload_base_offset,packet->raw_length,packet->payload_length,size);
					*/
					freeems_store_new_block(canID,locID,offset,packet->data+packet->payload_base_offset,size);
					freeems_backup_current_data(canID,locID);

					freeems_packet_cleanup(packet);
					tmpi = (GINT)DATA_GET(global_data,"ve_goodread_count");
					DATA_SET(global_data,"ve_goodread_count",GINT_TO_POINTER(++tmpi));
				}
			}
			else
			{
				printf("timeout, no packet found in queue for sequence %i (%.2X), locID %i\n",seq,seq,locID);
				printf("Re-issuing command!\n");
				retry = initialize_outputdata_f();
				seq = g_rand_int_range(rand,2,255);
				DATA_SET(retry->data,"canID",DATA_GET(output->data,"canID"));
				DATA_SET(retry->data,"sequence_num",GINT_TO_POINTER(seq));
				DATA_SET(retry->data,"location_id",DATA_GET(output->data,"location_id"));
				DATA_SET(retry->data,"payload_id",DATA_GET(output->data,"payload_id"));
				DATA_SET(retry->data,"offset",DATA_GET(output->data,"offset"));
				DATA_SET(retry->data,"num_wanted",DATA_GET(output->data,"num_wanted"));
				DATA_SET(retry->data,"mode",DATA_GET(output->data,"mode"));
				queue = g_async_queue_new();
				register_packet_queue(SEQUENCE_NUM,queue,seq);
				DATA_SET(retry->data,"queue",queue);
				io_cmd_f(firmware->read_command,retry);
				printf("Re-issued command sent, seq %i!\n",seq);
			}
			break;
		case GENERIC_FLASH_WRITE:
		case GENERIC_RAM_WRITE:
			canID = (GINT)DATA_GET(output->data,"canID");
			locID = (GINT)DATA_GET(output->data,"location_id");
			offset = (GINT)DATA_GET(output->data,"offset");
			data_length = (GINT)DATA_GET(output->data,"data_length");
			packet = retrieve_packet(output->data,"RAM_write_queue");
			if (packet)
			{
				/*printf("Packet arrived for GENERIC_RAM_WRITE case locID %i\n",locID);*/
				if (packet->is_nack)
					printf("DATA Write Response PACKET NACK ERROR!!!!\n");
				else
					update_write_status(data);

				freeems_packet_cleanup(packet);
			}
			else
			{
				printf("timeout, no packet found in generic RAM write queue, locID %i\n",locID);
				printf("DATA_SET REISSUE NOT WRITTEN YET");
			}
			break;
		case GENERIC_BURN:
			canID = (GINT)DATA_GET(output->data,"canID");
			locID = (GINT)DATA_GET(output->data,"location_id");
			offset = (GINT)DATA_GET(output->data,"offset");
			data_length = (GINT)DATA_GET(output->data,"data_length");
			packet = retrieve_packet(output->data,"burn_queue");
			if (packet)
			{
				/*printf("Packet arrived for GENERIC_RAM_WRITE case locID %i\n",locID);*/
				if (packet->is_nack)
					printf("BURN Flash Response PACKET NACK ERROR!!!!\n");
				else
				{
					/*printf("burn success!\n");*/
					post_single_burn_pf(data);
					update_write_status(data);
				}

				freeems_packet_cleanup(packet);
			}
			else
			{
				printf("timeout, no packet found in generic BURN to flash queue, locID %i\n",locID);
				printf("BURN REISSUE NOT WRITTEN YET");
			}
			break;
		default:
			printf("Don't know how to handle this type..\n");
			break;
	}
}


G_MODULE_EXPORT gboolean freeems_burn_all(void *data, FuncCall type)
{
	OutputData *output = NULL;
	Command *command = NULL;
	gint i = 0;
	gint last_page = 0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	/*printf("burn all helper\n");*/
	if (!DATA_GET(global_data,"offline"))
	{
		/* FreeEMS allows all pages to be in ram at will*/
		for (i=0;i<firmware->total_pages;i++)
		{
			if (!firmware->page_params[i]->dl_by_default)
				continue;
			if (firmware->page_params[i]->needs_burn)
			{
				output = initialize_outputdata_f();
				/*printf("Burning page %i (locid %i)\n",i,firmware->page_params[i]->phys_ecu_page);*/
				DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
				DATA_SET(output->data,"page",GINT_TO_POINTER(i));
				DATA_SET(output->data,"location_id",GINT_TO_POINTER(firmware->page_params[i]->phys_ecu_page));
				DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_BURN_BLOCK_FROM_RAM_TO_FLASH));
				DATA_SET(output->data,"offset",GINT_TO_POINTER(0));
				DATA_SET(output->data,"data_length",GINT_TO_POINTER(firmware->page_params[i]->length));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->burn_command,output);
			}
		}
	}
	return TRUE;
}


G_MODULE_EXPORT FreeEMS_Packet * retrieve_packet(gconstpointer *object,const gchar * queue_name)
{
	GTimeVal tval;
	FreeEMS_Packet *packet = NULL;
	GAsyncQueue *queue = NULL;

	g_return_val_if_fail(object,NULL);

	if (queue_name) 
		queue = DATA_GET(global_data,queue_name);
	else /* Use "queue" key via DATA_GET */
		queue = DATA_GET(object,"queue");
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	return packet;
}
