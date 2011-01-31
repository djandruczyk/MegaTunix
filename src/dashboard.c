/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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



static gboolean timer_active = FALSE;
static volatile gboolean moving = FALSE;
static volatile gboolean resizing = FALSE;
extern gconstpointer *global_data;

/*!
 \brief load_dashboard() loads the specified dashboard configuration file
 and initializes the dash.
 \param  chooser, the fileshooser that triggered the signal
 \param data, user date
 */
G_MODULE_EXPORT void load_dashboard(gchar *filename, gpointer data)
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
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;

	if (!DATA_GET(global_data,"interrogated"))
		return;
	if (filename == NULL)
		return;

	LIBXML_TEST_VERSION

		/*parse the file and get the DOM */
		doc = xmlReadFile(filename, NULL, 0);
	if (doc == NULL)
	{
		printf(_("Error: could not parse dashboard XML file %s"),filename);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	register_widget(filename,window);
	gtk_window_set_title(GTK_WINDOW(window),_("Dash Cluster"));
	gtk_window_set_decorated(GTK_WINDOW(window),FALSE);

	g_signal_connect(G_OBJECT (window), "configure_event",
			G_CALLBACK (dash_configure_event), NULL);
	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (dummy), NULL);
	g_signal_connect (G_OBJECT (window), "destroy_event",
			G_CALLBACK (dummy), NULL);
	ebox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(window),ebox);

	gtk_widget_add_events(GTK_WIDGET(ebox),
			GDK_POINTER_MOTION_MASK|
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_KEY_PRESS_MASK 
			);

	g_signal_connect (G_OBJECT (ebox), "motion_notify_event",
			G_CALLBACK (dash_motion_event), NULL);
	g_signal_connect (G_OBJECT (window), "focus-in-event",
			G_CALLBACK (focus_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "button_press_event",
			G_CALLBACK (dash_button_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "popup-menu",
			G_CALLBACK (dash_popup_menu_handler), NULL);
	g_signal_connect (G_OBJECT (window), "key_press_event",
			G_CALLBACK (dash_key_event), NULL);

	dash = gtk_fixed_new();
	gtk_fixed_set_has_window(GTK_FIXED(dash),TRUE);
	gtk_widget_modify_bg(GTK_WIDGET(dash),GTK_STATE_NORMAL,&black);
	OBJ_SET(window,"dash",dash);
	OBJ_SET(ebox,"dash",dash);
	gtk_container_add(GTK_CONTAINER(ebox),dash);

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	load_elements(dash,root_element);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	link_dash_datasources(dash,data);


	/* Store global info about this dash */
	prefix = g_strdup_printf("dash_%i",(GINT)data);
	key = g_strdup_printf("%s_name",prefix);
	DATA_SET_FULL(global_data,key, g_strdup(filename),g_free);
	g_free(key);
	/* retrieve coord info from global store */
	key = g_strdup_printf("%s_x_origin",prefix);
	x = (GINT)DATA_GET(global_data,key);
	g_free(key);
	key = g_strdup_printf("%s_y_origin",prefix);
	y = (GINT)DATA_GET(global_data,key);
	g_free(key);
	key = g_strdup_printf("%s_size_ratio",prefix);
	ratio = (gfloat *)DATA_GET(global_data,key);
	g_free(key);
	g_free(prefix);
	g_free(filename);
	OBJ_SET(ebox,"index", data);

	width = (GINT)OBJ_GET(dash,"orig_width");
	height = (GINT)OBJ_GET(dash,"orig_height");
	/*printf("move/resize to %i,%i, %ix%i\n",x,y,width,height); */
	gtk_window_move(GTK_WINDOW(window), x,y);
	if (ratio)
		gtk_window_set_default_size(GTK_WINDOW(window), (gint)(width*(*ratio)),(gint)(height*(*ratio)));
	else
		gtk_window_set_default_size(GTK_WINDOW(window), width,height);
	gtk_widget_show_all(window);
	dash_shape_combine(dash,TRUE);
}

G_MODULE_EXPORT gboolean dash_configure_event(GtkWidget *widget, GdkEventConfigure *event)
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
	guint i = 0;
	gint child_w = 0;
	gint child_h = 0;
	gint child_x = 0;
	gint child_y = 0;
	gfloat ratio = 0.0;
	GtkWidget * dash  = NULL;

	dash = OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
		return FALSE;

	orig_width = (GINT) OBJ_GET(dash,"orig_width");
	orig_height = (GINT) OBJ_GET(dash,"orig_height");
	cur_width = event->width;
	cur_height = event->height;

	x_ratio = (float)cur_width/(float)orig_width;
	y_ratio = (float)cur_height/(float)orig_height;
	ratio = x_ratio > y_ratio ? y_ratio:x_ratio;
	w_constricted = x_ratio > y_ratio ? FALSE:TRUE;
	
	/*printf("dash_config_event\n");*/
	g_signal_handlers_block_by_func(G_OBJECT(widget),(gpointer)dash_configure_event,NULL);
	children = GTK_FIXED(dash)->children;
	for (i=0;i<g_list_length(children);i++)
	{
		child = g_list_nth_data(children,i);
		gauge = child->widget;
		child_x = (GINT)OBJ_GET(gauge,"orig_x_offset");
		child_y = (GINT)OBJ_GET(gauge,"orig_y_offset");
		child_w = (GINT)OBJ_GET(gauge,"orig_width");
		child_h = (GINT)OBJ_GET(gauge,"orig_height");
		if (w_constricted)
			gtk_fixed_move(GTK_FIXED(dash),gauge,ratio*child_x,ratio*child_y+(((y_ratio-x_ratio)*orig_height)/2));
		else
			gtk_fixed_move(GTK_FIXED(dash),gauge,ratio*child_x-(((y_ratio-x_ratio)*orig_width)/2),ratio*child_y);
		gtk_widget_set_size_request(gauge,child_w*ratio,child_h*ratio);
	}
	dash_shape_combine(dash,FALSE);
	if (!timer_active)
	{
		gdk_threads_add_timeout(4000,hide_dash_resizers,dash);
		timer_active = TRUE;
	}

	g_signal_handlers_unblock_by_func(G_OBJECT(widget),(gpointer)dash_configure_event,NULL);
	return FALSE;
}


