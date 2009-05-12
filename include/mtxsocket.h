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

#include <enums.h>
#include <gtk/gtk.h>


typedef enum
{
	MTX_ASCII = 0x410,
	MTX_BINARY
}SocketMode;


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

typedef struct _MtxSocketClient MtxSocketClient;

struct _MtxSocketClient 
{
	gchar *ip;
	guint16 port;
	SocketMode mode;
	guint8 ** ecu_data;
	gint fd;
};
/* Prototypes */
int setup_socket(void);
void *socket_thread_manager(gpointer);
void * socket_client(gpointer );
gboolean validate_remote_ascii_cmd(MtxSocketClient *, gchar *, gint);
gboolean validate_remote_binary_cmd(MtxSocketClient *, gchar *, gint);
/* Socket handler functions */
void return_socket_error(gint);
void socket_get_rt_vars(gint, gchar *);
void socket_get_rtv_list(gint);
void socket_get_ecu_var(MtxSocketClient *, gchar *, DataSize);
void socket_get_ecu_vars(MtxSocketClient *, gchar *);
void socket_set_ecu_var(MtxSocketClient *, gchar *, DataSize);
gboolean check_for_changes(MtxSocketClient *);
gint * convert_socket_data(gchar *, gint);
void *network_repair_thread(gpointer);
/* Prototypes */

#endif
