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
#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <string.h>



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
 \brief sets the gauge font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \param index (TextIndex) index into internall array to correspond scale 
 \param scale (gdfloat) font scale as a percentage of radius
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
 \brief returns the current font scale as a percentage of radius
 \param gauge (MtxGaugeFace *) pointer to gauge
 \params index (TextIndex) index to internal array
 \returns (gfloat) font scale as a percentage of radius
 */
gfloat mtx_gauge_face_get_font_scale (MtxGaugeFace *gauge, TextIndex index)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	g_return_val_if_fail (index < NUM_TEXTS, -1);
	return gauge->font_scale[index];
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
	if (value > gauge->ubound)
		gauge->value = gauge->ubound;
	else if (value < gauge->lbound)
		gauge->value = gauge->lbound;
	else
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
/*DEPRECATED!!! 
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
	g_array_append_val(gauge->c_ranges,range);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
}
*/


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
/* DEPRECATED!!
 *
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
*/


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
	g_array_append_val(gauge->c_ranges,newrange);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return gauge->c_ranges->len-1;
}


/*!
 \brief adds a new text block  from the struct passed
 \param gauge, MtxGaugeFace * pointer to gauge
 \param tblock, MtxTextBlock * pointer to text block struct to copy
 \returns index of where this text block is stored...
 */
gint mtx_gauge_face_set_text_block_struct(MtxGaugeFace *gauge, MtxTextBlock *tblock)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	MtxTextBlock * new_tblock = g_new0(MtxTextBlock, 1);
	new_tblock->font = g_strdup(tblock->font);
	new_tblock->text = g_strdup(tblock->text);
	new_tblock->color = tblock->color;
	new_tblock->font_scale = tblock->font_scale;
	new_tblock->x_pos = tblock->x_pos;
	new_tblock->y_pos = tblock->y_pos;
	g_array_append_val(gauge->t_blocks,new_tblock);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return gauge->t_blocks->len-1;
}


/*!
 \brief adds a new tick group from the struct passed
 \param gauge, MtxGaugeFace * pointer to gauge
 \param tblock, MtxTickGroup * pointer to tick group struct to copy
 \returns index of where this text block is stored...
 */
gint mtx_gauge_face_set_tick_group_struct(MtxGaugeFace *gauge, MtxTickGroup *tgroup)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	MtxTickGroup * new_tgroup = g_new0(MtxTickGroup, 1);
	new_tgroup->text = g_strdup(tgroup->text);
	new_tgroup->text_color = tgroup->text_color;
	new_tgroup->text_inset = tgroup->text_inset;
	new_tgroup->font = g_strdup(tgroup->font);
	new_tgroup->font_scale = tgroup->font_scale;
	new_tgroup->num_maj_ticks = tgroup->num_maj_ticks;
	new_tgroup->maj_tick_color = tgroup->maj_tick_color;
	new_tgroup->maj_tick_inset = tgroup->maj_tick_inset;
	new_tgroup->maj_tick_length = tgroup->maj_tick_length;
	new_tgroup->maj_tick_width = tgroup->maj_tick_width;
	new_tgroup->num_min_ticks = tgroup->num_min_ticks;
	new_tgroup->min_tick_color = tgroup->min_tick_color;
	new_tgroup->min_tick_inset = tgroup->min_tick_inset;
	new_tgroup->min_tick_length = tgroup->min_tick_length;
	new_tgroup->min_tick_width = tgroup->min_tick_width;
	new_tgroup->start_angle = tgroup->start_angle;
	new_tgroup->stop_angle = tgroup->stop_angle;
	g_array_append_val(gauge->tick_groups,new_tgroup);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return gauge->tick_groups->len-1;
}



/*!
 \brief adds a polygon from the struct passed
 \param gauge, MtxGaugeFace * pointer to gauge
 \param tblock, MtxPolygon * pointer to polygon struct to copy
 \returns index of where this text block is stored...
 */
