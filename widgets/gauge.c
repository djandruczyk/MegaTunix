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
 */
void mtx_gauge_face_set_color_range(MtxGaugeFace *gauge, gfloat lower, gfloat upper, GdkColor color)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));

	g_object_freeze_notify (G_OBJECT (gauge));
	MtxColorRange * range = g_new0(MtxColorRange, 1);
	range->lowpoint = lower;
	range->highpoint = upper;
	range->color = color;
	g_array_append_val(gauge->ranges,range);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
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
 \brief Initializesthe mtx gauge face class and links in the primary
 signal handlers for config event, expose event, and button press/release
 \param class_name (MtxGaugeFaceClass *) pointer to the class
 */
void mtx_gauge_face_class_init (MtxGaugeFaceClass *class_name)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class_name);
	widget_class = GTK_WIDGET_CLASS (class_name);

	/* GtkWidget signals */
	widget_class->configure_event = mtx_gauge_face_configure;
	widget_class->expose_event = mtx_gauge_face_expose;
	widget_class->button_press_event = mtx_gauge_face_button_press;
	widget_class->button_release_event = mtx_gauge_face_button_release;

	g_type_class_add_private (obj_class, sizeof (MtxGaugeFacePrivate));
}


/*!
 \brief Initializes the gauge attributes to sane defaults
 \param gauge (MtxGaugeFace *) pointer to the gauge object
 */
