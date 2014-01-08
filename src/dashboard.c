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
  \file src/dashboard.c
  \ingroup CoreMtx
  \brief Handles dashboard management.  Currently Mtx only allows two separate
  dashclusters active at any one time.  This code needs refactoring/improvement
  \author David Andruczyk
  */

#include <args.h>
#include <xmlbase.h>
#include <dashboard.h>
#include <defines.h>
#include <debugging.h>
#include <getfiles.h>
#include <gdk/gdkkeysyms.h>
#include <gui_handlers.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <stdio.h>
#include <string.h>
#include <widgetmgmt.h>

extern gconstpointer *global_data;

/*!
  \brief load_dashboard() loads the specified dashboard configuration file
  and initializes the dash.
  \param filename is the pointer to the file we should load
  \param index is the dash ID (1 or 2)
  \returns pointer to a new dashboard container widget
  */
G_MODULE_EXPORT GtkWidget * load_dashboard(const gchar *filename, gint index)
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
	
	ENTER();

	if (!DATA_GET(global_data,"interrogated"))
		return NULL;
	if (filename == NULL)
		return NULL;

	LIBXML_TEST_VERSION

		/*parse the file and get the DOM */
		doc = xmlReadFile(filename, NULL, 0);
	if (doc == NULL)
	{
		printf(_("Error: could not parse dashboard XML file %s"),filename);
		return NULL;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),_("Dash Cluster"));
	gtk_window_set_decorated(GTK_WINDOW(window),FALSE);

	g_signal_connect (G_OBJECT (window), "configure_event",
			G_CALLBACK (dash_configure_event), NULL);
	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (dummy), NULL);
	g_signal_connect (G_OBJECT (window), "destroy_event",
			G_CALLBACK (dummy), NULL);
	ebox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(window),ebox);

	gtk_widget_add_events(GTK_WIDGET(ebox),
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			/*
			GDK_POINTER_MOTION_HINT_MASK |
			GDK_POINTER_MOTION_MASK |
			*/
			GDK_KEY_PRESS_MASK |
			GDK_LEAVE_NOTIFY_MASK
			);

//	g_signal_connect (G_OBJECT (ebox), "motion_notify_event",
//			G_CALLBACK (dash_motion_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "leave-notify-event",
			G_CALLBACK (enter_leave_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "button_release_event",
			G_CALLBACK (dash_button_event), NULL);
	g_signal_connect(G_OBJECT (ebox), "button_press_event",
			G_CALLBACK (dash_button_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "popup-menu",
			G_CALLBACK (dash_popup_menu_handler), NULL);
	g_signal_connect (G_OBJECT (window), "key_press_event",
			G_CALLBACK (dash_key_event), NULL);

	dash = gtk_fixed_new();
	gtk_widget_set_has_window(dash,TRUE);
	gtk_widget_modify_bg(GTK_WIDGET(dash),GTK_STATE_NORMAL,&black);
	OBJ_SET(window,"dash",dash);
	OBJ_SET(ebox,"dash",dash);
	gtk_container_add(GTK_CONTAINER(ebox),dash);

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	load_elements(dash,root_element);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	link_dash_datasources(dash,GINT_TO_POINTER(index));

	/* Store global info about this dash */
	prefix = g_strdup_printf("dash_%i",index);
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
	OBJ_SET(ebox,"index", GINT_TO_POINTER(index));

	width = (GINT)OBJ_GET(dash,"orig_width");
	height = (GINT)OBJ_GET(dash,"orig_height");
	/*printf("move/resize to %i,%i, %ix%i\n",x,y,width,height); */
	gtk_window_move(GTK_WINDOW(window), x,y);
	if (ratio)
		gtk_window_set_default_size(GTK_WINDOW(window), (GINT)(width*(*ratio)),(GINT)(height*(*ratio)));
	else
		gtk_window_set_default_size(GTK_WINDOW(window), width,height);
	gtk_widget_show_all(window);
	dash_shape_combine(dash,TRUE);
	return window;
}


/*!
  \brief dashboard configure event.  Handles the dashboard setup and render
  of the spots for each gauge and initiates the shape combine
  \param widget is the pointer to the dash drawing area
  \param event is the pointer to the GdkEventConfigure event structure
  \returns FALSE so other signals run
  */
G_MODULE_EXPORT gboolean dash_configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
	gint orig_width = 0;
	gint orig_height = 0;
	gint cur_width = 0;
	gint cur_height = 0;
	gfloat x_ratio = 0.0;
	gfloat y_ratio = 0.0;
	gboolean w_constricted = FALSE;
	GtkWidget *gauge = NULL;
	GList *children = NULL;
	guint i = 0;
	gint child_w = 0;
	gint child_h = 0;
	gint child_x = 0;
	gint child_y = 0;
	gfloat ratio = 0.0;
	GtkWidget * dash  = NULL;
	
	ENTER();

	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
		return FALSE;

	if (OBJ_GET(dash,"moving"))
		return FALSE;

	orig_width = (GINT) OBJ_GET(dash,"orig_width");
	orig_height = (GINT) OBJ_GET(dash,"orig_height");
	cur_width = event->width;
	cur_height = event->height;

	x_ratio = (float)cur_width/(float)orig_width;
	y_ratio = (float)cur_height/(float)orig_height;
	ratio = x_ratio > y_ratio ? y_ratio:x_ratio;
	w_constricted = x_ratio > y_ratio ? FALSE:TRUE;

	g_signal_handlers_block_by_func(G_OBJECT(widget),(gpointer)dash_configure_event,NULL);
	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
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

	g_signal_handlers_unblock_by_func(G_OBJECT(widget),(gpointer)dash_configure_event,NULL);
	return FALSE;
}


/*!
  \brief XML processing function to load the elements for each dashboard
  \param dash is the pointer to dashboard window
  \param a_node is the pointer to XML node
  */
G_MODULE_EXPORT void load_elements(GtkWidget *dash, xmlNode *a_node)
{
	xmlNode *cur_node = NULL;
	
	ENTER();

	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"dash_geometry") == 0)
				load_geometry(dash,cur_node);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"gauge") == 0)
				load_gauge(dash,cur_node);
		}
		load_elements(dash,cur_node->children);
	}
}


/*!
  \brief XML processing function to load the geometry data for the dashboard
  \param dash is the pointer to dashboard widget
  \param node is the pointer to XML node
  */
G_MODULE_EXPORT void load_geometry(GtkWidget *dash, xmlNode *node)
{
	GdkGeometry hints;
	xmlNode *cur_node = NULL;
	gint width = 0;
	gint height = 0;
	
	ENTER();
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
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"width") == 0)
				generic_xml_gint_import(cur_node,&width);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"height") == 0)
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


