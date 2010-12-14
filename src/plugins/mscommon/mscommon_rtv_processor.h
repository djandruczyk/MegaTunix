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

#ifndef __MSCOMMON_RTV_PROCESSOR_H__
#define __MSCOMMON_RTV_PROCESSOR_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <threads.h>

/* Externs */
extern guint (*get_bitshift_f)(guint);
/* Externs */

/* Prototypes */
gdouble common_rtv_processor(gconstpointer *, gchar *, ComplexExprType);
gdouble common_rtv_processor_obj(GObject *, gchar *, ComplexExprType);
/* Prototypes */

#endif
