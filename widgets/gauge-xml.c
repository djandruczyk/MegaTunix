/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt gauge widget XML I/O
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */


#include <defines.h>
#include <gauge.h>
#include <gauge-xml.h>
#include <getfiles.h>
#include <gauge-private.h>
#include <stdio.h>
#include <xmlbase.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

/**
 * load_elements:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void
load_elements(MtxGaugeFace *gauge, xmlNode * a_node)
{
	xmlNode *cur_node = NULL;
	MtxXMLFuncs *xml_funcs = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
	{
		if (cur_node->type == XML_ELEMENT_NODE) 
		{
			/*printf("node type: Element, name: \"%s\"\n", cur_node->name);*/
			xml_funcs = NULL;
			xml_funcs = g_hash_table_lookup(priv->xmlfunc_hash,cur_node->name);
			/* If current element name has a set of function 
			 * handlers, call the handlers passing the child node.
			 * NOTE in cases where the xml tag has no data the 
			 * child is NULL, we STILL need to call the handler 
			 * as in this context it measn to CLEAR the value
			 * (applies to text vals like "units_str"). We do
			 * this in the handler by detecting that the node 
			 * passed is null and handle it appropriately
			 */ 
			if (xml_funcs) 
			{
				xml_funcs->import_func(gauge,cur_node,xml_funcs->dest_var);
			}

		}

		load_elements(gauge,cur_node->children);
	}
}


/**
 * Simple example to parse a file called "file.xml", 
 * walk down the DOM, and print the name of the 
 * xml elements nodes.
 * FILENAME passed is a SHORTname,  this function will use get_file to
 * get a full path on the system.
 */
void mtx_gauge_face_import_xml(MtxGaugeFace *gauge, gchar * filename)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	gchar *tmpbuf = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
	LIBXML_TEST_VERSION

	/*parse the file and get the DOM */
	doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL) {
		printf("error: could not parse file %s\n",filename);
	}
	else
	{

		/*Get the root element node */
		root_element = xmlDocGetRootElement(doc);

		g_object_freeze_notify(G_OBJECT(gauge));
		mtx_gauge_face_remove_all_text_blocks(gauge);
		mtx_gauge_face_remove_all_alert_ranges(gauge);
		mtx_gauge_face_remove_all_color_ranges(gauge);
		mtx_gauge_face_remove_all_tick_groups(gauge);
		mtx_gauge_face_remove_all_polygons(gauge);
		load_elements(gauge, root_element);
		priv->xc = priv->w / 2;
		priv->yc = priv->h / 2;
		priv->radius = MIN (priv->w/2, priv->h/2) - 5;
		g_object_thaw_notify(G_OBJECT(gauge));
		if (GTK_IS_WINDOW(GTK_WIDGET(gauge)->parent))
			gtk_window_resize((GtkWindow *)(((GtkWidget *)gauge)->parent),priv->w,priv->h);
		generate_gauge_background(gauge);
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),priv->lbound);
		mtx_gauge_face_redraw_canvas (gauge);

		priv->xml_filename = g_strdup(filename);
	}
	g_free(tmpbuf);

	/*free the document */
	xmlFreeDoc(doc);
	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();
}

