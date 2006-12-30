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
	GtkWidget *window;
	GtkWidget *tmp;
	GladeXML *xml = NULL;
	gchar * filename = NULL;

	gtk_init (&argc, &argv);

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

	tmp = glade_xml_get_widget(xml,"save_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,FALSE);

	tmp = glade_xml_get_widget(xml,"save_as_menuitem");
	gtk_widget_set_sensitive(tmp,FALSE);

	g_free(filename);
	gtk_widget_show_all(window);
	gtk_main();
	return (0);
}