/*!
  \brief XML processing function to load the gauge data for the dashboard
  \param dash is the pointer to dashboard widget
  \param node is the pointer to XML node
  */
G_MODULE_EXPORT void load_gauge(GtkWidget *dash, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	GtkWidget *gauge = NULL;
	GList *children = NULL;
	gchar * filename = NULL;
	gint width = 0;
	gint height = 0;
	gint x_offset = 0;
	gint y_offset = 0;
	gchar *xml_name = NULL;
	gchar *datasource = NULL;
	gchar *pathstub = NULL;
	
	ENTER();

	if (!node->children)
	{
		printf(_("ERROR, load_gauge, xml node is empty!!\n"));
		return;
	}
	cur_node = node->children;
	while (cur_node->next) { if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"width") == 0)
				generic_xml_gint_import(cur_node,&width);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"height") == 0)
				generic_xml_gint_import(cur_node,&height);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"x_offset") == 0)
				generic_xml_gint_import(cur_node,&x_offset);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"y_offset") == 0)
				generic_xml_gint_import(cur_node,&y_offset);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"gauge_xml_name") == 0)
				generic_xml_gchar_import(cur_node,&xml_name);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"datasource") == 0)
				generic_xml_gchar_import(cur_node,&datasource);
		}
		cur_node = cur_node->next;

	}
	if (xml_name && datasource)
	{
		gauge = mtx_gauge_face_new();
		gtk_fixed_put(GTK_FIXED(dash),gauge,x_offset,y_offset);
		children = OBJ_GET(dash,"children");
		children = g_list_prepend(children,gauge);
		OBJ_SET(dash,"children",children);
		xml_name = g_strdelimit(xml_name,"\\",'/');
		pathstub = g_build_filename(GAUGES_DATA_DIR,xml_name,NULL);
		filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,NULL);
		g_free(pathstub);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		gtk_widget_set_size_request(gauge,width,height);
		g_free(filename);
		OBJ_SET_FULL(gauge,"datasource",g_strdup(datasource),g_free);
		OBJ_SET(gauge,"orig_width",GINT_TO_POINTER(width));
		OBJ_SET(gauge,"orig_height",GINT_TO_POINTER(height));
		OBJ_SET(gauge,"orig_x_offset",GINT_TO_POINTER(x_offset));
		OBJ_SET(gauge,"orig_y_offset",GINT_TO_POINTER(y_offset));
		g_free(xml_name);
		g_free(datasource);
	}

}


/*!
  \brief Links the dashboard datasources defined in the XML to actual 
  datasources within megatunix itself (match is via name)
  \param dash is the pointer to dashboard widget
  \param data is unused
  */
G_MODULE_EXPORT void link_dash_datasources(GtkWidget *dash,gpointer data)
{
	Dash_Gauge *d_gauge = NULL;
	GList *children = NULL;
	GtkWidget *cwidget = NULL;
	gint len = 0;
	gint i = 0;
	GData * rtv_obj = NULL;
	gchar * source = NULL;
	GHashTable *dash_hash = NULL;
	Rtv_Map *rtv_map = NULL;
	
	ENTER();

	if(!GTK_IS_FIXED(dash))
		return;
	
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	dash_hash = (GHashTable *)DATA_GET(global_data,"dash_hash");
	if (!dash_hash)
	{
		dash_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		DATA_SET_FULL(global_data,"dash_hash",dash_hash,(GDestroyNotify)g_hash_table_destroy);
	}

	children = (GList *)OBJ_GET(dash,"children");
	len = g_list_length(children);

	for (i=0;i<len;i++)
	{
		cwidget = (GtkWidget *)g_list_nth_data(children,i);
		source = (gchar *)OBJ_GET(cwidget,"datasource");
		if (!source)
			continue;

		if (!rtv_map)
			return;
		if (!(rtv_map->rtv_hash))
			return;
		rtv_obj = (GData *)g_hash_table_lookup(rtv_map->rtv_hash,source);
		d_gauge = g_new0(Dash_Gauge, 1);
		if (!(rtv_obj))
			MTXDBG(CRITICAL,_("Bad things man!, object doesn't exist for %s\n"),source);
		else
			d_gauge->object = rtv_obj;
		d_gauge->source = g_strdup(source);
		d_gauge->gauge = cwidget;
		d_gauge->dash = dash;
		g_hash_table_insert(dash_hash,g_strdup_printf("dash_%i_gauge_%i",(GINT)data,i),(gpointer)d_gauge);
	}
}


/*!
  \brief Updates a dashboard gauge with a new value
  \param key is the gauge name
  \param value is the pointer to Dash_Gauge structure
  \param user_data is unused
  */
G_MODULE_EXPORT void update_dash_gauge(gpointer key, gpointer value, gpointer user_data)
{
	Dash_Gauge *d_gauge = (Dash_Gauge *)value;
	static GMutex *rtv_mutex = NULL;;
	GArray *history;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GtkWidget *gauge = NULL;
	
	ENTER();

	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");
	
	gauge = d_gauge->gauge;
	/* If no RTV object (i.e. not found), silently return */
	if (!d_gauge->object)
		return;

	history = (GArray *)DATA_GET(d_gauge->object,"history");
	if ((GINT)history->len-1 <= 0)
		return;
	g_mutex_lock(rtv_mutex);
	current = g_array_index(history, gfloat, history->len-1);
	g_mutex_unlock(rtv_mutex);

	mtx_gauge_face_get_value(MTX_GAUGE_FACE(gauge),&previous);
	if ((current != previous) || 
			(DATA_GET(global_data,"forced_update")))
	{
		/*printf("updating gauge %s\n",(gchar *)key);*/
		/*printf("updating gauge %s\n",mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(gauge)));*/
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),current);
	}

}


/*!
  \brief gives the dashboard that floating look without a bounding window/box
  \param dash is the pointer to dashboard widget
  \param hide_resizers is the flag to display or hide the dashboard resizers
  */
