/* c_gauge.h - Prototypes for functions in c_gauge.c

   Copyright (C) 2000  Henning Kulander <hennikul@ifi.uio.no>
   Copyright (C) 2003  Dave J. Andruczyk <djandruczyk@yahoo.com>

   Originally called "dial"
   Code taken from gupsc written by: Henning Kulander <hennikul@ifi.uio.no>
   and adapted for use within MegaTunix.  

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

5/26/2003 - Renamed widget to C_Gauge for Circular Gauge,  Didn't like 
"dial" as to me that represented a knob, something to turn,  not a gauge.

*/

#include <libgnomecanvas/libgnomecanvas.h>

typedef struct _C_Gauge C_Gauge;

struct _C_Gauge {
	GtkWidget *canvas;
	GdkBitmap *shape;
	GnomeCanvasGroup *group;
	GnomeCanvasItem *pointer;
	GnomeCanvasItem *text_value;
	GnomeCanvasItem *circle;
	GnomeCanvasPoints *points;
	gfloat max, min;
	gfloat value, radius, center_x, center_y;
	gchar *format;
	guint32 color;
};

gboolean 
c_gauge_area_mark( C_Gauge *c_gauge, guint32 color, 
		gdouble s_value, gdouble b_value );

gboolean 
c_gauge_value_set( C_Gauge *c_gauge, gdouble value );

gboolean 
c_gauge_make( C_Gauge *c_gauge, GtkWidget *canvas, char *label, gchar *format, 
	   gchar *fontname, guint32 color, gint labels[]);

