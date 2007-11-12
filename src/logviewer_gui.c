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
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <listmgmt.h>
#include <logviewer_core.h>
#include <logviewer_events.h>
#include <logviewer_gui.h>
#include <math.h>
#include <mode_select.h>
#include <structures.h>
#include <tabloader.h>
#include <timeout_handlers.h>

static gint max_viewables = 0;
static gboolean adj_scale = TRUE;
static gboolean blocked = FALSE;
static gfloat hue = -60.0;
static gfloat col_sat = 1.0;
static gfloat col_val = 1.0;
extern gint dbg_lvl;

Logview_Data *lv_data = NULL;
gint lv_zoom = 0;		/* logviewer scroll amount */
gboolean playback_mode = FALSE;
static GStaticMutex update_mutex = G_STATIC_MUTEX_INIT;
extern Log_Info *log_info;


/*!
 \brief present_viewer_choices() presents the user with the a list of 
 variabels form EIRHT the realtime vars (if in realtime mode) or from a 
 datalog (playback mode)
 */
void present_viewer_choices(void)
{
	GtkWidget *window;
	GtkWidget *table;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget *sep;
	GObject * object;
	extern GtkTooltips *tip;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint table_rows = 0;
	gint table_cols = 5;
	gchar * name = NULL;
	gchar * tooltip = NULL;
	GtkWidget *darea = NULL;
	extern GHashTable *dynamic_widgets;
	extern Rtv_Map *rtv_map;

	darea = g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea");
	lv_data->darea = darea;

	if (!darea)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": present_viewer_choices()\n\tpointer to drawing area was NULL, returning!!!\n"));
		return;
	}

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
	/* Playback mode..... */
	if (playback_mode)
	{
		gtk_window_set_title(GTK_WINDOW(window),
				"Playback Mode: Logviewer Choices");
		frame = gtk_frame_new("Select Variables to playback from the list below...");
		max_viewables = log_info->field_count;
	}
	else
	{
		/* Realtime Viewing mode... */
		gtk_window_set_title(GTK_WINDOW(window),
				"Realtime Mode: Logviewer Choices");
		frame = gtk_frame_new("Select Realtime Variables to view from the list below...");
		max_viewables = rtv_map->derived_total;

	}
	g_signal_connect_swapped(G_OBJECT(window),"destroy_event",
			G_CALLBACK(reenable_select_params_button),
			NULL);
	g_signal_connect_swapped(G_OBJECT(window),"destroy_event",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);
	g_signal_connect_swapped(G_OBJECT(window),"delete_event",
			G_CALLBACK(reenable_select_params_button),
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
		store_list("viewables",NULL);
	}

	for (i=0;i<max_viewables;i++)
	{
		object = NULL;
		name = NULL;
		tooltip = NULL;
		if (playback_mode)
		{
			object =  g_array_index(log_info->log_list,GObject *,i);
			name = g_strdup(g_object_get_data(object,"lview_name"));
		}
		else
		{
			object =  g_array_index(rtv_map->rtv_list,GObject *,i);
			name = g_strdup(g_object_get_data(object,"dlog_gui_name"));
			tooltip = g_strdup(g_object_get_data(object,"tooltip"));
		}

		button = gtk_check_button_new();
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),name);
		gtk_container_add(GTK_CONTAINER(button),label);
		store_list("viewables",g_list_prepend(
					get_list("viewables"),(gpointer)button));

		if (tooltip)
			gtk_tooltips_set_tip(tip,button,tooltip,NULL);

		if (object)
		{
			g_object_set_data(G_OBJECT(button),"object",
					(gpointer)object);
			// so we can set the state from elsewhere... 
			g_object_set_data(G_OBJECT(object),"lview_button",
					(gpointer)button);
			if ((gboolean)g_object_get_data(object,"being_viewed"))
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

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox),sep,FALSE,TRUE,20);

	hbox = gtk_hbox_new(FALSE,20);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);
	button = gtk_button_new_with_label("Select All");
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,15);
	g_object_set_data(G_OBJECT(button),"state",GINT_TO_POINTER(TRUE));
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(set_lview_choices_state),
			GINT_TO_POINTER(TRUE));
	button = gtk_button_new_with_label("De-select All");
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,15);
	g_object_set_data(G_OBJECT(button),"state",GINT_TO_POINTER(FALSE));
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(set_lview_choices_state),
			GINT_TO_POINTER(FALSE));

	button = gtk_button_new_with_label("Close");
	gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(reenable_select_params_button),
			NULL);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);

	gtk_widget_show_all(window);
	return;
}


