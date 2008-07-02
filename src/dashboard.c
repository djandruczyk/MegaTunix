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

#include <args.h>
#include <config.h>
#include <xmlbase.h>
#include <dashboard.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <getfiles.h>
#include <gauge.h>
#include <gdk/gdkkeysyms.h>
#include <gui_handlers.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <string.h>
#include <widgetmgmt.h>



extern gint dbg_lvl;
static gboolean timer_active = FALSE;
static volatile gboolean moving = FALSE;
static volatile gboolean resizing = FALSE;
GStaticMutex dash_mutex = G_STATIC_MUTEX_INIT;
extern GObject *global_data;
static volatile gboolean fullscreen = FALSE;


/*!
 \brief load_dashboard() loads the specified dashboard configuration file
 and initializes the dash.
 \param  chooser, the fileshooser that triggered the signal
 \param data, user date
 */
void load_dashboard(gchar *filename, gpointer data)
{
	GtkWidget *window = NULL;
	GtkWidget *dash = NULL;
	GtkWidget *ebox = NULL;
	gchar *key = NULL;
	gchar *prefix = NULL;
	gint width = 0;
	gint height = 0;
	gint x = 0;
	gint y = 0;
	gfloat * ratio = NULL;
	extern GdkColor black;
	//extern GdkColor white;
	extern GtkWidget * main_window;
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	extern gboolean interrogated;

	if (!interrogated)
		return;
	if (filename == NULL)
		return;

	LIBXML_TEST_VERSION

	/*parse the file and get the DOM */
	doc = xmlReadFile(filename, NULL, 0);
	if (doc == NULL)
	{
		printf("error: could not parse file %s\n",filename);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	register_widget(filename,window);
	gtk_window_set_title(GTK_WINDOW(window),"Dash Cluster");
	gtk_window_set_decorated(GTK_WINDOW(window),FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(main_window));


	g_signal_connect(G_OBJECT (window), "configure_event",
			G_CALLBACK (dash_configure_event), NULL);
	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (dummy), NULL);
	g_signal_connect (G_OBJECT (window), "destroy_event",
			G_CALLBACK (dummy), NULL);
	ebox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(window),ebox);

	gtk_widget_add_events(GTK_WIDGET(ebox),GDK_POINTER_MOTION_MASK|
			GDK_BUTTON_PRESS_MASK |GDK_BUTTON_RELEASE_MASK);
			
	g_signal_connect (G_OBJECT (ebox), "motion_notify_event",
			G_CALLBACK (dash_motion_event), NULL);
	g_signal_connect (G_OBJECT (window), "focus-in-event",
			G_CALLBACK (focus_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "button_press_event",
			G_CALLBACK (dash_button_event), NULL);
	g_signal_connect (G_OBJECT (window), "key_press_event",
			G_CALLBACK (dash_key_event), NULL);

	dash = gtk_fixed_new();
	gtk_fixed_set_has_window(GTK_FIXED(dash),TRUE);
	gtk_widget_modify_bg(GTK_WIDGET(dash),GTK_STATE_NORMAL,&black);
	OBJ_SET(window,"dash",dash);
	gtk_container_add(GTK_CONTAINER(ebox),dash);

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	load_elements(dash,root_element);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	link_dash_datasources(dash,data);


	/* Store global info about this dash */
	prefix = g_strdup_printf("dash_%i",(gint)data);
	key = g_strdup_printf("%s_name",prefix);
	g_free(OBJ_GET(global_data,key));
	OBJ_SET(global_data,key, g_strdup(filename));
	g_free(key);
	/* retrieve coord info from global store */
	key = g_strdup_printf("%s_x_origin",prefix);
	x = (gint)OBJ_GET(global_data,key);
	g_free(key);
	key = g_strdup_printf("%s_y_origin",prefix);
	y = (gint)OBJ_GET(global_data,key);
	g_free(key);
	key = g_strdup_printf("%s_size_ratio",prefix);
	ratio = (gfloat *)OBJ_GET(global_data,key);
	g_free(key);
	g_free(prefix);
	g_free(filename);

	width = (gint)OBJ_GET(dash,"orig_width");
	height = (gint)OBJ_GET(dash,"orig_height");
	/*printf("move/resize to %i,%i, %ix%i\n",x,y,width,height); */
	gtk_window_move(GTK_WINDOW(window), x,y);
	if (ratio)
		gtk_window_set_default_size(GTK_WINDOW(window), (gint)(width*(*ratio)),(gint)(height*(*ratio)));
	else
		gtk_window_set_default_size(GTK_WINDOW(window), width,height);
	gtk_widget_show_all(window);
	dash_shape_combine(dash,TRUE);
}

