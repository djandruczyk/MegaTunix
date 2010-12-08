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

#ifndef __USER_OUTPUTS_H__
#define __USER_OUTPUTS_H__

#include <gtk/gtk.h>

/* Externs */
extern void *(*evaluator_create_f)(char *);
extern void *(*evaluator_destroy_f)(void *);
extern double (*evaluator_evaluate_x_f)(void *, double);
extern void (*dbg_func_f)(gint, gchar *);
extern glong (*get_extreme_from_size_f)(DataSize, Extreme);
extern gfloat (*direct_lookup_data_f)(gchar *, gint );
extern gint (*direct_reverse_lookup_f)(gchar *, gint );

/* Externs */

enum
{
        UO_CHOICE_COL,
        UO_BITVAL_COL,
	UO_DL_CONV_COL,
	UO_UL_CONV_COL,
	UO_LOWER_COL,
	UO_UPPER_COL,
	UO_RANGE_COL,
	UO_SIZE_COL,
	UO_PRECISION_COL,
        UO_COMBO_COLS
}UOComboCols;

/* Prototypes */
 void build_model_and_view(GtkWidget *);
GtkTreeModel * create_model(void);
void add_columns (GtkTreeView *, GtkWidget *);
void update_model_from_view(GtkWidget * );
void cell_edited(GtkCellRendererText *, const gchar * ,const gchar * ,gpointer );
gboolean force_view_recompute(gpointer);
gboolean deferred_model_update(GtkWidget * );
 void ms2_output_combo_setup(GtkWidget *);

/* Prototypes */

#endif
