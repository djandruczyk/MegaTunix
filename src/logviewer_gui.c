/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/logviewer_gui.c
  \ingroup CoreMtx
  \brief The old Logviewer Gui handler functions. 
  \author David Andruczyk
  */

#include <debugging.h>
#include <getfiles.h>
#include <glade/glade.h>
#include <listmgmt.h>
#include <logviewer_events.h>
#include <logviewer_gui.h>
#include <math.h>
#include <rtv_map_loader.h>
#include <stdlib.h>
#include <timeout_handlers.h>
#include <widgetmgmt.h>

static gint max_viewables = 0;
static gboolean adj_scale = TRUE;
static gboolean blocked = FALSE;
static gfloat hue = -60.0;
static gfloat col_sat = 1.0;
static gfloat col_val = 1.0;
Logview_Data *lv_data = NULL;
static GMutex update_mutex;
extern gconstpointer *global_data;

/*!
  \brief present_viewer_choices() presents the user with the a list of 
  variables from EITHER the realtime vars (if in realtime mode) or from a 
  datalog (playback mode)
  */
G_MODULE_EXPORT void present_viewer_choices(void)
{
	GtkWidget *window = NULL;
	GtkWidget *table = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *button = NULL;
	GtkWidget *label = NULL;
	GtkWidget *sep = NULL;
	GtkWidget *darea = NULL;
	GList *list = NULL;
	gconstpointer * object = NULL;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint table_rows = 0;
	gint table_cols = 5;
	gchar * name = NULL;
	gchar * tooltip = NULL;
	gboolean playback = FALSE;
	Rtv_Map *rtv_map = NULL;
	Log_Info *log_info;

	ENTER();
	log_info = (Log_Info *)DATA_GET(global_data,"log_info");
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	darea = lookup_widget("logviewer_trace_darea");
	lv_data->darea = darea;
	playback = (GBOOLEAN)DATA_GET(global_data,"playback_mode");

	if (!darea)
	{
		MTXDBG(CRITICAL,_("Pointer to drawing area was NULL, returning!!!\n"));
		EXIT();
		return;
	}

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(lookup_widget("main_window")));
	gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
	/* Playback mode..... */
	if (playback)
	{
		gtk_window_set_title(GTK_WINDOW(window),
				_("Playback Mode: Logviewer Choices"));
		frame = gtk_frame_new(_("Select Variables to playback from the list below..."));
		max_viewables = log_info->field_count;
	}
	else
	{
		/* Realtime Viewing mode... */
		gtk_window_set_title(GTK_WINDOW(window),
				_("Realtime Mode: Logviewer Choices"));
		frame = gtk_frame_new(_("Select Realtime Variables to view from the list below..."));
		max_viewables = rtv_map->derived_total;

	}
	g_signal_connect_swapped(G_OBJECT(window),"destroy_event",
			G_CALLBACK(reenable_select_params_button),
			NULL);
	g_signal_connect_swapped(G_OBJECT(window),"destroy_event",
			G_CALLBACK(save_default_choices),
			NULL);
	g_signal_connect_swapped(G_OBJECT(window),"destroy_event",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);
	g_signal_connect_swapped(G_OBJECT(window),"delete_event",
			G_CALLBACK(reenable_select_params_button),
			NULL);
	g_signal_connect_swapped(G_OBJECT(window),"delete_event",
			G_CALLBACK(save_default_choices),
			NULL);
	g_signal_connect_swapped(G_OBJECT(window),"delete_event",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);

	gtk_container_set_border_width(GTK_CONTAINER(window),5);
	gtk_container_add(GTK_CONTAINER(window),frame);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);

	table_rows = ceil((float)max_viewables/(float)table_cols);
	table = gtk_table_new(table_rows,table_cols,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,0);

	j = 0;
	k = 0;
	if(get_list("viewables"))
	{
		g_list_free(get_list("viewables"));
		remove_list("viewables");
	}

	for (i=0;i<max_viewables;i++)
	{
		if (playback)
			list = g_list_prepend(list,(gpointer)g_ptr_array_index(log_info->log_list,i));
		else
			list = g_list_prepend(list,(gpointer)g_ptr_array_index(rtv_map->rtv_list,i));
	}
	if (playback)
		list=g_list_sort_with_data(list,list_object_sort,(gpointer)"lview_name");
	else
		list=g_list_sort_with_data(list,list_object_sort,(gpointer)"dlog_gui_name");

	for (i=0;i<max_viewables;i++)
	{
		object = NULL;
		name = NULL;
		tooltip = NULL;

		object = (gconstpointer *)g_list_nth_data(list,i);

		if (playback)
			name = g_strdup((gchar *)DATA_GET(object,"lview_name"));
		else
		{
			name = g_strdup((gchar *)DATA_GET(object,"dlog_gui_name"));
			tooltip = g_strdup((gchar *)DATA_GET(object,"tooltip"));
		}

		button = gtk_check_button_new();
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),name);
		gtk_container_add(GTK_CONTAINER(button),label);
		store_list("viewables",g_list_prepend(
					get_list("viewables"),(gpointer)button));
		if (tooltip)
			gtk_widget_set_tooltip_text(button,tooltip);

		if (object)
		{
			OBJ_SET(button,"object",(gpointer)object);
			/* so we can set the state from elsewhere...*/
			DATA_SET(object,"lview_button",(gpointer)button);
			if ((GBOOLEAN)DATA_GET(object,"being_viewed"))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		}
		g_signal_connect(G_OBJECT(button),"toggled",
				G_CALLBACK(view_value_set),
				NULL);
		gtk_table_attach (GTK_TABLE (table), button, j, j+1, k, k+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
		j++;

		if (j == table_cols)
		{
			k++;
			j = 0;
		}
		g_free(name);
	}
	g_list_free(list);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox),sep,FALSE,TRUE,20);

	hbox = gtk_hbox_new(FALSE,20);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);
	button = gtk_button_new_with_label("Select All");
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,15);
	OBJ_SET(button,"state",GINT_TO_POINTER(TRUE));
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(set_all_lview_choices_state),
			GINT_TO_POINTER(TRUE));
	button = gtk_button_new_with_label("De-select All");
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,15);
	OBJ_SET(button,"state",GINT_TO_POINTER(FALSE));
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(set_all_lview_choices_state),
			GINT_TO_POINTER(FALSE));

	button = gtk_button_new_with_label("Close");
	gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(reenable_select_params_button),
			NULL);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(save_default_choices),
			NULL);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);

	set_default_lview_choices_state();
	gtk_widget_show_all(window);
	EXIT();
	return;
}


/*!
  \brief re-enabled the select params button
  \param widget is unused
  \returns FALSE
 */
