/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Christopher Mire, 2006
 *
 * MegaTunix gauge widget
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

/*! @file widgets/gauge.c
 *
 * @brief ...
 *
 *
 */


#include <gauge-private.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

/*!
 \brief sets the color for the index passed.  The index to use used is an opaque enum
 inside of gauge.h
 \param gauge is the pointer to the gauge object
 \param index is the index of color to set.
 \param color is the new color to use for the specified index
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_set_color (MtxGaugeFace *gauge, GaugeColorIndex index, GdkColor color)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));
	priv->colors[index].red = color.red;
	priv->colors[index].green = color.green;
	priv->colors[index].blue = color.blue;
	priv->colors[index].pixel = color.pixel;
	g_object_thaw_notify (G_OBJECT (gauge));

#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}


/*!
 \brief gets the color for the index passed.  The index to use used is an opaque enum
 inside of gauge.h
 \param gauge is the pointer to the gauge object
 \param index is the index of color to set.
 \param color_ref is the reference to a user allocated GdkColor
 \returns GdkColor* color_ref on success, null otherwise
 */
GdkColor *mtx_gauge_face_get_color (MtxGaugeFace *gauge, GaugeColorIndex index, GdkColor *color_ref)
{
	MtxGaugeFacePrivate *const priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);

	if (color_ref)
	{
		memcpy (color_ref, &priv->colors[index], sizeof(GdkColor));
	}

	return ( color_ref );
}


/*!
 \brief gets the current value 
 \param gauge is the pointer to the gauge object
 \param value is the place to store the value of the gauge
 \returns TRUE on succes, FALSE otherwise
 */
gboolean mtx_gauge_face_get_value (MtxGaugeFace *gauge, gfloat *value)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail ((MTX_IS_GAUGE_FACE (gauge)),FALSE);
	g_return_val_if_fail (value,FALSE);
	*value = priv->value;
	return TRUE;
}


/*!
 \brief sets the current value 
 \param gauge is the pointer to the gauge object
 \param value is the new value
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_set_value (MtxGaugeFace *gauge, gfloat value)
{
	gboolean new_bg = FALSE;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);

	/* If no change,  no point updating */
	if (value == priv->value)
		return TRUE;

	g_object_freeze_notify (G_OBJECT (gauge));
	if (value > priv->ubound)
		priv->clamped = CLAMP_UPPER;
	else if (value < priv->lbound)
		priv->clamped = CLAMP_LOWER;
	else
		priv->clamped = CLAMP_NONE;
	if (value > priv->value)
		priv->direction = ASCENDING;
	else
		priv->direction = DESCENDING;
	if ((value > priv->peak) && (priv->show_tattletale))
	{
		priv->show_tattletale = FALSE;
		priv->reenable_tattletale = TRUE;
		new_bg = TRUE;
	}
	priv->value = value;
	if (value > priv->peak)
		priv->peak = value;
	if ((priv->reenable_tattletale) && (priv->direction == DESCENDING))
	{
		priv->reenable_tattletale = FALSE;
		priv->show_tattletale = TRUE;
		new_bg = TRUE;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	if (new_bg)
		generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}


/*!
 \brief gets a newly allocated copy of the value font.  Free when done.
 \param gauge is the pointer to the gauge object
 \returns a string of the value font in use
 */
gchar * mtx_gauge_face_get_value_font (MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail ((MTX_IS_GAUGE_FACE (gauge)),NULL);
	return	g_strdup(priv->value_font);
}


/*!
 \brief sets the current value_font 
 \param gauge is the pointer to the gauge object
 \param new is the new value
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_set_value_font (MtxGaugeFace *gauge, gchar * new)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_return_val_if_fail (new,FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));
	if (priv->value_font)
		g_free(priv->value_font);
	priv->value_font = g_strdup(new);
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}


/*!
 \brief adds a new color range between the limits specified in the struct passed
 \param gauge is the pointer to the gauge object
 \param range is the pointer to the MtxWarningRange structure to copy
 \returns index of where this range is stored.
 */
