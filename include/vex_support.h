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
gboolean vetable_export(void *);
gboolean vetable_import(void *);
GIOStatus process_vex_line();
GIOStatus process_vex_range(void *, ImportParserArg, gchar *, GIOChannel * );
GIOStatus process_vex_table(void *, gchar *, GIOChannel * );
GIOStatus read_number_from_line(gint *, GIOChannel *);
GIOStatus process_header(void *, ImportParserArg , gchar *);
GIOStatus process_page(void *, gchar * );
GIOStatus handler_dispatch(void *, ImportParserFunc , ImportParserArg , gchar *, GIOChannel * );
void dealloc_vex_struct(void *);
void feed_import_data_to_ecu(void *);
void revert_to_previous_data();
gint vex_comment_parse(GtkWidget *, gpointer);
/* Prototypes */

#endif
