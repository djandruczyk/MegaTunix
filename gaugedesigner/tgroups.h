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

#ifndef __TGROUPS_H__
#define __TGROUPS_H__

#include <defines.h>
#include <gauge.h>
#include <gtk/gtk.h>


/* Prototypes */
EXPORT gboolean create_tick_group_event(GtkWidget *, gpointer );
void reset_onscreen_tgroups(void);
void update_onscreen_tgroups(void);
gboolean alter_tgroup_data(GtkWidget *, gpointer );
gboolean remove_tgroup(GtkWidget *, gpointer );
GtkWidget * build_tgroup(MtxTickGroup *, gint );
/* Prototypes */

#endif
