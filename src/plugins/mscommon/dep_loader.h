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
  \file src/plugins/mscommon/dep_loader.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon dependancy loading
  \author David Andruczyk
  */

#ifndef __DEP_LOADER_H__
#define __DEP_LOADER_H__

#include <configfile.h>
#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
void load_dependancies(gconstpointer *,ConfigFile * ,gchar *, gchar *);
void load_dependancies_obj(GObject *,ConfigFile * ,gchar *, gchar *);
gboolean check_size(DataSize );
/* Prototypes */

#endif
