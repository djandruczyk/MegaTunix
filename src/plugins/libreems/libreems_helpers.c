/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/libreems/libreems_helpers.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS utility functions, mostly referenced from comm.xml as post
  functions
  \author David Andruczyk
  */

#include <datamgmt.h>
#include <firmware.h>
#include <libreems_benchtest.h>
#include <libreems_comms.h>
#include <libreems_errors.h>
#include <libreems_helpers.h>
#include <libreems_plugin.h>
#include <interrogate.h>
#include <serialio.h>
#include <stdio.h>

extern gconstpointer *global_data;


/*!
  \brief Sends a packet to rest the ECU counters
  */
G_MODULE_EXPORT void reset_counters(void)
{
	OutputData *output = NULL;
	ENTER();
	output = initialize_outputdata_f();
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_CLEAR_COUNTERS_AND_FLAGS_TO_ZERO));
	io_cmd_f("empty_payload_pkt",output);

	EXIT();
	return;
}


/*!
  \brief Sends a packet to stop the datalog streaming from the ECU
  */
G_MODULE_EXPORT void stop_streaming(void)
{
	OutputData *output = NULL;
	GByteArray *payload = NULL;
	guint8 byte = 0; /*Stop Streaming */

	ENTER();
	output = initialize_outputdata_f();
	payload = g_byte_array_new();
	g_byte_array_append(payload,&byte,1);
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_ASYNC_DATALOG_TYPE));
	DATA_SET(output->data,"payload_data_array",payload);
	io_cmd_f("basic_payload_pkt",output);
	EXIT();
	return;
}


/*!
  \brief Sends a packet to start the datalog streaming from the ECU
  */
G_MODULE_EXPORT void start_streaming(void)
{
	OutputData *output = NULL;
	GByteArray *payload = NULL;
	guint8 byte = 1;/* Start streaming */

	ENTER();
	output = initialize_outputdata_f();
	payload = g_byte_array_new();
	g_byte_array_append(payload,&byte,1);
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_ASYNC_DATALOG_TYPE));
	DATA_SET(output->data,"payload_data_array",payload);
	io_cmd_f("basic_payload_pkt",output);
	EXIT();
	return;
}


/*!
  \brief Sends a packet to soft-boot the ECU
  */
G_MODULE_EXPORT void soft_boot_ecu(void)
{
	OutputData *output = NULL;
	ENTER();
	output = initialize_outputdata_f();
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SOFT_SYSTEM_RESET));
	io_cmd_f("empty_payload_pkt",output);
	EXIT();
	return;
}


/*!
  \brief Sends a packet to hard-boot the ECU
  */
G_MODULE_EXPORT void hard_boot_ecu(void)
{
	OutputData *output = NULL;
	ENTER();
	output = initialize_outputdata_f();
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_HARD_SYSTEM_RESET));
	io_cmd_f("empty_payload_pkt",output);
	EXIT();
	return;
}


/*!
  \brief Initiates a call to read all ECU data
  */
G_MODULE_EXPORT void spawn_read_all_pf(void)
{
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!firmware)
	{
		EXIT();
		return;
	}

	set_title_f(g_strdup(_("Queuing read of all ECU data...")));
	io_cmd_f(firmware->get_all_command,NULL);
	EXIT();
	return;
}


/*!
  \brief This handler is called to issue packets to read each ECU page in turn
  then pass the original post functions to run.
  */
