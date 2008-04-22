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

#ifndef __POST_PROCESS_H__
#define __POST_PROCESS_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
void post_process_raw_memory(void *, gint);
void update_raw_memory_view(ToggleButton, gint);
gchar * get_bin(gint );
/* Prototypes */

#endif