gboolean dash_configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
	gint orig_width = 0;
	gint orig_height = 0;
	gint cur_width = 0;
	gint cur_height = 0;
	gfloat x_ratio = 0.0;
	gfloat y_ratio = 0.0;
	gboolean w_constricted = FALSE;
	GtkFixedChild *child = NULL;
	GtkWidget *gauge = NULL;
	GList *children = NULL;
	gint i = 0;
	gint child_w = 0;
	gint child_h = 0;
	gint child_x = 0;
	gint child_y = 0;
	gfloat ratio = 0.0;
	GtkWidget * dash  = NULL;

	dash = OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
		return FALSE;

	orig_width = (gint) OBJ_GET(dash,"orig_width");
	orig_height = (gint) OBJ_GET(dash,"orig_height");
	cur_width = event->width;
	cur_height = event->height;

	x_ratio = (float)cur_width/(float)orig_width;
	y_ratio = (float)cur_height/(float)orig_height;
	ratio = x_ratio > y_ratio ? y_ratio:x_ratio;
	w_constricted = x_ratio > y_ratio ? FALSE:TRUE;
	
	//printf("dash_config_event\n");
	g_signal_handlers_block_by_func(G_OBJECT(widget),G_CALLBACK(dash_configure_event),NULL);
	children = GTK_FIXED(dash)->children;
	for (i=0;i<g_list_length(children);i++)
	{
		child = g_list_nth_data(children,i);
		gauge = child->widget;
		child_x = (gint)OBJ_GET(gauge,"orig_x_offset");
		child_y = (gint)OBJ_GET(gauge,"orig_y_offset");
		child_w = (gint)OBJ_GET(gauge,"orig_width");
		child_h = (gint)OBJ_GET(gauge,"orig_height");
		if (w_constricted)
			gtk_fixed_move(GTK_FIXED(dash),gauge,ratio*child_x,ratio*child_y+(((y_ratio-x_ratio)*orig_height)/2));
		else
			gtk_fixed_move(GTK_FIXED(dash),gauge,ratio*child_x-(((y_ratio-x_ratio)*orig_width)/2),ratio*child_y);
		gtk_widget_set_size_request(gauge,child_w*ratio,child_h*ratio);
	}
	dash_shape_combine(dash,FALSE);
	if (!timer_active)
	{
		g_timeout_add(4000,hide_dash_resizers,dash);
		timer_active = TRUE;
	}

	g_signal_handlers_unblock_by_func(G_OBJECT(widget),G_CALLBACK(dash_configure_event),NULL);
	return FALSE;
}


void load_elements(GtkWidget *dash, xmlNode *a_node)
{
	xmlNode *cur_node = NULL;

	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"dash_geometry") == 0)
				load_geometry(dash,cur_node);
			if (g_strcasecmp((gchar *)cur_node->name,"gauge") == 0)
				load_gauge(dash,cur_node);
		}
		load_elements(dash,cur_node->children);
	}
}

void load_geometry(GtkWidget *dash, xmlNode *node)
{
	GdkGeometry hints;
	xmlNode *cur_node = NULL;
	gint width = 0;
	gint height = 0;
	if (!node->children)
	{
		printf("ERROR, load_geometry, xml node is empty!!\n");
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"width") == 0)
				generic_xml_gint_import(cur_node,&width);
			if (g_strcasecmp((gchar *)cur_node->name,"height") == 0)
				generic_xml_gint_import(cur_node,&height);
		}
		cur_node = cur_node->next;

	}
	OBJ_SET(dash,"orig_width", GINT_TO_POINTER(width));
	OBJ_SET(dash,"orig_height", GINT_TO_POINTER(height));
	
	hints.min_width = 100;
	hints.min_height = 100;
