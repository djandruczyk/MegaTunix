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
#include <logviewer_gui.h>
#include <mode_select.h>
#include <ms_structures.h>
#include <structures.h>
#include <tabloader.h>

static gint tcount = 0;	/* used to draw_infotext to know where to put the text*/
static gint info_width = 120;
static gint max_viewables = 0;
static gint active_viewables = 0;
static GtkWidget *lv_darea = NULL; //for scroller  :( 
static gboolean adj_scale = TRUE;
static gboolean blocked = FALSE;
static GStaticMutex update_mutex = G_STATIC_MUTEX_INIT;

static GHashTable *active_traces = NULL;
gint lv_zoom = 0;		/* logviewer scroll amount */
gboolean playback_mode = FALSE;
extern struct Log_Info *log_info;


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
	extern struct Rtv_Map *rtv_map;

	darea = g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea");
	lv_darea = darea;

	if (!darea)
	{
		dbg_func(__FILE__": present_viewer_choices()\n\tpointer to drawing area was NULL, returning!!!\n",CRITICAL);
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
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);
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
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
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
		store_list("viewables",g_list_append(
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
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);

	gtk_widget_show_all(window);
	return;
}

gboolean view_value_set(GtkWidget *widget, gpointer data)
{
	GObject *object = NULL;
	gboolean state = FALSE;


	state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widget));

	/* get object from widget */
	object = (GObject *)g_object_get_data(G_OBJECT(widget),"object");
	if (!object)
		printf("object was null during view_value_set()\n");
	g_object_set_data(object,"being_viewed",GINT_TO_POINTER(state));
	populate_viewer();
	return FALSE;
}

void populate_viewer()
{
	gint i = 0;
	gint total = 0;
	struct Viewable_Value *v_value = NULL;
	gchar * name = NULL;
        gboolean being_viewed = FALSE;
        extern struct Rtv_Map *rtv_map;
        extern GHashTable *dynamic_widgets;
	GObject *object = NULL;

	g_static_mutex_lock(&update_mutex);

	/* Checks if hash is created, if not, makes one, allocates data
	 * for strcutres defining each viewable element., sets those attribute
	 * and adds them to the list, also checks if entires are removed and
	 * pulls them from the hashtable and de-allocates them...
	 */
	if (active_traces == NULL)
		active_traces = g_hash_table_new(g_str_hash,g_str_equal);

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
			printf("ERROR, name is NULL\n");
		if (!object)
			printf("ERROR, object is NULL\n");

		being_viewed = (gboolean)g_object_get_data(object,"being_viewed");
		/* if not found in table check to see if we need to insert*/
		if (g_hash_table_lookup(active_traces,name)==NULL)
		{
			if (being_viewed)	/* Marked viewable widget */
			{
				/* Call the build routine, feed it the drawing_area*/
				v_value = build_v_value(lv_darea,object);
				// store location of master
				g_hash_table_insert(active_traces,
						g_strdup(name),
						(gpointer)v_value);
			}
		}
		else
		{	/* If in table but now de-selected, remove it */
			if (!being_viewed)
			{
				v_value = (struct Viewable_Value *)g_hash_table_lookup(active_traces,name);
				/* Remove entry in from hash table */
				g_hash_table_remove(active_traces,name);

				/* Free all resources of the datastructure 
				 * before de-allocating it... 
				 */
				if (!playback_mode)
					g_array_free(v_value->data_array,TRUE);

				g_object_unref(v_value->trace_gc);
				g_object_unref(v_value->font_gc);
				g_free(v_value->vname);
				g_free(v_value);
				v_value = NULL;
			}
		}
	}
	active_viewables = g_hash_table_size(active_traces);
	/* If traces selected, emit a configure_Event to clear the window
	 * and draw the traces (IF ONLY reading a log for playback)
	 */
	g_static_mutex_unlock(&update_mutex);
	if ((active_traces) && (g_hash_table_size(active_traces) >= 0))
		lv_configure_event(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea"),NULL,NULL);

	return; 
}

