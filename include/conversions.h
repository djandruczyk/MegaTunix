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
void read_conversions(void);
void reset_temps(gpointer);
gint convert_before_download_p0(gint, gfloat);
gfloat convert_after_upload_p0(gint);
gint convert_before_download_p1(gint, gfloat);
gfloat convert_after_upload_p1(gint);
/* Prototypes */

struct Conversion_Chart
{
	gint page0_conv_type[PAGE_SIZE];
	gfloat page0_conv_factor[PAGE_SIZE];
	gint page1_conv_type[PAGE_SIZE];
	gfloat page1_conv_factor[PAGE_SIZE];
};

#endif
