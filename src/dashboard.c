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
#include <defines.h>
#include <enums.h>
#include <getfiles.h>
#include <gauge.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


/*!
 \brief load_dashboard() loads hte specified dashboard configuration file
 and initializes the dash.
 \param  chooser, the fileshooser that triggered the signal
 \param data, user date
 */
void load_dashboard(GtkFileChooser *chooser, gpointer data)
{
	GtkWidget *window = NULL;
	GtkWidget *dash = NULL;
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	gchar * filename = NULL;

	filename = gtk_file_chooser_get_filename(chooser);
	if (filename == NULL)
		return;

	printf("file chosen %s\n",filename);
	LIBXML_TEST_VERSION

		/*parse the file and get the DOM */
		doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL)
	{
		printf("error: could not parse file %s\n",filename);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	dash = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window),dash);
	gtk_widget_show_all(window);

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	load_elements(dash,root_element);

	gtk_widget_show_all(window);
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
		/* Cheat to get property window created... */
//		create_preview_list(NULL,NULL);
//		update_properties(gauge,GAUGE_ADD);
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