void mtx_gauge_face_init (MtxGaugeFace *gauge)
{
	//which events widget receives
	gtk_widget_add_events (GTK_WIDGET (gauge),GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK);

	gauge->w = 0;
	gauge->h = 0;
	gauge->xc = 0.0;
	gauge->yc = 0.0;
	gauge->radius = 0.0;
	gauge->value = 0.0;//default values
	gauge->lbound = 0.0;
	gauge->ubound = 100.0;
	gauge->precision = 2;
	gauge->start_radian = 0.75 * M_PI;//M_PI is left, 0 is right
	gauge->stop_radian = 2.25 * M_PI;
	gauge->major_ticks = 9;
	gauge->minor_ticks = 3;  /* B S S S B S S S B  tick style */
	gauge->tick_inset = 0.15;    /* how much in from gauge radius fortick */
	gauge->major_tick_len = 0.1; /* 1 = 100% of radius, so 0.1 = 10% */
	gauge->minor_tick_len = 0.05;/* 1 = 100% of radius, so 0.1 = 10% */
	gauge->needle_width = 0.05;  /* % of radius */
	gauge->needle_tail = 0.083;  /* % of radius */
	gauge->name_font = g_strdup("Bitstream Vera Sans");
	gauge->name_str = g_strdup("No name");
	gauge->name_font_scale = 0.15;
	gauge->units_font = g_strdup("Bitstream Vera Sans");
	gauge->units_str = g_strdup("No units");
	gauge->units_font_scale = 0.1;
	gauge->value_font = g_strdup("Bitstream Vera Sans");
	gauge->value_str = g_strdup("000");
	gauge->value_font_scale = 0.2;
	gauge->span = gauge->ubound - gauge->lbound;
#ifdef HAVE_CAIRO
	gauge->cr = NULL;
	gauge->antialias = TRUE;
#else
	gauge->antialias = FALSE;
#endif
	gauge->start_deg = -((gauge->start_radian/M_PI) *180.0);
	gauge->stop_deg = -(((gauge->stop_radian-gauge->start_radian)/M_PI) *180.0);
	gauge->colormap = gdk_colormap_get_system();
	gauge->gc = NULL;
	gauge->ranges = g_array_new(FALSE,TRUE,sizeof(MtxColorRange *));
	mtx_gauge_face_init_colors(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief updates the gauge position,  This is a wrapper function conditionally
 compiled to call a corresponsing GDK or cairo function.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void mtx_gauge_face_init_colors(MtxGaugeFace *gauge)
{
	/* Defaults for the gauges,  user over-ridable */

	/*! Background */
	gauge->colors[COL_BG].red=0*65535;
	gauge->colors[COL_BG].green=0*65535;
	gauge->colors[COL_BG].blue=0*65535;
	/*! Needle */
	gauge->colors[COL_NEEDLE].red=1.0*65535;
	gauge->colors[COL_NEEDLE].green=1.0*65535;
	gauge->colors[COL_NEEDLE].blue=1.0*65535;
	/*! Major Ticks */
	gauge->colors[COL_MAJ_TICK].red=0.8*65535;
	gauge->colors[COL_MAJ_TICK].green=0.8*65535;
	gauge->colors[COL_MAJ_TICK].blue=0.8*65535;
	/*! Minor Ticks */
	gauge->colors[COL_MIN_TICK].red=0.8*65535;
	gauge->colors[COL_MIN_TICK].green=0.8*65535;
	gauge->colors[COL_MIN_TICK].blue=0.8*65535;
	/*! Units Font*/
	gauge->colors[COL_UNIT_FONT].red=0.8*65535;
	gauge->colors[COL_UNIT_FONT].green=0.8*65535;
	gauge->colors[COL_UNIT_FONT].blue=0*65535;
	/*! Name Font */
	gauge->colors[COL_NAME_FONT].red=0*65535;
	gauge->colors[COL_NAME_FONT].green=0.8*65535;
	gauge->colors[COL_NAME_FONT].blue=0.8*65535;
	/*! Value Font */
	gauge->colors[COL_VALUE_FONT].red=0.8*65535;
	gauge->colors[COL_VALUE_FONT].green=0*65535;
	gauge->colors[COL_VALUE_FONT].blue=0.8*65535;

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
 \brief updates the gauge position,  This is the CAIRO implementation that
 looks a bit nicer, though is a little bit slower
 \param widget (GtkWidget *) pointer to the gauge object
 */
void cairo_update_gauge_position (GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	gfloat radian_span = 0.0;
	gfloat tmpf = 0.0;
	gfloat needle_pos = 0.0;
	gchar * message = NULL;
	gint i = 0;
	gfloat n_width = 0.0;
	gfloat n_tail = 0.0;
	gfloat n_tip = 0.0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	cairo_t *cr = NULL;
	cairo_text_extents_t extents;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;

	/* Copy bacground pixmap to intermediaary for final rendering */
	gdk_draw_drawable(gauge->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);


	cr = gdk_cairo_create (gauge->pixmap);

	/* Update the VALUE text */
	cairo_set_source_rgb (cr, gauge->colors[COL_VALUE_FONT].red/65535.0,
				gauge->colors[COL_VALUE_FONT].green/65535.0,
				gauge->colors[COL_VALUE_FONT].blue/65535.0);
	cairo_select_font_face (cr, gauge->value_font, CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_NORMAL);

	cairo_set_font_size (cr, (gauge->radius * gauge->value_font_scale));

	message = g_strdup_printf("%.*f", gauge->precision,gauge->value);

	cairo_text_extents (cr, message, &extents);

	cairo_move_to (cr, gauge->xc-(extents.width/2), gauge->yc+(0.55 * gauge->radius));
	cairo_show_text (cr, message);
	g_free(message);

	cairo_stroke (cr);

	/* gauge hands */
	radian_span = (gauge->stop_radian - gauge->start_radian);
	tmpf = (gauge->value-gauge->lbound)/(gauge->ubound-gauge->lbound);
	needle_pos = gauge->start_radian+(tmpf*radian_span)+M_PI/2.0;

	cairo_set_source_rgb (cr, gauge->colors[COL_NEEDLE].red/65535.0,
				gauge->colors[COL_NEEDLE].green/65535.0,
				gauge->colors[COL_NEEDLE].blue/65535.0);
	cairo_set_line_width (cr, 1);

	n_width = gauge->needle_width * gauge->radius;
	n_tail = gauge->needle_tail * gauge->radius;
	n_tip = gauge->radius * 0.850;
	xc = gauge->xc;
	yc = gauge->yc;

	/* STORE needle coordinates to make the expese event a LOT more 
	 * efficient */
	for (i=0;i<4;i++)
	{
		gauge->last_needle_coords[i].x = gauge->needle_coords[i].x;
		gauge->last_needle_coords[i].y = gauge->needle_coords[i].y;
	}
	gauge->needle_coords[0].x = xc + (n_tip) * sin (needle_pos);
	gauge->needle_coords[0].y = yc + (n_tip) * -cos (needle_pos);

	gauge->needle_coords[1].x = xc + (n_width) *cos(needle_pos);
	gauge->needle_coords[1].y = yc + (n_width) *sin(needle_pos);

	gauge->needle_coords[2].x = xc + (n_tail) * -sin (needle_pos);
	gauge->needle_coords[2].y = yc + (n_tail) * cos (needle_pos);
	gauge->needle_coords[3].x = xc - (n_width) * cos (needle_pos);
	gauge->needle_coords[3].y = yc - (n_width) * sin (needle_pos);
	gauge->needle_polygon_points = 4;

	cairo_move_to (cr, gauge->needle_coords[0].x,gauge->needle_coords[0].y);
	cairo_line_to (cr, gauge->needle_coords[1].x,gauge->needle_coords[1].y);
	cairo_line_to (cr, gauge->needle_coords[2].x,gauge->needle_coords[2].y);
	cairo_line_to (cr, gauge->needle_coords[3].x,gauge->needle_coords[3].y);
	cairo_fill_preserve (cr);
	cairo_destroy(cr);
#endif
}


/*!
 \brief updates the gauge position,  This is the GDK implementation that
 looks doesn't do antialiasing,  but is the fastest one.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void gdk_update_gauge_position (GtkWidget *widget)
{
	gint i= 0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	gfloat tmpf = 0.0;
	gfloat radian_span = 0.0;
	gfloat needle_pos = 0.0;
	gint n_width = 0;
	gint n_tail = 0;
	gint n_tip = 0;
	gchar * message = NULL;
	gchar * tmpbuf = NULL;
	PangoRectangle logical_rect;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;

	/* Copy bacground pixmap to intermediaary for final rendering */
	gdk_draw_drawable(gauge->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);


	/* the text */
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_VALUE_FONT]);
	message = g_strdup_printf("%.*f", gauge->precision,gauge->value);

	tmpbuf = g_strdup_printf("%s %i",gauge->value_font,(gint)(gauge->radius *gauge->value_font_scale));
	gauge->font_desc = pango_font_description_from_string(tmpbuf);
	g_free(tmpbuf);
	pango_layout_set_font_description(gauge->layout,gauge->font_desc);
	pango_layout_set_text(gauge->layout,message,-1);
	pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);
	g_free(message);


	gdk_draw_layout(gauge->pixmap,gauge->gc,
			gauge->xc-(logical_rect.width/2),
			gauge->yc+(0.55 * gauge->radius)-logical_rect.height,gauge->layout);

	//	lwidth = MIN (w/2, h/2)/100 < 1 ? 1: MIN (w/2, h/2)/100;
	gdk_gc_set_line_attributes(gauge->gc,1,
			GDK_LINE_SOLID,
			GDK_CAP_ROUND,
			GDK_JOIN_ROUND);

	/* gauge hands */
	radian_span = (gauge->stop_radian - gauge->start_radian);
	tmpf = (gauge->value-gauge->lbound)/(gauge->ubound-gauge->lbound);
	needle_pos = gauge->start_radian+(tmpf*radian_span)+M_PI/2.0;
	//	printf("start_rad %f, stop_rad %f\n",gauge->start_radian, gauge->stop_radian);
	//	printf("tmpf %f, radian span %f, position, %f\n",tmpf,radian_span,needle_pos);

	xc= gauge->xc;
	yc= gauge->yc;
	n_width = gauge->needle_width * gauge->radius;
	n_tail = gauge->needle_tail * gauge->radius;
	n_tip = gauge->radius * 0.850;

	/* Four POINT needle,  point 0 is the tip (easiest to find) */
	for (i=0;i<4;i++)
	{
		gauge->last_needle_coords[i].x = gauge->needle_coords[i].x;
		gauge->last_needle_coords[i].y = gauge->needle_coords[i].y;
	}
	gauge->needle_coords[0].x = xc + (n_tip) * sin (needle_pos);
	gauge->needle_coords[0].y = yc + (n_tip) * -cos (needle_pos);

	gauge->needle_coords[1].x = xc + (n_width) *cos(needle_pos);
	gauge->needle_coords[1].y = yc + (n_width) *sin(needle_pos);

	gauge->needle_coords[2].x = xc + (n_tail) * -sin (needle_pos);
	gauge->needle_coords[2].y = yc + (n_tail) * cos (needle_pos);
	gauge->needle_coords[3].x = xc - (n_width) * cos (needle_pos);
	gauge->needle_coords[3].y = yc - (n_width) * sin (needle_pos);
	gauge->needle_polygon_points = 4;

	/* Draw the needle */
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_NEEDLE]);
	gdk_draw_polygon(gauge->pixmap,
			widget->style->white_gc,
			TRUE,gauge->needle_coords,4);

}


