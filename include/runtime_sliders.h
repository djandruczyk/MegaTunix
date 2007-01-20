/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __RUNTIME_SLIDERS_H__
#define __RUNTIME_SLIDERS_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
void load_sliders(void );
void load_ve3d_sliders(gint );
void register_rt_range(GtkWidget *);
struct Rt_Slider * add_slider(gchar *, gint, gint, gint, gchar *,TabIdent );
gboolean free_ve3d_sliders(gint);
/* Prototypes */

#endif
