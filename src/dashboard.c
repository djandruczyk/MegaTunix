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
#include <rtv_processor.h>
#include <structures.h>
#include <widgetmgmt.h>


gboolean dash_configure_event(GtkWidget * , GdkEventConfigure * );
extern gint dbg_lvl;


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
	extern GObject * global_data;
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

	g_signal_connect (G_OBJECT (window), "configure_event",
			G_CALLBACK (dash_configure_event), NULL);
	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (dummy), NULL);
	g_signal_connect (G_OBJECT (window), "destroy_event",
			G_CALLBACK (dummy), NULL);
	ebox = gtk_event_box_new();
	//gtk_event_box_set_visible_window(GTK_EVENT_BOX(ebox), FALSE);
	gtk_container_add(GTK_CONTAINER(window),ebox);

	gtk_widget_add_events(GTK_WIDGET(ebox),GDK_POINTER_MOTION_MASK|
			GDK_BUTTON_PRESS_MASK |GDK_BUTTON_RELEASE_MASK);
	g_signal_connect (G_OBJECT (ebox), "motion_notify_event",
			G_CALLBACK (dash_motion_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "button_press_event",
			G_CALLBACK (dash_button_event), NULL);
	g_signal_connect (G_OBJECT (window), "key_press_event",
			G_CALLBACK (dash_key_event), NULL);


	dash = gtk_fixed_new();
	gtk_fixed_set_has_window(GTK_FIXED(dash),TRUE);
	g_object_set_data(G_OBJECT(window),"dash",dash);
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
	g_object_set_data(global_data,key, g_strdup(filename));
	g_free(key);
	/* retrieve coord info from global store */
	key = g_strdup_printf("%s_x_origin",prefix);
	x = (gint)g_object_get_data(global_data,key);
	g_free(key);
	key = g_strdup_printf("%s_y_origin",prefix);
	y = (gint)g_object_get_data(global_data,key);
	g_free(key);
	key = g_strdup_printf("%s_size_ratio",prefix);
	ratio = (gfloat *)g_object_get_data(global_data,key);
	g_free(key);
	g_free(prefix);
	g_free(filename);

	width = (gint)g_object_get_data(G_OBJECT(dash),"orig_width");
	height = (gint)g_object_get_data(G_OBJECT(dash),"orig_height");
//	printf("move/resize to %i,%i, %ix%i\n",x,y,width,height);
	gtk_window_move(GTK_WINDOW(window), x,y);
	if (ratio)
		gtk_window_set_default_size(GTK_WINDOW(window), width**ratio,height**ratio);
	else
		gtk_window_set_default_size(GTK_WINDOW(window), width,height);
	gtk_widget_show_all(window);
	dash_shape_combine(dash);
}

