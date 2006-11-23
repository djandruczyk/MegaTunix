#include <defines.h>
#include <events.h>
#include <gauge.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#ifndef M_PI 
#define M_PI 3.1415926535897932384626433832795 
#endif

static gint child_x = 0;
static gint child_y = 0;
static gboolean grabbed = FALSE;
static GtkWidget *dragged_widget = NULL;

EXPORT gboolean dashdesigner_about(GtkWidget * widget, gpointer data)
{
#if GTK_MINOR_VERSION >= 8
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
#endif
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

EXPORT gboolean motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	gint x_cur;
	gint y_cur;
	gint origin_x;
	gint origin_y;

	gdk_window_get_origin(widget->window,&origin_x,&origin_y);
	/* Current cursor locatio nrelatuive to upper left corner */
	x_cur = (gint)event->x_root-origin_x;
	y_cur = (gint)event->y_root-origin_y;

	printf("motion event\n");
	if (grabbed)
	{
		printf("event occurred at EVENT %i,%i, REL %i,%i, ABS %i,%i\n",(gint)event->x, (gint)event->y,x_cur,y_cur,(gint)event->x_root,(gint)event->y_root);
		gtk_fixed_move(GTK_FIXED(dragged_widget->parent),dragged_widget,x_cur-child_x,y_cur-child_y);
	}

	return TRUE;
}



EXPORT gboolean button_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkWidget * fixed = gtk_bin_get_child(GTK_BIN(widget));
	GList *list = NULL;
	gint i = 0;
	gint origin_x = 0;
	gint origin_y = 0;
	gboolean found_one = FALSE;
	static gint x_last = 0;
	static gint y_last = 0;
	gint x = 0;
	gint y = 0;
	gint x_cur = 0;
	gint y_cur = 0;
	gint child_x_origin = 0;
	gint child_y_origin = 0;
	gint child_width = 0;
	gint child_height = 0;
	GtkFixedChild * fchild = NULL;
	gint len = g_list_length(GTK_FIXED(fixed)->children);

	if (event->state & GDK_BUTTON1_MASK)
	{
		grabbed = FALSE;
		printf("button1 released, unlocking\n");
		return TRUE;
	}

	gdk_window_get_origin(widget->window,&origin_x,&origin_y);
	/* Current cursor locatio nrelatuive to upper left corner */
	x_cur = (gint)event->x_root-origin_x;
	y_cur = (gint)event->y_root-origin_y;

	if (event->button == 1)
	{
		list = g_list_first(GTK_FIXED(fixed)->children);
		for (i=0;i<len;i++)
		{
			fchild = (GtkFixedChild *)  g_list_nth_data(list,i);

			child_x_origin = fchild->widget->allocation.x;
			child_y_origin = fchild->widget->allocation.y;
			child_width = fchild->widget->allocation.width;
			child_height = fchild->widget->allocation.height;

					printf("Gauge %i is at %i,%i, w/h %i,%i\n",i, child_x_origin,child_y_origin, child_width,child_height);
			if ((x_cur > child_x_origin) && (x_cur < (child_x_origin+child_width)) && (y_cur > child_y_origin) && (y_cur < (child_y_origin+child_height))) 
			{
				printf("clicked in a gauge\n");
				found_one = TRUE;
				break;
			}
			else
			{
				printf("clicked elsewhere\n");
				found_one = FALSE;
			}
		}
		if (found_one)
		{
			if (event->button == 1)
			{
				printf("grabbed it \n");
				grabbed = TRUE;
				dragged_widget = fchild->widget;
				child_x = (gint)event->x;
				child_y = (gint)event->y;
			}
			else
			{
				printf("didn't grab squat\n");
				grabbed = FALSE;
			}
		}
	}



	
	printf("\n\n");

	return TRUE;

}
