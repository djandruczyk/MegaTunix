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

#ifndef __THREADS_H__
#define __THREADS_H__

#include <gtk/gtk.h>

/* Prototypes */
void * raw_reader_thread(void *); /* Serial raw reader thread */
int stop_serial_thread();       /* cancels reader thread */
void start_serial_thread(void); /*bootstrp function fopr above */
void *reset_reader_locks(void *);
/* Prototypes */

#endif