//	hints.min_aspect = (gfloat)width/(gfloat)height;
//	hints.max_aspect = hints.min_aspect;

	//gtk_window_set_geometry_hints(GTK_WINDOW(gtk_widget_get_toplevel(dash)),NULL,&hints,GDK_HINT_ASPECT|GDK_HINT_MIN_SIZE);
	gtk_window_set_geometry_hints(GTK_WINDOW(gtk_widget_get_toplevel(dash)),NULL,&hints,GDK_HINT_MIN_SIZE);

}

void load_gauge(GtkWidget *dash, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	GtkWidget *gauge = NULL;
	gchar * filename = NULL;
	gint width = 0;
	gint height = 0;
	gint x_offset = 0;
	gint y_offset = 0;
	gchar *xml_name = NULL;
	gchar *datasource = NULL;
	if (!node->children)
	{
		printf("ERROR, load_gauge, xml node is empty!!\n");
		return;
	}
	cur_node = node->children;
	while (cur_node->next) { if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"width") == 0)
				generic_xml_gint_import(cur_node,&width);
			if (g_strcasecmp((gchar *)cur_node->name,"height") == 0)
				generic_xml_gint_import(cur_node,&height);
			if (g_strcasecmp((gchar *)cur_node->name,"x_offset") == 0)
				generic_xml_gint_import(cur_node,&x_offset);
			if (g_strcasecmp((gchar *)cur_node->name,"y_offset") == 0)
				generic_xml_gint_import(cur_node,&y_offset);
			if (g_strcasecmp((gchar *)cur_node->name,"gauge_xml_name") == 0)
				generic_xml_gchar_import(cur_node,&xml_name);
			if (g_strcasecmp((gchar *)cur_node->name,"datasource") == 0)
				generic_xml_gchar_import(cur_node,&datasource);
		}
		cur_node = cur_node->next;

	}
	if (xml_name && datasource)
	{
		gauge = mtx_gauge_face_new();
		gtk_fixed_put(GTK_FIXED(dash),gauge,x_offset,y_offset);
		xml_name = g_strdelimit(xml_name,"\\",'/');
		filename = get_file(g_strconcat(GAUGES_DATA_DIR,PSEP,xml_name,NULL),NULL);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		gtk_widget_set_size_request(gauge,width,height);
		g_free(filename);
		g_free(OBJ_GET(gauge,"datasource"));
		OBJ_SET(gauge,"datasource",g_strdup(datasource));
		OBJ_SET(gauge,"orig_width",GINT_TO_POINTER(width));
		OBJ_SET(gauge,"orig_height",GINT_TO_POINTER(height));
		OBJ_SET(gauge,"orig_x_offset",GINT_TO_POINTER(x_offset));
		OBJ_SET(gauge,"orig_y_offset",GINT_TO_POINTER(y_offset));
		g_free(xml_name);
		g_free(datasource);
	}

}


void link_dash_datasources(GtkWidget *dash,gpointer data)
{
	Dash_Gauge *d_gauge = NULL;
	GtkFixedChild *child;
	GList *children = NULL;
	gint len = 0;
	gint i = 0;
	GObject * rtv_obj = NULL;
	gchar * source = NULL;
	extern GHashTable *dash_gauges;
	extern Rtv_Map *rtv_map;

	if(!GTK_IS_FIXED(dash))
		return;
	
	if (!dash_gauges)
		dash_gauges = g_hash_table_new(g_str_hash,g_str_equal);

	children = GTK_FIXED(dash)->children;
	len = g_list_length(children);

	for (i=0;i<len;i++)
	{
		child = g_list_nth_data(children,i);
		source = (gchar *)OBJ_GET(child->widget,"datasource");
		if (!source)
			continue;

		if (!rtv_map)
			return;
		if (!(rtv_map->rtv_hash))
			return;
		rtv_obj = g_hash_table_lookup(rtv_map->rtv_hash,source);
		if (!G_IS_OBJECT(rtv_obj))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": link_dash_datasourcesn\tBad things man, object doesn't exist for %s\n",source));
			continue ;
		}
		d_gauge = g_new0(Dash_Gauge, 1);
		d_gauge->object = rtv_obj;
		d_gauge->source = source;
		d_gauge->gauge = child->widget;
		d_gauge->dash = dash;
		g_hash_table_insert(dash_gauges,g_strdup_printf("dash_%i_gauge_%i",(gint)data,i),(gpointer)d_gauge);

	}
}