gint mtx_gauge_face_set_warning_range_struct(MtxGaugeFace *gauge, MtxWarningRange *range)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxWarningRange * newrange = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	newrange = g_new0(MtxWarningRange, 1);
	newrange = g_memdup(range,sizeof(MtxWarningRange)); 
	g_array_append_val(priv->w_ranges,newrange);
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return priv->w_ranges->len-1;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return priv->w_ranges->len-1;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return priv->w_ranges->len-1;
}


/*!
 \brief adds a new alert range between the limits specified in the struct passed
 \param gauge is the pointer to the gauge object
 \param range is the pointer to a MtxAlertRange struct to copy
 \returns index of where this range is stored.
 */
gint mtx_gauge_face_set_alert_range_struct(MtxGaugeFace *gauge, MtxAlertRange *range)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxAlertRange * newrange = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	newrange = g_new0(MtxAlertRange, 1);
	newrange = g_memdup(range,sizeof(MtxAlertRange)); 
	g_array_append_val(priv->a_ranges,newrange);
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return priv->a_ranges->len-1;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return priv->a_ranges->len-1;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return priv->a_ranges->len-1;
}


/*!
 \brief adds a new text block  from the struct passed
 \param gauge is the pointer to the gauge object
 \param tblock is the pointer to a MtxTextBlock struct to copy
 \returns index of where this text block is stored...
 */
gint mtx_gauge_face_set_text_block_struct(MtxGaugeFace *gauge, MtxTextBlock *tblock)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxTextBlock * new_tblock = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	new_tblock = g_new0(MtxTextBlock, 1);
	new_tblock->font = g_strdup(tblock->font);
	new_tblock->text = g_strdup(tblock->text);
	new_tblock->color[MTX_DAY] = tblock->color[MTX_DAY];
	new_tblock->color[MTX_NITE] = tblock->color[MTX_NITE];
	new_tblock->font_scale = tblock->font_scale;
	new_tblock->font_scale = tblock->font_scale;
	new_tblock->x_pos = tblock->x_pos;
	new_tblock->y_pos = tblock->y_pos;
	g_array_append_val(priv->t_blocks,new_tblock);
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return priv->t_blocks->len-1;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return priv->t_blocks->len-1;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return priv->t_blocks->len-1;
}


/*!
 \brief adds a new tick group from the struct passed
 \param gauge is the pointer to the gauge object
 \param tgroup is the pointer to a MtxTickGroup struct to copy
 \returns index of where this tick group is stored...
 */
gint mtx_gauge_face_set_tick_group_struct(MtxGaugeFace *gauge, MtxTickGroup *tgroup)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxTickGroup * new_tgroup = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	new_tgroup = g_new0(MtxTickGroup, 1);
	new_tgroup->text = g_strdup(tgroup->text);
	new_tgroup->text_color[MTX_DAY] = tgroup->text_color[MTX_DAY];
	new_tgroup->text_color[MTX_NITE] = tgroup->text_color[MTX_NITE];
	new_tgroup->text_inset = tgroup->text_inset;
	new_tgroup->font = g_strdup(tgroup->font);
	new_tgroup->font_scale = tgroup->font_scale;
	new_tgroup->num_maj_ticks = tgroup->num_maj_ticks;
	new_tgroup->maj_tick_color[MTX_DAY] = tgroup->maj_tick_color[MTX_DAY];
	new_tgroup->maj_tick_color[MTX_NITE] = tgroup->maj_tick_color[MTX_NITE];
	new_tgroup->maj_tick_inset = tgroup->maj_tick_inset;
	new_tgroup->maj_tick_length = tgroup->maj_tick_length;
	new_tgroup->maj_tick_width = tgroup->maj_tick_width;
	new_tgroup->num_min_ticks = tgroup->num_min_ticks;
	new_tgroup->min_tick_color[MTX_DAY] = tgroup->min_tick_color[MTX_DAY];
	new_tgroup->min_tick_color[MTX_NITE] = tgroup->min_tick_color[MTX_NITE];
	new_tgroup->min_tick_inset = tgroup->min_tick_inset;
	new_tgroup->min_tick_length = tgroup->min_tick_length;
	new_tgroup->min_tick_width = tgroup->min_tick_width;
	new_tgroup->start_angle = tgroup->start_angle;
	new_tgroup->sweep_angle = tgroup->sweep_angle;
	g_array_append_val(priv->tick_groups,new_tgroup);
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return priv->tick_groups->len-1;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return priv->tick_groups->len-1;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return priv->tick_groups->len-1;
}



