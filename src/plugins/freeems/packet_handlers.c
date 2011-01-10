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
#include <freeems_plugin.h>
#include <packet_handlers.h>
#include <string.h>

extern gconstpointer *global_data;

G_MODULE_EXPORT void handle_data(guchar *buf, gint len)
{
	static GAsyncQueue *queue = NULL;
	/* Statistic collection variables */
	static guchar packetBuffer[3000];
	static unsigned int packets = 0;
	static unsigned int charsDropped = 0;
	static unsigned int badChecksums = 0;
	static unsigned int goodChecksums = 0;
	static unsigned int startsInsidePacket = 0;
	static unsigned int totalFalseStartLost = 0;
	static unsigned int doubleStartByteOccurances = 0;
	static unsigned int strayDataBytesOccurances = 0;
	static unsigned int escapeBytesFound = 0;
	static unsigned int escapedStopBytesFound = 0;
	static unsigned int escapedStartBytesFound = 0;
	static unsigned int escapedEscapeBytesFound = 0;
	static unsigned int escapePairMismatches = 0;
	static unsigned long sumOfGoodPacketLengths = 0;
	/* Loop and state variables */
	static gboolean insidePacket = FALSE;
	static gboolean unescapeNext = FALSE;
	static unsigned int processed = 0;
	static unsigned char checksum = 0;
	static unsigned char lastChar = 0;
	static unsigned int currentPacketLength = 0;

	guchar character;
	gint i = 0;
	FreeEMS_Packet *packet = NULL;
	if (!queue)
		queue = DATA_GET(global_data,"packet_queue");

	for (i=0;i<len;i++)
	{
		character = buf[i];
		if (character == START_BYTE)
		{
			if (insidePacket)
			{
				startsInsidePacket++;
				if (currentPacketLength == 0)
				{
					doubleStartByteOccurances++;
				}
				else    
				{       
					totalFalseStartLost += currentPacketLength;
					strayDataBytesOccurances++;
				}
			}
			insidePacket = TRUE;
			checksum = 0;
			currentPacketLength = 0;
		}
		else if (insidePacket)
		{
			if (unescapeNext)
			{	/* Clear escaped byte next flag */
				unescapeNext = FALSE;
				if (character == ESCAPED_ESCAPE_BYTE)
				{
					checksum += ESCAPE_BYTE;
					lastChar = ESCAPE_BYTE;
					escapedEscapeBytesFound++;
					packetBuffer[currentPacketLength] = ESCAPE_BYTE;
					currentPacketLength++;
				}
				else if (character == ESCAPED_START_BYTE)
				{
					/* Store and checksum start byte */
					checksum += START_BYTE;
					lastChar = START_BYTE;
					escapedStartBytesFound++;
					packetBuffer[currentPacketLength] = START_BYTE;
					currentPacketLength++;
				}
				else if(character == ESCAPED_STOP_BYTE)
				{
					/* Store and checksum stop byte */
					checksum += STOP_BYTE;
					lastChar = STOP_BYTE;
					escapedStopBytesFound++;
					packetBuffer[currentPacketLength] = STOP_BYTE;
					currentPacketLength++;
				}else
				{
					/* Otherwise reset and record as data is bad */
					insidePacket = FALSE;
					checksum = 0;
					currentPacketLength = 0;
					escapePairMismatches++;
				}
			}
			else if (character == ESCAPE_BYTE)
			{
				/* Set flag to indicate that the next byte should be un-escaped. */
				unescapeNext = TRUE;
				escapeBytesFound++;
			}
			else if (character == STOP_BYTE)
			{
				packets++;
				/* Bring the checksum back to where it should be */
				checksum -= lastChar;

				/* Check that the checksum matches */
				if(checksum != lastChar)
				{
					badChecksums++;
					/*printf("Packet number %u ending of length %u at char number %u failed checksum! Received %u Calculated %u\n", packets, currentPacketLength, processed, lastChar, checksum);*/
				}
				else
				{
					goodChecksums++;
					/* Add the length to the SUM */
					sumOfGoodPacketLengths += currentPacketLength;
				}
				/* Clear the state */
				printf("Full packet received, len %i!\n",currentPacketLength);
				if (queue)
				{
					packet = g_new0(FreeEMS_Packet, 1);
					packet->data = g_memdup(packetBuffer,currentPacketLength);
					packet->raw_length = currentPacketLength;
					g_async_queue_ref(queue);
					g_async_queue_push(queue,(gpointer)packet);
					g_async_queue_unref(queue);
				}
				else
					printf("packet queue not found!?!!\n");
				insidePacket = FALSE;
				currentPacketLength= 0;
				checksum = 0;
			}
			else
			{
				/* If it isn't special checksum it! */
				checksum += character;
				lastChar = character;
				packetBuffer[currentPacketLength] = character;
				currentPacketLength++;
			}
		}
		else
			charsDropped++;
	}
}


