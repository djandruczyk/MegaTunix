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
  \file src/plugins/mscommon/dep_loader.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon dependancy loading
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DEP_LOADER_H__
#define __DEP_LOADER_H__

#include <configfile.h>
#include <enums.h>
#include <libxml/parser.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean check_size(DataSize);
void load_dependencies(gconstpointer *,xmlNode *, const gchar *);
void load_dependencies_obj(GObject *,ConfigFile * , const gchar *, const gchar *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
