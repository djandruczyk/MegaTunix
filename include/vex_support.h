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

#ifndef __VEX_SUPPORT_H__
#define __VEX_SUPPORT_H__

#include <gtk/gtk.h>
#include <enums.h>

/* Prototypes */
gboolean vetable_export(void);
gboolean vetable_import(void);
gint process_vex_line(void);
gint process_vex_rpm_range(void);
gint process_vex_map_range(void);
gint process_vex_table(void);
gint read_number_from_line(void);
void clear_vexfile(void);
gint vex_comment_parse(GtkWidget *, gpointer);
/* Prototypes */

#endif
