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

#ifndef __ALERTS_H__
#define __ALERTS_H__

#include <defines.h>
#include <gauge.h>
#include <gtk/gtk.h>


/* Prototypes */
EXPORT gboolean create_alert_span_event(GtkWidget *, gpointer );
void reset_onscreen_a_ranges(void);
void update_onscreen_a_ranges(void);
gboolean alter_a_range_data(GtkWidget *, gpointer );
gboolean remove_a_range(GtkWidget *, gpointer );
GtkWidget * build_a_range(MtxAlertRange *, gint );
/* Prototypes */

#endif