void *packet_handler(gpointer data)
{
	GTimeVal tval;
	FreeEMS_Packet *packet = NULL;
	GAsyncQueue *queue = DATA_GET(global_data,"packet_queue");
	GCond *cond = NULL;

	while(TRUE)
	{
		if (DATA_GET(global_data,"leaving"))
		{
			cond = DATA_GET(global_data,"packet_handler_cond");
			if (cond)
				g_cond_signal(cond);
                        g_thread_exit(0);
		}
		g_get_current_time(&tval);
		g_time_val_add(&tval,100000);
		packet = g_async_queue_timed_pop(queue,&tval);
		if (packet)
		{
			packet_decode(packet);
			dispatch_packet_queues(packet);
		}
	}
}


void packet_decode(FreeEMS_Packet *packet)
{
	guint8 *ptr = packet->data;
	gint i = 0;
	gint tmpi = 3; /* header and payload Are ALWAYS present */

	packet->header_bits = ptr[0];
	/*
	   printf("Raw len %i\n",packet->raw_length);
	   for (i=0;i<packet->raw_length;i++)
	   printf("packet byte %i, valud 0x%0.2X\n",i,ptr[i]);
	 */
	printf("Ack/Nack Flag: %i\n",((packet->header_bits & ACK_TYPE_MASK) > 0) ? 1:0);
	printf("Has Sequence Flag: %i\n",((packet->header_bits & HAS_SEQUENCE_MASK) > 0) ? 1:0);
	printf("Has Length Flag: %i\n",((packet->header_bits & HAS_LENGTH_MASK) > 0) ? 1:0);
	if ((packet->header_bits & HAS_LENGTH_MASK) > 0)
	{
		tmpi += 2;
		if (packet->header_bits & HAS_SEQUENCE_MASK)
			packet->payload_length = (ptr[H_LEN_IDX] << 8) + ptr [L_LEN_IDX];
		else
			packet->payload_length = (ptr[H_LEN_IDX-1] << 8) + ptr [L_LEN_IDX-1];
		printf("Payload length %i\n",packet->payload_length);
	}
	if ((packet->header_bits & HAS_SEQUENCE_MASK) > 0)
	{
		tmpi += 1;
		packet->seq_num = ptr[SEQ_IDX];
		printf("Sequence id: %i\n",packet->seq_num); 
	}
	packet->payload_id = (ptr[H_PAYLOAD_IDX] << 8) + ptr[L_PAYLOAD_IDX];
	packet->payload_base_offset = tmpi;

	printf("Payload id: %i\n",packet->payload_id);
	printf("Payload base offset: %i\n",packet->payload_base_offset);
	printf("\n");
}


/*!
 *\brief registers a queue for a subscriber to gets the packets it wants
 based on provider criteria
 */
G_MODULE_EXPORT void register_packet_queue(gint type, GAsyncQueue *queue, gint data)
{
	static GHashTable *payloads = NULL;
	static GHashTable *sequences = NULL;
	static GMutex *mutex = NULL;
	GList *list = NULL;

	if (!mutex)
		mutex = DATA_GET(global_data,"queue_mutex");
	if (!payloads)
		payloads = DATA_GET(global_data,"payload_id_queue_hash");
	if (!sequences)
		sequences = DATA_GET(global_data,"sequence_num_queue_hash");
	g_return_if_fail(mutex);
	g_mutex_lock(mutex);

	switch ((FreeEMSArgTypes)type)
	{
		case PAYLOAD_ID:
			list = g_hash_table_lookup(payloads,GINT_TO_POINTER(data));
			g_async_queue_ref(queue);
			list = g_list_append(list,queue);
			g_hash_table_replace(payloads,GINT_TO_POINTER(data),list);
			break;
		case SEQUENCE_NUM:
			list = g_hash_table_lookup(sequences,GINT_TO_POINTER(data));
			g_async_queue_ref(queue);
			list = g_list_append(list,queue);
			g_hash_table_replace(sequences,GINT_TO_POINTER(data),list);
			break;
		default:
			printf("Need to specific approrpriate criteria to match a packet\n");
			break;
	}
	g_mutex_unlock(mutex);
	return;
}