/*!
 \brief adds a polygon from the struct passed
 \param gauge is the pointer to the gauge object
 \param poly is the pointer to the  MtxPolygon struct to copy
 \returns index of where this polygon is stored...
 */
gint mtx_gauge_face_set_polygon_struct(MtxGaugeFace *gauge, MtxPolygon *poly)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	gint size = 0;
	MtxGenPoly * new = NULL;
	MtxGenPoly * temp = NULL;
	MtxPolygon * new_poly = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),-1);

	g_object_freeze_notify (G_OBJECT (gauge));
	new_poly = g_new0(MtxPolygon, 1);
	new_poly->type = poly->type;
	new_poly->color[MTX_DAY] = poly->color[MTX_DAY];
	new_poly->color[MTX_NITE] = poly->color[MTX_NITE];
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
	/* Generic polygons have a dynmically allocated part, copy it */

	if (poly->type == MTX_GENPOLY)
	{
		new = (MtxGenPoly *)new_poly->data;
		temp = (MtxGenPoly *)poly->data;
		new->points = g_memdup(temp->points,sizeof(MtxPoint)*temp->num_points);
		/*printf("copied over %i MTxPoints (%i bytes) \n",temp->num_points,sizeof(MtxPoint)*temp->num_points);*/

	}
	g_array_append_val(priv->polygons,new_poly);
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return priv->polygons->len-1;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return priv->polygons->len-1;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return priv->polygons->len-1;
}


/*!
 \brief returns the array of color ranges to the requestor
 \param gauge is the pointer to the gauge object
 \returns pointer to the GArray of ranges,  DO NOT FREE THIS.
 */
const GArray * mtx_gauge_face_get_warning_ranges(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return priv->w_ranges;
}


/*!
 \brief returns the array of alert ranges to the requestor
 \param gauge is the pointer to the gauge object
 \returns a pointer to the GArray of ranges,  DO NOT FREE THIS.
 */
const GArray * mtx_gauge_face_get_alert_ranges(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return priv->a_ranges;
}


/*!
 \brief returns the array of textblocks to the requestor
 \param gauge is the pointer to the gauge object
 \returns a pointer to the GArray of textblocks,  DO NOT FREE THIS.
 */
const GArray * mtx_gauge_face_get_text_blocks(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return priv->t_blocks;
}


/*!
 \brief returns the array of tickgroups to the requestor
 \param gauge is the pointer to the gauge object
 \returns a pointer to the GArray of tickgroups,  DO NOT FREE THIS.
 */
const GArray * mtx_gauge_face_get_tick_groups(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return priv->tick_groups;
}


/*!
 \brief returns the array of polygons to the requestor
 \param gauge is the pointer to the gauge object
 \returns a pointer to the GArray of polygons,  DO NOT FREE THIS.
 */
const GArray * mtx_gauge_face_get_polygons(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),NULL);
	return priv->polygons;
}