void reset_logviewer_state()
{
	extern GHashTable *dynamic_widgets;
	GtkWidget * widget = NULL;
	extern struct Rtv_Map *rtv_map;
	extern struct Log_Info *log_info;
	gint i = 0 ;
	GObject * object = NULL;

	if (playback_mode)
	{
		widget = g_hash_table_lookup(dynamic_widgets,"logviewer_log_position_hscale");
		if (GTK_IS_RANGE(widget))
			gtk_range_set_value(GTK_RANGE(widget),1.0);
		if (!log_info)
			return;
		for (i=0;i<log_info->field_count;i++)
		{
			object = g_array_index(log_info->log_list,GObject *,i);
			g_object_set_data(object,"being_viewed",GINT_TO_POINTER(FALSE));
		}
	}
	else
	{
		if (!rtv_map)
			return;
		for (i=0;i<rtv_map->derived_total;i++)
		{
			object = g_array_index(rtv_map->rtv_list,GObject *,i);
			g_object_set_data(object,"being_viewed",GINT_TO_POINTER(FALSE));
		}
		widget = g_hash_table_lookup(dynamic_widgets,"logviewer_log_position_hscale");
		if (GTK_IS_RANGE(widget))
			gtk_range_set_value(GTK_RANGE(widget),100.0);
	}
	populate_viewer();

}

struct Viewable_Value * build_v_value(GtkWidget * d_area, GObject *object)
{
	struct Viewable_Value *v_value = NULL;
	GdkPixmap *pixmap =  NULL;
	GArray *tmp_array = NULL;

	pixmap = (GdkPixmap *) g_object_get_data(G_OBJECT(d_area),"pixmap");

	v_value = g_malloc(sizeof(struct Viewable_Value));		

	/* Set limits of this variable. (it's ranges, used for scaling */

	if (playback_mode)
	{
		/* IS it a floating point value? */
		v_value->is_float = TRUE;
		/* textual name of the variable we're viewing.. */
		v_value->vname = g_strdup(g_object_get_data(object,"lview_name"));
		/* data was already read from file and stored, copy pointer
		 * over to v_value so it can be drawn...
		 */
		tmp_array = (GArray *)g_object_get_data(object,"data_array");
		v_value->data_array = tmp_array;
	}
	else
	{
		v_value->is_float = (gboolean)g_object_get_data(object,"is_float");;
		// textual name of the variable we're viewing.. 
		v_value->vname = g_strdup(g_object_get_data(object,"dlog_gui_name"));
		/* Array to keep history for resize/redraw and export 
		 * to datalog we use the _sized_ version to give a big 
		 * enough size to prevent reallocating memory too often. 
		 * (more initial mem usage,  but less calls to malloc...
		 */
		v_value->data_array = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
	}
	/* Store pointer to d_area, but DO NOT FREE THIS on v_value destruction
	 * as its the SAME one used for all Viewable_Values */
	v_value->d_area = d_area;
	v_value->object = object;
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

	return v_value;
}

GdkGC * initialize_gc(GdkDrawable *drawable, GcType type)
{
	GdkColor color;
	GdkGC * gc = NULL;
	GdkGCValues values;
	GdkColormap *cmap = NULL;
	static gfloat hue_angle = 0.0;

	cmap = gdk_colormap_get_system();

	switch((GcType)type)
	{
		case FONT:
			color.red = 60000;
			color.green = 60000;
			color.blue = 60000;
			gdk_colormap_alloc_color(cmap,&color,TRUE,TRUE);
			values.foreground = color;
			gc = gdk_gc_new_with_values(GDK_DRAWABLE(drawable),
					&values,
					GDK_GC_FOREGROUND);
			break;

		case TRACE:
			color = (GdkColor)  get_colors_from_hue(hue_angle);
			gdk_colormap_alloc_color(cmap,&color,TRUE,TRUE);
			values.foreground = color;
			gc = gdk_gc_new_with_values(GDK_DRAWABLE(drawable),
					&values,
					GDK_GC_FOREGROUND);
			hue_angle += 75;
			if (hue_angle >= 360)
				hue_angle -= 360.0;
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
	}	
	return gc;	
}

