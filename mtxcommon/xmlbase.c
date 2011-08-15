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

/*!
  \file
  \ingroup MtxCommon
  \brief
  \author David Andruczyk
  */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <xmlbase.h>

/*!
  \brief Reads an Integer from an XML node and stores in the dest var
  \param node is the pointer to the XML node
  \param dest is the pointer to the place to store the read value
  */
void generic_xml_gint_import(xmlNode *node, gpointer dest)
{
	gint *val = NULL;

	g_return_if_fail(node);
	g_return_if_fail(dest);

	val = (gint *)dest;
	if (!node->children)
	{
		printf("ERROR, generic_xml_gint_import, xml node is empty!!\n");
		return;
	}
	if (!(node->children->type == XML_TEXT_NODE))
	{
		printf("TXT none node\n");
		return;
	}
	*val = (gint)g_ascii_strtod((gchar*)node->children->content,NULL);
}


/*!
  \brief Reads an boolean from an XML node and stores in the dest var
  \param node is the pointer to the XML node
  \param dest is the pointer to the place to store the read value
  */
void generic_xml_gboolean_import(xmlNode *node, gpointer dest)
{
	gboolean *val = NULL;

	g_return_if_fail(node);
	g_return_if_fail(dest);

	val = (gboolean *)dest;
	if (!node->children)
	{
		printf("ERROR, generic_xml_gboolean_import, xml node is empty!!\n");
		return;
	}
	if (!(node->children->type == XML_TEXT_NODE))
		return;
	if (!g_ascii_strcasecmp((gchar *)node->children->content, "TRUE"))
		*val = TRUE;
	else
		*val = FALSE;
}


/*!
  \brief Writes an integer to an XML node
  \param parent is the pointer to the parent XML node
  \param element_name is the name of the new XML node
  \param val is the value to store at that node
  */
