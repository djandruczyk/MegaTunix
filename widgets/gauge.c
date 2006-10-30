/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Christopher Mire, 2006
 *
 * Megasquirt gauge widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 *  
 *
 * -------------------------------------------------------------------------
 *  Hacked and slashed to hell by David J. Andruczyk in order to bend and
 *  tweak it to my needs for MegaTunix.  Added in rendering ability using
 *  cairo and raw GDK callls for those less fortunate (OS-X)
 *  Added a HUGE number of functions to get/set every gauge attribute
 *
 *
 *  Was offered a fine contribution by Ari Karhu 
 *  "ari <at> ultimatevw <dot> com" from the msefi.com forums.
 *  His contribution made the gauges look, ohh so much nicer than I could 
 *  have come up with!
 */


#include <config.h>
#ifdef HAVE_CAIRO
#include <cairo/cairo.h>
#endif
#include <gauge.h>
#include <gauge-private.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <string.h>



/*!
 \brief sets the units string for the gauge and kicks off a full redraw
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param units_str (gchar *) new units text string to use
 */
void mtx_gauge_face_set_units_str(MtxGaugeFace *gauge, gchar * units_str)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (gauge->units_str)
		g_free(gauge->units_str);
	gauge->units_str = g_strdup(units_str);;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the color for the index passed.  The index to use used is an opaque enum
 inside of gauge.h
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param index (gint) index of color to set.
 \param color (GdkColor) new color to use for the specified index
 */
void mtx_gauge_face_set_color (MtxGaugeFace *gauge, gint index, GdkColor color)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->colors[index].red = color.red;
	gauge->colors[index].green = color.green;
	gauge->colors[index].blue = color.blue;
	gauge->colors[index].pixel = color.pixel;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief gets the color for the index passed.  The index to use used is an opaque enum
 inside of gauge.h
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param index (gint) index of color to set.
 \returns a pointer to the internal GdkColor struct
 */
GdkColor* mtx_gauge_face_get_color (MtxGaugeFace *gauge, gint index)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return (&gauge->colors[index]);
}


/*!
 \brief sets the name string for the gauge and kicks off a full redraw
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param name_str (gchar *) new name text string to use
 */
void mtx_gauge_face_set_name_str(MtxGaugeFace *gauge, gchar * name_str)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (gauge->name_str)
		g_free(gauge->name_str);
	gauge->name_str = g_strdup(name_str);;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the current value 
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param value (gfloat) new value
 */
void mtx_gauge_face_set_value (MtxGaugeFace *gauge, gfloat value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->value = value;
	g_object_thaw_notify (G_OBJECT (gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}

/*!
 \brief sets the current display precision 
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param precision (gint) new precision (# of decimal places to show)
 */
void mtx_gauge_face_set_precision (MtxGaugeFace *gauge, gint precision)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->precision = precision;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}

/*!
 \brief sets antialias mode
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param state (gboolean) new precision (# of decimal places to show)
 */
void mtx_gauge_face_set_antialias(MtxGaugeFace *gauge, gboolean state)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	gauge->antialias = state;
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief returns the current gauge value
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) the gauge value
 */
gfloat mtx_gauge_face_get_value (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->value;
}


/*!
 \brief returns the current precision
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) the precision
 */
gint mtx_gauge_face_get_precision (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->precision;
}


/*!
 \brief returns the antialias flag
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) the antialias flag
 */
gboolean mtx_gauge_face_get_antialias (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), FALSE);
	return gauge->antialias;
}


/*!
 \brief returns the units string
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) the units string
 */
gchar * mtx_gauge_face_get_units_string (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	return gauge->units_str;
}


/*!
 \brief returns the name string
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) the name string
 */
gchar * mtx_gauge_face_get_name_string (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	return gauge->name_str;
}


