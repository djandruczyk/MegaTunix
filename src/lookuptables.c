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
#include <configfile.h>
#include <debugging.h>
#include <defines.h>
#include <dep_processor.h>
#include <enums.h>
#include <getfiles.h>
#include <lookuptables.h>
#include <stdlib.h>
#include <structures.h>
#include <timeout_handlers.h>

static gboolean ltc_visible = FALSE;

GHashTable *lookuptables = NULL;
extern gint dbg_lvl;

enum
{
	INTERNAL_NAME_COL,
	FILENAME_COL,
	VIEW_EDIT_COL,
	N_COLS,
};

/*!
 \brief get_table() gets a valid filehandle of the lookuptable from 
 get_file and passes it to load_table()
 \see load_table
 \see get_File
 \param table_name (gpointer) textual name of the table to use as the key
 to the lookuptables hashtable
 \param fname (gpointer) textual name of the filename to load
 \param user_data (gpointer) unused
 */
void get_table(gpointer table_name, gpointer fname, gpointer user_data)
{
	gboolean status = FALSE;
	gchar * filename = NULL;
	gchar ** vector = NULL;
	
	vector = g_strsplit(fname,".",2);

	filename = get_file(g_strconcat(LOOKUPTABLES_DATA_DIR,PSEP,vector[0],NULL),g_strdup(vector[1]));
	g_strfreev(vector);

	if (filename)
	{
		status = load_table((gchar *)table_name,filename);
		g_free(filename);
	}
	if (!status)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_lookuptables()\n\tFAILURE loading \"%s\" lookuptable, EXITING!!\n",(gchar *)table_name));
		exit (-2);
	}

}


/*!
 \brief load_table() physically handles load ingthe table datafrom disk, 
 populating and array and sotring a pointer to that array in the lookuptables
 hashtable referenced by the table_name passed
 \param table_name (gchar *) key to lookuptables hashtable
 \param filename (gchar *) filename to load table data from
 \returns TRUE on success, FALSE on failure
 */
gboolean load_table(gchar *table_name, gchar *filename)
{
	GIOStatus status;
	GIOChannel *iochannel;
	gboolean go = TRUE;
	gchar * str = NULL;
	gchar * tmp = NULL;
	gchar * end = NULL;
	GString *a_line; 
	LookupTable *lookuptable = NULL;
	gint tmparray[2048]; // bad idea being static!!
	gchar ** vector = NULL;
	gint i = 0;

	iochannel = g_io_channel_new_file(filename,"r", NULL);
	status = g_io_channel_seek_position(iochannel,0,G_SEEK_SET,NULL);
	if (status != G_IO_STATUS_NORMAL)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": load_lookuptables()\n\tError seeking to beginning of the file\n"));
	}
	while (go)	
	{
		a_line = g_string_new("\0");
		status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
		if (status == G_IO_STATUS_EOF)
			go = FALSE;
		else
		{
			str = g_strchug(g_strdup(a_line->str));
			if (g_str_has_prefix(str,"DB"))
			{
				str+=2; // move 2 places in	
				end = g_strrstr(str,"T");
				tmp = g_strndup(str,end-str);
				tmparray[i]=atoi(tmp);
				g_free(tmp);
				i++;
			}
			//g_free(str);
		}
		g_string_free(a_line,TRUE);
	}
	g_io_channel_shutdown(iochannel,FALSE,NULL);

	vector = g_strsplit(filename,PSEP,-1);
	lookuptable = g_new0(LookupTable, 1);
	lookuptable->array = g_memdup(&tmparray,i*sizeof(gint));
	lookuptable->filename = g_strdup(vector[g_strv_length(vector)-1]);
	g_strfreev(vector);
	if (!lookuptables)
		lookuptables = g_hash_table_new(g_str_hash,g_str_equal);
	g_hash_table_insert(lookuptables,table_name,lookuptable);

	return TRUE;
}


/*!
 \brief reverse_lookup() looks for the INDEX of this value in the specified
 lookuptable.  This does an interesting weighted search in an attempt to handle
 lookuptables that contain the same value in multiple places.  When it finds
 a match it begins counting for sequential matches,  if so it increases the 
 "weight" of that match at the startpoint.  Following the weighted search, 
 another iteration of the weight array to find the biggest one, and then
 choose the midpoint of that span. (i.e. if there are 11 sequential target
 values, we choose the middle one (6th).  This algorithm can STILL however
 be tricked by multiple SINGLE values. in that case it'll take the last one.
 \param object (GObject *) pointer to object.
 \param value (gint ) value to be reverse looked up
 \returns the index closest to that data
 */