G_MODULE_EXPORT gboolean reenable_select_params_button(GtkWidget *widget)
{
	ENTER();
	gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget("logviewer_select_params_button")),TRUE);
	EXIT();
	return FALSE;

}


/*!
  \brief Saves the default loggable choices
  \param widget is unused
  \returns FALSE
 */
G_MODULE_EXPORT gboolean save_default_choices(GtkWidget *widget)
{
	GtkWidget *tmpwidget = NULL;
	GList * list = NULL;
	GList * defaults = NULL;
	gconstpointer *object = NULL;
	gchar *name = NULL;
	guint i = 0;

	ENTER();
	defaults = get_list("logviewer_defaults");
	if (defaults)
	{
		g_list_foreach(defaults,(GFunc)g_free,NULL);
		g_list_free(defaults);
		defaults = NULL;
		remove_list("logviewer_defaults");
	}
	list = get_list("viewables");
	for (i=0;i<g_list_length(list);i++)
	{
		tmpwidget = (GtkWidget *)g_list_nth_data(list,i);
		object = (gconstpointer *)OBJ_GET(tmpwidget,"object");
		if ((GBOOLEAN)DATA_GET(object,"being_viewed"))
		{
			if (DATA_GET(global_data,"playback_mode"))
				name = (gchar *)DATA_GET(object,"lview_name");
			else
				name = (gchar *)DATA_GET(object,"dlog_gui_name");

			defaults = g_list_append(defaults,g_strdup(name));
		}
	}
	store_list("logviewer_defaults",defaults);
	EXIT();
	return FALSE;
}

/*!
  \brief view_value_set() is called when a value to be viewed is selected
  or not. We tag the widget with a marker if it is to be displayed
  \param widget is the button clicked, we extract the object this
  represents and mark it
  \param data is unused
  \returns FALSE
  */
G_MODULE_EXPORT gboolean view_value_set(GtkWidget *widget, gpointer data)
{
	gconstpointer *object = NULL;
	gboolean state = FALSE;

	ENTER();
	state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widget));

	/* get object from widget */
	object = (gconstpointer *)OBJ_GET(widget,"object");
	if (!object)
	{
		MTXDBG(CRITICAL,_("NO object was bound to the button\n"));
	}
	DATA_SET(object,"being_viewed",GINT_TO_POINTER(state));
	populate_viewer();
	EXIT();
	return FALSE;
}


/*!
  \brief populate_viewer() creates/removes the list of viewable values from
  the objects in use (playback list or realtime vars list)
  */
G_MODULE_EXPORT void populate_viewer(void)
{
	gint i = 0;
	gint total = 0;
	Viewable_Value *v_value = NULL;
	gchar * name = NULL;
	gboolean being_viewed = FALSE;
	Rtv_Map *rtv_map = NULL;
	gconstpointer *object = NULL;
	Log_Info *log_info = NULL;

	ENTER();

	g_mutex_lock(&update_mutex);
	log_info = (Log_Info *)DATA_GET(global_data,"log_info");
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	/* Checks if hash is created, if not, makes one, allocates data
	 * for strcutres defining each viewable element., sets those attribute
	 * and adds them to the list, also checks if entires are removed and
	 * pulls them from the hashtable and de-allocates them...
	 */
	if (lv_data == NULL)
	{
		lv_data = g_new0(Logview_Data,1);
		lv_data->traces = g_hash_table_new(g_str_hash,g_str_equal);
		lv_data->info_width = 120;
	}

	/* check to see if it's already in the table, if so ignore, if not
	 * malloc datastructure, populate it's values and insert a pointer
	 * into the table for it..
	 */
	if (DATA_GET(global_data,"playback_mode"))
		total = log_info->field_count;
	else
		total = rtv_map->derived_total;

	for (i=0;i<total;i++)
	{
		object = NULL;
		name = NULL;
		if (DATA_GET(global_data,"playback_mode"))
		{
			object = (gconstpointer *)g_ptr_array_index(log_info->log_list,i);
			name = (gchar *)DATA_GET(object,"lview_name");
		}
		else
		{
			object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list,i); 
			name = (gchar *)DATA_GET(object,"dlog_gui_name");
		}
		if (!name)
			MTXDBG(CRITICAL,_("ERROR, name is NULL\n"));
		if (!object)
			MTXDBG(CRITICAL,_("ERROR, object is NULL\n"));

		being_viewed = (GBOOLEAN)DATA_GET(object,"being_viewed");
		/* if not found in table check to see if we need to insert*/
		if (!(g_hash_table_lookup(lv_data->traces,name)))
		{
			if (being_viewed)	/* Marked viewable widget */
			{
				/* Call the build routine, feed it the drawing_area*/
				v_value = build_v_value(object);
				/* store location of master*/
				g_hash_table_insert(lv_data->traces,
						g_strdup(name),
						(gpointer)v_value);
				lv_data->tlist = g_list_prepend(lv_data->tlist,(gpointer)v_value);
			}
		}
		else
		{	/* If in table but now de-selected, remove it */
			if (!being_viewed)
			{
				v_value = (Viewable_Value *)g_hash_table_lookup(lv_data->traces,name);
				lv_data->tlist = g_list_remove(lv_data->tlist,(gpointer)v_value);
				if ((hue > 0) && ((GINT)hue%1110 == 0))
				{
					hue-= 30;
					col_sat = 1.0;
					col_val = 0.75;
					/*printf("hue at 1110 deg, reducing to 1080, sat at 1.0, val at 0.75\n");*/
				}
				if ((hue > 0) && ((GINT)hue%780 == 0))
				{
					hue-= 30;
					col_sat = 0.5;
					col_val = 1.0;
					/*printf("hue at 780 deg, reducing to 750, sat at 0.5, val at 1.0\n");*/
				}
				if ((hue > 0) && ((GINT)hue%390 == 0)) /* phase shift */
				{
					hue-=30.0;
					col_sat=1.0;
					col_val = 1.0;
					/*printf("hue at 390 deg, reducing to 360, sat at 0.5, val at 1.0\n");*/
				}
				hue -=60;
				/*printf("angle at %f, sat %f, val %f\n",hue,col_sat,col_val);*/

				/* Remove entry in from hash table */
				g_hash_table_remove(lv_data->traces,name);

				/* Free all resources of the datastructure 
				 * before de-allocating it... 
				 */

				g_object_unref(v_value->trace_gc);
				g_object_unref(v_value->font_gc);
				g_free(v_value->vname);
				g_free(v_value);
				g_free(v_value->ink_rect);
				g_free(v_value->log_rect);
				v_value = NULL;
			}
		}
	}
	lv_data->active_traces = g_hash_table_size(lv_data->traces);
	/* If traces selected, emit a configure_Event to clear the window
	 * and draw the traces (IF ONLY reading a log for playback)
	 */
	g_mutex_unlock(&update_mutex);
	if ((lv_data->traces) && (g_list_length(lv_data->tlist) > 0))
		lv_configure_event(lookup_widget("logviewer_trace_darea"),NULL,NULL);

	EXIT();
	return; 
}