gboolean reenable_select_params_button(GtkWidget *widget)
{
	extern GHashTable *dynamic_widgets;
	gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button")),TRUE);
	return FALSE;

}

/*!
 \brief view_value_set() is called when a value to be viewed is selected
 or not. We tag the widget with a marker if it is to be displayed
 \param widget (GtkWidget *) button clicked, we extract the object this
 represents and mark it
 \param data (gpointer) unused
 */
gboolean view_value_set(GtkWidget *widget, gpointer data)
{
	GObject *object = NULL;
	gboolean state = FALSE;


	state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widget));

	/* get object from widget */
	object = (GObject *)g_object_get_data(G_OBJECT(widget),"object");
	if (!object)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": view_value_set()\n\t NO object was bound to the button\n"));
	}
	g_object_set_data(object,"being_viewed",GINT_TO_POINTER(state));
	populate_viewer();
	return FALSE;
}


/*!
 \brief populate_viewer() creates/removes the list of viewable values from
 the objects in use (playback list or realtiem vars list)
 */
void populate_viewer()
{
	gint i = 0;
	gint total = 0;
	Viewable_Value *v_value = NULL;
	gchar * name = NULL;
	gboolean being_viewed = FALSE;
	extern Rtv_Map *rtv_map;
	extern GHashTable *dynamic_widgets;
	GObject *object = NULL;

	g_static_mutex_lock(&update_mutex);

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
	if (playback_mode)
		total = log_info->field_count;
	else
		total = rtv_map->derived_total;

	for (i=0;i<total;i++)
	{
		object = NULL;
		name = NULL;
		if (playback_mode)
		{
			object = g_array_index(log_info->log_list,GObject *, i);        
			name = g_strdup(g_object_get_data(object,"lview_name"));
		}
		else
		{
			object = g_array_index(rtv_map->rtv_list,GObject *, i); 
			name = g_strdup(g_object_get_data(object,"dlog_gui_name"));
		}
		if (!name)
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup("ERROR, name is NULL\n"));
		}
		if (!object)
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup("ERROR, object is NULL\n"));
		}

		being_viewed = (gboolean)g_object_get_data(object,"being_viewed");
		/* if not found in table check to see if we need to insert*/
		if (g_hash_table_lookup(lv_data->traces,name)==NULL)
		{
			if (being_viewed)	/* Marked viewable widget */
			{
				/* Call the build routine, feed it the drawing_area*/
				v_value = build_v_value(object);
				// store location of master
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
				if ((hue > 0) && ((gint)hue%1110 == 0))
				{
					hue-= 30;
					col_sat = 1.0;
					col_val = 0.75;
				//	printf("hue at 1110 deg, reducing to 1080, sat at 1.0, val at 0.75\n");
				}
				if ((hue > 0) && ((gint)hue%780 == 0))
				{
					hue-= 30;
					col_sat = 0.5;
					col_val = 1.0;
				//	printf("hue at 780 deg, reducing to 750, sat at 0.5, val at 1.0\n");
				}
				if ((hue > 0) && ((gint)hue%390 == 0)) /* phase shift */
				{
					hue-=30.0;
					col_sat=1.0;
					col_val = 1.0;
				//	printf("hue at 390 deg, reducing to 360, sat at 0.5, val at 1.0\n");
				}
				hue -=60;
			//	printf("angle at %f, sat %f, val %f\n",hue,col_sat,col_val);

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
	g_free(name);
	lv_data->active_traces = g_hash_table_size(lv_data->traces);
	/* If traces selected, emit a configure_Event to clear the window
	 * and draw the traces (IF ONLY reading a log for playback)
	 */
	g_static_mutex_unlock(&update_mutex);
	if ((lv_data->traces) && (g_list_length(lv_data->tlist) >= 0))
		lv_configure_event(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea"),NULL,NULL);

	return; 
}


/*!
 \brief reset_logviewer_state() deselects any traces, resets the position 
 slider.  This function is called when switching from playback to rt mode
 and back
 */
void reset_logviewer_state()
{
	extern Rtv_Map *rtv_map;
	extern Log_Info *log_info;
	gint i = 0 ;
	GObject * object = NULL;

	if (playback_mode)
	{
		if (!log_info)
			return;
		for (i=0;i<log_info->field_count;i++)
		{
			object = NULL;
			object = g_array_index(log_info->log_list,GObject *,i);
			if (object)
				g_object_set_data(object,"being_viewed",GINT_TO_POINTER(FALSE));
		}
	}
	else
	{
		if (!rtv_map)
			return;
		for (i=0;i<rtv_map->derived_total;i++)
		{
			object = NULL;
			object = g_array_index(rtv_map->rtv_list,GObject *,i);
			if (object)
				g_object_set_data(object,"being_viewed",GINT_TO_POINTER(FALSE));
		}
	}
	populate_viewer();

}


/*!
 \brief build_v_value() allocates a viewable_value structure and populates
 it with sane defaults and returns it to the caller
 \param object (GObject *) objet to get soem of the data from
 \returns a newly allocated and populated Viewable_Value structure
 */
Viewable_Value * build_v_value(GObject *object)
{
	Viewable_Value *v_value = NULL;
	GdkPixmap *pixmap =  NULL;

	pixmap = lv_data->pixmap;

	v_value = g_malloc(sizeof(Viewable_Value));		

	/* Set limits of this variable. (it's ranges, used for scaling */

	if (playback_mode)
	{
		/* textual name of the variable we're viewing.. */
		v_value->vname = g_strdup(g_object_get_data(object,"lview_name"));
		/* data was already read from file and stored, copy pointer
		 * over to v_value so it can be drawn...
		 */
		v_value->data_source = g_strdup("data_array");
	}
	else
	{
		// textual name of the variable we're viewing.. 
		v_value->vname = g_strdup(g_object_get_data(object,"dlog_gui_name"));
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
	v_value->is_float = (gboolean)g_object_get_data(object,"is_float");
	v_value->lower = (gint)g_object_get_data(object,"lower_limit");
	v_value->upper = (gint)g_object_get_data(object,"upper_limit");
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

	return v_value;
}


/*!
 \brief initialize_gc() allocates and initializes the graphics contexts for
 the logviewer trace window.
 \param drawable (GdkDrawable *) pointer to the drawable surface
 \param type (GcType) Graphics Context type? (I donno for sure)
 \returns Pointer to a GdkGC *
 */
GdkGC * initialize_gc(GdkDrawable *drawable, GcType type)
{
	GdkColor color;
	GdkGC * gc = NULL;
	GdkGCValues values;
	GdkColormap *cmap = NULL;

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
			//printf("angle at %f, sat %f, val %f\n",hue,col_sat,col_val);

			if ((hue > 0) && ((gint)hue%360 == 0))
			{
				hue+=30.0;
				col_sat=0.5;
				col_val=1.0;
			}
			if ((hue > 0) && ((gint)hue%750 == 0))
			{
				hue+=30;
				col_sat=1.0;
				col_val = 0.75;
			}
			//printf("JBA angle at %f, sat %f, val %f\n",hue,col_sat,col_val);
			color = (GdkColor)  get_colors_from_hue(hue,col_sat,col_val);
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
	return gc;	
}


/*!
 \brief get_colors_from_hue(gets a color back from an angle passed in degrees.
 The degrees represent the arc aroudn a color circle.
 \param hue (gfloat) degrees around the color circle
 \param sat (gfloat) col_sat from 0-1.0
 \param val (gfloat) col_val from 0-1.0
 \returns a GdkColor at the hue angle requested
 */
GdkColor get_colors_from_hue(gfloat hue, gfloat sat, gfloat val)
{
	GdkColor color;
	color.pixel=0;
	gfloat tmp = 0.0;	
	gint i = 0;
	gfloat fract = 0.0;
	gfloat S = sat;	// using col_sat of 1.0
	gfloat V = val;	// using Value of 1.0
	gfloat p = 0.0;
	gfloat q = 0.0;
	gfloat t = 0.0;
	gfloat r = 0.0;
	gfloat g = 0.0;
	gfloat b = 0.0;

	while (hue > 360.0)
		hue -= 360.0;
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

	return (color);	
}


/*!
 \brief draw_infotext() draws the static textual data for the trace on 
 the left hand side of the logviewer
 */
void draw_infotext()
{
	// Draws the textual (static) info on the left side of the window..

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
	PangoLayout *layout;
	GdkPixmap *pixmap = lv_data->pixmap;

	h = lv_data->darea->allocation.height;

	gdk_draw_rectangle(pixmap,
			lv_data->darea->style->black_gc,
			TRUE, 0,0,
			lv_data->info_width,h);


	if (!lv_data->font_desc)
	{
		lv_data->font_desc = pango_font_description_from_string("courier");
		pango_font_description_set_size(lv_data->font_desc,(10)*PANGO_SCALE);
	}
	if (!lv_data->highlight_gc)
		lv_data->highlight_gc = initialize_gc(lv_data->pixmap,HIGHLIGHT);
	
	lv_data->spread = (gint)((float)h/(float)lv_data->active_traces);
	name_x = text_border;
	for (i=0;i<lv_data->active_traces;i++)
	{
		v_value = (Viewable_Value *)g_list_nth_data(lv_data->tlist,i);
		info_ctr = (lv_data->spread * (i+1))- (lv_data->spread/2);

		layout = gtk_widget_create_pango_layout(lv_data->darea,NULL);
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
				lv_data->darea->style->white_gc,
				FALSE, 0,i*lv_data->spread,
				lv_data->info_width-1,lv_data->spread);
	}

}


/*!
 \brief draw_valtext() draws the dynamic values for the traces on 
 the left hand side of the logviewer. This is optimized so that if the value
 becomes temporarily static, it won't keep blindly updating the screen and
 wasting CPU time.
 \param force_draw (gboolean) when true to write the values to screen for
 all controls no matter if hte previous value is the same or not.
 */
void draw_valtext(gboolean force_draw)
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

	h = lv_data->darea->allocation.height;

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
		array = g_object_get_data(G_OBJECT(v_value->object),v_value->data_source);
		val = g_array_index(array,gfloat,last_index);
		if (array->len > 1)
			last_val = g_array_index(array,gfloat,last_index-1);
		/* IF this value matches the last one,  don't bother
		 * updating the text as there's no point... */
		if ((val == last_val) && (!force_draw) && (!v_value->force_update))
			continue;
		
		v_value->force_update = FALSE;
		gdk_draw_rectangle(pixmap,
				lv_data->darea->style->black_gc,
				TRUE,
				v_value->ink_rect->x+val_x,
				v_value->ink_rect->y+val_y,
				lv_data->info_width-1-v_value->ink_rect->x-val_x,
				v_value->ink_rect->height);

		if (v_value->is_float)
			layout = gtk_widget_create_pango_layout(lv_data->darea,g_strdup_printf("%.3f",val));
		else
			layout = gtk_widget_create_pango_layout(lv_data->darea,g_strdup_printf("%i",(gint)val));

		pango_layout_set_font_description(layout,lv_data->font_desc);
		pango_layout_get_pixel_extents(layout,v_value->ink_rect,v_value->log_rect);
		gdk_draw_layout(pixmap,v_value->font_gc,val_x,val_y,layout);
	}

}


