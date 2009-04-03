/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt curve widget
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */


#include <config.h>
#include <cairo/cairo.h>
#include <curve.h>
#include <curve-private.h>
#include <gtk/gtk.h>


/*!
 \brief gets called when  a user wants a new curve
 \returns a pointer to a newly created curve widget
 */
GtkWidget *mtx_curve_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_CURVE, NULL));
}


/*!
 \brief gets the current value 
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_get_coords (MtxCurve *curve, gint *num_points, MtxCurveCoord *array)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	array = priv->coords;
	*num_points = priv->num_points;
	return TRUE;
}


/*!
 \brief sets the current points 
 \param curve (MtxCurve *) pointer to curve
 \param num_points (gint) new value
 \param array (MtxCurveCoord*) Array of points
 */
gboolean mtx_curve_set_coords (MtxCurve *curve, gint num_points, MtxCurveCoord *array)
{
	/* gint i = 0; */
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	if (priv->coords)
		g_free(priv->coords);
	priv->coords = g_memdup(array,(num_points*sizeof(MtxCurveCoord)));
	priv->num_points = num_points;
	recalc_extremes(priv);
	/*
 	for (i=0;i<num_points;i++)
		printf("new coord %f,%f\n",priv->coords[i].x,priv->coords[i].y);
	*/
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief sets the current points to empty array
 \param curve (MtxCurve *) pointer to curve
 \param num_points (gint) size of array to create
 */
gboolean mtx_curve_set_empty_array (MtxCurve *curve, gint num_points)
{
	gint i = 0;
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	if (priv->coords)
		g_free(priv->coords);
	priv->coords = g_new0(MtxCurveCoord,num_points);
	priv->num_points = num_points;
	priv->highest_x = 0;
	priv->highest_y = 0;
	priv->lowest_x = 0;
	priv->lowest_y = 0;
	for (i=0;i<num_points;i++)
	{
		priv->coords[i].x = 0;
		priv->coords[i].y = 0;
	}
	recalc_extremes(priv);
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief sets the value of one point
 \param curve (MtxCurve *) pointer to curve
 \param index (gfloat) index of point
 \param point (gfloat) new point coords
 */
gboolean mtx_curve_set_coords_at_index (MtxCurve *curve, gint index, MtxCurveCoord point)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_return_val_if_fail (priv->num_points > index,FALSE);
	g_return_val_if_fail (index >= 0,FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->coords[index].x = point.x;
	priv->coords[index].y = point.y;
	/*printf("set_coords_at_index at index %i changed to %.2f,%.2f\n",index,priv->coords[index].x,priv->coords[index].y);
 	*/
	recalc_extremes(priv);
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief gets the value of one point
 \param curve (MtxCurve *) pointer to curve
 \param index (gfloat) index of point
 \param point (gfloat) new point coords
 */
gboolean mtx_curve_get_coords_at_index (MtxCurve *curve, gint index, MtxCurveCoord *point)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_return_val_if_fail (priv->num_points > index,FALSE);
	g_return_val_if_fail (index >= 0,FALSE);
	point->x = priv->coords[index].x;
	point->y = priv->coords[index].y;
	return TRUE;
}


/*!
 \brief sets the color for the passed index
 \param curve (MtxCurve *) pointer to curve
 \param value (gfloat) new value
 */
gboolean mtx_curve_set_color (MtxCurve *curve, CurveColorIndex index, GdkColor color)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_return_val_if_fail (index < CURVE_NUM_COLORS,FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->colors[index].red = color.red;
        priv->colors[index].green = color.green;
        priv->colors[index].blue = color.blue;
        priv->colors[index].pixel = color.pixel;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}



/*!
 \brief gets the color for the passed index
 \param curve (MtxCurve *) pointer to curve
 \param index (gfloat) new value
 \param color (gfloat) pointer to color struct being filled in.
 \returns a point to the internal GdkColor struct which should NOT be freed
 */
gboolean mtx_curve_get_color (MtxCurve *curve, CurveColorIndex index, GdkColor *color)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_return_val_if_fail (index < CURVE_NUM_COLORS,FALSE);
	color->red = priv->colors[index].red;
        color->green = priv->colors[index].green;
        color->blue = priv->colors[index].blue;
        color->pixel = priv->colors[index].pixel;
	return TRUE;
}


/*!
 \brief gets the title of the display
 \param curve (MtxCurve *) pointer to curve
 \returns title text(DO NOT FREE this)
 */
const gchar * mtx_curve_get_title (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	return priv->title;
}


/*!
 \brief sets the curve title
 \param curve (MtxCurve *) pointer to curve
 \returns title text(DO NOT FREE this)
 */
gboolean mtx_curve_set_title (MtxCurve *curve, gchar * new_title)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->title = g_strdup(new_title);
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief sets the show_vertex param
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_set_show_vertexes (MtxCurve *curve, gboolean value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->show_vertexes = value;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief gets the show_vertexes param
 \param curve (MtxCurve *) pointer to curve
 \returns true or false
 */
gboolean mtx_curve_get_show_vertexes (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	return priv->show_vertexes;
}


/*!
 \brief sets the auto_hide vertexes param
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_set_auto_hide_vertexes (MtxCurve *curve, gboolean value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->auto_hide = value;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief gets the auto_hide param
 \param curve (MtxCurve *) pointer to curve
 \returns true or false
 */
gboolean mtx_curve_get_auto_hide_vertexes (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	return priv->auto_hide;
}


/*!
 \brief sets the show_grat param
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_set_show_graticule (MtxCurve *curve, gboolean value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->show_grat = value;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief sets the show_x_marker param
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_set_show_x_marker (MtxCurve *curve, gboolean value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->show_x_marker = value;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief sets the show_y_marker param
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_set_show_y_marker (MtxCurve *curve, gboolean value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->show_y_marker = value;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief displays a live marker for the X axis (vertical Line)
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_set_x_marker_value (MtxCurve *curve, gfloat value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->x_marker = value;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief displays a live marker for the Y axis (horizontal Line)
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_set_y_marker_value (MtxCurve *curve, gfloat value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->y_marker = value;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief gets the show_graticule param
 \param curve (MtxCurve *) pointer to curve
 \returns true or false
 */
gboolean mtx_curve_get_show_graticule (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	return priv->show_grat;
}


/*!
 \brief gets the show_x_marker param
 \param curve (MtxCurve *) pointer to curve
 \returns true or false
 */
gboolean mtx_curve_get_show_x_marker (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	return priv->show_x_marker;
}


/*!
 \brief gets the show_y_marker param
 \param curve (MtxCurve *) pointer to curve
 \returns true or false
 */
gboolean mtx_curve_get_show_y_marker (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	return priv->show_y_marker;
}


/*!
 \brief gets the x_precision param
 \param curve (MtxCurve *) pointer to curve
 \returns true or false
 */
gint mtx_curve_get_x_precision (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),-1);
	return priv->x_precision;
}


/*!
 \brief gets the y_precision param
 \param curve (MtxCurve *) pointer to curve
 \returns true or false
 */
gint mtx_curve_get_y_precision (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),-1);
	return priv->y_precision;
}


/*!
 \brief sets the x_precision param
 \param curve (MtxCurve *) pointer to curve
 \param precision (gint) precision in significant digits
 \returns true or false on success/failure
 */
gboolean mtx_curve_set_x_precision (MtxCurve *curve, gint precision)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	priv->x_precision = precision;
	return TRUE;
}


/*!
 \brief sets the y_precision param
 \param curve (MtxCurve *) pointer to curve
 \param precision (gint) precision in significant digits
 \returns true or false on success/failure
 */
gboolean mtx_curve_set_y_precision (MtxCurve *curve, gint precision)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	priv->y_precision = precision;
	return TRUE;
}


/*!
 \brief gets the active_vertex index
 \param curve (MtxCurve *) pointer to curve
 \returns active vertex
 */
gint mtx_curve_get_active_coord_index (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),-1);
	return priv->active_coord;
}