/*!
  \brief reset_logviewer_state() deselects any traces, resets the position 
  slider.  This function is called when switching from playback to rt mode
  and back
  */
G_MODULE_EXPORT void reset_logviewer_state(void)
{
	guint i = 0 ;
	gconstpointer * object = NULL;
	Rtv_Map *rtv_map = NULL;;
	Log_Info *log_info;

	ENTER();
	log_info = (Log_Info *)DATA_GET(global_data,"log_info");

	if (DATA_GET(global_data,"playback_mode"))
	{
		if (!log_info)
		{
			EXIT();
			return;
		}
		for (i=0;i<log_info->field_count;i++)
		{
			object = NULL;
			object = (gconstpointer *)g_ptr_array_index(log_info->log_list,i);
			if (object)
				DATA_SET(object,"being_viewed",GINT_TO_POINTER(FALSE));
		}
	}
	else
	{
		rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
		if (!rtv_map)
		{
			EXIT();
			return;
		}
		for (i=0;i<rtv_map->derived_total;i++)
		{
			object = NULL;
			object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list,i);
			if (object)
				DATA_SET(object,"being_viewed",GINT_TO_POINTER(FALSE));
		}
	}
	populate_viewer();
	EXIT();
	return;
}


/*!
  \brief build_v_value() allocates a viewable_value structure and populates
  it with sane defaults and returns it to the caller
  \param object is the object to get soem of the data from
  \returns a pointer to a newly allocated and populated Viewable_Value structure
  */
G_MODULE_EXPORT Viewable_Value * build_v_value(gconstpointer *object)
{
	Viewable_Value *v_value = NULL;
	GdkPixmap *pixmap =  NULL;

	ENTER();
	pixmap = lv_data->pixmap;

	v_value = (Viewable_Value *)g_malloc(sizeof(Viewable_Value));		

	/* Set limits of this variable. (it's ranges, used for scaling */

	if (DATA_GET(global_data,"playback_mode"))
	{
		/* textual name of the variable we're viewing.. */
		v_value->vname = g_strdup((gchar *)DATA_GET(object,"lview_name"));
		/* data was already read from file and stored, copy pointer
		 * over to v_value so it can be drawn...
		 */
		v_value->data_source = g_strdup("data_array");
	}
	else
	{
		/* textual name of the variable we're viewing.. */
		v_value->vname = g_strdup((gchar *)DATA_GET(object,"dlog_gui_name"));
		/* Array to keep history for resize/redraw and export 
		 * to datalog we use the _sized_ version to give a big 
		 * enough size to prevent reallocating memory too often. 
		 * (more initial mem usage,  but less calls to malloc...
		 */
		v_value->data_source = g_strdup("history");
	}
	/* Store pointer to object, but DO NOT FREE THIS on v_value destruction
	 * as its the SAME one used for all Viewable_Values */
	v_value->object = object;
	/* IS it a floating point value? */
	v_value->precision = (GINT)DATA_GET(object,"precision");
	v_value->lower = (GINT)strtol((gchar *)DATA_GET(object,"real_lower"),NULL,10);
	v_value->upper = (GINT)strtol((gchar *)DATA_GET(object,"real_upper"),NULL,10);
	/* Sets last "y" value to -1, needed for initial draw to be correct */
	v_value->last_y = -1;

	/* User adjustable scales... */
	v_value->cur_low = v_value->lower;
	v_value->cur_high = v_value->upper;
	v_value->min = 0;
	v_value->max = 0;

	/* Allocate the colors (GC's) for the font and trace */
	v_value->font_gc = initialize_gc(pixmap, FONT);
	v_value->trace_gc = initialize_gc(pixmap, TRACE);

	/* Allocate the structs to hold the text screen dimensions */
	v_value->ink_rect = g_new0(PangoRectangle, 1);
	v_value->log_rect = g_new0(PangoRectangle, 1);

	v_value->force_update = TRUE;
	v_value->highlight = FALSE;

	EXIT();
	return v_value;
}


/*!
  \brief initialize_gc() allocates and initializes the graphics contexts for
  the logviewer trace window.
  \param drawable is the pointer to the drawable surface
  \param type is the Graphics Context type? (I donno for sure)
  \returns Pointer to a GdkGC *
  */
G_MODULE_EXPORT GdkGC * initialize_gc(GdkDrawable *drawable, GcType type)
{
	GdkColor color;
	GdkGC * gc = NULL;
	GdkGCValues values;
	GdkColormap *cmap = NULL;

	ENTER();
	cmap = gdk_colormap_get_system();

	switch((GcType)type)
	{
		case HIGHLIGHT:
			color.red = 60000;
			color.green = 0;
			color.blue = 0;
			gdk_colormap_alloc_color(cmap,&color,TRUE,TRUE);
			values.foreground = color;
			gc = gdk_gc_new_with_values(GDK_DRAWABLE(drawable),
					&values,
					GDK_GC_FOREGROUND);
			break;
		case FONT:
			color.red = 65535;
			color.green = 65535;
			color.blue = 65535;
			gdk_colormap_alloc_color(cmap,&color,TRUE,TRUE);
			values.foreground = color;
			gc = gdk_gc_new_with_values(GDK_DRAWABLE(drawable),
					&values,
					GDK_GC_FOREGROUND);
			break;

		case TRACE:
			hue += 60;
			/*printf("angle at %f, sat %f, val %f\n",hue,col_sat,col_val);*/

			if ((hue > 0) && ((GINT)hue%360 == 0))
			{
				hue+=30.0;
				col_sat=0.5;
				col_val=1.0;
			}
			if ((hue > 0) && ((GINT)hue%750 == 0))
			{
				hue+=30;
				col_sat=1.0;
				col_val = 0.75;
			}
			/*printf("JBA angle at %f, sat %f, val %f\n",hue,col_sat,col_val);*/
			color = get_colors_from_hue(hue,col_sat,col_val);
			gdk_colormap_alloc_color(cmap,&color,TRUE,TRUE);
			values.foreground = color;
			gc = gdk_gc_new_with_values(GDK_DRAWABLE(drawable),
					&values,
					GDK_GC_FOREGROUND);


			break;
		case GRATICULE:
			color.red = 36288;
			color.green = 2048;
			color.blue = 2048;
			gdk_colormap_alloc_color(cmap,&color,TRUE,TRUE);
			values.foreground = color;
			gc = gdk_gc_new_with_values(GDK_DRAWABLE(drawable),
					&values,
					GDK_GC_FOREGROUND);
			break;
		case TTM_AXIS:
			color.red = 32768;
			color.green = 32768;
			color.blue = 32768;
			gdk_colormap_alloc_color(cmap,&color,TRUE,TRUE);
			values.foreground = color;
			gc = gdk_gc_new_with_values(GDK_DRAWABLE(drawable),
					&values,
					GDK_GC_FOREGROUND);
			break;
		case TTM_TRACE:
			color.red = 0;
			color.green = 0;
			color.blue = 0;
			gdk_colormap_alloc_color(cmap,&color,TRUE,TRUE);
			values.foreground = color;
			gc = gdk_gc_new_with_values(GDK_DRAWABLE(drawable),
					&values,
					GDK_GC_FOREGROUND);
			break;
	}	
	EXIT();
	return gc;	
}


