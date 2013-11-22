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
  \file include/vetable_gui.h
  \ingroup Headers
  \brief Header for the VEtable rescaler code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __VETABLE_GUI_H__
#define __VETABLE_GUI_H__

#include <gtk/gtk.h>
#include <enums.h>

/* Prototypes */
gboolean draw_ve_marker(void );
gfloat rescale(gfloat , ScaleOp , gfloat );
void rescale_table(GtkWidget * );
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