gint reverse_lookup(GObject *object, gint value)
{
	gint i = 0;
	gint j = 0;
	gint closest_index = 0;
	gint min = 0;
	gint len = 0;
	gint weight[255];

	extern GHashTable *lookuptables;
	GObject *dep_obj = NULL;
	LookupTable *lookuptable = NULL;
	gint *array = NULL;
	gchar *table = NULL;
	gchar *alt_table = NULL;
	gboolean state = FALSE;

	table = (gchar *)g_object_get_data(object,"lookuptable");
	alt_table = (gchar *)g_object_get_data(object,"alt_lookuptable");
	dep_obj = (GObject *)g_object_get_data(object,"dep_object");
	if (dep_obj)
		state = check_dependancies(dep_obj);
	if (state)
		lookuptable = (LookupTable *)g_hash_table_lookup(lookuptables,alt_table);	
	else
		lookuptable = (LookupTable *)g_hash_table_lookup(lookuptables,table);	

	array = lookuptable->array;
	len=255;
	for (i=0;i<len;i++)
		weight[i]=0;

	for (i=0;i<len;i++)
	{
		//		printf("counter is %i\n",i);
		if (array[i] == value)
		{
			//			printf("match at %i\n",i);
			j = i;
			while (array[j] == value)
			{
				//				printf("searching for dups to upp the weight\n");
				weight[i]++;
				if (j+1 == len)
					break;
				else
					j++;
			}
			i=j;
		}
	}
	for (i=0;i<len;i++)
	{
		if (weight[i] > min)
		{
			//			printf("weight[%i]= %i greater than %i\n",i,weight[i],min);
			min = weight[i];
			closest_index=i+(min/2);
		}
	}

	//	printf("closest index is %i\n",closest_index);

	return closest_index;
}

gint direct_reverse_lookup(gchar *table, gint value)
{
	gint i = 0;
	gint j = 0;
	gint closest_index = 0;
	gint min = 0;
	gint len = 0;
	gint weight[255];

	extern GHashTable *lookuptables;
	LookupTable *lookuptable = NULL;
	gint *array = NULL;

	lookuptable = (LookupTable *)g_hash_table_lookup(lookuptables,table);	
	if (!lookuptable)
		return value;
	array = lookuptable->array;

	len=255;
	for (i=0;i<len;i++)
		weight[i]=0;

	for (i=0;i<len;i++)
	{
		//		printf("counter is %i\n",i);
		if (array[i] == value)
		{
			//			printf("match at %i\n",i);
			j = i;
			while (array[j] == value)
			{
				//				printf("searching for dups to upp the weight\n");
				weight[i]++;
				if (j+1 == len)
					break;
				else
					j++;
			}
			i=j;
		}
	}
	for (i=0;i<len;i++)
	{
		if (weight[i] > min)
		{
			//			printf("weight[%i]= %i greater than %i\n",i,weight[i],min);
			min = weight[i];
			closest_index=i+(min/2);
		}
	}

	//	printf("closest index is %i\n",closest_index);

	return closest_index;
}


/*!
 \brief lookup_data() returns the value represented by the lookuptable 
 associated with the passed object and offset
 \param object (GObject *) container of parameters we need to do the lookup
 \param offset (gint) offset into lookuptable
 \returns the value at that offset of the lokuptable
 */
gfloat lookup_data(GObject *object, gint offset)
{
	extern GHashTable *lookuptables;
	GObject *dep_obj = NULL;
	LookupTable *lookuptable = NULL;
	gchar *table = NULL;
	gchar *alt_table = NULL;
	gboolean state = FALSE;

	table = (gchar *)g_object_get_data(object,"lookuptable");
	alt_table = (gchar *)g_object_get_data(object,"alt_lookuptable");
	dep_obj = (GObject *)g_object_get_data(object,"dep_object");
	/*
	   if (GTK_IS_OBJECT(dep_obj))
	   printf("checking dependancy\n");
	   else
	   printf("no dependancy\n");
	   */
	if (dep_obj)
	{
		state = check_dependancies(dep_obj);
	}
	if (state)
	{
		//printf("ALTERNATE\n");
		lookuptable = (LookupTable *)g_hash_table_lookup(lookuptables,alt_table);	
	}
	else
	{
		//printf("NORMAL\n");
		lookuptable = (LookupTable *)g_hash_table_lookup(lookuptables,table);	
	}

	if (!lookuptable)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": lookup_data()\n\t Lookuptable is NULL for control %s\n",(gchar *) g_object_get_data(object,"internal_name")));
		return 0.0;
	}
	return lookuptable->array[offset];
}



