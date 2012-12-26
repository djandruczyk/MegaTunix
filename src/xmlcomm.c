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
  \file src/xmlcomm.c
  \ingroup CoreMtx
  \brief XML code for dealing with COMM.XML I/O dedfinitions for all firmware
  \author David Andruczyk
  */

#include <debugging.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <plugin.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <xmlcomm.h>
#include <xmlbase.h>

extern gconstpointer *global_data;

/*!
  \brief Loads the comm_xml specific to this firmware persona
  \param filename is the name of the file to open/parse
  */
G_MODULE_EXPORT void load_comm_xml(gchar *filename)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;

	ENTER();
	if (filename == NULL)
	{
		MTXDBG(CRITICAL,_("XML filename is NULL!\n"));
		EXIT();
		return;
	}

	LIBXML_TEST_VERSION

	/*parse the file and get the DOM */
	doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL)
	{
		MTXDBG(CRITICAL,_("Could not parse file %s\n"),filename);
		EXIT();
		return;
	}

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	load_xmlcomm_elements(root_element);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	/*g_hash_table_foreach((GHashTable *)DATA_GET(global_data,"commands_hash"),xmlcomm_dump_commands,NULL);*/

	EXIT();
	return;
}


/*!
  \brief recursive function to iterate over the XML nodes
  \param a_node is the current XML node
  */
G_MODULE_EXPORT void load_xmlcomm_elements(xmlNode *a_node)
{
	static GHashTable *arguments = NULL;
	static GHashTable *commands = NULL;
	xmlNode *cur_node = NULL;

	ENTER();
	if (!arguments)
		arguments = (GHashTable *)DATA_GET(global_data,"potential_arguments");
	if (!commands)
		commands = (GHashTable *)DATA_GET(global_data,"commands_hash");

	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"potential_args") == 0)
				load_potential_args(arguments,cur_node);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"commands") == 0)
				load_commands(commands,cur_node);
		}
		load_xmlcomm_elements(cur_node->children);
	}
	EXIT();
	return;
}


/*!
  \brief loads the details of a potential argument from the XML, creates the
  PotentialArg structure and stores it
  \param arguments is the pointer to hashtable to store the arg 
  structures within
  \param node is the XML node
  */
G_MODULE_EXPORT void load_potential_args(GHashTable *arguments, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	PotentialArg *arg = NULL;

	ENTER();
	if (!node->children)
	{
		MTXDBG(CRITICAL,_("XML node is empty!!\n"));
		EXIT();
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"arg") == 0)
			{
				arg = g_new0(PotentialArg, 1);
				load_arg_details(arg, cur_node);
				g_hash_table_insert(arguments,g_strdup(arg->name),arg);
			}
		}
		cur_node = cur_node->next;

	}
	EXIT();
	return;
}


/*!
  \brief loads the details of an XML command argument from the XML, creates the
  Command structure and stores it
  \param commands_hash is the pointer to hashtable to store the Cmd 
  structures within
  \param node is the XML node
  */
G_MODULE_EXPORT void load_commands(GHashTable *commands_hash, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	Command *cmd = NULL;

	ENTER();
	if (!node->children)
	{
		MTXDBG(CRITICAL,_("XML node is empty!!\n"));
		EXIT();
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"cmd") == 0)
			{
				cmd = g_new0(Command, 1);
				cmd->dynamic = FALSE;
				cmd->post_functions = g_array_new(FALSE,TRUE,sizeof(PostFunction *));
				cmd->args = g_array_new(FALSE,TRUE,sizeof(PotentialArg *));
				load_cmd_details(cmd, cur_node);
				g_hash_table_insert(commands_hash,g_strdup(cmd->name),cmd);
			}
		}
		cur_node = cur_node->next;

	}
	EXIT();
	return;
}


/*!
  \brief loads the details of an XML PotentialArg from the XML.
  \param arg is the pointer to the PotentialArg structure
  \param node is the XML node
  */
