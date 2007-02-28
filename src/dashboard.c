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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <rtv_processor.h>
#include <structures.h>


extern gint dbg_lvl;
/*!
 \brief load_dashboard() loads hte specified dashboard configuration file
 and initializes the dash.
 \param  chooser, the fileshooser that triggered the signal
 \param data, user date
 */
void load_dashboard(gchar *filename, gpointer data)
{
	GtkWidget *window = NULL;
	GtkWidget *dash = NULL;
	//	GtkWidget *ebox = NULL;
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	extern gchar * cluster_1_name;
	extern gchar * cluster_2_name;
	extern gboolean interrogated;

	if (!interrogated)
		return;
	if (filename == NULL)
	{
		return;
	}

	LIBXML_TEST_VERSION

		/*parse the file and get the DOM */
		doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL)
	{
		printf("error: could not parse file %s\n",filename);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),"Dash Cluster");
	gtk_window_set_decorated(GTK_WINDOW(window),FALSE);
//	gtk_window_set_type_hint(GTK_WINDOW(window),GDK_WINDOW_TYPE_HINT_UTILITY);
	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (dummy), NULL);
	g_signal_connect (G_OBJECT (window), "destroy_event",
			G_CALLBACK (dummy), NULL);
	//	ebox = gtk_event_box_new();
	//	gtk_container_add(GTK_CONTAINER(window),ebox);
	if ((gint)data == 1)
	{
		if (cluster_1_name)
			g_free(cluster_1_name);
		cluster_1_name = g_strdup(filename);
	}
	if ((gint)data == 2)
	{
		if (cluster_2_name)
			g_free(cluster_2_name);
		cluster_2_name = g_strdup(filename);
	}

	dash = gtk_fixed_new();
	gtk_widget_add_events(GTK_WIDGET(dash),GDK_POINTER_MOTION_MASK|
			GDK_BUTTON_PRESS_MASK |GDK_BUTTON_RELEASE_MASK);
	g_signal_connect (G_OBJECT (dash), "motion_notify_event",
			G_CALLBACK (dash_motion_event), NULL);
	g_signal_connect (G_OBJECT (dash), "button_press_event",
			G_CALLBACK (dash_button_event), NULL);


	gtk_container_add(GTK_CONTAINER(window),dash);


	gtk_widget_show_all(window);
	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	load_elements(dash,root_element);

	link_dash_datasources(dash,data);

	dash_shape_combine(dash);


	gtk_widget_show_all(window);
	g_free(filename);
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
	gtk_window_resize(GTK_WINDOW(gtk_widget_get_toplevel(dash)),width,height);

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
		gtk_widget_set_usize(gauge,width,height);
		g_free(filename);
		g_object_set_data(G_OBJECT(gauge),"datasource",g_strdup(datasource));
		g_free(xml_name);
		g_free(datasource);
		gtk_widget_show_all(dash);
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
	struct Dash_Gauge *d_gauge = NULL;
	GtkFixedChild *child;
	GList *children = NULL;
	gint len = 0;
	gint i = 0;
	GObject * rtv_obj = NULL;
	gchar * source = NULL;
	extern GHashTable *dash_gauges;
	extern struct Rtv_Map *rtv_map;

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
		d_gauge = g_new0(struct Dash_Gauge, 1);
		d_gauge->object = rtv_obj;
		d_gauge->source = source;
		d_gauge->gauge = child->widget;
		d_gauge->dash = dash;
		g_hash_table_insert(dash_gauges,g_strdup_printf("dash_cluster_%i_gauge_%i",(gint)data,i),(gpointer)d_gauge);

	}
}