G_MODULE_EXPORT void dash_shape_combine(GtkWidget *dash, gboolean hide_resizers)
{
	GtkWidget *cwidget = NULL;
	cairo_t *cr = NULL;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	gint xc = 0;
	gint yc = 0;
	gint radius = 0;
	guint i = 0;
	GList *children = NULL;
	GdkBitmap *bitmap = NULL;
	GtkRequisition req;
	GtkAllocation alloc;
	gint width = 0;
	gint height = 0;
	GMutex *dash_mutex = (GMutex *)DATA_GET(global_data,"dash_mutex");
	GdkWindow *window = NULL;
	
	ENTER();

	if(!GTK_IS_WIDGET(dash))
		return;
	if(!GTK_IS_WINDOW(gtk_widget_get_toplevel(dash)))
		return;
	g_mutex_lock(dash_mutex);

	window = gtk_widget_get_window(dash);
	gtk_window_get_size(GTK_WINDOW(gtk_widget_get_toplevel(dash)),&width,&height);
	bitmap = gdk_pixmap_new(NULL,width,height,1);
	cr = gdk_cairo_create(bitmap);
	cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
	cairo_paint(cr);
	cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgb(cr, 1.0,1.0,1.0);
	if (hide_resizers == FALSE)
	{
		cairo_rectangle(cr,0,0,16,3);
		cairo_rectangle(cr,0,0,3,16);
		cairo_rectangle(cr,width-16,0,16,3);
		cairo_rectangle(cr,width-3,0,3,16);
		cairo_rectangle(cr,width-16,height-3,16,3);
		cairo_rectangle(cr,width-3,height-16,3,16);
		cairo_rectangle(cr,0,height-3,16,3);
		cairo_rectangle(cr,0,height-16,3,16);
		cairo_fill(cr);
	}

	if ((GBOOLEAN)DATA_GET(global_data,"dash_fullscreen"))
		cairo_rectangle(cr,0,0,width,height);

	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		cwidget = (GtkWidget *)g_list_nth_data(children,i);
		gtk_widget_get_allocation(cwidget,&alloc);
		x = alloc.x;
		y = alloc.y;
		gtk_widget_size_request(cwidget,&req);
		w = req.width;
		h = req.height;
		radius = MIN(w,h)/2;
		xc = x+w/2;
		yc = y+h/2;

		cairo_arc(cr,xc,yc,radius,0,2 * M_PI);
		cairo_fill(cr);
		cairo_stroke(cr);
	}
	cairo_destroy(cr);
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
			gdk_window_input_shape_combine_mask(window,bitmap,0,0);
		}
#endif
		gdk_window_shape_combine_mask(window,bitmap,0,0);
	}
	g_object_unref(bitmap);
	g_mutex_unlock(dash_mutex);
	return;
}


/*!
  \brief Dashboard motion event handler (not used yet)
  \param widget is unused
  \param event is a pointer to GdkEventMotion structure
  \param data is unused
  \return FALSE so other handlers run
  */
/*
G_MODULE_EXPORT gboolean dash_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	int x,y;
	GdkModifierType state;
	ENTER();
	if (event->is_hint)
	{
		gdk_window_get_pointer(event->window, &x, &y, &state);
		printf("motion hint, at %i,%i\n",x,y);
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
		printf("motion at %i,%i\n",x,y);
	}
	printf("dash motion event!\n");
	EXIT();
	return FALSE;
}
*/


/*!
  \brief Dashboard keyboard event handler that handles dashboard hotkeys
  \param widget is th pointer to dashboard widget
  \param event is th pointer to a GdkEventKey structure
  \param data is unused
  \return FALSE so other handlers run
  */
G_MODULE_EXPORT gboolean dash_key_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	gboolean retval = FALSE;
	ENTER();
	if (event->type == GDK_KEY_RELEASE)
	{
		EXIT();
		return FALSE;
	}

	switch (event->keyval)
	{
		case GDK_q:
		case GDK_Q:
			leave(NULL,NULL);
			retval = TRUE;
			break;
		case GDK_M:
		case GDK_m:
			toggle_main_visible();
			EXIT();
			retval = TRUE;
			break;
		case GDK_R:
		case GDK_r:
			toggle_rtt_visible();
			EXIT();
			retval = TRUE;
			break;
		case GDK_S:
		case GDK_s:
			toggle_status_visible();
			retval = TRUE;
			break;
		case GDK_f:
		case GDK_F:
			toggle_dash_fullscreen(widget,NULL);
			retval = TRUE;
		case GDK_T:
		case GDK_t:
			dash_toggle_attribute(widget,TATTLETALE);
			retval = TRUE;
		case GDK_A:
		case GDK_a:
			dash_toggle_attribute(widget,ANTIALIAS);
			retval = TRUE;
			break;
	}
	EXIT();
	return retval;
}


/*!
  \brief Makes the runtime status window appear/disappear
  */
G_MODULE_EXPORT void toggle_status_visible(void)
{
	GtkWidget *tmpwidget = lookup_widget("status_window");

	ENTER();
	if (!GTK_IS_WIDGET(tmpwidget))
	{
		EXIT();
		return;
	}
	if (gtk_widget_get_visible(tmpwidget))
	{
		gtk_widget_hide(tmpwidget);
		DATA_SET(global_data,"status_visible",GINT_TO_POINTER(FALSE));
	}
	else
	{
		gint x = (GINT)DATA_GET(global_data,"status_x_origin");
		gint y = (GINT)DATA_GET(global_data,"status_y_origin");
		gtk_widget_show_all(tmpwidget);
		gtk_window_move(GTK_WINDOW(tmpwidget),x,y);
		DATA_SET(global_data,"status_visible",GINT_TO_POINTER(TRUE));
	}
	EXIT();
	return;
}


/*!
  \brief Makes the runtime text window appear/disappear
  */
G_MODULE_EXPORT void toggle_rtt_visible(void)
{
	GtkWidget *tmpwidget = lookup_widget("rtt_window");

	ENTER();
	if (!GTK_IS_WIDGET(tmpwidget))
	{
		EXIT();
		return;
	}
	if (gtk_widget_get_visible(tmpwidget))
	{
		gtk_widget_hide(tmpwidget);
		DATA_SET(global_data,"rtt_visible",GINT_TO_POINTER(FALSE));
	}
	else
	{
		gint x = (GINT)DATA_GET(global_data,"rtt_x_origin");
		gint y = (GINT)DATA_GET(global_data,"rtt_y_origin");
		gtk_widget_show_all(tmpwidget);
		gtk_window_move(GTK_WINDOW(tmpwidget),x,y);
		DATA_SET(global_data,"rtt_visible",GINT_TO_POINTER(TRUE));
	}
	EXIT();
	return;
}


/*!
  \brief Makes the main window appear/disappear
  */
G_MODULE_EXPORT void toggle_main_visible(void)
{
	GtkWidget *tmpwidget = lookup_widget("main_window");
	ENTER();
	if (!GTK_IS_WIDGET(tmpwidget))
	{
		EXIT();
		return;
	}
	if (gtk_widget_get_visible(tmpwidget))
	{
		gtk_widget_hide (tmpwidget);
		DATA_SET(global_data,"main_visible",GINT_TO_POINTER(FALSE));
	}
	else
	{
		gtk_widget_show(tmpwidget);
		gint x = (GINT)DATA_GET(global_data,"main_x_origin");
		gint y = (GINT)DATA_GET(global_data,"main_y_origin");
		gtk_widget_show_all(tmpwidget);
		gtk_window_move(GTK_WINDOW(tmpwidget),x,y);
		DATA_SET(global_data,"main_visible",GINT_TO_POINTER(TRUE));
	}
	EXIT();
	return;
}


