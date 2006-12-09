#include <gauge.h>
#include <getfiles.h>
#include <glib.h>
#include <stdio.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <math.h>

int main (int argc, char ** argv )
{
	GtkWidget *window;
	GtkWidget *vbox;
	GladeXML *xml = NULL;
	gchar *filename = NULL;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window),"destroy_event",
			G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(gtk_main_quit),NULL);


	filename = get_file(g_build_filename(DASH_DATA_DIR,"dashdesigner.glade",NULL),NULL);
	if (filename)
		xml = glade_xml_new(filename, "main_vbox", NULL);
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}
	g_free(filename);
	
	glade_xml_signal_autoconnect(xml);
	vbox = glade_xml_get_widget(xml, "main_vbox");

	/* Bind the appropriate handlers */
//	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"dashboard")),"GB_WIDGET_DATA","True");

	gtk_container_add(GTK_CONTAINER(window),vbox);

	gtk_widget_show_all(window);

	gtk_main();
	return (0);
}
