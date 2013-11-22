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
  \file include/keybinder.h
  \ingroup Headers
  \brief Header for the object config file key binding routines
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __KEYBINDER_H__
#define __KEYBINDER_H__

#include <gtk/gtk.h>
#include <configfile.h>

/* Prototypes */
void bind_keys(GObject *, ConfigFile *, gchar *, gchar **, gint);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