void mtx_gauge_face_export_xml(MtxGaugeFace * gauge, gchar * filename)
{
	gint i = 0;
	xmlDocPtr doc = NULL;       /* document pointer */
	xmlNodePtr root_node = NULL;/* node pointers */
	xmlDtdPtr dtd = NULL;       /* DTD pointer */
	MtxDispatchHelper *helper = NULL;
	MtxXMLFuncs * xml_funcs = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	LIBXML_TEST_VERSION;

	/* 
	 * Creates a new document, a node and set it as a root node
	 */
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "gauge");
	xmlDocSetRootElement(doc, root_node);

	/*
	 * Creates a DTD declaration. Isn't mandatory. 
	 */
	dtd = xmlCreateIntSubset(doc, BAD_CAST "gauge", NULL, BAD_CAST "mtxgauge.dtd");

	/* Create a helper struct o bind data to for to 
	 * trim up the XML export functions.
	 */
	helper = g_new0(MtxDispatchHelper, 1);
	helper->gauge = gauge;
	helper->root_node = root_node;
	/** For each element,  get the varname, the pointer to the memory 
	 * where hte data is stored in the current gauge structure, and call
	 * the export function defined in the xml_funcs struct passing in the
	 * helper struct so that the generic export functions can get the 
	 * key names right and the memory addresses right.  It looks confusing
	 * but it works great.  See gauge-xml.h for the static struct binding
	 * keynames to import/export generic handler functions */
	for (i=0;i<priv->xmlfunc_array->len;i++)
	{
		xml_funcs = g_array_index(priv->xmlfunc_array,MtxXMLFuncs *, i);
		helper->element_name = xml_funcs->varname;
		helper->src = (gpointer)g_object_get_data(G_OBJECT(gauge),xml_funcs->varname);
		xml_funcs->export_func(helper);
	}
	
	g_free(helper);
	/*
	*/


	xmlSaveFormatFileEnc(filename, doc, "utf-8", 1);

	/*free the document */
	xmlFreeDoc(doc);

	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();

	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
	if (priv->xml_filename)
		g_free(priv->xml_filename);

	priv->xml_filename = g_strdup(filename);
}


void mtx_gauge_color_import(MtxGaugeFace *gauge,xmlNode *node,gpointer dest)
{
	generic_xml_color_import(node,dest);
}


void mtx_gauge_gfloat_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	generic_xml_gfloat_import(node,dest);
}

void mtx_gauge_gint_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	generic_xml_gint_import(node,dest);
}

void mtx_gauge_gchar_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	generic_xml_gchar_import(node,dest);
}


void mtx_gauge_color_range_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	MtxColorRange *range = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	if (!node->children)
	{
		printf("ERROR, mtx_gauge_color_range_import, xml node is empty!!\n");
		return;
	}

	range = g_new0(MtxColorRange, 1);
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"lowpoint") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&range->lowpoint);
			if (g_strcasecmp((gchar *)cur_node->name,"highpoint") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&range->highpoint);
			if (g_strcasecmp((gchar *)cur_node->name,"lwidth") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&range->lwidth);
			if (g_strcasecmp((gchar *)cur_node->name,"inset") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&range->inset);
			if (g_strcasecmp((gchar *)cur_node->name,"color") == 0)
				mtx_gauge_color_import(gauge, cur_node,&range->color);
		}
		cur_node = cur_node->next;
	}
	g_array_append_val(priv->c_ranges,range);
}


void mtx_gauge_alert_range_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	MtxAlertRange *range = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	if (!node->children)
	{
		printf("ERROR, mtx_gauge_alert_range_import, xml node is empty!!\n");
		return;
	}

	range = g_new0(MtxAlertRange, 1);
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"lowpoint") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&range->lowpoint);
			if (g_strcasecmp((gchar *)cur_node->name,"highpoint") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&range->highpoint);
			if (g_strcasecmp((gchar *)cur_node->name,"lwidth") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&range->lwidth);
			if (g_strcasecmp((gchar *)cur_node->name,"inset") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&range->inset);
			if (g_strcasecmp((gchar *)cur_node->name,"color") == 0)
				mtx_gauge_color_import(gauge, cur_node,&range->color);
		}
		cur_node = cur_node->next;
	}
	g_array_append_val(priv->a_ranges,range);
}


void mtx_gauge_text_block_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	MtxTextBlock *tblock = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	if (!node->children)
	{
		printf("ERROR, mtx_gauge_text_block_import, xml node is empty!!\n");
		return;
	}

	tblock = g_new0(MtxTextBlock, 1);
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"font") == 0)
				mtx_gauge_gchar_import(gauge, cur_node,&tblock->font);
			if (g_strcasecmp((gchar *)cur_node->name,"text") == 0)
				mtx_gauge_gchar_import(gauge, cur_node,&tblock->text);
			if (g_strcasecmp((gchar *)cur_node->name,"color") == 0)
				mtx_gauge_color_import(gauge, cur_node,&tblock->color);
			if (g_strcasecmp((gchar *)cur_node->name,"font_scale") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tblock->font_scale);
			if (g_strcasecmp((gchar *)cur_node->name,"x_pos") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tblock->x_pos);
			if (g_strcasecmp((gchar *)cur_node->name,"y_pos") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tblock->y_pos);
		}
		cur_node = cur_node->next;
	}
	g_array_append_val(priv->t_blocks,tblock);
}