gboolean dash_configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
	gint orig_width = 0;
	gint orig_height = 0;
	gint cur_width = 0;
	gint cur_height = 0;
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


	dash = g_object_get_data(G_OBJECT(widget),"dash");
	if (!GTK_IS_WIDGET(dash))
		return FALSE;

	orig_width = (gint) g_object_get_data(G_OBJECT(dash),"orig_width");
	orig_height = (gint) g_object_get_data(G_OBJECT(dash),"orig_height");
	cur_width =  event->width;
	cur_height =  event->height;

	ratio = (((gfloat)cur_height/(gfloat)orig_height)+((gfloat)cur_width/(gfloat)orig_width))/2.0;

	if (((ratio - (gint)ratio)*100) < 1)
		return FALSE;

	g_signal_handlers_block_by_func(G_OBJECT(widget),G_CALLBACK(dash_configure_event),NULL);
	children = GTK_FIXED(dash)->children;
	for (i=0;i<g_list_length(children);i++)
	{
		child = g_list_nth_data(children,i);
		gauge = child->widget;
		child_x = (gint)g_object_get_data(G_OBJECT(gauge),"orig_x_offset");
		child_y = (gint)g_object_get_data(G_OBJECT(gauge),"orig_y_offset");
		child_w = (gint)g_object_get_data(G_OBJECT(gauge),"orig_width");
		child_h = (gint)g_object_get_data(G_OBJECT(gauge),"orig_height");
		gtk_fixed_move(GTK_FIXED(dash),gauge,ratio*child_x,ratio*child_y);
		gtk_widget_set_size_request(gauge,child_w*ratio,child_h*ratio);
	}

	dash_shape_combine(dash);

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
				load_integer_from_xml(cur_node,&width);
			if (g_strcasecmp((gchar *)cur_node->name,"height") == 0)
				load_integer_from_xml(cur_node,&height);
		}
		cur_node = cur_node->next;

	}
	g_object_set_data(G_OBJECT(dash),"orig_width", GINT_TO_POINTER(width));
	g_object_set_data(G_OBJECT(dash),"orig_height", GINT_TO_POINTER(height));
	
	hints.min_width = 100;
	hints.min_height = 100;
	hints.min_aspect = (gfloat)width/(gfloat)height;
	hints.max_aspect = hints.min_aspect;

	gtk_window_set_geometry_hints(GTK_WINDOW(gtk_widget_get_toplevel(dash)),NULL,&hints,GDK_HINT_ASPECT|GDK_HINT_MIN_SIZE);

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
				load_integer_from_xml(cur_node,&width);
			if (g_strcasecmp((gchar *)cur_node->name,"height") == 0)
				load_integer_from_xml(cur_node,&height);
			if (g_strcasecmp((gchar *)cur_node->name,"x_offset") == 0)
				load_integer_from_xml(cur_node,&x_offset);
			if (g_strcasecmp((gchar *)cur_node->name,"y_offset") == 0)
				load_integer_from_xml(cur_node,&y_offset);
			if (g_strcasecmp((gchar *)cur_node->name,"gauge_xml_name") == 0)
				load_string_from_xml(cur_node,&xml_name);
			if (g_strcasecmp((gchar *)cur_node->name,"datasource") == 0)
				load_string_from_xml(cur_node,&datasource);
		}
		cur_node = cur_node->next;

	}
	if (xml_name && datasource)
	{
		gauge = mtx_gauge_face_new();
		gtk_fixed_put(GTK_FIXED(dash),gauge,x_offset,y_offset);
		filename = get_file(g_strconcat(GAUGES_DATA_DIR,PSEP,xml_name,NULL),NULL);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		gtk_widget_set_size_request(gauge,width,height);
		g_free(filename);
		g_object_set_data(G_OBJECT(gauge),"datasource",g_strdup(datasource));
		g_object_set_data(G_OBJECT(gauge),"orig_width",GINT_TO_POINTER(width));
		g_object_set_data(G_OBJECT(gauge),"orig_height",GINT_TO_POINTER(height));
		g_object_set_data(G_OBJECT(gauge),"orig_x_offset",GINT_TO_POINTER(x_offset));
		g_object_set_data(G_OBJECT(gauge),"orig_y_offset",GINT_TO_POINTER(y_offset));
		g_free(xml_name);
		g_free(datasource);
	}

}

void load_integer_from_xml(xmlNode *node, gint *dest)
{
	if (!node->children)
	{
		printf("ERROR, load_integer_from_xml, xml node is empty!!\n");
		return;
	}
	if (!(node->children->type == XML_TEXT_NODE))
		return;
	*dest = (gint)g_ascii_strtod((gchar*)node->children->content,NULL);

}

void load_string_from_xml(xmlNode *node, gchar **dest)
{
	if (!node->children) /* EMPTY node, thus, clear the var on the gauge */
	{
		if (*dest)
			g_free(*dest);
		*dest = g_strdup("");
		return;
	}
	if (!(node->children->type == XML_TEXT_NODE))
		return;

	if (*dest)
		g_free(*dest);
	if (node->children->content)
		*dest = g_strdup((gchar*)node->children->content);
	else
		*dest = g_strdup("");

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
		source = (gchar *)g_object_get_data(G_OBJECT(child->widget),"datasource");
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
//	printf("updating gauge %s\n",(gchar *)key);

	history = (GArray *)g_object_get_data(d_gauge->object,"history");
	current_index = (gint)g_object_get_data(d_gauge->object,"current_index");
	g_static_mutex_lock(&rtv_mutex);
	current = g_array_index(history, gfloat, current_index);
	if (current_index > 0)
		current_index-=1;
	previous = g_array_index(history, gfloat, current_index);
	g_static_mutex_unlock(&rtv_mutex);

	if ((current != previous) || (forced_update))
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),current);

}