/*!
 \brief rt_update_logview_traces() updates each trace in turn and then scrolls 
 the display
 \param force_redraw (gboolean) flag to force all data to be redrawn not 
 just the new data...
 \returns TRUE
 */
gboolean rt_update_logview_traces(gboolean force_redraw)
{

	if (playback_mode)
		return TRUE;
	if ((lv_data->traces) && (g_list_length(lv_data->tlist) > 0))
	{
		adj_scale = TRUE;
		g_static_mutex_lock(&update_mutex);
		trace_update(force_redraw);
		g_static_mutex_unlock(&update_mutex);
		scroll_logviewer_traces();
	}

	return TRUE;
}


/*!
 \brief pb_update_logview_traces() updates each trace in turn and then scrolls 
 the display
 \param force_redraw (gboolean) flag to force all data to be redrawn not 
 just the new data...
 \returns TRUE
 */
gboolean pb_update_logview_traces(gboolean force_redraw)
{

	if (!playback_mode)
		return TRUE;
	if ((lv_data->traces) && (g_list_length(lv_data->tlist) > 0))
	{
		adj_scale = TRUE;
		g_static_mutex_lock(&update_mutex);
		trace_update(force_redraw);
		g_static_mutex_unlock(&update_mutex);
		scroll_logviewer_traces();
	}

	return TRUE;
}


