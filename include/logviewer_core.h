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

#ifndef __LOGVIEWER_CORE_H__
#define __LOGVIEWER_CORE_H__

#include <gtk/gtk.h>

/* Prototypes */
void load_logviewer_file(void * );
void read_log_header(GIOChannel *, void * );
void read_log_data(GIOChannel *, void * );
void initialize_log_info(void *);
void allocate_buffers(void *);
/* Prototypes */

#endif
