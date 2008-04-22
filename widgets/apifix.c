/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Chris Mire (czb)
 *
 * Megasquirt gauge widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */


#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <gauge.h>
#include <math.h>

GtkWidget *gauge = NULL;
void fix_file (const gchar * file);
void traverse (const gchar * file);

int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	gchar * topdir = NULL;

	if (argc != 2)
		exit(-1);

	gtk_init (&argc, &argv);
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gauge = mtx_gauge_face_new ();
	gtk_container_add (GTK_CONTAINER (window), gauge);
	gtk_widget_realize(gauge);
	gtk_widget_show_all (window);

	printf("Attempting to repair gauges starting from: \"%s\"\n",argv[1]);
	topdir = argv[1];

	if (g_file_test(topdir,G_FILE_TEST_IS_DIR))
		printf("Top dir %s exists and is valid\n",topdir);
	else
	{
		printf("Top dir %s is invalid, exiting!\n",topdir);
		exit(-1);
	}

	traverse(topdir);


	gtk_window_set_decorated(GTK_WINDOW(window),FALSE);
	mtx_gauge_face_set_show_drag_border (MTX_GAUGE_FACE (gauge), TRUE);

	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_main ();
	return 0;
}

void traverse(const gchar * top)
{
	GDir *dir = NULL;
	const gchar *entry = NULL;
	gchar * filename = NULL;

	dir = g_dir_open(top,0,NULL);
	if (!dir)
	{
		printf("can't open %s for reading\n",top);
		return;
	}
	entry = g_dir_read_name(dir);
	while (entry != NULL)
	{
		
		filename = g_build_filename(top,entry,NULL);
		if (g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		{
			if (g_strrstr(filename,".xml"))
			{
				printf("fixing %s\n",filename);
				fix_file(filename);
			}
		}
		else if (g_file_test(filename,G_FILE_TEST_IS_DIR))
			traverse(filename);

		entry = g_dir_read_name(dir);
		g_free(filename);
	}
	g_dir_close(dir);
	return;
}


void fix_file (const gchar * file)
{
	gchar * newfile = NULL;
	mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),(gchar *)file);
	newfile = g_strdup_printf("%s_new",file);
	mtx_gauge_face_export_xml(MTX_GAUGE_FACE(gauge),(gchar *) file);
	g_free(newfile);
	return;
}
