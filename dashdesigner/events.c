#include <defines.h>
#include <events.h>
#include <gauge.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#ifndef M_PI 
#define M_PI 3.1415926535897932384626433832795 
#endif


EXPORT gboolean dashdesigner_about(GtkWidget * widget, gpointer data)
{
	gchar *authors[] = {"David Andruczyk",NULL};
	gchar *artists[] = {"Dale Anderson",NULL};
	gtk_show_about_dialog(NULL,
			"name","MegaTunix Dashboard Designer",
			"version",VERSION,
			"copyright","David J. Andruczyk(2006)",
			"comments","Dashboard Designer is a tool to design custom Dash gauge layouts for the MegaTunix Megasquirt tuning software",
			"license","GNU GPL v2",
			"website","http://megatunix.sourceforge.net",
			"authors",authors,
			"artists",artists,
			"documenters",authors,
			NULL);
	return TRUE;
}

EXPORT gboolean import_dash_xml(GtkWidget *widget, gpointer data)
{
	printf("Import Dash XML\n");
	return TRUE;
}

EXPORT gboolean add_gauge(GtkWidget *widget, gpointer data)
{
	static gint x=0;
	static gint y=0;
	GtkWidget * gauge = NULL;
	GtkWidget * dash = NULL;
	GladeXML *xml = glade_get_widget_tree(widget);
	dash =  glade_xml_get_widget(xml,"dashboard");
	gauge = mtx_gauge_face_new();
	g_object_set_data (G_OBJECT (gauge), "GB_WIDGET_DATA", "True");
	//gtk_widget_set_size_request(gauge,100,100);
	gtk_fixed_put(GTK_FIXED(dash),gauge,x,y);
	x+=125;
	y+=125;
	mtx_gauge_face_import_xml(gauge,"test.xml");
	gtk_widget_show_all(dash);

	printf("Add Gauge to dash\n");
	return TRUE;
}

EXPORT gboolean dashdesigner_quit(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
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
