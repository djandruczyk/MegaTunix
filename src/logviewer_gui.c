/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <datalogging_const.h>
#include <defines.h>
#include <enums.h>
#include <globals.h>
#include <gui_handlers.h>
#include <logviewer_gui.h>
#include <structures.h>

static gint max_viewables = 0;
static gint total_viewables = 0;
struct Logables viewables;
GHashTable *active_traces = NULL;


void build_logviewer(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *vbox4;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *frame;
	GSList *group;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	/* Traces frame */
	frame = gtk_frame_new("Log Viewer");
	gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,0);

	/* Holds all the log traces... */
	vbox2 = gtk_vbox_new(TRUE,2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	/* Settings/Parameters frame... */
	frame = gtk_frame_new("Viewer Parameters");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	hbox = gtk_hbox_new(FALSE,15);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	/* Hold the realtime/[playback buttons */
	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);

	button = gtk_radio_button_new_with_label(NULL,"Realtime Mode");
	gtk_box_pack_start(GTK_BOX(vbox3),button,FALSE,FALSE,0);

	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
	button = gtk_radio_button_new_with_label(group,"Playback Mode");
	gtk_box_pack_start(GTK_BOX(vbox3),button,FALSE,FALSE,0);

	/* Holds the Select Button */
	vbox4 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox4,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Select Parameters to view");
	g_object_set_data(G_OBJECT(button),"data",(gpointer)vbox2);
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(SEL_PARAMS));
	gtk_box_pack_start(GTK_BOX(vbox4),button,TRUE,FALSE,0);
	
	/* Not written yet */
	return;
}

void present_viewer_choices( void *ptr)
{
	GtkWidget *window;
	GtkWidget *table;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *sep;
	extern GtkTooltips *tip;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint table_rows;
	gint table_cols = 5;
	GtkWidget *special = NULL;

	if (ptr != NULL)
		special = (GtkWidget *)ptr;
	else
		printf("pointer fed was NULL (present_viewer_choices)\n");

	/* basty hack to prevent a compiler warning... */
	max_viewables = sizeof(mt_compat_names)/sizeof(gchar *);
	max_viewables = sizeof(logable_names)/sizeof(gchar *);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window),575,300);
	gtk_window_set_title(GTK_WINDOW(window),"Logviewer Realtime choices");
	g_signal_connect_swapped(G_OBJECT(window),"destroy_event",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);
	g_signal_connect_swapped(G_OBJECT(window),"delete_event",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);

	gtk_container_set_border_width(GTK_CONTAINER(window),5);
	frame = gtk_frame_new("Select Variables to view from the list below");
	gtk_container_add(GTK_CONTAINER(window),frame);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);

	max_viewables = sizeof(logable_names)/sizeof(gchar *);
	table_rows = ceil((float)max_viewables/(float)table_cols);
	table = gtk_table_new(table_rows,table_cols,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,0);

	j = 0;
	k = 0;
	for (i=0;i<max_viewables;i++)
	{
		button = gtk_check_button_new_with_label(logable_names[i]);
		gtk_tooltips_set_tip(tip,button,logable_names_tips[i],NULL);
		if (viewables.index[i] == TRUE)
        		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),TRUE);
			
		viewables.widgets[i] = button;

		g_object_set_data(G_OBJECT(button),"index",
				GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"toggled",
				G_CALLBACK(view_value_set),
				NULL);
		gtk_table_attach (GTK_TABLE (table), button, j, j+1, k, k+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
		j++;

		if (j == 5)
		{
			k++;
			j = 0;
		}
	}

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox),sep,TRUE,TRUE,0);
	button = gtk_button_new_with_label("Close");
	gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(populate_viewer),
			(gpointer)special);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);

	gtk_widget_show_all(window);
	printf("present choices...\n");
	return;
}

gboolean view_value_set(GtkWidget *widget, gpointer data)
{
	gint index = 0;
        gint i = 0;

        index = (gint)g_object_get_data(G_OBJECT(widget),"index");

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
                viewables.index[index] = TRUE;
        else
                viewables.index[index] = FALSE;

        total_viewables = 0;
        for (i=0;i<max_viewables;i++)
        {
                if (viewables.index[i])
                        total_viewables++;
        }

	printf("total viewables is %i\n",total_viewables);

	return TRUE;
}

