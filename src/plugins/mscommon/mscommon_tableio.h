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

#ifndef __MSCOMMON_TABLEIO_H__
#define __MSCOMMON_TABLEIO_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <tableio.h>

/* Prototypes */
void ecu_table_import(gint, gfloat *, gfloat *, gfloat *);
gint * convert_toecu_bins(gint, gfloat *, Axis);
gfloat * convert_fromecu_bins(gint, Axis);
TableExport * ecu_table_export(gint);
/* Prototypes */

#endif