/*!
 \brief handles configure events whe nthe gauge gets created or resized.
 Takes care of creating/destroying graphics contexts, backing pixmaps (two 
 levels are used to split the rendering for speed reasons) colormaps are 
 also created here as well
 \param widget (GtkWidget *) pointer to the gauge object
 \param event (GdkEventConfigure *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_gauge_face_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	MtxGaugeFace * gauge = MTX_GAUGE_FACE(widget);

	if(widget->window)
	{
		gauge->w = widget->allocation.width;
		gauge->h = widget->allocation.height;

		if (gauge->gc)
			g_object_unref(gauge->gc);
		if (gauge->layout)
			g_object_unref(gauge->layout);
		/* Backing pixmap (copy of window) */
		if (gauge->pixmap)
			g_object_unref(gauge->pixmap);
		gauge->pixmap=gdk_pixmap_new(widget->window,
				gauge->w,gauge->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(gauge->pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				gauge->w,gauge->h);
		/* Static Background pixmap */
		if (gauge->bg_pixmap)
			g_object_unref(gauge->bg_pixmap);
		gauge->bg_pixmap=gdk_pixmap_new(widget->window,
				gauge->w,gauge->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(gauge->bg_pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				gauge->w,gauge->h);

		gdk_window_set_back_pixmap(widget->window,gauge->pixmap,0);
		gauge->layout = gtk_widget_create_pango_layout(GTK_WIDGET(&gauge->parent),NULL);	
		gauge->gc = gdk_gc_new(gauge->bg_pixmap);
		gdk_gc_set_colormap(gauge->gc,gauge->colormap);

		gauge->xc = gauge->w / 2;
		gauge->yc = gauge->h / 2;
		gauge->radius = MIN (gauge->w/2, gauge->h/2) - 5;
	}
	generate_gauge_background(widget);
	update_gauge_position(widget);

	return TRUE;
}