void update_dash_gauge(gpointer key, gpointer value, gpointer user_data)
{
	struct Dash_Gauge *d_gauge = (struct Dash_Gauge *)value;
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

	gtk_widget_size_request(dash,&req);

	bitmap = gdk_pixmap_new(NULL,req.width,req.height,1);

	colormap = gdk_colormap_get_system ();
	gdk_color_parse ("black", & black);
	gdk_colormap_alloc_color(colormap, &black,TRUE,TRUE);
	gdk_color_parse ("white", & white);
	gdk_colormap_alloc_color(colormap, &white,TRUE,TRUE);
	gc = gdk_gc_new (bitmap);
	gdk_gc_set_foreground (gc, &black);

	gdk_draw_rectangle(bitmap,gc,TRUE,0,0,req.width,req.height);
			

	children = GTK_FIXED(dash)->children;
	gdk_gc_set_foreground (gc, &white);

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
	if (GTK_IS_WINDOW(dash->parent))
	{
#ifdef HAVE_CAIRO
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
			gtk_widget_input_shape_combine_mask(dash->parent,bitmap,0,0);
#endif
#endif
		gtk_widget_shape_combine_mask(dash->parent,bitmap,0,0);
	}
	else
	{
#ifdef HAVE_CAIRO
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
			gdk_window_input_shape_combine_mask(dash->window,bitmap,0,0);
#endif
#endif
		gdk_window_shape_combine_mask(dash->window,bitmap,0,0);
	}

	return;
}

gboolean dash_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	return FALSE;
}


gboolean dash_button_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	 if ((event->type == GDK_BUTTON_PRESS) && (event->button == 1))
	 {
		 if (GTK_IS_WINDOW(widget->parent))
		 {
			 gtk_window_begin_move_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
					 event->button,
					 event->x_root,
					 event->y_root,
					 event->time);
			 return TRUE;
		 }
	 }
	 return FALSE;
}

void initialize_dashboards()
{
	GtkWidget * label = NULL;
	extern gchar * cluster_1_name;
	extern gchar * cluster_2_name;
	extern GHashTable *dynamic_widgets;

	label = g_hash_table_lookup(dynamic_widgets,"dash_cluster_1_label");
	if ((GTK_IS_LABEL(label)) && (cluster_1_name != NULL) && (g_ascii_strcasecmp(cluster_1_name,"") != 0))
	{
		gtk_label_set_text(GTK_LABEL(label),g_filename_to_utf8(cluster_1_name,-1,NULL,NULL,NULL));
		load_dashboard(g_strdup(cluster_1_name),GINT_TO_POINTER(1));
	}

	label = g_hash_table_lookup(dynamic_widgets,"dash_cluster_2_label");
	if ((GTK_IS_LABEL(label)) && (cluster_2_name != NULL) && (g_ascii_strcasecmp(cluster_2_name,"") != 0))
	{
		gtk_label_set_text(GTK_LABEL(label),g_filename_to_utf8(cluster_2_name,-1,NULL,NULL,NULL));
		load_dashboard(g_strdup(cluster_2_name),GINT_TO_POINTER(2));
	}
}


EXPORT gboolean present_dash_filechooser(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GtkWidget *label = NULL;
	extern gboolean interrogated;

	if (!interrogated)
		return FALSE;
	extern GHashTable *dash_gauges;

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("Dashboards");
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
	extern gchar * cluster_1_name;
	extern gchar * cluster_2_name;

	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		return FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),FALSE);
	label = g_object_get_data(G_OBJECT(widget),"label");
	if (GTK_IS_WIDGET(label))
	{
		gtk_label_set_text(GTK_LABEL(label),"Choose a Dashboard File");
		if ((gint)data == 1)
		{
			if (cluster_1_name)
			{
				g_free(cluster_1_name);
				cluster_1_name = NULL;
			}
		}
		if ((gint)data == 2)
		{
			if (cluster_2_name)
			{
				g_free(cluster_2_name);
				cluster_2_name = NULL;
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
	struct Dash_Gauge *d_gauge = NULL;

	tmpbuf = g_strdup_printf("dash_cluster_%i",(gint)user_data);
	if (g_strrstr((gchar *)key,tmpbuf) != NULL)
	{
		g_free(tmpbuf);
		/* Foudn gauge in soon to be destroyed dash */
		d_gauge = (struct Dash_Gauge *)value;
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
