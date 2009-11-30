#include "events.h"
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib.h>
#include <gd_init.h>
#include <stdio.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <math.h>


extern GtkWidget *gauge;
gchar * cwd = NULL;
gboolean direct_path = FALSE;
GladeXML *topxml = NULL;
GtkWidget *main_window = NULL;
extern gboolean gauge_loaded;

int main (int argc, char ** argv )
{
	GtkWidget *tmp;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gchar * dirname = NULL;

	gtk_init (&argc, &argv);

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
		topxml = glade_xml_new(filename, "main_window", NULL);
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}
	glade_xml_signal_autoconnect(topxml);
	main_window = glade_xml_get_widget(topxml,"main_window");

	gtk_widget_set_sensitive(glade_xml_get_widget(topxml,"save_gauge_menuitem"),FALSE);

	gtk_widget_set_sensitive(glade_xml_get_widget(topxml,"save_as_menuitem"),FALSE);

	gtk_widget_set_sensitive(glade_xml_get_widget(topxml,"close_gauge_menuitem"),FALSE);

	init_text_attributes(topxml);
	init_general_attributes(topxml);
	
	gtk_widget_show_all(main_window);
	if (argc == 2)
	{
		tmpbuf = g_get_current_dir();
		dirname = g_path_get_dirname(argv[1]);
		cwd = g_strconcat(tmpbuf,PSEP,dirname,NULL);
		g_free(tmpbuf);
		g_free(dirname);
		create_new_gauge(main_window,NULL);
		if (g_file_test(argv[1],G_FILE_TEST_IS_REGULAR))
		{
			mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),argv[1]);
			gauge_loaded = TRUE;
			gtk_widget_set_sensitive(glade_xml_get_widget(topxml,"tab_notebook"),TRUE);
			update_attributes();

			direct_path = TRUE;
		}
	}
	else
		gtk_widget_set_sensitive(glade_xml_get_widget(topxml,"tab_notebook"),FALSE);

	g_free(filename);
	gtk_main();
	return (0);
}