/*!
 \brief adds a new color range between the limits specified
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param lower (gfloat) lower limit for hte range in real world units 
 corresponding to the gauge span
 \param upper (gfloat) upper limit for hte range in real world units 
 corresponding to the gauge span
 \param color (GdkColor) a color struct passed defining the color of the range
 \param lwidth (gfloat) percentage of radius that determines the width of the range line
 \param inset (gfloat) percentage of radius that determines the inset from the edge
 */
void mtx_gauge_face_set_color_range(MtxGaugeFace *gauge, gfloat lower, gfloat upper, GdkColor color, gfloat lwidth, gfloat inset)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));

	g_object_freeze_notify (G_OBJECT (gauge));
	MtxColorRange * range = g_new0(MtxColorRange, 1);
	range->lowpoint = lower;
	range->highpoint = upper;
	range->color = color;
	range->lwidth = lwidth;
	range->inset = inset;
	g_array_append_val(gauge->ranges,range);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief adds a new color range between the limits specified in the struct passed
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param range (MtxColorRange*) pointer to color range struct to copy
 \returns index of where this range is stored...
 */
gint mtx_gauge_face_set_color_range_struct(MtxGaugeFace *gauge, MtxColorRange *range)
{
	GArray * array;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	MtxColorRange * newrange = g_new0(MtxColorRange, 1);
	newrange = g_memdup(range,sizeof(MtxColorRange)); 
	array = g_array_append_val(gauge->ranges,newrange);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return array->len-1;
}


/*!
 \brief returns the array of ranges to the requestor
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \returns GArray * or the ranges,  DO NOT FREE THIS.
 */
GArray * mtx_gauge_face_get_color_ranges(MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return gauge->ranges;
}


/*!
 \brief clears all color ranges from the gauge
 \param gauge (MtxGaugeFace *), pointer to gauge object
 */
void mtx_gauge_face_remove_all_color_ranges(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxColorRange *range = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=0;i<gauge->ranges->len;i++)
	{
		range = g_array_index(gauge->ranges,MtxColorRange *, i);
		gauge->ranges = g_array_remove_index(gauge->ranges,i);
		g_free(range);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}



/*!
 \brief clears a specific color range based on index passed
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \param index gint index of the one we want to remove.
 */
void mtx_gauge_face_remove_color_range(MtxGaugeFace *gauge, gint index)
{
	MtxColorRange *range = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if ((index <= gauge->ranges->len) && (index >= 0 ))
	{
		range = g_array_index(gauge->ranges,MtxColorRange *, index);
		gauge->ranges = g_array_remove_index(gauge->ranges,index);
		g_free(range);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return;

}

/*!
 \brief sets the guage bounds (real worl units)
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param value1 (gfloat) lower bound
 \param value1 (gfloat) upper bound
 */
void mtx_gauge_face_set_bounds (MtxGaugeFace *gauge, gfloat value1, gfloat value2)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->lbound = value1;
	gauge->ubound = value2;
	gauge->span = gauge->ubound -gauge->lbound;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the lower bound only of the gauge
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param value (gfloat) lower end of span
 */
void mtx_gauge_face_set_lbound (MtxGaugeFace *gauge, gfloat value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	mtx_gauge_face_set_bounds(gauge,value, gauge->ubound);
}


/*!
 \brief sets the upper bound only of the gauge
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param value (gfloat) upper end of span
 */
void mtx_gauge_face_set_ubound (MtxGaugeFace *gauge, gfloat value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	mtx_gauge_face_set_bounds(gauge,gauge->lbound,value);
}


/*!
 \brief retreives the bound by passing in pointers to two gfloats
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param value1 (gfloat) pointer to a gfloat to store the lower value into
 \param value2 (gfloat) pointer to a gfloat to store the upper value into
 \returns TRUE always
 */
gboolean mtx_gauge_face_get_bounds(MtxGaugeFace *gauge, gfloat *value1, gfloat *value2)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	*value1 = gauge->lbound;
	*value2 = gauge->ubound;
	return TRUE;
}


/*!
 \brief retreives the angular span in radians by passing in pointers to two gfloats
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param value1 (gfloat) pointer to a gfloat to store the lower point angle
 \param value2 (gfloat) pointer to a gfloat to store the upper point angle
 \returns TRUE always
 */
