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

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

/* Prototypes */
gfloat *convert_fromecu_bins(gint, Axis);
gint *convert_toecu_bins(gint, gfloat *, Axis);
void ecu_table_import(gint, gfloat *, gfloat *, gfloat *);
void export_single_table(gint);
void export_table_to_yaml(gchar *, gint);
const gchar *get_table_suffix(gint, Axis);
void import_single_table(gint);
void select_all_tables_for_export(void);
/* Prototypes */

#endif
#ifdef __cplusplus
}
#endif