G_MODULE_EXPORT void load_elements(GtkWidget *dash, xmlNode *a_node)
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

G_MODULE_EXPORT void load_geometry(GtkWidget *dash, xmlNode *node)
{
	GdkGeometry hints;
	xmlNode *cur_node = NULL;
	gint width = 0;
	gint height = 0;
	if (!node->children)
	{
		printf(_("ERROR, load_geometry, xml node is empty!!\n"));
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

	gtk_window_set_geometry_hints(GTK_WINDOW(gtk_widget_get_toplevel(dash)),NULL,&hints,GDK_HINT_MIN_SIZE);

}

G_MODULE_EXPORT void load_gauge(GtkWidget *dash, xmlNode *node)
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
		printf(_("ERROR, load_gauge, xml node is empty!!\n"));
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
		filename = get_file(g_build_path(PSEP,GAUGES_DATA_DIR,xml_name,NULL),NULL);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		gtk_widget_set_size_request(gauge,width,height);
		g_free(filename);
		g_free(OBJ_GET(gauge,"datasource"));
		OBJ_SET_FULL(gauge,"datasource",g_strdup(datasource),g_free);
		OBJ_SET(gauge,"orig_width",GINT_TO_POINTER(width));
		OBJ_SET(gauge,"orig_height",GINT_TO_POINTER(height));
		OBJ_SET(gauge,"orig_x_offset",GINT_TO_POINTER(x_offset));
		OBJ_SET(gauge,"orig_y_offset",GINT_TO_POINTER(y_offset));
		g_free(xml_name);
		g_free(datasource);
	}

}


G_MODULE_EXPORT void link_dash_datasources(GtkWidget *dash,gpointer data)
{
	Dash_Gauge *d_gauge = NULL;
	GtkFixedChild *child = NULL;
	GList *children = NULL;
	gint len = 0;
	gint i = 0;
	GData * rtv_obj = NULL;
	gchar * source = NULL;
	GHashTable *dash_hash = NULL;
	Rtv_Map *rtv_map = NULL;

	if(!GTK_IS_FIXED(dash))
		return;
	
	rtv_map = DATA_GET(global_data,"rtv_map");
	dash_hash = DATA_GET(global_data,"dash_hash");
	if (!dash_hash)
	{
		dash_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		DATA_SET_FULL(global_data,"dash_hash",dash_hash,(GDestroyNotify)g_hash_table_destroy);
	}

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
		if (!(rtv_obj))
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": link_dash_datasources\n\tBad things man!, object doesn't exist for %s\n",source));
			continue ;
		}
		d_gauge = g_new0(Dash_Gauge, 1);
		d_gauge->object = rtv_obj;
		d_gauge->source = source;
		d_gauge->gauge = child->widget;
		d_gauge->dash = dash;
		g_hash_table_insert(dash_hash,g_strdup_printf("dash_%i_gauge_%i",(GINT)data,i),(gpointer)d_gauge);
	}
}