void update_dash_gauge(gpointer key, gpointer value, gpointer user_data)
{
	Dash_Gauge *d_gauge = (Dash_Gauge *)value;
	extern GStaticMutex rtv_mutex;
	GArray *history;
	gint current_index = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	extern gboolean forced_update;

	GtkWidget *gauge = NULL;
	
	gauge = d_gauge->gauge;
	/*printf("updating gauge %s\n",(gchar *)key);*/

	history = (GArray *)OBJ_GET(d_gauge->object,"history");
	current_index = (gint)OBJ_GET(d_gauge->object,"current_index");
	g_static_mutex_lock(&rtv_mutex);
	current = g_array_index(history, gfloat, current_index);
	if (current_index > 0)
		current_index-=1;
	previous = g_array_index(history, gfloat, current_index);
	g_static_mutex_unlock(&rtv_mutex);

	if ((current != previous) || (forced_update))
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),current);

}


void dash_shape_combine(GtkWidget *dash, gboolean hide_resizers)
{
	GtkFixedChild *child = NULL;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	gint xc = 0;
	gint yc = 0;
	gint radius = 0;
	gint i = 0;
	GList *children = NULL;
	GdkGC *gc1 = NULL;
	GdkBitmap *bitmap = NULL;
	GtkRequisition req;
	gint width = 0;
	gint height = 0;
	static GdkColormap *colormap = NULL;
	static GdkColor black;
	static GdkColor white;

	if(!GTK_IS_WIDGET(dash))
		return;
	if(!GTK_IS_WINDOW(gtk_widget_get_toplevel(dash)))
		return;

	g_static_mutex_lock(&dash_mutex);
	gtk_window_get_size(GTK_WINDOW(gtk_widget_get_toplevel(dash)),&width,&height);
	if (!colormap)
	{
		colormap = gdk_colormap_get_system ();
		gdk_color_parse ("black", & black);
		gdk_colormap_alloc_color(colormap, &black,TRUE,TRUE);
		gdk_color_parse ("white", & white);
		gdk_colormap_alloc_color(colormap, &white,TRUE,TRUE);
	}
	bitmap = gdk_pixmap_new(NULL,width,height,1);
	gc1 = gdk_gc_new (bitmap);
	gdk_gc_set_foreground (gc1, &black);
	gdk_draw_rectangle(bitmap,gc1,TRUE,0,0,width,height);

	if (hide_resizers == FALSE)
	{
		gdk_gc_set_foreground (gc1, &white);
		gdk_draw_rectangle(bitmap,gc1,TRUE,width-16,0,16,16);
		gdk_draw_rectangle(bitmap,gc1,TRUE,0,0,16,16);
		gdk_draw_rectangle(bitmap,gc1,TRUE,0,height-16,16,16);
		gdk_draw_rectangle(bitmap,gc1,TRUE,width-16,height-16,16,16);

		gdk_gc_set_foreground (gc1, &black);
		gdk_draw_rectangle(bitmap,gc1,TRUE,width-16,3,13,13);
		gdk_draw_rectangle(bitmap,gc1,TRUE,3,3,13,13);
		gdk_draw_rectangle(bitmap,gc1,TRUE,3,height-16,13,13);
		gdk_draw_rectangle(bitmap,gc1,TRUE,width-16,height-16,13,13);
	}

	gdk_gc_set_foreground (gc1, &white);
	if (fullscreen)
		gdk_draw_rectangle(bitmap,gc1,TRUE,0,0,width,height);
	else
	{
		children = GTK_FIXED(dash)->children;
		for (i=0;i<g_list_length(children);i++)
		{
			child = g_list_nth_data(children,i);
			x = child->x;
			y = child->y;
			gtk_widget_size_request(child->widget,&req);
			w = req.width;
			h = req.height;
			radius = MIN(w,h)/2;
			xc = x+w/2;
			yc = y+h/2;
			gdk_draw_arc (bitmap,
					gc1,
					TRUE,     /* filled */
					xc-radius,
					yc-radius,
					2*radius,
					2*radius,
					0,        /* angle 1 */
					360*64);  /* angle 2: full circle */
		}
	}
	if (GTK_IS_WINDOW(gtk_widget_get_toplevel(dash)))
	{
#ifdef HAVE_CAIRO
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
		{
			gtk_widget_input_shape_combine_mask(gtk_widget_get_toplevel(dash),bitmap,0,0);
		}
#endif
#endif
		gtk_widget_shape_combine_mask(gtk_widget_get_toplevel(dash),bitmap,0,0);
	}
	else
	{
#ifdef HAVE_CAIRO
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
		{
			gdk_window_input_shape_combine_mask(dash->window,bitmap,0,0);
		}
#endif
#endif
		gdk_window_shape_combine_mask(dash->window,bitmap,0,0);
	}
	g_object_unref(colormap);
	g_object_unref(gc1);
	g_object_unref(bitmap);
	g_static_mutex_unlock(&dash_mutex);
	return;
}

