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
  \file src/plugins/libreems/libreems_rtv_processor.h
  \ingroup LibreEMSPlugin,Headers
  \brief LibreEMS RTV processor
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LIBREEMS_RTV_PROCESSOR_H__
#define __LIBREEMS_RTV_PROCESSOR_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <threads.h>

/* Prototypes */
gdouble common_rtv_processor(gconstpointer *, gchar *, ComplexExprType);
gdouble common_rtv_processor_obj(GObject *, gchar *, ComplexExprType);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
