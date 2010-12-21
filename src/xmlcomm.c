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
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <plugin.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <xmlcomm.h>
#include <xmlbase.h>


extern gconstpointer *global_data;

G_MODULE_EXPORT void load_comm_xml(gchar *filename)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;

	if (filename == NULL)
	{
		dbg_func(CRITICAL,g_strdup(_("comm xml filename is NULL!\n")));
		return;
	}

	LIBXML_TEST_VERSION

	/*parse the file and get the DOM */
	doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL)
	{
		dbg_func(CRITICAL,g_strdup_printf(_("error: could not parse file %s\n"),filename));
		return;
	}

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	load_xmlcomm_elements(root_element);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	/*g_hash_table_foreach((GHashTable *)DATA_GET(global_data,"commands_hash"),xmlcomm_dump_commands,NULL);*/

}

G_MODULE_EXPORT void load_xmlcomm_elements(xmlNode *a_node)
{
	static GHashTable *arguments = NULL;
	static GHashTable *commands = NULL;
	xmlNode *cur_node = NULL;

	if (!arguments)
		arguments = (GHashTable *)DATA_GET(global_data,"potential_arguments");
	if (!commands)
		commands = (GHashTable *)DATA_GET(global_data,"commands_hash");

	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"potential_args") == 0)
				load_potential_args(arguments,cur_node);
			if (g_strcasecmp((gchar *)cur_node->name,"commands") == 0)
				load_commands(commands,cur_node);
		}
		load_xmlcomm_elements(cur_node->children);
	}
}

G_MODULE_EXPORT void load_potential_args(GHashTable *arguments, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	PotentialArg *arg = NULL;

	if (!node->children)
	{
		dbg_func(CRITICAL,g_strdup(_("ERROR, get_potential_arg_name, xml node is empty!!\n")));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"arg") == 0)
			{
				arg = g_new0(PotentialArg, 1);
				load_arg_details(arg, cur_node);
				g_hash_table_insert(arguments,g_strdup(arg->name),arg);
			}
		}
		cur_node = cur_node->next;

	}
}


G_MODULE_EXPORT void load_commands(GHashTable *commands_hash, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	Command *cmd = NULL;

	if (!node->children)
	{
		dbg_func(CRITICAL,g_strdup(_("ERROR, load_commands, xml node is empty!!\n")));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"cmd") == 0)
			{
				cmd = g_new0(Command, 1);
				cmd->post_functions = g_array_new(FALSE,TRUE,sizeof(PostFunction *));
				cmd->args = g_array_new(FALSE,TRUE,sizeof(PotentialArg *));
				load_cmd_details(cmd, cur_node);
				g_hash_table_insert(commands_hash,g_strdup(cmd->name),cmd);
			}
		}
		cur_node = cur_node->next;

	}
}


G_MODULE_EXPORT void load_arg_details(PotentialArg *arg, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	gchar *tmpbuf = NULL;

	if (!node->children)
	{
		dbg_func(CRITICAL,g_strdup(_("ERROR, load_arg_details, xml node is empty!!\n")));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"name") == 0)
				generic_xml_gchar_import(cur_node,&arg->name);
			if (g_strcasecmp((gchar *)cur_node->name,"desc") == 0)
				generic_xml_gchar_import(cur_node,&arg->desc);
			if (g_strcasecmp((gchar *)cur_node->name,"internal_name") == 0)
				generic_xml_gchar_import(cur_node,&arg->internal_name);
			if (g_strcasecmp((gchar *)cur_node->name,"size") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				arg->size = translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_strcasecmp((gchar *)cur_node->name,"type") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				arg->type = translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_strcasecmp((gchar *)cur_node->name,"action") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				arg->action = translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_strcasecmp((gchar *)cur_node->name,"action_arg") == 0)
				generic_xml_gint_import(cur_node,&arg->action_arg);
			if (g_strcasecmp((gchar *)cur_node->name,"string") == 0)
			{
				generic_xml_gchar_import(cur_node,&arg->static_string);
				arg->string_len = strlen(arg->static_string);
			}
			if (g_strcasecmp((gchar *)cur_node->name,"hex_string") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				parse_hex_string(tmpbuf, arg->static_string, &arg->string_len);
				g_free(tmpbuf);
			}

		}
		cur_node = cur_node->next;
	}
}