/*!
 \brief changes an attribute based on passed enum
 \param gauge is the pointer to the gauge object
 \param field is the enumeration of the field to change
 \param value is the new value
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_set_attribute(MtxGaugeFace *gauge,MtxGenAttr field, gfloat value)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_return_val_if_fail (field < NUM_ATTRIBUTES,FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));

	switch (field)
	{
		case START_ANGLE:
			priv->start_angle = value;
			break;
		case SWEEP_ANGLE:
			priv->sweep_angle = value;
			break;
		case ROTATION:
			priv->rotation = value;
			break;
		case LBOUND:
			priv->lbound = value;
			break;
		case UBOUND:
			priv->ubound = value;
			if (priv->peak > priv->ubound)
				priv->peak = priv->ubound;
			break;
		case NEEDLE_TAIL:
			priv->needle_tail = value;
			break;
		case NEEDLE_LENGTH:
			priv->needle_length = value;
			break;
		case NEEDLE_WIDTH:
			priv->needle_width = value;
			break;
		case NEEDLE_TIP_WIDTH:
			priv->needle_tip_width = value;
			break;
		case NEEDLE_TAIL_WIDTH:
			priv->needle_tail_width =value;
			break;
		case VALUE_FONTSCALE:
			priv->value_font_scale = value;
			break;
		case VALUE_XPOS:
			priv->value_xpos = value;
			break;
		case VALUE_YPOS:
			priv->value_ypos = value;
			break;
		case TATTLETALE_ALPHA:
			priv->tattletale_alpha = value;
			break;
		case PRECISION:
			priv->precision = (gint)value;
			break;
		case ANTIALIAS:
			priv->antialias = (gint)value;
			break;
		case TATTLETALE:
			priv->show_tattletale = (gint)value;
			break;
		case SHOW_VALUE:
			priv->show_value = (gint)value;
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}


/*!
 \brief gets and attribute based on passed enum
 \param gauge is the pointer to the gauge object
 \param field is the enumeration of the field to change
 \param value is the pointer to where to store the new value
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_get_attribute(MtxGaugeFace *gauge,MtxGenAttr field, gfloat * value)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail ((MTX_IS_GAUGE_FACE (gauge)),FALSE);
	g_return_val_if_fail (value,FALSE);
	g_return_val_if_fail ((field < NUM_ATTRIBUTES),FALSE);

	switch (field)
	{
		case START_ANGLE:
			*value = priv->start_angle;
			break;
		case SWEEP_ANGLE:
			*value = priv->sweep_angle;
			break;
		case ROTATION:
			*value = priv->rotation;
			break;
		case LBOUND:
			*value = priv->lbound;
			break;
		case UBOUND:
			*value = priv->ubound;
			break;
		case NEEDLE_TAIL:
			*value = priv->needle_tail;
			break;
		case NEEDLE_LENGTH:
			*value = priv->needle_length;
			break;
		case NEEDLE_WIDTH:
			*value = priv->needle_width;
			break;
		case NEEDLE_TIP_WIDTH:
			*value = priv->needle_tip_width;
			break;
		case NEEDLE_TAIL_WIDTH:
			*value = priv->needle_tail_width;
			break;
		case VALUE_FONTSCALE:
			*value = priv->value_font_scale;
			break;
		case VALUE_XPOS:
			*value = priv->value_xpos;
			break;
		case VALUE_YPOS:
			*value = priv->value_ypos;
			break;
		case TATTLETALE_ALPHA:
			*value = priv->tattletale_alpha;
			break;
		case PRECISION:
			*value = (gfloat)priv->precision;
			break;
		case ANTIALIAS:
			*value = (gfloat)priv->antialias;
			break;
		case TATTLETALE:
			*value = (gfloat)priv->show_tattletale;
			break;
		case SHOW_VALUE:
			*value = (gfloat)priv->show_value;
			break;
		default:
			return FALSE;
			break;
	}
	return TRUE;
}


/*!
 \brief changes a text block field by passing in an index 
 to the test block and the field name to change
 \param gauge is the pointer to the gauge object
 \param index is the index of the textblock
 \param field is the enumeration of the field to change
 \param value is a pointer to the new value
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_alter_text_block(MtxGaugeFace *gauge, gint index,TbField field, void * value)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxTextBlock *tblock = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_return_val_if_fail (value,FALSE);
	g_return_val_if_fail (field < TB_NUM_FIELDS,FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));

	tblock = g_array_index(priv->t_blocks,MtxTextBlock *,index);
	g_return_val_if_fail (tblock != NULL,FALSE);
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
		case TB_COLOR_DAY:
			tblock->color[MTX_DAY] = *(GdkColor *)value;
			break;
		case TB_COLOR_NITE:
			tblock->color[MTX_NITE] = *(GdkColor *)value;
			break;
		case TB_FONT:
			g_free(tblock->font);
			tblock->font = g_strdup(value);
			break;
		case TB_TEXT:
			g_free(tblock->text);
			tblock->text = g_strdup(value);
			break;
		case TB_LAYER:
			tblock->layer = (gint)*(gfloat *)value;
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}


/*!
 \brief changes a tick group field by passing in an index 
 to the tick group and the field name to change
 \param gauge is the pointer to the gauge object
 \param index is the index of the tickgroup
 \param field is the enumeration of the field to change
 \param value is the pointer to the new value
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_alter_tick_group(MtxGaugeFace *gauge, gint index,TgField field, void * value)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxTickGroup *tgroup = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_return_val_if_fail (field < TG_NUM_FIELDS,FALSE);
	g_return_val_if_fail (value,FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));

	tgroup = g_array_index(priv->tick_groups,MtxTickGroup *,index);
	g_return_val_if_fail (tgroup != NULL,FALSE);
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
		case TG_TEXT_COLOR_DAY:
			tgroup->text_color[MTX_DAY] = *(GdkColor *)value;
			break;
		case TG_TEXT_COLOR_NITE:
			tgroup->text_color[MTX_NITE] = *(GdkColor *)value;
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
		case TG_MAJ_TICK_COLOR_DAY:
			tgroup->maj_tick_color[MTX_DAY] = *(GdkColor *)value;
			break;
		case TG_MAJ_TICK_COLOR_NITE:
			tgroup->maj_tick_color[MTX_NITE] = *(GdkColor *)value;
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
		case TG_MIN_TICK_COLOR_DAY:
			tgroup->min_tick_color[MTX_DAY] = *(GdkColor *)value;
			break;
		case TG_MIN_TICK_COLOR_NITE:
			tgroup->min_tick_color[MTX_NITE] = *(GdkColor *)value;
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
		case TG_SWEEP_ANGLE:
			tgroup->sweep_angle = *(gfloat *)value;
			break;
		case TG_LAYER:
			tgroup->layer = (gint)*(gfloat *)value;
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}



/*!
 \brief changes a polygon field by passing in an index 
 to the polygon and the field name to change
 \param gauge is the pointer to the gauge object
 \param index is the index of the tickgroup
 \param field is the enumeration of the field to change
 \param value is the pointer to the new value
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_alter_polygon(MtxGaugeFace *gauge, gint index,PolyField field, void * value)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxPolygon *poly = NULL;
	void *data = NULL;
	gint i = 0;
	gint num_points = 0;

	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_return_val_if_fail (field < POLY_NUM_FIELDS,FALSE);
	g_return_val_if_fail (value, FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));


	poly = g_array_index(priv->polygons,MtxPolygon *,index);
	g_return_val_if_fail (poly != NULL,FALSE);
	data = poly->data;
	switch (field)
	{
		case POLY_COLOR_DAY:
			poly->color[MTX_DAY] = *(GdkColor *)value;
			break;
		case POLY_COLOR_NITE:
			poly->color[MTX_NITE] = *(GdkColor *)value;
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
		case POLY_LAYER:
			poly->layer = (gint)*(gfloat *)value;
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}


/*!
 \brief changes a color range field by passing in an index 
 to the color range array and the field name to change
 \param gauge is the pointer to the gauge object
 \param index is the index of the WarningRange
 \param field is the enumeration of the field to change
 \param value is the pointer to the new value
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_alter_warning_range(MtxGaugeFace *gauge, gint index,WrField field, void * value)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxWarningRange *w_range = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_return_val_if_fail (value,FALSE);
	g_return_val_if_fail (field < WR_NUM_FIELDS,FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));

	w_range = g_array_index(priv->w_ranges,MtxWarningRange *,index);
	g_return_val_if_fail (w_range != NULL,FALSE);
	switch (field)
	{
		case WR_LOWPOINT:
			w_range->lowpoint = *(gfloat *)value;
			break;
		case WR_HIGHPOINT:
			w_range->highpoint = *(gfloat *)value;
			break;
		case WR_INSET:
			w_range->inset = *(gfloat *)value;
			break;
		case WR_LWIDTH:
			w_range->lwidth = *(gfloat *)value;
			break;
		case WR_COLOR_DAY:
			w_range->color[MTX_DAY] = *(GdkColor *)value;
			break;
		case WR_COLOR_NITE:
			w_range->color[MTX_NITE] = *(GdkColor *)value;
			break;
		case WR_LAYER:
			w_range->layer = (gint)*(gfloat *)value;
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}


/*!
 \brief changes a alert range field by passing in an index 
 to the alert range array and the field name to change
 \param gauge is the pointer to the gauge object
 \param index is the index of the AlertRange
 \param field is the enumeration of the field to change
 \param value is the pointe to the new value
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_alter_alert_range(MtxGaugeFace *gauge, gint index,AlertField field, void * value)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxAlertRange *a_range = NULL;
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_return_val_if_fail (value,FALSE);
	g_return_val_if_fail (field < ALRT_NUM_FIELDS,FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));

	a_range = g_array_index(priv->a_ranges,MtxAlertRange *,index);
	g_return_val_if_fail (a_range != NULL,FALSE);
	switch (field)
	{
		case ALRT_LOWPOINT:
			a_range->lowpoint = *(gfloat *)value;
			break;
		case ALRT_HIGHPOINT:
			a_range->highpoint = *(gfloat *)value;
			break;
		case ALRT_X_OFFSET:
			a_range->x_offset = *(gfloat *)value;
			break;
		case ALRT_Y_OFFSET:
			a_range->y_offset = *(gfloat *)value;
			break;
		case ALRT_INSET:
			a_range->inset = *(gfloat *)value;
			break;
		case ALRT_LWIDTH:
			a_range->lwidth = *(gfloat *)value;
			break;
		case ALRT_COLOR_DAY:
			a_range->color[MTX_DAY] = *(GdkColor *)value;
			break;
		case ALRT_COLOR_NITE:
			a_range->color[MTX_NITE] = *(GdkColor *)value;
			break;
		case ALRT_LAYER:
			a_range->layer = (gint)*(gfloat *)value;
			break;
		default:
			break;
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}


/*!
 \brief clears all color ranges from the gauge
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_remove_all_warning_ranges(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxWarningRange *w_range = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=priv->w_ranges->len-1;i>=0;i--)
	{
		w_range = g_array_index(priv->w_ranges,MtxWarningRange *, i);
		priv->w_ranges = g_array_remove_index(priv->w_ranges,i);
		g_free(w_range);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}


/*!
 \brief clears all alert ranges from the gauge
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_remove_all_alert_ranges(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxAlertRange *a_range = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=priv->a_ranges->len-1;i>=0;i--)
	{
		a_range = g_array_index(priv->a_ranges,MtxAlertRange *, i);
		priv->a_ranges = g_array_remove_index(priv->a_ranges,i);
		g_free(a_range);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}


/*!
 \brief clears all text blocks from the gauge
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_remove_all_text_blocks(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxTextBlock *tblock = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=priv->t_blocks->len-1;i>=0;i--)
	{
		tblock = g_array_index(priv->t_blocks,MtxTextBlock *, i);
		priv->t_blocks = g_array_remove_index(priv->t_blocks,i);
		g_free(tblock->font);
		g_free(tblock->text);
		g_free(tblock);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}



/*!
 \brief clears all tick groups from the gauge
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_remove_all_tick_groups(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxTextBlock *tgroup = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=priv->tick_groups->len-1;i>=0;i--)
	{
		tgroup = g_array_index(priv->tick_groups,MtxTextBlock *, i);
		priv->tick_groups = g_array_remove_index(priv->tick_groups,i);
		g_free(tgroup->font);
		g_free(tgroup->text);
		g_free(tgroup);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}


/*!
 \brief clears all polygons from the gauge
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_remove_all_polygons(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxPolygon *poly = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	for (i=priv->polygons->len-1;i>=0;i--)
	{
		poly = g_array_index(priv->polygons,MtxPolygon *, i);
		priv->polygons = g_array_remove_index(priv->polygons,i);
		if (poly->type == MTX_GENPOLY)
		{
			if (((MtxGenPoly *)poly->data)->points)
				g_free(((MtxGenPoly *)poly->data)->points);
		}
		g_free(poly->data);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;
}


/*!
 \brief clears a specific color range based on index passed
 \param gauge is the pointer to the gauge object
 \param index is the index of the one we want to remove.
 */