G_MODULE_EXPORT void load_arg_details(PotentialArg *arg, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	gchar *tmpbuf = NULL;

	ENTER();
	if (!node->children)
	{
		MTXDBG(CRITICAL,_("XML node is empty!!\n"));
		EXIT();
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"name") == 0)
				generic_xml_gchar_import(cur_node,&arg->name);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"desc") == 0)
				generic_xml_gchar_import(cur_node,&arg->desc);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"internal_name") == 0)
				generic_xml_gchar_import(cur_node,&arg->internal_name);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"size") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				arg->size = (DataSize)translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"type") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				arg->type = (ArgType)translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"action") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				arg->action = (Action)translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"action_arg") == 0)
				generic_xml_gint_import(cur_node,&arg->action_arg);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"string") == 0)
			{
				generic_xml_gchar_import(cur_node,&arg->static_string);
				arg->string_len = strlen(arg->static_string);
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"hex_string") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				parse_hex_string(tmpbuf, arg->static_string, &arg->string_len);
				g_free(tmpbuf);
			}

		}
		cur_node = cur_node->next;
	}
	EXIT();
	return;
}


/*!
  \brief loads the details of an XML Command from the XML.
  \param cmd is the pointer to the Command structure
  \param node is the XML node
  */
G_MODULE_EXPORT void load_cmd_details(Command *cmd, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	gchar *tmpbuf = NULL;

	ENTER();
	if (!node->children)
	{
		MTXDBG(CRITICAL,_("XML node is empty!!\n"));
		EXIT();
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"name") == 0)
				generic_xml_gchar_import(cur_node,&cmd->name);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"desc") == 0)
				generic_xml_gchar_import(cur_node,&cmd->desc);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"type") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				cmd->type = (CmdType)translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"func_call_name") == 0)
			{
				generic_xml_gchar_import(cur_node,&cmd->func_call_name);
				if (!get_symbol(cmd->func_call_name,(void **)&cmd->function))
					printf(_("Unable to locate Function Call %s within MegaTunix or active plugins\n"),cmd->func_call_name);
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"func_call_arg") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				cmd->func_call_arg = translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"defer_post_functions") == 0)
				generic_xml_gboolean_import(cur_node,&cmd->defer_post_functions);

			if (g_ascii_strcasecmp((gchar *)cur_node->name,"base") == 0)
				generic_xml_gchar_import(cur_node,&cmd->base);

			if (g_ascii_strcasecmp((gchar *)cur_node->name,"helper_func") == 0)
			{
				generic_xml_gchar_import(cur_node,&cmd->helper_func_name);
				if (!get_symbol(cmd->helper_func_name,(void **)&cmd->helper_function))
					printf(_("Unable to locate Helper Function %s within MegaTunix or active plugins\n"),cmd->helper_func_name);
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"helper_func_arg") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				cmd->helper_func_arg = translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"post_functions") == 0)
			{
				load_cmd_post_functions(cmd,cur_node);
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"args") == 0)
				load_cmd_args(cmd,cur_node);
		}
		cur_node = cur_node->next;
	}
	EXIT();
	return;
}


/*!
  \brief loads the command args of an XML Command from the XML.
  \param cmd is the pointer to the Command structure
  \param node is the XML node
  */
G_MODULE_EXPORT void load_cmd_args(Command *cmd, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	gchar * tmpbuf = NULL;
	PotentialArg *arg = NULL;

	ENTER();
	if (!node->children)
	{
		MTXDBG(CRITICAL,_("XML node is empty!!\n"));
		EXIT();
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"arg") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				arg = (PotentialArg *)g_hash_table_lookup((GHashTable *)DATA_GET(global_data,"potential_arguments"),tmpbuf);
				cmd->args = g_array_append_val(cmd->args,arg);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
		}
		cur_node = cur_node->next;
	}
	EXIT();
	return;
}


/*!
  \brief loads the command post functions of an XML Command from the XML.
  \param cmd is the pointer to the Command structure
  \param node is the XML node
  */