/*!
 *\brief de-registers a queue for a subscriber
 */
G_MODULE_EXPORT void deregister_packet_queue(gint type, GAsyncQueue *queue, gint data)
{
	static GHashTable *payloads = NULL;
	static GHashTable *sequences = NULL;
	static GMutex *mutex = NULL;
	GList *list = NULL;

	if (!mutex)
		mutex = DATA_GET(global_data,"queue_mutex");
	if (!payloads)
		payloads = DATA_GET(global_data,"payload_id_queue_hash");
	if (!sequences)
		sequences = DATA_GET(global_data,"sequence_num_queue_hash");

	if (!queue)
		return;

	g_return_if_fail(mutex);
	g_mutex_lock(mutex);
	switch ((FreeEMSArgTypes)type)
	{
		case PAYLOAD_ID:
			list = g_hash_table_lookup(payloads,GINT_TO_POINTER(data));
			if (list)
			{
				g_async_queue_unref(queue);
				list = g_list_remove(list,queue);
				if (g_list_length(list) == 0)
				{
					g_list_free(list);
					list = NULL;
				}
				g_hash_table_replace(payloads,GINT_TO_POINTER(data),list);
			}
			break;
		case SEQUENCE_NUM:
			list = g_hash_table_lookup(sequences,GINT_TO_POINTER(data));
			if (list)
			{
				g_async_queue_unref(queue);
				list = g_list_remove(list,queue);
				if (g_list_length(list) == 0)
				{
					g_list_free(list);
					list = NULL;
				}
				g_hash_table_replace(sequences,GINT_TO_POINTER(data),list);
			}
			break;
		default:
			printf("Need to specific approrpriate criteria to match a packet\n");
			break;
	}
	g_mutex_unlock(mutex);
	return;
}


/*
 *\brief Purpose is to dispatch the conditions waiting on a specific packet 
 * criteria.  Uses g_signal_broadcast to wakeup all threads that went to 
 * sleep on a specific condition
 */
G_MODULE_EXPORT void dispatch_packet_queues(FreeEMS_Packet *packet)
{
	static GHashTable *payloads = NULL;
	static GHashTable *sequences = NULL;
	static GMutex *mutex = NULL;
	GAsyncQueue *queue = NULL;
	guint8 header = packet->data[0];
	gint i = 0;
	GList *list = NULL;

	if (!mutex)
		mutex = DATA_GET(global_data,"queue_mutex");
	if (!payloads)
		payloads = DATA_GET(global_data,"payload_id_queue_hash");
	if (!sequences)
		sequences = DATA_GET(global_data,"sequence_num_queue_hash");

	g_return_if_fail(mutex);
	g_mutex_lock(mutex);
	/* If sequence set, look for it and dispatch if found */
	if ((sequences) && ((packet->header_bits & HAS_SEQUENCE_MASK) > 0))
	{
		list = g_hash_table_lookup(sequences,GINT_TO_POINTER((gint)packet->seq_num));
		if (list)
		{
			for (i=0;i<g_list_length(list);i++)
			{
				queue = g_list_nth_data(list,i);
				if (queue)
				{
					g_async_queue_ref(queue);
					g_async_queue_push(queue,(gpointer)packet_deep_copy(packet));
					g_async_queue_unref(queue);
				}
			}
		}
	}
	if (payloads)
	{
		/* If payload ID matches, dispatch if found */
		list = g_hash_table_lookup(payloads,GINT_TO_POINTER((gint)packet->payload_id));
		if (list)
		{
			for (i=0;i<g_list_length(list);i++)
			{
				queue = g_list_nth_data(list,i);
				if (queue)
				{
					g_async_queue_ref(queue);
					g_async_queue_push(queue,(gpointer)packet_deep_copy(packet));
					g_async_queue_unref(queue);
				}
			}
		}
	}
	g_mutex_unlock(mutex);
	freeems_packet_cleanup(packet);
}



