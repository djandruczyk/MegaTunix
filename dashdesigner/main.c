#include <gauge.h>
#include <getfiles.h>
#include <glib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <math.h>
#include <menu_handlers.h>
#include <rtv_parser.h>
#include <xml.h>

GtkWidget *main_window = NULL;
gchar *cwd = NULL;
GtkBuilder *toplevel = NULL;

int main (int argc, char ** argv )
{
	GtkWidget *vbox;
	gchar *filename = NULL;
	gchar *tmpbuf = NULL;
	gchar *dirname = NULL;
	GError *error = NULL;

	gtk_init (&argc, &argv);

	filename = get_file(g_build_filename(DASHDESIGNER_GLADE_DIR,"main.ui",NULL),NULL);
	if (filename)
	{
		toplevel = gtk_builder_new();
		if(!gtk_builder_add_from_file(toplevel,filename,&error))
		{
			g_warning ("Couldn't load builder file: %s", error->message);
			g_error_free(error);
			exit(-1);
		}
		g_free(filename);
	}
	else
	{
		printf("Can't locate primary ui file!!!!\n");
		exit(-1);
	}
	
	gtk_builder_connect_signals(toplevel,NULL);
	main_window = GTK_WIDGET (gtk_builder_get_object(toplevel,"main_window"));
	menu_setup(toplevel);
	gtk_window_set_resizable(GTK_WINDOW(main_window),TRUE);
        gtk_widget_set_sensitive(OBJ_GET(toplevel,"save_dash_menuitem"),FALSE);
        gtk_widget_set_sensitive(OBJ_GET(toplevel,"save_dash_as_menuitem"),FALSE);
        gtk_widget_set_sensitive(OBJ_GET(toplevel,"close_dash_menuitem"),FALSE);

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