/*!
  \brief get_colors_from_hue(gets a color back from an angle passed in degrees.
  The degrees represent the arc aroudn a color circle.
  \param hue is the degrees around the color circle
  \param sat is the col_sat from 0-1.0
  \param val is the col_val from 0-1.0
  \returns a GdkColor at the hue angle requested
  */
G_MODULE_EXPORT GdkColor get_colors_from_hue(gfloat hue, gfloat sat, gfloat val)
{
	static gint count = 0;
	GdkColor color;
	gint i = 0;
	gfloat tmp = 0.0;	
	gfloat fract = 0.0;
	gfloat S = sat;	/* using col_sat of 1.0*/
	gfloat V = val;	/* using Value of 1.0*/
	gfloat p = 0.0;
	gfloat q = 0.0;
	gfloat t = 0.0;
	gfloat r = 0.0;
	gfloat g = 0.0;
	gfloat b = 0.0;
	static GdkColormap *colormap = NULL;

	ENTER();
	count++;
	if (!colormap)
		colormap = gdk_colormap_get_system();

	/*printf("get_color_from_hue count %i\n",count); */

	hue = (gint)hue % 360;
/*	while (hue >= 360.0)
		hue -= 360.0;
		*/
	while (hue < 0.0)
		hue += 360.0;
	tmp = hue/60.0;
	i = floor(tmp);
	fract = tmp-i;

	p = V*(1.0-S);	
	q = V*(1.0-(S*fract));	
	t = V*(1.0-(S*(1.0-fract)));

	switch (i)
	{
		case 0:
			r = V;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = V;
			b = p;
			break;
		case 2:
			r = p;
			g = V;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = V;
			break;
		case 4:
			r = t;
			g = p;
			b = V;
			break;
		case 5:
			r = V;
			g = p;
			b = q;
			break;
	}
	color.red = r * 65535;
	color.green = g * 65535;
	color.blue = b * 65535;
	gdk_colormap_alloc_color(colormap,&color,FALSE,TRUE);

	EXIT();
	return (color);	
}


/*!
  \brief draw_infotext() draws the static textual data for the trace on 
  the left hand side of the logviewer
  */
G_MODULE_EXPORT void draw_infotext(void)
{
	/* Draws the textual (static) info on the left side of the window..*/

	gint name_x = 0;
	gint name_y = 0;
	gint text_border = 10;
	gint info_ctr = 0;
	gint h = 0;
	gint i = 0;
	gint width = 0;
	gint height = 0;
	gint max = 0;
	Viewable_Value *v_value = NULL;
	cairo_t *cr = NULL;
	PangoLayout *layout;
	GdkPixmap *pixmap = lv_data->pixmap;
	GtkAllocation allocation;

	ENTER();
	gtk_widget_get_allocation(lv_data->darea,&allocation);

	h = allocation.height;

	cr = gdk_cairo_create(pixmap);
	cairo_set_source_rgb(cr,0.0,0.0,0.0);
	cairo_rectangle(cr,0,0,lv_data->info_width,h);
	cairo_fill(cr);
	cairo_destroy(cr);

	if (!lv_data->font_desc)
	{
		lv_data->font_desc = pango_font_description_from_string("courier");
		pango_font_description_set_size(lv_data->font_desc,(10)*PANGO_SCALE);
	}
	if (!lv_data->highlight_gc)
		lv_data->highlight_gc = initialize_gc(lv_data->pixmap,HIGHLIGHT);
	
	lv_data->spread = (GINT)((float)h/(float)lv_data->active_traces);
	name_x = text_border;
	layout = gtk_widget_create_pango_layout(lv_data->darea,NULL);
	for (i=0;i<lv_data->active_traces;i++)
	{
		v_value = (Viewable_Value *)g_list_nth_data(lv_data->tlist,i);
		info_ctr = (lv_data->spread * (i+1))- (lv_data->spread/2);

		pango_layout_set_markup(layout,v_value->vname,-1);
		pango_layout_set_font_description(layout,lv_data->font_desc);
		pango_layout_get_pixel_size(layout,&width,&height);
		name_y = info_ctr - height - 2;

		if (width > max)
			max = width;
		
		gdk_draw_layout(pixmap,v_value->trace_gc,name_x,name_y,layout);
	}
	lv_data->info_width = max + (text_border * 2.5);

	for (i=0;i<lv_data->active_traces;i++)
	{
		gdk_draw_rectangle(pixmap,
				gtk_widget_get_style(lv_data->darea)->white_gc,
				FALSE, 0,i*lv_data->spread,
				lv_data->info_width-1,lv_data->spread);
	}
	EXIT();
	return;
}


/*!
  \brief draw_valtext() draws the dynamic values for the traces on 
  the left hand side of the logviewer. This is optimized so that if the value
  becomes temporarily static, it won't keep blindly updating the screen and
  wasting CPU time.
  \param force_draw when true to write the values to screen for
  all controls no matter if hte previous value is the same or not.
  */
