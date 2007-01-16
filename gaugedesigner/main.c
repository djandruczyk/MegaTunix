#include "events.h"
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib.h>
#include <stdio.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <math.h>


extern GtkWidget *gauge;
gchar * cwd = NULL;

int main (int argc, char ** argv )
{
	GtkWidget *window;
	GtkWidget *tmp;
	GladeXML *xml = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gchar * dirname = NULL;

	gtk_init (&argc, &argv);

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
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

	tmp = glade_xml_get_widget(xml,"close_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,FALSE);
	if (argc == 2)
	{
		tmpbuf = g_get_current_dir();
		dirname = g_path_get_dirname(argv[1]);
		cwd = g_strconcat(tmpbuf,PSEP,dirname,NULL);
		g_free(tmpbuf);
		g_free(dirname);
		create_new_gauge(tmp,NULL);
		if (g_file_test(argv[1],G_FILE_TEST_IS_REGULAR))
			mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),argv[1]);
	}

	g_free(filename);
	gtk_widget_show_all(window);
	gtk_main();
	return (0);
}