/*!
 \brief trace_update() updates a trace onscreen,  this is run for EACH 
 individual trace (yeah, not very optimized)
 \param redraw_all (gpointer) flag to redraw all or just recent data
 */
void trace_update(gboolean redraw_all)
{
	GdkPixmap * pixmap = NULL;
	gint w = 0;
	gint h = 0;
	gfloat val = 0.0;
	gfloat last_val = 0.0;
	gfloat percent = 0.0;
	gfloat last_percent = 0.0;
	gint current_index = 0;
	gint len = 0;
	gint lo_width;
	gint total = 0;
	gint last_index = 0;
	gint i = 0;
	gint j = 0;
	gint x = 0;
	gfloat log_pos = 0.0;
	gfloat newpos = 0.0;
	GArray *array = NULL;
	GdkPoint pts[2048]; // Bad idea as static...
	Viewable_Value *v_value = NULL;
	static gulong sig_id = 0;
	static GtkWidget *scale = NULL;
	extern GHashTable *dynamic_widgets;

	pixmap = lv_data->pixmap;

	if (sig_id == 0)
		sig_id = g_signal_handler_find(g_hash_table_lookup(dynamic_widgets,"logviewer_log_position_hscale"),G_SIGNAL_MATCH_FUNC,0,0,NULL,logviewer_log_position_change,NULL);

	if (!scale)
		scale = g_hash_table_lookup(dynamic_widgets,"logviewer_log_position_hscale");
	w = lv_data->darea->allocation.width;
	h = lv_data->darea->allocation.height;

	log_pos = (gfloat)((gint)g_object_get_data(G_OBJECT(lv_data->darea),"log_pos_x100"))/100.0;
	//printf("log_pos is %f\n",log_pos);
	/* Full screen redraw, only with configure events (usually) */
	if ((gboolean)redraw_all)
	{
		lo_width = lv_data->darea->allocation.width-lv_data->info_width;
		for (i=0;i<g_list_length(lv_data->tlist);i++)
		{
			v_value = (Viewable_Value *)g_list_nth_data(lv_data->tlist,i);
			array = g_object_get_data(G_OBJECT(v_value->object),v_value->data_source);
			len = array->len;
			if (len == 0)	/* If empty */
			{
				return;
			}
			//		printf("length is %i\n", len);
			len *= (log_pos/100.0);
			//		printf("length after is  %i\n", len);
			/* Determine total number of points that'll fit on the window
			 * taking into account the scroll amount
			 */
			total = len < lo_width/lv_zoom ? len : lo_width/lv_zoom;


			// Draw is reverse order, from right to left, 
			// easier to think out in my head... :) 
			//
			for (x=0;x<total;x++)
			{
				val = g_array_index(array,gfloat,len-1-x);
				percent = 1.0-(val/(float)(v_value->upper-v_value->lower));
				pts[x].x = w-(x*lv_zoom)-1;
				pts[x].y = (gint) (percent*(h-2))+1;
			}
			gdk_draw_lines(pixmap,
					v_value->trace_gc,
					pts,
					total);
			if (v_value->highlight)
			{
				for (j=0;j<total;j++)	
					pts[j].y -= 1;
				gdk_draw_lines(pixmap,
						lv_data->darea->style->white_gc,
						pts,
						total);
				for (j=0;j<total;j++)	
					pts[j].y += 2;
				gdk_draw_lines(pixmap,
						lv_data->darea->style->white_gc,
						pts,
						total);
			}

			v_value->last_y = pts[0].y;
			v_value->last_index = len-1;

			//printf ("last index displayed was %i from %i,%i to %i,%i\n",v_value->last_index,pts[1].x,pts[1].y, pts[0].x,pts[0].y );
		}
		draw_valtext(TRUE);
		//printf("redraw complete\n");
		return;
	}
	/* Playback mode, playing from logfile.... */
	if (playback_mode)
	{
		for (i=0;i<g_list_length(lv_data->tlist);i++)
		{
			v_value = (Viewable_Value *)g_list_nth_data(lv_data->tlist,i);
			array = g_object_get_data(G_OBJECT(v_value->object),v_value->data_source);
			last_index = v_value->last_index;
			if(last_index >= array->len)
				return;

			//printf("got data from array at index %i\n",last_index+1);
			val = g_array_index(array,gfloat,last_index+1);
			percent = 1.0-(val/(float)(v_value->upper-v_value->lower));
			if (val > (v_value->max))
				v_value->max = val;
			if (val < (v_value->min))
				v_value->min = val;

			gdk_draw_line(pixmap,
					v_value->trace_gc,
					w-lv_zoom-1,v_value->last_y,
					w-1,(gint)(percent*(h-2))+1);
			//printf("drawing from %i,%i to %i,%i\n",w-lv_zoom-1,v_value->last_y,w-1,(gint)(percent*(h-2))+1);

			v_value->last_y = (gint)((percent*(h-2))+1);

			v_value->last_index = last_index + 1;
			if (adj_scale)
			{
				newpos = 100.0*((gfloat)(v_value->last_index)/(gfloat)array->len);
				blocked=TRUE;
				gtk_range_set_value(GTK_RANGE(scale),newpos);
				blocked=FALSE;
				g_object_set_data(G_OBJECT(lv_data->darea),"log_pos_x100",GINT_TO_POINTER((gint)(newpos*100.0)));
				adj_scale = FALSE;
				if (newpos >= 100)
					stop_tickler(LV_PLAYBACK_TICKLER);
				//	printf("playback reset slider to position %i\n",(gint)(newpos*100.0));
			}
			if (v_value->highlight)
			{
				gdk_draw_line(pixmap,
						lv_data->darea->style->white_gc,
						w-lv_zoom-1,v_value->last_y-1,
						w-1,(gint)(percent*(h-2)));
				gdk_draw_line(pixmap,
						lv_data->darea->style->white_gc,
						w-lv_zoom-1,v_value->last_y+1,
						w-1,(gint)(percent*(h-2))+2);
			}
		}
		draw_valtext(FALSE);
		return;
	}

	/* REALTIME mode... all traces updated at once.. */
	for (i=0;i<g_list_length(lv_data->tlist);i++)
	{
		v_value = (Viewable_Value *)g_list_nth_data(lv_data->tlist,i);
		array = g_object_get_data(G_OBJECT(v_value->object),v_value->data_source);
		current_index = (gint)g_object_get_data(v_value->object,"current_index");
		val = g_array_index(array,gfloat, current_index);

		if (val > (v_value->max))
			v_value->max = val;
		if (val < (v_value->min))
			v_value->min = val;

		if (v_value->last_y == -1)
			v_value->last_y = (gint)((percent*(h-2))+1);

		/* If watching at the edge (full realtime) */
		if (log_pos == 100)
		{
			v_value->last_index = array->len-1;
			percent = 1.0-(val/(float)(v_value->upper-v_value->lower));
			gdk_draw_line(pixmap,
					v_value->trace_gc,
					w-lv_zoom-1,v_value->last_y,
					w-1,(gint)(percent*(h-2))+1);
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
					w-1,(gint)(percent*(h-2))+1);
			if (adj_scale)
			{
				newpos = 100.0*((gfloat)v_value->last_index/(gfloat)array->len);
				blocked = TRUE;
				gtk_range_set_value(GTK_RANGE(scale),newpos);
				blocked = FALSE;
				g_object_set_data(G_OBJECT(lv_data->darea),"log_pos_x100",GINT_TO_POINTER((gint)(newpos*100.0)));
				adj_scale = FALSE;
			}
		}
		/* Draw the data.... */
		v_value->last_y = (gint)((percent*(h-2))+1);
		if (v_value->highlight)
		{
			gdk_draw_line(pixmap,
					lv_data->darea->style->white_gc,
					w-lv_zoom-1,v_value->last_y-1,
					w-1,(gint)(percent*(h-2)));
			gdk_draw_line(pixmap,
					lv_data->darea->style->white_gc,
					w-lv_zoom-1,v_value->last_y+1,
					w-1,(gint)(percent*(h-2))+2);
		}
	}
	/* Update textual data */
	draw_valtext(FALSE);
}