gboolean populate_viewer(GtkWidget * widget)
{
	struct Viewable_Value *v_value = NULL;
	gint i = 0;

	printf("populate_viewer()\n");
	/* Checks if list is created, if not,  makes one, allocates data
	 * for strcutres defining each viewable element., sets those attribute
	 * and adds the mto the list,  also checks if entires are removed and
	 * pulles them fromthe list and de-allocates them...
	 */
	if (active_traces == NULL)
	{
		active_traces = g_hash_table_new(NULL,NULL);
	}
	
	/* check to see if it's already in the table, if so ignore, if not
	 * malloca datastructure, populate it's values and insert a pointer
	 * into the table for it..
	 */
	for (i=0;i<max_viewables;i++)
	{
		/* if not found in table check to see if we need to insert*/
		if (g_hash_table_lookup(active_traces,GINT_TO_POINTER(i))==NULL)
		{
			if (viewables.index[i])	/* Marked viewable widget */
			{
				printf("allocating struct and putting into table\n");
				/* Allocate data struct and insert ptr to it*/
				v_value = g_malloc(sizeof(struct Viewable_Value));		
				v_value->parent = widget;
				v_value->d_area = gtk_drawing_area_new();

				v_value->pmap = gdk_pixmap_new(
						v_value->d_area->window, 
						10,10, 
						gtk_widget_get_visual(
							v_value->d_area)->depth);
				gtk_box_pack_start(GTK_BOX(widget),
						v_value->d_area,
						TRUE,TRUE,0);
				gtk_widget_add_events(v_value->d_area,
						GDK_BUTTON_PRESS_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_FOCUS_CHANGE_MASK);
						
				g_signal_connect(G_OBJECT
						(v_value->d_area),
						"configure_event",
						G_CALLBACK(lv_configure_event),
						NULL);
						
				g_signal_connect(G_OBJECT
						(v_value->d_area),
						"expose_event",
						G_CALLBACK(lv_expose_event),
						NULL);
						
				g_object_set_data(G_OBJECT(v_value->d_area),
						"data",(gpointer)v_value);
		
				v_value->runtime_offset = logging_offset_map[i];
				v_value->size = logging_datasizes_map[i];
				
				gtk_widget_show_all(widget);
				printf("put in offset %i, runtime_offset %i, size %i\n",i,v_value->runtime_offset, v_value->size);
				g_hash_table_insert(active_traces,
						GINT_TO_POINTER(i),
						(gpointer)v_value);
			}
		}
		else
		{	/* If in table but now de-selected, remove it */
			if (!viewables.index[i])
			{
				v_value = (struct Viewable_Value *)
					g_hash_table_lookup(
							active_traces,
							GINT_TO_POINTER(i));
				/* Need to remove the widgets as well */
				gtk_widget_destroy(v_value->d_area);
				g_hash_table_remove(active_traces,
						GINT_TO_POINTER(i));
				v_value->parent = NULL;
				v_value->d_area = NULL;
				v_value->runtime_offset = -1;
				v_value->size = 0;
				g_free(v_value);
				v_value = NULL;
			}
		}
	}
		
	return FALSE; /* want other handlers to run... */
}

gboolean lv_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GdkPixmap *pixmap = NULL;
	gint w = 0;
	gint h = 0;
	struct Viewable_Value *v_value = NULL;

	/* Get rest of important data... */
	v_value = (struct Viewable_Value *)g_object_get_data(G_OBJECT(widget),
			"data");
	//	w= v_value->
	//	h= v_value->
	w=300;  /* HACK FIXME!!!! */
	h=50;
	pixmap = v_value->pmap;

	if ((w!=event->width)||(h!=event->height))
	{
		if (pixmap)
			g_object_unref(pixmap);

		w=event->width;
		h=event->height;
		pixmap=gdk_pixmap_new(widget->window,
				w,h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				w,h);
		gdk_window_set_back_pixmap(widget->window,pixmap,0);
	}
	v_value->pmap = pixmap;
	gdk_window_clear(widget->window);



	printf("configure event....\n");
	return TRUE;
}

gboolean lv_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GdkPixmap *pixmap = NULL;
	struct Viewable_Value *v_value = NULL;
	v_value = (struct Viewable_Value *)g_object_get_data(G_OBJECT(widget),
				"data");

	/* Expose event handler... */
	pixmap = v_value->pmap;
	gdk_draw_drawable(widget->window,
                        widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                        pixmap,
                        event->area.x, event->area.y,
                        event->area.x, event->area.y,
                        event->area.width, event->area.height);

	printf("expose event....\n");
	return TRUE;
}
