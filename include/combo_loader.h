/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \brief
  \author David Andruczyk
  */

#ifndef __COMBO_LOADER_H__
#define __COMBO_LOADER_H__

#include <configfile.h>
#include <gtk/gtk.h>

enum
{
	CHOICE_COL,
	BITVAL_COL,
	COMBO_COLS
}ComboCols;

/* Prototypes */
void combo_setup(GObject *, ConfigFile *, gchar * );
gboolean combo_match_selected(GtkEntryCompletion *, GtkTreeModel *, GtkTreeIter *, gpointer);
/* Prototypes */

#endif