/*!
  \brief Turns on/off gauge attributes of a dash (tattletales, etc )
  \param widget is the pointer to dashboard widget 
  \param attr is the Enumeration for type of attribute
  */
G_MODULE_EXPORT void dash_toggle_attribute(GtkWidget *widget,MtxGenAttr attr)
{
	GList *children = NULL;
	guint i = 0;
	gboolean state = FALSE;
	GtkWidget * dash  = NULL;
	GtkWidget * gauge  = NULL;
	gchar * text_attr = NULL;

	ENTER();

	text_attr = g_strdup_printf("%i",attr);
	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
	{
		printf(_("dashboard widget is null cannot set attribute(s)!\n"));
		EXIT();
		return;
	}
	children = (GList *)OBJ_GET(dash,"children");
	if ((GBOOLEAN)OBJ_GET(dash,text_attr))
		state = FALSE;
	else
		state = TRUE;
	OBJ_SET(dash,text_attr,GINT_TO_POINTER(state));
	g_free(text_attr);
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		mtx_gauge_face_set_attribute(MTX_GAUGE_FACE(gauge),attr,(gfloat)state);
	}
	EXIT();
	return;
}


/*!
  \brief Queries the status of gauge attirbutes for a dashboard
  \param widget is the pointer to dashboard widget 
  \param attr is the Enumeration for type of attribute
  */
G_MODULE_EXPORT gboolean dash_lookup_attribute(GtkWidget *widget, MtxGenAttr attr)
{
	gchar * text_attr = NULL;
	GtkWidget * dash  = NULL;
	GList *children = NULL;
	GtkWidget * gauge  = NULL;
	gfloat tmpf = 0.0;
	guint i = 0;
	gint t_count = 0;
	gint f_count = 0;

	ENTER();

	text_attr = g_strdup_printf("%i",attr);
	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	g_free(text_attr);
	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge),attr,&tmpf);
		if ((GBOOLEAN)tmpf)
			t_count++;
		else
			f_count++;
	}
	EXIT();
	if (t_count > f_count)
		return TRUE;
	else
		return FALSE;
}


/*!
  \brief Pops up the menu for the dashboard when right clicked upon the dash
  \param widget is the pointer to dashboard window
  \returns TRUE to block any other handlers from running
  */
G_MODULE_EXPORT gboolean dash_popup_menu_handler(GtkWidget *widget)
{
	ENTER();
	dash_context_popup(widget, NULL);
	EXIT();
	return TRUE;
}


/*!
  \brief Pops up the menu for the dashboard when right clicked upon the dash
  \param widget is the pointer to dashboard window
  \param event is the pointer to a GdkEventButton structure
  */
G_MODULE_EXPORT void dash_context_popup(GtkWidget *widget, GdkEventButton *event)
{
	static GtkWidget *menu = NULL;
	GtkWidget *item = NULL;
	GtkWidget *d_item = NULL;
	GtkWidget *n_item = NULL;
	gint button = 0;
	gint event_time = 0;
	GtkWidget *dash = gtk_bin_get_child(GTK_BIN(widget));

	ENTER();

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
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),(GBOOLEAN)DATA_GET(global_data,"dash_fullscreen"));
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_dash_fullscreen),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_check_menu_item_new_with_label("Stay on Top");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),(GBOOLEAN)OBJ_GET(dash,"dash_on_top"));
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_dash_on_top),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	if ((GBOOLEAN)DATA_GET(global_data,"gui_visible"))
		item = gtk_check_menu_item_new_with_label("Hide All Windows");
	else
		item = gtk_check_menu_item_new_with_label("Show All Windows");
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_gui_visible),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	if ((GBOOLEAN)DATA_GET(global_data,"main_visible"))
		item = gtk_check_menu_item_new_with_label("Hide Main Gui");
	else
		item = gtk_check_menu_item_new_with_label("Show Main Gui");
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_main_visible),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	if ((GBOOLEAN)DATA_GET(global_data,"rtt_visible"))
		item = gtk_check_menu_item_new_with_label("Hide Runtime Text");
	else
		item = gtk_check_menu_item_new_with_label("Show Runtime Text");
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_rtt_visible),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	if ((GBOOLEAN)DATA_GET(global_data,"status_visible"))
		item = gtk_check_menu_item_new_with_label("Hide Runtime Status");
	else
		item = gtk_check_menu_item_new_with_label("Show Runtime Status");
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_status_visible),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	if ((GBOOLEAN)DATA_GET(global_data,"gui_visible"))
	{
		item = gtk_menu_item_new_with_label("Close Dash...");
		g_signal_connect(G_OBJECT(item),"activate",
				G_CALLBACK(close_dash),OBJ_GET(widget,"index"));
	}
	else
	{
		item = gtk_menu_item_new_with_label("Quit Megatunix");
		g_signal_connect(G_OBJECT(item),"activate",
				G_CALLBACK(leave),NULL);
	}
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
	EXIT();
	return;
}


/*!
  \brief Closes the dashboard
  \param widget is the pointer to dashboard  window
  \param data is the pointer to dashboard index
  \returns TRUE
  */
G_MODULE_EXPORT gboolean close_dash(GtkWidget *widget, gpointer data)
{
	gint index = 0;
	gchar * tmpbuf = NULL;
	GtkWidget *close_button = NULL;

	ENTER();

	/* IF gui isn't visible, make it visible */
	if (!(GBOOLEAN)DATA_GET(global_data,"gui_visible"))
		toggle_gui_visible(NULL,NULL);

	DATA_SET(global_data,"dash_fullscreen",GINT_TO_POINTER(FALSE));
	index = (GINT)data;
	tmpbuf = g_strdup_printf("dash_%i_close_button",index);
	close_button = lookup_widget(tmpbuf);
	if (GTK_IS_BUTTON(close_button))
		g_signal_emit_by_name(close_button,"clicked");
	g_free(tmpbuf);
	EXIT();
	return TRUE;
}


/*!
  \brief Enables or disables the dashboard tattletales...
  \param menuitem is the pointer to context menuitem
  \param data is the pointer to dashboard widget
  \returns TRUE
  */
G_MODULE_EXPORT gboolean toggle_dash_tattletales(GtkWidget *menuitem, gpointer data)
{
	GtkWidget *widget = (GtkWidget *)data;
	ENTER();
	dash_toggle_attribute(gtk_widget_get_toplevel(widget),TATTLETALE);
	EXIT();
	return TRUE;
}


/*!
  \brief Conext menu handler that Sets the dashboard to daytime or nitetime mode
  \param menuitem is the pointer to context menuitem
  \param data is the pointer to dashboard widget
  \returns TRUE
  */
