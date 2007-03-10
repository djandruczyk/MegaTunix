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

#include <enums.h>
#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
EXPORT gboolean select_vex_for_export(GtkWidget *, gpointer );
EXPORT gboolean select_vex_for_export(GtkWidget *, gpointer );
gboolean all_table_export(GIOChannel *);
gboolean all_table_import(GIOChannel *);
void single_table_export(GIOChannel *, gint );
void single_table_import(GIOChannel *, gint );
void select_table_for_export(gint);
void select_table_for_import(gint);
GIOStatus process_vex_line();
GIOStatus process_vex_range(struct Vex_Import *, ImportParserArg, gchar *, GIOChannel * );
GIOStatus process_vex_table(struct Vex_Import *, gchar *, GIOChannel * );
GIOStatus read_number_from_line(gint *, GIOChannel *);
GIOStatus process_header(struct Vex_Import *, ImportParserArg , gchar *);
GIOStatus process_page(struct Vex_Import *, gchar * );
GIOStatus process_table(struct Vex_Import *);
GIOStatus handler_dispatch(struct Vex_Import *, ImportParserFunc , ImportParserArg , gchar *, GIOChannel * );
void dealloc_vex_struct(struct Vex_Import *);
void feed_import_data_to_ecu(struct Vex_Import *);
void revert_to_previous_data();
gint vex_comment_parse(GtkWidget *, gpointer);
/* Prototypes */

#endif
