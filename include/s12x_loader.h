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

#ifndef __S12X_LOADER_H__
#define __S12X_LOADER_H__

#include <winserialio.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean do_ms2_load(gint, gint);
gboolean do_freeems_load(gint, gint);
gint read_s19(gint);
void ms2_enter_boot_mode(gint);
gboolean wakeup_S12(gint);
gboolean check_status(gint);
gboolean erase_S12(gint);
gboolean send_S12(gint, guint);
void free_s19(guint);
void reset_proc(gint);
void output(gchar *, gboolean);

#endif
