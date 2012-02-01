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
  \file src/plugins/mscommon/mscommon_rtv_loader.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon rtv map loader 
  \author David Andruczyk
  */

#ifndef __MSCOMMON_RTV_LOADER_H__
#define __MSCOMMON_RTV_LOADER_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <threads.h>

/* Prototypes */
void common_rtv_loader(gconstpointer *, ConfigFile *, gchar *, gchar *, ComplexExprType);
void common_rtv_loader_obj(GObject *, ConfigFile *, gchar *, gchar *, ComplexExprType);
/* Prototypes */

#endif