G_MODULE_EXPORT void draw_valtext(gboolean force_draw)
{
	gint last_index = 0;
	gfloat val = 0.0;
	gfloat last_val = 0.0;
	gint val_x = 0;
	gint val_y = 0;
	gint info_ctr = 0;
	gint h = 0;
	gint i = 0;
	GArray *array = NULL;
	Viewable_Value *v_value = NULL;
	PangoLayout *layout;
	GdkPixmap *pixmap = lv_data->pixmap;
	GtkAllocation allocation;

	ENTER();
	gtk_widget_get_allocation(lv_data->darea,&allocation);

	h = allocation.height;

	if (!lv_data->font_desc)
	{
		lv_data->font_desc = pango_font_description_from_string("courier");
		pango_font_description_set_size(lv_data->font_desc,(10)*PANGO_SCALE);
	}
	
	val_x = 7;
	for (i=0;i<lv_data->active_traces;i++)
	{
		v_value = (Viewable_Value *)g_list_nth_data(lv_data->tlist,i);
		info_ctr = (lv_data->spread * (i+1))- (lv_data->spread/2);
		val_y = info_ctr + 1;

		last_index = v_value->last_index;
		array = (GArray *)DATA_GET(v_value->object,v_value->data_source);
		val = g_array_index(array,gfloat,last_index);
		if (array->len > 1)
			last_val = g_array_index(array,gfloat,last_index-1);
		/* IF this value matches the last one,  don't bother
		 * updating the text as there's no point... */
		if ((val == last_val) && (!force_draw) && (!v_value->force_update))
			continue;
		
		v_value->force_update = FALSE;
		gdk_draw_rectangle(pixmap,
				gtk_widget_get_style(lv_data->darea)->black_gc,
				TRUE,
				v_value->ink_rect->x+val_x,
				v_value->ink_rect->y+val_y,
				lv_data->info_width-1-v_value->ink_rect->x-val_x,
				v_value->ink_rect->height);

		layout = gtk_widget_create_pango_layout(lv_data->darea,g_strdup_printf("%1$.*2$f",val,v_value->precision));

		pango_layout_set_font_description(layout,lv_data->font_desc);
		pango_layout_get_pixel_extents(layout,v_value->ink_rect,v_value->log_rect);
		gdk_draw_layout(pixmap,v_value->font_gc,val_x,val_y,layout);
	}

	EXIT();
	return;
}


/*!
  \brief update_logview_traces_pf() updates each trace in turn and then scrolls 
  the display
  \param force_redraw flag to force all data to be redrawn not 
  just the new data...
  \returns TRUE
  */
G_MODULE_EXPORT gboolean update_logview_traces_pf(gboolean force_redraw)
{
	ENTER();
	if (DATA_GET(global_data,"playback_mode"))
	{
		EXIT();
		return TRUE;
	}

	if (!((DATA_GET(global_data,"connected")) && 
				(DATA_GET(global_data,"interrogated"))))
	{
		EXIT();
		return FALSE;
	}
	
	if (!lv_data)
	{
		EXIT();
		return FALSE;
	}

	if ((lv_data->traces) && (g_list_length(lv_data->tlist) > 0))
	{
		adj_scale = TRUE;
		g_mutex_lock(&update_mutex);
		trace_update(force_redraw);
		g_mutex_unlock(&update_mutex);
		scroll_logviewer_traces();
	}

	EXIT();
	return TRUE;
}


/* 
 * \brief wrapper for pb_update_logview_traces
 * */
G_MODULE_EXPORT gboolean pb_update_logview_traces_wrapper(gpointer data)
{
	ENTER();
	g_idle_add(pb_update_logview_traces,data);
	EXIT();
	return FALSE;
}


/*!
  \brief pb_update_logview_traces() updates each trace in turn and then scrolls 
  the display
  \param force_redraw flag to force all data to be redrawn not 
  just the new data...
  \returns TRUE
  */
G_MODULE_EXPORT gboolean pb_update_logview_traces(gpointer data)
{
	gboolean force_redraw = (GBOOLEAN)data;
	ENTER();

	if (!DATA_GET(global_data,"playback_mode"))
	{
		EXIT();
		return FALSE;
	}
	if ((lv_data->traces) && (g_list_length(lv_data->tlist) > 0))
	{
		adj_scale = TRUE;
		g_mutex_lock(&update_mutex);
		trace_update(force_redraw);
		g_mutex_unlock(&update_mutex);
		scroll_logviewer_traces();
	}
	EXIT();
	return FALSE;
}


/*!
  \brief trace_update() updates a trace onscreen,  this is run for EACH 
  individual trace (yeah, not very optimized)
  \param redraw_all flag to redraw all or just recent data
  */
