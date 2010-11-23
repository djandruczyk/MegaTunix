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

#ifndef __MTXSOCKET_H__
#define __MTXSOCKET_H__

#include <gtk/gtk.h>
#include <enums.h>

#if GTK_MINOR_VERSION >= 18
#include <gio/gio.h>


#define MTX_SOCKET_ASCII_PORT 12764 /* (ascii math) (m*t)+x */
#define MTX_SOCKET_BINARY_PORT 12765
#define MTX_SOCKET_CONTROL_PORT 12766


typedef struct _MtxSocketClient MtxSocketClient;
typedef struct _MtxSocketData MtxSocketData;
typedef struct _MtxSocket MtxSocket;
typedef struct _SlaveMessage SlaveMessage;

typedef enum
{
	MTX_SOCKET_ASCII = 0x410,
	MTX_SOCKET_BINARY,
	MTX_SOCKET_CONTROL
}SocketType;

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
	SET_COLOR
}State;

typedef enum
{
	UNDEFINED_SUBSTATE = 0x430,
	SEND_FULL_TABLE,
	SEND_PARTIAL_TABLE,
	BURN_MS2_FLASH,
	GET_VAR_DATA,
	RECV_LOOKUPTABLE
}SubState;

typedef enum
{
	HELP = 0x3F0,
	QUIT,
	GET_SIGNATURE,
	GET_REVISION,
	GET_RT_VARS,
	GET_RTV_LIST,
	GET_ECU_VARS,
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
	BURN_FLASH,
	GET_RAW_ECU,
	SET_RAW_ECU
}TcpCommand;

struct _MtxSocketClient 
{
	gchar *ip;
	guint16 port;
	guint8 ** ecu_data;
	GSocket *socket;
	GSocket *control_socket;
	gint fd;
	gint control_fd;
	SocketType type;
	gpointer container;
};

struct _MtxSocketData
{
	guchar cmd;
	guint8 canID;
	guint8 tableID;
	guint16 offset;
	guint16 count;
};

struct _MtxSocket
{
	GSocket *socket;
	gint fd;
	SocketType type;
};

struct _SlaveMessage
{
	guint8 canID;
	guint8 page;
	guint16 offset;
	guint16 length;
	DataSize size;
	gint value;
	void * data;
	WriteMode mode;
	SlaveMsgType type;
	RemoteAction action;
};

/* Prototypes */
void open_tcpip_sockets(void);
void close_tcpip_sockets(void);
GSocket * setup_socket(gint);
void *socket_thread_manager(gpointer);
void * ascii_socket_server(gpointer );
void * binary_socket_server(gpointer );
void * control_socket_client(gpointer );
void * notify_slaves_thread(gpointer );
gboolean validate_remote_ascii_cmd(MtxSocketClient *, gchar *, gint);
void return_socket_error(GSocket *);
void socket_get_rt_vars(GSocket *, gchar *);
void socket_get_rtv_list(GSocket *);
void socket_get_ecu_var(MtxSocketClient *, gchar *, DataSize);
void socket_get_ecu_vars(MtxSocketClient *, gchar *);
void socket_set_ecu_var(MtxSocketClient *, gchar *, DataSize);
gboolean check_for_changes(MtxSocketClient *);
gint * convert_socket_data(gchar *, gint);
void *network_repair_thread(gpointer);
gboolean open_network(gchar *, gint);
gboolean open_notification_link(gchar *, gint);
gboolean close_network(void);
gboolean close_control_socket(void);
gint socket_get_more_data(gint, void *, gint, gint);
gboolean open_control_socket(gchar *, gint);
void notify_slave(gpointer, gpointer);
guint8 * build_netmsg(guint8,SlaveMessage *,gint *);
guint8 * build_status_update(guint8,SlaveMessage *,gint *);
gint net_send(GSocket *, guint8 *, gint);
/* Prototypes */

#else

/* Legacy GTK+ less than 2.18.x which doesn't have GSocket */

#define MTX_SOCKET_ASCII_PORT 12764 /* (ascii math) (m*t)+x */
#define MTX_SOCKET_BINARY_PORT 12765
#define MTX_SOCKET_CONTROL_PORT 12766


typedef struct _MtxSocketClient MtxSocketClient;
typedef struct _MtxSocketData MtxSocketData;
typedef struct _MtxSocket MtxSocket;
typedef struct _SlaveMessage SlaveMessage;

typedef enum
{
	MTX_SOCKET_ASCII = 0x410,
	MTX_SOCKET_BINARY,
	MTX_SOCKET_CONTROL
}SocketType;

typedef enum
{
	WAITING_FOR_CMD = 0x420,
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
	SET_COLOR
}State;

typedef enum
{
	UNDEFINED_SUBSTATE = 0x430,
	SEND_FULL_TABLE,
	SEND_PARTIAL_TABLE,
	BURN_MS2_FLASH,
	GET_VAR_DATA,
	RECV_LOOKUPTABLE
}SubState;

typedef enum
{
	HELP = 0x3F0,
	QUIT,
	GET_SIGNATURE,
	GET_REVISION,
	GET_RT_VARS,
	GET_RTV_LIST,
	GET_ECU_VARS,
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
	BURN_FLASH,
	GET_RAW_ECU,
	SET_RAW_ECU
}TcpCommand;

struct _MtxSocketClient 
{
	gchar *ip;
	guint16 port;
	guint8 ** ecu_data;
	gint fd;
	gint control_fd;
	SocketType type;
	gpointer container;
};

struct _MtxSocketData
{
	guchar cmd;
	guint8 canID;
	guint8 tableID;
	guint16 offset;
	guint16 count;
};

struct _MtxSocket
{
	gint fd;
	SocketType type;
};

struct _SlaveMessage
{
	guint8 canID;
	guint8 page;
	guint16 offset;
	guint16 length;
	DataSize size;
	gint value;
	void * data;
	WriteMode mode;
	SlaveMsgType type;
	RemoteAction action;
};

/* Prototypes */
void open_tcpip_sockets(void);
void close_tcpip_sockets(void);
gboolean setup_socket(gint);
void *socket_thread_manager(gpointer);
void * ascii_socket_server(gpointer );
void * binary_socket_server(gpointer );
void * control_socket_client(gpointer );
void * notify_slaves_thread(gpointer );
gboolean validate_remote_ascii_cmd(MtxSocketClient *, gchar *, gint);
void return_socket_error(gint);
void socket_get_rt_vars(gint, gchar *);
void socket_get_rtv_list(gint);
void socket_get_ecu_var(MtxSocketClient *, gchar *, DataSize);
void socket_get_ecu_vars(MtxSocketClient *, gchar *);
void socket_set_ecu_var(MtxSocketClient *, gchar *, DataSize);
gboolean check_for_changes(MtxSocketClient *);
gint * convert_socket_data(gchar *, gint);
void *network_repair_thread(gpointer);
gboolean open_network(gchar *, gint);
gboolean open_notification_link(gchar *, gint);
gboolean close_network(void);
gboolean close_control_socket(void);
gint socket_get_more_data(gint, void *, gint, gint);
gboolean open_control_socket(gchar *, gint);
void notify_slave(gpointer, gpointer);
guint8 * build_netmsg(guint8,SlaveMessage *,gint *);
guint8 * build_status_update(guint8,SlaveMessage *,gint *);
gint net_send(gint,guint8 *, gint, gint);
/* Prototypes */

#endif
#endif
