// Christopher Mire, 2006

#include <gtk/gtk.h>
#include <gauge.h>
#include <math.h>

gboolean update_gauge(gpointer );

int main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *gauge;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gauge = mtx_gauge_face_new ();
	gtk_container_add (GTK_CONTAINER (window), gauge);

	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_widget_show_all (window);
	mtx_gauge_face_set_bounds (MTX_GAUGE_FACE (gauge), -13.0, 21.0);
	mtx_gauge_face_set_value (MTX_GAUGE_FACE (gauge), 4.0);
	mtx_gauge_face_set_span (MTX_GAUGE_FACE (gauge), 1.5 * M_PI, 3 * M_PI);
	mtx_gauge_face_set_resolution (MTX_GAUGE_FACE (gauge), 36);
	gtk_timeout_add(10,(GtkFunction)update_gauge,(gpointer)gauge);


	gfloat value = mtx_gauge_face_get_value (MTX_GAUGE_FACE (gauge));
	printf ("current value is %f\n", value);

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
	mtx_gauge_face_get_bounds(MTX_GAUGE_FACE (gauge),&lower,&upper);
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
