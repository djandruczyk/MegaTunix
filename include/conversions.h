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

#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

#include <defines.h>
#include <gtk/gtk.h>

/* Prototypes */
void reset_temps(gpointer);
void convert_temps(gpointer,gpointer);
gint convert_before_download(GtkWidget *, gfloat);
gfloat convert_after_upload(GtkWidget *);
/* Prototypes */

#endif