G_MODULE_EXPORT void update_dash_gauge(gpointer key, gpointer value, gpointer user_data)
{
	Dash_Gauge *d_gauge = (Dash_Gauge *)value;
	static GMutex *rtv_mutex = NULL;;
	GArray *history;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GtkWidget *gauge = NULL;

	if (!rtv_mutex)
		rtv_mutex = DATA_GET(global_data,"rtv_mutex");
	
	gauge = d_gauge->gauge;

	history = (GArray *)DATA_GET(d_gauge->object,"history");
	if ((gint)history->len-1 <= 0)
		return;
	g_mutex_lock(rtv_mutex);
	current = g_array_index(history, gfloat, history->len-1);
	g_mutex_unlock(rtv_mutex);

	gdk_threads_enter();
	mtx_gauge_face_get_value(MTX_GAUGE_FACE(gauge),&previous);
	if ((current != previous) || 
			(DATA_GET(global_data,"forced_update")))
	{
		/*printf("updating gauge %s\n",(gchar *)key);*/
		/*printf("updating gauge %s\n",mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(gauge)));*/
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),current);
	}
	gdk_threads_leave();

}


G_MODULE_EXPORT void dash_shape_combine(GtkWidget *dash, gboolean hide_resizers)
{
	static GdkColormap *colormap = NULL;
	static GdkColor black;
	static GdkColor white;
	GtkFixedChild *child = NULL;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	gint xc = 0;
	gint yc = 0;
	gint radius = 0;
	guint i = 0;
	GList *children = NULL;
	GdkGC *gc1 = NULL;
	GdkBitmap *bitmap = NULL;
	GtkRequisition req;
	gint width = 0;
	gint height = 0;
	GMutex *dash_mutex = DATA_GET(global_data,"dash_mutex");

	if(!GTK_IS_WIDGET(dash))
		return;
	if(!GTK_IS_WINDOW(gtk_widget_get_toplevel(dash)))
		return;
	g_mutex_lock(dash_mutex);

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
	if ((gboolean)DATA_GET(global_data,"dash_fullscreen"))
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
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
		{
			gtk_widget_input_shape_combine_mask(gtk_widget_get_toplevel(dash),bitmap,0,0);
		}
#endif
		gtk_widget_shape_combine_mask(gtk_widget_get_toplevel(dash),bitmap,0,0);
	}
	else
	{
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
		{
			gdk_window_input_shape_combine_mask(dash->window,bitmap,0,0);
		}
#endif
		gdk_window_shape_combine_mask(dash->window,bitmap,0,0);
	}
	g_object_unref(colormap);
	g_object_unref(gc1);
	g_object_unref(bitmap);
	g_mutex_unlock(dash_mutex);
	return;
}

G_MODULE_EXPORT gboolean dash_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	GtkWidget *dash = GTK_BIN(widget)->child;
	if (!timer_active)
	{
		dash_shape_combine(dash,FALSE);
		gdk_threads_add_timeout(4000,hide_dash_resizers,dash);
		timer_active = TRUE;
	}
	return TRUE;
}

G_MODULE_EXPORT gboolean dash_key_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	if (event->type == GDK_KEY_RELEASE)
		return FALSE;

	switch (event->keyval)
	{
		case GDK_q:
		case GDK_Q:
			leave(NULL,NULL);
			return TRUE;
			break;
		case GDK_M:
		case GDK_m:
			toggle_main_visible();
			return TRUE;
			break;
		case GDK_R:
		case GDK_r:
			toggle_rtt_visible();
			return TRUE;
			break;
		case GDK_S:
		case GDK_s:
			toggle_status_visible();
			return TRUE;
			break;
		case GDK_f:
		case GDK_F:
			toggle_dash_fullscreen(widget,NULL);
			return TRUE;
		case GDK_T:
		case GDK_t:
			dash_toggle_attribute(widget,TATTLETALE);
			return TRUE;
		case GDK_A:
		case GDK_a:
			dash_toggle_attribute(widget,ANTIALIAS);
			return TRUE;
			break;
	}
	return FALSE;
}


G_MODULE_EXPORT void toggle_status_visible(void)
{
	gint x = 0;
	gint y = 0;
	GtkWidget *tmpwidget = lookup_widget("status_window");
	if (!GTK_IS_WIDGET(tmpwidget))
		return;
	if (GTK_WIDGET_VISIBLE(tmpwidget))
	{
		gtk_widget_hide_all (tmpwidget);
		DATA_SET(global_data,"status_visible",GINT_TO_POINTER(FALSE));
	}
	else
	{
		x = (GINT)DATA_GET(global_data,"status_x_origin");
		y = (GINT)DATA_GET(global_data,"status_y_origin");
		gtk_widget_show_all(tmpwidget);
		gtk_window_move(GTK_WINDOW(tmpwidget),x,y);
		DATA_SET(global_data,"status_visible",GINT_TO_POINTER(TRUE));
	}
}