void mtx_gauge_face_remove_warning_range(MtxGaugeFace *gauge, guint index)
{
	MtxWarningRange *w_range = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (index < priv->w_ranges->len)
	{
		w_range = g_array_index(priv->w_ranges,MtxWarningRange *, index);
		priv->w_ranges = g_array_remove_index(priv->w_ranges,index);
		if (w_range)
			g_free(w_range);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;

}


/*!
 \brief clears a specific alert range based on index passed
 \param gauge is the pointer to the gauge object
 \param index is the index of the one we want to remove.
 */
void mtx_gauge_face_remove_alert_range(MtxGaugeFace *gauge, guint index)
{
	MtxAlertRange *a_range = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (index < priv->a_ranges->len)
	{
		a_range = g_array_index(priv->a_ranges,MtxAlertRange *, index);
		priv->a_ranges = g_array_remove_index(priv->a_ranges,index);
		if (a_range)
			g_free(a_range);
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;

}


/*!
 \brief clears a specific text_block based on index passed
 \param gauge is the pointer to the gauge object
 \param index is the index of the one we want to remove.
 */
void mtx_gauge_face_remove_text_block(MtxGaugeFace *gauge, guint index)
{
	MtxTextBlock *tblock = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (index < priv->t_blocks->len)
	{
		tblock = g_array_index(priv->t_blocks,MtxTextBlock *, index);
		priv->t_blocks = g_array_remove_index(priv->t_blocks,index);
		if (tblock)
		{
			g_free(tblock->font);
			g_free(tblock->text);
			g_free(tblock);
		}
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;

}


/*!
 \brief clears a specific tick_group based on index passed
 \param gauge is the pointer to the gauge object
 \param index is the index of the one we want to remove.
 */
void mtx_gauge_face_remove_tick_group(MtxGaugeFace *gauge, guint index)
{
	MtxTextBlock *tgroup = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (index < priv->tick_groups->len)
	{
		tgroup = g_array_index(priv->tick_groups,MtxTextBlock *, index);
		priv->tick_groups = g_array_remove_index(priv->tick_groups,index);
		if (tgroup)
		{
			g_free(tgroup->font);
			g_free(tgroup->text);
			g_free(tgroup);
		}
	}
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;

}


/*!
 \brief clears a specific polygon based on index passed
 \param gauge is the pointer to the gauge object
 \param index is the index of the one we want to remove.
 */
void mtx_gauge_face_remove_polygon(MtxGaugeFace *gauge, guint index)
{
	MtxPolygon *poly = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	if (index < priv->polygons->len)
	{
		poly = g_array_index(priv->polygons,MtxPolygon *, index);
		priv->polygons = g_array_remove_index(priv->polygons,index);
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
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return;

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
 \param gauge is the pointer to the gauge object
 \param state is the state to set the drag border to
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_set_show_drag_border(MtxGaugeFace *gauge, gboolean state)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));
	priv->show_drag_border = state;
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	mtx_gauge_face_configure(GTK_WIDGET(gauge),NULL);
	gdk_window_clear_area_e(GTK_WIDGET(gauge)->window,0,0,priv->w, priv->h);
	return TRUE;
}


/*!
 \brief enables showing of gauge tattletale
 \param gauge is the pointer to the gauge object
 \param state is the state to set it to
 */
void mtx_gauge_face_set_show_tattletale(MtxGaugeFace *gauge, gboolean state)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	priv->show_tattletale = state;
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief gets whether tattletale is enabled or not
 \param gauge is the pointer to the gauge object
 \returns TRUE if tattle tale is set to be shown, or FALSE otherwise
 */
gboolean mtx_gauge_face_get_show_tattletale(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	return priv->show_tattletale;
}


/*!
 \brief sets alpha for tattletale
 \param gauge is the pointer to the gauge object
 \param alpha is the alpha value from 0-1 to set for the tattletale
 */
void mtx_gauge_face_set_tattletale_alpha(MtxGaugeFace *gauge, gfloat alpha)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	priv->tattletale_alpha = alpha;
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief gets tattletale alpha value
 \param gauge is the pointer to the gauge object
 \returns the Tattletale alpha value
 */
gfloat mtx_gauge_face_get_tattletale_alpha(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),0.0);
	return priv->tattletale_alpha;
}