gboolean dash_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	GtkWidget *dash = GTK_BIN(widget)->child;
	if (!timer_active)
	{
		dash_shape_combine(dash,FALSE);
		g_timeout_add(4000,hide_dash_resizers,dash);
		timer_active = TRUE;
	}
	return TRUE;
}

gboolean dash_key_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	extern GtkWidget *main_window;
	extern GtkWidget *rtt_window;
	extern GtkWidget *status_window;
	if (event->type == GDK_KEY_RELEASE)
		return FALSE;

	switch (event->keyval)
	{
		case GDK_q:
		case GDK_Q:
				leave(NULL,NULL);
			break;
		case GDK_M:
		case GDK_m:
			if (GTK_WIDGET_VISIBLE(main_window))
				gtk_widget_hide (main_window);
			else
				gtk_widget_show_all(main_window);
			break;
		case GDK_R:
		case GDK_r:
			if (!GTK_IS_WIDGET(rtt_window))
				break;
			if (GTK_WIDGET_VISIBLE(rtt_window))
				gtk_widget_hide (rtt_window);
			else
				gtk_widget_show_all(rtt_window);
			break;
		case GDK_S:
		case GDK_s:
			if (!GTK_IS_WIDGET(status_window))
				break;
			if (GTK_WIDGET_VISIBLE(status_window))
				gtk_widget_hide (status_window);
			else
				gtk_widget_show_all(status_window);
			break;
	}
	return TRUE;
}


gboolean dash_button_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	/*printf("button event\n"); */
	gint edge = -1;

	GtkWidget *dash = GTK_BIN(widget)->child;
	if (!timer_active)
	{
		dash_shape_combine(dash,FALSE);
		g_timeout_add(4000,hide_dash_resizers,dash);
		timer_active = TRUE;
	}

	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 1))
	{
		/*printf("dash button event\n"); */
		if (event->x > (widget->allocation.width-16))
		{
			/* Upper portion */
			if (event->y < 16)
				edge = GDK_WINDOW_EDGE_NORTH_EAST;
			/* Lower portion */
			else if (event->y > (widget->allocation.height-16))
				edge = GDK_WINDOW_EDGE_SOUTH_EAST;
			else
				edge = -1;
		}
		/* Left Side of window */
		else if (event->x < 16)
		{
			/* If it's in the middle portion */
			/* Upper portion */
			if (event->y < 16)
				edge = GDK_WINDOW_EDGE_NORTH_WEST;
			/* Lower portion */
			else if (event->y > (widget->allocation.height-16))
				edge = GDK_WINDOW_EDGE_SOUTH_WEST;
			else
				edge = -1;
		}
		else
			edge = -1;


		if ((edge == -1 ) && (GTK_IS_WINDOW(widget->parent)))
		{
			/*printf("MOVE drag\n"); */
			moving = TRUE;
			g_signal_handlers_block_by_func(G_OBJECT(gtk_widget_get_toplevel(widget)),G_CALLBACK(dash_configure_event),NULL);
			gtk_window_begin_move_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
					event->button,
					event->x_root,
					event->y_root,
					event->time);
			return TRUE;
		}
		else if (GTK_IS_WINDOW(widget->parent))
		{
			/*printf("RESIZE drag\n"); */
			resizing = TRUE;
			gtk_window_begin_resize_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
					edge,
					event->button,
					event->x_root,
					event->y_root,
					event->time);
		}
	}
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
	{
		if (fullscreen)
		{
			fullscreen = FALSE;
			gtk_window_unfullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
		}
		else
		{
			fullscreen = TRUE;
			gtk_window_fullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
		}
	}
	return FALSE;
}