gboolean mtx_gauge_face_get_span_rad(MtxGaugeFace *gauge, gfloat *value1, gfloat *value2)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	*value1 = gauge->start_radian;
	*value2 = gauge->stop_radian;
	return TRUE;
}


/*!
 \brief retreives the angular span in degrees by passing in pointers to two gfloats
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param value1 (gfloat) pointer to a gfloat to store the lower point angle
 \param value2 (gfloat) pointer to a gfloat to store the upper point angle
 \returns TRUE always
 */
gboolean mtx_gauge_face_get_span_deg(MtxGaugeFace *gauge, gfloat *value1, gfloat *value2)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	*value1 = gauge->start_deg;
	*value2 = gauge->stop_deg;
	return TRUE;
}


/*!
 \brief Sets the total number of major ticks for a gauge
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param ticks (gint) number of ticks to use i nthe gauge
 */
void mtx_gauge_face_set_major_ticks (MtxGaugeFace *gauge, int ticks)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->major_ticks = ticks;
	generate_gauge_background(GTK_WIDGET(gauge));
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}

/*!
 \brief Sets the length of the major ticks as a percentage of the radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param len (gfloat) value (0-1.0) multiple by the gauge radius to determine
 the major tick length (fully scalable solution)
 */
void mtx_gauge_face_set_major_tick_len (MtxGaugeFace *gauge, gfloat len)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->major_tick_len = len;
	generate_gauge_background(GTK_WIDGET(gauge));
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}

/*!
 \brief returns the tick inset percentage
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns the value for the tick inset as a percentage of the gauge radius
 */
gfloat mtx_gauge_face_get_tick_inset (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->tick_inset;
}


/*!
 \brief returns the current needle width 
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) the needle width as a percentage of gauge radius
 */
gfloat mtx_gauge_face_get_needle_width (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->needle_width;
}


/*!
 \brief returns the current needle tail length 
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) the needle tail length as a percentage of gauge radius
 */
gfloat mtx_gauge_face_get_needle_tail (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->needle_tail;
}


/*!
 \brief returns the current units font name
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gchar *) units font textual name
 */
gchar * mtx_gauge_face_get_units_font (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	return gauge->units_font;
}


/*!
 \brief returns the current gauge name font name
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gchar *) gauge name font textual name
 */
gchar * mtx_gauge_face_get_name_font (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	return gauge->name_font;
}


/*!
 \brief returns the current value font name
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gchar *) value font textual name
 */
gchar * mtx_gauge_face_get_value_font (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	return gauge->value_font;
}


/*!
 \brief returns the current unit font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) unit font scale as a percentage of radius
 */
gfloat mtx_gauge_face_get_units_font_scale (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->units_font_scale;
}


/*!
 \brief returns the current name font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) name font scale as a percentage of radius
 */
gfloat mtx_gauge_face_get_name_font_scale (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->name_font_scale;
}


/*!
 \brief returns the current value font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) value  font scale as a percentage of radius
 */
gfloat mtx_gauge_face_get_value_font_scale (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->value_font_scale;
}


/*!
 \brief returns the current number of major tickmarks
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gint) number of major tickmarks
 */
int mtx_gauge_face_get_major_ticks (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->major_ticks;
}


/*!
 \brief returns the current major tick length as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) major tick length as a percentage of radius
 */
gfloat mtx_gauge_face_get_major_tick_len (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->major_tick_len;
}


/*!
 \brief sets the number of minor ticks.  This number is the number of baby 
 ticks IN BETWEEN any two major ticks,  NOT the TOTAL number of minor ticks
 for the entire gauge.
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param ticks (int) number of minor ticks between any two major ticks.
 */
void mtx_gauge_face_set_minor_ticks (MtxGaugeFace *gauge, int ticks)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->minor_ticks = ticks;
	generate_gauge_background(GTK_WIDGET(gauge));
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief returns the current number of minor tickmarks
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gint) number of minor tickmarks
 */
