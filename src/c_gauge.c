/* c_gauge - A Round gauge (like an automotive or instrumentation gauge
    component in a GNOME widget.

   Copyright (C) 2000  Henning Kulander <hennikul@ifi.uio.no>
   Copyright (C) 2003  Dave J. Andruczyk <djandruczyk@yahoo.com>

   Originally called "dial".
   Code taken from gupsc written by:Henning Kulander <hennikul@ifi.uio.no>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <libgnomecanvas/libgnomecanvas.h>
#include <gtk/gtk.h>
#include <errno.h>

#include "c_gauge.h"
#include <math.h>

#define STEP 2
#define DIAL_P 0.9
#define CENTER_CIRCLE_RADIUS 5

/* Returns the coordinates of a point on a circle */
static void 
point_on_circle (gdouble c_x, gdouble c_y, gdouble radius, 
		 gdouble degree, gdouble *x, gdouble *y)
{
	gdouble r;
	
	r = ((225 - degree) * M_PI) / 180;
	*x = c_x + radius * cos (r);
	*y = c_x - radius * sin (r);
}

/* Converts value within a range to degrees */
static gdouble 
value_to_degree (C_Gauge * c_gauge, gdouble value)
{
	return (value - c_gauge->min) / (c_gauge->max - c_gauge->min) * 270;
}

/* Mark a given area with a color. */
gboolean 
c_gauge_area_mark (C_Gauge *c_gauge, guint32 color, 
		gdouble s_value, gdouble b_value)
{
	gdouble temp, s_degree, b_degree, x, y;
	GnomeCanvasPoints *points;
	GnomeCanvasItem *bow;
	guint counter=0;

	g_return_val_if_fail (c_gauge != NULL, FALSE);
	g_return_val_if_fail (s_value != b_value, FALSE); /* Area == 0 */

        /* s_value must be smaller than b_value*/
	if (s_value > b_value) {
		temp = s_value;
		s_value = b_value;
		b_value = temp;
	}

	s_degree = value_to_degree (c_gauge, s_value);
	b_degree = value_to_degree (c_gauge, b_value);

	/* Calculate number of points needed */
	for (temp = s_degree, counter = 1; temp < b_degree; temp += STEP)
		counter++;
	points = gnome_canvas_points_new (counter);

	/* Find the points along the circle */
	counter = 0;
	while (s_degree < b_degree) {
		point_on_circle (c_gauge->center_x, c_gauge->center_y, 
				 c_gauge->radius - 2, s_degree, &x, &y);
		points->coords[counter++]=x;
		points->coords[counter++]=y;
		s_degree += STEP;
	}
	point_on_circle (c_gauge->center_x, c_gauge->center_y, 
			 c_gauge->radius - 2, b_degree, &x, &y);		
	points->coords[counter++]=x; /* Remember last point */
	points->coords[counter++]=y;

	/* Draw the arc */
	bow = gnome_canvas_item_new (c_gauge->group,
				     gnome_canvas_line_get_type (),
				     "points", points,
				     "smooth", TRUE,
				     "fill_color_rgba", color,
				     "width_pixels", 5,
				     NULL);
	
	/* Lower it to not interfear with other items in the c_gauge. */
	gnome_canvas_item_lower_to_bottom (bow);
	gnome_canvas_item_lower_to_bottom (c_gauge->circle);
	
	gnome_canvas_points_unref (points);

	return TRUE;
}

/* Make the pointer point at a given value */
gboolean 
c_gauge_value_set (C_Gauge *c_gauge, gdouble value)
{
	gchar *text;
	gdouble x, y;

	g_return_val_if_fail (c_gauge != NULL, FALSE);

	/* Set the text field representing the new value */
	c_gauge->value = value;
	text = g_strdup_printf (c_gauge->format, value); 
	gnome_canvas_item_set (c_gauge->text_value,
			       "text", text,
			       NULL);
	g_free(text);

	/* Set value within limits */
	value = (value > c_gauge->max ? c_gauge->max : value);
	value = (value < c_gauge->min ? c_gauge->min : value);
	
	/* Find coordinates for the start- and endingpoint of the line*/
	point_on_circle (c_gauge->center_x, c_gauge->center_y, c_gauge->radius - 20, 
			 value_to_degree (c_gauge, value), &x, &y );
	c_gauge->points->coords[0]=c_gauge->center_x;
	c_gauge->points->coords[1]=c_gauge->center_y;
	c_gauge->points->coords[2]=x;
	c_gauge->points->coords[3]=y;

	/* Repaint the line with new coordinates */
	gnome_canvas_item_set (c_gauge->pointer,
			       "points", c_gauge->points,
			       NULL);
	return TRUE;
}

