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
  \file src/plugins/libreems/packet_handlers.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS packet handling code for parsing, generating and verifying
  ECU packets of data
  \author David Andruczyk
  */

#include <libreems_errors.h>
#include <libreems_plugin.h>
#include <packet_handlers.h>
#include <stdio.h>
#include <string.h>

extern gconstpointer *global_data;


/*!
  \brief This functions handles all incoing data from the ECU and validates
  its content for proper START/STOP/ESCAPING and allocates a LibreEMS_Packet
  structure for VALID packets and populates the required fields as needed
  \param buf is a pointer to the incoming data buffer
  \param len is the numbe of bytes to pull from the incoming buffer
  */
G_MODULE_EXPORT void handle_data(guchar *buf, gint len)
{
	static GAsyncQueue *queue = NULL;
	/* Statistic collection variables */
	static guchar packetBuffer[3000];
	static unsigned int packets = 0;
	static unsigned int charsDropped = 0;
	static unsigned int badChecksums = 0;
	static unsigned int badPackets = 0;
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

	ENTER();
	guchar character;
	gint i = 0;
	LibreEMS_Packet *packet = NULL;
	if (!queue)
		queue = (GAsyncQueue *)DATA_GET(global_data,"packet_queue");
	log_inbound_data_f(buf,len);

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
					MTXDBG(PACKETS,_("Packet number %u ending of length %u at char number %u failed checksum! Received %u Calculated %u\n"), packets, currentPacketLength, processed, lastChar, checksum);
					mtxlog_packet(packetBuffer,currentPacketLength,FALSE);
				}
				else
				{
					goodChecksums++;
					/* Add the length to the SUM */
					sumOfGoodPacketLengths += currentPacketLength;
					/* Clear the state */
					packet = g_new0(LibreEMS_Packet, 1);
					packet->data = (guchar *)g_memdup(packetBuffer,currentPacketLength);
					packet->raw_length = currentPacketLength;
					mtxlog_packet(packet->data,packet->raw_length,FALSE);
					if (!packet_decode(packet))
					{
						printf("Packet fields don't make sense!\n");
						libreems_packet_cleanup(packet);
						badPackets++;
					}
					else if (queue)
					{
						g_async_queue_ref(queue);
						g_async_queue_push(queue,(gpointer)packet);
						g_async_queue_unref(queue);
					}
					else
						printf("packet queue not found!?!!\n");
				}
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
	EXIT();
	return;
}


/*!
  \brief This function is a thread that sits and listens on the packet_queue
  for incoming data that has come from handle_data() after passing basic
  validation.  This thread will call the dispatcher with the packet, which
  will fire off all appropriate subscribers to this packet
  \param data is unused
  \returns NULL
  */
void *packet_handler(gpointer data)
{
	LibreEMS_Packet *packet = NULL;
	GAsyncQueue *queue = (GAsyncQueue *)DATA_GET(global_data,"packet_queue");
	GCond *cond = NULL;

	ENTER();
	while(TRUE)
	{
		if ((DATA_GET(global_data,"leaving") || (DATA_GET(global_data,"packet_handler_thread_exit"))))
		{
			cond = (GCond *)DATA_GET(global_data,"packet_handler_cond");
			if (cond)
				g_cond_signal(cond);
                        g_thread_exit(0);
		}
		if (!queue)
			MTXDBG(PACKETS,_("Packet queue pointer is NULL!!\n\t"));
		else
		{
			packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,250000);
			if (packet)
				dispatch_packet_queues(packet);
		}
	}
	g_thread_exit(0);
	EXIT();
	return NULL;
}


/*!
  \brief packet decoder and verifier of packet syntax.  This function takes
  in a LibreEMS_Packet pointer and validates that the fields within it make
  sense,  i.e. lengths add up, fields are sane (where applicable), if all is
  well it returns TRUE, otherwise FALSE
  \param packet is a pointer to a populated LibreEMS_Packet structure
  \returns TRUE on good packet, FALSE otherwise
  */