/*!
 \brief gets the peak value 
 \param gauge is the pointer to the gauge object
 */
gfloat mtx_gauge_face_get_peak (MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail ((MTX_IS_GAUGE_FACE (gauge)),0.0);
	return	priv->peak;
}


/*!
 \brief clears the peak value 
 \param gauge is the pointer to the gauge object
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_gauge_face_clear_peak (MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail ((MTX_IS_GAUGE_FACE (gauge)),FALSE);
	g_object_freeze_notify (G_OBJECT (gauge));
	priv->peak = priv->value;
	g_object_thaw_notify (G_OBJECT (gauge));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
		return TRUE;
#endif
	generate_gauge_background(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
	return TRUE;
}


/*!
 \brief gets the state of the drag border
 \param gauge is the pointer to the gauge object
 \returns the state of whether the gauge shows the drag border
 */
gboolean mtx_gauge_face_get_show_drag_border(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), FALSE);
	return priv->show_drag_border;
}


/*!
 \brief gets called to redraw the entire display manually
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_redraw_canvas (MtxGaugeFace *gauge)
{
	if (!GTK_WIDGET(gauge)->window) return;

	update_gauge_position(gauge);
	gdk_window_clear(GTK_WIDGET(gauge)->window);
}


/*!
 \brief gets the state of the daytime_mode flag
 \param gauge is the pointer to the gauge object
 \returns the state of whether the gauge is in daytime or nitetime mode
 */
