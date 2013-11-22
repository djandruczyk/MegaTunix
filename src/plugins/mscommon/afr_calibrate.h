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
  \file src/plugins/mscommon/afr_calibrate.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon Temp table management
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __AFR_CALIBRATE_H__
#define __AFR_CALIBRATE_H__

#include <defines.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean afr_calibrate_calc_and_dl(GtkWidget *, gpointer);
void afr_combo_changed(GtkWidget *, gpointer);
gboolean populate_afr_calibrator_combo(GtkWidget *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
