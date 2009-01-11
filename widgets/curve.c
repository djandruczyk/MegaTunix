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
 \brief Recalculates the extremes of all points in the graph
 \param priv (MtxCurvePrivate *) pointer to private data
 */
void recalc_extremes(MtxCurvePrivate *priv)
{
	gint i = 0;
	priv->highest_x = -10000;
	priv->highest_y = -10000;
	priv->lowest_x = 10000;
	priv->lowest_y = 10000;
	for (i=0;i<priv->num_points;i++)
	{
		if (priv->points[i].x < priv->lowest_x)
			priv->lowest_x = priv->points[i].x;
		if (priv->points[i].x > priv->highest_x)
			priv->highest_x = priv->points[i].x;
		if (priv->points[i].y < priv->lowest_y)
			priv->lowest_y = priv->points[i].y;
		if (priv->points[i].y > priv->highest_y)
			priv->highest_y = priv->points[i].y;
	}
}
/*!
 \brief gets the current value 
 \param curve (MtxCurve *) pointer to curve
 */
void mtx_curve_get_points (MtxCurve *curve, gint *num_points, GdkPoint *array)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_if_fail (MTX_IS_CURVE (curve));
	array = priv->points;
	*num_points = priv->num_points;
}


/*!
 \brief sets the current points 
 \param curve (MtxCurve *) pointer to curve
 \param num_points (gint) new value
 \param array (GdkPoint*) Array of points
 */
void mtx_curve_set_points (MtxCurve *curve, gint num_points, GdkPoint *array)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_if_fail (MTX_IS_CURVE (curve));
	g_object_freeze_notify (G_OBJECT (curve));
	if (priv->points)
		g_free(priv->points);
	priv->points = g_memdup(array,(num_points*sizeof(GdkPoint)));
	priv->num_points = num_points;
	recalc_extremes(priv);
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
}


/*!
 \brief sets the current points to empty array
 \param curve (MtxCurve *) pointer to curve
 \param num_points (gint) size of array to create
 */
void mtx_curve_set_empty_array (MtxCurve *curve, gint num_points)
{
	gint i = 0;

	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_if_fail (MTX_IS_CURVE (curve));
	g_object_freeze_notify (G_OBJECT (curve));
	if (priv->points)
		g_free(priv->points);
	priv->points = g_new0(GdkPoint,num_points);
	priv->num_points = num_points;
	priv->highest_x = 0;
	priv->highest_y = 0;
	priv->lowest_x = 0;
	priv->lowest_y = 0;
	for (i=0;i<num_points;i++)
	{
		priv->points[i].x = 0;
		priv->points[i].y = 0;
	}
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
}


/*!
 \brief sets the value of one point
 \param curve (MtxCurve *) pointer to curve
 \param index (gfloat) index of point
 \param point (gfloat) new point coords
 */
gboolean mtx_curve_set_point_at_index (MtxCurve *curve, gint index, GdkPoint point)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_return_val_if_fail (priv->num_points > index,FALSE);
	g_return_val_if_fail (index >= 0,FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->points[index].x = point.x;
	priv->points[index].y = point.y;
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
gboolean mtx_curve_get_point_at_index (MtxCurve *curve, gint index, GdkPoint *point)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_return_val_if_fail (priv->num_points > index,FALSE);
	g_return_val_if_fail (index >= 0,FALSE);
	point->x = priv->points[index].x;
	point->y = priv->points[index].y;
	return TRUE;
}


/*!
 \brief sets the color for the passed index
 \param curve (MtxCurve *) pointer to curve
 \param value (gfloat) new value
 */
gboolean mtx_curve_set_color (MtxCurve *curve, ColorIndex index, GdkColor color)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_return_val_if_fail (index < NUM_COLORS,FALSE);
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
gboolean mtx_curve_get_color (MtxCurve *curve, ColorIndex index, GdkColor *color)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_return_val_if_fail (index < NUM_COLORS,FALSE);
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
void mtx_curve_set_title (MtxCurve *curve, gchar * new_title)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->title = g_strdup(new_title);
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
}


/*!
 \brief sets the show_vertex param
 \param curve (MtxCurve *) pointer to curve
 */
void mtx_curve_set_show_vertexes (MtxCurve *curve, gboolean value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->show_vertexes = value;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
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
 \brief sets the show_grat param
 \param curve (MtxCurve *) pointer to curve
 */
void mtx_curve_set_show_graticule (MtxCurve *curve, gboolean value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_object_freeze_notify (G_OBJECT (curve));
	priv->show_grat = value;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
}


/*!
 \brief gets the show_vertexes param
 \param curve (MtxCurve *) pointer to curve
 \returns true or false
 */
gboolean mtx_curve_get_show_graticule (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	return priv->show_grat;
}