G_MODULE_EXPORT gboolean read_libreems_data(void *data, FuncCall type)
{
	static Firmware_Details *firmware = NULL;
	OutputData *output = NULL;
	Command *command = NULL;
	gint seq = 0;
	GAsyncQueue *queue = NULL;
	gint i = 0;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,FALSE);

	switch (type)
	{
		case LIBREEMS_ALL:
			if (!DATA_GET(global_data,"offline"))
			{
				g_list_foreach(get_list_f("get_data_buttons"),set_widget_sensitive_f,GINT_TO_POINTER(FALSE));
				for (i=0;i<firmware->total_pages;i++)
				{
					if (!firmware->page_params[i]->dl_by_default)
						continue;
					seq = atomic_sequence();
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
	EXIT();
	return TRUE;
}


/*
 *\brief handle_transaction_hf is defined in comm.xml to handle the results
 of certain IO operations. This runs in the IOthread context so it CAN NOT
 do any GUI operations, but can queue gui ops via the thread_update_* calls
 \param data is a pointer to an Io_Message structure
 \param type is the FuncCall enumeration
 \see Io_Message
 \see FuncCall
 */
G_MODULE_EXPORT void handle_transaction_hf(void * data, FuncCall type)
{
	static Firmware_Details *firmware = NULL;
	Io_Message *message = NULL;
	OutputData *output = NULL;
	OutputData *retry = NULL;
	GAsyncQueue *queue = NULL;
	LibreEMS_Packet *packet = NULL;
	gint payload_id = 0;
	gint seq = 0;
	gint clock = 0;
	gint id = 0;
	gint tmpi = 0;
	gint canID = 0;
	gint length = 0;
	gint locID = 0;
	gint offset = 0;
	gint size = 0;
	gint page = 0;
	gint errorcode = 0;
	const gchar * errmsg = NULL;
	GTimeVal tval;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	message = (Io_Message *)data;
	output = (OutputData *)message->payload;
	g_return_if_fail(firmware);
	g_return_if_fail(message);
	g_return_if_fail(output);

	/* Get common data */
	seq = (GINT)DATA_GET(output->data,"sequence_num");
	canID = (GINT)DATA_GET(output->data,"canID");
	locID = (GINT)DATA_GET(output->data,"location_id");
	offset = (GINT)DATA_GET(output->data,"offset");
	size = (GINT)DATA_GET(output->data,"num_wanted");
	length = (GINT)DATA_GET(output->data,"length");

	switch (type)
	{
		case GENERIC_READ:
			packet = retrieve_packet(output->data,NULL);
			queue = (GAsyncQueue *)DATA_GET(output->data,"queue");
			if (queue)
			{
				deregister_packet_queue(SEQUENCE_NUM,queue,seq);
				g_async_queue_unref(queue);
				DATA_SET(output->data,"queue",NULL);
			}
			if (packet)
			{
				if (packet->is_nack)
					printf("GENERIC_READ packet ACK FAILURE!\n");
				else
				{

					/*printf("Packet arrived for GENERIC_READ case with sequence %i (%.2X), locID %i\n",seq,seq,locID);
					  printf("store new block locid %i, offset %i, data %p raw pkt len %i, payload len %i, num_wanted %i\n",locID,offset,packet->data+packet->payload_base_offset,packet->raw_length,packet->payload_length,size);
					  */
					libreems_store_new_block(canID,locID,offset,packet->data+packet->payload_base_offset,size);
					libreems_backup_current_data(canID,locID);

					libreems_packet_cleanup(packet);
					tmpi = (GINT)DATA_GET(global_data,"ve_goodread_count");
					DATA_SET(global_data,"ve_goodread_count",GINT_TO_POINTER(++tmpi));
				}
			}
			else
			{
				printf("timeout, no packet found in GENERIC_READ queue for sequence %i (%.2X), locID %i\n",seq,seq,locID);
				retry = initialize_outputdata_f();
				seq = atomic_sequence();
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
		case BENCHTEST_RESPONSE:
			packet = retrieve_packet(output->data,NULL);
			queue = (GAsyncQueue *)DATA_GET(output->data,"queue");
			if (queue)
			{
				deregister_packet_queue(SEQUENCE_NUM,queue,seq);
				g_async_queue_unref(queue);
				DATA_SET(output->data,"queue",NULL);
			}
			if (packet)
			{
				if (packet->is_nack)
				{
					errorcode = ((guint8)packet->data[packet->payload_base_offset] << 8) + (guint8)packet->data[packet->payload_base_offset+1];
					errmsg = lookup_error(errorcode);
					thread_update_logbar_f("libreems_benchtest_view","warning",g_strdup_printf(_("Benchtest Packet ERROR, Code (0X%.4X), \"%s\"\n"),errorcode,errmsg),FALSE,FALSE);
				}
				else
				{
					/* get the current clock/addition value*/
					clock = (GINT)DATA_GET(output->data,"clock");
					/* If bumping, increase the total time */
					if (DATA_GET(output->data,"bump"))
					{
						thread_update_logbar_f("libreems_benchtest_view",NULL,g_strdup_printf(_("Benchtest bumped by the user (added %.2f seconds to the clock), Total time remaining is now %.2f seconds\n"),clock/1000.0, ((GINT)DATA_GET(global_data,"benchtest_total")+clock)/1000.0),FALSE,FALSE);
						DATA_SET(global_data,"benchtest_total",GINT_TO_POINTER(((GINT)DATA_GET(global_data,"benchtest_total")+clock)));
					}
					else if (DATA_GET(output->data, "start")) /* start */
					{
						thread_update_logbar_f("libreems_benchtest_view",NULL,g_strdup_printf(_("Initiating LibreEMS Benchtest: Run time should be about %.1f seconds...\n"),clock/1000.0),FALSE,FALSE);
						id = g_timeout_add(500,benchtest_clock_update_wrapper,GINT_TO_POINTER(clock));
						DATA_SET(global_data,"benchtest_clock_id",GINT_TO_POINTER(id));
					}
					else if (DATA_GET(output->data, "stop")) /* stop */
						thread_update_logbar_f("libreems_benchtest_view",NULL,g_strdup_printf(_("Benchtest stopped by the user...\n")),FALSE,FALSE);

				}
				libreems_packet_cleanup(packet);
			}
			break;
		case GENERIC_FLASH_WRITE:
			packet = retrieve_packet(output->data,"FLASH_write_queue");
			goto handle_write;
			break;
		case GENERIC_RAM_WRITE:
			packet = retrieve_packet(output->data,"RAM_write_queue");
handle_write:
			if (packet)
			{
				/*printf("Packet arrived for GENERIC_RAM_WRITE case locID %i\n",locID);*/
				if (packet->is_nack)
				{
					printf("DATA Write Response PACKET NACK ERROR, rollback not implemented yet!!!!\n");
					message->status = FALSE;
				}
				update_write_status(data);
				libreems_packet_cleanup(packet);
			}
			else
			{
				printf("timeout, no packet found in GENERIC_[RAM|FLASH]_WRITE queue for sequence %i (%.2X), locID %i\n",seq,seq,locID);
				retry = initialize_outputdata_f();
				seq = atomic_sequence();
				DATA_SET(retry->data,"canID",DATA_GET(output->data,"canID"));
				DATA_SET(retry->data,"page",DATA_GET(output->data,"page"));
				DATA_SET(retry->data,"sequence_num",GINT_TO_POINTER(seq));
				DATA_SET(retry->data,"location_id",DATA_GET(output->data,"location_id"));
				DATA_SET(retry->data,"payload_id",DATA_GET(output->data,"payload_id"));
				DATA_SET(retry->data,"offset",DATA_GET(output->data,"offset"));
				DATA_SET(retry->data,"size",DATA_GET(output->data,"size"));
				DATA_SET(retry->data,"value",DATA_GET(output->data,"value"));
				DATA_SET(retry->data,"length",DATA_GET(output->data,"length"));
				DATA_SET(retry->data,"data",DATA_GET(output->data,"data"));
				DATA_SET(retry->data,"mode",DATA_GET(output->data,"mode"));
				queue = g_async_queue_new();
				register_packet_queue(SEQUENCE_NUM,queue,seq);
				DATA_SET(retry->data,"queue",queue);
				if (type == GENERIC_RAM_WRITE)
					io_cmd_f(firmware->write_command,retry);
				if (type == GENERIC_FLASH_WRITE)
					io_cmd_f("generic_FLASH_write",retry);
				printf("Re-issued command sent, seq %i!\n",seq);
			}
			break;
		case GENERIC_BURN:
			packet = retrieve_packet(output->data,"burn_queue");
			if (packet)
			{
				/*printf("Packet arrived for GENERIC_BURN case locID %i\n",locID);*/
				if (packet->is_nack)
				{
					printf("BURN Flash Response PACKET NACK ERROR. Ack! I Don't know what to do now!!!!\n");
					message->status = FALSE;
				}
				else
				{
					/*printf("burn success!\n");*/
					post_single_burn_pf(data);
				}
				update_write_status(data);
				libreems_packet_cleanup(packet);
			}
			else
			{
				printf("timeout, no packet found in GENERIC_BURN queue for sequence %i (%.2X), locID %i\n",seq,seq,locID);
				retry = initialize_outputdata_f();
				seq = atomic_sequence();
				DATA_SET(retry->data,"canID",DATA_GET(output->data,"canID"));
				DATA_SET(retry->data,"page",DATA_GET(output->data,"page"));
				DATA_SET(retry->data,"sequence_num",GINT_TO_POINTER(seq));
				DATA_SET(retry->data,"location_id",DATA_GET(output->data,"location_id"));
				DATA_SET(retry->data,"payload_id",DATA_GET(output->data,"payload_id"));
				DATA_SET(retry->data,"offset",DATA_GET(output->data,"offset"));
				DATA_SET(retry->data,"length",DATA_GET(output->data,"length"));
				DATA_SET(retry->data,"mode",DATA_GET(output->data,"mode"));
				queue = g_async_queue_new();
				register_packet_queue(SEQUENCE_NUM,queue,seq);
				DATA_SET(retry->data,"queue",queue);
				io_cmd_f(firmware->burn_command,retry);
			}
			break;
		case EMPTY_PAYLOAD:
			packet = retrieve_packet(output->data,NULL);
			queue = (GAsyncQueue *)DATA_GET(output->data,"queue");
			if (queue)
			{
				deregister_packet_queue(SEQUENCE_NUM,queue,seq);
				g_async_queue_unref(queue);
				DATA_SET(output->data,"queue",NULL);
			}
			if (packet)
			{
				payload_id = packet->payload_id;
				switch (payload_id)
				{
					case RESPONSE_FIRMWARE_VERSION:
						DATA_SET_FULL(global_data,"fw_version",g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_length),g_free);
						update_ecu_info();
						break;
					case RESPONSE_INTERFACE_VERSION:
						DATA_SET_FULL(global_data,"int_version",g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_length),g_free);
						update_ecu_info();
						break;
					case RESPONSE_DECODER_NAME:
						DATA_SET_FULL(global_data,"decoder_name",g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_length),g_free);
						update_ecu_info();
						break;
					case RESPONSE_FIRMWARE_BUILD_DATE:
						DATA_SET_FULL(global_data,"build_date",g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_length),g_free);
						update_ecu_info();
						break;
					case RESPONSE_FIRMWARE_COMPILER_VERSION:
						DATA_SET_FULL(global_data,"compiler",g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_length),g_free);
						update_ecu_info();
						break;
					case RESPONSE_FIRMWARE_COMPILER_OS:
						DATA_SET_FULL(global_data,"build_os",g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_length),g_free);
						update_ecu_info();
						break;
					default:
						printf("payload ID not matched, %i!\n",payload_id);
						break;
				}
				libreems_packet_cleanup(packet);
			}
			else
				printf("EMPTY PAYLOAD PACKET TIMEOUT, retry not implemented for this one yet!!\n");
			break;
		default:
			printf("MegaTunix does NOT know how to handle this packet response type..\n");
			break;
	}
	EXIT();
	return;
}


