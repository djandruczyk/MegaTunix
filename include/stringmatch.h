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

#ifndef __STRINGMATCH_H__
#define __STRINGMATCH_H__

#include <gtk/gtk.h>
#include <enums.h>

/* Prototypes */
void build_string_2_enum_table(void);
gint translate_string(gchar *);
void dump_hash(gpointer,gpointer,gpointer);
/* Prototypes */

#endif
