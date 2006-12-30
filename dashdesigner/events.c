#include <defines.h>
#include <events.h>
#include <getfiles.h>
#include <gauge.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#ifndef M_PI 
#define M_PI 3.1415926535897932384626433832795 
#endif

static gboolean grabbed = FALSE;
static gboolean resizing = FALSE;
static gboolean moving = FALSE;
static gint corner = -1;
static GladeXML *prop_xml = NULL;
static GtkWidget *grabbed_widget = NULL;
static struct T
{
	gint child_x_origin;
	gint child_y_origin;
	gint child_width;
	gint child_height;
	gint rel_grab_x;
	gint rel_grab_y;
}tt = {0,0,0,0};

typedef enum 
{
	LL = 10,
	LR,
	UL,
	UR,
}CornerType;

EXPORT gboolean dashdesigner_about(GtkWidget * widget, gpointer data)
{
#if GTK_MINOR_VERSION >= 8
	if (gtk_minor_version >= 8)
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
	}
#endif
	return TRUE;
}


EXPORT gboolean create_preview_list(GtkWidget *widget, gpointer data)
{

	static gboolean created = FALSE;
	static gboolean prop_created = FALSE;
	static GladeXML *preview_xml = NULL;
	GtkWidget * gauge = NULL;
	GtkWidget * table = NULL;
	GtkWidget * window = NULL;
	gchar ** files = NULL;
	extern GladeXML *main_xml;
	gint i = 0;
	if (created)
	{
		window = glade_xml_get_widget(preview_xml,"preview_window");
		if (GTK_IS_WIDGET(window))
			gtk_widget_show_all(window);
	}
	else
	{
		preview_xml = glade_xml_new(main_xml->filename, "preview_window", NULL);
		glade_xml_signal_autoconnect(preview_xml);
	}

	if (prop_created)
	{
		printf("window created, tring to show it\n");
		window = glade_xml_get_widget(prop_xml,"property_editor_window");
		if (GTK_IS_WIDGET(window))
			gtk_widget_show_all(window);
		else
			printf("widget clobbered\n");
	}
	else
	{
		prop_xml = glade_xml_new(main_xml->filename, "property_editor_window", NULL);
		glade_xml_signal_autoconnect(prop_xml);
	}
	if (created && prop_created)
		return TRUE;


	table = glade_xml_get_widget(preview_xml,"gauge_preview_table");
	files = get_files(g_strconcat(GAUGES_DIR,PSEP,NULL),g_strdup("xml"));
	if (files)
	{
		while (files[i])
		{
			gauge = mtx_gauge_face_new();
			gtk_table_attach_defaults(GTK_TABLE(table),gauge,0,1,i,i+1);
			gtk_widget_realize(gauge);
			mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),files[i]);
			gtk_widget_set_usize(GTK_WIDGET(gauge),150,150);
			i++;
		}
		gtk_widget_show_all(table);
		g_strfreev(files);
	}
	created = TRUE;
	prop_created = TRUE;
	return TRUE;

}

EXPORT gboolean gauge_choice_button_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkWidget *table = NULL;
	gint i = 0;
	gint x_cur = 0;
	gint y_cur = 0;
	gint width = 0;
	gint height = 0;
	gint origin_x = 0;
	gint origin_y = 0;
	gint total_gauges = 0;
	GtkWidget * dash = NULL;
	GtkWidget * gauge = NULL;
	GtkTableChild *child = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gint row = 0;
	extern GladeXML *main_xml;

	gdk_window_get_origin(widget->window,&origin_x,&origin_y);
	gdk_drawable_get_size(widget->window,&width,&height);
	
	/* Current cursor location relatuive to upper left corner */
	x_cur = (gint)event->x_root-origin_x;
	y_cur = (gint)event->y_root-origin_y;

	table = gtk_bin_get_child(GTK_BIN(widget));
	if(GTK_IS_TABLE(table))
		total_gauges = g_list_length(GTK_TABLE(table)->children);

	row = (gint)(((float)y_cur/(float)height)*(float)total_gauges);

	if (event->button == 1)
	{
		for (i=0;i<total_gauges;i++)
		{
			child = (GtkTableChild *) g_list_nth_data(GTK_TABLE(table)->children,i);
			if (row == child->top_attach)
				tmpbuf = mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(child->widget));
		}
		dash =  glade_xml_get_widget(main_xml,"dashboard");
		gauge = mtx_gauge_face_new();
		gtk_fixed_put(GTK_FIXED(dash),gauge,130,130);
		filename = get_file(g_strconcat(GAUGES_DIR,PSEP,tmpbuf,NULL),NULL);
		g_free(tmpbuf);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		gtk_widget_show_all(dash);
		g_free(filename);
		update_properties(gauge,GAUGE_ADD);
	}

