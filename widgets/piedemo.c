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
#include <piegauge.h>
#include <math.h>

gboolean update_gauge(gpointer );

int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	GtkWidget *gauge = NULL;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gauge = mtx_pie_gauge_new ();
	gtk_container_add (GTK_CONTAINER (window), gauge);
	/*gtk_widget_realize(gauge);*/
	gtk_widget_show_all (window);

	/*mtx_pie_gauge_set_value(MTX_PIE_GAUGE(gauge), 0.0);*/
	/*mtx_gauge_face_set_attribute(MTX_PIE_GAUGE(gauge),LBOUND, 0.0);*/
	/*mtx_gauge_face_set_attribute(MTX_PIE_GAUGE(gauge),UBOUND, 8000.0);*/
	gtk_timeout_add(20,(GtkFunction)update_gauge,(gpointer)gauge);

	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_main ();
	return 0;
}

gboolean update_gauge(gpointer data)
{
	static gfloat lower = 0.0;
	static gfloat upper = 100.0;
	gfloat cur_val = 0.0;
	gfloat interval = 0.0;
	static gboolean rising = TRUE;

	GtkWidget * gauge = data;
	interval = (upper-lower)/100.0;
	/*mtx_gauge_face_get_attribute(MTX_PIE_GAUGE(gauge), LBOUND, &lower);*/
	/*mtx_gauge_face_get_attribute(MTX_PIE_GAUGE(gauge), UBOUND, &upper);*/
	cur_val = mtx_pie_gauge_get_value(MTX_PIE_GAUGE (gauge));
	if (cur_val >= upper)
		rising = FALSE;
	if (cur_val <= lower)
		rising = TRUE;

	if (rising)
		cur_val+=interval;
	else
		cur_val-=interval;

	mtx_pie_gauge_set_value (MTX_PIE_GAUGE (gauge),cur_val);
	return TRUE;

}