void mtx_gauge_tick_group_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	MtxTickGroup *tgroup = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	if (!node->children)
	{
		printf("ERROR, mtx_gauge_tick_group_import, xml node is empty!!\n");
		return;
	}

	tgroup = g_new0(MtxTickGroup, 1);
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"font") == 0)
				mtx_gauge_gchar_import(gauge, cur_node,&tgroup->font);
			if (g_strcasecmp((gchar *)cur_node->name,"font_scale") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->font_scale);
			if (g_strcasecmp((gchar *)cur_node->name,"text") == 0)
				mtx_gauge_gchar_import(gauge, cur_node,&tgroup->text);
			if (g_strcasecmp((gchar *)cur_node->name,"text_color") == 0)
				mtx_gauge_color_import(gauge, cur_node,&tgroup->text_color);
			if (g_strcasecmp((gchar *)cur_node->name,"text_inset") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->text_inset);
			if (g_strcasecmp((gchar *)cur_node->name,"maj_tick_color") == 0)
				mtx_gauge_color_import(gauge, cur_node,&tgroup->maj_tick_color);
			if (g_strcasecmp((gchar *)cur_node->name,"min_tick_color") == 0)
				mtx_gauge_color_import(gauge, cur_node,&tgroup->min_tick_color);
			if (g_strcasecmp((gchar *)cur_node->name,"maj_tick_inset") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->maj_tick_inset);
			if (g_strcasecmp((gchar *)cur_node->name,"min_tick_inset") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->min_tick_inset);
			if (g_strcasecmp((gchar *)cur_node->name,"maj_tick_length") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->maj_tick_length);
			if (g_strcasecmp((gchar *)cur_node->name,"min_tick_length") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->min_tick_length);
			if (g_strcasecmp((gchar *)cur_node->name,"maj_tick_width") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->maj_tick_width);
			if (g_strcasecmp((gchar *)cur_node->name,"min_tick_width") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->min_tick_width);
			if (g_strcasecmp((gchar *)cur_node->name,"start_angle") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->start_angle);
			if (g_strcasecmp((gchar *)cur_node->name,"sweep_angle") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&tgroup->sweep_angle);
			if (g_strcasecmp((gchar *)cur_node->name,"num_maj_ticks") == 0)
				mtx_gauge_gint_import(gauge, cur_node,&tgroup->num_maj_ticks);
			if (g_strcasecmp((gchar *)cur_node->name,"num_min_ticks") == 0)
				mtx_gauge_gint_import(gauge, cur_node,&tgroup->num_min_ticks);
		}
		cur_node = cur_node->next;
	}
	g_array_append_val(priv->tick_groups,tgroup);
}