G_MODULE_EXPORT void trace_update(gboolean redraw_all)
{
	GdkPixmap * pixmap = NULL;
	gint w = 0;
	gint h = 0;
	gfloat val = 0.0;
	gfloat last_val = 0.0;
	gfloat percent = 0.0;
	gfloat last_percent = 0.0;
	gint len = 0;
	guint last_index = 0;
	guint i = 0;
	gfloat log_pos = 0.0;
	gfloat newpos = 0.0;
	GArray *array = NULL;
	GdkPoint pts[2048]; /* Bad idea as static...*/
	Viewable_Value *v_value = NULL;
	gint lv_zoom;
	/*static gulong sig_id = 0;*/
	static GtkWidget *scale = NULL;
	GtkAllocation allocation;

	ENTER();
	gtk_widget_get_allocation(lv_data->darea,&allocation);

	pixmap = lv_data->pixmap;

	lv_zoom = (GINT)DATA_GET(global_data,"lv_zoom");
	/*
	if (sig_id == 0)
		sig_id = g_signal_handler_find(lookup_widget("logviewer_log_position_hscale"),G_SIGNAL_MATCH_FUNC,0,0,NULL,(gpointer)logviewer_log_position_change,NULL);
		*/

	if (!scale)
		scale = lookup_widget("logviewer_log_position_hscale");
	w = allocation.width;
	h = allocation.height;

	log_pos = (gfloat)((GINT)OBJ_GET(lv_data->darea,"log_pos_x100"))/100.0;
	/*printf("log_pos is %f\n",log_pos);*/
	/* Full screen redraw, only with configure events (usually) */
	if ((GBOOLEAN)redraw_all)
	{
		gint lo_width = allocation.width-lv_data->info_width;
		for (i=0;i<g_list_length(lv_data->tlist);i++)
		{
			gint total = 0;
			v_value = (Viewable_Value *)g_list_nth_data(lv_data->tlist,i);
			array = (GArray *)DATA_GET(v_value->object,v_value->data_source);
			len = array->len;
			if (len == 0)	/* If empty */
			{
				EXIT();
				return;
			}
			/*printf("length is %i\n", len);*/
			len *= (log_pos/100.0);
			/*printf("length after is  %i\n", len);*/
			/* Determine total number of points 
			 * that'll fit on the window
			 * taking into account the scroll amount
			 */
			total = len < lo_width/lv_zoom ? len : lo_width/lv_zoom;


			/* Draw is reverse order, from right to left, 
			 * easier to think out in my head... :) 
			 */
			for (gint x=0;x<total;x++)
			{
				val = g_array_index(array,gfloat,len-1-x);
				percent = 1.0-(val/(float)(v_value->upper-v_value->lower));
				pts[x].x = w-(x*lv_zoom)-1;
				pts[x].y = (GINT) (percent*(h-2))+1;
			}
			gdk_draw_lines(pixmap,
					v_value->trace_gc,
					pts,
					total);
			if (v_value->highlight)
			{
				gint j = 0;
				for (j=0;j<total;j++)	
					pts[j].y -= 1;
				gdk_draw_lines(pixmap,
						gtk_widget_get_style(lv_data->darea)->white_gc,
						pts,
						total);
				for (j=0;j<total;j++)	
					pts[j].y += 2;
				gdk_draw_lines(pixmap,
						gtk_widget_get_style(lv_data->darea)->white_gc,
						pts,
						total);
			}

			v_value->last_y = pts[0].y;
			v_value->last_index = len-1;

			/*printf ("last index displayed was %i from %i,%i to %i,%i\n",v_value->last_index,pts[1].x,pts[1].y, pts[0].x,pts[0].y );*/
		}
		draw_valtext(TRUE);
		/*printf("redraw complete\n");*/
		EXIT();
		return;
	}
	/* Playback mode, playing from logfile.... */
	if (DATA_GET(global_data,"playback_mode"))
	{
		for (i=0;i<g_list_length(lv_data->tlist);i++)
		{
			v_value = (Viewable_Value *)g_list_nth_data(lv_data->tlist,i);
			array = (GArray *)DATA_GET(v_value->object,v_value->data_source);
			last_index = v_value->last_index;
			if(last_index >= array->len)
			{
				EXIT();
				return;
			}

			/*printf("got data from array at index %i\n",last_index+1);*/
			val = g_array_index(array,gfloat,last_index+1);
			percent = 1.0-(val/(float)(v_value->upper-v_value->lower));
			if (val > (v_value->max))
				v_value->max = val;
			if (val < (v_value->min))
				v_value->min = val;

			gdk_draw_line(pixmap,
					v_value->trace_gc,
					w-lv_zoom-1,v_value->last_y,
					w-1,(GINT)(percent*(h-2))+1);
			/*printf("drawing from %i,%i to %i,%i\n",w-lv_zoom-1,v_value->last_y,w-1,(GINT)(percent*(h-2))+1);*/

			v_value->last_y = (GINT)((percent*(h-2))+1);

			v_value->last_index = last_index + 1;
			if (adj_scale)
			{
				newpos = 100.0*((gfloat)(v_value->last_index)/(gfloat)array->len);
				blocked=TRUE;
				gtk_range_set_value(GTK_RANGE(scale),newpos);
				blocked=FALSE;
				OBJ_SET(lv_data->darea,"log_pos_x100",GINT_TO_POINTER((GINT)(newpos*100.0)));
				adj_scale = FALSE;
				if (newpos >= 100)
					stop_tickler(LV_PLAYBACK_TICKLER);
				/*	printf("playback reset slider to position %i\n",(GINT)(newpos*100.0));*/
			}
			if (v_value->highlight)
			{
				gdk_draw_line(pixmap,
						gtk_widget_get_style(lv_data->darea)->white_gc,
						w-lv_zoom-1,v_value->last_y-1,
						w-1,(GINT)(percent*(h-2)));
				gdk_draw_line(pixmap,
						gtk_widget_get_style(lv_data->darea)->white_gc,
						w-lv_zoom-1,v_value->last_y+1,
						w-1,(GINT)(percent*(h-2))+2);
			}
		}
		draw_valtext(FALSE);
		EXIT();
		return;
	}

	/* REALTIME mode... all traces updated at once.. */
	for (i=0;i<g_list_length(lv_data->tlist);i++)
	{
		v_value = (Viewable_Value *)g_list_nth_data(lv_data->tlist,i);
		array = (GArray *)DATA_GET(v_value->object,v_value->data_source);
		val = g_array_index(array,gfloat, array->len-1);

		if (val > (v_value->max))
			v_value->max = val;
		if (val < (v_value->min))
			v_value->min = val;

		if (v_value->last_y == -1)
			v_value->last_y = (GINT)((percent*(h-2))+1);

		/* If watching at the edge (full realtime) */
		if (log_pos >= 100)
		{
			v_value->last_index = array->len-1;
			percent = 1.0-(val/(float)(v_value->upper-v_value->lower));
			gdk_draw_line(pixmap,
					v_value->trace_gc,
					w-lv_zoom-1,v_value->last_y,
					w-1,(GINT)(percent*(h-2))+1);
		}
		else
		{	/* Watching somewhat behind realtime... */
			last_index = v_value->last_index;

			last_val = g_array_index(array,gfloat,last_index);
			last_percent = 1.0-(last_val/(float)(v_value->upper-v_value->lower));
			val = g_array_index(array,gfloat,last_index+1);
			percent = 1.0-(val/(float)(v_value->upper-v_value->lower));

			v_value->last_index = last_index + 1;
			gdk_draw_line(pixmap,
					v_value->trace_gc,
					w-lv_zoom-1,(last_percent*(h-2))+1,
					w-1,(GINT)(percent*(h-2))+1);
			if (adj_scale)
			{
				newpos = 100.0*((gfloat)v_value->last_index/(gfloat)array->len);
				blocked = TRUE;
				gtk_range_set_value(GTK_RANGE(scale),newpos);
				blocked = FALSE;
				OBJ_SET(lv_data->darea,"log_pos_x100",GINT_TO_POINTER((GINT)(newpos*100.0)));
				adj_scale = FALSE;
			}
		}
		/* Draw the data.... */
		v_value->last_y = (GINT)((percent*(h-2))+1);
		if (v_value->highlight)
		{
			gdk_draw_line(pixmap,
					gtk_widget_get_style(lv_data->darea)->white_gc,
					w-lv_zoom-1,v_value->last_y-1,
					w-1,(GINT)(percent*(h-2)));
			gdk_draw_line(pixmap,
					gtk_widget_get_style(lv_data->darea)->white_gc,
					w-lv_zoom-1,v_value->last_y+1,
					w-1,(GINT)(percent*(h-2))+2);
		}
	}
	/* Update textual data */
	draw_valtext(FALSE);
	EXIT();
	return;
}


/*!
  \brief scroll_logviewer_traces() scrolls the traces to the left
  */
