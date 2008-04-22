
#include "../include/defines.h"
#include <events.h>
#include <loadsave.h>
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <xml.h>


EXPORT gboolean load_handler(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("Dashboards");
	fileio->title = g_strdup("Select Dashboard to Open");
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->shortcut_folders = g_strdup("Dashboards");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;


	filename = choose_file(fileio);
	if (filename)
	{
		import_dash_xml(filename);
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
	MtxFileIO *fileio = NULL;
	extern GladeXML *main_xml;
	gchar *filename = NULL;
	gboolean result = FALSE;
	GtkWidget *dash;


	dash = glade_xml_get_widget(main_xml,"dashboard");
	result = check_datasources_set(dash);
	if (!result)
		return FALSE;


	filename = OBJ_GET((dash),"dash_xml_filename");

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("Dashboards");
	fileio->filename = filename;
	fileio->title = g_strdup("Save Dashboard to File");
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->default_filename = g_strdup("Untitled-Dash.xml");
	fileio->default_extension = g_strdup("xml");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;


	filename = choose_file(fileio);
	if (filename)
	{
		export_dash_xml(filename);
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


gboolean check_datasources_set(GtkWidget *dash)
{
	gint i = 0;
	gboolean state = FALSE;
	GtkTreeIter iter;
	GtkFixedChild *child = NULL;
	GtkWidget *dialog = NULL;

	for (i=0;i<g_list_length(GTK_FIXED(dash)->children);i++)
	{
		child = g_list_nth_data(GTK_FIXED(dash)->children,i);
		state = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(OBJ_GET((child->widget),"combo")),&iter);
		if (!state)
		{
			dialog = gtk_message_dialog_new(NULL,
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"At least one gauge has no datasrouce set, PLEASE fix and re-attempt to save\n");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			return FALSE;
		}

	}
	return TRUE;
}
