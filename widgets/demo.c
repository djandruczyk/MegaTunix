/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Chris Mire (czb)
 *
 * Megasquirt gauge widget
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
#include <gauge.h>
#include <math.h>

gboolean update_gauge(gpointer );

int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	GtkWidget *gauge = NULL;
	GdkColor  color = { 0, 50000,50000,0};

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gauge = mtx_gauge_face_new ();
	gtk_container_add (GTK_CONTAINER (window), gauge);
	gtk_widget_realize(gauge);
	gtk_widget_show_all (window);

	if (argc < 2)
	{
		printf("Using defaults\n");
		mtx_gauge_face_set_attribute(MTX_GAUGE_FACE(gauge),LBOUND, 0.0);
		mtx_gauge_face_set_attribute(MTX_GAUGE_FACE(gauge),UBOUND, 8000.0);
		mtx_gauge_face_set_value (MTX_GAUGE_FACE (gauge), 0.0);
		mtx_gauge_face_set_attribute(MTX_GAUGE_FACE(gauge),START_ANGLE, 135.0);
		mtx_gauge_face_set_attribute(MTX_GAUGE_FACE(gauge),SWEEP_ANGLE, 270.0);
		mtx_gauge_face_set_attribute(MTX_GAUGE_FACE (gauge), ANTIALIAS, (gfloat)TRUE);
		
		color.red = 50000;
		color.green = 0;
		mtx_gauge_face_set_attribute(MTX_GAUGE_FACE (gauge), PRECISION, (gfloat)1);
	}
	else
	{
		printf("Attempting to load user specified \"%s\"\n",argv[1]);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),argv[1]);
	}
	gtk_timeout_add(20,(GtkFunction)update_gauge,(gpointer)gauge);



	mtx_gauge_face_export_xml(MTX_GAUGE_FACE(gauge),"output2.xml");


	gtk_window_set_decorated(GTK_WINDOW(window),FALSE);
	mtx_gauge_face_set_show_drag_border (MTX_GAUGE_FACE (gauge), TRUE);

	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_main ();
	return 0;
}

gboolean update_gauge(gpointer data)
{
	static gfloat lower = 0.0;
	static gfloat upper = 0.0;
	gfloat cur_val = 0.0;
	gfloat interval = 0.0;
	static gboolean rising = TRUE;

	GtkWidget * gauge = data;
	interval = (upper-lower)/100.0;
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &lower);
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &upper);
	cur_val = mtx_gauge_face_get_value(MTX_GAUGE_FACE (gauge));
	if (cur_val >= upper)
		rising = FALSE;
	if (cur_val <= lower)
		rising = TRUE;

	if (rising)
		cur_val+=interval;
	else
		cur_val-=interval;

	mtx_gauge_face_set_value (MTX_GAUGE_FACE (gauge),cur_val);
	return TRUE;

}