void dash_shape_combine(GtkWidget *dash)
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
	GdkGC *gc = NULL;
	GdkColormap *colormap = NULL;
	GdkColor black;
	GdkColor white;
	GdkBitmap *bitmap = NULL;
	GtkRequisition req;
	gint width = 0;
	gint height = 0;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);
	gtk_window_get_size(GTK_WINDOW(gtk_widget_get_toplevel(dash)),&width,&height);
	bitmap = gdk_pixmap_new(NULL,width,height,1);

	colormap = gdk_colormap_get_system ();
	gdk_color_parse ("black", & black);
	gdk_colormap_alloc_color(colormap, &black,TRUE,TRUE);
	gdk_color_parse ("white", & white);
	gdk_colormap_alloc_color(colormap, &white,TRUE,TRUE);
	gc = gdk_gc_new (bitmap);
	gdk_gc_set_foreground (gc, &black);
	gdk_draw_rectangle(bitmap,gc,TRUE,0,0,width,height);

	gdk_gc_set_foreground (gc, &white);
	gdk_draw_rectangle(bitmap,gc,TRUE,width-8,0,8,8);
	gdk_draw_rectangle(bitmap,gc,TRUE,0,0,8,8);
	gdk_draw_rectangle(bitmap,gc,TRUE,0,height-8,8,8);
	gdk_draw_rectangle(bitmap,gc,TRUE,width-8,height-8,8,8);


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
				gc,
				TRUE,     // filled
				xc-radius,
				yc-radius,
				2*radius,
				2*radius,
				0,        // angle 1
				360*64);  // angle 2: full circle

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
	g_object_unref(gc);
	g_object_unref(bitmap);
	g_static_mutex_unlock(&mutex);
	return;
}

gboolean dash_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
//	printf("motion detected\n");
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
			if (GTK_WIDGET_VISIBLE(main_window))
				leave(NULL,GINT_TO_POINTER(FALSE));
			else
				leave(NULL,GINT_TO_POINTER(TRUE));
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
			if (GTK_WIDGET_VISIBLE(rtt_window))
				gtk_widget_hide (rtt_window);
			else
				gtk_widget_show_all(rtt_window);
			break;
		case GDK_S:
		case GDK_s:
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
//	printf("button event\n");
	gint edge = -1;

	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 1))
	{
//		printf("dash button event\n");
		if (event->x > (widget->allocation.width-8))
		{
			/* Upper portion */
			if (event->y < 8)
				edge = GDK_WINDOW_EDGE_NORTH_EAST;
			/* Lower portion */
			else if (event->y > (widget->allocation.height-8))
				edge = GDK_WINDOW_EDGE_SOUTH_EAST;
			else
				edge = -1;
		}
		/* Left Side of window */
		else if (event->x < 8)
		{
			/* If it's in the middle portion */
			/* Upper portion */
			if (event->y < 8)
				edge = GDK_WINDOW_EDGE_NORTH_WEST;
			/* Lower portion */
			else if (event->y > (widget->allocation.height-8))
				edge = GDK_WINDOW_EDGE_SOUTH_WEST;
			else
				edge = -1;
		}
		else
			edge = -1;

	
		if ((edge == -1 ) && (GTK_IS_WINDOW(widget->parent)))
		{
//			printf("MOVE drag\n");
			gtk_window_begin_move_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
					event->button,
					event->x_root,
					event->y_root,
					event->time);
			return TRUE;
		}
		else if (GTK_IS_WINDOW(widget->parent))
		{
//			printf("RESIZE drag\n");
			gtk_window_begin_resize_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
					edge,
					event->button,
					event->x_root,
					event->y_root,
					event->time);
		}
	}
	return FALSE;
}