G_MODULE_EXPORT void toggle_rtt_visible(void)
{
	gint x = 0;
	gint y = 0;
	GtkWidget *tmpwidget = lookup_widget("rtt_window");
	if (!GTK_IS_WIDGET(tmpwidget))
		return;
	if (GTK_WIDGET_VISIBLE(tmpwidget))
	{
		gtk_widget_hide_all (tmpwidget);
		DATA_SET(global_data,"rtt_visible",GINT_TO_POINTER(FALSE));
	}
	else
	{
		x = (GINT)DATA_GET(global_data,"rtt_x_origin");
		y = (GINT)DATA_GET(global_data,"rtt_y_origin");
		gtk_widget_show_all(tmpwidget);
		gtk_window_move(GTK_WINDOW(tmpwidget),x,y);
		DATA_SET(global_data,"rtt_visible",GINT_TO_POINTER(TRUE));
	}
}

G_MODULE_EXPORT void toggle_main_visible(void)
{
	gint x = 0;
	gint y = 0;
	GtkWidget *tmpwidget = lookup_widget("main_window");
	if (!GTK_IS_WIDGET(tmpwidget))
		return;
	if (GTK_WIDGET_VISIBLE(tmpwidget))
	{
		gtk_widget_hide_all (tmpwidget);
		DATA_SET(global_data,"main_visible",GINT_TO_POINTER(FALSE));
	}
	else
	{
		x = (GINT)DATA_GET(global_data,"main_x_origin");
		y = (GINT)DATA_GET(global_data,"main_y_origin");
		gtk_widget_show_all(tmpwidget);
		gtk_window_move(GTK_WINDOW(tmpwidget),x,y);
		DATA_SET(global_data,"main_visible",GINT_TO_POINTER(TRUE));
	}
}

G_MODULE_EXPORT void dash_toggle_attribute(GtkWidget *widget,MtxGenAttr attr)
{
	GList *children = NULL;
	guint i = 0;
	gboolean state = FALSE;
	GtkFixedChild *child = NULL;
	GtkWidget * dash  = NULL;
	GtkWidget * gauge  = NULL;
	gchar * text_attr = NULL;

	text_attr = g_strdup_printf("%i",attr);
	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
	{
		printf(_("dashboard widget is null cannot set attribute(s)!\n"));
		return;
	}
	children = GTK_FIXED(dash)->children;
	if ((GBOOLEAN)OBJ_GET(dash,text_attr))
		state = FALSE;
	else
		state = TRUE;
	OBJ_SET(dash,text_attr,GINT_TO_POINTER(state));
	g_free(text_attr);
	for (i=0;i<g_list_length(children);i++)
	{
		child = g_list_nth_data(children,i);
		gauge = child->widget;
		mtx_gauge_face_set_attribute(MTX_GAUGE_FACE(gauge),attr,(gfloat)state);
	}

}


G_MODULE_EXPORT gboolean dash_lookup_attribute(GtkWidget *widget, MtxGenAttr attr)
{
	gchar * text_attr = NULL;
	GtkWidget * dash  = NULL;
	GList *children = NULL;
	GtkFixedChild *child = NULL;
	GtkWidget * gauge  = NULL;
	gfloat tmpf = 0.0;
	gint i = 0;
	gint t_count = 0;
	gint f_count = 0;

	text_attr = g_strdup_printf("%i",attr);
	dash = OBJ_GET(widget,"dash");
	g_free(text_attr);
	children = GTK_FIXED(dash)->children;
	for (i=0;i<g_list_length(children);i++)
	{
		child = g_list_nth_data(children,i);
		gauge = child->widget;
		mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge),attr,&tmpf);
		if ((gboolean)tmpf)
			t_count++;
		else
			f_count++;
	}
	if (t_count > f_count)
		return TRUE;
	else
		return FALSE;
}


G_MODULE_EXPORT gboolean dash_popup_menu_handler(GtkWidget *widget, gpointer data)
{
	dash_context_popup(widget, NULL);
	return TRUE;
}


