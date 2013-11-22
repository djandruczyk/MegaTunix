/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/ms2/user_outputs.h
  \ingroup MS2Plugin,Headers
  \brief MS2 user outputs code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __USER_OUTPUTS_H__
#define __USER_OUTPUTS_H__

#include <gtk/gtk.h>

typedef enum
{
	UO_CHOICE_COL,
	UO_BITVAL_COL,
	UO_FROMECU_MULT_COL,
	UO_FROMECU_ADD_COL,
	UO_RAW_LOWER_COL,
	UO_RAW_UPPER_COL,
	UO_RANGE_COL,
	UO_RANGE_TEMPC_COL,
	UO_RANGE_TEMPF_COL,
	UO_RANGE_TEMPK_COL,
	UO_SIZE_COL,
	UO_PRECISION_COL,
	UO_TEMP_DEP_COL,
	UO_COMBO_COLS
}UOComboCols;

/* Prototypes */
gboolean find_in_list(gchar **, gchar *);
void ms2_output_combo_setup(GtkWidget *);
void update_ms2_user_outputs(GtkWidget *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