void mtx_gauge_polygon_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	MtxPolygon *poly = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	if (!node->children)
	{
		printf("ERROR, mtx_gauge_polygon_import, xml node is empty!!\n");
		return;
	}

	poly = g_new0(MtxPolygon, 1);
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"color") == 0)
				mtx_gauge_color_import(gauge, cur_node,&poly->color);
			if (g_strcasecmp((gchar *)cur_node->name,"filled") == 0)
				mtx_gauge_gint_import(gauge, cur_node,&poly->filled);
			if (g_strcasecmp((gchar *)cur_node->name,"line_width") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&poly->line_width);
			if (g_strcasecmp((gchar *)cur_node->name,"line_style") == 0)
				mtx_gauge_gint_import(gauge, cur_node,&poly->line_style);
			if (g_strcasecmp((gchar *)cur_node->name,"join_style") == 0)
				mtx_gauge_gint_import(gauge, cur_node,&poly->join_style);
			if (g_strcasecmp((gchar *)cur_node->name,"Circle") == 0)
			{
				poly->data = g_new0(MtxCircle,1);
				poly->type = MTX_CIRCLE;
				mtx_gauge_poly_circle_import(gauge, cur_node,poly->data);
			}
			if (g_strcasecmp((gchar *)cur_node->name,"Arc") == 0)
			{
				poly->data = g_new0(MtxArc,1);
				poly->type = MTX_ARC;
				mtx_gauge_poly_arc_import(gauge, cur_node,poly->data);
			}
			if (g_strcasecmp((gchar *)cur_node->name,"Rectangle") == 0)
			{
				poly->data = g_new0(MtxRectangle,1);
				poly->type = MTX_RECTANGLE;
				mtx_gauge_poly_rectangle_import(gauge, cur_node,poly->data);
			}
			if (g_strcasecmp((gchar *)cur_node->name,"GenPolygon") == 0)
			{
				poly->data = g_new0(MtxGenPoly,1);
				poly->type = MTX_GENPOLY;
				mtx_gauge_poly_generic_import(gauge, cur_node,poly->data);
			}
		}
		cur_node = cur_node->next;
	}
	g_array_append_val(priv->polygons,poly);
}


void mtx_gauge_poly_circle_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	MtxCircle *data = NULL;
	if (!node->children)
	{
		printf("ERROR, mtx_gauge_poly_circle_import, xml node is empty!!\n");
		return;	
	}
	data = (MtxCircle *) dest;

	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"x") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->x);
			if (g_strcasecmp((gchar *)cur_node->name,"y") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->y);
			if (g_strcasecmp((gchar *)cur_node->name,"radius") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->radius);
		}
		cur_node = cur_node->next;
	}
}


void mtx_gauge_poly_rectangle_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	MtxRectangle *data = NULL;
	if (!node->children)
	{
		printf("ERROR, mtx_gauge_poly_rect_import, xml node is empty!!\n");
		return;
	}
	data = (MtxRectangle *) dest;

	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"x") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->x);
			if (g_strcasecmp((gchar *)cur_node->name,"y") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->y);
			if (g_strcasecmp((gchar *)cur_node->name,"width") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->width);
			if (g_strcasecmp((gchar *)cur_node->name,"height") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->height);
		}
		cur_node = cur_node->next;
	}
}


void mtx_gauge_poly_arc_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	MtxArc *data = NULL;
	if (!node->children)
	{
		printf("ERROR, mtx_gauge_poly_arc_import, xml node is empty!!\n");
		return;
	}
	data = (MtxArc *) dest;

	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"x") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->x);
			if (g_strcasecmp((gchar *)cur_node->name,"y") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->y);
			if (g_strcasecmp((gchar *)cur_node->name,"width") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->width);
			if (g_strcasecmp((gchar *)cur_node->name,"height") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->height);
			if (g_strcasecmp((gchar *)cur_node->name,"start_angle") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->start_angle);
			if (g_strcasecmp((gchar *)cur_node->name,"sweep_angle") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node,&data->sweep_angle);
		}
		cur_node = cur_node->next;
	}
}


void mtx_gauge_poly_generic_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	gint i = 0;
	gchar *tmpbuf = NULL;
	gchar **x_vector = NULL;
	gchar **y_vector = NULL;
	xmlNode *cur_node = NULL;
	MtxGenPoly *data = NULL;
	data = (MtxGenPoly *) dest;

	cur_node = node->children;
	if (!node->children)
	{
		printf("ERROR, mtx_gauge_poly_generic_import, xml node is empty!!\n");
		return;
	}
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"num_points") == 0)
			{
				mtx_gauge_gint_import(gauge, cur_node,&data->num_points);
				data->points = g_new0(MtxPoint, data->num_points);
			}
			if (g_strcasecmp((gchar *)cur_node->name,"x_coords") == 0)
			{
				mtx_gauge_gchar_import(gauge, cur_node,&tmpbuf);
				x_vector = g_strsplit(tmpbuf," ", -1);
				g_free(tmpbuf);
				for (i=0;i<g_strv_length(x_vector);i++)
				{
					data->points[i].x = g_ascii_strtod(x_vector[i],NULL);
				}
				g_strfreev(x_vector);
				tmpbuf = NULL;
			}
			if (g_strcasecmp((gchar *)cur_node->name,"y_coords") == 0)
			{
				mtx_gauge_gchar_import(gauge, cur_node,&tmpbuf);
				y_vector = g_strsplit(tmpbuf," ", -1);
				g_free(tmpbuf);
				for (i=0;i<g_strv_length(y_vector);i++)
				{
					data->points[i].y = g_ascii_strtod(y_vector[i],NULL);
				}
				g_strfreev(y_vector);
				tmpbuf = NULL;
			}


		}
		cur_node = cur_node->next;
	}
}