gfloat direct_lookup_data(gchar *table, gint offset)
{
	extern GHashTable *lookuptables;
	LookupTable *lookuptable = NULL;

	lookuptable = (LookupTable *)g_hash_table_lookup(lookuptables,table);	

	if (!lookuptable)
	{
		return offset;
	}
	return lookuptable->array[offset];
}


gboolean lookuptables_configurator(GtkWidget *widget, gpointer data)
{
	static gboolean ltc_created = FALSE;
	static GtkWidget * lookuptables_config_window = NULL;
	extern Firmware_Details *firmware;
	GtkListStore *store = NULL;
	GtkListStore *combostore = NULL;
	GtkTreeIter iter;
	GtkCellRenderer *renderer = NULL;
	GtkTreeViewColumn *column = NULL;
	GtkWidget * vbox = NULL;
	GtkWidget * label = NULL;
	GtkWidget * tree = NULL;
	ConfigFile *cfgfile = NULL;
	gint i = 0;
	gchar * tmpbuf = NULL;
	gchar ** vector = NULL;
	gchar ** tmpvector = NULL;

	if ((ltc_created) && (ltc_visible))
		return TRUE;
	if ((ltc_created) && (!ltc_visible))
	{
		gtk_widget_show_all(lookuptables_config_window);
		return TRUE;
	}
	else	/* i.e.  NOT created,  build it */
	{
		lookuptables_config_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(lookuptables_config_window),"MegaTunix LookupTables");
		vbox = gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(lookuptables_config_window),vbox);
		gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
		g_signal_connect(G_OBJECT(lookuptables_config_window),"delete_event", G_CALLBACK(lookuptables_configurator_hide),NULL);

		ltc_created = TRUE;
		ltc_visible = TRUE;
		label = gtk_label_new("lookuptables_configurator");
		gtk_box_pack_start (GTK_BOX(vbox),label,TRUE,TRUE,0);

		store = gtk_list_store_new(N_COLS,	/* total cols */
				G_TYPE_STRING, /* int name */
				G_TYPE_STRING, /* filename  combo*/
				G_TYPE_BOOLEAN,/* View/Edit */
				G_TYPE_BOOLEAN); /* change */

		combostore = gtk_list_store_new(1,G_TYPE_STRING);
		vector = get_files(g_strdup(LOOKUPTABLES_DATA_DIR),g_strdup("inc"));
		for (i=0;i<g_strv_length(vector);i++)
		{
			tmpvector = g_strsplit(vector[i],PSEP,-1);
			gtk_list_store_append(combostore,&iter);
			gtk_list_store_set(combostore,&iter,
					0,tmpvector[g_strv_length(tmpvector)-1],
					-1);
			g_strfreev(tmpvector);
		}
		g_strfreev(vector);

		cfgfile = cfg_open_file(firmware->profile_filename);
		if (!cfgfile)
			return FALSE;
		cfg_read_string(cfgfile,"lookuptables","tables",&tmpbuf);
		vector = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
		for (i=0;i<g_strv_length(vector);i++)
		{
			cfg_read_string(cfgfile,"lookuptables",vector[i],&tmpbuf);
			gtk_list_store_append(store,&iter);
			gtk_list_store_set(store,&iter,
					INTERNAL_NAME_COL,vector[i],
					FILENAME_COL,tmpbuf,
					VIEW_EDIT_COL,FALSE,
					-1);
			g_free(tmpbuf);
		}
		g_strfreev(vector);

		tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
		gtk_box_pack_start(GTK_BOX(vbox),tree,TRUE,TRUE,0);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes("Internal Name",renderer,"text",INTERNAL_NAME_COL,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree),column);
		renderer = gtk_cell_renderer_combo_new();
		g_object_set(G_OBJECT(renderer),"editable",TRUE,"model",combostore,"text-column",0,NULL);
		g_signal_connect(G_OBJECT(renderer),"edited", G_CALLBACK(lookuptable_change),store);
		column = gtk_tree_view_column_new_with_attributes("Table Filename",renderer,"text",FILENAME_COL,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree),column);
		renderer = gtk_cell_renderer_toggle_new();
		column = gtk_tree_view_column_new_with_attributes("View/Edit",renderer,"active",VIEW_EDIT_COL,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree),column);


		gtk_widget_show_all (lookuptables_config_window);
		return TRUE;
	}

}