G_MODULE_EXPORT void dash_context_popup(GtkWidget *widget, GdkEventButton *event)
{
	GtkWidget *menu = NULL;
	GtkWidget *item = NULL;
	GtkWidget *d_item = NULL;
	GtkWidget *n_item = NULL;
	gint button = 0;
	gint event_time = 0;
	GtkWidget *dash = GTK_BIN(widget)->child;

	menu = gtk_menu_new();

	/* Create Menu here */
	d_item = gtk_radio_menu_item_new_with_label(NULL,"Daytime Mode");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(d_item),get_dash_daytime_mode(gtk_widget_get_toplevel(widget)));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),d_item);
	n_item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(d_item),"Nitetime Mode");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(n_item),!get_dash_daytime_mode(gtk_widget_get_toplevel(widget)));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),n_item);
	g_signal_connect(G_OBJECT(d_item),"toggled",
		       	G_CALLBACK(set_dash_time_mode),(gpointer)gtk_widget_get_toplevel(widget));
	
	item = gtk_check_menu_item_new_with_label("Show Tattletales");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),dash_lookup_attribute(gtk_widget_get_toplevel(widget),TATTLETALE));
	g_signal_connect(G_OBJECT(item),"toggled",
		       	G_CALLBACK(toggle_dash_tattletales),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_menu_item_new_with_label("Reset Tattletales");
	gtk_widget_set_sensitive(item,dash_lookup_attribute(gtk_widget_get_toplevel(widget),TATTLETALE));
	g_signal_connect(G_OBJECT(item),"activate",
		       	G_CALLBACK(reset_dash_tattletales),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_check_menu_item_new_with_label("Antialiasing");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),dash_lookup_attribute(gtk_widget_get_toplevel(widget),ANTIALIAS));
	g_signal_connect(G_OBJECT(item),"toggled",
		       	G_CALLBACK(toggle_dash_antialias),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_check_menu_item_new_with_label("Fullscreen");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),(gboolean)DATA_GET(global_data,"dash_fullscreen"));
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
		       	G_CALLBACK(toggle_dash_fullscreen),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_check_menu_item_new_with_label("Stay on Top");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),(gboolean)OBJ_GET(dash,"dash_on_top"));
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
		       	G_CALLBACK(toggle_dash_on_top),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_check_menu_item_new_with_label("Hide MTX Gui");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),!(gboolean)DATA_GET(global_data,"gui_visible"));
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
		       	G_CALLBACK(toggle_gui_visible),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_menu_item_new_with_label("Close...");
	g_signal_connect(G_OBJECT(item),"activate",
		       	G_CALLBACK(close_dash),OBJ_GET(widget,"index"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	if (event)
	{
		button = event->button;
		event_time = event->time;
	}
	else
	{
		button = 0;
		event_time = gtk_get_current_event_time ();
	}

	gtk_menu_attach_to_widget (GTK_MENU (menu), widget, NULL);
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 
			button, event_time);
	gtk_widget_show_all(menu);
}


G_MODULE_EXPORT gboolean close_dash(GtkWidget *widget, gpointer data)
{
	gint index = 0;
	gchar * tmpbuf = NULL;
	GtkWidget *cbutton = NULL;

	if (!(gboolean)DATA_GET(global_data,"gui_visible"))
		toggle_gui_visible(NULL,NULL);
        DATA_SET(global_data,"dash_fullscreen",GINT_TO_POINTER(FALSE));
	index = (GINT)data;
	tmpbuf = g_strdup_printf("dash%i_cbutton",index);
	cbutton = lookup_widget(tmpbuf);
	if (GTK_IS_BUTTON(cbutton))
		g_signal_emit_by_name(cbutton,"clicked");
	g_free(tmpbuf);
	return TRUE;
}


G_MODULE_EXPORT gboolean toggle_dash_tattletales(GtkWidget *menuitem, gpointer data)
{
	GtkWidget *widget = (GtkWidget *)data;
	dash_toggle_attribute(gtk_widget_get_toplevel(widget),TATTLETALE);
	return TRUE;
}


G_MODULE_EXPORT gboolean set_dash_time_mode(GtkWidget *menuitem, gpointer data)
{
	gboolean value;
	GtkWidget *widget = (GtkWidget *)data;
	g_object_get(menuitem,"active",&value, NULL);
	set_dash_daytime_mode(widget,value);
	return TRUE;
}


G_MODULE_EXPORT gboolean get_dash_daytime_mode(GtkWidget *widget)
{
	GtkWidget * dash  = NULL;
	GList *children = NULL;
	GtkFixedChild *child = NULL;
	GtkWidget * gauge  = NULL;
	gint i = 0;
	gint t_count = 0;
	gint f_count = 0;

	dash = OBJ_GET(widget,"dash");
	children = GTK_FIXED(dash)->children;
	for (i=0;i<g_list_length(children);i++)
	{
		child = g_list_nth_data(children,i);
		gauge = child->widget;
		if(mtx_gauge_face_get_daytime_mode(MTX_GAUGE_FACE(gauge)))
			t_count++;
		else
			f_count++;
	}
	if (t_count > f_count)
		return TRUE;
	else
		return FALSE;
}