void mtx_gauge_color_range_export(MtxDispatchHelper * helper)
{
	gint i = 0;
	gchar * tmpbuf = NULL;
	MtxColorRange *range = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(helper->gauge);
	xmlNodePtr node = NULL;

	for (i=0;i<priv->c_ranges->len;i++)
	{
		range = g_array_index(priv->c_ranges,MtxColorRange *, i);
		node = xmlNewChild(helper->root_node, NULL, BAD_CAST "color_range",NULL );

		tmpbuf = g_strdup_printf("%f",range->lowpoint);
		xmlNewChild(node, NULL, BAD_CAST "lowpoint",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",range->highpoint);
		xmlNewChild(node, NULL, BAD_CAST "highpoint",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",range->lwidth);
		xmlNewChild(node, NULL, BAD_CAST "lwidth",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",range->inset);
		xmlNewChild(node, NULL, BAD_CAST "inset",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		generic_xml_color_export(node,"color",&range->color);

	}
}


void mtx_gauge_alert_range_export(MtxDispatchHelper * helper)
{
	gint i = 0;
	gchar * tmpbuf = NULL;
	MtxAlertRange *range = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(helper->gauge);
	xmlNodePtr node = NULL;

	for (i=0;i<priv->a_ranges->len;i++)
	{
		range = g_array_index(priv->a_ranges,MtxAlertRange *, i);
		node = xmlNewChild(helper->root_node, NULL, BAD_CAST "alert_range",NULL );

		tmpbuf = g_strdup_printf("%f",range->lowpoint);
		xmlNewChild(node, NULL, BAD_CAST "lowpoint",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",range->highpoint);
		xmlNewChild(node, NULL, BAD_CAST "highpoint",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",range->lwidth);
		xmlNewChild(node, NULL, BAD_CAST "lwidth",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",range->inset);
		xmlNewChild(node, NULL, BAD_CAST "inset",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		generic_xml_color_export(node,"color",&range->color);
	}
}


void mtx_gauge_text_block_export(MtxDispatchHelper * helper)
{
	gint i = 0;
	gchar * tmpbuf = NULL;
	MtxTextBlock *tblock = NULL;
	xmlNodePtr node = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(helper->gauge);

	for (i=0;i<priv->t_blocks->len;i++)
	{
		tblock = g_array_index(priv->t_blocks,MtxTextBlock *, i);
		node = xmlNewChild(helper->root_node, NULL, BAD_CAST "text_block",NULL );

		tmpbuf = g_strdup_printf("%s",tblock->font);
		xmlNewChild(node, NULL, BAD_CAST "font",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%s",tblock->text);
		xmlNewChild(node, NULL, BAD_CAST "text",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tblock->font_scale);
		xmlNewChild(node, NULL, BAD_CAST "font_scale",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tblock->x_pos);
		xmlNewChild(node, NULL, BAD_CAST "x_pos",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tblock->y_pos);
		xmlNewChild(node, NULL, BAD_CAST "y_pos",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		generic_xml_color_export(node,"color",&tblock->color);
	}
}


void mtx_gauge_tick_group_export(MtxDispatchHelper * helper)
{
	gint i = 0;
	gchar * tmpbuf = NULL;
	MtxTickGroup *tgroup = NULL;
	xmlNodePtr node = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(helper->gauge);

	for (i=0;i<priv->tick_groups->len;i++)
	{
		tgroup = g_array_index(priv->tick_groups,MtxTickGroup *, i);
		node = xmlNewChild(helper->root_node, NULL, BAD_CAST "tick_group",NULL );

		tmpbuf = g_strdup_printf("%s",tgroup->font);
		xmlNewChild(node, NULL, BAD_CAST "font",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tgroup->font_scale);
		xmlNewChild(node, NULL, BAD_CAST "font_scale",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%s",tgroup->text);
		xmlNewChild(node, NULL, BAD_CAST "text",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tgroup->text_inset);
		xmlNewChild(node, NULL, BAD_CAST "text_inset",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		generic_xml_color_export(node,"text_color",&tgroup->text_color);

		tmpbuf = g_strdup_printf("%i",tgroup->num_maj_ticks);
		xmlNewChild(node, NULL, BAD_CAST "num_maj_ticks",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		generic_xml_color_export(node,"maj_tick_color",&tgroup->maj_tick_color);

		tmpbuf = g_strdup_printf("%f",tgroup->maj_tick_inset);
		xmlNewChild(node, NULL, BAD_CAST "maj_tick_inset",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tgroup->maj_tick_length);
		xmlNewChild(node, NULL, BAD_CAST "maj_tick_length",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tgroup->maj_tick_width);
		xmlNewChild(node, NULL, BAD_CAST "maj_tick_width",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%i",tgroup->num_min_ticks);
		xmlNewChild(node, NULL, BAD_CAST "num_min_ticks",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		generic_xml_color_export(node,"min_tick_color",&tgroup->min_tick_color);

		tmpbuf = g_strdup_printf("%f",tgroup->min_tick_inset);
		xmlNewChild(node, NULL, BAD_CAST "min_tick_inset",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tgroup->min_tick_length);
		xmlNewChild(node, NULL, BAD_CAST "min_tick_length",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tgroup->min_tick_width);
		xmlNewChild(node, NULL, BAD_CAST "min_tick_width",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tgroup->start_angle);
		xmlNewChild(node, NULL, BAD_CAST "start_angle",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",tgroup->sweep_angle);
		xmlNewChild(node, NULL, BAD_CAST "sweep_angle",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);

	}
}


void mtx_gauge_poly_circle_export(xmlNodePtr root_node, MtxPolygon* poly)
{
	gchar * tmpbuf = NULL;
	MtxCircle * data = (MtxCircle *)poly->data;
	xmlNodePtr node = NULL;

	node = xmlNewChild(root_node, NULL, BAD_CAST "Circle",NULL );

	tmpbuf = g_strdup_printf("%f",data->x);
	xmlNewChild(node, NULL, BAD_CAST "x",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->y);
	xmlNewChild(node, NULL, BAD_CAST "y",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->radius);
	xmlNewChild(node, NULL, BAD_CAST "radius",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
}


void mtx_gauge_poly_rectangle_export(xmlNodePtr root_node, MtxPolygon* poly)
{
	gchar * tmpbuf = NULL;
	MtxRectangle * data = (MtxRectangle *)poly->data;
	xmlNodePtr node = NULL;

	node = xmlNewChild(root_node, NULL, BAD_CAST "Rectangle",NULL );

	tmpbuf = g_strdup_printf("%f",data->x);
	xmlNewChild(node, NULL, BAD_CAST "x",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->y);
	xmlNewChild(node, NULL, BAD_CAST "y",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->width);
	xmlNewChild(node, NULL, BAD_CAST "width",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->height);
	xmlNewChild(node, NULL, BAD_CAST "height",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
}


void mtx_gauge_poly_arc_export(xmlNodePtr root_node, MtxPolygon* poly)
{
	gchar * tmpbuf = NULL;
	MtxArc * data = (MtxArc *)poly->data;
	xmlNodePtr node = NULL;

	node = xmlNewChild(root_node, NULL, BAD_CAST "Arc",NULL );

	tmpbuf = g_strdup_printf("%f",data->x);
	xmlNewChild(node, NULL, BAD_CAST "x",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->y);
	xmlNewChild(node, NULL, BAD_CAST "y",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->width);
	xmlNewChild(node, NULL, BAD_CAST "width",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->height);
	xmlNewChild(node, NULL, BAD_CAST "height",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->start_angle);
	xmlNewChild(node, NULL, BAD_CAST "start_angle",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%f",data->sweep_angle);
	xmlNewChild(node, NULL, BAD_CAST "sweep_angle",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
}


void mtx_gauge_poly_generic_export(xmlNodePtr root_node, MtxPolygon* poly)
{
	gint i = 0;
	gchar * tmpbuf = NULL;
	GString *string = NULL;
	MtxGenPoly * data = (MtxGenPoly *)poly->data;
	xmlNodePtr node = NULL;

	node = xmlNewChild(root_node, NULL, BAD_CAST "GenPolygon",NULL );

	tmpbuf = g_strdup_printf("%i",data->num_points);
	xmlNewChild(node, NULL, BAD_CAST "num_points",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
	string = g_string_new(NULL);
	for (i=0;i<data->num_points;i++)
	{
		tmpbuf = g_strdup_printf("%f",data->points[i].x);
		g_string_append(string,tmpbuf);
		g_free(tmpbuf);
		if (i < (data->num_points-1))
			g_string_append(string," ");
	}
	xmlNewChild(node, NULL, BAD_CAST "x_coords",
			BAD_CAST string->str);
	g_string_free(string,TRUE);
	string = g_string_new(NULL);
	for (i=0;i<data->num_points;i++)
	{
		tmpbuf = g_strdup_printf("%f",data->points[i].y);
		g_string_append(string,tmpbuf);
		g_free(tmpbuf);
		if (i < (data->num_points-1))
			g_string_append(string," ");
	}
	xmlNewChild(node, NULL, BAD_CAST "y_coords",
			BAD_CAST string->str);
	g_string_free(string,TRUE);

}


void mtx_gauge_polygon_export(MtxDispatchHelper * helper)
{
	gint i = 0;
	gchar * tmpbuf = NULL;
	MtxPolygon *poly = NULL;
	xmlNodePtr node = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(helper->gauge);

	for (i=0;i<priv->polygons->len;i++)
	{
		poly = g_array_index(priv->polygons,MtxPolygon *, i);
		node = xmlNewChild(helper->root_node, NULL, BAD_CAST "polygon",NULL );
		generic_xml_color_export(node,"color",&poly->color);

		tmpbuf = g_strdup_printf("%i",poly->filled);
		xmlNewChild(node, NULL, BAD_CAST "filled",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",poly->line_width);
		xmlNewChild(node, NULL, BAD_CAST "line_width",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%i",poly->line_style);
		xmlNewChild(node, NULL, BAD_CAST "line_style",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%i",poly->join_style);
		xmlNewChild(node, NULL, BAD_CAST "join_style",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		switch (poly->type)
		{
			case MTX_CIRCLE:
				mtx_gauge_poly_circle_export(node,poly);
				break;
			case MTX_RECTANGLE:
				mtx_gauge_poly_rectangle_export(node,poly);
				break;
			case MTX_ARC:
				mtx_gauge_poly_arc_export(node,poly);
				break;
			case MTX_GENPOLY:
				mtx_gauge_poly_generic_export(node,poly);
				break;
			default:
				break;
		}
	}
}


void mtx_gauge_color_export(MtxDispatchHelper * helper)
{
	generic_xml_color_export(helper->root_node,helper->element_name,helper->src);
}


void mtx_gauge_gfloat_export(MtxDispatchHelper * helper)
{
	generic_xml_gfloat_export(helper->root_node,helper->element_name,helper->src);
}

void mtx_gauge_gint_export(MtxDispatchHelper * helper)
{
	generic_xml_gint_export(helper->root_node,helper->element_name,helper->src);
}

void mtx_gauge_gchar_export(MtxDispatchHelper * helper)
{
	generic_xml_gchar_export(helper->root_node,helper->element_name,helper->src);
}


gchar * mtx_gauge_face_get_xml_filename(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	return g_strdup(priv->xml_filename);
}
#endif