G_MODULE_EXPORT gboolean set_dash_time_mode(GtkWidget *menuitem, gpointer data)
{
	gboolean value;
	GtkWidget *widget = (GtkWidget *)data;
	ENTER();
	g_object_get(menuitem,"active",&value, NULL);
	set_dash_daytime_mode(widget,value);
	EXIT();
	return TRUE;
}


/*!
  \brief Gets the dashboard to daytime or nitetime mode
  \param widget is the pointer to dashboard widget
  \returns TRUE if daytime, otherwise FALSE
  */
G_MODULE_EXPORT gboolean get_dash_daytime_mode(GtkWidget *widget)
{
	GtkWidget * dash  = NULL;
	GList *children = NULL;
	GtkWidget * gauge  = NULL;
	guint i = 0;
	gint t_count = 0;
	gint f_count = 0;

	ENTER();

	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		if(mtx_gauge_face_get_daytime_mode(MTX_GAUGE_FACE(gauge)))
			t_count++;
		else
			f_count++;
	}
	EXIT();
	if (t_count > f_count)
		return TRUE;
	else
		return FALSE;
}


/*!
  \brief Sets the dashboard to daytime or nitetime mode
  \param widget is the pointer to dashboard widget
  \param state is the flag whether we are daytime or nitetime
  */
G_MODULE_EXPORT void set_dash_daytime_mode(GtkWidget *widget, gboolean state)
{
	GtkWidget * dash  = NULL;
	GList *children = NULL;
	GtkWidget * gauge  = NULL;
	guint i = 0;

	ENTER();

	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	children = (GList *)OBJ_GET(dash, "children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		mtx_gauge_face_set_daytime_mode(MTX_GAUGE_FACE(gauge),state);
	}
	EXIT();
	return;
}


/*!
  \brief resets the dashboard tattletales for all gauges 
  \param menuitem is the pointer to the dash context menuitem
  \param data is unused
  \returns TRUE if successfull, FALSE otherwise
  */
G_MODULE_EXPORT gboolean reset_dash_tattletales(GtkWidget *menuitem, gpointer data)
{
	GList *children = NULL;
	guint i = 0;
	GtkWidget * widget  = NULL;
	GtkWidget * dash  = NULL;
	GtkWidget * gauge  = NULL;

	ENTER();

	widget = gtk_widget_get_toplevel(GTK_WIDGET(data));
	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
	{
		printf(_("dashboard widget is null cannot reset tattletale!\n"));
		EXIT();
		return FALSE;
	}
	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		mtx_gauge_face_clear_peak(MTX_GAUGE_FACE(gauge));
	}
	EXIT();
	return TRUE;
}


/*!
  \brief Toggles the dashboard antialiasing
  \param menuitem is the dash context menu item
  \param data is the pointer to dashboard widget
  \returns TRUE
  */
G_MODULE_EXPORT gboolean toggle_dash_antialias(GtkWidget *menuitem, gpointer data)
{
	GtkWidget *widget = (GtkWidget *)data;
	ENTER();
	dash_toggle_attribute(gtk_widget_get_toplevel(widget),ANTIALIAS);
	EXIT();
	return TRUE;
}



/*!
  \brief Dash mouse button event handler to handler move/resize
  \param widget is the pointer to dashboard widget
  \param event is the pointer to EventButton structure.
  \param data is unused
  \returns TRUE, if it handles something, otherwise FALSE
  */
G_MODULE_EXPORT gboolean dash_button_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkAllocation allocation;

	ENTER();

	gtk_widget_get_allocation(widget,&allocation);
	GtkWidget *dash = gtk_bin_get_child(GTK_BIN(widget));
	if (!OBJ_GET(dash,"resizers_visible"))
	{
		dash_shape_combine(dash,FALSE);
		OBJ_SET(dash,"resizers_visible",GINT_TO_POINTER(TRUE));
	}
	if ((event->type == GDK_BUTTON_RELEASE) && (event->button == 1))
	{
		OBJ_SET(dash,"moving",GINT_TO_POINTER(FALSE));
		OBJ_SET(dash,"resizing",GINT_TO_POINTER(FALSE));
		EXIT();
		return TRUE;
	}

	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
	{
		dash_context_popup(widget,event);
		EXIT();
		return TRUE;
	}
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 1))
	{
		gint edge = -1;
		/*printf("dash button event\n"); */
		if (event->x > (allocation.width-16))
		{
			/* Upper portion */
			if (event->y < 16)
				edge = GDK_WINDOW_EDGE_NORTH_EAST;
			/* Lower portion */
			else if (event->y > (allocation.height-16))
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
			else if (event->y > (allocation.height-16))
				edge = GDK_WINDOW_EDGE_SOUTH_WEST;
			else
				edge = -1;
		}
		else
			edge = -1;

		if ((edge == -1 ) && (GTK_IS_WINDOW(gtk_widget_get_parent(widget))))
		{
			/*printf("MOVE drag\n"); */
			OBJ_SET(dash,"moving",GINT_TO_POINTER(TRUE));
			gtk_window_begin_move_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
					event->button,
					event->x_root,
					event->y_root,
					event->time);
			EXIT();
			return TRUE;
		}
		else if (GTK_IS_WINDOW(gtk_widget_get_parent(widget)))
		{
			/*printf("RESIZE drag\n"); */
			OBJ_SET(dash,"resizing",GINT_TO_POINTER(TRUE));
			gtk_window_begin_resize_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
					(GdkWindowEdge)edge,
					event->button,
					event->x_root,
					event->y_root,
					event->time);
		}
	}
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 2))
	{
		toggle_dash_fullscreen(widget,NULL);
		EXIT();
		return TRUE;
	}
	EXIT();
	return FALSE;
}


/*!
  \brief Initialize dashboard post function. Loads dashboards as previously set
  from a previous mtx run
  */
