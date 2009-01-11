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
 \param value (gfloat) new value
 */
void mtx_curve_set_points (MtxCurve *curve, gint num_points, GdkPoint *array)
{
	gint i = 0;

	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_if_fail (MTX_IS_CURVE (curve));
	g_object_freeze_notify (G_OBJECT (curve));
	if (priv->points)
		g_free(priv->points);
	priv->points = g_memdup(array,(num_points*sizeof(GdkPoint)));
	priv->num_points = num_points;
	priv->highest_x = -10000;
	priv->highest_y = -10000;
	priv->lowest_x = 10000;
	priv->lowest_y = 10000;
	for (i=0;i<num_points;i++)
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
	g_object_freeze_notify (G_OBJECT (curve));
	priv->points[index].x = point.x;
	priv->points[index].y = point.y;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
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
 \brief sets the update policy
 \param curve (MtxCurve *) pointer to curve
 \param type (GtkUpdateType) new value
 */
void mtx_curve_set_update_policy (MtxCurve *curve, GtkUpdateType type)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_if_fail (MTX_IS_CURVE (curve));
	g_return_if_fail ((type == GTK_UPDATE_CONTINUOUS) ||
			  (type == GTK_UPDATE_DISCONTINUOUS) ||
			  (type == GTK_UPDATE_DELAYED));
	g_object_freeze_notify (G_OBJECT (curve));
	priv->type = type;
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
}


/*!
 \brief sets the update policy
 \param curve (MtxCurve *) pointer to curve
 \param type (GtkUpdateType) new value
 \returns update policy
 */
GtkUpdateType mtx_curve_get_update_policy (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	return priv->type;
}


/*!
 \brief sets the update policy
 \param curve (MtxCurve *) pointer to curve
 \returns title text(DO NOT FREE this)
 */
const gchar * mtx_curve_get_title (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
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