GdkColor get_colors_from_hue(gfloat hue_angle)
{
	GdkColor color;
	gfloat tmp = 0.0;	
	gint i = 0;
	gfloat fract = 0.0;
	gfloat S = 1.0;	// using saturation of 1.0
	gfloat V = 1.0;	// using Value of 1.0
	gfloat p = 0.0;
	gfloat q = 0.0;
	gfloat t = 0.0;
	gfloat r = 0.0;
	gfloat g = 0.0;
	gfloat b = 0.0;

	tmp = hue_angle/60.0;
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

void draw_infotext(void *data)
{
	// Draws the textual (static) info on the left side of the window..

	struct Viewable_Value *v_value = (struct Viewable_Value *) data;
	gint last_index = 0;
	gfloat val = 0.0;
	gint name_x = 10;
	gint name_y = 0;
	gint h = 0;
	gint val_x_offset = 10;
	gint val_y_offset = 14;
	PangoFontDescription *font_desc;
	PangoLayout *layout;
	GdkPixmap *pixmap = (GdkPixmap *) g_object_get_data(G_OBJECT(v_value->d_area),"pixmap");

	h = v_value->d_area->allocation.height;
	if (tcount == 0)  /*clear text area */
	{
		gdk_draw_rectangle(pixmap,
				v_value->d_area->style->black_gc,
				TRUE, 0,0,
				info_width,h);
	}
	
	name_y = (gint) (((float)h/(float)(active_viewables+1))*(tcount+1));
	name_y -= 15;
	last_index = v_value->last_index;
	val = g_array_index(v_value->data_array,gfloat,last_index);

	font_desc = pango_font_description_from_string("courier 11");
	layout = gtk_widget_create_pango_layout(v_value->d_area,NULL);
	pango_layout_set_markup(layout,v_value->vname,-1);
	pango_layout_set_font_description(layout,font_desc);
	gdk_draw_layout(pixmap,v_value->trace_gc,name_x,name_y,layout);

	gdk_draw_rectangle(pixmap,
			v_value->d_area->style->black_gc,
			TRUE,
			name_x+val_x_offset,name_y+val_y_offset,
			info_width-(name_x+val_x_offset),15);
	if (v_value->is_float)
		layout = gtk_widget_create_pango_layout(v_value->d_area,g_strdup_printf("%.2f",val));
	else
		layout = gtk_widget_create_pango_layout(v_value->d_area,g_strdup_printf("%i",(gint)val));

	pango_layout_set_font_description(layout,font_desc);
	gdk_draw_layout(pixmap,v_value->font_gc,name_x+val_x_offset,name_y+val_y_offset,layout);
	tcount++;

}

gboolean update_logview_traces(gboolean force_redraw)
{

	/* Called from one of two possible places:
	 * 1. a GTK timeout used when playing back a log...
	 * 2. as a hook onto the update_realtime stats box...
	 *
	 * If table does NOT exist or table is empty do NOTHING
	 */

	if ((active_traces) && (g_hash_table_size(active_traces) > 0))
	{
		adj_scale = TRUE;
		g_static_mutex_lock(&update_mutex);
		g_hash_table_foreach(active_traces, trace_update,GINT_TO_POINTER(force_redraw));
		scroll_logviewer_traces();
		g_static_mutex_unlock(&update_mutex);
	}

	return TRUE;
}

void trace_update(gpointer key, gpointer value, gpointer redraw_all)
{
	GdkPixmap * pixmap = NULL;
	gint w = 0;
	gint h = 0;
	gfloat val = 0.0;
	gfloat last_val = 0.0;
	gfloat percent = 0.0;
	gfloat last_percent = 0.0;
	gfloat *history = NULL;
	gint last_entry = 0;
	gint len = 0;
	gint lo_width;
	gint total = 0;
	gint last_index = 0;
	gint i = 0;
	gfloat log_pos = 0.0;
	gfloat newpos = 0.0;
	static gulong sig_id = 0;
	GdkPoint pts[2048]; // Bad idea as static...
	static GtkWidget *scale = NULL;
	struct Viewable_Value * v_value = NULL;
	extern GHashTable *dynamic_widgets;

	v_value = (struct Viewable_Value *) value;
	if (v_value == NULL)
	{
		printf("no traces, exiting...\n");
		return;
	}
	pixmap = (GdkPixmap *) g_object_get_data(G_OBJECT(v_value->d_area),
			"pixmap");
	if (sig_id == 0)
		sig_id = g_signal_handler_find(g_hash_table_lookup(dynamic_widgets,"logviewer_log_position_hscale"),G_SIGNAL_MATCH_FUNC,0,0,NULL,logviewer_log_position_change,NULL);

	if (!scale)
		scale = g_hash_table_lookup(dynamic_widgets,"logviewer_log_position_hscale");
	w = v_value->d_area->allocation.width;
	h = v_value->d_area->allocation.height;

	log_pos = (gfloat)((gint)g_object_get_data(G_OBJECT(v_value->d_area),"log_pos_x100"))/100.0;
	//printf("log_pos is %f\n",log_pos);
	if ((gboolean)redraw_all)
	{
		lo_width = v_value->d_area->allocation.width-info_width;
		len = v_value->data_array->len;
		if (len == 0)	/* If empty */
			return;
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
		for (i=0;i<total;i++)
		{
			val = g_array_index(v_value->data_array,gfloat,len-1-i);
			percent = 1.0-(val/(float)(v_value->upper-v_value->lower));
			pts[i].x = w-(i*lv_zoom)-1;
			pts[i].y = (gint) (percent*(h-2))+1;
		}
		gdk_draw_lines(pixmap,
				v_value->trace_gc,
				pts,
				total);

		v_value->last_y = pts[0].y;
		v_value->last_index = len;

		//printf ("last index displayed was %i from %i,%i to %i,%i\n",v_value->last_index,pts[1].x,pts[1].y, pts[0].x,pts[0].y );
		draw_infotext(v_value);
		//printf("redraw complete\n");
		return;
	}
	/* we don't get data from the new infrastructure in playback mode... */
	if (playback_mode)
	{
		last_index = v_value->last_index;
		if(last_index >= v_value->data_array->len)
			return;

		//printf("got data from array at index %i\n",last_index+1);
		val = g_array_index(v_value->data_array,gfloat,last_index+1);
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
			newpos = 100.0*((gfloat)(v_value->last_index)/(gfloat)v_value->data_array->len);
			blocked=TRUE;
			gtk_range_set_value(GTK_RANGE(scale),newpos);
			blocked=FALSE;
			g_object_set_data(G_OBJECT(v_value->d_area),"log_pos_x100",GINT_TO_POINTER((gint)(newpos*100.0)));
			adj_scale = FALSE;
		//	printf("playback reset slider to position %i\n",(gint)(newpos*100.0));
		}
		draw_infotext(v_value);
		return;
	}

	history = (gfloat *)g_object_get_data(v_value->object,"history");
	last_entry = (gint)g_object_get_data(v_value->object,"last_entry");
	val = history[last_entry];
	
	if (val > (v_value->max))
		v_value->max = val;
	if (val < (v_value->min))
		v_value->min = val;

	g_array_append_val(v_value->data_array,val);

	if (v_value->last_y == -1)
		v_value->last_y = (gint)((percent*(h-2))+1);

	/* If watching at the edge (full realtime) */
	if (log_pos == 100)
	{
		v_value->last_index = v_value->data_array->len-1;
		percent = 1.0-(val/(float)(v_value->upper-v_value->lower));
		gdk_draw_line(pixmap,
				v_value->trace_gc,
				w-lv_zoom-1,v_value->last_y,
				w-1,(gint)(percent*(h-2))+1);
	}
	else
	{	/* Watching somewhat behind realtime... */
		last_index = v_value->last_index;

		last_val = g_array_index(v_value->data_array,gfloat,last_index);
		last_percent = 1.0-(last_val/(float)(v_value->upper-v_value->lower));
		val = g_array_index(v_value->data_array,gfloat,last_index+1);
		percent = 1.0-(val/(float)(v_value->upper-v_value->lower));

		v_value->last_index = last_index + 1;
		gdk_draw_line(pixmap,
				v_value->trace_gc,
				w-lv_zoom-1,(last_percent*(h-2))+1,
				w-1,(gint)(percent*(h-2))+1);
		if (adj_scale)
		{
			newpos = 100.0*((gfloat)v_value->last_index/(gfloat)v_value->data_array->len);
			blocked = TRUE;
			gtk_range_set_value(GTK_RANGE(scale),newpos);
			blocked = FALSE;
			g_object_set_data(G_OBJECT(v_value->d_area),"log_pos_x100",GINT_TO_POINTER((gint)(newpos*100.0)));
			adj_scale = FALSE;
		}
	}
		/* Draw the data.... */
	v_value->last_y = (gint)((percent*(h-2))+1);


	/* Update textual data */
	draw_infotext(v_value);

}

