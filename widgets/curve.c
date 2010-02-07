/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix curve widget
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
#include <math.h>
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
/*	gint i = 0; */
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
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
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
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
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
	/*printf("set_coords_at_index at index %i changed to %.2f,%.2f\n",index,priv->coords[index].x,priv->coords[index].y);*/
	recalc_extremes(priv);
	g_object_thaw_notify (G_OBJECT (curve));
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
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
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
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
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
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
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
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
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
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
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
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
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
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
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief displays a live marker for the X axis (vertical Line)
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_get_x_marker_value (MtxCurve *curve, gfloat *value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);

	*value = priv->x_marker;
	return TRUE;
}


/*!
 \brief displays a live marker for the Y axis (horizontal Line)
 \param curve (MtxCurve *) pointer to curve
 */
gboolean mtx_curve_get_y_marker_value (MtxCurve *curve, gfloat *value)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);

	*value = priv->y_marker;
	return TRUE;
}


/*!
 \brief displays a live marker for the X axis (vertical Line)
 \param curve (MtxCurve *) pointer to curve
 */
void mtx_curve_set_x_marker_value (MtxCurve *curve, gfloat value)
{
	MtxCurvePrivate *priv = NULL;
	gint i = 0;
	gfloat x = 0.0;
	gfloat y = 0.0;
	gfloat d1 = 0.0;
	gfloat d2 = 0.0;
	gboolean get_peak_cross = FALSE;

	g_return_if_fail (MTX_IS_CURVE (curve));
	priv = MTX_CURVE_GET_PRIVATE(curve);

	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return;
	if (priv->x_marker == value)
		return;
	if (((value < priv->lowest_x) && (priv->x_marker_clamp == LOW)) || ((value > priv->highest_x) && (priv->x_marker_clamp == HIGH)))
		return;
	/* Filter out jitter to within 1% */
	if (fabs(value-priv->x_marker) < (fabs(priv->highest_x-priv->lowest_x)/100.0))
		return;

	g_object_freeze_notify (G_OBJECT (curve));
	if (value < priv->lowest_x)
	{
		priv->x_marker = priv->lowest_x;
		priv->x_marker_clamp = LOW;
	}
	else if (value > priv->highest_x)
	{
		priv->x_marker = priv->highest_x;
		priv->x_marker_clamp = HIGH;
	}
	else
	{
		priv->x_marker = value;
		priv->x_marker_clamp = NONE;
	}
	if (value > priv->peak_x_marker)
	{
		priv->peak_x_marker = value;
		get_peak_cross = TRUE;
		if (priv->x_peak_timeout)
		{
			g_source_remove(priv->x_peak_timeout);
			priv->x_peak_timeout = -1;
		}
	}
	else
	{
		priv->x_draw_peak = TRUE;
		g_object_set_data(G_OBJECT(curve),"axis",GINT_TO_POINTER(_X_));
		if (priv->x_peak_timeout <= 0)
			priv->x_peak_timeout = g_timeout_add(5000,(GtkFunction)cancel_peak,(gpointer)curve);
	}
	for (i = 0;i<priv->num_points - 1;i++)
	{
		if (value < priv->coords[0].x)
		{
			priv->y_at_x_marker = priv->coords[0].y;
			if (0 != priv->marker_proximity_vertex)
			{
				priv->marker_proximity_vertex = 0;
				g_signal_emit_by_name((gpointer)curve, "marker-proximity");
			}
		}
		else if (value > priv->coords[priv->num_points-1].x)
		{
			priv->y_at_x_marker = priv->coords[priv->num_points-1].y;
			if (get_peak_cross)
				priv->peak_y_at_x_marker = priv->y_at_x_marker;
			if (priv->num_points-1 != priv->marker_proximity_vertex)
			{
				priv->marker_proximity_vertex = priv->num_points-1;
				g_signal_emit_by_name((gpointer)curve, "marker-proximity");
			}
		}
		else if ((value > priv->coords[i].x) && (value < priv->coords[i+1].x))
		{
			if (get_intersection(priv->coords[i].x,priv->coords[i].y,priv->coords[i+1].x,priv->coords[i+1].y,value,0,value,priv->h,&x,&y))
			{
				priv->y_at_x_marker = y;
				if (get_peak_cross)
					priv->peak_y_at_x_marker = y;
				d1 = sqrt(pow((priv->coords[i].x-value),2)+ pow((priv->coords[i].y-y),2));
				d2 = sqrt(pow((priv->coords[i+1].x-value),2)+ pow((priv->coords[i+1].y-y),2));
				if (d1 < d2)
				{
					if (i != priv->marker_proximity_vertex)
					{
						priv->marker_proximity_vertex = i;
						g_signal_emit_by_name((gpointer)curve, "marker-proximity");
					}
				}
				else
				{
					if (i+1 != priv->marker_proximity_vertex)
					{
						priv->marker_proximity_vertex = i+1;
						g_signal_emit_by_name((gpointer)curve, "marker-proximity");
					}
				}

			}
			else
				printf("couldn't find intersection\n");
			break;
		}
	}
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return;
}


