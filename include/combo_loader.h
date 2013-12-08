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
  \file include/combo_loader.h
  \ingroup Headers
  \brief Header for the Mtx Specific combobox loader/initializer
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __COMBO_LOADER_H__
#define __COMBO_LOADER_H__

#include <configfile.h>
#include <gtk/gtk.h>

typedef enum 
{
	CHOICE_COL,
	BITVAL_COL,
	COMBO_COLS
}ComboCols;

/* Prototypes */
gboolean combo_match_selected(GtkEntryCompletion *, GtkTreeModel *, GtkTreeIter *, gpointer);
void combo_setup(GObject *, ConfigFile *, gchar * );
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
