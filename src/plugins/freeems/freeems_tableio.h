/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file include/freeems_tableio.h
  \ingroup Headers
  \brief freeems_tableio header
  \author David Andruczyk
  */

#ifndef __FREEEMS_TABLEIO_H__
#define __FREEEMS_TABLEIO_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
void ecu_table_import(gint, gfloat *, gfloat *, gfloat *);
gint * convert_bins(gint, gfloat *, Axis);
/* Prototypes */

#endif
