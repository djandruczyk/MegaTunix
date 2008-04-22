
#include "../include/defines.h"
#include <events.h>
#include <loadsave.h>
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


extern gboolean hold_handlers;
extern GtkWidget *gauge;
extern gboolean direct_path;

EXPORT gboolean load_handler(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	extern gchar *cwd;

	if (hold_handlers)
		return TRUE;

	fileio = g_new0(MtxFileIO ,1);
	if (cwd)
		fileio->absolute_path = g_strdup(cwd);
	else
		fileio->default_path = g_strdup("Gauges");
	fileio->title = g_strdup("Select Gauge to Open");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->shortcut_folders = g_strdup("Gauges");

	filename = choose_file(fileio);
	if (filename)
	{
		if (!gauge)
			create_new_gauge(widget,NULL);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		/*printf("loading gauge file %s\n",filename);*/
		update_attributes();
		g_free (filename);
	}
	free_mtxfileio(fileio);
	return TRUE;
}


EXPORT gboolean save_as_handler(GtkWidget *widget, gpointer data)
{
	save_handler(widget,GINT_TO_POINTER(TRUE));
	return TRUE;
}

EXPORT gboolean save_handler(GtkWidget *widget, gpointer data)
{
	gchar * tmpbuf = NULL;
	gchar * filename = NULL;
	gchar *defdir = NULL;
	gchar **vector = NULL;
	gint len = 0;
	MtxFileIO *fileio = NULL;

	if (!MTX_IS_GAUGE_FACE(gauge))
		return FALSE;

	if (hold_handlers)
		return TRUE;

	defdir = g_build_path(PSEP,HOME(), ".MegaTunix",GAUGES_DATA_DIR, NULL);


	fileio = g_new0(MtxFileIO ,1);
	fileio->title = g_strdup("Save Dashboard to File");
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->default_filename = g_strdup("Untitled-Gauge.xml");
	fileio->default_extension = g_strdup("xml");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(gauge));
	if (!filename)
	{
		fileio->filename = NULL;
		fileio->default_path = g_strdup(GAUGES_DATA_DIR);
	}
	else
	{
		if (direct_path)
		{
			fileio->filename = g_build_path(PSEP,g_get_current_dir(),filename,NULL);
			vector = g_strsplit(filename,PSEP,-1);
			len = g_strv_length(vector);
			g_free(vector[len-1]);
			vector[len-1] = NULL;
			tmpbuf = g_strjoinv(PSEP,vector);
			fileio->default_path = g_build_path(PSEP,g_get_current_dir(),tmpbuf,NULL);
			g_free(tmpbuf);

			g_strfreev(vector);
		}
		else
		{

			fileio->filename = filename;
			tmpbuf = g_strrstr(filename,"Gauges");
			vector = g_strsplit(tmpbuf,PSEP,-1);
			if (g_strv_length(vector) == 3) /* Themed gauge */
				fileio->default_path = g_build_path(PSEP,GAUGES_DATA_DIR,vector[1],NULL);
			else if (g_strv_length(vector) == 2)
				fileio->default_path = g_strdup(GAUGES_DATA_DIR);
			g_strfreev(vector);
		}
	}

	filename = choose_file(fileio);
	if (filename)
	{
		mtx_gauge_face_export_xml(MTX_GAUGE_FACE(gauge),filename);
		g_free (filename);
	}
	free_mtxfileio(fileio);
	return TRUE;

}

void setup_file_filters(GtkFileChooser *chooser)
{
	GtkFileFilter * filter = NULL;
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter),"*.*");
	gtk_file_filter_set_name(GTK_FILE_FILTER(filter),"All Files");
	gtk_file_chooser_add_filter(chooser,filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter),"*.xml");
	gtk_file_filter_set_name(GTK_FILE_FILTER(filter),"XML Files");
	gtk_file_chooser_add_filter(chooser,filter);
	gtk_file_chooser_set_filter(chooser,filter);
	return ;
}