gboolean mtx_gauge_face_get_daytime_mode(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), FALSE);
	return priv->daytime_mode;
}


/*!
 \brief sets the state of the daytime_mode flag
 \param gauge is the pointer to the gauge object
 \returns TRUE on sucess, FALSE otherwise
 */
gboolean mtx_gauge_face_set_daytime_mode(MtxGaugeFace *gauge, gboolean mode)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	if (priv->daytime_mode == mode)
		return TRUE;
	else
	{
		priv->daytime_mode = mode;
#if GTK_MINOR_VERSION >= 18
		if (!gtk_widget_is_sensitive(GTK_WIDGET(gauge)))
			return TRUE;
#else
		if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(gauge)))
			return TRUE;
#endif
		generate_gauge_background(gauge);
		mtx_gauge_face_redraw_canvas (gauge);
	}
	return TRUE;
}


/*!
 \brief gets the last click coords in normalized -1 <-> +1 scales
 \param gauge is the pointer to the gauge object
 \param x is the pointer to where to store the X coordinate or NULL
 \param y is the pointer to where to store the Y coordinate or NULL
 */
void mtx_gauge_face_get_last_click_coords(MtxGaugeFace *gauge, gdouble *x, gdouble *y)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	if (x)
		*x = priv->last_click_x;
	if (y)
		*y = priv->last_click_y;
	return;
}