G_MODULE_EXPORT void load_cmd_details(Command *cmd, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	gchar *tmpbuf = NULL;

	if (!node->children)
	{
		dbg_func(CRITICAL,g_strdup(_("ERROR, load_cmd_details, xml node is empty!!\n")));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"name") == 0)
				generic_xml_gchar_import(cur_node,&cmd->name);
			if (g_strcasecmp((gchar *)cur_node->name,"desc") == 0)
				generic_xml_gchar_import(cur_node,&cmd->desc);
			if (g_strcasecmp((gchar *)cur_node->name,"type") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				cmd->type = translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_strcasecmp((gchar *)cur_node->name,"func_call_name") == 0)
			{
				generic_xml_gchar_import(cur_node,&cmd->func_call_name);
				if (!get_symbol(cmd->func_call_name,(void *)&cmd->function))
					printf(_("Unable to locate Function %s within MegaTunix or active plugins\n"),cmd->func_call_name);
			}
			if (g_strcasecmp((gchar *)cur_node->name,"func_call_arg") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				cmd->func_call_arg = translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_strcasecmp((gchar *)cur_node->name,"defer_post_functions") == 0)
				generic_xml_gboolean_import(cur_node,&cmd->defer_post_functions);

			if (g_strcasecmp((gchar *)cur_node->name,"base") == 0)
				generic_xml_gchar_import(cur_node,&cmd->base);

			if (g_strcasecmp((gchar *)cur_node->name,"helper_func") == 0)
			{
				generic_xml_gchar_import(cur_node,&cmd->helper_func_name);
				if (!get_symbol(cmd->helper_func_name,(void *)&cmd->helper_function))
					printf(_("Unable to locate Function %s within MegaTunix or active plugins\n"),cmd->func_call_name);
			}
			if (g_strcasecmp((gchar *)cur_node->name,"helper_func_arg") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				cmd->helper_func_arg = translate_string(tmpbuf);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
			if (g_strcasecmp((gchar *)cur_node->name,"post_functions") == 0)
				load_cmd_post_functions(cmd,cur_node);
			if (g_strcasecmp((gchar *)cur_node->name,"args") == 0)
				load_cmd_args(cmd,cur_node);
		}
		cur_node = cur_node->next;
	}
}


G_MODULE_EXPORT void load_cmd_args(Command *cmd, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	gchar * tmpbuf = NULL;
	PotentialArg *arg = NULL;

	if (!node->children)
	{
		dbg_func(CRITICAL,g_strdup(_("ERROR, load_cmd_args, xml node is empty!!\n")));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"arg") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				arg = g_hash_table_lookup(DATA_GET(global_data,"potential_arguments"),tmpbuf);
				cmd->args = g_array_append_val(cmd->args,arg);
				g_free(tmpbuf);
				tmpbuf = NULL;
			}
		}
		cur_node = cur_node->next;
	}
}

G_MODULE_EXPORT void load_cmd_post_functions(Command *cmd, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	PostFunction *pf = NULL;

	if (!node->children)
	{
		dbg_func(CRITICAL,g_strdup(_("ERROR, load_cmd_post_functions, xml node is empty!!\n")));
		return;
	}

	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"function") == 0)
			{
				pf = g_new0(PostFunction, 1);
				generic_xml_gchar_import(cur_node,&pf->name);
				if (!get_symbol(pf->name,(void *)&pf->function))
					printf(_("Unable to locate Function %s within MegaTunix or active plugins\n"),cmd->func_call_name);
				pf->w_arg = FALSE;
				g_array_append_val(cmd->post_functions,pf);
			}
			if (g_strcasecmp((gchar *)cur_node->name,"function_w_arg") == 0)
			{
				pf = g_new0(PostFunction, 1);
				generic_xml_gchar_import(cur_node,&pf->name);
				if (!get_symbol(pf->name,(void *)&pf->function_w_arg))
					printf(_("Unable to locate Function %s within MegaTunix or active plugins\n"),cmd->func_call_name);
				pf->w_arg = TRUE;
				g_array_append_val(cmd->post_functions,pf);
			}
		}
		cur_node = cur_node->next;
	}
}


G_MODULE_EXPORT void xmlcomm_dump_commands(gpointer key, gpointer value, gpointer data)
{
	Command *cmd = NULL;
	PostFunction *pf = NULL;
	PotentialArg *arg = NULL;
	guint i = 0;

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
				printf("  %s: %p\n",pf->name,pf->function_w_arg);
			else
				printf("  %s: %p\n",pf->name,pf->function);
		}
	}
	if (cmd->type == FUNC_CALL)
	{

		printf(_("Function call %s (%p)\n"),cmd->func_call_name,cmd->function);
	}
	printf("\n\n");
}


void parse_hex_string(gchar *str, gchar *dest, gint *str_len)
{
	gchar **vector = NULL;
	gint i = 0;
	gint len = 0;
	gint tmpi = 0;
	

	printf("parse_hex_string, passed string %s\n",str);
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
	return;
}
