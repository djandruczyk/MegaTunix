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
#include <default_limits.h>
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <logviewer_gui.h>
#include <mode_select.h>
#include <ms_structures.h>
#include <structures.h>

static gint tcount = 0;	/* used to draw_infotext to know where to put the text*/
static gint info_width = 120;
static gint max_viewables = 0;
static gint total_viewables = 0;
static struct Logables viewables;
static GHashTable *active_traces = NULL;
GtkWidget * lv_darea;
gint lv_scroll = 0;		/* logviewer scroll amount */
gboolean logviewer_mode = FALSE;


/* This table is the same dimensions as the table used for datalogging.
 * FALSE means it's greyed out as a choice for the logviewer, TRUE means
 * it's visible.  Some logable variables (like the clock) don't make a lot
 * of sense on a stripchart view...
 */
static const gboolean valid_logables[]=
{
	FALSE,TRUE,TRUE,FALSE,TRUE,
	TRUE,TRUE,TRUE,TRUE,TRUE,
	TRUE,TRUE,TRUE,TRUE,TRUE,
	TRUE,TRUE,TRUE,TRUE,TRUE,
	TRUE,TRUE,TRUE,TRUE,TRUE,
	TRUE,TRUE,TRUE,TRUE,TRUE,
	TRUE,TRUE,TRUE,TRUE,TRUE,
	TRUE,TRUE,TRUE,TRUE,TRUE,
	TRUE,TRUE
};

void build_logviewer(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *sbar;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *hbox;
	GtkWidget *d_area;
	GtkWidget *button;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *spinner;
	GdkPixmap *pixmap;
	GtkAdjustment *adj;
	extern struct DynamicButtons buttons;
	GSList *group = NULL;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	/* Traces frame */
	frame = gtk_frame_new("Log Viewer");
	gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,0);

	/* Holds all the log traces... */
	vbox2 = gtk_vbox_new(FALSE,1);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	d_area = gtk_drawing_area_new();
	lv_darea = d_area;
	pixmap = gdk_pixmap_new(d_area->window,100,100,
			gtk_widget_get_visual(d_area)->depth);

	gtk_box_pack_start(GTK_BOX(vbox2),d_area,TRUE,TRUE,0);

	/* Add events to capture mouse button presses (for popup menus...) */
	gtk_widget_add_events(d_area,
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_FOCUS_CHANGE_MASK);

	g_signal_connect(G_OBJECT
			(d_area),
			"configure_event",
			G_CALLBACK(lv_configure_event),
			NULL);
	g_signal_connect(G_OBJECT
			(d_area),
			"expose_event",
			G_CALLBACK(lv_expose_event),
			NULL);

	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,1.0,0.001,0.001,0);
	sbar = gtk_hscrollbar_new(adj);
	gtk_box_pack_start(GTK_BOX(vbox2),sbar,FALSE,TRUE,0);

	/* Settings/Parameters frame... */
	frame = gtk_frame_new("Playback/Viewer Parameters");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	hbox = gtk_hbox_new(FALSE,7);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	/* Hold the realtime/playback toggle buttons */
	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,5);

	button = gtk_radio_button_new_with_label(NULL,"Realtime Mode");
	g_object_set_data(G_OBJECT(button),"data",(gpointer)d_area);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(REALTIME_VIEW));
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(reset_viewables),
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox3),button,FALSE,FALSE,0);

	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
	button = gtk_radio_button_new_with_label(group,"Playback Mode");
	g_object_set_data(G_OBJECT(button),"data",(gpointer)d_area);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(PLAYBACK_VIEW));
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(toggle_button_handler),
			GINT_TO_POINTER(PLAYBACK_VIEW));
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(reset_viewables),
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox3),button,FALSE,FALSE,0);

	/* Holds the Select Button */
	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,5);

	button = gtk_button_new_with_label("Select Logfile to Playback");
	buttons.logplay_sel_log_but = button;
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(SELECT_DLOG_IMP));
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(std_button_handler),
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox3),button,TRUE,FALSE,0);
	gtk_widget_set_sensitive(button,FALSE);

	button = gtk_button_new_with_label("Select Parameters to view");
	buttons.logplay_sel_parm_but = button;
	g_object_set_data(G_OBJECT(button),"data",(gpointer)d_area);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(SELECT_PARAMS));
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(std_button_handler),
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox3),button,TRUE,FALSE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,5);
	button = gtk_button_new_with_label("Start Reading RT Vars");
	buttons.logplay_start_rt_but = button;
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(START_REALTIME));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox3),button,TRUE,FALSE,0);
	button = gtk_button_new_with_label("Stop Reading RT vars");
	buttons.logplay_stop_rt_but = button;
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(STOP_REALTIME));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);

	gtk_box_pack_start(GTK_BOX(vbox3),button,TRUE,FALSE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,5);

	label = gtk_label_new("Zoom Level");
	gtk_box_pack_start(GTK_BOX(vbox3),label,TRUE,FALSE,5);

	adj =  (GtkAdjustment *) gtk_adjustment_new((gfloat)lv_scroll,1.0,15.0,1.0,1.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	gtk_widget_set_size_request(spinner,45,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"info",(gpointer)d_area);
	g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(LOGVIEW_ZOOM));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox3),spinner,TRUE,FALSE,0);

	return;
}