G_MODULE_EXPORT void scroll_logviewer_traces(void)
{
	gint start = lv_data->info_width;
	gint end = lv_data->info_width;
	gint w = 0;
	gint h = 0;
	gint lv_zoom = 0;
	GdkPixmap *pixmap = NULL;
	GdkPixmap *pmap = NULL;
	static GtkWidget * widget = NULL;
	GtkAllocation allocation;
	GdkWindow *window = NULL;

	ENTER();
	if (!widget)
		widget = lookup_widget("logviewer_trace_darea");
	if (!widget)
	{
		EXIT();
		return;
	}
	window = gtk_widget_get_window(widget);
	gtk_widget_get_allocation(widget,&allocation);
	pixmap = lv_data->pixmap;
	pmap = lv_data->pmap;
	if (!pixmap)
	{
		EXIT();
		return;
	}

	lv_zoom = (GINT)DATA_GET(global_data,"lv_zoom");
	w = allocation.width;
	h = allocation.height;
	start = end + lv_zoom;

	/* NASTY NASTY NASTY win32 hack to get it to scroll because
	 * draw_drawable seems to fuckup on windows when souce/dest are 
	 * in the same widget...  This works however on EVERY OTHER
	 * OS where GTK+ runs.  grr.....
	 */
#ifdef __WIN32__
	/* Scroll the screen to the left... */
	gdk_draw_drawable(pmap,
			gtk_widget_get_style(widget)->black_gc,
			pixmap,
			lv_data->info_width+lv_zoom,0,
			lv_data->info_width,0,
			w-lv_data->info_width-lv_zoom,h);

	gdk_draw_drawable(pixmap,
			gtk_widget_get_style(widget)->black_gc,
			pmap,
			lv_data->info_width,0,
			lv_data->info_width,0,
			w-lv_data->info_width,h);
#else

	/* Scroll the screen to the left... */
	gdk_draw_drawable(pixmap,
			gtk_widget_get_style(widget)->black_gc,
			pixmap,
			lv_data->info_width+lv_zoom,0,
			lv_data->info_width,0,
			w-lv_data->info_width-lv_zoom,h);
#endif

	/* Init new "blank space" as black */
	gdk_draw_rectangle(pixmap,
			gtk_widget_get_style(widget)->black_gc,
			TRUE,
			w-lv_zoom,0,
			lv_zoom,h);

	gdk_window_clear(window);
	EXIT();
	return;
}


/*!
  \brief set_all_lview_choices_state() sets all the logables to either be 
  selected or deselected (select all fucntionality)
  \param widget is  unused
  \param data The state to set the widgets to
  \returns TRUE
  */
G_MODULE_EXPORT gboolean set_all_lview_choices_state(GtkWidget *widget, gpointer data)
{
	gboolean state = (GBOOLEAN)data;

	ENTER();
	g_list_foreach(get_list("viewables"),set_widget_active,GINT_TO_POINTER(state));

	EXIT();
	return TRUE;
}


/*!
  \brief set_default_lview_choices_state() sets the default logviewer values
  */
G_MODULE_EXPORT void set_default_lview_choices_state(void)
{
	GList *defaults = NULL;
	GList *list = NULL;
	GtkWidget * widget = NULL;
	guint i = 0;
	guint j = 0;
	gchar * name;
	gchar * potential;
	gconstpointer *object;

	ENTER();
	defaults = get_list("logviewer_defaults");
	list = get_list("viewables");
	for (i=0;i<g_list_length(defaults);i++)
	{
		name = (gchar *)g_list_nth_data(defaults,i);
		for (j=0;j<g_list_length(list);j++)
		{
			widget = (GtkWidget *)g_list_nth_data(list,j);
			object = (gconstpointer *)OBJ_GET(widget,"object");
			if (DATA_GET(global_data,"playback_mode"))
				potential = (gchar *)DATA_GET(object,"lview_name");
			else
				potential = (gchar *)DATA_GET(object,"dlog_gui_name");
			if (g_ascii_strcasecmp(name,potential) == 0)

				set_widget_active(GTK_WIDGET(widget),GINT_TO_POINTER(TRUE));
		}
	}
	EXIT();
	return;
}


/*!
  \brief logviewer_log_position_change() gets called when the log position 
  slider is moved by the user to fast forware/rewind through a datalog
  \param widget is the widget that received the event
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean logviewer_log_position_change(GtkWidget * widget, gpointer data)
{
	gfloat val = 0.0;
	GtkWidget *darea = NULL;

	ENTER();
	/* If we pass "TRUE" as the widget data we just ignore this signal as 
	 * the redraw routine wil have to adjsut the slider as it scrolls 
	 * through the data...
	 */
	if ((GBOOLEAN)data)
	{
		EXIT();
		return TRUE;
	}
	if (blocked)
	{
		EXIT();
		return TRUE;
	}

	val = gtk_range_get_value(GTK_RANGE(widget));
	darea = lookup_widget("logviewer_trace_darea");
	if (GTK_IS_WIDGET(darea))
		OBJ_SET(darea,"log_pos_x100",GINT_TO_POINTER((GINT)(val*100)));
	lv_configure_event(darea,NULL,NULL);
	scroll_logviewer_traces();
	if ((val >= 100.0) && (DATA_GET(global_data,"playback_mode")))
		stop_tickler(LV_PLAYBACK_TICKLER);
	EXIT();
	return TRUE;
}


/*!
  \brief Enable log playback controls
  \param state whether to enable or disable the playback controls
  */
G_MODULE_EXPORT void enable_playback_controls(gboolean state)
{	
	static GtkWidget * playback_controls_window = NULL;
	gchar *fname = NULL;
	gchar * filename = NULL;
	GladeXML *xml = NULL;
	GtkWidget *widget = NULL;
	GtkAdjustment *adj = NULL;
	ENTER();
	if (state) /* show the controls */
	{
		if (!GTK_IS_WIDGET(playback_controls_window))
		{
			fname = g_build_filename(GUI_DATA_DIR,"logviewer.glade",NULL);
			filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),fname,NULL);
			if (filename)
			{
				xml = glade_xml_new(filename, "logviewer_controls_window",NULL);
				g_free(filename);
				g_free(fname);
				glade_xml_signal_autoconnect(xml);
				playback_controls_window = glade_xml_get_widget(xml,"logviewer_controls_window");
				OBJ_SET(glade_xml_get_widget(xml,"goto_start_button"),"handler",GINT_TO_POINTER(LV_GOTO_START));
				OBJ_SET(glade_xml_get_widget(xml,"goto_end_button"),"handler",GINT_TO_POINTER(LV_GOTO_END));
				OBJ_SET(glade_xml_get_widget(xml,"rewind_button"),"handler",GINT_TO_POINTER(LV_REWIND));
				OBJ_SET(glade_xml_get_widget(xml,"fast_forward_button"),"handler",GINT_TO_POINTER(LV_FAST_FORWARD));
				OBJ_SET(glade_xml_get_widget(xml,"play_button"),"handler",GINT_TO_POINTER(LV_PLAY));
				OBJ_SET(glade_xml_get_widget(xml,"stop_button"),"handler",GINT_TO_POINTER(LV_STOP));
				register_widget("logviewer_controls_hbox",glade_xml_get_widget(xml,"controls_hbox"));
				widget = lookup_widget("logviewer_scroll_hscale");
				if (GTK_IS_WIDGET(widget))
				{
					adj = gtk_range_get_adjustment(GTK_RANGE(widget));
					gtk_range_set_adjustment(GTK_RANGE(glade_xml_get_widget(xml,"scroll_speed")),adj);
				}
				widget = lookup_widget("logviewer_log_position_hscale");
				if (GTK_IS_WIDGET(widget))
				{
					adj = gtk_range_get_adjustment(GTK_RANGE(widget));
					gtk_range_set_adjustment(GTK_RANGE(glade_xml_get_widget(xml,"log_position_hscale")),adj);
				}
			}
			register_widget("playback_controls_window",playback_controls_window);
		}
		else
			gtk_widget_show_all(playback_controls_window);
	}
	else
	{
		if (GTK_IS_WIDGET(playback_controls_window))
			gtk_widget_hide(playback_controls_window);
	}
	EXIT();
	return;
}


