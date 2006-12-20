#include <defines.h>
#include <getfiles.h>
#include <gauge.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <xml.h>


EXPORT gboolean import_dash_xml(GtkWidget *widget, gpointer data)
{
	printf("Import Dash XML\n");
	return TRUE;
}


EXPORT gboolean export_dash_xml_default(GtkWidget *widget, gpointer data)
{
	printf("Export Dash XML default\n");
	return TRUE;
}


EXPORT gboolean export_dash_xml_as(GtkWidget *widget, gpointer data)
{
	printf("Export Dash XML as...\n");
	return TRUE;
}

