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
  \file include/tableio.h
  \ingroup Headers
  \brief tableio header
  \author David Andruczyk
  */

#ifndef __TABLEIO_H__
#define __TABLEIO_H__

#include <gtk/gtk.h>

typedef struct _TableExport TableExport;

struct _TableExport 
{
	gint x_len;
	gint y_len;
	gfloat *x_bins;
	gfloat *y_bins;
	gfloat *z_bins;
	const gchar * x_label;
	const gchar * y_label;
	const gchar * z_label;
	const gchar * x_unit;
	const gchar * y_unit;
	const gchar * z_unit;
	const gchar * title;
	const gchar * desc;
};

/* Prototypes */
void import_single_table(gint);
void export_single_table(gint);
/* Prototypes */

#endif