gboolean packet_decode(LibreEMS_Packet *packet)
{
	guint8 *ptr = packet->data;
	const gchar * errmsg = NULL;
	gint tmpi = 3; /* header and payload Are ALWAYS present */

	ENTER();
	packet->header_bits = ptr[0];
	/*
	   printf("Raw len %i\n",packet->raw_length);
	   for (i=0;i<packet->raw_length;i++)
	   printf("packet byte %i, valud 0x%0.2X\n",i,ptr[i]);
	 */
	if ((packet->header_bits & HAS_LENGTH_MASK) > 0)
	{
		packet->has_length = TRUE;
		tmpi += 2;
		if (packet->header_bits & HAS_SEQUENCE_MASK)
			packet->payload_length = (ptr[H_LEN_IDX] << 8) + ptr [L_LEN_IDX];
		else
			packet->payload_length = (ptr[H_LEN_IDX-1] << 8) + ptr [L_LEN_IDX-1];
	}
	if ((packet->header_bits & HAS_SEQUENCE_MASK) > 0)
	{
		packet->has_sequence = TRUE;
		tmpi += 1;
		packet->seq_num = ptr[SEQ_IDX];
	}
	packet->payload_id = (ptr[H_PAYLOAD_IDX] << 8) + ptr[L_PAYLOAD_IDX];
	packet->payload_base_offset = tmpi;
	packet->is_nack = ((packet->header_bits & ACK_TYPE_MASK) > 0) ? 1:0;

	if (g_getenv("PKT_DEBUG"))
	{
		printf("Full packet received, %i bytes!\n",packet->raw_length);
		if (packet->is_nack)
			printf("WARNING packet NACK received for payload ID %i\n",packet->payload_id);
		printf("Ack/Nack Flag: %i\n",packet->is_nack);
		printf("Has Sequence Flag: %i\n",((packet->header_bits & HAS_SEQUENCE_MASK) > 0) ? 1:0);
		if ((packet->header_bits & HAS_SEQUENCE_MASK) > 0)
			printf("Sequence id: %i\n",packet->seq_num); 
		printf("Payload id: %i\n",packet->payload_id);
		printf("Has Length Flag: %i\n",((packet->header_bits & HAS_LENGTH_MASK) > 0) ? 1:0);
		if ((packet->header_bits & HAS_LENGTH_MASK) > 0)
			printf("Payload length %i\n",packet->payload_length);
		printf("Payload base offset: %i\n",packet->payload_base_offset);
		printf("RAW PACKET: ->> ");
		for (gint i=0;i<packet->raw_length;i++)
			printf("%.2X ",(guint8)(packet->data)[i]);
		printf("\n");
	}
	if (packet->is_nack)
	{
		guint error = ((guint8)packet->data[tmpi] << 8) + (guint8)packet->data[tmpi+1];
		errmsg = lookup_error(error);
		printf("Packet ERROR Code 0x%.4X, \"%s\"\n",error,errmsg);
	}
	if (packet->header_bits & HAS_LENGTH_MASK)
	{
		if ((packet->payload_length - 3) > packet->raw_length)
		{
			printf("BAD PACKET, PAYLOAD LENGTH issue!\n");
			printf("payload length + header/payload EXCEEDS packet length, BUGGY PACKET!!\n");
			printf("Payload ID: %i, Payload Length %i, raw pkt len %i\n",packet->payload_id,packet->payload_length,packet->raw_length);
			EXIT();
			return FALSE;
		}
	}
	EXIT();
	return TRUE;
}


/*!
 *\brief Registers a queue for a subscriber such that whe na packet comes in 
 matching the criteria a copy of the packet is pushed down that queue to the
 waiting subscriber
 \param type is an enumeration statign whether we are matching on payload ID
 OR sequence number (can't match on a combination of both yet)
 \param queue is a pointer to the queue where this packet should be sent on a 
 match
 \param data is the payloadID or sequence number to match on
 */
