#include <configfile.h>
#include <getfiles.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <rtv_parser.h>
#include <stdio.h>
#include <strings.h>

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
	rtv_data->persona_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	rtv_data->persona_array = g_array_new(FALSE,TRUE,sizeof(Persona_Info *));
	rtv_data->total_files = i;
	load_rtvars(files,rtv_data);
	g_array_free(classes,TRUE);

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

		cfg_read_string(cfgfile,"realtime_map", "persona",&persona);
		value = NULL;
		info = NULL;
		/* Check if we already know about it */


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
		cfg_read_int(cfgfile,"realtime_map", "derived_total",&total);
		for (j=0;j<total;j++)
		{
			section = g_strdup_printf("derived_%i",j);
			cfg_read_string(cfgfile,section,"dlog_gui_name",&dlog_name);
			if(cfg_read_string(cfgfile,section,"internal_names",&tmpbuf))
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
						g_hash_table_insert(info->int_ext_hash,g_strdup(dlog_name),g_strdup(vector[k]));
						info->rtv_list = g_list_prepend(info->rtv_list,g_strdup(dlog_name));
					}
				}
				g_strfreev(vector);
			}
			g_free(section);
			g_free(dlog_name);
			g_free(int_name);
		}
		info->rtv_list = g_list_sort(info->rtv_list,sort);
		cfg_free(cfgfile);
		i++;
	}

	store = gtk_tree_store_new(NUM_COLS,G_TYPE_STRING,G_TYPE_STRING);
	/*printf("Total number of uniq vars is %i\n",len);*/



	for (i=0;i<(gint)rtv_data->persona_array->len;i++)
	{
		info = NULL;
		info = g_array_index(rtv_data->persona_array,Persona_Info *,i);
		gtk_tree_store_append(store,&parent,NULL);
		gtk_tree_store_set(store,&parent,VARNAME_COL,g_strdup(info->persona),-1);
		for (j=0;j<(gint)g_list_length(info->rtv_list);j++)
		{
			gtk_tree_store_append(store,&iter,&parent);
			element = g_list_nth_data(info->rtv_list,j);
			int_name = g_hash_table_lookup(info->int_ext_hash,element);
			gtk_tree_store_set(store,&iter,VARNAME_COL,g_strdup(element),DATASOURCE_COL,g_strdup(int_name),-1);
		}
	}
}


gint sort(gconstpointer a, gconstpointer b)
{
	return g_strcasecmp((gchar *)a, (gchar *)b);
}

