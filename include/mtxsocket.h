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

/* Prototypes */
int setup_socket(void);
void *socket_thread_manager(gpointer);
void * socket_client(gpointer );
gboolean validate_remote_cmd(gint, gchar *, gint);
/* Prototypes */

#endif
