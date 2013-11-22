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
  \file src/plugins/mscommon/therm.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon temp scale handling
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __THERM_H__
#define __THERM_H__

#include <defines.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean flip_table_gen_temp_label(GtkWidget *, gpointer);
gboolean table_gen_process_and_dl(GtkWidget *, gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
