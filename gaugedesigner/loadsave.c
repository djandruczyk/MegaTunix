
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

EXPORT gboolean load_handler(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	extern gchar *cwd;
#ifndef __WIN32__
	gchar * tmpbuf = NULL;
#endif
	gchar * p_dir = NULL;
	if (hold_handlers)
		return TRUE;


	dialog = gtk_file_chooser_dialog_new ("Open File",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	p_dir = g_strconcat(HOME(),PSEP,".MegaTunix",PSEP,GAUGES_DATA_DIR,NULL);
#ifndef __WIN32__
	/* System wide dir */
	/* UNIX */
	tmpbuf = g_strconcat(DATA_DIR,PSEP,GAUGES_DATA_DIR,NULL);
	gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),tmpbuf,NULL);
	if (!cwd)
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),tmpbuf);
	else
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),cwd);
	g_free(tmpbuf);
#else
	/* Windows */
	if (!cwd)
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),p_dir);
	else
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),cwd);
#endif
	/* Personal dir */
	gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),p_dir,NULL);
	g_free(p_dir);
	setup_file_filters(GTK_FILE_CHOOSER(dialog));

#if GTK_MINOR_VERSION >= 6
	if (gtk_minor_version >= 6)
		gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog),TRUE);
#endif
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		if (!gauge)
			create_new_gauge(widget,NULL);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		//printf("loading gauge file %s\n",filename);
		update_attributes();
		update_onscreen_ranges();
		update_onscreen_tblocks();

		g_free (filename);
	}
	if (GTK_IS_WIDGET(dialog))
		gtk_widget_destroy (dialog);
	return TRUE;
}


EXPORT gboolean save_as_handler(GtkWidget *widget, gpointer data)
{
	save_handler(widget,GINT_TO_POINTER(TRUE));
	return TRUE;
}

EXPORT gboolean save_handler(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	gchar * tmpbuf = NULL;
	gchar * filename = NULL;
	gchar *defdir = NULL;
	gchar **vector = NULL;
	extern gchar *cwd;

	if (!MTX_IS_GAUGE_FACE(gauge))
		return FALSE;

	if (hold_handlers)
		return TRUE;

	defdir = g_build_path(PSEP,HOME(), ".MegaTunix",GAUGES_DATA_DIR, NULL);

	dialog = gtk_file_chooser_dialog_new ("Save File",
			NULL,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
			NULL);
	setup_file_filters(GTK_FILE_CHOOSER(dialog));
#if GTK_MINOR_VERSION >= 8
	if (gtk_minor_version >= 8)
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
#endif

	filename = mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(gauge));
	if ((filename != NULL) && ((gint)data == FALSE)) /* Saving PRE-existin*/
	{
		if (g_open(filename,O_RDWR,O_CREAT|O_APPEND) == -1)
		{
			/* Path was sys path, save locally (UNIX ONLY)*/
			tmpbuf = g_strrstr(filename,"Gauges");
			/*get pointer to point after sys path */
			vector = g_strsplit(tmpbuf,PSEP,-1);
			if (g_strv_length(vector) == 3) /* Themed gauge */
			{
				tmpbuf = g_build_path(PSEP,defdir,vector[1],NULL);
				if (!g_file_test(tmpbuf,G_FILE_TEST_IS_DIR))
					g_mkdir(tmpbuf,0755);
				gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),tmpbuf);
				g_free(tmpbuf);
				gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),vector[2]);

			}
			if (g_strv_length(vector) == 2) /* NOT Themed gauge */
			{
				gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),defdir);
				gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),vector[1]);
			}
			g_strfreev(vector);
		}
		else
		{
			if (cwd != NULL)
			{
				vector = g_strsplit(filename,PSEP,-1);
				gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),cwd);
				gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),vector[g_strv_length(vector)-1]);
				g_strfreev(vector);
			}
			else
				gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),filename);
		}
	}
	else	/* NEW Document (Save As) */
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),defdir);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),"New_Gauge.xml");
	}
	g_free(filename);
	g_free(defdir);

#if GTK_MINOR_VERSION >= 6
	if (gtk_minor_version >= 6)
		gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog),TRUE);
#endif
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *tmp;

		tmp = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		mtx_gauge_face_export_xml(MTX_GAUGE_FACE(gauge),tmp);

		g_free (tmp);
	}
	if (GTK_IS_WIDGET(dialog))
		gtk_widget_destroy (dialog);
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