/*!
 \brief handles exposure events when the screen is covered and then 
 exposed. Works by copying from a backing pixmap to screen,
 \param widget (GtkWidget *) pointer to the gauge object
 \param event (GdkEventExpose *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_gauge_face_expose (GtkWidget *widget, GdkEventExpose *event)
{
	MtxGaugeFace * gauge = MTX_GAUGE_FACE(widget);

	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return FALSE;
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
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the cairo version.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void cairo_generate_gauge_background(GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	cairo_t *cr = NULL;
	gfloat arc = 0.0;
	gfloat radians_per_major_tick = 0.0;
	gfloat radians_per_minor_tick = 0.0;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint j = 0;
	gfloat counter = 0;
	gfloat subcounter = 0;
	gint inset = 0;
	gint insetfrom = 0;
	gfloat lwidth = 0.0;
	gfloat angle1, angle2;
	cairo_pattern_t *gradient = NULL;
	cairo_text_extents_t extents;
	MtxColorRange *range = NULL;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;

	w = widget->allocation.width;
	h = widget->allocation.height;
	/* get a cairo_t */
	cr = gdk_cairo_create (gauge->bg_pixmap);
	/* Background set to black */
	cairo_rectangle (cr,
			0,0,
			widget->allocation.width, widget->allocation.height);
	cairo_set_source_rgb (cr, 0,0,0);

	cairo_fill(cr);
	if (gauge->antialias)
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);

	/* Create that cool gradient that gives the shadow illusion */
	gradient = cairo_pattern_create_linear(0,gauge->w,gauge->w,0);
	cairo_pattern_add_color_stop_rgb(gradient, 0, 1, 1, 1);
	cairo_pattern_add_color_stop_rgb(gradient, gauge->radius, 0, 0, 0);

	/* Filled Arcs */
	cairo_arc(cr, gauge->xc, gauge->yc, (0.985 * gauge->radius), 0, 2*M_PI);
	cairo_set_source_rgb(cr, 0.53, 0.53, 0.53);
	cairo_fill(cr);

	cairo_arc(cr, gauge->xc, gauge->yc, (0.985 * gauge->radius), 0, 2 * M_PI);
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
	cairo_stroke(cr);

	cairo_arc(cr, gauge->xc, gauge->yc, (0.934 * gauge->radius), 0, 2 * M_PI);
	cairo_set_source(cr, gradient);
	cairo_fill(cr);

	cairo_arc(cr, gauge->xc, gauge->yc, (0.880 * gauge->radius), 0, 2 * M_PI);
	cairo_set_source_rgb (cr, gauge->colors[COL_BG].red/65535.0,
				gauge->colors[COL_BG].green/65535.0,
				gauge->colors[COL_BG].blue/65535.0);
	cairo_fill(cr);

	/* The warning color ranges */
	arc = (gauge->stop_radian - gauge->start_radian) / (2 * M_PI);
	for (i=0;i<gauge->ranges->len;i++)
	{
		range = g_array_index(gauge->ranges,MtxColorRange *, i);
		cairo_set_source_rgb(cr,range->color.red/65535.0,
				range->color.green/65535.0,
				range->color.blue/65535.0);
		/* percent of full scale is (lbound-range_lbound)/(fullspan)*/
		angle1 = (range->lowpoint-gauge->lbound)/(gauge->ubound-gauge->lbound);
		angle2= (range->highpoint-gauge->lbound)/(gauge->ubound-gauge->lbound);
		//printf("gauge color range should be from %f, to %f of full scale\n",angle1, angle2);
		lwidth = MIN (w/2, h/2)/20 < 1 ? 1: MIN (w/2, h/2)/20;
		cairo_set_line_width (cr, lwidth);
		cairo_arc(cr, gauge->xc, gauge->yc, (0.775 * gauge->radius),gauge->start_radian+(angle1*(gauge->stop_radian-gauge->start_radian)), gauge->start_radian+(angle2*(gauge->stop_radian-gauge->start_radian)));

		cairo_stroke(cr);

	}

	/* gauge ticks */
	cairo_set_source_rgb (cr, gauge->colors[COL_MAJ_TICK].red/65535.0,
				gauge->colors[COL_MAJ_TICK].green/65535.0,
				gauge->colors[COL_MAJ_TICK].blue/65535.0);
	radians_per_major_tick = (gauge->stop_radian - gauge->start_radian)/(float)(gauge->major_ticks-1);
	radians_per_minor_tick = radians_per_major_tick/(float)(1+gauge->minor_ticks);
	/* Major ticks first */
	insetfrom = gauge->radius * gauge->tick_inset;

	counter = gauge->start_radian;
	for (i=0;i<gauge->major_ticks;i++)
	{
		inset = (gint) (gauge->major_tick_len * gauge->radius);
		lwidth = MIN (w/2, h/2)/80 < 1 ? 1: MIN (w/2, h/2)/80;
		cairo_set_line_width (cr, lwidth);
		cairo_move_to (cr,
				gauge->xc + (gauge->radius - insetfrom) * cos (counter),
				gauge->yc + (gauge->radius - insetfrom) * sin (counter));
		cairo_line_to (cr,
				gauge->xc + (gauge->radius - insetfrom - inset) * cos (counter),
				gauge->yc + (gauge->radius - insetfrom - inset) * sin (counter));
		cairo_stroke (cr);
		/* minor ticks */
		if ((gauge->minor_ticks > 0) && (i < (gauge->major_ticks-1)))
		{
			cairo_save (cr); /* stack-pen-size */
			cairo_set_source_rgb (cr, 
					gauge->colors[COL_MIN_TICK].red/65535.0,
					gauge->colors[COL_MIN_TICK].green/65535.0,
					gauge->colors[COL_MIN_TICK].blue/65535.0);
				inset = (gint) (gauge->minor_tick_len * gauge->radius);
			lwidth = MIN (w/2, h/2)/160 < 1 ? 1: MIN (w/2, h/2)/160;
			cairo_set_line_width (cr, lwidth);
			for (j=1;j<=gauge->minor_ticks;j++)
			{
				subcounter = j*radians_per_minor_tick;
				cairo_move_to (cr,
						gauge->xc + (gauge->radius - insetfrom) * cos (counter+subcounter),
						gauge->yc + (gauge->radius - insetfrom) * sin (counter+subcounter));
				cairo_line_to (cr,
						gauge->xc + (gauge->radius - insetfrom - inset) * cos (counter+subcounter),
						gauge->yc + (gauge->radius - insetfrom - inset) * sin (counter+subcounter));
				cairo_stroke (cr);
			}
			cairo_restore (cr); /* stack-pen-size */
		}
		counter += radians_per_major_tick;
	}

	/* The units string */
	if (gauge->units_str)
	{
		cairo_set_source_rgb (cr, 
				gauge->colors[COL_UNIT_FONT].red/65535.0,
				gauge->colors[COL_UNIT_FONT].green/65535.0,
				gauge->colors[COL_UNIT_FONT].blue/65535.0);
		cairo_select_font_face (cr, gauge->units_font, CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);

		cairo_set_font_size (cr, (gauge->radius * gauge->units_font_scale));
		cairo_text_extents (cr, gauge->units_str, &extents);
		cairo_move_to (cr, gauge->xc-(extents.width/2), gauge->yc+(0.75 * gauge->radius));
		cairo_show_text (cr, gauge->units_str);
	}

	/* The name string */
	if (gauge->name_str)
	{
		cairo_set_source_rgb (cr, 
				gauge->colors[COL_NAME_FONT].red/65535.0,
				gauge->colors[COL_NAME_FONT].green/65535.0,
				gauge->colors[COL_NAME_FONT].blue/65535.0);
		cairo_select_font_face (cr, gauge->name_font, CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);

		cairo_set_font_size (cr, (gauge->radius * gauge->name_font_scale));
		cairo_text_extents (cr, gauge->name_str, &extents);
		cairo_move_to (cr, gauge->xc-(extents.width/2), gauge->yc-(0.35 * gauge->radius));
		cairo_show_text (cr, gauge->name_str);
	}

	cairo_destroy (cr);
