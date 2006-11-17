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
 \brief sets the text string for the passed in index. Index is an enumeration
 \see TextIndex
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param TextIndex  enumeration to know where to store the new string
 \param string (gchar *) string to store
 */
void mtx_gauge_face_set_text(MtxGaugeFace *gauge, TextIndex index, gchar * str)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (index < NUM_TEXTS);
	g_object_freeze_notify (G_OBJECT (gauge));
	if (gauge->txt_str[index])
		g_free(gauge->txt_str[index]);
	gauge->txt_str[index] = g_strdup(str);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief gets the text string for the passed in index. Index is an enumeration
 \see TextIndex
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param TextIndex  enumeration to know where to store the new string
 \returns a copy of the string,  free it when done.
 */
gchar * mtx_gauge_face_get_text (MtxGaugeFace *gauge,TextIndex index)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	g_return_val_if_fail (index < NUM_TEXTS,NULL);
	return g_strdup(gauge->txt_str[index]);
}


/*!
 \brief sets the font text string for the passed in index. 
 Index is an enumeration
 \see TextIndex
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param TextIndex  enumeration to know where to store the new string
 \param string (gchar *) string to store
 */
void mtx_gauge_face_set_font (MtxGaugeFace *gauge, TextIndex index, gchar * str)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (index < NUM_TEXTS);
	g_object_freeze_notify (G_OBJECT (gauge));
	if (gauge->font_str[index])
		g_free(gauge->font_str[index]);
	gauge->font_str[index] = g_strdup(str);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief returns the current font for the passed index
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param index an enuermation to determine which one to send back
 \returns a copy of the font requested, free it when done
 */
gchar * mtx_gauge_face_get_font (MtxGaugeFace *gauge, TextIndex index)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	g_return_val_if_fail (index < NUM_TEXTS, NULL);
	g_return_val_if_fail (gauge->font_str[index], NULL);
	return g_strdup(gauge->font_str[index]);
}


/*!
 \brief sets the gauge major tick font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param scale (gdfloat) major tick font scale as a percentage of radius
 */
void mtx_gauge_face_set_font_scale (MtxGaugeFace *gauge, TextIndex index, gfloat scale)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (index < NUM_TEXTS);
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->font_scale[index] = scale;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief returns the current major tick font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) major tick font scale as a percentage of radius
 */
gfloat mtx_gauge_face_get_font_scale (MtxGaugeFace *gauge, TextIndex index)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	g_return_val_if_fail (index < NUM_TEXTS, -1);
	return gauge->font_scale[index];
}


/*!
 \brief sets the major tick tet inset as a percentage of the gauge radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param inset (gfloat) major tick text inset as a percentage of gauge radius
 */
void mtx_gauge_face_set_major_tick_text_inset (MtxGaugeFace *gauge, gfloat inset)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->major_tick_text_inset = inset;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief returns the major tick text inset percentage
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns the value for the major tick text inset as a percentage of the 
 gauge radius
 */
gfloat mtx_gauge_face_get_major_tick_text_inset (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->major_tick_text_inset;
}


/*!
 \brief sets the color for the index passed.  The index to use used is an opaque enum
 inside of gauge.h
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param index (gint) index of color to set.
 \param color (GdkColor) new color to use for the specified index
 */
void mtx_gauge_face_set_color (MtxGaugeFace *gauge, ColorIndex index, GdkColor color)
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
 \returns a pointer to the internal GdkColor struct WHICH MUST NOT be FREED!!
 */
GdkColor* mtx_gauge_face_get_color (MtxGaugeFace *gauge, ColorIndex index)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return (&gauge->colors[index]);
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
 \param state (gboolean) antialias state
 */
void mtx_gauge_face_set_antialias(MtxGaugeFace *gauge, gboolean state)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->antialias = state;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets/clears showing of value
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param state (gboolean) TRUE, show value, FALSE,  don't show it
 */
void mtx_gauge_face_set_show_value(MtxGaugeFace *gauge, gboolean state)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->show_value = state;
	g_object_thaw_notify (G_OBJECT (gauge));
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
 \returns (gboolean) the antialias flag
 */
gboolean mtx_gauge_face_get_antialias (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), FALSE);
	return gauge->antialias;
}


/*!
 \brief returns the show_value flag
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gboolean) the show_value flag
 */
gboolean mtx_gauge_face_get_show_value (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), FALSE);
	return gauge->show_value;
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
 \brief adds a new text block to the gauge
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param font, gchar, font to use
 \param text, gchar, text to display
 \param color, GdkColor,  color to use for text
 \param font_scale, gfloat,  font scale in % of radius to use
 \param x_pos, gfloat,  position in relation to center, 0 == center -1 = left
 border, +1 = right border.  refernces the CENTER of the textblock
 \param y_pos, gfloat,  position in relation to center, 0 == center -1 = top
 border, +1 = bottom.  refernces the CENTER of the textblock
 */
void mtx_gauge_face_set_text_block(MtxGaugeFace *gauge, gchar *font, gchar *text, gfloat font_scale, GdkColor color, gfloat x_pos, gfloat y_pos)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	MtxTextBlock * tblock = g_new0(MtxTextBlock, 1);
	tblock->font = g_strdup(font);
	tblock->text = g_strdup(text);
	tblock->color = color;
	tblock->font_scale = font_scale;
	tblock->x_pos = x_pos;
	tblock->y_pos = y_pos;
	g_array_append_val(gauge->t_blocks,tblock);
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
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	MtxColorRange * newrange = g_new0(MtxColorRange, 1);
	newrange = g_memdup(range,sizeof(MtxColorRange)); 
	g_array_append_val(gauge->ranges,newrange);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return gauge->ranges->len-1;
}


