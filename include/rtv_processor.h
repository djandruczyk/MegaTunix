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

#ifndef __RTV_PROCESSOR_H__
#define __RTV_PROCESSOR_H__

#include <gtk/gtk.h>
#include <configfile.h>

/* Prototypes */
void process_rt_vars(void * );
gfloat lookup_data(GObject *, gint );
gdouble handle_complex_expr(GObject *, void *);
/* Prototypes */

#endif
