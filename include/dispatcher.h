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

/* Prototypes */
gboolean dispatcher(gpointer);
gboolean textmessage_dispatcher(gpointer);
void dealloc_textmessage(void * );
void dealloc_message(void * );
/* Prototypes */

#endif
