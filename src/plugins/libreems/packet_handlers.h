/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/libreems/packet_handlers.h
  \ingroup LibreEMSPlugin,Headers
  \brief LibreEMS Packet handler functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __PACKET_HANDLERS_H__
#define __PACKET_HANDLERS_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <configfile.h>
#include <libreems_globaldefs.h>
#include <enums.h>
#include <threads.h>

typedef enum
{
	SEQUENCE_NUM = LAST_ARG_TYPE + 1,
	PAYLOAD_ID,
	LOCATION_ID,
	OFFSET,
	LENGTH,
	/*DATABYTE, NOT USED*/
	PAYLOAD_DATA
}LibreEMSArgTypes;

typedef enum
{
	BENCH_TEST_STOP = 0,
	BENCH_TEST_INIT,
	BENCH_TEST_BUMP
}LibreEMSBenchTest;
/* For raw packet generation in interrogator */
#define DECODER_NAME_REQ_PKT_LEN 4
#define FIRMWARE_COMPILER_VER_REQ_PKT_LEN 4
#define FIRMWARE_BUILD_DATE_REQ_PKT_LEN 4
#define FIRMWARE_COMPILER_OS_REQ_PKT_LEN 4
#define FIRMWARE_VERSION_REQ_PKT_LEN 4
#define INTERFACE_VERSION_REQ_PKT_LEN 4
#define CLEAR_COUNTERS_REQ_PKT_LEN 4
#define DATALOG_REQ_PKT_LEN 5
#define LOCATION_ID_LIST_REQ_PKT_LEN 7
#define LOCATION_ID_DETAILS_REQ_PKT_LEN 6
#define SOFT_SYSTEM_RESET_PKT_LEN 4
#define HARD_SYSTEM_RESET_PKT_LEN 4
#define BENCH_TEST_PKT_LEN 28	/* Big packet... */

/* Block bits logic */
#define BLOCK_BITS_ALL		0
#define BLOCK_BITS_OR		BIT0
#define BLOCK_BITS_AND		BIT1

/* Masks for the flag field in the blockDetails struct below */
#define BLOCK_HAS_PARENT	BIT0_16
#define BLOCK_IN_RAM		BIT1_16
#define BLOCK_IN_FLASH		BIT2_16
#define BLOCK_IS_INDEXABLE	BIT3_16
#define BLOCK_IS_READONLY	BIT4_16
#define BLOCK_GETS_VERIFIED	BIT5_16
#define BLOCK_SPARE_FLAG_6	BIT6_16
#define BLOCK_SPARE_FLAG_7	BIT7_16
#define BLOCK_SPARE_FLAG_8	BIT8_16
#define BLOCK_SPARE_FLAG_9	BIT9_16
#define BLOCK_SPARE_FLAG_10	BIT10_16
#define BLOCK_SPARE_FLAG_11	BIT11_16
#define BLOCK_IS_2DUS_TABLE	BIT12_16
#define BLOCK_IS_MAIN_TABLE	BIT13_16
#define BLOCK_IS_LOOKUP_TABLE	BIT14_16
#define BLOCK_IS_CONFIGURATION	BIT15_16