#endif
}


/*!
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the gdk version.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void gdk_generate_gauge_background(GtkWidget *widget)
{
	gfloat radians_per_major_tick = 0.0;
	gfloat radians_per_minor_tick = 0.0;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint j = 0;
	gint lwidth = 0;
	gfloat arc = 0.0;
	gint inset = 0;
	gint insetfrom = 0;
	gfloat counter = 0.0;
	gfloat subcounter = 0.0;
	gchar * tmpbuf = NULL;
	gfloat angle1 = 0.0;
	gfloat angle2 = 0.0;
	gfloat start_pos = 0.0;
	gfloat stop_pos = 0.0;
	gfloat start_angle = 0.0;
	gfloat span = 0.0;
	MtxColorRange *range = NULL;
	GdkColor color;
	PangoRectangle logical_rect;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;
	w = widget->allocation.width;
	h = widget->allocation.height;

	/* Wipe the display, black */
	gdk_draw_rectangle(gauge->bg_pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			w,h);

	/* The main grey (will have a thin overlay on top of it) 
	 * This is a FILLED circle */

	color.red=(gint)(0.53 * 65535);
	color.green=(gint)(0.53 * 65535);
	color.blue=(gint)(0.53 * 65535);
	gdk_gc_set_rgb_fg_color(gauge->gc,&color);
	/* Create the border stuff */
	gdk_draw_arc(gauge->bg_pixmap,gauge->gc,TRUE,
			gauge->xc-gauge->radius*0.985,
			gauge->yc-gauge->radius*0.985,
			2*(gauge->radius*0.985),
			2*(gauge->radius*0.985),
			0,360*64);

	/* Outermost thin "ring" NOTE!! not filled */
	lwidth = MIN (w/2, h/2)/120 < 1 ? 1: MIN (w/2, h/2)/120;
	gdk_gc_set_line_attributes(gauge->gc,lwidth,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);
	color.red=(gint)(0.8 * 65535);
	color.green=(gint)(0.8 * 65535);
	color.blue=(gint)(0.8 * 65535);
	gdk_gc_set_rgb_fg_color(gauge->gc,&color);

	gdk_draw_arc(gauge->bg_pixmap,gauge->gc,FALSE,
			gauge->xc-gauge->radius*0.985,
			gauge->yc-gauge->radius*0.985,
			2*(gauge->radius*0.985),
			2*(gauge->radius*0.985),
			0,360*64);

	lwidth = MIN (gauge->xc,gauge->yc)/20 < 1 ? 1: MIN (gauge->xc,gauge->yc)/20;
	gdk_gc_set_line_attributes(gauge->gc,lwidth,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);


	/* Funky hack to get a pretty gradient sorta like the cairo version*/
	for(i=0;i<36;i++)
	{
		color.red=(gint)(((i+6)/48.0) * 65535);
		color.green=(gint)(((i+6)/48.0) * 65535);
		color.blue=(gint)(((i+6)/48.0) * 65535);
		gdk_gc_set_rgb_fg_color(gauge->gc,&color);

		gdk_draw_arc(gauge->bg_pixmap,gauge->gc,FALSE,
				gauge->xc-gauge->radius*0.903,
				gauge->yc-gauge->radius*0.903,
				2*(gauge->radius*0.903),
				2*(gauge->radius*0.903),
				(45+(i*5))*64,5*64);
	}
	for(i=0;i<36;i++)
	{
		color.red=(gint)(((42-i)/48.0) * 65535);
		color.green=(gint)(((42-i)/48.0) * 65535);
		color.blue=(gint)(((42-i)/48.0) * 65535);
		gdk_gc_set_rgb_fg_color(gauge->gc,&color);

		gdk_draw_arc(gauge->bg_pixmap,gauge->gc,FALSE,
				gauge->xc-gauge->radius*0.903,
				gauge->yc-gauge->radius*0.903,
				2*(gauge->radius*0.903),
				2*(gauge->radius*0.903),
				(225+(i*5))*64,5*64);
	}



	/* Create the INNER filled black arc to draw the ticks and everything
	 * else onto
	 */
	gdk_gc_set_line_attributes(gauge->gc,1,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_BG]);

	gdk_draw_arc(gauge->bg_pixmap,gauge->gc,TRUE,
			gauge->xc-gauge->radius*0.880,
			gauge->yc-gauge->radius*0.880,
			2*(gauge->radius*0.880),
			2*(gauge->radius*0.880),
			0,360*64);

	/* The warning color ranges */
	arc = (gauge->stop_radian - gauge->start_radian) / (2 * M_PI);
	for (i=0;i<gauge->ranges->len;i++)
	{
		range = g_array_index(gauge->ranges,MtxColorRange *, i);
		gdk_gc_set_rgb_fg_color(gauge->gc,&range->color);
		/* percent of full scale is (lbound-range_lbound)/(fullspan)*/
		span = gauge->stop_radian - gauge->start_radian;
		angle1 = (range->lowpoint-gauge->lbound)/(gauge->span);
		angle2 = (range->highpoint-gauge->lbound)/(gauge->span);

		/* positions of the range in radians */
		start_pos = gauge->start_radian+(angle1*span);
		stop_pos = gauge->start_radian+(angle2*span);
		/* Converted to funky GDK units */
		start_angle = -start_pos*(180/M_PI)*64;
		span = - (stop_pos-start_pos)*(180/M_PI)*64;

		lwidth = MIN (w/2, h/2)/20 < 1 ? 1: MIN (w/2, h/2)/20;
		gdk_gc_set_line_attributes(gauge->gc,lwidth,
				GDK_LINE_SOLID,
				GDK_CAP_BUTT,
				GDK_JOIN_BEVEL);
		gdk_draw_arc(gauge->bg_pixmap,gauge->gc,FALSE, 
				gauge->xc-gauge->radius*0.775, 
				gauge->yc-gauge->radius*0.775,
				2*(gauge->radius*0.775),
				2*(gauge->radius*0.775),
				start_angle,
				span);
	}


	/* gauge ticks */
	radians_per_major_tick = (gauge->stop_radian - gauge->start_radian)/(float)(gauge->major_ticks-1);
	radians_per_minor_tick = radians_per_major_tick/(float)(1+gauge->minor_ticks);
	/* Major ticks first */
	insetfrom = gauge->radius * gauge->tick_inset;

	counter = gauge->start_radian;
	for (i=0;i<gauge->major_ticks;i++)
	{
		inset = (gint) (gauge->major_tick_len * gauge->radius);
		lwidth = MIN (w/2, h/2)/80 < 1 ? 1: MIN (w/2, h/2)/80;
		gdk_gc_set_line_attributes(gauge->gc,lwidth,
				GDK_LINE_SOLID,
				GDK_CAP_BUTT,
				GDK_JOIN_BEVEL);

		gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_MAJ_TICK]);
		gdk_draw_line(gauge->bg_pixmap,gauge->gc,

				gauge->xc + (gauge->radius - insetfrom) * cos (counter),
				gauge->yc + (gauge->radius - insetfrom) * sin (counter),
				gauge->xc + ((gauge->radius - insetfrom - inset) * cos (counter)),
				gauge->yc + ((gauge->radius - insetfrom - inset) * sin (counter)));
		/* Now the minor ticks... */
		if ((gauge->minor_ticks > 0) && (i < (gauge->major_ticks-1)))
		{
			gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_MIN_TICK]);
			inset = (gint) (gauge->minor_tick_len * gauge->radius);
			lwidth = MIN (w/2, h/2)/160 < 1 ? 1: MIN (w/2, h/2)/160;
			gdk_gc_set_line_attributes(gauge->gc,lwidth,
					GDK_LINE_SOLID,
					GDK_CAP_BUTT,
					GDK_JOIN_BEVEL);
			for (j=1;j<=gauge->minor_ticks;j++)
			{
				subcounter = j*radians_per_minor_tick;
				gdk_draw_line(gauge->bg_pixmap,gauge->gc,

						gauge->xc + (gauge->radius - insetfrom) * cos (counter+subcounter),
						gauge->yc + (gauge->radius - insetfrom) * sin (counter+subcounter),
						gauge->xc + ((gauge->radius - insetfrom - inset) * cos (counter+subcounter)),
						gauge->yc + ((gauge->radius - insetfrom - inset) * sin (counter+subcounter)));

			}

		}
		counter += radians_per_major_tick;

	}

	/* Units String */
	if (gauge->units_str)
	{
		gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_UNIT_FONT]);
		tmpbuf = g_strdup_printf("%s %i",gauge->units_font,(gint)(gauge->radius*gauge->units_font_scale));
		gauge->font_desc = pango_font_description_from_string(tmpbuf);
		g_free(tmpbuf);
		pango_layout_set_font_description(gauge->layout,gauge->font_desc);
		pango_layout_set_text(gauge->layout,gauge->units_str,-1);
		pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);

		gdk_draw_layout(gauge->bg_pixmap,gauge->gc,
				gauge->xc-(logical_rect.width/2),
				gauge->yc+(0.75 * gauge->radius)-logical_rect.height,gauge->layout);
	}

	/* Name String */
	if (gauge->name_str)
	{
		gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_NAME_FONT]);
		tmpbuf = g_strdup_printf("%s %i",gauge->name_font,(gint)(gauge->radius*gauge->name_font_scale));
		gauge->font_desc = pango_font_description_from_string(tmpbuf);
		g_free(tmpbuf);
		pango_layout_set_font_description(gauge->layout,gauge->font_desc);
		pango_layout_set_text(gauge->layout,gauge->name_str,-1);
		pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);


		gdk_draw_layout(gauge->bg_pixmap,gauge->gc,
				gauge->xc-(logical_rect.width/2),
				gauge->yc-(0.35 * gauge->radius)-logical_rect.height,gauge->layout);
	}


}

