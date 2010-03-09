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

gboolean update_stripchart(gpointer data);
gboolean remove_trace(gpointer data);
GtkWidget *chart = NULL;

int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	GdkColor col = { 0,100,200,300};
	gint i = 0;
	gfloat j = 0.0;
	gint trace1 = 0;
	gint trace2 = 0;
	gint trace3 = 0;
	gfloat data[3] = {0.1,1.1,2.2};

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_size_request(GTK_WIDGET(window),320,320);
	chart = mtx_stripchart_new ();
	gtk_container_add (GTK_CONTAINER (window), chart);
	gtk_widget_realize(chart);
	trace1 = mtx_stripchart_add_trace(MTX_STRIPCHART(chart),-512.0,1536.0,0,"Trace 1", NULL);
	printf("trace 1's ID %i\n",trace1);
	trace2 = mtx_stripchart_add_trace(MTX_STRIPCHART(chart),-100.0,512.0,0,"Trace 2", NULL);
	printf("trace 2's ID %i\n",trace2);
	trace3 = mtx_stripchart_add_trace(MTX_STRIPCHART(chart),512.0,1024.0,1,"Trace 3", NULL);
	printf("trace 3's ID %i\n",trace3);
	for (j=0;j<1024;j+=2.5)
	{
		for (i=0;i<3;i++)
			data[i] = j;
		//printf("setting values to %f, %f, %f\n",data[0],data[1],data[2]);
		mtx_stripchart_set_values(MTX_STRIPCHART(chart),data);
	}
	mtx_stripchart_get_latest_values(MTX_STRIPCHART(chart),data);
//	printf("latest values are %f, %f, %f\n",data[0],data[1],data[2]);

	//mtx_stripchart_delete_trace(MTX_STRIPCHART(chart),trace2);
	

	gtk_timeout_add(40,(GtkFunction)update_stripchart,(gpointer)chart);
//	gtk_timeout_add(4000,(GtkFunction)remove_trace,GINT_TO_POINTER(trace2));

	gtk_widget_show_all (window);


	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_main ();
	return 0;
}

gboolean update_stripchart(gpointer data)
{
	GtkWidget *chart = data;
	gint min = -1000;
	gint max = 11000;
	static gfloat vals[3] = {0.0,0.0,0.0};
	vals[0]++;
	vals[1]+=1.25;
	vals[2]+=2.125;
	mtx_stripchart_set_values(MTX_STRIPCHART(chart),vals);
//	printf("This should scroll stripchart \n");
	return TRUE;
}

gboolean remove_trace(gpointer data)
{
	mtx_stripchart_delete_trace(MTX_STRIPCHART(chart),(gint)data);
	return FALSE;
}
