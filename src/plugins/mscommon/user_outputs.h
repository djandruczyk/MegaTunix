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
  \file src/plugins/mscommon/user_outputs.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon user output handling
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __USER_OUTPUTS_H__
#define __USER_OUTPUTS_H__

#include <gtk/gtk.h>

/* Prototypes */
void add_columns (GtkTreeView *, GtkWidget *);
void build_model_and_view(GtkWidget *);
GtkTreeModel * create_model(void);
void cell_edited(GtkCellRendererText *, const gchar * ,const gchar * ,gpointer );
gboolean deferred_model_update(GtkWidget * );
gboolean force_view_recompute_wrapper(gpointer);
gboolean force_view_recompute(gpointer);
void update_model_from_view(GtkWidget * );
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
