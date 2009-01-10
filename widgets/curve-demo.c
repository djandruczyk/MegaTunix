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


int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	GtkWidget *curve = NULL;
	GdkPoint points[5];
	gint i = 0;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	curve = mtx_curve_new ();
	gtk_container_add (GTK_CONTAINER (window), curve);
	for (i=0;i<5;i++)
	{
		points[i].x=i*10;
		points[i].y=i*10;
	}
	gtk_widget_realize(curve);
	mtx_curve_set_points(MTX_CURVE(curve),5,points);

	gtk_widget_show_all (window);


	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_main ();
	return 0;
}