/*!
 \brief handler called when a button is pressed,  currently unused 
 \param gauge (GtkWidget *) pointer to the gauge widget
 \param event (GdkEventButton *) struct containing details on the button(s) 
 pressed
 \returns FALSE
 */
gboolean mtx_gauge_face_button_press (GtkWidget *gauge,
					     GdkEventButton *event)
{
	MtxGaugeFacePrivate *priv;
	priv = MTX_GAUGE_FACE_GET_PRIVATE (gauge);

	return FALSE;
}


/*!
 \brief gets called to redraw the entire display manually
 \param gauge (MtxGaugeFace *) pointer to the gauge object
 */
void mtx_gauge_face_redraw_canvas (MtxGaugeFace *gauge)
{
	GtkWidget *widget;

	widget = GTK_WIDGET (gauge);

	if (!widget->window) return;

	update_gauge_position(widget);
	gdk_window_clear(widget->window);
}


/*!
 \brief handler called when a button is released,  currently unused 
 \param gauge (GtkWidget *) pointer to the gauge widget
 \param event (GdkEventButton *) struct containing details on the button(s) 
 released
 \returns FALSE
 */
gboolean mtx_gauge_face_button_release (GtkWidget *gauge,
					       GdkEventButton *event)
{
	MtxGaugeFacePrivate *priv;
	priv = MTX_GAUGE_FACE_GET_PRIVATE (gauge);
	return FALSE;
}


/*!
 \brief gets called when  a user wants a new gauge
 \returns a pointer to a newly created gauge widget
 */
GtkWidget *mtx_gauge_face_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_GAUGE_FACE, NULL));
}
