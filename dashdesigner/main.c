#include <gauge.h>
#include <getfiles.h>
#include <glib.h>
#include <stdio.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <math.h>
#include <rtv_parser.h>
#include <xml.h>

GladeXML *main_xml = NULL;
gchar *cwd = NULL;

int main (int argc, char ** argv )
{
	GtkWidget *vbox;
	gchar *filename = NULL;
	gchar *tmpbuf = NULL;
	gchar *dirname = NULL;
	GtkWidget *main_window = NULL;

	gtk_init (&argc, &argv);

	main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(main_window),"destroy_event",
			G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(G_OBJECT(main_window),"delete_event",
			G_CALLBACK(gtk_main_quit),NULL);
	gtk_window_set_resizable(GTK_WINDOW(main_window),TRUE);

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
	if (argc == 2)
	{
		tmpbuf = g_get_current_dir();
		dirname = g_path_get_dirname(argv[1]);
		cwd = g_strconcat(tmpbuf,PSEP,dirname,NULL);
		g_free(tmpbuf);
		g_free(dirname);
		if (g_file_test(argv[1],G_FILE_TEST_IS_REGULAR))
			import_dash_xml(argv[1]);

	}

	gtk_main();
	return (0);
}
