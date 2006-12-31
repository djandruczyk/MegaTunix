#include <gauge.h>
#include <getfiles.h>
#include <glib.h>
#include <stdio.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <math.h>
#include <rtv_parser.h>

GladeXML *main_xml = NULL;
GtkWidget *main_window = NULL;
int main (int argc, char ** argv )
{
	GtkWidget *vbox;
	gchar *filename = NULL;

	gtk_init (&argc, &argv);

	main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(main_window),"destroy_event",
			G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(G_OBJECT(main_window),"delete_event",
			G_CALLBACK(gtk_main_quit),NULL);


	gtk_widget_set_size_request(main_window,320,240);
	filename = get_file(g_build_filename(DASHDESIGNER_GLADE_DIR,"dashdesigner.glade",NULL),NULL);
	if (filename)
		main_xml = glade_xml_new(filename, "main_vbox", NULL);
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}
	g_free(filename);
	
	glade_xml_signal_autoconnect(main_xml);
	vbox = glade_xml_get_widget(main_xml, "main_vbox");

	/* Bind the appropriate handlers */

	gtk_container_add(GTK_CONTAINER(main_window),vbox);
	gtk_widget_show_all(vbox);

	retrieve_rt_vars();
	gtk_widget_show_all(main_window);

	gtk_main();
	return (0);
}