void present_viewer_choices(void *ptr)
{
	GtkWidget *window;
	GtkWidget *table;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *sep;
	extern GtkTooltips *tip;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint table_rows = 0;
	gint table_cols = 5;
	gchar * name = NULL;
	GtkWidget *hand_me_down = NULL;
	extern gint ecu_caps;
	struct Log_Info *log_info = NULL;

	if (ptr != NULL)
		hand_me_down = (GtkWidget *)ptr;
	else
	{
		dbg_func(__FILE__": present_viewer_choices()\n\tpointer fed was NULL, returning!!!\n",CRITICAL);
		return;
	}

	/* basty hack to prevent a compiler warning... */
	max_viewables = sizeof(mt_classic_names)/sizeof(gchar *);
	max_viewables = sizeof(mt_full_names)/sizeof(gchar *);
	max_viewables = sizeof(logable_names)/sizeof(gchar *);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
	if (logviewer_mode)
	{
		log_info = (struct Log_Info *)g_object_get_data(G_OBJECT(hand_me_down),"log_info");
		gtk_window_set_title(GTK_WINDOW(window),
				"Playback Mode: Logviewer Choices");
		frame = gtk_frame_new("Select Variables to playback from the list below...");
		max_viewables = log_info->field_count;
	}
	else
	{
		gtk_window_set_title(GTK_WINDOW(window),
				"Realtime Mode: Logviewer Choices");
		frame = gtk_frame_new("Select Realtime Variables to view from the list below...");
		max_viewables = sizeof(logable_names)/sizeof(gchar *);
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
	for (i=0;i<max_viewables;i++)
	{
		if (logviewer_mode)
			name = g_strdup(log_info->fields[i]);
		else
			name = g_strdup(logable_names[i]);


		button = gtk_check_button_new_with_label(name);
		if (!logviewer_mode)
		{
			if (valid_logables[i] == FALSE)
				gtk_widget_set_sensitive(button,FALSE);
			gtk_tooltips_set_tip(tip,button,logable_names_tips[i],NULL);
		}
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
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(select_all_controls),
			NULL);
	button = gtk_button_new_with_label("De-select All");
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,15);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(deselect_all_controls),
			NULL);

	button = gtk_button_new_with_label("Close");
	gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(populate_viewer),
			(gpointer)hand_me_down);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);

	parse_ecu_capabilities(ecu_caps);
	gtk_widget_show_all(window);
	return;
}

gboolean view_value_set(GtkWidget *widget, gpointer data)
{
	gint index = 0;

	index = (gint)g_object_get_data(G_OBJECT(widget),"index");

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		viewables.index[index] = TRUE;
	else
		viewables.index[index] = FALSE;

	return TRUE;
}