gint mtx_gauge_face_set_polygon_struct(MtxGaugeFace *gauge, MtxPolygon *poly)
{
	gint size = 0;
	MtxGenPoly * new = NULL;
	MtxGenPoly * temp = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	MtxPolygon * new_poly = g_new0(MtxPolygon, 1);
	new_poly->type = poly->type;
	new_poly->color = poly->color;
	new_poly->filled = poly->filled;
	/* Get size of struct to copy */
	switch(poly->type)
	{
		case MTX_CIRCLE:
			size = sizeof(MtxCircle);
			break;
		case MTX_ARC:
			size = sizeof(MtxArc);
			break;
		case MTX_RECTANGLE:
			size = sizeof(MtxRectangle);
			break;
		case MTX_GENPOLY:
			size = sizeof(MtxGenPoly);
			break;
		default:
			break;
	}
	new_poly->data = g_memdup(poly->data,size);
	/* Generic polygons have a dynmically allocated part,  copy it */
	if (poly->type == MTX_GENPOLY)
	{
		new = (MtxGenPoly *)new_poly->data;
		temp = (MtxGenPoly *)poly->data;
		new->points = g_memdup(temp->points,sizeof(MtxPoint)*temp->num_points);
		//printf("copied over %i MTxPoints (%i bytes) \n",temp->num_points,sizeof(MtxPoint)*temp->num_points);
	}
	g_array_append_val(gauge->polygons,new_poly);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return gauge->polygons->len-1;
}


/*!
 \brief returns the array of ranges to the requestor
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \returns GArray * of the ranges,  DO NOT FREE THIS.
 */
