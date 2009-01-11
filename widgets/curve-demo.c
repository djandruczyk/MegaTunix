/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Chris Mire (czb)
 *
 * Megasquirt curve widget
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

int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	GtkWidget *curve = NULL;
	MtxCurveCoord points[10];
	gint i = 0;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	curve = mtx_curve_new ();
	
	gtk_container_add (GTK_CONTAINER (window), curve);
	for (i=0;i<10;i++)
	{
		points[i].x=i*1000;
		points[i].y=exp(i/2.0);
	}
	gtk_widget_realize(curve);
	mtx_curve_set_coords(MTX_CURVE(curve),10,points);
	mtx_curve_set_title(MTX_CURVE(curve),"Curve Demo");
	mtx_curve_set_auto_hide_vertexes(MTX_CURVE(curve),FALSE);
	mtx_curve_set_show_vertexes(MTX_CURVE(curve),TRUE);
	mtx_curve_set_y_precision(MTX_CURVE(curve),1);
	g_signal_connect(G_OBJECT(curve), "coords-changed",
			G_CALLBACK(coords_changed),NULL);

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
	printf("changed coord %i, to %.1f,%.1f\n",index,point.x,point.y);
	
}
