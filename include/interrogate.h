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

#ifndef __INTERROGATE_H__
#define __INTERROGATE_H__

#include <gtk/gtk.h>

/* Prototypes */
void interrogate_ecu(void);
void determine_ecu(void *, GArray *,GHashTable *);
GArray * validate_and_load_tests(GHashTable *);
void free_test_commands(GArray *);
void parse_bytecounts(GArray *, GHashTable *, void *);
void close_profile(void * );
void * load_profile(GArray * , gchar * );
gint translate_capabilities(gchar *);
/* Prototypes */

#endif
