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

#ifndef __DATAIO_H__
#define __DATAIO_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <threads.h>

/* Prototypes */
gboolean handle_ecu_data(InputHandler, Io_Message * );
void dump_output(gint, guchar *);
gint read_data(gint , void **);
/* Prototypes */

#endif