gboolean populate_viewer(GtkWidget * d_area)
{
	struct Viewable_Value *v_value = NULL;
	gint i = 0;
	total_viewables = 0;
	/* Reget to total Viewables count */
	for (i=0;i<max_viewables;i++)
	{
		if (viewables.index[i])
			total_viewables++;
	}

	/* Checks if hash is created, if not, makes one, allocates data
	 * for strcutres defining each viewable element., sets those attribute
	 * and adds them to the list, also checks if entires are removed and
	 * pulls them from the hashtable and de-allocates them...
	 */
	if (active_traces == NULL)
	{
		active_traces = g_hash_table_new(NULL,NULL);
	}

	/* check to see if it's already in the table, if so ignore, if not
	 * malloc datastructure, populate it's values and insert a pointer
	 * into the table for it..
	 */
	for (i=0;i<max_viewables;i++)
	{
		/* if not found in table check to see if we need to insert*/
		if (g_hash_table_lookup(active_traces,GINT_TO_POINTER(i))==NULL)
		{
			if (viewables.index[i])	/* Marked viewable widget */
			{
				/* Call the build routine, feed it the drawing_area*/
				v_value = build_v_value(d_area,i);

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
				/* Remove entry in from hash table */
				g_hash_table_remove(active_traces,
						GINT_TO_POINTER(i));

				/* Free all resources of the datastructure 
				 * before de-allocating it... 
				 */
				g_array_free(v_value->data_array,TRUE);
				g_object_unref(v_value->trace_gc);
				g_object_unref(v_value->font_gc);
				g_free(v_value->vname);
				g_free(v_value);
				v_value = NULL;
			}
		}
	}
	/* If traces selected, emit a configure_Event to clear the window
	 * and draw the traces (IF ONLY reading a log for playback)
	 */
	if ((active_traces) && (g_hash_table_size(active_traces) > 0))
		g_signal_emit_by_name(G_OBJECT(d_area),"configure_event",NULL);

	return FALSE; /* want other handlers to run... */
}

struct Viewable_Value * build_v_value(GtkWidget * d_area, gint offset)
{
	struct Viewable_Value *v_value = NULL;
	GdkPixmap *pixmap =  NULL;
	struct Log_Info *log_info = NULL; 
	GArray *tmp_array = NULL;

	pixmap = (GdkPixmap *) g_object_get_data(G_OBJECT(d_area),"pixmap");
	log_info = (struct Log_Info *)g_object_get_data(G_OBJECT(d_area),"log_info");

	v_value = g_malloc(sizeof(struct Viewable_Value));		

	/* Store pointer to d_area, but DO NOT FREE THIS on v_value destruction
	 * as its the SAME one used for all Viewable_Values */
	v_value->d_area = d_area;

	/* Set limits of this variable. (it's ranges, used for scaling */
	if (logviewer_mode)
	{
		//		v_value->lower = 0.0;
		//		v_value->upper = 255.0;
		v_value->lower = g_array_index(log_info->lowers,gfloat,offset);
		v_value->upper = g_array_index(log_info->uppers,gfloat,offset);
		v_value->runtime_offset = -1; // INVALID for playback
		v_value->size = -1; // ??? INVALID for playback (maybe)
		/* textual name of the variable we're viewing.. */
		v_value->vname = g_strdup(log_info->fields[offset]);
		/* data was already read from file and stored, copy pointer
		 * over to v_value so it can be drawn...
		 */
		tmp_array = g_array_index(log_info->fields_data,GArray *, offset);
		v_value->data_array = tmp_array;
	}
	else
	{
		v_value->lower = def_limits[offset].lower;
		v_value->upper = def_limits[offset].upper;
		/* Tell where we get the data from */
		v_value->runtime_offset = logging_offset_map[offset];
		/* How big the data is (needed when indexing into the data */
		v_value->size = logging_datasizes_map[offset];
		/* textual name of the variable we're viewing.. */
		v_value->vname = g_strdup(logable_names[offset]);
		/* Array to keep history for resize/redraw and export 
		 * to datalog we use the _sized_ version to give a big 
		 * enough size to prevent reallocating memory too often. 
		 * (more initial mem usage,  but less calls to malloc...
		 */
		v_value->data_array = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
	}
	/* Sets last "y" value to -1, needed for initial draw to be correct */
	v_value->last_y = -1;