EXPORT void initialize_dashboards_pf()
{
	GtkWidget * label = NULL;
	extern GtkWidget *main_window;
	gboolean retval = FALSE;
	gchar * tmpbuf = NULL;
	gchar * tmpstr = NULL;
	gboolean nodash1 = TRUE;
	gboolean nodash2 = TRUE;
	extern GHashTable *dynamic_widgets;
	CmdLineArgs *args = OBJ_GET(global_data,"args");

	label = g_hash_table_lookup(dynamic_widgets,"dash_1_label");
	if (OBJ_GET(global_data,"dash_1_name") != NULL)
		tmpbuf = (gchar *)OBJ_GET(global_data,"dash_1_name");
	if ((GTK_IS_LABEL(label)) && (tmpbuf != NULL) && (strlen(tmpbuf) != 0))
	{
		tmpstr = g_filename_to_utf8(tmpbuf,-1,NULL,NULL,NULL);
		gtk_label_set_text(GTK_LABEL(label),tmpstr);
		g_free(tmpstr);
		load_dashboard(g_strdup(tmpbuf),GINT_TO_POINTER(1));
		tmpbuf = NULL;
		nodash1 = FALSE;
	}

	label = g_hash_table_lookup(dynamic_widgets,"dash_2_label");
	if (OBJ_GET(global_data,"dash_2_name") != NULL)
		tmpbuf = (gchar *)OBJ_GET(global_data,"dash_2_name");
	if ((GTK_IS_LABEL(label)) && (tmpbuf != NULL) && (strlen(tmpbuf) != 0))
	{
		tmpstr = g_filename_to_utf8(tmpbuf,-1,NULL,NULL,NULL);
		gtk_label_set_text(GTK_LABEL(label),tmpstr);
		g_free(tmpstr);
		load_dashboard(g_strdup(tmpbuf),GINT_TO_POINTER(2));
		tmpbuf = NULL;
		nodash2 = FALSE;
	}
	/* Case to handle when no default dashboards are set, but the user
	 * choose to run with no main gui (thus can't quit or select a dash)
	 * So we force the dash chooser
	 */
	if ((nodash1) && (nodash2) && (args->hide_maingui))
	{
		retval = present_dash_filechooser(NULL,GINT_TO_POINTER(1));
		return;
		if (!retval)
		{
			CmdLineArgs *args = OBJ_GET(global_data,"args");
			args->be_quiet = TRUE;
			g_signal_emit_by_name(main_window,"destroy_event");
		}

	}
}


EXPORT gboolean present_dash_filechooser(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GtkWidget *label = NULL;
	extern GtkWidget *main_window;
	extern gboolean interrogated;
	extern GHashTable *dash_gauges;

	if (!interrogated)
		return FALSE;

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("Dashboards");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Select Dashboard to Open");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->shortcut_folders = g_strdup("Dashboards");


	filename = choose_file(fileio);
	free_mtxfileio(fileio);
	if (filename)
	{
		if (dash_gauges)
			g_hash_table_foreach_remove(dash_gauges,remove_dashcluster,data);
		if (GTK_IS_WIDGET(widget))
		{
			label = OBJ_GET(widget,"label");
			if (GTK_IS_LABEL(label))
				gtk_label_set_text(GTK_LABEL(label),g_filename_to_utf8(filename,-1,NULL,NULL,NULL));
		}
		load_dashboard(filename,data);
		return TRUE;
	}
	else
	      	return FALSE;

}