G_MODULE_EXPORT void register_packet_queue(gint type, GAsyncQueue *queue, gint data)
{
	static GHashTable *payloads = NULL;
	static GHashTable *sequences = NULL;
	static GMutex *mutex = NULL;
	GList *list = NULL;

	ENTER();
	if (!mutex)
		mutex = (GMutex *)DATA_GET(global_data,"queue_mutex");
	if (!payloads)
		payloads = (GHashTable *)DATA_GET(global_data,"payload_id_queue_hash");
	if (!sequences)
		sequences = (GHashTable *)DATA_GET(global_data,"sequence_num_queue_hash");
	g_return_if_fail(mutex);
	g_return_if_fail(queue);
	g_mutex_lock(mutex);

	switch ((LibreEMSArgTypes)type)
	{
		case PAYLOAD_ID:
			list = (GList *)g_hash_table_lookup(payloads,GINT_TO_POINTER(data));
			g_async_queue_ref(queue);
			list = g_list_append(list,queue);
			/*printf("Paylod ID %i list added entry, list length %i\n",data,g_list_length(list));*/
			g_hash_table_replace(payloads,GINT_TO_POINTER(data),list);
			break;
		case SEQUENCE_NUM:
			list = (GList *)g_hash_table_lookup(sequences,GINT_TO_POINTER(data));
			g_async_queue_ref(queue);
			list = g_list_append(list,queue);
			/*printf("Sequence num %i list added entry, list length %i\n",data,g_list_length(list));*/
			g_hash_table_replace(sequences,GINT_TO_POINTER(data),list);
			break;
		default:
			printf("Need to specific approrpriate criteria to match a packet\n");
			break;
	}
	g_mutex_unlock(mutex);
	EXIT();
	return;
}


/*!
 \brief de-registers a queue for a subscriber based on the passed criteria
 \param type is an enum representing payload ID or Sequence
 \param queue is a pointer to the queue used during registration
 \param data is the payloadID or Sequence Number
 */
G_MODULE_EXPORT void deregister_packet_queue(gint type, GAsyncQueue *queue, gint data)
{
	static GHashTable *payloads = NULL;
	static GHashTable *sequences = NULL;
	static GMutex *mutex = NULL;
	GList *list = NULL;

	ENTER();
	if (!mutex)
		mutex = (GMutex *)DATA_GET(global_data,"queue_mutex");
	if (!payloads)
		payloads = (GHashTable *)DATA_GET(global_data,"payload_id_queue_hash");
	if (!sequences)
		sequences = (GHashTable *)DATA_GET(global_data,"sequence_num_queue_hash");

	g_return_if_fail(mutex);
	g_return_if_fail(queue);
	g_mutex_lock(mutex);
	switch ((LibreEMSArgTypes)type)
	{
		case PAYLOAD_ID:
			list = (GList *)g_hash_table_lookup(payloads,GINT_TO_POINTER(data));
			if (list)
			{
				list = g_list_remove(list,queue);
				/*printf("Payload ID %i list REMOVED entry, list length %i\n",data,g_list_length(list));*/
				g_async_queue_unref(queue);
				if (g_list_length(list) == 0)
				{
					g_list_free(list);
					list = NULL;
				}
				g_hash_table_replace(payloads,GINT_TO_POINTER(data),list);
			}
			/*
			else
				printf("No payload list for id %i to remove from...\n",data);
			*/
			break;
		case SEQUENCE_NUM:
			list = (GList *)g_hash_table_lookup(sequences,GINT_TO_POINTER(data));
			if (list)
			{
				list = g_list_remove(list,queue);
				/*printf("Sequence num %i REMOVED entry, list length %i\n",data,g_list_length(list));*/
				g_async_queue_unref(queue);
				if (g_list_length(list) == 0)
				{
					g_list_free(list);
					list = NULL;
				}
				g_hash_table_replace(sequences,GINT_TO_POINTER(data),list);
			}
			/*
			else
				printf("No sequence number %i list to remove from...\n",data);
			*/
			break;
		default:
			printf("Need to specific approrpriate criteria to match a packet\n");
			break;
	}
	g_mutex_unlock(mutex);
	EXIT();
	return;
}


/*
 \brief This dispatches out packets to awaiting subscribers (if any)
 If the payload or sequnce number matches what's in the packet, this packet
 is copied and pushed down the supplied queue to the lucky winner. This allows
 multiple subscribers per payloadID or sequence number, which offers some
 interesting flexibility
 \param packet is a pointer to the new packet
 \see LibreEMS_Packet
 */