G_MODULE_EXPORT void initialize_dashboards_pf(void)
{
	GtkWidget *widget = NULL;
	GtkWidget *label = NULL;
	GtkWidget *choice_button = NULL;
	gboolean retval = FALSE;
	const gchar * tmpbuf = NULL;
	gchar *filename = NULL;
	gboolean nodash1 = TRUE;
	gboolean nodash2 = TRUE;
	CmdLineArgs *args = (CmdLineArgs *)DATA_GET(global_data,"args");

	ENTER();

	if (args->dashboard)
	{
		choice_button = lookup_widget("dash_1_choice_button");
		if (GTK_IS_WIDGET(choice_button))
		{
			gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(choice_button),args->dashboard);
			gtk_widget_set_sensitive(lookup_widget("dash_1_close_button"),TRUE);
			filename = g_strdup(args->dashboard);
			widget = load_dashboard(args->dashboard,1);
			register_widget(filename,widget);
			g_free(filename);
			OBJ_SET_FULL(lookup_widget("dash_1_close_button"),"filename",g_strdup(args->dashboard),g_free);
			nodash1 = FALSE;
		}
		if ((GTK_IS_WIDGET(widget) && (args->dash_fullscreen)))
			toggle_dash_fullscreen(widget,NULL);
	}
	else
	{
		choice_button = lookup_widget("dash_1_choice_button");
		if (DATA_GET(global_data,"dash_1_name") != NULL)
			tmpbuf = (gchar *)DATA_GET(global_data,"dash_1_name");
		if ((GTK_IS_WIDGET(choice_button)) && (tmpbuf != NULL) && (strlen(tmpbuf) != 0))
		{
			gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(choice_button),tmpbuf);
			gtk_widget_set_sensitive(lookup_widget("dash_1_close_button"),TRUE);
			filename = g_strdup(tmpbuf);
			widget = load_dashboard(tmpbuf,1);
			register_widget(filename,widget);
			OBJ_SET_FULL(lookup_widget("dash_1_close_button"),"filename",g_strdup(filename),g_free);
			g_free(filename);
			tmpbuf = NULL;
			nodash1 = FALSE;
		}
		choice_button = lookup_widget("dash_2_choice_button");
		if (DATA_GET(global_data,"dash_2_name") != NULL)
			tmpbuf = (gchar *)DATA_GET(global_data,"dash_2_name");
		if ((GTK_IS_WIDGET(choice_button)) && (tmpbuf != NULL) && (strlen(tmpbuf) != 0))
		{
			gtk_widget_set_sensitive(lookup_widget("dash_2_close_button"),TRUE);
			gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(choice_button),tmpbuf);
			filename = g_strdup(tmpbuf);
			widget = load_dashboard(tmpbuf,2);
			register_widget(filename,widget);
			OBJ_SET_FULL(lookup_widget("dash_2_close_button"),"filename",g_strdup(filename),g_free);
			g_free(filename);
			tmpbuf = NULL;
			nodash2 = FALSE;
		}
	}
	/* Case to handle when no default dashboards are set, but the user
	 * choose to run with no main gui (thus can't quit or select a dash)
	 * So we force the dash chooser
	 */
	if ((nodash1) && (nodash2) && (args->hide_maingui))
	{
		error_msg("You've selected a mode with the gui hidden, but NO dashboards specified,  you need to specify a dash on the command line, or run megatunix ONCE and select a working dash and close cleanly before attempting this again");
		gtk_main_quit();
	}
	EXIT();
	return;
}


/*
	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("Dashboards");
	fileio->parent = lookup_widget("main_window");
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Select Dashboard to Open");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->shortcut_folders = g_strdup("Dashboards");
	*/


/*!
  \brief Removes a dashboard
  \param widget is the pointer to dash close button
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean remove_dashboard(GtkWidget *widget, gpointer data)
{
	GtkWidget *choice_button = NULL;
	GMutex *dash_mutex = (GMutex *)DATA_GET(global_data,"dash_mutex");
	GHashTable *dash_hash = (GHashTable *)DATA_GET(global_data,"dash_hash");

	ENTER();

	g_mutex_lock(dash_mutex);
	choice_button = (GtkWidget *)OBJ_GET(widget,"choice_button");
	if (GTK_IS_WIDGET(choice_button))
	{
		/* Resets to "None" */
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(choice_button),"None");
		if (OBJ_GET(choice_button,"last_folder"))
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(choice_button),(gchar *)OBJ_GET(choice_button,"last_folder"));
		else
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(choice_button),(gchar *)OBJ_GET(choice_button,"syspath"));
		if ((GINT)data == 1)
		{
			DATA_SET(global_data,"dash_1_name",NULL);
			gtk_widget_set_sensitive(lookup_widget("dash_1_close_button"),FALSE);
			deregister_widget((const gchar *)OBJ_GET(lookup_widget("dash_1_close_button"),"filename"));
		}
		if ((GINT)data == 2)
		{
			DATA_SET(global_data,"dash_2_name",NULL);
			gtk_widget_set_sensitive(lookup_widget("dash_2_close_button"),FALSE);
			deregister_widget((const gchar *)OBJ_GET(lookup_widget("dash_2_close_button"),"filename"));
		}
	}
	if (dash_hash)
		g_hash_table_foreach_remove(dash_hash,remove_dashcluster,data);
	g_mutex_unlock(dash_mutex);
	EXIT();
	return TRUE;
}


/*!
  \brief Removes a dashboard
  \param key is the dash gauge name pointer
  \param value is the dash gauge pointer
  \param user_data is the pointer to dash number
  \returns TRUE if it handles someting, FALSE otherwise
  */
G_MODULE_EXPORT gboolean remove_dashcluster(gpointer key, gpointer value, gpointer user_data)
{
	gchar *tmpbuf = NULL;
	Dash_Gauge *d_gauge = NULL;

	ENTER();

	tmpbuf = g_strdup_printf("dash_%i",(GINT)user_data);
	if (g_strrstr((gchar *)key,tmpbuf) != NULL)
	{
		g_free(tmpbuf);
		/* Found gauge in soon to be destroyed dash */
		d_gauge = (Dash_Gauge *)value;
		g_free(d_gauge->source);
		if (GTK_IS_WIDGET(d_gauge->dash))
		{
			guint id = (GINT)OBJ_GET(d_gauge->dash,"timer_id");
			if (id)
			{
				g_source_remove(id);
				OBJ_SET(d_gauge->dash,"timer_id", NULL);
			}
			gtk_widget_destroy(gtk_widget_get_toplevel(d_gauge->dash));
		}
		g_free(d_gauge);
		EXIT();
		return TRUE;
	}
	else
		g_free(tmpbuf);

	EXIT();
	return FALSE;
}

/*! 
  \brief dummy function that jsut returns true to block signal...
  \param widget is unused
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean dummy(GtkWidget *widget,gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*!
  \brief creates a gauge for a tab
  \param parent is the container for the gauge to be created
  */
G_MODULE_EXPORT void create_gauge(GtkWidget *parent)
{
	GtkWidget * gauge = NULL;
	gchar * xml_name = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gint table_num = -1;
	GList **tab_gauges = NULL;
	gchar *pathstub = NULL;

	ENTER();

	tab_gauges = (GList **)DATA_GET(global_data,"tab_gauges");
	gauge = mtx_gauge_face_new();
	gtk_container_add(GTK_CONTAINER(parent),gauge);
	xml_name = (gchar *)OBJ_GET(parent,"gaugexml");
	if (xml_name)
	{
		pathstub = g_build_filename(GAUGES_DATA_DIR,xml_name,NULL);
		filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,NULL);
		g_free(pathstub);
	}
	if (filename)
	{
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		g_free(filename);
	}
	OBJ_SET_FULL(gauge,"datasource",g_strdup((gchar *)OBJ_GET(parent,"datasource")),g_free);
	tmpbuf = (gchar *)OBJ_GET(parent,"table_num");
	table_num = (GINT)g_ascii_strtod(tmpbuf,NULL);
	tab_gauges[table_num] = g_list_prepend(tab_gauges[table_num],gauge);
	gtk_widget_show(gauge);
	EXIT();
	return;
}