	/* User adjustable scales... */
	v_value->cur_low = v_value->lower;
	v_value->cur_high = v_value->upper;

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
	gint len = 0;
	gfloat val = 0.0;
	gint name_x = 10;
	gint name_y = 0;
	gint h = 0;
	gint val_x_offset = 10;
	gint val_y_offset = 20;
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

	name_y = (gint) (((float)h/(float)(total_viewables+1))*(tcount+1));
	name_y -= 15;
	len = v_value->data_array->len;
	val = g_array_index(v_value->data_array,gfloat,len-1);

	font_desc = pango_font_description_from_string("courier 12");
	layout = gtk_widget_create_pango_layout(v_value->d_area,v_value->vname);
	pango_layout_set_font_description(layout,font_desc);
	gdk_draw_layout(pixmap,v_value->trace_gc,name_x,name_y,layout);

	gdk_draw_rectangle(pixmap,
			lv_darea->style->black_gc,
			TRUE,
			name_x+val_x_offset,name_y+val_y_offset,
			info_width-(name_x+val_x_offset),15);
	if (v_value->size == 4)
		layout = gtk_widget_create_pango_layout(v_value->d_area,g_strdup_printf("%.2f",val));
	else
		layout = gtk_widget_create_pango_layout(v_value->d_area,g_strdup_printf("%i",(gint)val));

	pango_layout_set_font_description(layout,font_desc);
	gdk_draw_layout(pixmap,v_value->font_gc,name_x+val_x_offset,name_y+val_y_offset,layout);
	tcount++;

}

gboolean update_logview_traces()
{
	/* Called from one of two possible places:
	 * 1. a GTK timeout used when playing back a log...
	 * 2. as a hook onto the update_realtime stats box...
	 *
	 * If table does NOT exist or table is empty do NOTHING
	 */

	if ((active_traces) && (g_hash_table_size(active_traces) > 0))
	{
		g_hash_table_foreach(active_traces, trace_update,NULL);
		scroll_logviewer_traces();
	}

	return TRUE;
}

void trace_update(gpointer key, gpointer value, gpointer data)
{
	GdkPixmap * pixmap = NULL;
	gint w = 0;
	gint h = 0;
	gfloat val = 0.0;
	gfloat percent = 0.0;
	gint size = 0;
	gint len = 0;
	gint lo_width;
	gint total = 0;
	gint i = 0;
	GdkPoint pts[2048]; // Bad idea as static...
	struct Viewable_Value * v_value = NULL;
	extern struct Runtime_Common *runtime;
	guchar * uc_ptr = NULL;
	short * s_ptr = NULL;
	float * f_ptr = NULL;

	v_value = (struct Viewable_Value *) value;
	if (v_value == NULL)
	{
		printf("no traces, exiting...\n");
		return;
	}
	pixmap = (GdkPixmap *) g_object_get_data(G_OBJECT(v_value->d_area),
			"pixmap");

	w = v_value->d_area->allocation.width;
	h = v_value->d_area->allocation.height;

	if ((gboolean)data == TRUE)
	{
		draw_infotext(v_value);
		lo_width = v_value->d_area->allocation.width-info_width;
		len = v_value->data_array->len;
		if (len == 0)	/* If empty */
			return;
		total = len < lo_width/lv_scroll ? len : lo_width/lv_scroll;

		/* Debugging code
		   printf("\nlo_width %i\n",lo_width);
		   printf("total points %i\n",total);
		 */

		// Draw is reverse order, from right to left, 
		// easier to think out in my head... :) 
		//
		for (i=0;i<total;i++)
		{
			val = g_array_index(v_value->data_array,gfloat,len-1-i);
			percent = 1.0-(val/(v_value->upper-v_value->lower));
			pts[i].x = w-(i*lv_scroll)-lv_scroll-1;
			pts[i].y = (gint) (percent*(h-2))+1;
		}
		/* Debugging code
		   i = total - 1;
		   printf("i %i; total %i, last coord is (%i,%i)\n",i,total,pts[i].x,pts[i].y);
		   i--;
		   printf("i %i; total %i, second last coord is (%i,%i)\n",i,total,pts[i].x,pts[i].y);
		   i = 0;
		   printf("i %i; total %i, first coord is (%i,%i)\n",i,total,pts[i].x,pts[i].y);
		   i = 1;
		   printf("i %i; total %i, second coord is (%i,%i)\n",i,total,pts[i].x,pts[i].y);
		 */
		gdk_draw_lines(pixmap,
				v_value->trace_gc,
				pts,
				total);
		return;
	}

	size = v_value->size;
	/* this weird way is needed cause we are accessing various sized
	 * objects from a datastructure in array notation.  This works as 
	 * long as the structure is arranged in largest order first. 
	 * i.e. floats before shorts before chars, etc...
	 */
	uc_ptr = (guchar *)runtime;
	s_ptr = (short *)runtime;
	f_ptr = (float *)runtime;
	switch (size)
	{	
		case FLOAT:
			val = (gfloat)f_ptr[v_value->runtime_offset/FLOAT];
			break;
		case SHORT:
			val = (gshort)s_ptr[v_value->runtime_offset/SHORT];
			break;
		case UCHAR:
			val = (guchar)uc_ptr[v_value->runtime_offset/UCHAR];
			break;
	}
	if (val > (v_value->max))
		v_value->max = val;
	if (val < (v_value->min))
		v_value->min = val;

	g_array_append_val(v_value->data_array,val);

	/* Draw the data.... */
	percent = 1.0-(val/(v_value->upper-v_value->lower));
	if (v_value->last_y == -1)
		v_value->last_y = (gint)(percent*(h-2))+1;

	gdk_draw_line(pixmap,
			v_value->trace_gc,
			w-lv_scroll-1,v_value->last_y,
			w-1,(gint)(percent*(h-2))+1);

	//	printf("drawing line from (%i,%i) to (%i,%i)\n",w-lv_scroll,v_value->last_y,w,(gint)(percent*(h-2))+1);

	v_value->last_y = (gint)((percent*(h-2))+1);

	/* Update textual data */
	draw_infotext(v_value);

}

