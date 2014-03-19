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
  \file src/plugins/libreems/datamgmt.h
  \ingroup LibreEMSPlugin,Headers
  \brief LibreEMS Data management functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DATAMGMT_H__
#define __DATAMGMT_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
void libreems_backup_current_data(gint, gint );
gboolean libreems_find_mtx_page(gint, gint *);
gint libreems_get_ecu_data(gint, gint, gint, DataSize); 
gint libreems_get_ecu_data_backup(gint, gint, gint, DataSize); 
gint libreems_get_ecu_data_last(gint, gint, gint, DataSize); 
void libreems_set_ecu_data(gint, gint, gint, DataSize, gint); 
void libreems_store_new_block(gint, gint, gint, void *, gint );
void set_ecu_data(gpointer, gint *);
gint get_ecu_data(gpointer);
void store_new_block(gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
