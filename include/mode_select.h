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

#ifndef __MODE_SELECT_H__
#define __MODE_SELECT_H__

#include <gtk/gtk.h>

/* Prototypes */
gboolean drain_hashtable(gpointer, gpointer, gpointer);
void parse_ecu_capabilities(gint);
void set_dt_table_mapping_state(gboolean);
void set_dualtable_mode(gboolean);
void set_enhanced_idle_state(gboolean);
void set_enhanced_mode(gboolean);
void set_ignition_mode(gboolean);
void set_raw_memory_mode(gboolean);
void set_iac_mode(gboolean);
void set_launch_ctrl_mode(gboolean);
void set_widget_state(gpointer, gpointer);
/* Prototypes */

#endif
