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

#include <configfile.h>
#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
void interrogate_ecu(void);
gboolean determine_ecu(Canidate *, GArray *,GHashTable *);
GArray * validate_and_load_tests(GHashTable *);
void free_test_commands(GArray *);
void load_bytecounts(GArray *, GHashTable *, ConfigFile *);
void close_profile(Canidate * );
Canidate * load_potential_match(GArray * , gchar * );
gboolean check_for_match(GArray *, Canidate *, Canidate *);
void load_profile_details(Canidate * );
gint translate_capabilities(gchar *);
/* Prototypes */

#endif