GArray * mtx_gauge_face_get_color_ranges(MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return gauge->c_ranges;
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
 \brief returns the array of tickgroups to the requestor
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \returns GArray * of the tickgroups,  DO NOT FREE THIS.
 */
GArray * mtx_gauge_face_get_tick_groups(MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return gauge->tick_groups;
}


/*!
 \brief returns the array of polygons to the requestor
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \returns GArray * of the polygons,  DO NOT FREE THIS.
 */
GArray * mtx_gauge_face_get_polygons(MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return gauge->polygons;
}


/*!
 \brief changes a text block field by passing in an index 
 to the test block and the field name to change
 \param gauge  pointer to gauge object
 \param index, index of the textblock
 \param field,  enumeration of the field to change
 \param value, new value
 */
void mtx_gauge_face_alter_text_block(MtxGaugeFace *gauge, gint index,TbField field, void * value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (field < TB_NUM_FIELDS);
	g_return_if_fail (field >= 0);
	g_object_freeze_notify (G_OBJECT (gauge));

	MtxTextBlock *tblock = NULL;
	tblock = g_array_index(gauge->t_blocks,MtxTextBlock *,index);
	g_return_if_fail (tblock != NULL);
	switch (field)
	{
		case TB_FONT_SCALE:
			tblock->font_scale = *(gfloat *)value;
			break;
		case TB_X_POS:
			tblock->x_pos = *(gfloat *)value;
			break;
		case TB_Y_POS:
			tblock->y_pos = *(gfloat *)value;
			break;
		case TB_COLOR:
			tblock->color = *(GdkColor *)value;
			break;
		case TB_FONT:
			g_free(tblock->font);
			tblock->font = g_strdup(value);
			break;
		case TB_TEXT:
			g_free(tblock->text);
			tblock->text = g_strdup(value);
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}


/*!
 \brief changes a tick group field by passing in an index 
 to the tick group and the field name to change
 \param gauge  pointer to gauge object
 \param index, index of the tickgroup
 \param field,  enumeration of the field to change
 \param value, new value
 */
void mtx_gauge_face_alter_tick_group(MtxGaugeFace *gauge, gint index,TgField field, void * value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (field < TG_NUM_FIELDS);
	g_return_if_fail (field >= 0);
	g_object_freeze_notify (G_OBJECT (gauge));

	MtxTickGroup *tgroup = NULL;
	tgroup = g_array_index(gauge->tick_groups,MtxTickGroup *,index);
	g_return_if_fail (tgroup != NULL);
	switch (field)
	{
		case TG_FONT:
			g_free(tgroup->font);
			tgroup->font = g_strdup(value);
			break;
		case TG_TEXT:
			g_free(tgroup->text);
			tgroup->text = g_strdup(value);
			break;
		case TG_TEXT_COLOR:
			tgroup->text_color = *(GdkColor *)value;
			break;
		case TG_TEXT_INSET:
			tgroup->text_inset = *(gfloat *)value;
			break;
		case TG_FONT_SCALE:
			tgroup->font_scale = *(gfloat *)value;
			break;
		case TG_NUM_MAJ_TICKS:
			tgroup->num_maj_ticks = (gint)(*(gfloat *)value);
			break;
		case TG_MAJ_TICK_COLOR:
			tgroup->maj_tick_color = *(GdkColor *)value;
			break;
		case TG_MAJ_TICK_INSET:
			tgroup->maj_tick_inset = *(gfloat *)value;
			break;
		case TG_MAJ_TICK_WIDTH:
			tgroup->maj_tick_width = *(gfloat *)value;
			break;
		case TG_MAJ_TICK_LENGTH:
			tgroup->maj_tick_length = *(gfloat *)value;
			break;
		case TG_NUM_MIN_TICKS:
			tgroup->num_min_ticks = (gint)(*(gfloat *)value);
			break;
		case TG_MIN_TICK_COLOR:
			tgroup->min_tick_color = *(GdkColor *)value;
			break;
		case TG_MIN_TICK_INSET:
			tgroup->min_tick_inset = *(gfloat *)value;
			break;
		case TG_MIN_TICK_WIDTH:
			tgroup->min_tick_width = *(gfloat *)value;
			break;
		case TG_MIN_TICK_LENGTH:
			tgroup->min_tick_length = *(gfloat *)value;
			break;
		case TG_START_ANGLE:
			tgroup->start_angle = *(gfloat *)value;
			break;
		case TG_STOP_ANGLE:
			tgroup->stop_angle = *(gfloat *)value;
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}



/*!
 \brief changes a polygon field by passing in an index 
 to the polygon and the field name to change
 \param gauge  pointer to gauge object
 \param index, index of the tickgroup
 \param field,  enumeration of the field to change
 \param value, new value
 */
void mtx_gauge_face_alter_polygon(MtxGaugeFace *gauge, gint index,PolyField field, void * value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (field < POLY_NUM_FIELDS);
	g_return_if_fail (field >= 0);
	g_object_freeze_notify (G_OBJECT (gauge));

	MtxPolygon *poly = NULL;
	void *data = NULL;
	gint i = 0;
	gint num_points = 0;

	poly = g_array_index(gauge->polygons,MtxPolygon *,index);
	g_return_if_fail (poly != NULL);
	data = poly->data;
	switch (field)
	{
		case POLY_COLOR:
			poly->color = *(GdkColor *)value;
			break;
		case POLY_FILLED:
			poly->filled = (gint)(*(gfloat *)value);
			break;
		case POLY_LINESTYLE:
			poly->line_style = (gint)(*(gfloat *)value);
			break;
		case POLY_JOINSTYLE:
			poly->join_style = (gint)(*(gfloat *)value);
			break;
		case POLY_LINEWIDTH:
			poly->line_width = *(gfloat *)value;
			break;
		case POLY_X:
			if (poly->type == MTX_CIRCLE)
				((MtxCircle *)data)->x = *(gfloat *)value;
			if (poly->type == MTX_ARC)
				((MtxArc *)data)->x = *(gfloat *)value;
			if (poly->type == MTX_RECTANGLE)
				((MtxRectangle *)data)->x = *(gfloat *)value;
			break;
		case POLY_Y:
			if (poly->type == MTX_CIRCLE)
				((MtxCircle *)data)->y = *(gfloat *)value;
			if (poly->type == MTX_ARC)
				((MtxArc *)data)->y = *(gfloat *)value;
			if (poly->type == MTX_RECTANGLE)
				((MtxRectangle *)data)->y = *(gfloat *)value;
			break;
		case POLY_WIDTH:
			if (poly->type == MTX_ARC)
				((MtxArc *)data)->width = *(gfloat *)value;
			if (poly->type == MTX_RECTANGLE)
				((MtxRectangle *)data)->width = *(gfloat *)value;
			break;
		case POLY_HEIGHT:
			if (poly->type == MTX_ARC)
				((MtxArc *)data)->height = *(gfloat *)value;
			if (poly->type == MTX_RECTANGLE)
				((MtxRectangle *)data)->height = *(gfloat *)value;
			break;
		case POLY_RADIUS:
			if (poly->type == MTX_CIRCLE)
				((MtxCircle *)data)->radius = *(gfloat *)value;
			break;
		case POLY_START_ANGLE:
			if (poly->type == MTX_ARC)
				((MtxArc *)data)->start_angle = *(gfloat *)value;
			break;
		case POLY_SWEEP_ANGLE:
			if (poly->type == MTX_ARC)
				((MtxArc *)data)->sweep_angle = *(gfloat *)value;
			break;
		case POLY_NUM_POINTS:
			if (poly->type == MTX_GENPOLY)
			{
				num_points = ((MtxGenPoly *)data)->num_points;
				((MtxGenPoly *)data)->num_points = (gint)(*(gfloat *)value);
				((MtxGenPoly *)data)->points = g_renew(MtxPoint,((MtxGenPoly *)data)->points,((MtxGenPoly *)data)->num_points);
				for (i=num_points;i<((MtxGenPoly *)data)->num_points;i++)
				{
					((MtxGenPoly *)data)->points[i].x = 0.0;
					((MtxGenPoly *)data)->points[i].y = 0.0;
				}
			}
			break;
		case POLY_POINTS:
			if (poly->type == MTX_GENPOLY)
			{
				if (((MtxGenPoly *)data)->points)
					g_free(((MtxGenPoly *)data)->points);
				((MtxGenPoly *)data)->points = g_memdup(value,((MtxGenPoly *)data)->num_points *sizeof(MtxPoint));
			}
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}


/*!
 \brief changes a color range field by passing in an index 
 to the test block and the field name to change
 \param gauge  pointer to gauge object
 \param index, index of the textblock
 \param field,  enumeration of the field to change
 \param value, new value
 */
void mtx_gauge_face_alter_color_range(MtxGaugeFace *gauge, gint index,CrField field, void * value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_return_if_fail (field < CR_NUM_FIELDS);
	g_return_if_fail (field >= 0);
	g_object_freeze_notify (G_OBJECT (gauge));

	MtxColorRange *c_range = NULL;
	c_range = g_array_index(gauge->c_ranges,MtxColorRange *,index);
	g_return_if_fail (c_range != NULL);
	switch (field)
	{
		case CR_LOWPOINT:
			c_range->lowpoint = *(gfloat *)value;
			break;
		case CR_HIGHPOINT:
			c_range->highpoint = *(gfloat *)value;
			break;
		case CR_INSET:
			c_range->inset = *(gfloat *)value;
			break;
		case CR_LWIDTH:
			c_range->lwidth = *(gfloat *)value;
			break;
		case CR_COLOR:
			c_range->color = *(GdkColor *)value;
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}


/*!
 \brief clears all color ranges from the gauge
 \param gauge (MtxGaugeFace *), pointer to gauge object
 */
void mtx_gauge_face_remove_all_color_ranges(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxColorRange *c_range = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=gauge->c_ranges->len-1;i>=0;i--)
	{
		c_range = g_array_index(gauge->c_ranges,MtxColorRange *, i);
		gauge->c_ranges = g_array_remove_index(gauge->c_ranges,i);
		g_free(c_range);
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
 \brief clears all tick groups from the gauge
 \param gauge (MtxGaugeFace *), pointer to gauge object
 */
void mtx_gauge_face_remove_all_tick_groups(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxTextBlock *tgroup = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=gauge->tick_groups->len-1;i>=0;i--)
	{
		tgroup = g_array_index(gauge->tick_groups,MtxTextBlock *, i);
		gauge->tick_groups = g_array_remove_index(gauge->tick_groups,i);
		g_free(tgroup->font);
		g_free(tgroup->text);
		g_free(tgroup);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}


/*!
 \brief clears all polygons from the gauge
 \param gauge (MtxGaugeFace *), pointer to gauge object
 */
void mtx_gauge_face_remove_all_polygons(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxPolygon *poly = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=gauge->polygons->len-1;i>=0;i--)
	{
		poly = g_array_index(gauge->polygons,MtxPolygon *, i);
		gauge->polygons = g_array_remove_index(gauge->polygons,i);
		if (poly->type == MTX_GENPOLY)
		{
			if (((MtxGenPoly *)poly->data)->points)
				g_free(((MtxGenPoly *)poly->data)->points);
		}
		g_free(poly->data);
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
	MtxColorRange *c_range = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if ((index <= gauge->c_ranges->len) && (index >= 0 ))
	{
		c_range = g_array_index(gauge->c_ranges,MtxColorRange *, index);
		g_array_remove_index(gauge->c_ranges,index);
		if (c_range)
			g_free(c_range);
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
		tblock = g_array_index(gauge->t_blocks,MtxTextBlock *, index);
		g_array_remove_index(gauge->t_blocks,index);
		if (tblock)
		{
			g_free(tblock->font);
			g_free(tblock->text);
			g_free(tblock);
		}
	}
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return;

}


/*!
 \brief clears a specific tick_group based on index passed
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \param index gint index of the one we want to remove.
 */
void mtx_gauge_face_remove_tick_group(MtxGaugeFace *gauge, gint index)
{
	MtxTextBlock *tgroup = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if ((index <= gauge->tick_groups->len) && (index >= 0 ))
	{
		tgroup = g_array_index(gauge->tick_groups,MtxTextBlock *, index);
		g_array_remove_index(gauge->tick_groups,index);
		if (tgroup)
		{
			g_free(tgroup->font);
			g_free(tgroup->text);
			g_free(tgroup);
		}
	}
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	return;

}


/*!
 \brief clears a specific polygon based on index passed
 \param gauge (MtxGaugeFace *), pointer to gauge object
 \param index gint index of the one we want to remove.
 */
void mtx_gauge_face_remove_polygon(MtxGaugeFace *gauge, gint index)
{
	MtxPolygon *poly = NULL;
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if ((index <= gauge->polygons->len) && (index >= 0 ))
	{
		poly = g_array_index(gauge->polygons,MtxPolygon *, index);
		g_array_remove_index(gauge->polygons,index);
		if (poly)
		{
			if (poly->type == MTX_GENPOLY)
			{
				if (((MtxGenPoly *)poly->data)->points)
					g_free(((MtxGenPoly *)poly->data)->points);
			}
			g_free(poly);
		}
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


/*!
 \brief enables showing the drag border boxes 
 \param gauge, pointer to gauge
 \param state, state to set it to
 */
void mtx_gauge_face_set_show_drag_border(MtxGaugeFace *gauge, gboolean state)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->show_drag_border = state;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);
	mtx_gauge_face_configure(GTK_WIDGET(gauge),NULL);
	gdk_window_clear_area_e(GTK_WIDGET(gauge)->window,0,0,gauge->w, gauge->h);
}


/*!
 \brief gets the state of the drag border
 \param gauge, pointer to gauge
 \returns the state of whether the gauge shows the drag border
 */
gboolean mtx_gauge_face_get_show_drag_border(MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), FALSE);
	return gauge->show_drag_border;
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