/*!
 \brief scroll_logviewer_traces() scrolls the traces to the left
 */
void scroll_logviewer_traces()
{
	gint start = lv_data->info_width;
	gint end = lv_data->info_width;
	gint w = 0;
	gint h = 0;
	GdkPixmap *pixmap = NULL;
	GdkPixmap *pmap = NULL;
	static GtkWidget * widget = NULL;
	extern GHashTable *dynamic_widgets;


	if (!widget)
		widget = g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea");
	if (!widget)
		return;
	pixmap = lv_data->pixmap;
	pmap = lv_data->pmap;
	if (!pixmap)
		return;

	w = widget->allocation.width;
	h = widget->allocation.height;
	start = end + lv_zoom;

	/* NASTY NASTY NASTY win32 hack to get it to scroll because
	 * draw_drawable seems to fuckup on windows when souce/dest are 
	 * in the same widget...  This works however on EVERY OTHER
	 * OS where GTK+ runs.  grr.....
	 */
#ifdef __WIN32__
	// Scroll the screen to the left... 
	gdk_draw_drawable(pmap,
			widget->style->black_gc,
			pixmap,
			lv_data->info_width+lv_zoom,0,
			lv_data->info_width,0,
			w-lv_data->info_width-lv_zoom,h);

	gdk_draw_drawable(pixmap,
			widget->style->black_gc,
			pmap,
			lv_data->info_width,0,
			lv_data->info_width,0,
			w-lv_data->info_width,h);
#else

	// Scroll the screen to the left... 
	gdk_draw_drawable(pixmap,
			widget->style->black_gc,
			pixmap,
			lv_data->info_width+lv_zoom,0,
			lv_data->info_width,0,
			w-lv_data->info_width-lv_zoom,h);
#endif

	// Init new "blank space" as black 
	gdk_draw_rectangle(pixmap,
			widget->style->black_gc,
			TRUE,
			w-lv_zoom,0,
			lv_zoom,h);

	gdk_window_clear(widget->window);
}