gboolean remove_dashboard(GtkWidget *widget, gpointer data)
{
	extern GHashTable *dash_gauges;
	GtkWidget *label = NULL;

	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		return FALSE;

	g_static_mutex_lock(&dash_mutex);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),FALSE);
	label = OBJ_GET(widget,"label");
	if (GTK_IS_WIDGET(label))
	{
		gtk_label_set_text(GTK_LABEL(label),"Choose a Dashboard File");
		if ((gint)data == 1)
		{
			g_free(OBJ_GET(global_data,"dash_1_name"));
			OBJ_SET(global_data,"dash_1_name",NULL);
		}
		if ((gint)data == 2)
		{
			g_free(OBJ_GET(global_data,"dash_2_name"));
			OBJ_SET(global_data,"dash_2_name",NULL);
		}
	}
	if (dash_gauges)
		g_hash_table_foreach_remove(dash_gauges,remove_dashcluster,data);
	g_static_mutex_unlock(&dash_mutex);
	return TRUE;
}

gboolean remove_dashcluster(gpointer key, gpointer value, gpointer user_data)
{
	gchar *tmpbuf = NULL;
	Dash_Gauge *d_gauge = NULL;

	tmpbuf = g_strdup_printf("dash_%i",(gint)user_data);
	if (g_strrstr((gchar *)key,tmpbuf) != NULL)
	{
		g_free(tmpbuf);
		/* Found gauge in soon to be destroyed dash */
		d_gauge = (Dash_Gauge *)value;
		g_free(d_gauge->source);
		if (GTK_IS_WIDGET(d_gauge->dash))
			gtk_widget_destroy(gtk_widget_get_toplevel(d_gauge->dash));
		return TRUE;
	}
	else
		g_free(tmpbuf);

	return FALSE;
}

gboolean dummy(GtkWidget *widget,gpointer data)
{
	return TRUE;
}


EXPORT void create_gauge(GtkWidget *widget)
{
	GtkWidget * gauge = NULL;
	gchar * xml_name = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gint table_num = -1;
	extern GList **tab_gauges;

	gauge = mtx_gauge_face_new();
	gtk_container_add(GTK_CONTAINER(widget),gauge);
	xml_name = OBJ_GET(widget,"gaugexml");
	if (xml_name)
		filename = get_file(g_strconcat(GAUGES_DATA_DIR,PSEP,xml_name,NULL),NULL);
	if (filename)
	{
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		g_free(filename);
	}
	g_free(OBJ_GET(gauge,"datasource"));
	OBJ_SET(gauge,"datasource",OBJ_GET(widget,"datasource"));
	tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
	table_num = (gint)g_ascii_strtod(tmpbuf,NULL);
	tab_gauges[table_num] = g_list_prepend(tab_gauges[table_num],gauge);
}

void update_tab_gauges()
{
	extern gint active_table;
	extern GList **tab_gauges;
	GtkWidget *gauge = NULL;
	gchar * source = NULL;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	extern gboolean forced_update;
	gint i = 0;
	GList *list = NULL;
	
	if ((!tab_gauges) || (active_table < 0))
		return;
	list = g_list_first(tab_gauges[active_table]);
	for (i=0;i<g_list_length(list);i++)
	{
		gauge = g_list_nth_data(list,i);
		source = OBJ_GET(gauge,"datasource");
		lookup_current_value(source,&current);
		lookup_previous_value(source,&previous);
		if ((current != previous) || (forced_update))
			mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),current);
	}

}


gboolean hide_dash_resizers(gpointer data)
{
	if (GTK_IS_WIDGET(data))
		dash_shape_combine(data,TRUE);
	timer_active = FALSE;
	return FALSE;
}

gboolean focus_event(GtkWidget *widget, gpointer data)
{
	if (moving)
	{
		g_signal_handlers_unblock_by_func(G_OBJECT(gtk_widget_get_toplevel(widget)),G_CALLBACK(dash_configure_event),NULL);
		moving = FALSE;
	}
	if (resizing)
	{
		resizing = FALSE;
		gtk_widget_queue_draw(widget);
	}
	return FALSE;

}
