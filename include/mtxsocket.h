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
/* Prototypes */

#endif
