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
  \file src/plugins/mscommon/mtxsocket.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon remote network tuning code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MTXSOCKET_H__
#define __MTXSOCKET_H__

#include <gtk/gtk.h>
#include <enums.h>
#include <gui_handlers.h>
#include <threads.h>
#include <gio/gio.h>


typedef struct _MtxSocketClient MtxSocketClient;
typedef struct _MtxSocketData MtxSocketData;
typedef struct _MtxSocket MtxSocket;
typedef struct _SlaveMessage SlaveMessage;

/*!
  \brief Different types of socket connections
  */
typedef enum
{
	MTX_SOCKET_ASCII = 0x410,
	MTX_SOCKET_BINARY,
	MTX_SOCKET_CONTROL
}SocketType;

/*!
  \brief Socket States used in the socket state machine
  */
typedef enum
{
	WAITING_FOR_CMD = 0x420,
	GET_JIMSTIM_RPM_HIGH,
	GET_JIMSTIM_RPM_LOW,
	GET_REINIT_OR_REBOOT,
	GET_MS1_EXTRA_REBOOT,
	GET_MS2_REBOOT,
	GET_CAN_ID,
	GET_TABLE_ID,
	GET_HIGH_OFFSET,
	GET_LOW_OFFSET,
	GET_HIGH_COUNT,
	GET_LOW_COUNT,
	GET_DATABLOCK,
	GET_SINGLE_BYTE,
	GET_MTX_PAGE,
	GET_MS1_PAGE,
	GET_MS1_OFFSET,
	GET_MS1_BYTE,
	GET_MS1_COUNT,
	GET_COLOR,
	GET_ACTION,
	GET_STRING,
	SET_COLOR,
	UNDEFINED_SUBSTATE, /* Beginning of Sub-States */
	SEND_FULL_TABLE,
	SEND_PARTIAL_TABLE,
	BURN_MS2_FLASH,
	GET_VAR_DATA,
	RECV_LOOKUPTABLE
}State;


/*!
  \brief Allowed Commands used in the socket state machine
  */
typedef enum
{
	HELP = 0x3F0,
	QUIT,
	GET_SIGNATURE,
	GET_REVISION,
	GET_RT_VARS,
	GET_RTV_LIST,
	GET_ECU_PAGE,
	GET_ECU_VAR_U08,
	GET_ECU_VAR_S08,
	GET_ECU_VAR_U16,
	GET_ECU_VAR_S16,
	GET_ECU_VAR_U32,
	GET_ECU_VAR_S32,
	SET_ECU_VAR_U08,
	SET_ECU_VAR_S08,
	SET_ECU_VAR_U16,
	SET_ECU_VAR_S16,
	SET_ECU_VAR_U32,
	SET_ECU_VAR_S32,
	GO_BURN_FLASH = BURN_FLASH,
	GET_RAW_ECU,
	SET_RAW_ECU
}TcpCommand;

/*!
  \brief Structure to keep all client data together
  */
struct _MtxSocketClient 
{
	gchar *ip;		/*!< IP of client */
	guint16 port;		/*!< Port on client */
	guint8 ** ecu_data;	/*!< Copy of ecu_data */
	GSocket *socket;	/*!< Socket structure */
	GSocket *control_socket;/*!< Control Socket structure */
	gint fd;		/*!< Filedescriptor related to socket */
	gint control_fd;	/*!< Filedescriptor related to ctrl socket */
	SocketType type;	/*!< Sock type (ascii, binary, etc) */
	gpointer container;	/*!< container ? */
};

/*!
  \brief Structure to for passing data around to socket sub functions
  */
struct _MtxSocketData
{
	guchar cmd;		/*!< Command to execute */
	guint8 canID;		/*!< can ID */
	guint8 tableID;		/*!< Table ID (page?) */
	guint16 offset;		/*!< offset */
	guint16 count;		/*!< Count (how many bytes */
};

/*!
  \brief MtxSocket wrapper
  */
struct _MtxSocket
{
	GSocket *socket;	/*!< Socket structure */
	gint fd;		/*!< fd this socket is linked to */
	SocketType type;	/*!< Ascii/Binary/ETC */
};

/*!
  \brief Notification message to slaves 
  */
struct _SlaveMessage
{
	guint8 canID;		/*!< can ID */
	guint8 page;		/*!< Page */
	guint16 offset;		/*!< Offset */
	guint16 length;		/*!< Length */
	DataSize size;		/*!< Size */
	gint value;		/*!< Value */
	void * data;		/*!< Data Block */
	WriteMode mode;		/*!< Write mode, simple or complex */
	SlaveMsgType type;	/*!< Slave message type */
	RemoteAction action;	/*!< Action slave should perform */
};

/* Prototypes */
void *ascii_socket_server(gpointer );
void *binary_socket_server(gpointer );
guint8 *build_netmsg(guint8,SlaveMessage *,gint *);
guint8 *build_status_update(guint8,SlaveMessage *,gint *);
gboolean check_for_changes(MtxSocketClient *);
gboolean close_control_socket(void);
gboolean close_network(void);
void close_tcpip_sockets(void);
void *control_socket_client(gpointer );
gint *convert_socket_data(gchar *, gint);
void dealloc_client_data(MtxSocketClient *);
gint net_send(GSocket *, guint8 *, gint);
void *network_repair_thread(gpointer);
void notify_slave(gpointer, gpointer);
void *notify_slaves_thread(gpointer );
gboolean open_control_socket(gchar *, gint);
gboolean open_network(gchar *, gint);
gboolean open_notification_link(gchar *, gint);
void open_tcpip_sockets(void);
void return_socket_error(GSocket *);
GSocket *setup_socket(gint);
void socket_get_ecu_page(MtxSocketClient *, gchar *);
void socket_get_ecu_var(MtxSocketClient *, gchar *, DataSize);
gint socket_get_more_data(gint, void *, gint, gint);
void socket_get_rt_vars(GSocket *, gchar *);
void socket_get_rtv_list(GSocket *);
void socket_set_ecu_var(MtxSocketClient *, gchar *, DataSize);
void *socket_thread_manager(gpointer);
gboolean validate_remote_ascii_cmd(MtxSocketClient *, gchar *, gint);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