/*!
  \brief set_logviewer_mode() sets things up for playback mode
  \param mode Enumeration defining the logviewr mode (live or playback)
  */
G_MODULE_EXPORT void set_logviewer_mode(Lv_Mode mode)
{
	GtkWidget *widget = NULL;

	ENTER();
	reset_logviewer_state();
	free_log_info((Log_Info *)DATA_GET(global_data,"log_info"));
	if (mode == LV_PLAYBACK)
	{
		DATA_SET(global_data,"playback_mode",GINT_TO_POINTER(TRUE));
		gtk_widget_set_sensitive(lookup_widget("logviewer_select_logfile_button"), TRUE);
		gtk_widget_set_sensitive(lookup_widget("logviewer_select_params_button"), FALSE);
		gtk_widget_hide(lookup_widget("logviewer_rt_control_vbox1"));
		/* This one should NOT be enabled until at least 1 var is selected */
		gtk_widget_show(lookup_widget("logviewer_playback_control_vbox1"));
		gtk_widget_show(lookup_widget("scroll_speed_vbox"));

		widget = lookup_widget("logviewer_log_position_hscale");
		if (GTK_IS_RANGE(widget))
			gtk_range_set_value(GTK_RANGE(widget),0.0);
		hue = -60.0;
		col_sat = 1.0;
		col_val = 1.0;
	}
	else if (mode == LV_REALTIME)
	{
		enable_playback_controls(FALSE);

		stop_tickler(LV_PLAYBACK_TICKLER);
		DATA_SET(global_data,"playback_mode",GINT_TO_POINTER(FALSE));
		gtk_widget_set_sensitive(lookup_widget("logviewer_select_logfile_button"), FALSE);
		gtk_widget_set_sensitive(lookup_widget("logviewer_select_params_button"), TRUE);
		gtk_widget_show(lookup_widget("logviewer_rt_control_vbox1"));
		gtk_widget_hide(lookup_widget("logviewer_playback_control_vbox1"));
		gtk_widget_hide(lookup_widget("scroll_speed_vbox"));
		widget = lookup_widget("logviewer_log_position_hscale");
		if (GTK_IS_RANGE(widget))
			gtk_range_set_value(GTK_RANGE(widget),100.0);
		hue = -60.0;
		col_sat = 1.0;
		col_val = 1.0;
	}
	EXIT();
}


/*!
  \brief finish_logviewer() sets button default states for the logviewer after
  it is created from it's glade config file
  */
G_MODULE_EXPORT void finish_logviewer(void)
{
	GtkWidget * widget = NULL;
	gint lv_zoom = 0;

	ENTER();
	lv_zoom = (GINT)DATA_GET(global_data,"lv_zoom");

	lv_data = g_new0(Logview_Data,1);
	lv_data->traces = g_hash_table_new(g_str_hash,g_str_equal);
	lv_data->info_width = 120;

	//if ((DATA_GET(global_data,"playback_mode")) || (DATA_GET(global_data,"offline")))
	if ((DATA_GET(global_data,"playback_mode")))
		set_logviewer_mode(LV_PLAYBACK);
	else
		set_logviewer_mode(LV_REALTIME);

	if (DATA_GET(global_data,"offline"))
	{
		widget = lookup_widget("logviewer_realtime_radio_button");
		gtk_widget_set_sensitive(widget,FALSE);
		widget = lookup_widget("logviewer_playback_radio_button");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
	}
	widget = lookup_widget("logviewer_zoom_spinner");
	if (GTK_IS_SPIN_BUTTON(widget))
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),lv_zoom);

	EXIT();
	return;
}


/*!
  \brief slider_ket_press_event() doesn't do anything yet (stub)
  \param widget is unused
  \param event is unused
  \param data is unused
  \returns FALSE
  */
G_MODULE_EXPORT gboolean slider_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	ENTER();
	EXIT();
	return FALSE;
}



/*!
  \brief write logviewer defaults to file
  \param cfgfile is the pointer to ConfigFile structure to save the settings to
  */
G_MODULE_EXPORT void write_logviewer_defaults(ConfigFile *cfgfile)
{
	GList * list = NULL;
	gchar * name = NULL;
	GString *string = NULL;

	ENTER();
	list = get_list("logviewer_defaults");
	if (list)
	{
		string = g_string_new(NULL);
		for (guint i=0;i<g_list_length(list);i++)
		{
			name = (gchar *)g_list_nth_data(list,i);
			g_string_append(string,name);
			if (i < (g_list_length(list)-1))
				g_string_append(string,",");
		}
		cfg_write_string(cfgfile,"Logviewer","defaults",string->str);
		g_string_free(string,TRUE);
	}
	else
		cfg_write_string(cfgfile,"Logviewer","defaults","");
	EXIT();
	return;
}


/*!
  \brief read logviewer defaults from file
  \param cfgfile is the pointer to ConfigFile structure to read the settings from
  */
G_MODULE_EXPORT void read_logviewer_defaults(ConfigFile *cfgfile)
{
	gchar *tmpbuf = NULL;
	GList *defaults = NULL;
	gchar **vector = NULL;
	guint i = 0;

	ENTER();
	cfg_read_string(cfgfile,"Logviewer","defaults",&tmpbuf);
	if (!tmpbuf)
	{
		EXIT();
		return;
	}
	
	vector = g_strsplit(tmpbuf,",",-1);
	g_free(tmpbuf);
	for (i=0;i<g_strv_length(vector);i++)
		defaults = g_list_append(defaults,g_strdup(vector[i]));
	g_strfreev(vector);
	if (defaults)
		store_list("logviewer_defaults",defaults);

	EXIT();
	return;
}