/* Make a new c_gauge with a format for the value-label and labels 
 * around circle*/
gboolean 
c_gauge_make (C_Gauge *c_gauge, GtkWidget *canvas, char *label, char *format, 
	   gchar *fontname, guint32 color, gint labels[])
{
	gdouble width, height;
	gdouble x1, x2, y1, y2;
	gdouble square_size, diameter;
	gdouble left_margin, top_margin;
	gdouble degree;
	GdkGC *gc;
	GdkVisual *visual;
	gint counter, last;
	gchar *text;

	/* Update values in structure and find usefull values */
	c_gauge->canvas = canvas;
	c_gauge->group = gnome_canvas_root (GNOME_CANVAS (canvas));
	c_gauge->format = format;
	c_gauge->color = color;
	c_gauge->points = gnome_canvas_points_new (2);

	gnome_canvas_get_scroll_region (GNOME_CANVAS (canvas), 
					&x1, &y1, &x2, &y2 );
	width  = x2 - x1;
	height = y2 - y1;
	square_size = (width < height ? width : height);
	diameter = square_size*DIAL_P;
	c_gauge->radius = diameter / 2;
	
	left_margin = (width  - diameter)/2;
	top_margin  = (height - diameter)/2;
	c_gauge->center_x =  width/2;
	c_gauge->center_y = height/2;


	/* Create an apply mask for the canvas */
	c_gauge->shape = gdk_pixmap_new (NULL, width, height, 1);
	gc = gdk_gc_new (c_gauge->shape);
	visual = gdk_visual_get_best_with_depth (1);
	gdk_gc_set_function (gc, GDK_CLEAR);
	gdk_gc_set_fill (gc, GDK_SOLID);
	gdk_draw_rectangle (c_gauge->shape, /* Clear rectangle */
			    gc,
			    TRUE,
			    0, 0,
			    width, height);
	gdk_gc_set_function (gc, GDK_INVERT);
	gdk_draw_arc (c_gauge->shape ,  /* Draw circle */
		      gc, 
		      TRUE,
		      left_margin-2, top_margin-2,
		      width*DIAL_P+4, height*DIAL_P+4,
		      0, 360*64);
	gtk_widget_shape_combine_mask (c_gauge->canvas ,
				       c_gauge->shape,
				       0, 0);
				  
	
	/* Draw circle */
	c_gauge->circle = gnome_canvas_item_new (c_gauge->group,
					      gnome_canvas_ellipse_get_type (),
					      "x1", left_margin,
					      "y1", top_margin,
					      "x2", left_margin+diameter,
					      "y2", top_margin+diameter,
					      "fill_color", "black",
					      "outline_color", "black",
					      "width_pixels", 0,
					      NULL);

	/* Draw bordercircle */
	gnome_canvas_item_new (c_gauge->group, 
			       gnome_canvas_ellipse_get_type (),
			       "x1", left_margin-1,
			       "y1", top_margin-1,
			       "x2", left_margin+diameter+1,
			       "y2", top_margin+diameter+1,
			       "outline_color", "grey55",
			       "fill_color", NULL,
			       "width_pixels", 2,
			       NULL);

	/* Draw spikes around the edges */
	for (degree = 0; degree <= 270; degree += 22.5) {
		gfloat pin_radius;
		
		/* Find length of spike, and coordinates for the spike*/
		pin_radius = c_gauge->radius 
			* ( (int) degree == degree ? 0.8 : 0.9 );
		point_on_circle (c_gauge->center_x, c_gauge->center_y, 
				 pin_radius, degree, 
				 &x1, &y1);		
		point_on_circle (c_gauge->center_x, c_gauge->center_y, 
				 c_gauge->radius, degree, 
				 &x2, &y2);
		c_gauge->points->coords[0]=x1;
		c_gauge->points->coords[1]=y1;
		c_gauge->points->coords[2]=x2;
		c_gauge->points->coords[3]=y2;
		
		/* Draw the spike */
		gnome_canvas_item_new (c_gauge->group,
				       gnome_canvas_line_get_type (),
				       "points", c_gauge->points,
				       "fill_color", "grey55",
				       "width_pixels", 1,
				       NULL);
	}

	/* Draw labels: */
	
	/* Find smallest and largest label */
	last = G_MININT;
	c_gauge->min = labels[0];
	for (counter = 0; labels[counter] > last; counter++)
		last = labels[counter];
	c_gauge->max = labels[counter - 1];
	last = counter;

	/* Make the labels: */
	for (counter = 0; counter < last; counter++) {
		int text_radius = c_gauge->radius - 15;
		gchar *llabel;
		
		point_on_circle (c_gauge->center_x, c_gauge->center_y, 
				 text_radius, 
				 (labels[counter] - c_gauge->min) 
				 / (c_gauge->max - c_gauge->min) * 270,
				 &x1, &y1);
		g_snprintf (llabel = g_malloc (8), 8, "%d", labels[counter]); 
		gnome_canvas_item_new (c_gauge->group,
				       gnome_canvas_text_get_type (),
				       "text", llabel,
				       "x", x1,
				       "y", y1,
				       "anchor", GTK_ANCHOR_CENTER,
				       "font_gdk", gdk_font_load (fontname),
				       "clip_width", 100.0,
				       "clip_height", 20.0,
				       "fill_color", "white",
				       NULL);
		g_free (llabel);
	}

	/* Draw TextLabel */
	gnome_canvas_item_new (c_gauge->group,
			       gnome_canvas_text_get_type (),
			       "text", label,
			       "x", c_gauge->center_x,
			       "y", c_gauge->center_y + c_gauge->radius - 20,
			       "anchor", GTK_ANCHOR_CENTER,
			       "justification", GTK_JUSTIFY_CENTER,
			       "font_gdk", gdk_font_load (fontname),
			       "clip_width", c_gauge->radius,
			       "clip_height", 40.0,
			       "fill_color", "white",
			       NULL);
	
	/* Draw value */
	c_gauge->value = 0;
	text = g_strdup_printf (format, c_gauge->value);
	c_gauge->text_value = 
		gnome_canvas_item_new (c_gauge->group,
				       gnome_canvas_text_get_type (),
				       "text", text,
				       "x", c_gauge->center_x,
				       "y", c_gauge->center_y+7,
				       "anchor", GTK_ANCHOR_NORTH,
				       "font_gdk", gdk_font_load (fontname),
				       "clip_width", 100.0,
				       "clip_height", 20.0,
				       "fill_color", "white",
				       NULL);
	g_free (text);

	/* Draw center_circle */
	gnome_canvas_item_new (c_gauge->group,
			       gnome_canvas_ellipse_get_type (),
			       "x1", c_gauge->center_x - CENTER_CIRCLE_RADIUS,
			       "y1", c_gauge->center_y - CENTER_CIRCLE_RADIUS,
			       "x2", c_gauge->center_x + CENTER_CIRCLE_RADIUS,
			       "y2", c_gauge->center_y + CENTER_CIRCLE_RADIUS,
			       "fill_color_rgba", color,
			       "outline_color_rgba", color,
			       "width_pixels", 0,
			       NULL); 

	/* Draw pointer */
	point_on_circle (c_gauge->center_x, c_gauge->center_y, c_gauge->radius - 20, 
			 c_gauge->value, &x1, &y1 );		
	c_gauge->points->coords[0]=c_gauge->center_x;
	c_gauge->points->coords[1]=c_gauge->center_y;
	c_gauge->points->coords[2]=x1;
	c_gauge->points->coords[3]=y1;
	c_gauge->pointer = gnome_canvas_item_new (c_gauge->group,
					       gnome_canvas_line_get_type (),
					       "points", c_gauge->points,
					       "fill_color_rgba", color,
					       "width_pixels", 3,
					       NULL);
	
	return TRUE;
}