G_MODULE_EXPORT void set_dash_daytime_mode(GtkWidget *widget, gboolean state)
{
	GtkWidget * dash  = NULL;
	GList *children = NULL;
	GtkFixedChild *child = NULL;
	GtkWidget * gauge  = NULL;
	gint i = 0;

	dash = OBJ_GET(widget,"dash");
	children = GTK_FIXED(dash)->children;
	for (i=0;i<g_list_length(children);i++)
	{
		child = g_list_nth_data(children,i);
		gauge = child->widget;
		mtx_gauge_face_set_daytime_mode(MTX_GAUGE_FACE(gauge),state);
	}
}


G_MODULE_EXPORT gboolean reset_dash_tattletales(GtkWidget *menuitem, gpointer data)
{
	GList *children = NULL;
	guint i = 0;
	GtkFixedChild *child = NULL;
	GtkWidget * widget  = NULL;
	GtkWidget * dash  = NULL;
	GtkWidget * gauge  = NULL;

	widget = gtk_widget_get_toplevel(GTK_WIDGET(data));
	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
	{
		printf(_("dashboard widget is null cannot reset tattletale!\n"));
		return FALSE;
	}
	children = GTK_FIXED(dash)->children;
	for (i=0;i<g_list_length(children);i++)
	{
		child = g_list_nth_data(children,i);
		gauge = child->widget;
		mtx_gauge_face_clear_peak(MTX_GAUGE_FACE(gauge));
	}
	return TRUE;
}


G_MODULE_EXPORT gboolean toggle_dash_antialias(GtkWidget *menuitem, gpointer data)
{
	GtkWidget *widget = (GtkWidget *)data;
	dash_toggle_attribute(gtk_widget_get_toplevel(widget),ANTIALIAS);
	return TRUE;
}



G_MODULE_EXPORT gboolean dash_button_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	/*printf("button event\n"); */
	gint edge = -1;

	GtkWidget *dash = GTK_BIN(widget)->child;
	if (!timer_active)
	{
		dash_shape_combine(dash,FALSE);
		gdk_threads_add_timeout(4000,hide_dash_resizers,dash);
		timer_active = TRUE;
	}

	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
	{
		dash_context_popup(widget,event);
		return TRUE;
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
			g_signal_handlers_block_by_func(G_OBJECT(gtk_widget_get_toplevel(widget)),(gpointer)dash_configure_event,NULL);
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
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 2))
	{
		toggle_dash_fullscreen(widget,NULL);
		return TRUE;
	}
	return FALSE;
}


G_MODULE_EXPORT void initialize_dashboards_pf(void)
{
	GtkWidget * label = NULL;
	gboolean retval = FALSE;
	gchar * tmpbuf = NULL;
	gchar * tmpstr = NULL;
	gboolean nodash1 = TRUE;
	gboolean nodash2 = TRUE;
	CmdLineArgs *args = DATA_GET(global_data,"args");

	gdk_threads_enter();
	label = lookup_widget("dash_1_label");
	if (DATA_GET(global_data,"dash_1_name") != NULL)
		tmpbuf = (gchar *)DATA_GET(global_data,"dash_1_name");
	if ((GTK_IS_LABEL(label)) && (tmpbuf != NULL) && (strlen(tmpbuf) != 0))
	{
		gtk_widget_set_sensitive(lookup_widget("dash1_cbutton"),TRUE);
		tmpstr = g_filename_to_utf8(tmpbuf,-1,NULL,NULL,NULL);
		gtk_label_set_text(GTK_LABEL(label),tmpstr);
		g_free(tmpstr);
		load_dashboard(g_strdup(tmpbuf),GINT_TO_POINTER(1));
		tmpbuf = NULL;
		nodash1 = FALSE;
	}

	label = lookup_widget("dash_2_label");
	if (DATA_GET(global_data,"dash_2_name") != NULL)
		tmpbuf = (gchar *)DATA_GET(global_data,"dash_2_name");
	if ((GTK_IS_LABEL(label)) && (tmpbuf != NULL) && (strlen(tmpbuf) != 0))
	{
		gtk_widget_set_sensitive(lookup_widget("dash2_cbutton"),TRUE);
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
		gtk_widget_set_sensitive(lookup_widget("dash1_cbutton"),TRUE);
		gdk_threads_leave();
		return;
		if (!retval)
		{
			CmdLineArgs *args = DATA_GET(global_data,"args");
			args->be_quiet = TRUE;
			g_signal_emit_by_name(lookup_widget("main_window"),"destroy_event");
		}
	}
	gdk_threads_leave();
	return;
}