void generic_xml_gint_export(xmlNode *parent, gchar *element_name, gint *val)
{
	gchar * tmpbuf = NULL;

	g_return_if_fail(parent);
	g_return_if_fail(element_name);
	g_return_if_fail(val);

	tmpbuf = g_strdup_printf("%i",*val);
	xmlNewChild(parent, NULL, BAD_CAST element_name,
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
}


/*!
  \brief Writes an boolean to an XML node
  \param parent is the pointer to the parent XML node
  \param element_name is the name of the new XML node
  \param val is the value to store at that node
  */
void generic_xml_gboolean_export(xmlNode *parent, gchar *element_name, gboolean *val)
{
	g_return_if_fail(parent);
	g_return_if_fail(element_name);
	g_return_if_fail(val);

	if (*val)
		xmlNewChild(parent, NULL, BAD_CAST element_name,
				BAD_CAST "TRUE");
	else
		xmlNewChild(parent, NULL, BAD_CAST element_name,
				BAD_CAST "FALSE");
}


/*!
  \brief Reads a Float from an XML node and stores in the dest var
  \param node is the pointer to the XML node
  \param dest is the pointer to the place to store the read value
  */
void generic_xml_gfloat_import(xmlNode *node, gpointer dest)
{
	gfloat *val = NULL;

	g_return_if_fail(node);
	g_return_if_fail(dest);

	val = (gfloat *)dest;
	if (!node->children)
	{
		printf("ERROR, generic_xml_gfloat_import, xml node is empty!!\n");
		return;
	}
	if (!(node->children->type == XML_TEXT_NODE))
		return;
	*val = g_ascii_strtod((gchar*)g_strdelimit((gchar *)node->children->content,",.",'.'),NULL);
}


/*!
  \brief Writes a gfloat to an XML node
  \param parent is the pointer to the parent XML node
  \param element_name is the name of the new XML node
  \param val is the value to store at that node
  */
void generic_xml_gfloat_export(xmlNode *parent, gchar *element_name, gfloat *val)
{
	gchar tmpbuf[10];
	gchar * buf = NULL;

	g_return_if_fail(parent);
	g_return_if_fail(element_name);
	g_return_if_fail(val);

	buf = g_ascii_dtostr(tmpbuf,10,*val);
	/*tmpbuf = g_strdup_printf("%f",*val); */
	xmlNewChild(parent, NULL, BAD_CAST element_name,
			BAD_CAST buf);
}


/*!
  \brief Reads a string from an XML node and stores in the dest var
  \param node is the pointer to the XML node
  \param dest is the pointer to the place to store the read value
  */
void generic_xml_gchar_import(xmlNode *node, gpointer dest)
{
	gchar **val = NULL;

	g_return_if_fail(node);
	g_return_if_fail(dest);

	val = (gchar **)dest;

	if (!node->children) /* EMPTY node, thus, clear the var on the gauge */
	{
		if (*val)
			g_free(*val);
		*val = g_strdup("");
		return;
	}
	if (!(node->children->type == XML_TEXT_NODE))
		return;

	if (*val)
		g_free(*val);
	if (node->children->content)
		*val = g_strdup((gchar *)node->children->content);
	else
		*val = g_strdup("");
}


/*!
  \brief Writes a gchar string to an XML node
  \param parent is the pointer to the parent XML node
  \param element_name is the name of the new XML node
  \param val is the value to store at that node
  */
void generic_xml_gchar_export(xmlNode *parent, gchar *element_name, gchar **val)
{
	g_return_if_fail(parent);
	g_return_if_fail(element_name);

	/* If the data to export is NOT null export it otherwise export an
	 * empty var */
	if (*(gchar **)val)
		xmlNewChild(parent, NULL, BAD_CAST element_name,BAD_CAST *(gchar **)val);
	else
		return;
}


/*!
  \brief Reads a GdkColor from an XML node and stores in the dest var
  \param node is the pointer to the XML node
  \param dest is the pointer to the place to store the read value
  */
void generic_xml_color_import(xmlNode *node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	GdkColor *color = NULL;
	gchar **vector = NULL;
	gint tmp = 0;

	g_return_if_fail(node);
	if (!node->children)
	{
		printf("ERROR, generic_xml_color_import, xml node is empty!!\n");
		return;
	}
	color = (GdkColor *)dest;
	cur_node = node->children;
	if (!cur_node->next)	/* OLD Style color block */
	{
		vector = g_strsplit((gchar*)node->children->content," ", 0);
		color->red = (guint16)g_ascii_strtod(vector[0],NULL);
		color->green = (guint16)g_ascii_strtod(vector[1],NULL);
		color->blue = (guint16)g_ascii_strtod(vector[2],NULL);
		g_strfreev(vector);
	}
	else
	{
		while (cur_node->next)
		{
			if (cur_node->type == XML_ELEMENT_NODE)
			{
				if (g_strcasecmp((gchar *)cur_node->name,"red") == 0)
				{
					generic_xml_gint_import(cur_node,&tmp);
					color->red=(guint16)tmp;
				}
				if (g_strcasecmp((gchar *)cur_node->name,"green") == 0)
				{
					generic_xml_gint_import(cur_node,&tmp);
					color->green=(guint16)tmp;
				}
				if (g_strcasecmp((gchar *)cur_node->name,"blue") == 0)
				{
					generic_xml_gint_import(cur_node,&tmp);
					color->blue=(guint16)tmp;
				}
			}
			cur_node = cur_node->next;
		}
	}
}


/*!
  \brief Writes a GdkColor to an XML node
  \param parent is the pointer to the parent XML node
  \param element_name is the name of the new XML node
  \param color is the value to store at that node
  */
void generic_xml_color_export(xmlNode *parent,gchar * element_name, GdkColor *color)
{
	gchar * tmpbuf =  NULL;
	xmlNode *child = NULL;

	g_return_if_fail(parent);
	g_return_if_fail(element_name);
	g_return_if_fail(color);

	child = xmlNewChild(parent, NULL, BAD_CAST element_name,NULL);

	tmpbuf = g_strdup_printf("%i",color->red);
	xmlNewChild(child, NULL, BAD_CAST "red",BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%i",color->green);
	xmlNewChild(child, NULL, BAD_CAST "green",BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%i",color->blue);
	xmlNewChild(child, NULL, BAD_CAST "blue",BAD_CAST tmpbuf);
	g_free(tmpbuf);
}


/*!
  \brief checks the API tags of an XML file for compliance
  \param node is the XML node to start from
  \param major is the major API to match on
  \param minor is the minor API to match on
  \returns TRUE on success, FALSE otherwise
  */
gboolean xml_api_check(xmlNode *node, gint major, gint minor)
{
	gint maj = -1;
	gint min = -1;
	xmlNode *cur_node = NULL;

	g_return_val_if_fail(node,FALSE);

	if (!node->children)
	{
		printf("ERROR, get_potential_arg_name, xml node is empty!!\n");
		return FALSE;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"major") == 0)
				generic_xml_gint_import(cur_node,&maj);
			if (g_strcasecmp((gchar *)cur_node->name,"minor") == 0)
				generic_xml_gint_import(cur_node,&min);

		}
		cur_node = cur_node->next;
	}
	
	if ((major != maj) || (minor != min))
		return FALSE;
	else
		return TRUE;
}
