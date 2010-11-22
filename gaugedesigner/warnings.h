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

#ifndef __WARNINGS_H__
#define __WARNINGS_H__

#include <defines.h>
#include <gauge.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean create_warning_span_event(GtkWidget *, gpointer );
void reset_onscreen_w_ranges(void);
void update_onscreen_w_ranges(void);
gboolean alter_w_range_data(GtkWidget *, gpointer );
gboolean remove_w_range(GtkWidget *, gpointer );
/* Prototypes */

#endif
