#include "events.h"
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib.h>
#include <stdio.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <math.h>


int main (int argc, char ** argv )
{
	extern GtkWidget *gauge;
	GtkWidget *window;
	GtkWidget *widget;
	GladeXML *xml = NULL;
	gchar * filename = NULL;

	gtk_init (&argc, &argv);

	//window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//g_signal_connect(G_OBJECT(window),"delete_event", G_CALLBACK(gtk_main_quit),NULL);
	//g_signal_connect(G_OBJECT(window),"destroy_event", G_CALLBACK(gtk_main_quit),NULL);

	filename = get_file(g_build_filename(GAUGE_DATA_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
		xml = glade_xml_new(filename, "main_window", NULL);
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}
	glade_xml_signal_autoconnect(xml);
	window = glade_xml_get_widget(xml,"main_window");
	//gtk_container_add(GTK_CONTAINER(window),widget);
	gtk_widget_show_all(window);


	if (argc > 1)  /* User specified xml file */
	{
		widget = glade_xml_get_widget(xml,"import_button");
		create_new_gauge(widget,NULL);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),argv[1]);
		update_attributes();
		gtk_widget_set_sensitive(widget,TRUE);
	}


	g_free(filename);
	gtk_main();
	return (0);
}
