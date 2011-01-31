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

#ifndef __POLYGONS_H__
#define __POLYGONS_H__

#include <defines.h>
#include <gauge.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean create_polygon_event(GtkWidget *, gpointer );
gboolean new_poly_response(GtkDialog *, gint, gpointer);
void reset_onscreen_polygons(void);
void update_onscreen_polygons(void);
gboolean alter_polygon_data(GtkWidget *, gpointer );
gboolean remove_polygon(GtkWidget *, gpointer );
GtkWidget *build_polygon(MtxPolygon *, gint );
/* Prototypes */

#endif
