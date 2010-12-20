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
	printf("Packet:\n");
	packet->header_bits = ptr[0];
	packet->payload_id = (ptr[1] << 8) + ptr[2];
	printf("Payload type %i\n",packet->header_bits & PAYLOAD_TYPE_MASK);
	printf("Ack valid %i\n",packet->header_bits & ACK_VALID_MASK);
	printf("Ack type %i\n",packet->header_bits & ACK_TYPE_MASK);
	printf("has addresses %i\n",packet->header_bits & HAS_ADDRESSES_MASK);
	printf("has length %i\n",packet->header_bits & HAS_LENGTH_MASK);
	printf("Payload ID %i\n",packet->payload_id);
	printf("\n");
	
}
