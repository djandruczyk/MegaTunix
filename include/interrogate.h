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
gboolean determine_ecu(GArray *,GHashTable *);
GArray * validate_and_load_tests(GHashTable ** );
gboolean check_for_match(GHashTable *,gchar *);
void free_results_array(GArray *);
void free_tests_array(GArray *);
void interrogate_error(gchar *, gint);
gint translate_capabilities(gchar *);
gboolean load_firmware_details(Firmware_Details *, gchar * );
void update_interrogation_gui(Firmware_Details *,GHashTable *);
/* Prototypes */

#endif
