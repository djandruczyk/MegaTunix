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
  \file include/about_gui.h
  \ingroup Headers
  \brief Header for the about_gui tab
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ABOUT_H__
#define __ABOUT_H__

#include <defines.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean about_popup(GtkWidget *, gpointer );
void build_about(GtkWidget *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