/*!
 \brief displays a live marker for the Y axis (horizontal Line)
 \param curve (MtxCurve *) pointer to curve
 */
void mtx_curve_set_y_marker_value (MtxCurve *curve, gfloat value)
{
	MtxCurvePrivate *priv = NULL;
	gint i = 0;
	gfloat x = 0.0;
	gfloat y = 0.0;
	gfloat d1 = 0.0;
	gfloat d2 = 0.0;
	gboolean get_peak_cross = FALSE;
	g_return_if_fail (MTX_IS_CURVE (curve));
	priv = MTX_CURVE_GET_PRIVATE(curve);

	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return;
	if (priv->y_marker == value)
		return;
	/* IF marker is clamped beyond ranges, don't bother updating*/
	if (((value < priv->lowest_y) && (priv->y_marker_clamp == LOW)) || ((value > priv->highest_y) && (priv->y_marker_clamp == HIGH)))
		return;

	/* Filter out jitter to within 1% */
	if (fabs(value-priv->y_marker) < (fabs(priv->highest_y-priv->lowest_y)/100.0))
		return;

	g_object_freeze_notify (G_OBJECT (curve));
	if (value <priv->lowest_y)
	{
		priv->y_marker = priv->lowest_y;
		priv->y_marker_clamp = LOW;
	}
	else if (value > priv->highest_y)
	{
		priv->y_marker = priv->highest_y;
		priv->y_marker_clamp = HIGH;
	}
	else
	{
		priv->y_marker = value;
		priv->y_marker_clamp = NONE;
	}
	if (value > priv->peak_y_marker)
	{
		priv->peak_y_marker = value;
		get_peak_cross = TRUE;
		if (priv->y_peak_timeout)
		{
			g_source_remove(priv->y_peak_timeout);
			priv->y_peak_timeout = -1;
		}
	}
	else
	{
		priv->y_draw_peak = TRUE;
		g_object_set_data(G_OBJECT(curve),"axis",GINT_TO_POINTER(_Y_));
		if (priv->y_peak_timeout <= 0)
			priv->y_peak_timeout = g_timeout_add(5000,(GtkFunction)cancel_peak,(gpointer)curve);
	}
	for (i = 0;i<priv->num_points - 1;i++)
	{
		if (value < priv->coords[0].y)
		{
			priv->x_at_y_marker = priv->coords[0].x;
			if (0 != priv->marker_proximity_vertex)
			{
				priv->marker_proximity_vertex = 0;
				g_signal_emit_by_name((gpointer)curve, "marker-proximity");
			}
		}
		else if (value > priv->coords[priv->num_points-1].y)
		{
			priv->x_at_y_marker = priv->coords[priv->num_points-1].x;
			if (get_peak_cross)
				priv->peak_x_at_y_marker = priv->x_at_y_marker;
			if (priv->num_points-1 != priv->marker_proximity_vertex)
			{
				priv->marker_proximity_vertex = priv->num_points-1;
				g_signal_emit_by_name((gpointer)curve, "marker-proximity");
			}
		}
		else if ((value > priv->coords[i].y) && (value < priv->coords[i+1].y))
		{
			if (get_intersection(priv->coords[i].x,priv->coords[i].y,priv->coords[i+1].x,priv->coords[i+1].y,0,value,priv->w,value,&x,&y))
			{
				priv->x_at_y_marker = x;
				if (get_peak_cross)
					priv->peak_x_at_y_marker = x;
				d1 = sqrt(pow((priv->coords[i].x-x),2)+ pow((priv->coords[i].y-value),2));
				d2 = sqrt(pow((priv->coords[i+1].x-x),2)+ pow((priv->coords[i+1].y-value),2));
				if (d1 < d2)
				{
					if (i != priv->marker_proximity_vertex)
					{
						priv->marker_proximity_vertex = i;
						g_signal_emit_by_name((gpointer)curve, "marker-proximity");
					}
				}
				else
				{
					if (i+1 != priv->marker_proximity_vertex)
					{
						priv->marker_proximity_vertex = i+1;
						g_signal_emit_by_name((gpointer)curve, "marker-proximity");
					}
				}

			}
			else
				printf("couldn't find intersection\n");
			break;
		}
	}
	g_object_thaw_notify (G_OBJECT (curve));
	mtx_curve_redraw(curve);
	return;
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




/*!
 \brief sets the hard limits for autoscroll
 \param curve (MtxCurve *) pointer to curve
 \returns active vertex
 */
gboolean mtx_curve_set_hard_limits (MtxCurve *curve, gfloat x_lower, gfloat x_upper, gfloat y_lower, gfloat y_upper)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	priv->x_lower_limit = x_lower;
	priv->x_upper_limit = x_upper;
	priv->y_lower_limit = y_lower;
	priv->y_upper_limit = y_upper;
	return TRUE;
}