/*!
 \brief set_lview_choices_state() sets all the logables to either be 
 selected or deselected (select all fucntionality)
 \param widget (GtkWidget *) unused
 \param data (gpointer) state to set the widgets to
 \returns TRUE
 */
gboolean set_lview_choices_state(GtkWidget *widget, gpointer data)
{
	gboolean state = (gboolean)data;

	g_list_foreach(get_list("viewables"),set_widget_active,(gpointer)state);

	return TRUE;
}


/*!
 \brief logviewer_log_position_change() gets called when the log position 
 slider is moved by the user to fast forware/rewind through a datalog
 \param widget (GtkWidget *) widget that received the event
 \param data (gpointer) unused
 \returns TRUE
 */
EXPORT gboolean logviewer_log_position_change(GtkWidget * widget, gpointer data)
{
	gfloat val = 0.0;
	GtkWidget *darea = NULL;
	extern GHashTable *dynamic_widgets;

	/* If we pass "TRUE" as the widget data we just ignore this signal as 
	 * the redraw routine wil have to adjsut the slider as it scrolls 
	 * through the data...
	 */
	if ((gboolean)data)
		return TRUE;
	if (blocked)
		return TRUE;

	val = gtk_range_get_value(GTK_RANGE(widget));
	darea = g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea");
	if (GTK_IS_WIDGET(darea))
		g_object_set_data(G_OBJECT(darea),"log_pos_x100",GINT_TO_POINTER((gint)(val*100)));
	lv_configure_event(darea,NULL,NULL);
	scroll_logviewer_traces();
	if ((val >= 100.0) && (playback_mode))
		stop_tickler(LV_PLAYBACK_TICKLER);
	return TRUE;
}


