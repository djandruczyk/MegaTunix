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


#include <gauge.h>
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void
print_element_names(xmlNode * a_node)
{
	xmlNode *cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
	{
		if (cur_node->type == XML_ELEMENT_NODE) {
			printf("node type: Element, name: %s\n", cur_node->name);
		}

		print_element_names(cur_node->children);
	}
}


/**
 * Simple example to parse a file called "file.xml", 
 * walk down the DOM, and print the name of the 
 * xml elements nodes.
 */
void testload()
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;

	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
	LIBXML_TEST_VERSION

		/*parse the file and get the DOM */
		doc = xmlReadFile("output.xml", NULL, 0);

	if (doc == NULL) {
		printf("error: could not parse file output.xml\n");
	}

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);

	print_element_names(root_element);

	/*free the document */
	xmlFreeDoc(doc);

	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();

}

void output_xml(MtxGaugeFace * gauge)
{
	gchar * tmpbuf;
	xmlDocPtr doc = NULL;       /* document pointer */
	xmlNodePtr root_node = NULL;/* node pointers */
	xmlNodePtr node = NULL;/* node pointers */
	xmlDtdPtr dtd = NULL;       /* DTD pointer */
	MtxColorRange *range = NULL;
	int i;

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

	/* 
	 * xmlNewChild() creates a new node, which is "attached" as child node
	 * of root_node node. 
	 */
	tmpbuf = g_strdup_printf("%i %i %i", 
			gauge->colors[COL_BG].red, 
			gauge->colors[COL_BG].green, 
			gauge->colors[COL_BG].blue); 
	xmlNewChild(root_node, NULL, BAD_CAST "bg_color",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i %i %i", 
			gauge->colors[COL_NEEDLE].red, 
			gauge->colors[COL_NEEDLE].green, 
			gauge->colors[COL_NEEDLE].blue); 
	xmlNewChild(root_node, NULL, BAD_CAST "needle_color",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->needle_width);
	xmlNewChild(root_node, NULL, BAD_CAST "needle_width",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->needle_tail);
	xmlNewChild(root_node, NULL, BAD_CAST "needle_tail",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i %i %i", 
			gauge->colors[COL_MAJ_TICK].red, 
			gauge->colors[COL_MAJ_TICK].green, 
			gauge->colors[COL_MAJ_TICK].blue); 
	xmlNewChild(root_node, NULL, BAD_CAST "majtick_color",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i %i %i", 
			gauge->colors[COL_MIN_TICK].red, 
			gauge->colors[COL_MIN_TICK].green, 
			gauge->colors[COL_MIN_TICK].blue); 
	xmlNewChild(root_node, NULL, BAD_CAST "mintick_color",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i %i %i", 
			gauge->colors[COL_UNIT_FONT].red, 
			gauge->colors[COL_UNIT_FONT].green, 
			gauge->colors[COL_UNIT_FONT].blue); 
	xmlNewChild(root_node, NULL, BAD_CAST "unit_font_color",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i %i %i", 
			gauge->colors[COL_UNIT_FONT].red, 
			gauge->colors[COL_UNIT_FONT].green, 
			gauge->colors[COL_UNIT_FONT].blue); 
	xmlNewChild(root_node, NULL, BAD_CAST "name_font_color",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i %i %i", 
			gauge->colors[COL_VALUE_FONT].red, 
			gauge->colors[COL_VALUE_FONT].green, 
			gauge->colors[COL_VALUE_FONT].blue); 
	xmlNewChild(root_node, NULL, BAD_CAST "value_font_color",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",gauge->precision);
	xmlNewChild(root_node, NULL, BAD_CAST "precision",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",gauge->w);
	xmlNewChild(root_node, NULL, BAD_CAST "width",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",gauge->h);
	xmlNewChild(root_node, NULL, BAD_CAST "height",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->start_deg);
	xmlNewChild(root_node, NULL, BAD_CAST "start_deg",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->stop_deg);
	xmlNewChild(root_node, NULL, BAD_CAST "stop_deg",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->start_radian);
	xmlNewChild(root_node, NULL, BAD_CAST "start_radian",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->stop_radian);
	xmlNewChild(root_node, NULL, BAD_CAST "stop_radian",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->lbound);
	xmlNewChild(root_node, NULL, BAD_CAST "lbound",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->ubound);
	xmlNewChild(root_node, NULL, BAD_CAST "ubound",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	xmlNewChild(root_node, NULL, BAD_CAST "units_font",
			BAD_CAST gauge->units_font);

	xmlNewChild(root_node, NULL, BAD_CAST "units_str",
			BAD_CAST gauge->units_str);

	tmpbuf = g_strdup_printf("%f",gauge->units_font_scale);
	xmlNewChild(root_node, NULL, BAD_CAST "units_font_scale",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	xmlNewChild(root_node, NULL, BAD_CAST "name_font",
			BAD_CAST gauge->name_font);

	xmlNewChild(root_node, NULL, BAD_CAST "name_str",
			BAD_CAST gauge->name_str);

	tmpbuf = g_strdup_printf("%f",gauge->name_font_scale);
	xmlNewChild(root_node, NULL, BAD_CAST "name_font_scale",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	xmlNewChild(root_node, NULL, BAD_CAST "value_font",
			BAD_CAST gauge->value_font);

	tmpbuf = g_strdup_printf("%f",gauge->value_font_scale);
	xmlNewChild(root_node, NULL, BAD_CAST "value_font_scale",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",gauge->antialias);
	xmlNewChild(root_node, NULL, BAD_CAST "antialias",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",gauge->major_ticks);
	xmlNewChild(root_node, NULL, BAD_CAST "major_ticks",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",gauge->minor_ticks);
	xmlNewChild(root_node, NULL, BAD_CAST "minor_ticks",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->tick_inset);
	xmlNewChild(root_node, NULL, BAD_CAST "tick_inset",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->major_tick_len);
	xmlNewChild(root_node, NULL, BAD_CAST "major_tick_len",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%f",gauge->minor_tick_len);
	xmlNewChild(root_node, NULL, BAD_CAST "minor_tick_len",
			BAD_CAST tmpbuf);
	g_free(tmpbuf);

	for (i=0;i<gauge->ranges->len;i++)
	{
		range = g_array_index(gauge->ranges,MtxColorRange *, i);
		node = xmlNewChild(root_node, NULL, BAD_CAST "color_range",
				NULL );
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
		tmpbuf = g_strdup_printf("%i %i %i", 
				range->color.red, 
				range->color.green, 
				range->color.blue); 
		xmlNewChild(node, NULL, BAD_CAST "color",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);

	}


	xmlSaveFormatFileEnc("output.xml", doc, "UTF-8", 1);

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
}

#endif
