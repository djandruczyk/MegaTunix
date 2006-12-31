#include <configfile.h>
#include <defines.h>
#include <dirent.h>
#include <getfiles.h>
#include <gauge.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <rtv_parser.h>
#include <string.h>

GtkListStore *store = NULL;
void retrieve_rt_vars(void)
{
	gchar **files = NULL;
	struct Rtv_Data *rtv_data = NULL;
	gint i = 0;
//	printf("retrieve rt_vars from mtx realtime maps\n");
	files = get_files(g_strconcat(REALTIME_MAPS_DATA_DIR,PSEP,NULL),g_strdup("rtv_map"));
	if (!files)
		return;
	while(files[i])
		i++;
	rtv_data = g_new0(struct Rtv_Data, 1);
	rtv_data->rtv_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	rtv_data->int_ext_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	rtv_data->rtv_list = NULL;
	rtv_data->total_files = i;
	load_rtvars(files,rtv_data);

}

void load_rtvars(gchar **files, struct Rtv_Data *rtv_data)
{
	GtkTreeIter iter;
	ConfigFile *cfgfile;
	gint total = 0;
	gpointer orig = NULL;
	gpointer value = NULL;
	gchar * tmpbuf = NULL;
	gchar *vname = NULL;
	gchar *iname = NULL;
	gchar *element = NULL;
	gint icount = 0;
	gint len = 0;
	gint tmpi = 0;
	gint i = 0;
	gint j = 0;
	while (files[i])
	{
		cfgfile = cfg_open_file(files[i]);
		if (cfgfile)
		{
			cfg_read_int(cfgfile,"realtime_map", "derived_total",&total);
			for (j=0;j<total;j++)
			{
				tmpbuf = g_strdup_printf("derived_%i",j);
				cfg_read_string(cfgfile,tmpbuf,"dlog_gui_name",&vname);
				cfg_read_string(cfgfile,tmpbuf,"internal_name",&iname);
				if (g_hash_table_lookup_extended(rtv_data->rtv_hash,vname,&orig,&value))
				{
					tmpi = (gint)value + 1;
					//printf("Value on pre-existing var %s is %i\n",(gchar *)orig,(gint)value);
					g_hash_table_replace(rtv_data->rtv_hash,g_strdup(vname),GINT_TO_POINTER(tmpi));
				}
				else
				{
					//printf("inserting var %s with value %i\n",vname,1);
					g_hash_table_insert(rtv_data->rtv_hash,g_strdup(vname),GINT_TO_POINTER(1));
					g_hash_table_insert(rtv_data->int_ext_hash,g_strdup(vname),g_strdup(iname));
					rtv_data->rtv_list = g_list_prepend(rtv_data->rtv_list,g_strdup(vname));
				}
				g_free(tmpbuf);
				g_free(vname);
				g_free(iname);
			}
		}
		cfg_free(cfgfile);
		g_free(cfgfile);
		i++;
	}

	rtv_data->rtv_list = g_list_sort(rtv_data->rtv_list,sort);
	store = gtk_list_store_new(NUM_COLS,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
	len = g_list_length(rtv_data->rtv_list);
//	printf("list length is %i\n",len);
	for (i=0;i<len;i++)
	{
		element = g_list_nth_data(rtv_data->rtv_list,i);
//		printf("element %s\n",element);
		iname = g_hash_table_lookup(rtv_data->int_ext_hash,element);
		icount = (gint)g_hash_table_lookup(rtv_data->rtv_hash,element);
//		printf("int name %s\n",iname);
		gtk_list_store_append(store,&iter);
		if (icount == rtv_data->total_files)
			gtk_list_store_set(store,&iter,VARNAME_COL,g_strdup(element),TYPE_COL,"  (common)",DATASOURCE_COL,g_strdup(iname),-1);
		else
			gtk_list_store_set(store,&iter,VARNAME_COL,g_strdup(element),TYPE_COL,"  (FW Specific)", DATASOURCE_COL,g_strdup(iname),-1);
	}
}

gint sort(gconstpointer a, gconstpointer b)
{
	return strcmp((gchar *)a, (gchar *)b);
}

