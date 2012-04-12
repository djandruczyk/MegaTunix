#include <events.h>
#include <getfiles.h>
#include <menu_handlers.h>
#include <gd_init.h>
#include <stdio.h>
#include <stdlib.h>


extern GtkWidget *gauge;
gchar * cwd = NULL;
gboolean direct_path = FALSE;
GtkBuilder *toplevel = NULL;
GtkBuilder *tgroups = NULL;
GtkBuilder *polygons = NULL;
GtkWidget *main_window = NULL;
extern gboolean gauge_loaded;

int main (int argc, char ** argv )
{
	GtkWidget *tmp;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gchar * dirname = NULL;
	GError * error = NULL;
	gchar *pathstub = NULL;

	gtk_init (&argc, &argv);

	pathstub = g_build_filename(GAUGEDESIGNER_GLADE_DIR,"main.ui",NULL);
	filename = get_file(NULL,pathstub,NULL);
	g_free(pathstub);
	if (filename)
	{
		toplevel = gtk_builder_new();
		if(!gtk_builder_add_from_file(toplevel,filename, &error))
		{
			g_warning ("Couldn't load builder file: %s", error->message);
			g_error_free(error);
			exit(-1);
		}
	}
	else
	{
		printf("Can't locate primary ui file!!!!\n");
		exit(-1);
	}
	gtk_builder_connect_signals (toplevel,NULL);
	menu_setup(toplevel);
	main_window = GTK_WIDGET (gtk_builder_get_object(toplevel,"main_window"));

	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_gauge_menuitem"),FALSE);

	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_as_menuitem"),FALSE);

	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"close_gauge_menuitem"),FALSE);

	init_text_attributes(toplevel);
	init_general_attributes(toplevel);
	
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
			gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(toplevel,"tab_notebook")),TRUE);
			update_attributes();

			direct_path = TRUE;
		}
	}
	else
		gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(toplevel,"tab_notebook")),FALSE);

	g_free(filename);
	gtk_main();
	return (0);
}