gboolean lookuptables_configurator_hide(GtkWidget *widget, gpointer data)
{
	gtk_widget_hide(widget);
	ltc_visible = FALSE;
	return TRUE;
}


gboolean lookuptable_change(GtkCellRenderer *renderer, gchar *path, gchar * new_text, gpointer data)
{
	GtkListStore *store = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = data;
	ConfigFile *cfgfile = NULL;
	gchar * int_name = NULL;
	gchar * old = NULL;
	gchar * new_name = NULL;
	gchar ** vector = NULL;
	gboolean restart_tickler = FALSE;
	extern gint realtime_id;
	extern GHashTable *lookuptables;
	extern GAsyncQueue *io_queue;
	extern Firmware_Details *firmware;
	gint count = 0;
	LookupTable *lookuptable = NULL;

	/* Get combo box model so we can set the combo to this new value */
	g_object_get(G_OBJECT(renderer),"model",&store,NULL);
	gtk_tree_model_get_iter_from_string(model,&iter,path);
	gtk_tree_model_get(model,&iter,INTERNAL_NAME_COL,&int_name,FILENAME_COL,&old,-1);
	if (g_strcasecmp(old,new_text) == 0) /* If no change, return */
		return TRUE;
	
	if (realtime_id)
	{
		restart_tickler = TRUE;
		stop_tickler(RTV_TICKLER);
		count = 0;
		while ((g_async_queue_length(io_queue) > 0) && (count < 30))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": LEAVE() draining I/O Queue,  current length %i\n",g_async_queue_length(io_queue)));
			while (gtk_events_pending())
				gtk_main_iteration();
			count++;
		}

	}
	lookuptable = (LookupTable *)g_hash_table_lookup(lookuptables,int_name);
	if (!lookuptable)
		printf("BAD things man,  gonna crash!\n");
	g_free(lookuptable->array); /* Free the old one */
	g_free(lookuptable->filename); /* Free the old one */
	g_free(lookuptable); /* Free the old one */
	get_table(int_name,new_text,NULL); /* Load the new one in it's place */
	gtk_list_store_set(GTK_LIST_STORE(model),&iter, FILENAME_COL, new_text,-1);
	if (restart_tickler)
		start_tickler(RTV_TICKLER);

		cfgfile = cfg_open_file(firmware->profile_filename);
		if (!cfgfile)
			return FALSE;
		g_hash_table_foreach(lookuptables,update_lt_config,cfgfile);
	if (g_strrstr(firmware->profile_filename,".MegaTunix"))
		cfg_write_file(cfgfile, firmware->profile_filename);
	else
	{
		vector = g_strsplit(firmware->profile_filename,PSEP,-1);
		new_name = g_build_filename(HOME(),".MegaTunix",INTERROGATOR_DATA_DIR,"Profiles",vector[g_strv_length(vector)-1],NULL);
		g_strfreev(vector);
		cfg_write_file(cfgfile, new_name);
		g_free(firmware->profile_filename);
		firmware->profile_filename=g_strdup(new_name);
		g_free(new_name);
	}
	cfg_free(cfgfile);
	g_free(cfgfile);
		
	//printf("internal name %s, old table %s, new table %s\n",int_name,old,new_text);
	return TRUE;

}

void update_lt_config(gpointer key, gpointer value, gpointer data)
{
	ConfigFile *cfgfile = data;
	LookupTable *lookuptable = value;
//	printf("updating %s, %s, %s\n",cfgfile->filename,(gchar *)key, lookuptable->filename);
	cfg_write_string(cfgfile,"lookuptables",(gchar *)key,lookuptable->filename);

}
