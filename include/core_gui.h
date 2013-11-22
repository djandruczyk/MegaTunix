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
  \file include/core_gui.h
  \ingroup Headers
  \brief Header for Core gui initialization handlers
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CORE_GUI_H__
#define __CORE_GUI_H__

#include <gtk/gtk.h>
#include <glade/glade.h>

/* Prototypes */
void finalize_core_gui(GladeXML *);
void set_connected_icons_state(gboolean);
gint setup_gui(void);
void setup_main_status(GtkWidget *);

/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