int mtx_gauge_face_get_minor_ticks (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->minor_ticks;
}

/*!
 \brief returns the current minor tick length as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) minor tick length as a percentage of radius
 */
gfloat mtx_gauge_face_get_minor_tick_len (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->minor_tick_len;
}


/*!
 \brief sets the minor tick length as a percentage of the gauge radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param len (gfloat) length of minor ticks as a percentage of gauge radius
 */
void mtx_gauge_face_set_minor_tick_len (MtxGaugeFace *gauge, gfloat len)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->minor_tick_len = len;
	generate_gauge_background(GTK_WIDGET(gauge));
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the tick inset as a percentage of the gauge radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param inset (gfloat) tick inset as a percentage of gauge radius
 */
void mtx_gauge_face_set_tick_inset (MtxGaugeFace *gauge, gfloat inset)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->tick_inset = inset;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the needle width as a percentage of the gauge radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param width (gfloat) needle width as a percentage of gauge radius
 */
void mtx_gauge_face_set_needle_width (MtxGaugeFace *gauge, gfloat width)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->needle_width = width;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the needle tail length as a percentage of the gauge radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param width (gfloat) needle tail length as a percentage of gauge radius
 */
void mtx_gauge_face_set_needle_tail (MtxGaugeFace *gauge, gfloat len)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->needle_tail = len;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the gauge units font
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param name (gchar *) units font name as a textual string
 */
void mtx_gauge_face_set_units_font (MtxGaugeFace *gauge, gchar * name)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (gauge->units_font)
		g_free(gauge->units_font);
	gauge->units_font = g_strdup(name);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the gauge name font
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param name (gchar *) name font name as a textual string
 */
void mtx_gauge_face_set_name_font (MtxGaugeFace *gauge, gchar * name)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (gauge->name_font)
		g_free(gauge->name_font);
	gauge->name_font = g_strdup(name);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the gauge value font
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param name (gchar *) value font name as a textual string
 */
void mtx_gauge_face_set_value_font (MtxGaugeFace *gauge, gchar * name)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (gauge->value_font)
		g_free(gauge->value_font);
	gauge->value_font = g_strdup(name);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the gauge units font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param scale (gdfloat) units font scale as a percentage of radius
 */
void mtx_gauge_face_set_units_font_scale (MtxGaugeFace *gauge, gfloat scale)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->units_font_scale = scale;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the gauge name font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param scale (gdfloat) name font scale as a percentage of radius
 */
void mtx_gauge_face_set_name_font_scale (MtxGaugeFace *gauge, gfloat scale)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->name_font_scale = scale;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the gauge value font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param scale (gdfloat) value font scale as a percentage of radius
 */
void mtx_gauge_face_set_value_font_scale (MtxGaugeFace *gauge, gfloat scale)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->value_font_scale = scale;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief Changes the angular span of the gauge in CAIRO STYLE units (CW 
 inscreaing, units in radians,  0 deg is at 3 o'clock position) Right now all
 gauges are assumed to have clockwise rotation with increasing value
 \param gauge (MtxGaugeFace *) pointer to the gauge
 \param start_radian (gfloat) start angle in radians
 \param stop_radian (gfloat) stop angle in radians
 */
void mtx_gauge_face_set_span_rad (MtxGaugeFace *gauge, gfloat start_radian, gfloat stop_radian)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->start_radian = start_radian;
	gauge->stop_radian = stop_radian;
	/* For some un know ndamn reason,  GDK draws it's arcs
	 * counterclockwisein units of 1/64th fo a degree, whereas cairo
	 * doe it clockwise in radians. At least they start at the same place
	 * "3 O'clock" 
	 * */
#ifndef HAVE_CAIRO
	gauge->start_deg = -((start_radian/M_PI) *180.0);
	gauge->stop_deg = -(((stop_radian-start_radian)/M_PI) *180.0);
#endif
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief Changes the angular span of the gauge in CAIRO STYLE units (CW 
 inscreaing, units in radians,  0 deg is at 3 o'clock position) Right now all
 gauges are assumed to have clockwise rotation with increasing value
 \param gauge (MtxGaugeFace *) pointer to the gauge
 \param start_radian (gfloat) start angle in radians
 */
void mtx_gauge_face_set_lspan_rad (MtxGaugeFace *gauge, gfloat start_radian)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	mtx_gauge_face_set_span_rad (gauge, start_radian, gauge->stop_radian);
}


/*!
 \brief Changes the angular span of the gauge in CAIRO STYLE units (CW 
 inscreaing, units in radians,  0 deg is at 3 o'clock position) Right now all
 gauges are assumed to have clockwise rotation with increasing value
 \param gauge (MtxGaugeFace *) pointer to the gauge
 \param stop_radian (gfloat) stop angle in radians
 */
void mtx_gauge_face_set_uspan_rad (MtxGaugeFace *gauge, gfloat stop_radian)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	mtx_gauge_face_set_span_rad (gauge, gauge->start_radian, stop_radian);
}

