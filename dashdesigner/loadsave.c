
#include "../include/defines.h"
#include <events.h>
#include <loadsave.h>
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <xml.h>

extern GtkWidget *main_window;
extern gboolean changed;

void prompt_to_save()
{
	GtkWidget *dialog = NULL;
	extern GtkWidget *main_window;
	gint result = 0;
	dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),GTK_DIALOG_MODAL,GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,"This dashboard has been modified, Save it or discard the changes?");
	gtk_dialog_add_button(GTK_DIALOG(dialog),"Save Dashboard", GTK_RESPONSE_OK);
	gtk_dialog_add_button(GTK_DIALOG(dialog),"Discard Changes", GTK_RESPONSE_CANCEL);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	switch (result)
	{
		case GTK_RESPONSE_NONE:
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			save_handler(NULL,GINT_TO_POINTER(TRUE));
			break;
	}
	return;
}


EXPORT gboolean load_handler(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	extern GtkBuilder *toplevel;

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
		changed = FALSE;
	        gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(toplevel,"save_dash_menuitem")),TRUE);
	        gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(toplevel,"save_dash_as_menuitem")),TRUE);
	        gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(toplevel,"close_dash_menuitem")),TRUE);
	        gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(toplevel,"load_dash_menuitem")),FALSE);
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
	gchar *filename = NULL;
	gboolean result = FALSE;
	GtkWidget *dash;
	extern GtkBuilder *toplevel;


	dash = GTK_WIDGET(gtk_builder_get_object(toplevel,"dashboard"));
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
		changed = FALSE;
	        gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(toplevel,"save_dash_menuitem")),FALSE);
	        gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(toplevel,"save_dash_as_menuitem")),FALSE);
	        gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(toplevel,"close_dash_menuitem")),TRUE);
	        gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(toplevel,"load_dash_menuitem")),TRUE);
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
	guint i = 0;
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
					"At least one gauge has no datasource set, PLEASE fix and re-attempt to save\n");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			return FALSE;
		}

	}
	return TRUE;
}
