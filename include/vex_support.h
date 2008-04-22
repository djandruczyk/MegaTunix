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

typedef struct _Vex_Import Vex_Import;


/*!
 \brief The Vex_Import structure holds all fields (lots) needed to load and
 process a VEX (VEtabalt eXport file) and load it into megatunix.
 \see vetable_import
 \see vetable_export
 */
struct _Vex_Import
{	
	gchar *version;		/* String */
	gchar *revision;	/* String */
	gchar *comment;		/* String */
	gchar *date;		/* String */
	gchar *time;		/* String */
	gint page;		/* Int */
	gint table;		/* Int */
	gint total_x_bins;	/* Int */
	gint *x_bins;		/* Int Array, dynamic */
	gint total_y_bins;	/* Int */
	gint *y_bins;		/* Int Array, dynamic */
	gint total_tbl_bins;	/* Int */
	gint *tbl_bins;		/* Int Array, dynamic */
	gboolean got_page;	/* Flag */
	gboolean got_rpm;	/* Flag */
	gboolean got_load;	/* Flag */
	gboolean got_ve;	/* Flag */

};


/* Prototypes */
EXPORT gboolean select_vex_for_import(GtkWidget *, gpointer );
EXPORT gboolean select_vex_for_export(GtkWidget *, gpointer );
gboolean all_table_export(GIOChannel *);
gboolean all_table_import(GIOChannel *);
void single_table_export(GIOChannel *, gint );
void single_table_import(GIOChannel *, gint );
void select_table_for_export(gint);
void select_table_for_import(gint);
GIOStatus process_vex_line();
GIOStatus process_vex_range(Vex_Import *, ImportParserArg, gchar *, GIOChannel * );
GIOStatus process_vex_table(Vex_Import *, gchar *, GIOChannel * );
GIOStatus read_number_from_line(gint *, GIOChannel *);
GIOStatus process_header(Vex_Import *, ImportParserArg , gchar *);
GIOStatus process_page(Vex_Import *, gchar * );
GIOStatus process_table(Vex_Import *);
GIOStatus handler_dispatch(Vex_Import *, ImportParserFunc , ImportParserArg , gchar *, GIOChannel * );
void dealloc_vex_struct(Vex_Import *);
void feed_import_data_to_ecu(Vex_Import *);
void revert_to_previous_data();
gint vex_comment_parse(GtkWidget *, gpointer);
/* Prototypes */

#endif