void scroll_logviewer_traces()
{
	gint start = info_width;
	gint end = info_width;
	gint w = 0;
	gint h = 0;
	GdkPixmap *pixmap = NULL;
	GtkWidget * widget = NULL;
	extern GHashTable *dynamic_widgets;

	widget = g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea");
	if (!widget)
		return;
	pixmap = (GdkPixmap *) g_object_get_data(G_OBJECT(widget),"pixmap");
	if (!pixmap)
		return;

	w = widget->allocation.width;
	h = widget->allocation.height;
	start = end + lv_zoom;
	// Scroll the screen to the left... 
	gdk_draw_drawable(pixmap,
			widget->style->black_gc,
			pixmap,
			info_width+lv_zoom,0,
			info_width,0,
			w-info_width-lv_zoom,h);

	// Init new "blank space" as black 
	gdk_draw_rectangle(pixmap,
			widget->style->black_gc,
			TRUE,
			w-lv_zoom,0,
			lv_zoom,h);

	tcount = 0;
	gdk_window_clear(widget->window);
}

EXPORT gboolean lv_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GdkPixmap *pixmap = NULL;
	gint w = 0;
	gint h = 0;

	/* Get pointer to backing pixmap ... */
	pixmap = (GdkPixmap *)g_object_get_data(G_OBJECT(widget),"pixmap");
			
	if (widget->window)
	{
		if (pixmap)
			g_object_unref(pixmap);

		w=widget->allocation.width;
		h=widget->allocation.height;
		pixmap=gdk_pixmap_new(widget->window,
				w,h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				w,h);
		gdk_window_set_back_pixmap(widget->window,pixmap,0);
		g_object_set_data(G_OBJECT(widget),"pixmap",pixmap);

		if ((active_traces) && (g_hash_table_size(active_traces) > 0))
		{
			tcount = 0;
			g_hash_table_foreach(active_traces, trace_update, GINT_TO_POINTER(TRUE));
			tcount = 0;
		}
		gdk_window_clear(widget->window);
	}

	return FALSE;
}


