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
  \file include/comms_gui.h
  \ingroup Headers
  \brief Header for the Communications tab specific routines
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __COMMS_GUI_H__
#define __COMMS_GUI_H__

#include <gtk/gtk.h>

/* Prototypes */
gboolean check_potential_ports(const gchar *);
gboolean enumerate_dev(GtkWidget *, gpointer);	/* Help find usb/serial adapter */
gboolean reset_errcounts(GtkWidget *);
gboolean update_errcounts_wrapper(gpointer);
gboolean update_errcounts(gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