/*!
 \brief gets the hard limits for autoscroll
 \param curve (MtxCurve *) pointer to curve
 \param x_lower (gint *) pointer to be filled in with value
 \param x_upper (gint *) pointer to be filled in with value
 \param y_lower (gint *) pointer to be filled in with value
 \param x_upper (gint *) pointer to be filled in with value
 \returns TRUE
 */
gboolean mtx_curve_get_hard_limits (MtxCurve *curve, gfloat *x_lower, gfloat *x_upper, gfloat *y_lower, gfloat *y_upper)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	if (x_lower)
		*x_lower = priv->x_lower_limit;
	if (y_lower)
		*y_lower = priv->y_lower_limit;
	if (x_upper)
		*x_lower = priv->x_upper_limit;
	if (y_upper)
		*y_lower = priv->y_upper_limit;
	return TRUE;
}


/*!
 \brief gets the current mouse proximity vertex
 \param curve (MtxCurve *) pointer to curve
 \returns active vertex
 */
gint mtx_curve_get_vertex_proximity_index (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),-1);
	return priv->proximity_vertex;
}


/*!
 \brief gets the current marker proximity vertex
 \param curve (MtxCurve *) pointer to curve
 \returns active vertex
 */
gint mtx_curve_get_marker_proximity_index (MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),-1);
	return priv->marker_proximity_vertex;
}


/*!
 \brief sets the axis lock state for the X axis
 \param curve (MtxCurve *) pointer to curve
 \param state (gboolean) state of axis lock
 \returns TRUE on succes
 */
gboolean mtx_curve_set_x_axis_lock_state(MtxCurve *curve, gboolean state)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	priv->x_blocked_from_edit = state;
	return TRUE;
}


/*!
 \brief sets the axis lock state for the Y axis
 \param curve (MtxCurve *) pointer to curve
 \param state (gboolean) state of axis lock
 \returns TRUE on succes
 */
gboolean mtx_curve_set_y_axis_lock_state(MtxCurve *curve, gboolean state)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	priv->y_blocked_from_edit = state;
	return TRUE;
}


/*!
 \brief gets the axis lock state for the X axis
 \param curve (MtxCurve *) pointer to curve
 \returns TRUE on succes
 */
gboolean mtx_curve_get_x_axis_lock_state(MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	return priv->x_blocked_from_edit;
}



/*!
 \brief gets the axis lock state for the Y axis
 \param curve (MtxCurve *) pointer to curve
 \returns TRUE on succes
 */
gboolean mtx_curve_get_y_axis_lock_state(MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	return priv->y_blocked_from_edit;
}


/*!
 \brief sets the X axis label
 \param curve (MtxCurve *) pointer to curve
 \param text (gchar *) label string
 \returns TRUE on succes
 */
gboolean mtx_curve_set_x_axis_label(MtxCurve *curve, const gchar *text)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	if (priv->x_axis_label)
		g_free(priv->x_axis_label);
	priv->x_axis_label = g_strdup(text);
	g_object_thaw_notify (G_OBJECT (curve));
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
	mtx_curve_redraw(curve);
	return TRUE;
}


/*!
 \brief sets the Y axis label
 \param curve (MtxCurve *) pointer to curve
 \param text (gchar *) label string
 \returns TRUE on succes
 */
gboolean mtx_curve_set_y_axis_label(MtxCurve *curve, const gchar *text)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	g_return_val_if_fail (MTX_IS_CURVE (curve),FALSE);
	g_object_freeze_notify (G_OBJECT (curve));
	if (priv->y_axis_label)
		g_free(priv->y_axis_label);
	priv->y_axis_label = g_strdup(text);
	g_object_thaw_notify (G_OBJECT (curve));
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
		return TRUE;
	mtx_curve_redraw(curve);
	return TRUE;
}


