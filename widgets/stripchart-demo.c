/*
 * Copyright (C) 2010 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix stripchart widget
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
#include <stripchart.h>
#include <math.h>

void update_stripchart(gpointer data);

int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	GtkWidget *chart = NULL;
	GdkColor col = { 0,100,200,300};
	gint i = 0;
	gint trace1 = 0;
	gint trace2 = 0;
	gint trace3 = 0;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

//	gtk_widget_set_size_request(GTK_WIDGET(window),320,320);
	chart = mtx_stripchart_new ();
	gtk_container_add (GTK_CONTAINER (window), chart);
	gtk_widget_realize(chart);
	trace1 = mtx_stripchart_add_trace(MTX_STRIPCHART(chart),0.0,255.0,0,"Trace 1", &col);
	printf("trace 1's ID %i\n",trace1);
	trace2 = mtx_stripchart_add_trace(MTX_STRIPCHART(chart),255.0,1024.0,0,"Trace 2", NULL);
	printf("trace 2's ID %i\n",trace2);
	trace3 = mtx_stripchart_add_trace(MTX_STRIPCHART(chart),0.0,65535.0,1,"Trace 3", NULL);
	printf("trace 3's ID %i\n",trace3);
	mtx_stripchart_delete_trace(MTX_STRIPCHART(chart),trace2);
	

	//gtk_timeout_add(40,(GtkFunction)update_stripchart,(gpointer)chart);

	gtk_widget_show_all (window);


	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_main ();
	return 0;
}

void update_stripchart(gpointer data)
{
	GtkWidget *chart = data;
	gint min = -1000;
	gint max = 11000;
	static gint step = 125;
	static gboolean rising = TRUE;
	static gint value = 0;

	if (value >= max)
		rising = FALSE;
	if (value <= min)
		rising = TRUE;

	if (rising)
		value+=step;
	else
		value-=step;
/*printf("Setting x marker to %i\n",value);*/
}
