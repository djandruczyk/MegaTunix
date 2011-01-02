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
					packet->raw_len = currentPacketLength;
					packet_decode(packet);
					dispatch_packet_conditions(packet);
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


void packet_decode(FreeEMS_Packet *packet)
{
	guint8 *ptr = packet->data;
	gint i = 0;

	packet->header_bits = ptr[0];
	/*
	   printf("Raw len %i\n",packet->raw_len);
	   for (i=0;i<packet->raw_len;i++)
	   printf("packet byte %i, valud 0x%0.2X\n",i,ptr[i]);
	 */
	printf("Ack/Nack Flag: %i\n",((packet->header_bits & ACK_TYPE_MASK) > 0) ? 1:0);
	printf("Has Sequence Flag: %i\n",((packet->header_bits & HAS_SEQUENCE_MASK) > 0) ? 1:0);
	printf("Has Length Flag: %i\n",((packet->header_bits & HAS_LENGTH_MASK) > 0) ? 1:0);
	if ((packet->header_bits & HAS_LENGTH_MASK) > 0)
	{
		if (packet->header_bits & HAS_SEQUENCE_MASK)
			packet->payload_len = (ptr[H_LEN_IDX] << 8) + ptr [L_LEN_IDX];
		else
			packet->payload_len = (ptr[H_LEN_IDX-1] << 8) + ptr [L_LEN_IDX-1];
		printf("Payload length %i\n",packet->payload_len);
	}
	if ((packet->header_bits & HAS_SEQUENCE_MASK) > 0)
	{
		packet->seq_num = ptr[SEQ_IDX];
		printf("Sequence id: %i\n",packet->seq_num); 
	}
	packet->payload_id = (ptr[H_PAYLOAD_IDX] << 8) + ptr[L_PAYLOAD_IDX];

	printf("Payload id: %i\n",packet->payload_id);
	printf("\n");
}


/*!
 *\brief registers a condition variable to be signalled when a packet arrives meeting the
 requested criteria
 */
G_MODULE_EXPORT gboolean register_packet_condition(gint type, GCond *cond, gint data)
{
	static GHashTable *payloads = NULL;
	static GHashTable *sequences = NULL;
	GList *list = NULL;
	gboolean res = FALSE;

	if (!payloads)
		payloads = DATA_GET(global_data,"payload_id_cond_hash");
	if (!sequences)
		sequences = DATA_GET(global_data,"sequence_num_cond_hash");

	switch ((FreeEMSArgTypes)type)
	{
		case PAYLOAD_ID:
			printf("register packet watch on payload_id\n");
			list = g_hash_table_lookup(payloads,GINT_TO_POINTER(data));
			list = g_list_append(list,cond);
			g_hash_table_replace(payloads,GINT_TO_POINTER(data),list);
			break;
		case SEQUENCE_NUM:
			printf("register packet watch on sequence number %i\n",data);
			list = g_hash_table_lookup(sequences,GINT_TO_POINTER(data));
			list = g_list_append(list,cond);
			g_hash_table_replace(sequences,GINT_TO_POINTER(data),list);
			break;
		default:
			printf("Need to specific approrpriate criteria to match a packet\n");
			res = FALSE;
			break;
	}
	return res;
}


/*
 *\brief Purpose is to dispatch the conditions waiting on a specific packet 
 * criteria.  Uses g_signal_broadcast to wakeup all threads that went to 
 * sleep on a specific condition
 */
G_MODULE_EXPORT void dispatch_packet_conditions(FreeEMS_Packet *packet)
{
	static GHashTable *payloads = NULL;
	static GHashTable *sequences = NULL;
	guint8 header = packet->data[0];
	gint i = 0;
	GList *list = NULL;

	printf("dispatch packet conditions\n");
	if (!payloads)
		payloads = DATA_GET(global_data,"payload_id_cond_hash");
	if (!sequences)
		sequences = DATA_GET(global_data,"sequence_num_cond_hash");

	/* If sequence set, look for it and dispatch if found */
	if ((sequences) && ((packet->header_bits & HAS_SEQUENCE_MASK) > 0))
	{
		printf("Seq, looking for list for seq %i\n",packet->seq_num);
		list = g_hash_table_lookup(sequences,GINT_TO_POINTER((gint)packet->seq_num));
		if (list)
			g_list_foreach(list,cond_bcast,GINT_TO_POINTER((gint)packet->seq_num));
	}
	if (payloads)
	{
		printf("PAyload, looking for list for payload id %i\n",packet->payload_id);
		/* If payload ID matches, dispatch if found */
		list = g_hash_table_lookup(payloads,GINT_TO_POINTER((gint)packet->payload_id));
		if (list)
			g_list_foreach(list,cond_bcast,GINT_TO_POINTER((gint)packet->payload_id));
	}
}


G_MODULE_EXPORT void cond_bcast (gpointer data, gpointer user_data)
{
	printf("broadcasting condition %p, user data %i\n",data,(gint)user_data);
	g_cond_signal((GCond *)data);
}