/*!
 \brief Changes the angular span of the gauge in GDK STYLE units (CCW 
 inscreaing, units in degrees,  0 deg is at 3 o'clock position) Right now all
 gauges are assumed to have clockwise rotation with increasing value
 \param gauge (MtxGaugeFace *) pointer to the gauge
 \param start_deg (gfloat) start angle in degrees
 \param stop_deg (gfloat) stop angle in degrees
 */
void mtx_gauge_face_set_span_deg (MtxGaugeFace *gauge, gfloat start_deg, gfloat stop_deg)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->start_deg = start_deg;
	gauge->stop_deg = stop_deg;
	/* For some un know ndamn reason,  GDK draws it's arcs
	 * counterclockwisein units of 1/64th fo a degree, whereas cairo
	 * doe it clockwise in radians. At least they start at the same place
	 * "3 O'clock" 
	 * */
#ifdef HAVE_CAIRO
	gauge->start_radian = -start_deg*M_PI/180.0;
	gauge->stop_radian = -stop_deg*M_PI/180.0;
#endif
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief Changes the angular span of the gauge in GDK STYLE units (CCW 
 inscreaing, units in degrees,  0 deg is at 3 o'clock position) Right now all
 gauges are assumed to have clockwise rotation with increasing value
 \param gauge (MtxGaugeFace *) pointer to the gauge
 \param start_deg (gfloat) start angle in degrees
 */
void mtx_gauge_face_set_lspan_deg (MtxGaugeFace *gauge, gfloat start_deg)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	mtx_gauge_face_set_span_deg (gauge, start_deg, gauge->stop_deg);
}


/*!
 \brief Changes the angular span of the gauge in GDK STYLE units (CCW 
 inscreaing, units in degrees,  0 deg is at 3 o'clock position) Right now all
 gauges are assumed to have clockwise rotation with increasing value
 \param gauge (MtxGaugeFace *) pointer to the gauge
 \param stop_deg (gfloat) stop angle in degrees
 */
void mtx_gauge_face_set_uspan_deg (MtxGaugeFace *gauge, gfloat stop_deg)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	mtx_gauge_face_set_span_deg (gauge, gauge->start_deg, stop_deg);
}


/*!
 \brief updates the gauge position,  This is a wrapper function conditionally
 compiled to call a corresponsing GDK or cairo function.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void update_gauge_position(GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	cairo_update_gauge_position (widget);
#else
	gdk_update_gauge_position (widget);
#endif
}



/*!
 \brief generates the gauge background, This is a wrapper function 
 conditionally compiled to call a corresponsing GDK or cairo function.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void generate_gauge_background(GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	cairo_generate_gauge_background(widget);
#else
	gdk_generate_gauge_background(widget);
#endif
}


/*!
 \brief gets called when  a user wants a new gauge
 \returns a pointer to a newly created gauge widget
 */
GtkWidget *mtx_gauge_face_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_GAUGE_FACE, NULL));
}