//	printf("button event in gauge choice window at %i,%i\n",x_cur,y_cur);
	return TRUE;

}


EXPORT gboolean dashdesigner_quit(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
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


	//printf("motion event\n");
//	printf("rel movement point %i,%i\n",x_cur,y_cur);
	if (grabbed)
	{
		if (moving)
		{
			gtk_fixed_move(GTK_FIXED(grabbed_widget->parent),grabbed_widget,
					x_cur-tt.rel_grab_x+tt.child_x_origin,
					y_cur-tt.rel_grab_y+tt.child_y_origin);
		}
		if (resizing)
		{
			if (corner == LR)
			{
				gtk_widget_set_usize(grabbed_widget,event->x,event->y);
			}
			else if (corner == UR)
			{
				gtk_widget_set_usize(grabbed_widget,
						x_cur-tt.child_x_origin,
						(tt.child_y_origin+tt.child_height)-y_cur);

				gtk_fixed_move(GTK_FIXED(grabbed_widget->parent),
						grabbed_widget,
						tt.child_x_origin,
						y_cur);
			}
			else if (corner == UL)
			{
				gtk_widget_set_usize(grabbed_widget,
						(tt.child_x_origin+tt.child_width)-x_cur,
						(tt.child_y_origin+tt.child_height)-y_cur);

				gtk_fixed_move(GTK_FIXED(grabbed_widget->parent),
						grabbed_widget,
						x_cur,
						y_cur);
			}
			else if (corner == LL)
			{
				gtk_widget_set_usize(grabbed_widget,
						(tt.child_x_origin+tt.child_width)-x_cur,event->y);

				gtk_fixed_move(GTK_FIXED(grabbed_widget->parent),
						grabbed_widget,
						x_cur,
						tt.child_y_origin);
			}
		}
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
	gint x_cur = 0;
	gint y_cur = 0;
	GtkFixedChild * fchild = NULL;
	GdkWindow *win = NULL;
	gint len = g_list_length(GTK_FIXED(fixed)->children);

	if (event->state & GDK_BUTTON1_MASK)
	{
		grabbed = FALSE;
		moving = FALSE;
		resizing = FALSE;
		corner = -1;
		//printf("button1 released, unlocking\n");
		return TRUE;
	}

	gdk_window_get_origin(widget->window,&origin_x,&origin_y);
	/* Current cursor locatio nrelatuive to upper left corner */
	x_cur = (gint)event->x_root-origin_x;
	y_cur = (gint)event->y_root-origin_y;

	if ((event->button))
	{
		win = gdk_window_at_pointer(NULL,NULL);
		list = g_list_first(GTK_FIXED(fixed)->children);
		found_one = FALSE;
		for (i=0;i<len;i++)
		{
			fchild = (GtkFixedChild *)  g_list_nth_data(list,i);

			if (win == fchild->widget->window)
			{
				tt.child_x_origin = fchild->widget->allocation.x;
				tt.child_y_origin = fchild->widget->allocation.y;
				tt.child_width = fchild->widget->allocation.width;
				tt.child_height = fchild->widget->allocation.height;

				raise_fixed_child(fchild->widget);
				found_one = TRUE;
				mtx_gauge_face_set_show_drag_border(MTX_GAUGE_FACE(fchild->widget),TRUE);
				grabbed_widget = fchild->widget;

			}
			else
			{
				mtx_gauge_face_set_show_drag_border(MTX_GAUGE_FACE(fchild->widget),FALSE);
			}
		}
		if (found_one)
		{
			if (event->button == 3)
			{
				update_properties(grabbed_widget,GAUGE_REMOVE);
				gtk_widget_destroy(grabbed_widget);
			}
			if (event->button == 1)
			{
				//printf("grabbed it \n");
				grabbed = TRUE;
				tt.rel_grab_x=x_cur;
				tt.rel_grab_y=y_cur;

				if (((x_cur < (tt.child_x_origin+DRAG_BORDER)) && ((y_cur < tt.child_y_origin+DRAG_BORDER) || (y_cur > tt.child_y_origin+tt.child_height-DRAG_BORDER))) || ((x_cur > (tt.child_x_origin+tt.child_width-DRAG_BORDER)) && ((y_cur < tt.child_y_origin+DRAG_BORDER) || (y_cur > tt.child_y_origin+tt.child_height-DRAG_BORDER))))
				{
					resizing = TRUE;
					/* Left border */
					if (x_cur < (tt.child_x_origin+7))
					{
						if (y_cur < (tt.child_y_origin+(0.33*tt.child_height)))
							corner = UL;
						else if (y_cur > (tt.child_y_origin+(0.66*tt.child_height)))
							corner = LL;
						else corner = -1;
						
					}
					/* Right border */
					else if (x_cur > (tt.child_x_origin+tt.child_width-7))
					{
						if (y_cur < (tt.child_y_origin+(0.33*tt.child_height)))
							corner = UR;
						else if (y_cur > (tt.child_y_origin+(0.66*tt.child_height)))
							corner = LR;
						else
							corner = -1;
					}
					/* Top border */
					if (y_cur < (tt.child_y_origin+7))
					{
						if (x_cur < (tt.child_x_origin+(0.33*tt.child_width)))
							corner = UL;
						else if (x_cur > (tt.child_x_origin+(0.66*tt.child_width)))
							corner = UR;
						else
							corner = -1;
					}
					/* Bottom border */
					else if (y_cur > (tt.child_y_origin+tt.child_height-7))
					{
						if (x_cur < (tt.child_x_origin+(0.33*tt.child_width)))
							corner = LL;
						else if (x_cur > (tt.child_x_origin+(0.66*tt.child_width)))
							corner = LR;
						else 
							corner = -1;
					}
				}
				else
					moving = TRUE;


			}
			else
			{
				//printf("didn't grab squat\n");
				grabbed = FALSE;
				moving = FALSE;
				resizing = FALSE;
			}
		}
	}



	/*
	if (moving)
		printf ("MOVING!\n");
	if (resizing)
		printf ("RESIZING\n");
	if (grabbed)
	{
		printf("grabbed TRUE\n");
		printf("child base %i,%i, rel click point %i,%i\n",tt.child_x_origin,tt.child_y_origin,tt.rel_grab_x,tt.rel_grab_y);
	}
	*/
	
	return TRUE;

}

void raise_fixed_child (GtkWidget * widget)
{
	GtkFixed *fixed;

	g_return_if_fail (GTK_IS_FIXED (widget->parent));
	fixed = GTK_FIXED (widget->parent);
	/* If widget hasn't got a window, move it to the back of the parent fixed's
	 *      children. If it has got a window, raise it. */
	/* Note: this is slightly naughty as it changes the GtkFixed's GList of
	 *      children, but it's better than removing the widget and adding it again. */
	if (GTK_WIDGET_NO_WINDOW (widget))
	{
		GList *child;
		GtkFixedChild *data;
		child = fixed->children;
		while (child)
		{
			data = child->data;
			if (data->widget == widget)
			{
				fixed->children = g_list_remove (fixed->children, data);
				fixed->children = g_list_append (fixed->children, data);
				break;
			}
			child = child->next;
		}
	}
	else
	{
		gdk_window_raise (widget->window);
	}
}


void update_properties(GtkWidget * widget, Choice choice)
{
	extern GtkListStore *store;
	GtkCellRenderer *renderer;
	extern GladeXML *prop_xml;
	GtkWidget * combo_box = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *table = NULL;
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;
	GtkWidget *entry = NULL;
	GtkWidget *sep = NULL;
	gint len = 0;

	if (choice == GAUGE_ADD)
	{
		printf ("gauge add\n");
		table = gtk_table_new(3,2,FALSE);
		gtk_container_set_border_width(GTK_CONTAINER(table),5);
		entry = gtk_entry_new();
		tmpbuf = mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(widget));
		vector = g_strsplit(tmpbuf,PSEP,-1);
		len = g_strv_length(vector);
		g_free(tmpbuf);
		tmpbuf = g_strdelimit(g_strdup(vector[len-1]),"_",' ');
		g_strfreev(vector);
		vector = g_strsplit(tmpbuf,".",-1);
		g_free(tmpbuf);
	
		gtk_entry_set_text(GTK_ENTRY(entry),vector[0]);
		g_strfreev(vector);
		gtk_table_attach(GTK_TABLE(table),entry,0,1,0,1,GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);

		combo_box = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
		gtk_table_attach(GTK_TABLE(table),combo_box,0,2,1,2,GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
		g_object_set_data(G_OBJECT(widget),"combo",combo_box);

		sep = gtk_hseparator_new();
		gtk_table_attach(GTK_TABLE(table),sep,0,2,1,2,GTK_FILL|GTK_EXPAND,GTK_FILL,0,5);

		renderer = gtk_cell_renderer_text_new();
		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo_box),renderer,FALSE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo_box),renderer,"markup",0,NULL);
		renderer = gtk_cell_renderer_text_new();
		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo_box),renderer,FALSE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo_box),renderer,"text",1,NULL);
		vbox = glade_xml_get_widget(prop_xml,"prop_top_vbox");
		g_object_set_data(G_OBJECT(widget),"prop_table",table);
		gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,TRUE,0);
		gtk_widget_show_all(vbox);
	}
	else if (choice == GAUGE_REMOVE)
	{
		printf ("gauge removal\n");
		table = g_object_get_data(G_OBJECT(widget),"prop_table");
		gtk_widget_destroy(table);
	}

	printf("update_properties\n");
}