void initialize_dashboards()
{
	GtkWidget * label = NULL;
	gchar * tmpbuf = NULL;
	gchar * tmpstr = NULL;
	extern GObject *global_data;
	extern GHashTable *dynamic_widgets;

	label = g_hash_table_lookup(dynamic_widgets,"dash_1_label");
	tmpbuf = (gchar *)g_object_get_data(global_data,"dash_1_name");
	if ((GTK_IS_LABEL(label)) && (tmpbuf != NULL) && (g_ascii_strcasecmp(tmpbuf,"") != 0))
	{
		tmpstr = g_filename_to_utf8(tmpbuf,-1,NULL,NULL,NULL);
		gtk_label_set_text(GTK_LABEL(label),tmpstr);
		g_free(tmpstr);
		load_dashboard(g_strdup(tmpbuf),GINT_TO_POINTER(1));
	}

	label = g_hash_table_lookup(dynamic_widgets,"dash_2_label");
	tmpbuf = (gchar *)g_object_get_data(global_data,"dash_2_name");
	if ((GTK_IS_LABEL(label)) && (tmpbuf != NULL) && (g_ascii_strcasecmp(tmpbuf,"") != 0))
	{
		tmpstr = g_filename_to_utf8(tmpbuf,-1,NULL,NULL,NULL);
		gtk_label_set_text(GTK_LABEL(label),tmpstr);
		g_free(tmpstr);
		load_dashboard(g_strdup(tmpbuf),GINT_TO_POINTER(2));
	}
}


EXPORT gboolean present_dash_filechooser(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GtkWidget *label = NULL;
	extern GtkWidget *main_window;
	extern gboolean interrogated;


	if (!interrogated)
		return FALSE;
	extern GHashTable *dash_gauges;

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("Dashboards");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Select Dashboard to Open");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->shortcut_folders = g_strdup("Dashboards");


	filename = choose_file(fileio);
	if (filename)
	{
		if (dash_gauges)
			g_hash_table_foreach_remove(dash_gauges,remove_dashcluster,data);
		label = g_object_get_data(G_OBJECT(widget),"label");
		if (GTK_IS_LABEL(label))
			gtk_label_set_text(GTK_LABEL(label),g_filename_to_utf8(filename,-1,NULL,NULL,NULL));
		load_dashboard(filename,data);
	}

	free_mtxfileio(fileio);

	return TRUE;
}



gboolean remove_dashboard(GtkWidget *widget, gpointer data)
{
	extern GHashTable *dash_gauges;
	GtkWidget *label = NULL;
	gchar *tmpbuf = NULL;
	extern GObject *global_data;

	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		return FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),FALSE);
	label = g_object_get_data(G_OBJECT(widget),"label");
	if (GTK_IS_WIDGET(label))
	{
		gtk_label_set_text(GTK_LABEL(label),"Choose a Dashboard File");
		if ((gint)data == 1)
		{
			tmpbuf = (gchar *)g_object_get_data(global_data,"dash_1_name");
			if (tmpbuf)
			{
				g_free(tmpbuf);
				g_object_set_data(global_data,"dash_1_name",NULL);
			}
		}
		if ((gint)data == 2)
		{
			tmpbuf = (gchar *)g_object_get_data(global_data,"dash_2_name");
			if (tmpbuf)
			{
				g_free(tmpbuf);
				g_object_set_data(global_data,"dash_2_name",NULL);
			}
		}
	}
	if (dash_gauges)
		g_hash_table_foreach_remove(dash_gauges,remove_dashcluster,data);
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
		/* Foudn gauge in soon to be destroyed dash */
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
	xml_name = g_object_get_data(G_OBJECT(widget),"gaugexml");
	if (xml_name)
		filename = get_file(g_strconcat(GAUGES_DATA_DIR,PSEP,xml_name,NULL),NULL);
	if (filename)
	{
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		g_free(filename);
	}
	g_object_set_data(G_OBJECT(gauge),"datasource",g_object_get_data(G_OBJECT(widget),"datasource"));
	tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"table_num");
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
		source = g_object_get_data(G_OBJECT(gauge),"datasource");
		lookup_current_value(source,&current);
		lookup_previous_value(source,&previous);
		if ((current != previous) || (forced_update))
			mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),current);
	}

}
