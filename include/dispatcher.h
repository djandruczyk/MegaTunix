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

#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
gboolean dispatcher(gpointer);
void dealloc_textmessage(struct Text_Message * );
void dealloc_message(struct Io_Message * );
void dealloc_w_update(struct Widget_Update * );
/* Prototypes */

#endif
