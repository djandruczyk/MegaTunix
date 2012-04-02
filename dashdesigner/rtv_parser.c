#include <configfile.h>
#include <getfiles.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <rtv_parser.h>
#include <stdio.h>
#include <strings.h>
#include <xmlbase.h>

GtkTreeStore *store = NULL;

void retrieve_rt_vars(void)
{
	gchar **files = NULL;
	GArray *classes = NULL;
	Rtv_Data *rtv_data = NULL;
	gint i = 0;
	/*printf("retrieve rt_vars from mtx realtime maps\n");*/

	files = get_files(g_build_path(PSEP,REALTIME_MAPS_DATA_DIR,NULL),g_strdup("xml"),&classes);
	if (!files)
		return;
	while(files[i])
		i++;
	rtv_data = g_new0(Rtv_Data, 1);
	rtv_data->persona_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,info_free);
	rtv_data->persona_array = g_array_new(FALSE,TRUE,sizeof(Persona_Info *));
	rtv_data->total_files = i;
	load_rtvars(files,rtv_data);
	g_array_free(classes,TRUE);
	g_strfreev(files);
	g_hash_table_destroy(rtv_data->persona_hash);
	g_array_free(rtv_data->persona_array,TRUE);
	g_free(rtv_data);

}

void load_rtvars(gchar **files, Rtv_Data *rtv_data)
{
	GtkTreeIter iter;
	GtkTreeIter parent;
	gboolean xml_result = FALSE;
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	Persona_Info *info = NULL;
	gint total = 0;
	gpointer orig = NULL;
	gpointer value = NULL;
	gchar * persona = NULL;
	gchar * tmpbuf = NULL;
	gchar * section = NULL;
	gchar ** vector = NULL;
	gchar *dlog_name = NULL;
	gchar *int_name = NULL;
	gchar *element = NULL;
	gint icount = 0;
	gint len = 0;
	gint tmpi = 0;
	gint i = 0;
	gint j = 0;
	guint k = 0;
	gint rtvcount = 0;
	gint personas = 0;

        LIBXML_TEST_VERSION

	while (files[i])
	{
		doc = xmlReadFile (files[i], NULL, 0);
		root_element = xmlDocGetRootElement(doc);
		if (!doc)
		{
			printf("XML PARSE FAILURE %s\n",files[i]);
			continue;
		}
		xml_result = parse_rtv_xml_for_dash(root_element,rtv_data);
		i++;
		xmlFreeDoc(doc);
	}
	xmlCleanupParser();

	store = gtk_tree_store_new(NUM_COLS,G_TYPE_STRING,G_TYPE_STRING);
	/*printf("Total number of uniq vars is %i\n",len);*/

	for (i=0;i<(gint)rtv_data->persona_array->len;i++)
	{
		info = NULL;
		info = g_array_index(rtv_data->persona_array,Persona_Info *,i);
		gtk_tree_store_append(store,&parent,NULL);
		gtk_tree_store_set(store,&parent,VARNAME_COL,info->persona,-1);
		for (j=0;j<(gint)g_list_length(info->rtv_list);j++)
		{
			gtk_tree_store_append(store,&iter,&parent);
			element = (gchar *)g_list_nth_data(info->rtv_list,j);
			int_name = (gchar *)g_hash_table_lookup(info->int_ext_hash,element);
			gtk_tree_store_set(store,&iter,VARNAME_COL,element,DATASOURCE_COL,int_name,-1);
		}
	}
}


gboolean parse_rtv_xml_for_dash(xmlNode *node, Rtv_Data *rtv_data)
{
	Persona_Info *info = NULL;
	xmlNode *cur_node = NULL;

	/* Iterate down... */
	for (cur_node = node; cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"realtime_map") == 0)
				load_rtv_defaults(cur_node,rtv_data,&info);
			if (g_strcasecmp((gchar *)cur_node->name,"derived") == 0)
				parse_derived_var(cur_node,rtv_data,info);
		}
		if (!parse_rtv_xml_for_dash(cur_node->children,rtv_data))
			return FALSE;
	}
	return TRUE;
}


void load_rtv_defaults(xmlNode *node, Rtv_Data *rtv_data, Persona_Info **persona_info)
{
	Persona_Info *info = NULL;
	gpointer orig = NULL;
	gpointer value = NULL;
	gchar * persona = NULL;
	if (!generic_xml_gchar_find(node,"persona",&persona))
	{
		printf("MISSING \"persona\" key in RTV XML, contact author!\n");
		return;
	}

	if (g_hash_table_lookup_extended(rtv_data->persona_hash,persona,&orig,&value))
		info = (Persona_Info *)value;
	else /* We just disovered this persona,  CREATE the hashtable for it and store */
	{
		info = g_new0(Persona_Info, 1);
		info->hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		info->int_ext_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
		info->rtv_list = NULL;
		info->persona = g_strdup(persona);
		g_hash_table_insert(rtv_data->persona_hash,g_strdup(persona),(gpointer)info);
		g_array_append_val(rtv_data->persona_array,info);
	}
	g_free(persona);
	*persona_info = info;
}


void parse_derived_var(xmlNode *node, Rtv_Data *rtv_data, Persona_Info *info)
{
	gpointer orig = NULL;
	gpointer value = NULL;
	gchar *dlog_gui_name = NULL;
	gchar * tmpbuf = NULL;
	gchar ** vector = NULL;
	guint k = 0;
	gint tmpi = 0;

	g_return_if_fail(info);
	generic_xml_gchar_find(node,"dlog_gui_name",&dlog_gui_name);
	if(generic_xml_gchar_find(node,"internal_names",&tmpbuf))
	{
		vector = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
		for (k=0;k<g_strv_length(vector);k++)
		{
			/* If we know about it, increase it's ref count */
			if (g_hash_table_lookup_extended(info->hash,vector[k],&orig,&value))
			{
				tmpi = (GINT)value + 1;
				g_hash_table_replace(info->hash,g_strdup(vector[k]),GINT_TO_POINTER(tmpi));
			}
			else
			{
				/*printf("inserting var %s with value %i\n",vector[k],1);*/
				g_hash_table_insert(info->hash,g_strdup(vector[k]),GINT_TO_POINTER(1));
				g_hash_table_insert(info->int_ext_hash,g_strdup(dlog_gui_name),g_strdup(vector[k]));
				info->rtv_list = g_list_prepend(info->rtv_list,g_strdup(dlog_gui_name));
			}
		}
		g_strfreev(vector);
		g_free(dlog_gui_name);
	}
	info->rtv_list = g_list_sort(info->rtv_list,sort);
}



gint sort(gconstpointer a, gconstpointer b)
{
	return g_strcasecmp((gchar *)a, (gchar *)b);
}


void info_free(gpointer data)
{
	Persona_Info *info = (Persona_Info *)data;
	g_hash_table_destroy(info->hash);
	g_hash_table_destroy(info->int_ext_hash);
	g_list_foreach(info->rtv_list, (GFunc)g_free, NULL);
	g_list_free(info->rtv_list);
	g_free(info->persona); 
	g_free(info);
}