/*!
  \brief updates tab gauges with new data
  */
G_MODULE_EXPORT gboolean update_tab_gauges(void)
{
	GtkWidget *gauge = NULL;
	gchar * source = NULL;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	guint i = 0;
	GList *list = NULL;
	GList **tab_gauges = NULL;

	ENTER();

	/*printf("updating gauges active table is %i\n",(GINT)DATA_GET(global_data,"active_table"));*/
	tab_gauges = (GList **)DATA_GET(global_data,"tab_gauges");
	
	if ((!tab_gauges) || ((GINT)DATA_GET(global_data,"active_table") < 0))
	{
		EXIT();
		return FALSE;
	}
	list = g_list_first(tab_gauges[(GINT)DATA_GET(global_data,"active_table")]);
	for (i=0;i<g_list_length(list);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(list,i);
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
				source = (gchar *)OBJ_GET(gauge,"datasource");
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
	EXIT();
	return FALSE;
}


/*!
  \brief wrapper to call the real function via g_idle_add
  \param data is the pointer to dashboard widget
  \returns FALSE
  */
G_MODULE_EXPORT gboolean hide_dash_resizers_wrapper(gpointer data)
{
	ENTER();
	g_idle_add(hide_dash_resizers,data);
	EXIT();
	return FALSE;
}

/*!
  \brief Hides dashboard resizers
  \param data is the pointer to dashboard widget
  \returns FALSE
  */
G_MODULE_EXPORT gboolean hide_dash_resizers(gpointer data)
{
	ENTER();
	if (!data)
	{
		EXIT();
		return FALSE;
	}
	if ((GTK_IS_WIDGET(data)) && (OBJ_GET(data,"resizers_visible")))
		dash_shape_combine((GtkWidget *)data,TRUE);
	OBJ_SET(data,"timer_active",GINT_TO_POINTER(FALSE));
	OBJ_SET(data,"timer_id",GINT_TO_POINTER(0));
	OBJ_SET(data,"resizers_visible",GINT_TO_POINTER(FALSE));
	EXIT();
	return FALSE;
}


/*!
  \brief Enter/Leave Event handler
  \param widget is the pointer to dash window/eventbox
  \param event is the pointer to a GdkEventCrossing structure
  \param data is unused
  \returns FALSE normally
  */
G_MODULE_EXPORT gboolean enter_leave_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	GtkWidget *dash = gtk_bin_get_child(GTK_BIN(widget));

	ENTER();

	if (event->state & GDK_BUTTON1_MASK)
	{
		EXIT();
		return TRUE;
	}
	OBJ_SET(dash,"moving",GINT_TO_POINTER(FALSE));
	OBJ_SET(dash,"resizing",GINT_TO_POINTER(FALSE));
	/* If "leaving" the window, set timeout to hide the resizers */
	if ((!OBJ_GET(dash,"timer_active")) && (OBJ_GET(dash,"resizers_visible")))
	{
		guint id = g_timeout_add(5000,hide_dash_resizers_wrapper,dash);
		OBJ_SET(dash,"timer_active",GINT_TO_POINTER(TRUE));
		OBJ_SET(dash,"timer_id",GINT_TO_POINTER(id));
	}
	EXIT();
	return FALSE;
}


/*!
  \brief Toggles the dashboard fullscreen status
  \param widget is the pointer to dashboard widget
  \param data is unused
  */
G_MODULE_EXPORT void toggle_dash_fullscreen(GtkWidget *widget, gpointer data)
{
	GtkWidget *dash = (GtkWidget *)OBJ_GET(widget,"dash");

	ENTER();

	if ((GBOOLEAN)DATA_GET(global_data,"dash_fullscreen"))
	{
        	DATA_SET(global_data,"dash_fullscreen",GINT_TO_POINTER(FALSE));
		gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);
		gtk_window_unfullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
		if ((GBOOLEAN)OBJ_GET(dash,"dash_on_top"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),GTK_WINDOW(lookup_widget("main_window")));
	}
	else
	{
        	DATA_SET(global_data,"dash_fullscreen",GINT_TO_POINTER(TRUE));
		if ((GBOOLEAN)OBJ_GET(dash,"dash_on_top"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);
		gtk_window_fullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	}
	EXIT();
	return;
}


/*!
  \brief Toggles the dashboard force on top function
  \param widget is the pointer to dashboard widget
  \param data is unused
  */
G_MODULE_EXPORT void toggle_dash_on_top(GtkWidget *widget, gpointer data)
{
	GtkWidget *dash = gtk_bin_get_child(GTK_BIN(widget));

	ENTER();

	if ((GBOOLEAN)OBJ_GET(dash,"dash_on_top"))
	{
        	OBJ_SET(dash,"dash_on_top",GINT_TO_POINTER(FALSE));
		gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);
	}
	else
	{
		if (!(GBOOLEAN)DATA_GET(global_data,"dash_fullscreen"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),GTK_WINDOW(lookup_widget("main_window")));
        	OBJ_SET(dash,"dash_on_top",GINT_TO_POINTER(TRUE));
	}
	EXIT();
}


/*!
  \brief Toggles the visibility of the remainder of the Gui
  \param widget is the pointer to dash context menu item
  \param data is unused
  */
G_MODULE_EXPORT void toggle_gui_visible(GtkWidget *widget, gpointer data)
{
	ENTER();
	/* IF visible, hide them */
	if ((GBOOLEAN)DATA_GET(global_data,"gui_visible"))
	{
		if ((GBOOLEAN)DATA_GET(global_data,"main_visible"))
			toggle_main_visible();
		if ((GBOOLEAN)DATA_GET(global_data,"status_visible"))
			toggle_status_visible();
		if ((GBOOLEAN)DATA_GET(global_data,"rtt_visible"))
			toggle_rtt_visible();
		DATA_SET(global_data,"gui_visible",GINT_TO_POINTER(FALSE));
	}
	else
	{
		if (!(GBOOLEAN)DATA_GET(global_data,"main_visible"))
			toggle_main_visible();
		if (!(GBOOLEAN)DATA_GET(global_data,"status_visible"))
			toggle_status_visible();
		if (!(GBOOLEAN)DATA_GET(global_data,"rtt_visible"))
			toggle_rtt_visible();
		DATA_SET(global_data,"gui_visible",GINT_TO_POINTER(TRUE));
	}
	EXIT();
	return;	
}


/*!
  \brief updates the values of all gauges on all dashboards in use
  \param data is unused
  \returns TRUE unless app is closing down
  */
