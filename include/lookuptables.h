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

/* MegaSquirt Linux tuning lookup tables for converting incoming values
 * to real world numbers. From Bruce Bowlings PCC tuning wsoftware.
 */

#ifndef __LOOKUPTABLES_H__
#define __LOOKUPTABLES_H__

#include <enums.h>

void load_lookuptables(void *);
gboolean load_table(gchar *, gchar *);
void get_table(gpointer, gpointer, gpointer );
gint reverse_lookup(gint , gint *);
gfloat lookup_data(GObject *, gint );

#endif