G_MODULE_EXPORT void load_cmd_post_functions(Command *cmd, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	PostFunction *pf = NULL;

	ENTER();
	if (!node->children)
	{
		MTXDBG(CRITICAL,_("XML node is empty!!\n"));
		EXIT();
		return;
	}

	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"function") == 0)
			{
				pf = g_new0(PostFunction, 1);
				generic_xml_gchar_import(cur_node,&pf->name);
				if (!get_symbol(pf->name,(void **)&pf->function))
					printf(_("Unable to locate Post Function %s within MegaTunix or active plugins\n"),pf->name);
				else
				{
					pf->w_arg = FALSE;
					g_array_append_val(cmd->post_functions,pf);
				}
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"function_w_arg") == 0)
			{
				pf = g_new0(PostFunction, 1);
				generic_xml_gchar_import(cur_node,&pf->name);
				if (!get_symbol(pf->name,(void **)&pf->function_w_arg))
					printf(_("Unable to locate Post Function with argument %s within MegaTunix or active plugins\n"),pf->name);
				else
				{
					pf->w_arg = TRUE;
					g_array_append_val(cmd->post_functions,pf);
				}
			}
		}
		cur_node = cur_node->next;
	}
	EXIT();
	return;
}


/*!
  \brief dumps the Commands from the Commands hashtable for debugging purposes
  \param key is the Commands hashtable key
  \param value is the pointer to the Command structure
  \param data is unused
  */
G_MODULE_EXPORT void xmlcomm_dump_commands(gpointer key, gpointer value, gpointer data)
{
	Command *cmd = NULL;
	PostFunction *pf = NULL;
	PotentialArg *arg = NULL;
	guint i = 0;

	ENTER();
	cmd = (Command *)value;
	printf(_("Command key \"%s\"\n"),(gchar *)key);
	printf(_("Command name \"%s\"\n"),cmd->name);
	printf(_("Command desc \"%s\"\n"),cmd->desc);
	if (cmd->base)
		printf(_("Command base \"%s\"\n"),cmd->base);
	if (cmd->helper_function)
		printf(_("Helper function \"%s\"\n"),cmd->helper_func_name);
	if (cmd->args->len > 0 )
	{
		printf(_("Command args (%i): \n"),cmd->args->len);
		for (i=0;i<cmd->args->len;i++)
		{
			arg = g_array_index(cmd->args,PotentialArg *,i);
			printf("  %s\n",arg->name);
		}
	}
	if (cmd->post_functions->len > 0 )
	{
		printf(_("Defer Post Functions (%i): \n"),cmd->defer_post_functions);
		printf(_("Post Functions (%i): \n"),cmd->post_functions->len);
		for (i=0;i<cmd->post_functions->len;i++)
		{
			pf = g_array_index(cmd->post_functions,PostFunction *,i);
			if (pf->w_arg)
				printf("  %s: %p\n",pf->name,(void *)pf->function_w_arg);
			else
				printf("  %s: %p\n",pf->name,(void *)pf->function);
		}
	}
	if (cmd->type == FUNC_CALL)
	{

		printf(_("Function call %s (%p)\n"),cmd->func_call_name,(void *)cmd->function);
	}
	printf("\n\n");
	EXIT();
	return;
}


/*!
  \brief parses a comma separated hex string 
  \param str is the input string
  \param dest is the place to put the parsed string
  \param str_len is the pointer to place the destination string length 
  into after parsing
  */
void parse_hex_string(gchar *str, gchar *dest, gint *str_len)
{
	gchar **vector = NULL;
	gint i = 0;
	gint len = 0;
	gint tmpi = 0;
	
	ENTER();
	g_assert(str_len);
	vector = g_strsplit(str,",",-1);
	len = g_strv_length(vector);
	dest = g_new0(gchar, len);
	for (i=0;i<len;i++)
	{
		tmpi = strtol(vector[i],NULL,16);
		dest[i]=tmpi;
	}
	g_strfreev(vector);
	*str_len = len;
	EXIT();
	return;
}