/*!
  \brief handler to issue the needed calls to burn every outstanding page
  to ECU flash
  \param data is a pointer to an Io_Message structure
  \param type is theFuncCall enumeration
  \see Io_Message
  \see FuncCall
  */
G_MODULE_EXPORT gboolean libreems_burn_all(void *data, FuncCall type)
{
	OutputData *output = NULL;
	Command *command = NULL;
	gint last_page = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	/*printf("burn all helper\n");*/
	if (!DATA_GET(global_data,"offline"))
	{
		/* LibreEMS allows all pages to be in ram at will*/
		for (gint i=0;i<firmware->total_pages;i++)
		{
			if (firmware->page_params[i]->read_only)
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
				DATA_SET(output->data,"length",GINT_TO_POINTER(firmware->page_params[i]->length));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->burn_command,output);
			}
		}
	}
	EXIT();
	return TRUE;
}


/*!
  \brief attemps to retrieve a packet from the named queue if provided or the
  "queue" variable within the object
  \param object is a gconstpointer to an object
  \param queue_name is the name of the queue to pull the packet from
  \returns a pointer to a LibreEMS_Packet structure or NULL of no packet is found
  */
G_MODULE_EXPORT LibreEMS_Packet * retrieve_packet(gconstpointer *object,const gchar * queue_name)
{
	LibreEMS_Packet *packet = NULL;
	GAsyncQueue *queue = NULL;

	ENTER();
	g_return_val_if_fail(object,NULL);

	if (queue_name) 
		queue = (GAsyncQueue *)DATA_GET(global_data,queue_name);
	else /* Use "queue" key via DATA_GET */
		queue = (GAsyncQueue *)DATA_GET(object,"queue");
	if (!queue)
	{
		EXIT();
		return NULL;
	}
	packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,5000000);
	EXIT();
	return packet;
}

