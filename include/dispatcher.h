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
  \file include/dispatcher.h
  \ingroup Headers
  \brief Headers for Gui/postfunction dispatchers
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include <gtk/gtk.h>

/* Prototypes */
void *clock_watcher(gpointer);
gboolean process_pf_message(gpointer);
gboolean process_gui_message(gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