G_MODULE_EXPORT gboolean update_dashboards(gpointer data)
{
	static GMutex *dash_mutex = NULL;
	ENTER();
	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return FALSE;
	}
	if (!dash_mutex)
		dash_mutex = (GMutex *)DATA_GET(global_data,"dash_mutex");

	g_mutex_lock(dash_mutex);
	if (DATA_GET(global_data,"dash_hash"))
		g_hash_table_foreach((GHashTable *)DATA_GET(global_data,"dash_hash"),update_dash_gauge,NULL);
	g_mutex_unlock(dash_mutex);
	EXIT();
	return FALSE;
}


/*!
  \brief Prints the available dashboard choices to std output
  */
G_MODULE_EXPORT void print_dash_choices(gchar *project)
{
	GDir * dir = NULL;
	gchar * path = NULL;
	const gchar * file = NULL;
	gchar **vector = NULL;
	const gchar *proj =  NULL;
	GError *err = NULL;

	ENTER();
	if (project)
		proj = (const gchar *)project;
	else
		proj = DEFAULT_PROJECT;

	/* Personal Path */
	path = g_build_filename(HOME(),"mtx",proj,"Dashboards",NULL);
	dir = g_dir_open(path,0,&err);
	g_free(path);

	printf("Here's the potential dash choices\n");
	if (dir)
	{
		while ((file = g_dir_read_name(dir)) != NULL)
		{
			vector = g_strsplit(file,".xml",2);
			printf ("%s\n",vector[0]);
			g_strfreev(vector);
		}
	}
	g_dir_close(dir);

	/* System Path */
	path = g_build_filename(MTXSYSDATA,"Dashboards",NULL);
	dir = g_dir_open(path,0,&err);
	g_free(path);

	if (dir)
	{
		while ((file = g_dir_read_name(dir)) != NULL)
		{
			vector = g_strsplit(file,".xml",2);
			printf ("%s\n",vector[0]);
			g_strfreev(vector);
		}
	}
	g_dir_close(dir);
	EXIT();
	return;
}


/*!
  \brief Validates the user supplied dashboard choice via the -D option
  \param choice is the user supplied dashboard choice
  \param result is the pointer to result variable, set to TRUE if 
  choice is valid, FALSE otherwise
  \returns pointer to dashboard config file found or NULL
  */
G_MODULE_EXPORT gchar * validate_dash_choice(gchar * choice, gboolean *result)
{
	gchar *path = NULL;
	gboolean found = FALSE;
	gchar * filename = NULL;
	const gchar *project = NULL;

	ENTER();

	filename = g_strdup_printf("%s.xml",choice);

	project = (const gchar *)DATA_GET(global_data,"project_name");
	if (!project)
		project = DEFAULT_PROJECT;
	/* Check personal path first */
	path = g_build_filename(HOME(),"mtx",project,"Dashboards",filename,NULL);
	if (g_file_test(path,G_FILE_TEST_IS_REGULAR))
		found = TRUE;
	else
		path = g_build_filename(MTXSYSDATA,"Dashboards",filename,NULL);
	if (g_file_test(path,G_FILE_TEST_IS_REGULAR))
		found = TRUE;
	g_free(filename);
	if (result)
		*result = found;
	EXIT();
	if (found)
		return (path);
	else 
		return NULL;
}


G_MODULE_EXPORT void dash_file_chosen(GtkFileChooserButton *button, gpointer data)
{
	gchar * filename = NULL;
	gint index = 0;
	GtkWidget *dash = NULL;
	GHashTable *dash_hash = (GHashTable *)DATA_GET(global_data,"dash_hash");
	ENTER();
	if (!dash_hash)
	{
		dash_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		DATA_SET_FULL(global_data,"dash_hash",dash_hash,(GDestroyNotify)g_hash_table_destroy);
	}

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button));
	OBJ_SET_FULL(button,"last_folder",gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button)),g_free);
	index = (GINT)OBJ_GET(button,"dash_index");
	if (filename)
	{
		if (dash_hash)
			g_hash_table_foreach_remove(dash_hash,remove_dashcluster,data);
		if (GTK_IS_WIDGET(button))
		{
			if (index == 1)
			{
				gtk_widget_set_sensitive(lookup_widget("dash_1_close_button"),TRUE);
				OBJ_SET_FULL(lookup_widget("dash_1_close_button"),"filename",g_strdup(filename),g_free);
			}
			if (index == 2)
			{
				gtk_widget_set_sensitive(lookup_widget("dash_2_close_button"),TRUE);
				OBJ_SET_FULL(lookup_widget("dash_2_close_button"),"filename",g_strdup(filename),g_free);
			}
		}
		dash = load_dashboard(filename,index);
		register_widget(filename,dash);
	}
	else
	{
		if (index == 1)
			gtk_widget_set_sensitive(lookup_widget("dash_1_close_button"),FALSE);
		if (index == 2)
			gtk_widget_set_sensitive(lookup_widget("dash_2_close_button"),FALSE);
	}
	g_free(filename);
	EXIT();
}


/*!
 *\brief sets up default settings for dash filechooserbuttons
 */
void dash_set_chooser_button_defaults(GtkFileChooser *button)
{
	gchar *syspath = NULL;
	gchar *homepath = NULL;
	GtkFileFilter *all_filter =  NULL;
	GtkFileFilter *xml_filter =  NULL;

	ENTER();

	syspath = g_build_filename(MTXSYSDATA,"Dashboards",NULL);                   
	homepath = g_build_filename(HOME(),"mtx",(const gchar *)DATA_GET(global_data,"project_name"),"Dashboards",NULL);
	gtk_file_chooser_set_current_folder(button,syspath);
	OBJ_SET_FULL(button,"syspath",g_strdup(syspath),g_free);
	OBJ_SET_FULL(button,"homepath",g_strdup(homepath),g_free);
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter,"*.*");
	gtk_file_filter_set_name(all_filter,"All Files");
	xml_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(xml_filter,"*.xml");
	gtk_file_filter_set_name(xml_filter,"XML Files");
	if (g_file_test(syspath,G_FILE_TEST_IS_DIR))                                
		gtk_file_chooser_set_current_folder(button,syspath);
	else if (g_file_test(homepath,G_FILE_TEST_IS_DIR))
		gtk_file_chooser_set_current_folder(button,homepath);
	gtk_file_chooser_add_filter(button,all_filter);
	gtk_file_chooser_add_filter(button,xml_filter);
	gtk_file_chooser_set_filter(button,xml_filter);
	gtk_file_chooser_add_shortcut_folder(button,syspath,NULL);
	gtk_file_chooser_add_shortcut_folder(button,homepath,NULL);
	g_free(syspath);                                                            
	g_free(homepath);

	EXIT();
	return;
}