G_MODULE_EXPORT void dispatch_packet_queues(LibreEMS_Packet *packet)
{
	static GHashTable *payloads = NULL;
	static GHashTable *sequences = NULL;
	static GMutex *mutex = NULL;
	GAsyncQueue *queue = NULL;
	guint8 header = packet->data[0];
	guint i = 0;
	GList *list = NULL;

	ENTER();
	if (!mutex)
		mutex = (GMutex *)DATA_GET(global_data,"queue_mutex");
	if (!payloads)
		payloads = (GHashTable *)DATA_GET(global_data,"payload_id_queue_hash");
	if (!sequences)
		sequences = (GHashTable *)DATA_GET(global_data,"sequence_num_queue_hash");

	g_return_if_fail(mutex);
	g_mutex_lock(mutex);
	/* If sequence set, look for it and dispatch if found */
	if ((sequences) && ((packet->header_bits & HAS_SEQUENCE_MASK) > 0))
	{
		list = (GList *)g_hash_table_lookup(sequences,GINT_TO_POINTER((GINT)packet->seq_num));
		if (list)
		{
			for (i=0;i<g_list_length(list);i++)
			{
				queue = (GAsyncQueue *)g_list_nth_data(list,i);
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
		list = (GList *)g_hash_table_lookup(payloads,GINT_TO_POINTER((GINT)packet->payload_id));
		if (list)
		{
			for (i=0;i<g_list_length(list);i++)
			{
				queue = (GAsyncQueue *)g_list_nth_data(list,i);
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
	libreems_packet_cleanup(packet);
	EXIT();
	return;
}



/*!
  \brief copies ALL fields of a packet and returns the deep copy
  \param packet is the packet to copy
  \returns a pointer to a full (deep) copy
  */
LibreEMS_Packet *packet_deep_copy(LibreEMS_Packet *packet)
{
	LibreEMS_Packet *newpkt = NULL;
	ENTER();
	if (!packet)
	{
		EXIT();
		return NULL;
	}
	newpkt = (LibreEMS_Packet *)g_memdup(packet,sizeof(LibreEMS_Packet));
	newpkt->data = (guchar *)g_memdup(packet->data,packet->raw_length);
	EXIT();
	return newpkt;
}


/*!
  \brief Deallocates a LibreEMS_Packet structure
  \param packet is a pointer to the packet being deallocated
  */
void libreems_packet_cleanup(LibreEMS_Packet *packet)
{
	ENTER();
	if (!packet)
	{
		EXIT();
		return ;
	}
	g_free(packet->data);
	g_free(packet);
	EXIT();
	return;
}


/*! 
 \brief build_output_message() is called when doing output to the ECU, to 
 append the needed data together into one nice structured piece for 
 sending to the ECU
 \param message is a pointer to a Io_Message structure
 \param command is a pointer to a Command structure which contains information
 about what fields are needed to be sent to the ecu. This is used to create the  message->sequence correctly
 \param data is an pointer to an OutputData structure that contains the source data to build the message successfully
 */
G_MODULE_EXPORT void build_output_message(Io_Message *message, Command *command, gpointer data)
{
	gboolean have_sequence = FALSE;
	gboolean have_payload_data = FALSE;
	gboolean have_locid = FALSE;
	gboolean have_offset = FALSE;
	guint8 *payload_data = NULL;
	GByteArray *array = NULL;
	gint length = 0;
	gint payload_data_length = 0;
	gint byte = 0;
	gint seq_num = -1;
	gint payload_id = -1;
	gint location_id = -1;
	gint offset = -1;
	guint packet_length = 2; /* Header + cksum, rest come in below */
	gint payload_length = 0;
	guint i = 0;
	gint pos = 0;
	guint8 sum = 0;
	OutputData *output = NULL;
	PotentialArg * arg = NULL;
	guint8 *buf = NULL; /* Raw packet before escapes/start/stop */
	DBlock *block = NULL;

	ENTER();
	if (data)
		output = (OutputData *)data;

	message->sequence = g_array_new(FALSE,TRUE,sizeof(DBlock *));

	/* Arguments */
	for (i=0;i<command->args->len;i++)
	{
		arg = g_array_index(command->args,PotentialArg *, i);
		switch ((GINT)arg->type)
		{
			case ACTION:
				block = g_new0(DBlock, 1);
				block->type = ACTION;
				block->action = arg->action;
				block->arg = arg->action_arg;
				g_array_append_val(message->sequence,block);
				break;
			case SEQUENCE_NUM:
				if (DATA_GET(output->data,arg->internal_name))
				{
					seq_num = (GINT)DATA_GET(output->data,arg->internal_name);
					have_sequence = TRUE;
					packet_length += 1;
				}
				break;
			case PAYLOAD_ID:
				payload_id = (GINT)DATA_GET(output->data,arg->internal_name);
				packet_length += 2;
				break;
				/* Payload specific stuff */
			case LOCATION_ID:
				location_id = (GINT)DATA_GET(output->data,arg->internal_name);
				have_locid = TRUE;
				payload_length += 4;
				break;
			case OFFSET:
				offset = (GINT)DATA_GET(output->data,arg->internal_name);
				have_offset = TRUE;
				payload_length += 2;
				break;
			case LENGTH:
				length = (GINT)DATA_GET(output->data,arg->internal_name);
				packet_length += 2;
				break;
			case DATA:
				have_payload_data = TRUE;
				payload_data = (guint8 *)DATA_GET(output->data,arg->internal_name);
				break;
				/* To bhe migrated to for all  DATA stuff */
			case PAYLOAD_DATA:
				have_payload_data = TRUE;
				array = (GByteArray *)DATA_GET(output->data,arg->internal_name);
				packet_length += 2;
				length = array->len;
				payload_data = (guint8 *)array->data;
				break;
			default:
				printf("LibreEMS doesn't handle this type %s\n",arg->name);
				break;
		}
	}

	if (g_getenv("PKT_DEBUG"))
	{
		printf("build_output_message(): \n");
		printf("Sequence number %i\n",seq_num);
		printf("Payload ID number 0x%0X\n",payload_id);
		printf("Location ID number %i\n",location_id);
		printf("Offset %i\n",offset);
		printf("Payload DATA length %i\n",length);
	}
	if (have_payload_data)
	{
		payload_data_length = length;
		payload_length += payload_data_length;
	}
	pos = 1; /* Header */
	packet_length += payload_length;
	/*printf("total raw packet length (-start/end markers) %i\n",packet_length);*/
	/* Raw Packet */
	buf = g_new0(guint8, packet_length);

	/* Payload ID */
	buf[H_PAYLOAD_IDX] = (guint8)((payload_id & 0xff00) >> 8);
	buf[L_PAYLOAD_IDX] = (guint8)(payload_id & 0x00ff);
	pos += 2; /* 3, header+payload_id*/

	/* Sequence number if present */
	if (have_sequence)
	{
		buf[HEADER_IDX] |= HAS_SEQUENCE_MASK;
		buf[SEQ_IDX] = (guint8)seq_num;
		pos += 1; /* 4, header+payload_id+seq */
	}

	/* Payload Length if present */
	if (payload_length > 0)
	{
		buf[HEADER_IDX] |= HAS_LENGTH_MASK;
		if (have_sequence)
		{
			buf[H_LEN_IDX] = (guint8)((payload_length & 0xff00) >> 8); 
			buf[L_LEN_IDX] = (guint8)(payload_length & 0x00ff); 
		}
		else
		{
			buf[H_LEN_IDX - 1] = (guint8)((payload_length & 0xff00) >> 8);
			buf[L_LEN_IDX - 1] = (guint8)(payload_length & 0x00ff); 
		}
		pos += 2; /* 5 or 6, header+payload_id+seq+payload_len */

		/* Location ID */
		if (have_locid)
		{
			buf[pos++] = (guint8)((location_id & 0xff00) >> 8); 
			buf[pos++] = (guint8)(location_id & 0x00ff); 
		}
		/* Offset */
		if (have_offset)
		{
			buf[pos++] = (guint8)((offset & 0xff00) >> 8); 
			buf[pos++] = (guint8)(offset & 0x00ff); 
		}
		/* Sub Length */
		if (have_locid)
		{
			buf[pos++] = (guint8)((length & 0xff00) >> 8); 
			buf[pos++] = (guint8)(length & 0x00ff); 
		}
		/* pos = 11 or 12 depending on if seq or not */

		if (payload_data_length > 0)
		{
			if ((!payload_data) && (payload_data_length > 0))
				printf("MAJOR ISSUE, payload_data is null, but payload data length is %i\n",payload_data_length);
			/*printf("position before appending blob is %i, len %i\n",pos, payload_data_length); */
			g_memmove(buf+pos,payload_data,payload_data_length);
			pos += payload_data_length;
		}
	}

	/* Checksum it */
	for (i=0;i<packet_length-1;i++)
		sum += buf[i];
	buf[pos] = sum;
	mtxlog_packet(buf,packet_length,TRUE);
	pos++;

	if (g_getenv("PKT_DEBUG"))
	{
		printf("RAW PACKET -> ");
		for (i=0;i<packet_length;i++)
			printf("%.2X ",buf[i]);
		printf("\n\n");
	}
	/* Escape + start/stop it */
	block = g_new0(DBlock, 1);
	block->type = DATA;
	block->data = finalize_packet(buf,packet_length,&block->len);
	g_free(buf);
	g_array_append_val(message->sequence,block);
	/*printf("\n\n\n");*/
	EXIT();
	return;
}


/*!
  \brief This handles escaping and wrapping the packet with the START/STOP
  markers. 
  \param raw is the raw unescaped packet (checksummed already)
  \param raw_length is the length of the raw packet in bytes
  \param final_length is a pointer to where to store the packets final length
  \returns a pointer to the finalized packet, free this when done with it
  */
guint8 *finalize_packet(guint8 *raw, gint raw_length, gint *final_length )
{
	gint i = 0;
	gint num_2_escape = 0;
	gint markers = 2;
	gint len = 0;
	gint pos = 0;
	guint8 *buf = NULL;
	ENTER();
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
	if (final_length)
		*final_length = len;
	EXIT();
	return buf;
}


/*!
  \brief logs a raw incoming or outgoing packet as HEX to the PACKETS dbg
  channel
  \param buf is a pointer to the raw packet
  \param len is the packets length in bytes
  \param toecu is a flag, TRUE means the packets is GOING from MtX to the ECU
  FALSE means the packet is coming FROM the ECU to MtX
  */
G_MODULE_EXPORT void mtxlog_packet(const void *buf, size_t len, gboolean toecu)
{
	guint i = 0;
	guint8 *ptr = (guint8 *)buf;

	ENTER();
	if (toecu)
		MTXDBG(PACKETS,_("Packet TO ECU %i bytes \n\t"),(gint)len);
	else
		MTXDBG(PACKETS,_("Packet FROM ECU %i bytes \n\t"),(gint)len);
	for (i=0;i<len;i++)
	{
		QUIET_MTXDBG(PACKETS,_("%.2X "),ptr[i]);
		if (!((i+1)%16))
			QUIET_MTXDBG(PACKETS,_("\n\t"));
	}
	QUIET_MTXDBG(PACKETS,_("\n"));
	EXIT();
	return;
}


/*!
 * \brief returns a guaranteed sequence number wiht proper locking (hopefully)
 * \returns an 8 bit integer from 1-254
 */
G_MODULE_EXPORT gint atomic_sequence()
{
	static GMutex * mutex;
	static guint8 seq = 0;

	ENTER();
	if (!mutex)
		mutex = (GMutex *)DATA_GET(global_data,"atomic_sequence_mutex");
	g_mutex_lock(mutex);
	if (seq > 254)
		seq = 0;
	seq++;
	g_mutex_unlock(mutex);
	EXIT();
	return seq;
}


/*!
 * \brief returns a packet when given suitable parameters, accepts enum/value pairs
 * terminated wiht a -1, failing to terminate the list or provide pairs is an ERROR
 * \returns a checksummed and escaped raw packet
 */
G_MODULE_EXPORT guint8 * make_me_a_packet(gint *final_length, ...)
{
	gint i = 0;
	gint count = 0;
	gboolean have_payload_id = FALSE;
	gboolean have_location_id = FALSE;
	gboolean have_sequence_num = FALSE;
	gboolean have_offset = FALSE;
	gboolean have_length = FALSE;
	gboolean have_payload_data = FALSE;
	int argtype = 0;
	gint payload_id = 0;
	gint location_id = 0;
	gint sequence_num = 0;
	gint packet_length = 2; /* Header byte + checksum byte */
	gint payload_length = 0; /* Only for packes whos location ID has a payload! */
	gint offset = 0;
	gint length = 0;
	gint pos = 0;
	gint sum = 0;
	guint8 *pkt = NULL;
	guint8 *buf = NULL;
	guint8 *payload_data = NULL;
	ENTER();
	va_list argp;
	va_start(argp,final_length);
	while (i != -1)
	{
		i = va_arg(argp,int);
		count++;
	}
	va_end(argp);
	if ((count%2) != 1)
		printf("Need a valid number of arguments terminated by -1\n");
	va_start(argp,final_length);
	for (i=0;i<count/2;i++)
	{
		argtype = va_arg(argp,int);
		switch (argtype) {
			case SEQUENCE_NUM:
				have_sequence_num = TRUE;
				sequence_num = va_arg(argp,int);
				packet_length += 1;
				break;
			case PAYLOAD_ID:
				have_payload_id = TRUE;
				payload_id = va_arg(argp,int);
				packet_length += 2;
				break;
			case LOCATION_ID:
				have_location_id = TRUE;
				location_id = va_arg(argp,int);
				packet_length += 4;
				payload_length += 2;
				break;
			case OFFSET:
				have_offset = TRUE;
				offset = va_arg(argp,int);
				packet_length += 2;
				payload_length += 2;
				break;
			case LENGTH: /* For reads this is how much we want, for WRITES this is how large
							the DATA field is
							*/
				have_length = TRUE;
				length = va_arg(argp,int);
				packet_length += 2;
				payload_length += 2;
				break;
			case DATA:
				have_payload_data = TRUE;
				payload_data = va_arg(argp,guint8 *);
				break;
		}
	}
	va_end(argp);

	if (g_getenv("PKT_DEBUG2"))
	{
		printf("build_output_message(): \n");
		printf("Sequence number %i (0x%0X)\n",sequence_num,sequence_num);
		printf("Payload ID number 0x%0X\n",payload_id);
		printf("Location ID number %i\n",location_id);
		printf("Offset %i\n",offset);
		printf("Length (request (R) or payload(W)) %i\n",length);
	}
	if (have_payload_data)
	{
		packet_length += length; /* Total packet is bigger */
		payload_length += length; /* The payload part of the packet */
	}
	pos = 1; /* Header */
	/* Raw Packet */
	buf = g_new0(guint8, packet_length);

	/* Payload ID */
	buf[H_PAYLOAD_IDX] = (guint8)((payload_id & 0xff00) >> 8);
	buf[L_PAYLOAD_IDX] = (guint8)(payload_id & 0x00ff);
	pos += 2; /* 3, header+payload_id*/

	/* Sequence number if present */
	if (have_sequence_num)
	{
		buf[HEADER_IDX] |= HAS_SEQUENCE_MASK;
		buf[SEQ_IDX] = (guint8)sequence_num;
		pos += 1; /* 4, header+payload_id+seq */
	}

	/* Payload Length if present */
	if (have_location_id)
	{
		buf[HEADER_IDX] |= HAS_LENGTH_MASK;
		if (have_sequence_num)
		{
			buf[H_LEN_IDX] = (guint8)((payload_length & 0xff00) >> 8); 
			buf[L_LEN_IDX] = (guint8)(payload_length & 0x00ff); 
		}
		else
		{
			buf[H_LEN_IDX - 1] = (guint8)((payload_length & 0xff00) >> 8);
			buf[L_LEN_IDX - 1] = (guint8)(payload_length & 0x00ff); 
		}
		pos += 2; /* 5 or 6, header+payload_id+seq+payload_len */

		/* Location ID */
		buf[pos++] = (guint8)((location_id & 0xff00) >> 8); 
		buf[pos++] = (guint8)(location_id & 0x00ff); 

		/* Offset */
		if (have_offset)
		{
			buf[pos++] = (guint8)((offset & 0xff00) >> 8); 
			buf[pos++] = (guint8)(offset & 0x00ff); 
		}
		/* Sub Length */
		if (have_length)
		{
			buf[pos++] = (guint8)((length & 0xff00) >> 8); 
			buf[pos++] = (guint8)(length & 0x00ff); 
		}
		/* pos = 11 or 12 depending on if seq or not */

		if (have_payload_data)
		{
			if ((!payload_data) && (length > 0))
				printf("MAJOR ISSUE, payload_data is null, but payload length is %i\n",length);
			/*printf("position before appending blob is %i, len %i\n",pos, payload_data_length); */
			g_memmove(buf+pos,payload_data,length);
			pos += length;
		}
	}

	/* Checksum it */
	for (i=0;i<packet_length-1;i++)
		sum += buf[i];
	buf[pos] = sum;
	mtxlog_packet(buf,packet_length,TRUE);
	pos++;

	if (g_getenv("PKT_DEBUG2"))
	{
		printf("RAW PACKET -> ");
		for (i=0;i<packet_length;i++)
			printf("%.2X ",buf[i]);
		printf("\n\n");
	}
	/* Escape + start/stop it */
	pkt = finalize_packet(buf,packet_length,final_length);
	g_free(buf);
	EXIT();
	return pkt;
}

