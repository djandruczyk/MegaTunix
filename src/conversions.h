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

/* Conversions.h structures */

#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

#include <config.h>
#include <sys/types.h>

struct Conversion_Chart
{
	gchar *conv_type[128];
	gint conv_factor[128];
};

#endif
