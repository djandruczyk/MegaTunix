/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __DATAMGMT_H__
#define __DATAMGMT_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
gint freeems_get_ecu_data(gint, gint, gint, DataSize); 
gint freeems_get_ecu_data_last(gint, gint, gint, DataSize); 
gint freeems_get_ecu_data_backup(gint, gint, gint, DataSize); 
void freeems_set_ecu_data(gint, gint, gint, DataSize, gint); 
void freeems_store_new_block(gint, gint, gint, void *, gint );
void freeems_backup_current_data(gint );
gboolean freeems_find_mtx_page(gint, gint *);
void set_ecu_data(gpointer, gint *);
gint get_ecu_data(gpointer);
void store_new_block(gpointer);
/* Prototypes */

#endif
