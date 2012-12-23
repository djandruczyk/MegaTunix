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

/*!
  \file widgets/stripchart-demo.c
  \brief MtxStripChart demo application
  \author David Andruczyk
  */

#include <defines.h>
#include <stripchart.h>
#include <stdio.h>

gboolean update_stripchart_wrapper(gpointer data);
gboolean update_stripchart(gpointer data);
gboolean remove_trace(gpointer data);
gboolean close_demo(GtkWidget *, gpointer);
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
	gint timeout = 0;

	g_thread_init(NULL);
	gdk_threads_init();
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
		/*printf("setting values to %f, %f, %f\n",data[0],data[1],data[2]); */

		mtx_stripchart_set_values(MTX_STRIPCHART(chart),data);
	}
	mtx_stripchart_get_latest_values(MTX_STRIPCHART(chart),data);
/*	printf("latest values are %f, %f, %f\n",data[0],data[1],data[2]);*/

	/*mtx_stripchart_delete_trace(MTX_STRIPCHART(chart),trace2);*/

	timeout = g_timeout_add(40,(GSourceFunc)update_stripchart_wrapper,(gpointer)chart);
/*	g_timeout_add(4000,(GSourceFunc)remove_trace,GINT_TO_POINTER(trace2));*/

	gtk_widget_show_all (window);

	g_signal_connect (window, "delete_event",
			G_CALLBACK (close_demo), GINT_TO_POINTER(timeout));
	g_signal_connect (window, "destroy_event",
			G_CALLBACK (close_demo), GINT_TO_POINTER(timeout));

	gdk_threads_enter();
	gtk_main ();
	gdk_threads_leave();
	return 0;
}

gboolean update_stripchart_wrapper(gpointer data)
{
	g_idle_add(update_stripchart,data);
	return TRUE;
}

gboolean update_stripchart(gpointer data)
{
	GtkWidget *chart = (GtkWidget *)data;
	gint min = -1000;
	gint max = 11000;
	gint i = 0;
	static gfloat vals[3] = {0.0,0.0,0.0};
	static gfloat **hist_vals = NULL;

	if (!hist_vals)
	{
		hist_vals = g_new0(gfloat *, 3);
		for (i=0;i<3;i++)
			hist_vals[i] = g_new0(gfloat, 3);
	}
	/* Simple one at a time */
	vals[0]++;
	vals[1]+=1.25;
	vals[2]+=2.125;
	hist_vals[0][0]++;
	hist_vals[0][1]++;
	hist_vals[0][2]++;
	hist_vals[1][0]+=1.25;
	hist_vals[1][1]+=1.25;
	hist_vals[1][2]+=1.25;
	hist_vals[2][0]+=2.125;
	hist_vals[2][1]+=2.125;
	hist_vals[2][2]+=2.125;
	mtx_stripchart_set_n_values(MTX_STRIPCHART(chart),3,hist_vals);
/*	printf("This should scroll stripchart \n"); */

	return FALSE;
}

gboolean remove_trace(gpointer data)
{
	mtx_stripchart_delete_trace(MTX_STRIPCHART(chart),(GINT)data);
	return FALSE;
}


gboolean close_demo(GtkWidget *widget, gpointer data)
{
	g_source_remove((guint)data);
	gtk_widget_destroy(widget);
	gtk_main_quit();
	return TRUE;
}
