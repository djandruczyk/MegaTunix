/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Chris Mire (czb)
 *
 * MegaTunix curve widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */


#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <curve.h>
#include <math.h>


void coords_changed(MtxCurve *, gpointer);
void vertex_proximity(MtxCurve *, gpointer);
void marker_proximity(MtxCurve *, gpointer);
gboolean update_curve_marker(gpointer );

int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	GtkWidget *curve = NULL;
	MtxCurveCoord points[11];
	gint i = 0;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_size_request(GTK_WIDGET(window),320,320);
	curve = mtx_curve_new ();
	
	gtk_container_add (GTK_CONTAINER (window), curve);
	for (i=0;i<11;i++)
	{
		//points[i].x=i*1000;
		//points[i].y=(i*1000)-5000;
		points[i].x=i-6;
		points[i].y=powf(2.0,(gfloat)i);
		/*points[i].y=exp(i/2.0);*/
	}
	gtk_widget_show(curve);
	mtx_curve_set_coords(MTX_CURVE(curve),11,points);
	mtx_curve_set_title(MTX_CURVE(curve),"Curve Demo");
	mtx_curve_set_auto_hide_vertexes(MTX_CURVE(curve),FALSE);
	mtx_curve_set_show_x_marker(MTX_CURVE(curve),TRUE);
/*	mtx_curve_set_show_y_marker(MTX_CURVE(curve),TRUE);*/
	mtx_curve_set_show_vertexes(MTX_CURVE(curve),TRUE);
	mtx_curve_set_x_axis_label(MTX_CURVE(curve),"X Axis");
	mtx_curve_set_y_axis_label(MTX_CURVE(curve),"This is the Y Axis");
	mtx_curve_set_hard_limits(MTX_CURVE(curve),-7.0,7.0,0.0,1200.0);
	//mtx_curve_set_hard_limits(MTX_CURVE(curve),-2000.0,12000.0,-6000.0,7000.0);
	mtx_curve_set_x_precision(MTX_CURVE(curve),2);
	mtx_curve_set_y_precision(MTX_CURVE(curve),2);
	g_signal_connect(G_OBJECT(curve), "coords-changed",
			G_CALLBACK(coords_changed),NULL);
	g_signal_connect(G_OBJECT(curve), "vertex-proximity",
			G_CALLBACK(vertex_proximity),NULL);
	g_signal_connect(G_OBJECT(curve), "marker-proximity",
			G_CALLBACK(marker_proximity),NULL);

	(void)g_timeout_add(40,(GtkFunction)update_curve_marker,(gpointer)curve);

	gtk_widget_show_all (window);


	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_main ();
	return 0;
}

void coords_changed(MtxCurve *curve, gpointer data)
{
	gint index = mtx_curve_get_active_coord_index(curve);
	MtxCurveCoord point;
	mtx_curve_get_coords_at_index(curve,index,&point);
	/*printf("changed coord %i, to %.1f,%.1f\n",index,point.x,point.y);*/
	
}


void vertex_proximity(MtxCurve *curve, gpointer data)
{
	gint index = mtx_curve_get_vertex_proximity_index(curve);
	printf("changed proximity to coordinate %i,\n",index);
}

void marker_proximity(MtxCurve *curve, gpointer data)
{
	gint index = mtx_curve_get_marker_proximity_index(curve);
/*	printf("marker proximity to coordinate %i,\n",index);*/
}

gboolean update_curve_marker(gpointer data)
{
	GtkWidget *curve = data;
	gfloat min = -8.0;
	gfloat max = 8.0;
	//gfloat min = -5000.0;
	//gfloat max = 8000.0;
	static gfloat step = 0.125;
	//static gfloat step = 125;
	static gboolean rising = TRUE;
	static gfloat value = 0;

	if (value > max)
		rising = FALSE;
	if (value < min)
		rising = TRUE;

	if (rising)
		value+=step;
	else
		value-=step;
/*	printf("Setting x marker to %f\n",value); */
/*	mtx_curve_set_y_marker_value(MTX_CURVE(curve),(gfloat)value); */
	mtx_curve_set_x_marker_value(MTX_CURVE(curve),(gfloat)value);
	return TRUE;
}
