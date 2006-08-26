// Christopher Mire, 2006

#include <gtk/gtk.h>
#include <gauge.h>
#include <math.h>

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

	gfloat value = mtx_gauge_face_get_value (MTX_GAUGE_FACE (gauge));
	printf ("current value is %f\n", value);

	gtk_main ();
	return 0;
}