G_MODULE_EXPORT gboolean present_dash_filechooser(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GtkWidget *label = NULL;
	GHashTable *dash_hash = DATA_GET(global_data,"dash_hash");

	if (!DATA_GET(global_data,"interrogated"))
		return FALSE;

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("Dashboards");
	fileio->parent = lookup_widget("main_window");
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Select Dashboard to Open");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->shortcut_folders = g_strdup("Dashboards");


	filename = choose_file(fileio);
	free_mtxfileio(fileio);
	if (filename)
	{
		if (dash_hash)
			g_hash_table_foreach_remove(dash_hash,remove_dashcluster,data);
		if (GTK_IS_WIDGET(widget))
		{
			label = OBJ_GET(widget,"label");
			if (GTK_IS_LABEL(label))
				gtk_label_set_text(GTK_LABEL(label),g_filename_to_utf8(filename,-1,NULL,NULL,NULL));
			if ((GINT)data == 1)
				gtk_widget_set_sensitive(lookup_widget("dash1_cbutton"),TRUE);
			if ((GINT)data == 2)
				gtk_widget_set_sensitive(lookup_widget("dash2_cbutton"),TRUE);
		}
		load_dashboard(filename,data);
		return TRUE;
	}
	else
	{
		if ((GINT)data == 1)
			gtk_widget_set_sensitive(lookup_widget("dash1_cbutton"),FALSE);
		if ((GINT)data == 2)
			gtk_widget_set_sensitive(lookup_widget("dash2_cbutton"),FALSE);
	}
	return FALSE;
}



G_MODULE_EXPORT gboolean remove_dashboard(GtkWidget *widget, gpointer data)
{
	GtkWidget *label = NULL;
	GMutex *dash_mutex = DATA_GET(global_data,"dash_mutex");
	GHashTable *dash_hash = DATA_GET(global_data,"dash_hash");

	g_mutex_lock(dash_mutex);
	label = OBJ_GET(widget,"label");
	if (GTK_IS_WIDGET(label))
	{
		gtk_label_set_text(GTK_LABEL(label),"Choose a Dashboard File");
		if ((GINT)data == 1)
		{
			DATA_SET(global_data,"dash_1_name",NULL);
			gtk_widget_set_sensitive(lookup_widget("dash1_cbutton"),FALSE);
		}
		if ((GINT)data == 2)
		{
			DATA_SET(global_data,"dash_2_name",NULL);
			gtk_widget_set_sensitive(lookup_widget("dash2_cbutton"),FALSE);
		}
	}
	if (dash_hash)
		g_hash_table_foreach_remove(dash_hash,remove_dashcluster,data);
	g_mutex_unlock(dash_mutex);
	return TRUE;
}

G_MODULE_EXPORT gboolean remove_dashcluster(gpointer key, gpointer value, gpointer user_data)
{
	gchar *tmpbuf = NULL;
	Dash_Gauge *d_gauge = NULL;

	tmpbuf = g_strdup_printf("dash_%i",(GINT)user_data);
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

G_MODULE_EXPORT gboolean dummy(GtkWidget *widget,gpointer data)
{
	return TRUE;
}


G_MODULE_EXPORT void create_gauge(GtkWidget *widget)
{
	GtkWidget * gauge = NULL;
	gchar * xml_name = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gint table_num = -1;
	GList **tab_gauges = NULL;

	tab_gauges = DATA_GET(global_data,"tab_gauges");
	gauge = mtx_gauge_face_new();
	gtk_container_add(GTK_CONTAINER(widget),gauge);
	xml_name = OBJ_GET(widget,"gaugexml");
	if (xml_name)
		filename = get_file(g_build_path(PSEP,GAUGES_DATA_DIR,xml_name,NULL),NULL);
	if (filename)
	{
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		g_free(filename);
	}
	OBJ_SET(gauge,"datasource",OBJ_GET(widget,"datasource"));
	tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
	table_num = (gint)g_ascii_strtod(tmpbuf,NULL);
	tab_gauges[table_num] = g_list_prepend(tab_gauges[table_num],gauge);
}

G_MODULE_EXPORT void update_tab_gauges(void)
{
	GtkWidget *gauge = NULL;
	gchar * source = NULL;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	guint i = 0;
	GList *list = NULL;
	GList **tab_gauges = NULL;

	tab_gauges = DATA_GET(global_data,"tab_gauges");
	
	if ((!tab_gauges) || ((gint)DATA_GET(global_data,"active_table") < 0))
		return;
	list = g_list_first(tab_gauges[(gint)DATA_GET(global_data,"active_table")]);
	for (i=0;i<g_list_length(list);i++)
	{
		gauge = g_list_nth_data(list,i);
#if GTK_MINOR_VERSION >= 18
		if (gtk_widget_get_visible(gauge))
#else
		if (GTK_WIDGET_VISIBLE(gauge))
#endif
		{
#if GTK_MINOR_VERSION >= 18
			if (gtk_widget_get_state(gauge) != GTK_STATE_INSENSITIVE)
#else
			if (GTK_WIDGET_STATE(gauge) != GTK_STATE_INSENSITIVE) 
#endif
			{
				source = OBJ_GET(gauge,"datasource");
				/*printf("gauge is visible/sensitive, source %s\n",source);*/
				if (source)
				{
					lookup_current_value(source,&current);
					mtx_gauge_face_get_value(MTX_GAUGE_FACE(gauge),&previous);
					if ((current != previous) || 
							(DATA_GET(global_data,"forced_update")))
						mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),current);
				}
			}
			/*
			else
				printf("insensitive\n");
				*/
		}
		/*
		else
			printf("not visible\n");
			*/
	}
	/*
	printf("done updating gauges\n");
	*/
}


