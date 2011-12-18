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
  \file include/vetable_gui.h
  \ingroup Headers
  \brief Header for the VEtable rescaler code
  \author David Andruczyk
  */

#ifndef __VETABLE_GUI_H__
#define __VETABLE_GUI_H__

#include <gtk/gtk.h>
#include <enums.h>

/* Prototypes */

void rescale_table(GtkWidget * );
gboolean draw_ve_marker(void );
gfloat rescale(gfloat , ScaleOp , gfloat );

/* Prototypes */

#endif
