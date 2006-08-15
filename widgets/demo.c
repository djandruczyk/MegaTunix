// Christopher Mire, 2006

#include <gtk/gtk.h>
#include <gauge.h>

int
main (int argc, char **argv)
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

	mtx_gauge_face_set_value (MTX_GAUGE_FACE (gauge), 0.5);
	float value = mtx_gauge_face_get_value (MTX_GAUGE_FACE (gauge));
	printf ("UD is %f\n", value);


	gtk_main ();
}



GtkWidget *window;
