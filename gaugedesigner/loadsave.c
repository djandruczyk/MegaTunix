
#include "../include/defines.h"
#include <events.h>
#include <loadsave.h>
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

extern gboolean hold_handlers;
extern GtkWidget *gauge;

EXPORT gboolean load_handler(GtkWidget *widget, gpointer data)
{
	GladeXML *xml = glade_get_widget_tree(widget);
	GtkWidget *dialog = NULL;
	if (hold_handlers)
		return TRUE;


	dialog = gtk_file_chooser_dialog_new ("Open File",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		if (!gauge)
			create_new_gauge(widget,NULL);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		update_attributes(xml);
		update_onscreen_ranges(widget);
		update_onscreen_tblocks(widget);

		g_free (filename);
	}
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
	if (hold_handlers)
		return TRUE;

	dialog = gtk_file_chooser_dialog_new ("Save File",
			NULL,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
			NULL);
#if GTK_MINOR_VERSION >= 8
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
#endif

	tmpbuf = mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(gauge));
	if ((tmpbuf != NULL) && ((gint)data == FALSE))
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), tmpbuf);
	else
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "NEW_GAUGE_NAME.xml");
	g_free(tmpbuf);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		mtx_gauge_face_export_xml(MTX_GAUGE_FACE(gauge),filename);

		g_free (filename);
	}
	gtk_widget_destroy (dialog);
	return TRUE;

}