EXPORT gboolean lv_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GdkPixmap *pixmap = NULL;
	pixmap = (GdkPixmap *)g_object_get_data(G_OBJECT(widget),"pixmap");

	/* Expose event handler... */
	gdk_draw_drawable(widget->window,
                        widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                        pixmap,
                        event->area.x, event->area.y,
                        event->area.x, event->area.y,
                        event->area.width, event->area.height);

	return TRUE;
}

gboolean set_lview_choices_state(GtkWidget *widget, gpointer data)
{
	gboolean state = (gboolean)data;

	g_list_foreach(get_list("viewables"),set_widget_active,(gpointer)state);

	return TRUE;
}


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
	return TRUE;
}

void set_playback_mode(void)
{
	extern GHashTable *dynamic_widgets;

	reset_logviewer_state();
	free_log_info();
	playback_mode = TRUE;
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button"), FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_logfile_button"), TRUE);
	gtk_widget_hide(g_hash_table_lookup(dynamic_widgets,"logviewer_rt_control_vbox1"));
	gtk_widget_show(g_hash_table_lookup(dynamic_widgets,"logviewer_playback_control_vbox1"));
}

void set_realtime_mode(void)
{
	extern GHashTable *dynamic_widgets;

	reset_logviewer_state();
	free_log_info();
	playback_mode = FALSE;
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_logfile_button"), FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button"), TRUE);
	gtk_widget_show(g_hash_table_lookup(dynamic_widgets,"logviewer_rt_control_vbox1"));
	gtk_widget_hide(g_hash_table_lookup(dynamic_widgets,"logviewer_playback_control_vbox1"));
}

EXPORT void finish_logviewer(void)
{
	GtkWidget * widget = NULL;
	extern GHashTable *dynamic_widgets;

	if (playback_mode)
		set_playback_mode();
	else
		set_realtime_mode();

	widget = g_hash_table_lookup(dynamic_widgets,"logviewer_zoom_spinner");
	if (GTK_IS_SPIN_BUTTON(widget))
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),lv_zoom);

	return;
}


EXPORT gboolean slider_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	return FALSE;
}