/*!
 \brief adds a new text block  from the struct passed
 \param gauge, MtxGaugeFace * pointer to gauge
 \param tblock, MtxTextBlock * pointer to color range struct to copy
 \returns index of where this range is stored...
 */
gint mtx_gauge_face_set_text_block_struct(MtxGaugeFace *gauge, MtxTextBlock *tblock)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	MtxTextBlock * new_tblock = g_new0(MtxTextBlock, 1);
	new_tblock = g_memdup(new_tblock,sizeof(MtxTextBlock)); 
	g_array_append_val(gauge->t_blocks,new_tblock);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return gauge->t_blocks->len-1;
}


/*!
 \brief returns the array of ranges to the requestor
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \returns GArray * of the ranges,  DO NOT FREE THIS.
 */
GArray * mtx_gauge_face_get_color_ranges(MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return gauge->ranges;
}


/*!
 \brief returns the array of textblocks to the requestor
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \returns GArray * of the textblocks,  DO NOT FREE THIS.
 */
GArray * mtx_gauge_face_get_text_blocks(MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return gauge->t_blocks;
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
	for (i=gauge->ranges->len-1;i>=0;i--)
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
 \brief clears all text blocks from the gauge
 \param gauge (MtxGaugeFace *), pointer to gauge object
 */
void mtx_gauge_face_remove_all_text_blocks(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxTextBlock *tblock = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=gauge->t_blocks->len-1;i>=0;i--)
	{
		tblock = g_array_index(gauge->t_blocks,MtxTextBlock *, i);
		gauge->t_blocks = g_array_remove_index(gauge->t_blocks,i);
		g_free(tblock->font);
		g_free(tblock->text);
		g_free(tblock);
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
 \brief clears a specific text_block based on index passed
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \param index gint index of the one we want to remove.
 */
void mtx_gauge_face_remove_text_block(MtxGaugeFace *gauge, gint index)
{
	MtxTextBlock *tblock = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if ((index <= gauge->t_blocks->len) && (index >= 0 ))
	{
		tblock = g_array_index(gauge->ranges,MtxTextBlock *, index);
		gauge->t_blocks = g_array_remove_index(gauge->t_blocks,index);
		g_free(tblock->font);
		g_free(tblock->text);
		g_free(tblock);
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
 \brief retreives the x/y position offsets for the string index passed
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param TextIndex enum to determine which one to send back..
 \param value1 (gfloat) pointer to a gfloat to store the x_pos value into
 \param value2 (gfloat) pointer to a gfloat to store the y_pos value into
 \returns TRUE always
 */
gboolean mtx_gauge_face_get_str_pos(MtxGaugeFace *gauge, TextIndex index, gfloat *x_pos, gfloat *y_pos)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_return_val_if_fail (index < NUM_TEXTS,FALSE);
	*x_pos = gauge->text_xpos[index];
	*y_pos = gauge->text_ypos[index];
	return TRUE;
}


/*!
 \brief sets the gauge name_str placement  in offset from center, so values
 of zero make the text centered in the gauge
 \param gauge MtxGaugeFace * pointer to gauge
 \param x_pos,  gfloat x_pos offset from center
 \param y_pos,  gfloat y_pos offset from center
 */
void mtx_gauge_face_set_str_pos (MtxGaugeFace *gauge, TextIndex index, gfloat x_pos, gfloat y_pos)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (index < NUM_TEXTS);
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->text_xpos[index] = x_pos;
	gauge->text_ypos[index] = y_pos;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief sets the name x_position only of the gauge
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param value (gfloat) x value to change
 */
void mtx_gauge_face_set_str_xpos (MtxGaugeFace *gauge, TextIndex index, gfloat value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (index < NUM_TEXTS);
	mtx_gauge_face_set_str_pos(gauge, index, value, gauge->text_ypos[index]);
}


/*!
 \brief sets the text y_position only of the gauge
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param index TextIndex enumeration to know which one we are changing
 \param value (gfloat) y value to change
 */
void mtx_gauge_face_set_str_ypos (MtxGaugeFace *gauge, TextIndex index, gfloat value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (index < NUM_TEXTS);
	mtx_gauge_face_set_str_pos(gauge, index, gauge->text_xpos[index], value);
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
 \brief retreives the angular span in radians by passing in pointers to 
 two gfloats
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
 \brief Sets the width of the major ticks as a percentage of 1/10th the radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param width (gfloat) value (0-1.0) multiple by the 1/10th the gauge radius to 
 determine the major tick width (fully scalable solution)
 */
void mtx_gauge_face_set_major_tick_width (MtxGaugeFace *gauge, gfloat width)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->major_tick_width = width;
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
 \brief returns the current major tick width as a percentage of 1/10th radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) major tick width as a percentage of 1/10th the radius
 */
gfloat mtx_gauge_face_get_major_tick_width (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->major_tick_width;
}


/*!
 \brief returns the current minor tick width as a percentage of 1/10th radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \returns (gfloat) minor tick width as a percentage of 1/10th the radius
 */
gfloat mtx_gauge_face_get_minor_tick_width (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->minor_tick_width;
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
 \brief Sets the width of the minor ticks as a percentage of 1/10th the radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param width (gfloat) value (0-1.0) multiple by the 1/10th the gauge radius to 
 determine the major tick width (fully scalable solution)
 */
void mtx_gauge_face_set_minor_tick_width (MtxGaugeFace *gauge, gfloat width)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->minor_tick_width = width;
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
	gauge->start_deg = -((start_radian/M_PI) *180.0);
	gauge->stop_deg = -(((stop_radian-start_radian)/M_PI) *180.0);
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
	gauge->start_radian = -start_deg*M_PI/180.0;
	gauge->stop_radian = -stop_deg*M_PI/180.0;
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