FreeEMS_Packet *packet_deep_copy(FreeEMS_Packet *packet)
{
	FreeEMS_Packet *new = NULL;
	if (!packet)
		return NULL;
	new = g_memdup(packet,sizeof(FreeEMS_Packet));
	new->data = g_memdup(packet->data,packet->raw_length);
	return new;
}


void freeems_packet_cleanup(FreeEMS_Packet *packet)
{
	if (!packet)
		return ;
	g_free(packet->data);
	g_free(packet);
	return;
}


/*! 
 \brief build_output_message() is called when doing output to the ECU, to 
 append the needed data together into one nice blob for sending
 */
G_MODULE_EXPORT void build_output_message(Io_Message *message, Command *command, gpointer data)
{
	gboolean have_sequence = FALSE;
	gboolean have_payload_id = FALSE;
	gboolean have_location_id = FALSE;
	gboolean have_offset = FALSE;
	gboolean have_length = FALSE;
	gboolean have_datablock = FALSE;
	gboolean have_databyte = FALSE;
	guint8 *payload_data = NULL;
	gint payload_data_length = 0;
	gint byte = 0;
	gint seq_num = -1;
	gint payload_id = -1;
	gint location_id = -1;
	gint offset = -1;
	gint length = -1;
	gint packet_length = 2; /* Header + cksum, rest come in below */
	gint payload_length = 0;
	guint i = 0;
	gint pos = 0;
	guint8 sum = 0;
	OutputData *output = NULL;
	PotentialArg * arg = NULL;
	guint8 *buf = NULL; /* Raw packet before escapes/start/stop */
	DBlock *block = NULL;

	if (data)
		output = (OutputData *)data;

	message->sequence = g_array_new(FALSE,TRUE,sizeof(DBlock *));

	payload_length = 0;
	/* Arguments */
	for (i=0;i<command->args->len;i++)
	{
		arg = g_array_index(command->args,PotentialArg *, i);
		switch (arg->type)
		{
			case ACTION:
				/*printf("build_output_message(): ACTION being created!\n");*/
				block = g_new0(DBlock, 1);
				block->type = ACTION;
				block->action = arg->action;
				block->arg = arg->action_arg;
				g_array_append_val(message->sequence,block);
				break;
			case SEQUENCE_NUM:
				have_sequence = TRUE;
				seq_num = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("Sequence number present %i\n",seq_num);*/
				packet_length += 1;
				break;
			case PAYLOAD_ID:
				have_payload_id = TRUE;
				payload_id = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("Payload ID number present %i\n",payload_id);*/
				packet_length += 2;
				break;
				/* Payload specific stuff */
			case LOCATION_ID:
				have_location_id = TRUE;
				location_id = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("Location ID number present %i\n",location_id);*/
				payload_length += 2;
				break;
			case OFFSET:
				have_offset = TRUE;
				offset = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("Location ID number present %i\n",location_id);*/
				payload_length += 2;
				break;
			case LENGTH:
				have_length = TRUE;
				length = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("Payload length present %i\n",length);*/
				payload_length += 2;
				break;
			case DATABYTE:
				have_databyte = TRUE;
				byte = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("DataByte present %i\n",byte);*/
				packet_length += 1;
				break;
			case DATA:
				have_datablock = TRUE;
				payload_data = (guint8 *)DATA_GET(output->data,arg->internal_name);
				payload_data_length = (GINT)DATA_GET(output->data,"num_bytes");
				payload_length += payload_data_length;
				break;
			default:
				printf("FreeEMS doesn't handle this type %s\n",arg->name);
				break;
		}
	}

	pos = 1;
	packet_length += payload_length;
	/*printf("total raw packet length (-start/end/cksum) %i\n",packet_length);*/
	/* Raw Packet */
	buf = g_new0(guint8, packet_length);

	/* Payload ID */
	buf[H_PAYLOAD_IDX] = (guint8)((payload_id & 0xff00) >> 8);
	buf[L_PAYLOAD_IDX] = (guint8)(payload_id & 0x00ff);
	pos += 2;

	/* Sequence number if present */
	if (have_sequence > 0)
	{
		buf[HEADER_IDX] |= HAS_SEQUENCE_MASK;
		buf[SEQ_IDX] = (guint8)seq_num;
		pos += 1;
	}

	/* Payload Length if present */
	if (payload_length > 0)
	{
		/*printf("payload length is %i\n",payload_length);*/
		buf[HEADER_IDX] |= HAS_LENGTH_MASK;
		if (have_sequence > 0)
		{
			buf[H_LEN_IDX] = (guint8)((payload_length & 0xff00) >> 8); 
			buf[L_LEN_IDX] = (guint8)(payload_length & 0x00ff); 
		}
		else
		{
			buf[H_LEN_IDX - 1] = (guint8)((payload_length & 0xff00) >> 8);
			buf[L_LEN_IDX - 1] = (guint8)(payload_length & 0x00ff); 
		}
		pos += 2;

		/* Location ID */
		buf[pos++] = (guint8)((location_id & 0xff00) >> 8); 
		buf[pos++] = (guint8)(location_id & 0x00ff); 
		/* Offset */
		buf[pos++] = (guint8)((offset & 0xff00) >> 8); 
		buf[pos++] = (guint8)(offset & 0x00ff); 
		/* Sub Length */
		buf[pos++] = (guint8)((length & 0xff00) >> 8); 
		buf[pos++] = (guint8)(length & 0x00ff); 

		g_memmove(buf+pos,payload_data,payload_data_length);
		pos += payload_data_length;
	}
	else if (have_databyte) /* For odd cmds that don't have a full payload */
		buf[pos++] = (guint8)(byte & 0x00ff); 

	/* Checksum it */
	for (i=0;i<packet_length;i++)
		sum += buf[i];
	buf[pos] = sum;
	pos++;

	/* Escape + start/stop it */
	block = g_new0(DBlock, 1);
	block->type = DATA;
	block->data = finalize_packet(buf,packet_length,&block->len);
	g_free(buf);
	g_array_append_val(message->sequence,block);
}