void scroll_logviewer_traces()
{
	gint start = info_width;
	gint end = info_width;
	gint w = lv_darea->allocation.width;
	gint h = lv_darea->allocation.height;

	start = end + lv_scroll;
	GdkPixmap *pixmap = (GdkPixmap *) g_object_get_data(G_OBJECT(lv_darea),
			"pixmap");

	// Scroll the screen to the left... 
	gdk_draw_drawable(pixmap,
			lv_darea->style->black_gc,
			pixmap,
			info_width+lv_scroll,0,
			info_width,0,
			w-info_width-lv_scroll,h);

	/*	Debugging code..
		printf("\nscreen dimensions: (%i,%i), info_width %i\n",w,h,info_width);
		printf("copying rect starting at (%i,%i), to (%i,%i), w,h of (%i,%i)\n",
		info_width+lv_scroll,0,
		info_width,0,
		w-info_width-lv_scroll,h);
	 */
	// Init new "blank space" as black 
	gdk_draw_rectangle(pixmap,
			lv_darea->style->black_gc,
			TRUE,
			w-lv_scroll,0,
			lv_scroll,h);

	tcount = 0;
	gdk_window_clear(lv_darea->window);
}

gboolean lv_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
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
			g_hash_table_foreach(active_traces, trace_update, (gpointer)TRUE);
			tcount = 0;
		}
		gdk_window_clear(widget->window);
	}

	return TRUE;
}

gboolean lv_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
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

gboolean deselect_all_controls(GtkWidget *widget, gpointer data)
{
	gint i = 0;
	for (i=0;i<max_viewables;i++)
	{
		if (viewables.index[i] == TRUE)
        		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (viewables.widgets[i]),FALSE);
	}
	return TRUE;
}

gboolean select_all_controls(GtkWidget *widget, gpointer data)
{
	gint i = 0;
	for (i=0;i<max_viewables;i++)
	{
		if (viewables.index[i] == FALSE)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (viewables.widgets[i]),TRUE);
	}
	return TRUE;
}
			
gboolean reset_viewables(GtkWidget *widget, gpointer data)
{
	gint i = 0;

	/* disables all controls and de-allocates memory cleanly */
        for (i=0;i<max_viewables;i++)
		viewables.index[i] = FALSE;

	populate_viewer(NULL);

	return FALSE;
	
}
