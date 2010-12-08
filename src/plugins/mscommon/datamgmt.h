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

/* Externs */
extern void (*get_symbol_f)(const gchar *, void **);
/* Externs */

/* Prototypes */
gint ms_get_ecu_data(gint, gint, gint, DataSize); 
gint ms_get_ecu_data_last(gint, gint, gint, DataSize); 
gint ms_get_ecu_data_backup(gint, gint, gint, DataSize); 
void ms_set_ecu_data(gint, gint, gint, DataSize, gint); 
void ms_set_ecu_data_pending(gint, gint, gint, DataSize, gint); 
void ms_store_new_block_pending(gint, gint, gint, void *, gint );
void ms_store_new_blocs(gint, gint, gint, void *, gint );
void ms_backup_current_data(gint, gint );
gboolean ms_find_mtx_page(gint,gint *);
void set_ecu_data(gconstpointer *);
void store_new_block(gconstpointer *);
/* Prototypes */

#endif
