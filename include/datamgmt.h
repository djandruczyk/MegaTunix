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

#ifndef __DATAMGMT_H__
#define __DATAMGMT_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
gint get_ecu_data(gint, gint, gint, DataSize); 
gint get_ecu_data_last(gint, gint, gint, DataSize); 
gint get_ecu_data_backup(gint, gint, gint, DataSize); 
gint _get_sized_data(guint8 *, gint, gint, DataSize); 

void set_ecu_data(gint, gint, gint, DataSize, gint); 

void store_new_block(gint, gint, gint, void *, gint );
void backup_current_data(gint, gint );
/* Prototypes */

#endif