guint8 *finalize_packet(guint8 *raw, gint raw_length, gint *final_length )
{
	gint i = 0;
	gint num_2_escape = 0;
	gint markers = 2;
	gint len = 0;
	gint pos = 0;
	guint8 *buf = NULL;
	/* This should allocate a buffer,
	   Escape any special bytes in the packet
	   Checksum it
	   Add start/end flags to it
	 */
	/*printf("finalize, raw input length is %i\n",raw_length);*/
	for (i=0;i<raw_length;i++)
	{
		/*printf("raw[%i] is %i\n",i,raw[i]);*/
		if ((raw[i] == START_BYTE) || (raw[i] == STOP_BYTE) || (raw[i] == ESCAPE_BYTE))
			num_2_escape++;
	}
	len = raw_length + num_2_escape + markers;
	/*printf("length of final pkt is %i\n",len);*/
	buf = g_new0(guint8,len);
	buf[0] = START_BYTE;
	pos = 1;
	for (i=0;i<raw_length;i++)
	{
		if ((raw[i] == START_BYTE) \
				|| (raw[i] == STOP_BYTE) \
				|| (raw[i] == ESCAPE_BYTE))
		{
			buf[pos] = ESCAPE_BYTE;
			pos++;
			buf[pos] = raw[i] ^ 0xFF;
			pos++;
		}
		else
		{
			buf[pos] = raw[i];
			pos++;
		}
	}
	buf[pos] = STOP_BYTE;
	/*printf("last byte at index %i, Stop is %i, buf %i\n",pos,STOP_BYTE,buf[pos]);
		printf("final length is %i\n",len);
		for (i=0;i<len;i++)
			printf("Packet index %i, value 0x%.2X\n",i,buf[i]);
			*/
	if (len -1 != pos)
		printf("packet finalize problem, length mismatch\n");
	*final_length = len;
	return buf;
}
