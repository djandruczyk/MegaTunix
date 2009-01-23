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

#ifndef __MS2_LOADER_H__
#define __MS2_LOADER_H__

#include <winserialio.h>
#include <gtk/gtk.h>

/* Prototypes */
void do_ms2_load(gint, gint);
gint read_s19(gint);
void enter_boot_mode(gint);
gboolean wakeup_S12(gint);
void erase_S12(gint);
void send_S12(gint, gint);
void free_s19(gint);
void reset_proc(gint);
void output(gchar *, gboolean);

#endif