G_MODULE_EXPORT gboolean hide_dash_resizers(gpointer data)
{
	if (GTK_IS_WIDGET(data))
		dash_shape_combine(data,TRUE);
	timer_active = FALSE;
	return FALSE;
}

G_MODULE_EXPORT gboolean focus_event(GtkWidget *widget, gpointer data)
{
	if (moving)
	{
		g_signal_handlers_unblock_by_func(G_OBJECT(gtk_widget_get_toplevel(widget)),(gpointer)dash_configure_event,NULL);
		moving = FALSE;
	}
	if (resizing)
	{
		resizing = FALSE;
		gtk_widget_queue_draw(widget);
	}
	return FALSE;

}


G_MODULE_EXPORT void toggle_dash_fullscreen(GtkWidget *widget, gpointer data)
{
	GtkWidget *dash = OBJ_GET(widget,"dash");

	if ((gboolean)DATA_GET(global_data,"dash_fullscreen"))
	{
        	DATA_SET(global_data,"dash_fullscreen",GINT_TO_POINTER(FALSE));
		gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);
		gtk_window_unfullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
		if ((gboolean)OBJ_GET(dash,"dash_on_top"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),GTK_WINDOW(lookup_widget("main_window")));
	}
	else
	{
        	DATA_SET(global_data,"dash_fullscreen",GINT_TO_POINTER(TRUE));
		if ((gboolean)OBJ_GET(dash,"dash_on_top"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);
		gtk_window_fullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	}
}


G_MODULE_EXPORT void toggle_dash_on_top(GtkWidget *widget, gpointer data)
{
	GtkWidget *dash = GTK_BIN(widget)->child;

	if ((gboolean)OBJ_GET(dash,"dash_on_top"))
	{
        	OBJ_SET(dash,"dash_on_top",GINT_TO_POINTER(FALSE));
		gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);
	}
	else
	{
		if (!(gboolean)DATA_GET(global_data,"dash_fullscreen"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),GTK_WINDOW(lookup_widget("main_window")));
        	OBJ_SET(dash,"dash_on_top",GINT_TO_POINTER(TRUE));
	}
}


G_MODULE_EXPORT void toggle_gui_visible(GtkWidget *widget, gpointer data)
{
	/* IF visible, hide them */
	if ((gboolean)DATA_GET(global_data,"gui_visible"))
	{
		if (DATA_GET(global_data,"main_visible"))
			toggle_main_visible();
		if (DATA_GET(global_data,"status_visible"))
			toggle_status_visible();
		if (DATA_GET(global_data,"rtt_visible"))
			toggle_rtt_visible();
		DATA_SET(global_data,"gui_visible",GINT_TO_POINTER(FALSE));
	}
	else
	{
		if (!DATA_GET(global_data,"main_visible"))
			toggle_main_visible();
		if (!DATA_GET(global_data,"status_visible"))
			toggle_status_visible();
		if (!DATA_GET(global_data,"rtt_visible"))
			toggle_rtt_visible();
		DATA_SET(global_data,"gui_visible",GINT_TO_POINTER(TRUE));
	}
			
}