typedef enum
{
	/* Firmware Independant */
	REQUEST_INTERFACE_VERSION=0x00,
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
	REQUEST_UPDATE_BLOCK_IN_RAM = 0x0100,
	RESPONSE_UPDATE_BLOCK_IN_RAM,
	REQUEST_REPLACE_BLOCK_IN_FLASH,
	RESPONSE_REPLACE_BLOCK_IN_FLASH,
	REQUEST_RETRIEVE_BLOCK_FROM_RAM,
	RESPONSE_RETRIEVE_BLOCK_FROM_RAM,
	REQUEST_RETRIEVE_BLOCK_FROM_FLASH,
	RESPONSE_RETRIEVE_BLOCK_FROM_FLASH,
	REQUEST_BURN_BLOCK_FROM_RAM_TO_FLASH,
	RESPONSE_BURN_BLOCK_FROM_RAM_TO_FLASH,

	/* Datalog Request Packets */
	REQUEST_BASIC_DATALOG = 0x0190,
	RESPONSE_BASIC_DATALOG,
	REQUEST_CONFIGURABLE_DATALOG,
	RESPONSE_CONFIGURABLE_DATALOG,
	REQUEST_SET_ASYNC_DATALOG_TYPE,
	RESPONSE_SET_ASYNC_DATALOG_TYPE,
	REQUEST_BYTE_LA_DATALOG,
	RESPONSE_BYTE_LA_DATALOG,

	/* Special Functions */
	REQUEST_FORWARD_PACKET_OVER_CAN = 0x01F4,
	RESPONSE_FORWARD_PACKET_OVER_CAN,
	REQUEST_FORWARD_PACKET_OVER_OTHER_UART,
	RESPONSE_FORWARD_PACKET_OVER_OTHER_UART,

	/* Generic memory grabber for debugging */
	REQUEST_RETRIEVE_ARBRITRARY_MEMORY = 0x258,
	RESPONSE_RETRIEVE_ARBRITRARY_MEMORY,

	/* Unit tests */
	REQUEST_UNIT_TEST_OVER_SERIAL = 0x6666,
	RESPONSE_UNIT_TEST_OVER_SERIAL,

	/* Bench Test Decoder */
	REQUEST_SET_BENCH_TEST_DATA = 0x8888,
	RESPONSE_SET_BENCH_TEST_DATA,

	/* Interrogation related commands */
	REQUEST_LIST_OF_LOCATION_IDS = 0xDA5E,
	RESPONSE_LIST_OF_LOCATION_IDS,
	REQUEST_DECODER_NAME = 0xEEEE,
	RESPONSE_DECODER_NAME,
	REQUEST_FIRMWARE_BUILD_DATE = 0xEEF0,
	RESPONSE_FIRMWARE_BUILD_DATE,
	REQUEST_FIRMWARE_COMPILER_VERSION,
	RESPONSE_FIRMWARE_COMPILER_VERSION,
	REQUEST_FIRMWARE_COMPILER_OS,
	RESPONSE_FIRMWARE_COMPILER_OS,
	REQUEST_LOCATION_ID_DETAILS = 0xF8E0,
	RESPONSE_LOCATION_ID_DETAILS,

	REQUEST_CLEAR_COUNTERS_AND_FLAGS_TO_ZERO = 0xFFF0,
	RESPONSE_CLEAR_COUNTERS_AND_FLAGS_TO_ZERO

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


typedef struct _LibreEMS_Packet LibreEMS_Packet;

/*!
  \brief _LibreEMS_Packet packet detailed container
  */
struct _LibreEMS_Packet
{
	guchar *data;		/*!< Raw packet data */
	guint16 raw_length;	/*!< Raw packet length */
	guint8 header_bits;	/*!< Header Bits */
	gboolean has_sequence;	/*!< Does this packet have a sequence */
	gboolean has_length;	/*!< Does this packet have a length */
	gboolean is_nack;	/*!< Is this a negative ack? */
	guint8 seq_num;		/*!< Sequence Number */
	guint16 payload_id;	/*!< Payload ID */
	guint16 payload_base_offset;	/*!< PAylaod base offset within pkt */
	guint16 payload_length;	/*!< Payload length in bytes */
};

#define	ESCAPE_BYTE			0xBB
#define	START_BYTE			0xAA
#define	STOP_BYTE			0xCC
#define ESCAPED_ESCAPE_BYTE             0x44
#define ESCAPED_START_BYTE              0x55
#define ESCAPED_STOP_BYTE               0x33


/* Prototypes */
gint atomic_sequence();
void build_output_message(Io_Message *, Command *, gpointer);
void cond_bcast (gpointer, gpointer);
void deregister_packet_queue(gint type, GAsyncQueue *queue, gint data);
void dispatch_packet_queues(LibreEMS_Packet *);
guint8 *finalize_packet(guint8 *, gint, gint *);
void libreems_packet_cleanup(LibreEMS_Packet *);
void handle_data(guchar *buf, gint);
guint8 * make_me_a_packet(gint *, ...);
void mtxlog_packet(const void *, size_t, gboolean);
gboolean packet_decode(LibreEMS_Packet *);
LibreEMS_Packet *packet_deep_copy(LibreEMS_Packet *);
void *packet_handler(gpointer data);
void register_packet_queue(gint type, GAsyncQueue *queue, gint data);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