/*!
 \brief set_playback_mode() sets things up for playback mode
 */
void set_playback_mode(void)
{
	extern GHashTable *dynamic_widgets;
	GtkWidget *widget;

	reset_logviewer_state();
	free_log_info();
	playback_mode = TRUE;
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button"), FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_logfile_button"), TRUE);
	gtk_widget_hide(g_hash_table_lookup(dynamic_widgets,"logviewer_rt_control_vbox1"));
	gtk_widget_show(g_hash_table_lookup(dynamic_widgets,"logviewer_playback_control_vbox1"));
	gtk_widget_show(g_hash_table_lookup(dynamic_widgets,"scroll_speed_vbox"));
	widget = g_hash_table_lookup(dynamic_widgets,"logviewer_log_position_hscale");
	if (GTK_IS_RANGE(widget))
		gtk_range_set_value(GTK_RANGE(widget),0.0);
	hue = -60.0;
	col_sat = 1.0;
	col_val = 1.0;
}


/*!
 \brief set_realtime_mode() sets things up for realtime mode
 */
void set_realtime_mode(void)
{
	extern GHashTable *dynamic_widgets;
	GtkWidget *widget = NULL;

	stop_tickler(LV_PLAYBACK_TICKLER);
	reset_logviewer_state();
	free_log_info();
	playback_mode = FALSE;
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_logfile_button"), FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button"), TRUE);
	gtk_widget_show(g_hash_table_lookup(dynamic_widgets,"logviewer_rt_control_vbox1"));
	gtk_widget_hide(g_hash_table_lookup(dynamic_widgets,"logviewer_playback_control_vbox1"));
	gtk_widget_hide(g_hash_table_lookup(dynamic_widgets,"scroll_speed_vbox"));
	widget = g_hash_table_lookup(dynamic_widgets,"logviewer_log_position_hscale");
	if (GTK_IS_RANGE(widget))
		gtk_range_set_value(GTK_RANGE(widget),100.0);
	hue = -60.0;
	col_sat = 1.0;
	col_val = 1.0;
}


/*!
 \brief finish_logviewer() sets button default states for the logviewer after
 it is created from it's glade config file
 */
EXPORT void finish_logviewer(void)
{
	GtkWidget * widget = NULL;
	extern GHashTable *dynamic_widgets;

	lv_data = g_new0(Logview_Data,1);
	lv_data->traces = g_hash_table_new(g_str_hash,g_str_equal);
	lv_data->info_width = 120;

	if (playback_mode)
		set_playback_mode();
	else
		set_realtime_mode();

	widget = g_hash_table_lookup(dynamic_widgets,"logviewer_zoom_spinner");
	if (GTK_IS_SPIN_BUTTON(widget))
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),lv_zoom);

	return;
}


/*!
 \brief slider_ket_press_event() doesn't do anything yet (stub)
 */
EXPORT gboolean slider_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	return FALSE;
}
