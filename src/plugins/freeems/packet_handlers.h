/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef __PACKET_HANDLERS_H__
#define __PACKET_HANDLERS_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <configfile.h>
#include <enums.h>

typedef enum
{
	SEQUENCE_NUM = LAST_ARG_TYPE + 1,
	PAYLOAD_ID,
	LOCATION_ID,
	OFFSET,
	LENGTH,
	DATABYTE
}FreeEMSArgTypes;

typedef enum
{
	/* Firmware Independant */
	REQUEST_INTERFACE_VERSION=0,
	RESPONSE_INTERFACE_VERSION,
	REQUEST_FIRMWARE_VERSION,
	RESPONSE_FIRMWARE_VERSION,
	REQUEST_MAX_PACKET_SIZE,
	RESPONSE_MAX_PACKET_SIZE,
	REQUEST_ECHO_PACKET_RETURN,
	RESPONSE_ECHO_PACKET_RETURN,
	REQUEST_SOFT_SYSTEM_RESET,
	RESPONSE_SOFT_SYSTEM_RESET,  /* System comes up fresh, so this never happens */
	REQUEST_HARD_SYSTEM_RESET,
	RESPONSE_HARD_SYSTEM_RESET,  /* System comes up fresh, so this never happens */
	REQUEST_ASYNC_ERROR_CODE,    /* Reserved, never used */
	RESPONSE_ASYNC_ERROR_CODE,
	REQUEST_ASYNC_DEBUG_INFO,    /* Reserved, never used */
	RESPONSE_ASYNC_DEBUG_INFO,   /* NOTE: unrequested debug info packet */

	/* Firmware Payload Type ID's */

	/* Block Manipulation */
	REQUEST_UPDATE_BLOCK_IN_RAM = 256,
	RESPONSE_UPDATE_BLOCK_IN_RAM,
	REQUEST_REPLACE_BLOCK_IN_FLASH,
	RESPONSE_REPLACE_BLOCK_IN_FLASH,
	REQUEST_RETRIEVE_BLOCK_FROM_RAM,
	RESPONSE_RETRIEVE_BLOCK_FROM_RAM,
	REQUEST_RETRIEVE_BLOCK_FROM_FLASH,
	RESPONSE_RETRIEVE_BLOCK_FROM_FLASH,
	REQUEST_BURN_BLOCK_FROM_RAM_TO_FLASH,
	RESPONSE_BURN_BLOCK_FROM_RAM_TO_FLASH,

	/* Main Table Manipulation */
	REQUEST_ADJUST_MAIN_TABLE_CELL = 300,
	RESPONSE_ADJUST_MAIN_TABLE_CELL,
	REQUEST_ADJUST_MAIN_TABLE_RPM_AXIS,
	RESPONSE_ADJUST_MAIN_TABLE_RPM_AXIS,
	REQUEST_ADJUST_MAIN_TABLE_LOAD_AXIS,
	RESPONSE_ADJUST_MAIN_TABLE_LOAD_AXIS,

	/* 2D Table Manipulation */
	REQUEST_ADJUST_2D_TABLE_AXIS,
	RESPONSE_ADJUST_2D_TABLE_AXIS,
	REQUEST_ADJUST_2D_TABLE_CELL,
	RESPONSE_ADJUST_2D_TABLE_CELL,

	/* Datalog Request Packets */
	REQUEST_BASIC_DATALOG = 400,
	RESPONSE_BASIC_DATALOG,
	REQUEST_CONFIGURABLE_DATALOG,
	RESPONSE_CONFIGURABLE_DATALOG,
	REQUEST_SET_ASYNC_DATALOG_TYPE,
	RESPONSE_SET_ASYNC_DATALOG_TYPE,

	/* Special Functions */
	REQUEST_FORWARD_PACKET_OVER_CAN = 500,
	RESPONSE_FORWARD_PACKET_OVER_CAN,
	REQUEST_FORWARD_PACKET_OVER_OTHER_UART,
	RESPONSE_FORWARD_PACKET_OVER_OTHER_UART,

	/* Generic memory grabber for debugging */
	REQUEST_RETRIEVE_ARBRITRARY_MEMORY = 600,
	RESPONSE_RETRIEVE_ARBRITRARY_MEMORY
}PacketType;
/*
struct
{
	guint8 has_length:1;
	guint8 ack_type:1;
	guint8 has_sequence:1;
	guint8 reserved_5:1;
	guint8 reserved_4:1;
	guint8 reserved_3:1;
	guint8 reserved_2:1;
	guint8 reserved_1:1;
}header_bits;
*/

#define HAS_LENGTH_MASK		1
#define ACK_TYPE_MASK		2
#define HAS_SEQUENCE_MASK	4
/*Where in the header the bytes are */
#define HEADER_IDX 0
#define H_PAYLOAD_IDX 1
#define L_PAYLOAD_IDX 2
#define SEQ_IDX 3
#define H_LEN_IDX 4
#define L_LEN_IDX 5


typedef struct _FreeEMS_Packet FreeEMS_Packet;

struct _FreeEMS_Packet
{
	guchar *data;		/* Raw packet data */
	guint16 raw_len;	/* Raw packet length */
	guint8 header_bits;
	guint16 payload_id;
	guint8 seq_num;
	guint16 payload_len;
};


#define	ESCAPE_BYTE			0xBB
#define	START_BYTE			0xAA
#define	STOP_BYTE			0xCC
#define ESCAPED_ESCAPE_BYTE             0x44
#define ESCAPED_START_BYTE              0x55
#define ESCAPED_STOP_BYTE               0x33


/* Prototypes */
void handle_data(guchar *buf, gint);
void packet_decode(FreeEMS_Packet *);
/* Prototypes */

#endif
