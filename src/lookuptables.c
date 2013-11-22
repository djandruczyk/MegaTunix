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
  \file src/lookuptables.c
  \ingroup CoreMtx
  \brief Handles lookuptable related operations
  \author David Andruczyk
  */

#include <assert.h>
#include <configfile.h>
#include <debugging.h>
#include <getfiles.h>
#include <init.h>
#include <listmgmt.h>
#include <lookuptables.h>
#include <plugin.h>
#include <stdlib.h>
#include <stdio.h>
#include <timeout_handlers.h>
#include <widgetmgmt.h>

static gboolean ltc_visible = FALSE;
extern gconstpointer *global_data;

enum
{
	INTERNAL_NAME_COL,
	FILENAME_COL,
	VIEW_EDIT_COL,
	N_COLS
};

void editing_started(GtkCellRenderer *renderer, GtkCellEditable *editable, gchar *path, gpointer data);
/*!
  \brief get_table() gets a valid filehandle of the lookuptable from 
  get_file and passes it to load_table(void)
  \see load_table
  \see get_File
  \param table_name is the textual name of the table to use as the key
  to the lookuptables hashtable
  \param fname is the textual name of the filename to load
  \param user_data is unused
  */
G_MODULE_EXPORT void get_table(gpointer table_name, gpointer fname, gpointer user_data)
{
	gboolean status = FALSE;
	gchar * filename = NULL;
	gchar ** vector = NULL;
	gchar *pathstub = NULL;
	
	ENTER();
	vector = g_strsplit((gchar *)fname,".",2);

	pathstub = g_build_filename(LOOKUPTABLES_DATA_DIR,vector[0],NULL);
	filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,vector[1]);
	g_free(pathstub);
	g_strfreev(vector);

	if (filename)
	{
		status = load_table((gchar *)table_name,filename);
		g_free(filename);
	}
	if (!status)
	{
		MTXDBG(CRITICAL,_("FAILURE loading \"%s\" lookuptable, EXITING!!\n"),(gchar *)table_name);
		exit (-2);
	}

	EXIT();
	return;
}


/*!
  \brief load_table() physically handles loading the table datafrom disk, 
  populating and array and sotring a pointer to that array in the lookuptables
  hashtable referenced by the table_name passed
  \param table_name is the key to lookuptables hashtable
  \param filename is the filename to load table data from
  \returns TRUE on success, FALSE on failure
  */
G_MODULE_EXPORT gboolean load_table(gchar *table_name, gchar *filename)
{
	GIOStatus status;
	GIOChannel *iochannel;
	gboolean done = FALSE;
	GHashTable *lookuptables = NULL;
	gchar * str = NULL;
	gchar * tmp = NULL;
	gchar * end = NULL;
	GString *a_line; 
	LookupTable *lookuptable = NULL;
	gint tmparray[2048]; /* bad idea being static!!*/
	gchar ** vector = NULL;
	gint i = 0;

	ENTER();
	iochannel = g_io_channel_new_file(filename,"r", NULL);
	status = g_io_channel_seek_position(iochannel,0,G_SEEK_SET,NULL);
	if (status != G_IO_STATUS_NORMAL)
	{
		MTXDBG(CRITICAL,_("Error seeking to beginning of the file\n"));
	}
	while (!done)	
	{
		a_line = g_string_new("\0");
		status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
		if (status == G_IO_STATUS_EOF)
			done = TRUE;
		else
		{
			/*	str = g_strchug(g_strdup(a_line->str));*/
			str = g_strchug(a_line->str);
			if (g_str_has_prefix(str,"DB"))
			{
				str+=2; /* move 2 places in	*/
				end = g_strrstr(str,"T");
				tmp = g_strndup(str,end-str);
				tmparray[i]=atoi(tmp);
				g_free(tmp);
				i++;
			}
		}
		g_string_free(a_line,TRUE);
	}
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);

	vector = g_strsplit(filename,PSEP,-1);
	lookuptable = g_new0(LookupTable, 1);
	lookuptable->array = (gint *)g_memdup(&tmparray,i*sizeof(gint));
	lookuptable->filename = g_strdup(vector[g_strv_length(vector)-1]);
	g_strfreev(vector);
	lookuptables = (GHashTable *)DATA_GET(global_data,"lookuptables");
	if (!lookuptables)
	{
		lookuptables = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_lookuptable);
		DATA_SET_FULL(global_data,"lookuptables",lookuptables,g_hash_table_destroy);
	}
	g_hash_table_replace((GHashTable *)DATA_GET(global_data,"lookuptables"),g_strdup(table_name),lookuptable);
	/*g_hash_table_foreach(DATA_GET(global_data,"lookuptables"),dump_lookuptables,NULL);*/

	EXIT();
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
  \param object is the pointer to object.
  \param value is the value to be reverse looked up
  \returns the index closest to that data
  */
G_MODULE_EXPORT gint reverse_lookup(gconstpointer *object, gint value)
{
	gint i = 0;
	gint j = 0;
	gint closest_index = 0;
	gint min = 0;
	gint len = 0;
	gint weight[255];

	ENTER();
	gconstpointer *dep_obj = NULL;
	LookupTable *lookuptable = NULL;
	gint *array = NULL;
	gchar *table = NULL;
	gchar *alt_table = NULL;
	gboolean state = FALSE;
	static gboolean (*check_deps)(gconstpointer *);

	if (!check_deps)
		get_symbol("check_dependencies",(void **)&check_deps);
	table = (gchar *)DATA_GET(object,"lookuptable");
	alt_table = (gchar *)DATA_GET(object,"alt_lookuptable");
	dep_obj = (gconstpointer *)DATA_GET(object,"dep_object");
	if (dep_obj) 
	{
		if (check_deps)
			state = check_deps(dep_obj);
		else
			MTXDBG(CRITICAL,_("Could NOT locate \"check_dependencies\" function in any of the plugins, BUG!\n"));
	}
	if (state)
		lookuptable = (LookupTable *)g_hash_table_lookup((GHashTable *)DATA_GET(global_data,"lookuptables"),alt_table);	
	else
		lookuptable = (LookupTable *)g_hash_table_lookup((GHashTable *)DATA_GET(global_data,"lookuptables"),table);	

	array = lookuptable->array;
	len=255;
	for (i=0;i<len;i++)
		weight[i]=0;

	for (i=0;i<len;i++)
	{
		/*printf("counter is %i\n",i);*/
		if (array[i] == value)
		{
			/*printf("match at %i\n",i);*/
			j = i;
			while (array[j] == value)
			{
				/*printf("searching for dups to upp the weight\n");*/
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
			/*printf("weight[%i]= %i greater than %i\n",i,weight[i],min);*/
			min = weight[i];
			closest_index=i+(min/2);
		}
	}

	/*printf("closest index is %i\n",closest_index);*/

	EXIT();
	return closest_index;
}



/*!
  \brief reverse_lookup_obj() looks for the INDEX of this value in the specified
  lookuptable.  This does an interesting weighted search in an attempt to handle
  lookuptables that contain the same value in multiple places.  When it finds
  a match it begins counting for sequential matches,  if so it increases the 
  "weight" of that match at the startpoint.  Following the weighted search, 
  another iteration of the weight array to find the biggest one, and then
  choose the midpoint of that span. (i.e. if there are 11 sequential target
  values, we choose the middle one (6th).  This algorithm can STILL however
  be tricked by multiple SINGLE values. in that case it'll take the last one.
  \param object is the pointer to object.
  \param value is the value to be reverse looked up
  \returns the index closest to that data
  */
G_MODULE_EXPORT gint reverse_lookup_obj(GObject *object, gint value)
{
	gint i = 0;
	gint j = 0;
	gint closest_index = 0;
	gint min = 0;
	gint len = 0;
	gint weight[255];

	ENTER();
	gconstpointer *dep_obj = NULL;
	LookupTable *lookuptable = NULL;
	gint *array = NULL;
	gchar *table = NULL;
	gchar *alt_table = NULL;
	gboolean state = FALSE;
	static gboolean (*check_deps)(gconstpointer *);

	if (!check_deps)
		get_symbol("check_dependencies",(void **)&check_deps);

	table = (gchar *)OBJ_GET(object,"lookuptable");
	alt_table = (gchar *)OBJ_GET(object,"alt_lookuptable");
	dep_obj = (gconstpointer *)OBJ_GET(object,"dep_object");
	if (dep_obj) 
	{
		if (check_deps)
			state = check_deps(dep_obj);
		else
			MTXDBG(CRITICAL,_("Could NOT locate \"check_dependencies\" function in any of the plugins, BUG!\n"));
	}
	if (state)
		lookuptable = (LookupTable *)g_hash_table_lookup((GHashTable *)DATA_GET(global_data,"lookuptables"),alt_table);	
	else
		lookuptable = (LookupTable *)g_hash_table_lookup((GHashTable *)DATA_GET(global_data,"lookuptables"),table);	

	array = lookuptable->array;
	len=255;
	for (i=0;i<len;i++)
		weight[i]=0;

	for (i=0;i<len;i++)
	{
		/*printf("counter is %i\n",i);*/
		if (array[i] == value)
		{
			/*printf("match at %i\n",i);*/
			j = i;
			while (array[j] == value)
			{
				/*printf("searching for dups to upp the weight\n");*/
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
			/*printf("weight[%i]= %i greater than %i\n",i,weight[i],min);*/
			min = weight[i];
			closest_index=i+(min/2);
		}
	}

	/*printf("closest index is %i\n",closest_index);*/

	EXIT();
	return closest_index;
}


/*!
  \brief looks up the index of a table based on its value. NOTE: returns the 
  "midpoint" betwen matching values.
  \param table is the name of table to lookup the index given the value
  \param value is the value to lookup the index for
  \returns the index, or midpoint index if multiple values in sequence equal 
  the passed value.
  */
G_MODULE_EXPORT gint direct_reverse_lookup(gchar *table, gint value)
{
	gint i = 0;
	gint j = 0;
	gint closest_index = 0;
	gint min = 0;
	gint len = 0;
	gint weight[255];

	ENTER();
	LookupTable *lookuptable = NULL;
	gint *array = NULL;

	lookuptable = (LookupTable *)g_hash_table_lookup((GHashTable *)DATA_GET(global_data,"lookuptables"),table);	
	if (!lookuptable)
	{
		EXIT();
		return value;
	}
	array = lookuptable->array;

	len=255;
	for (i=0;i<len;i++)
		weight[i]=0;

	for (i=0;i<len;i++)
	{
		/*printf("counter is %i\n",i);*/
		if (array[i] == value)
		{
			/*printf("match at %i\n",i);*/
			j = i;
			while (array[j] == value)
			{
				/*printf("searching for dups to upp the weight\n");*/
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
			/*printf("weight[%i]= %i greater than %i\n",i,weight[i],min);*/
			min = weight[i];
			closest_index=i+(min/2);
		}
	}

	/*printf("closest index is %i\n",closest_index);*/

	EXIT();
	return closest_index;
}


/*!
  \brief lookup_data() returns the value represented by the lookuptable 
  associated with the passed object and offset
  \param object is the container of parameters we need to do the lookup
  \param offset is the offset into lookuptable
  \returns the value at that offset of the lookuptable
  */
G_MODULE_EXPORT gfloat lookup_data(gconstpointer *object, gint offset)
{
	static gboolean (*check_deps)(gconstpointer *);
	static GHashTable *lookuptable_hash = NULL;
	gconstpointer *dep_obj = NULL;
	LookupTable *lookuptable = NULL;
	gchar *table = NULL;
	gchar *alt_table = NULL;
	gboolean state = FALSE;

	ENTER();
	if (!lookuptable_hash)
		lookuptable_hash = (GHashTable *)DATA_GET(global_data,"lookuptables");
	if (!check_deps)
		get_symbol("check_dependencies",(void **)&check_deps);

	table = (gchar *)DATA_GET(object,"lookuptable");
	alt_table = (gchar *)DATA_GET(object,"alt_lookuptable");
	dep_obj = (gconstpointer *)DATA_GET(object,"dep_object");

	g_return_val_if_fail(check_deps,0.0);
	g_return_val_if_fail(lookuptable_hash,0.0);
	g_return_val_if_fail(table,0.0);

	/*
	   if (dep_obj)
	   printf("checking dependancy %s\n",DATA_GET(object,"internal_names"));
	   else
	   printf("no dependancy\n");
	 */

	if (dep_obj) 
	{
		if (check_deps)
			state = check_deps(dep_obj);
		else
		{
			MTXDBG(CRITICAL,_("Could NOT locate \"check_dependencies\" function in any of the plugins, BUG!\n"));
			EXIT();
			return 0.0;
		}
	}
	if (state)
	{
		/*printf("ALTERNATE\n");*/
		lookuptable = (LookupTable *)g_hash_table_lookup(lookuptable_hash,alt_table);	
	}
	else
	{
		/*printf("NORMAL\n");*/
		lookuptable = (LookupTable *)g_hash_table_lookup(lookuptable_hash,table);	
	}

	if (!lookuptable)
	{
		MTXDBG(CRITICAL,_("Lookuptable is NULL for control %s\n"),(gchar *) DATA_GET(object,"internal_names"));
		EXIT();
		return 0.0;
	}
	EXIT();
	return lookuptable->array[offset];
}


/*!
  \brief lookup_data_obj() returns the value represented by the lookuptable 
  associated with the passed object and offset
  \param object is the container of parameters we need to do the lookup
  \param offset is the offset into lookuptable
  \returns the value at that offset of the lookuptable
  */
G_MODULE_EXPORT gfloat lookup_data_obj(GObject *object, gint offset)
{
	gconstpointer *dep_obj = NULL;
	LookupTable *lookuptable = NULL;
	gchar *table = NULL;
	gchar *alt_table = NULL;
	gboolean state = FALSE;
	static gboolean (*check_deps)(gconstpointer *);

	ENTER();
	if (!check_deps)
		get_symbol("check_dependencies",(void **)&check_deps);

	table = (gchar *)OBJ_GET(object,"lookuptable");
	alt_table = (gchar *)OBJ_GET(object,"alt_lookuptable");
	dep_obj = (gconstpointer *)OBJ_GET(object,"dep_object");


	if (dep_obj) 
	{
		if (check_deps)
			state = check_deps(dep_obj);
		else
			MTXDBG(CRITICAL,_("Could NOT locate \"check_dependencies\" function in any of the plugins, BUG!\n"));
	}
	if (state)
	{
		/*printf("ALTERNATE\n");*/
		lookuptable = (LookupTable *)g_hash_table_lookup((GHashTable *)DATA_GET(global_data,"lookuptables"),alt_table);	
	}
	else
	{
		/*printf("NORMAL\n");*/
		lookuptable = (LookupTable *)g_hash_table_lookup((GHashTable *)DATA_GET(global_data,"lookuptables"),table);	
	}

	if (!lookuptable)
	{
		MTXDBG(CRITICAL,_("Lookuptable is NULL for control %s\n"),(gchar *) DATA_GET(object,"internal_names"));
		EXIT();
		return 0.0;
	}
	EXIT();
	return lookuptable->array[offset];
}



/*!
  \brief looks up the value given the index
  \param table is the name of table to look in
  \param offset is the index into this table..
  \returns, the value at the index
  */
G_MODULE_EXPORT gfloat direct_lookup_data(gchar *table, gint offset)
{
	LookupTable *lookuptable = NULL;

	ENTER();
	if (!table)
	{
		printf(_("FATAL_ERROR: direct_lookup_data, table parameter is null\n"));
		assert(table);
	}

	lookuptable = (LookupTable *)g_hash_table_lookup((GHashTable *)DATA_GET(global_data,"lookuptables"),table);	
	if (!lookuptable)
	{
		printf(_("FATAL_ERROR: direct_lookup_data, table \"%s\" is null\n"),table);
		EXIT();
		return offset;
	}
	if (!lookuptable->array)
	{
		printf(_("FATAL_ERROR: direct_lookup_data, %s->array is null\n"),table);
		EXIT();
		return offset;
	}
	EXIT();
	return lookuptable->array[offset];
}


/*!
  \brief Creates the lookuptables configurator window (MS1 FW's only)
  \param widget is unused
  \param data is unused
  \returns TRUE on success
  */
G_MODULE_EXPORT gboolean lookuptables_configurator(GtkWidget *widget, gpointer data)
{
	static gboolean ltc_created = FALSE;
	static GtkWidget * lookuptables_config_window = NULL;
	GtkListStore *store = NULL;
	GtkTreeStore *combostore = NULL;
	GtkTreeIter iter;
	GtkTreeIter per_iter;
	GtkTreeIter sys_iter;
	GtkCellRenderer *renderer = NULL;
	GtkTreeViewColumn *column = NULL;
	GtkWidget * vbox = NULL;
	GtkWidget * tree = NULL;
	GtkWidget * frame = NULL;
	GtkWidget * label = NULL;
	ListElement *element = NULL;
	ConfigFile *cfgfile = NULL;
	GArray *classes = NULL;
	GList *p_list = NULL;
	GList *s_list = NULL;
	gchar * tmpbuf = NULL;
	gchar ** vector = NULL;
	gchar ** tmpvector = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if ((ltc_created) && (ltc_visible))
	{
		EXIT();
		return TRUE;
	}
	if ((ltc_created) && (!ltc_visible))
	{
		gtk_widget_show_all(lookuptables_config_window);
		EXIT();
		return TRUE;
	}
	else	/* i.e.  NOT created,  build it */
	{
		lookuptables_config_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(lookuptables_config_window),_("MegaTunix LookupTables"));
		gtk_window_set_default_size(GTK_WINDOW(lookuptables_config_window),300,200);
		vbox = gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(lookuptables_config_window),vbox);
		gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
		g_signal_connect(G_OBJECT(lookuptables_config_window),"delete_event", G_CALLBACK(lookuptables_configurator_hide),NULL);

		ltc_created = TRUE;
		ltc_visible = TRUE;
		frame = gtk_frame_new("MegaTunix LookupTables");
		gtk_box_pack_start (GTK_BOX(vbox),frame,TRUE,TRUE,5);
		vbox = gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(frame),vbox);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),_(" Click on the <i>LookupTable Filename</i> and press <b><u> Enter </u></b> to change "));
		gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,5);

		store = gtk_list_store_new(N_COLS,	/* total cols */
				G_TYPE_STRING, /* int name */
				G_TYPE_STRING, /* filename  combo*/
				G_TYPE_BOOLEAN,/* View/Edit */
				G_TYPE_BOOLEAN); /* change */

		combostore = gtk_tree_store_new(1,G_TYPE_STRING);/* lookuptable filename */

		gtk_tree_store_append(combostore,&per_iter,NULL);
		gtk_tree_store_append(combostore,&sys_iter,NULL);
		gtk_tree_store_set(combostore,&per_iter,
				0,"Personal", -1);
		gtk_tree_store_set(combostore,&sys_iter,
				0,"System", -1);
		vector = get_files((const gchar *)DATA_GET(global_data,"project_name"),LOOKUPTABLES_DATA_DIR,"inc",&classes);
		for (guint i=0;i<g_strv_length(vector);i++)
		{
			tmpvector = g_strsplit(vector[i],PSEP,-1);
			if (g_array_index(classes,FileClass,i) == PERSONAL)
			{
				element = (ListElement *)g_new0(ListElement, 1);
				element->name = g_strdup(tmpvector[g_strv_length(tmpvector)-1]);
				p_list = g_list_append(p_list,element);
			}
			if (g_array_index(classes,FileClass,i) == SYSTEM)
			{
				element = (ListElement *)g_new0(ListElement, 1);
				element->name = g_strdup(tmpvector[g_strv_length(tmpvector)-1]);
				s_list = g_list_append(s_list,element);
			}
			g_strfreev(tmpvector);
		}
		g_strfreev(vector);
		g_array_free(classes,TRUE);
		p_list = g_list_sort(p_list,list_sort);
		s_list = g_list_sort(s_list,list_sort);
		for (guint i=0;i<g_list_length(p_list);i++)
		{
			gtk_tree_store_append(combostore,&iter,&per_iter);
			element = (ListElement *)g_list_nth_data(p_list,i);
			gtk_tree_store_set(combostore,&iter,
					0,element->name,
					-1);
		}
		for (guint i=0;i<g_list_length(s_list);i++)
		{
			gtk_tree_store_append(combostore,&iter,&sys_iter);
			element = (ListElement *)g_list_nth_data(s_list,i);
			gtk_tree_store_set(combostore,&iter,
					0,element->name,
					-1);
		}
		g_list_foreach(p_list,free_element,NULL);
		g_list_foreach(s_list,free_element,NULL);
		g_list_free(p_list);
		g_list_free(s_list);

		cfgfile = cfg_open_file(firmware->profile_filename);
		if (!cfgfile)
		{
			EXIT();
			return FALSE;
		}
		cfg_read_string(cfgfile,"lookuptables","tables",&tmpbuf);
		vector = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
		for (guint i=0;i<g_strv_length(vector);i++)
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
		gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree),TRUE);
		gtk_box_pack_start(GTK_BOX(vbox),tree,TRUE,TRUE,0);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes("Internal Table Name",renderer,"text",INTERNAL_NAME_COL,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree),column);

		renderer = gtk_cell_renderer_combo_new();
		g_object_set(G_OBJECT(renderer),"has-entry",FALSE,"editable",TRUE,"model",combostore,"text-column",0,"style",PANGO_STYLE_ITALIC,NULL);
		g_signal_connect(G_OBJECT(renderer),"changed", G_CALLBACK(lookuptable_changed),store);
//		g_signal_connect(G_OBJECT(renderer),"editing-started", G_CALLBACK(editing_started),store);
		column = gtk_tree_view_column_new_with_attributes("LookupTable Filename",renderer,"text",FILENAME_COL,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree),column);

		/*		renderer = gtk_cell_renderer_toggle_new();
				column = gtk_tree_view_column_new_with_attributes("View/Edit",renderer,"active",VIEW_EDIT_COL,NULL);
				gtk_tree_view_append_column(GTK_TREE_VIEW(tree),column);
		 */
		gtk_window_set_transient_for(GTK_WINDOW(lookuptables_config_window),GTK_WINDOW(lookup_widget("main_window")));
		gtk_widget_show_all (lookuptables_config_window);
		gtk_tree_view_columns_autosize( GTK_TREE_VIEW(tree));
		gtk_tree_view_set_grid_lines( GTK_TREE_VIEW(tree),GTK_TREE_VIEW_GRID_LINES_BOTH);
//		g_signal_connect(G_OBJECT(tree),"row-activated", G_CALLBACK(row_activated),NULL);
	}
	EXIT();
	return TRUE;
}


/*!
  \brief hides the lookuptables config window
  \param widget is the pointer to the lookuptables config window
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean lookuptables_configurator_hide(GtkWidget *widget, gpointer data)
{
	ENTER();
	gtk_widget_hide(widget);
	ltc_visible = FALSE;
	EXIT();
	return TRUE;
}


/*!
  \brief Loads up the new lookuptable
  \param renderer is  the cell renderer of the cell selected
  \param path is  the treepath in text
  \param new_text is  the personal/system column name
  \param data is unused
  \returns TRUE on success, FALSE otherwise
   */
//G_MODULE_EXPORT gboolean lookuptable_changed(GtkCellRendererCombo *renderer, gchar *path, gchar * new_text, gpointer data)
G_MODULE_EXPORT gboolean lookuptable_changed(GtkCellRendererCombo *renderer, gchar *path, GtkTreeIter *new_iter, gpointer data)
{
	GtkTreeModel *combostore = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = (GtkTreeModel *)data;
	ConfigFile *cfgfile = NULL;
	gchar * new_text = NULL;
	gchar * int_name = NULL;
	gchar * old = NULL;
	gchar * new_name = NULL;
	gchar ** vector = NULL;
	const gchar * project = NULL;
	gboolean restart_tickler = FALSE;
	GAsyncQueue *io_data_queue = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	io_data_queue = (GAsyncQueue *)DATA_GET(global_data,"io_data_queue");

	/* Get combo box model so we can set the combo to this new value */
	g_object_get(G_OBJECT(renderer),"model",&combostore,NULL);
	gtk_tree_model_get(combostore,new_iter,0,&new_text,-1);

	/* Get combo box model so we can set the combo to this new value */
	gtk_tree_model_get_iter_from_string(model,&iter,path);
	gtk_tree_model_get(model,&iter,INTERNAL_NAME_COL,&int_name,FILENAME_COL,&old,-1);
	//printf("New text is %s, int name associated %s, filename %s\n",new_text,int_name,old);
	if (g_ascii_strcasecmp(old,new_text) == 0) /* If no change, return */
	{
		g_free(int_name);
		g_free(old);
		EXIT();
		return TRUE;
	}

	if (g_ascii_strcasecmp(new_text,"Personal") == 0)
	{
		g_free(int_name);
		g_free(old);
		EXIT();
		return TRUE;
	}
	if (g_ascii_strcasecmp(new_text,"System") == 0)
	{
		g_free(int_name);
		g_free(old);
		EXIT();
		return TRUE;
	}
	if (DATA_GET(global_data,"realtime_id"))
	{
		gint count = 0;
		restart_tickler = TRUE;
		stop_tickler(RTV_TICKLER);
		while ((g_async_queue_length(io_data_queue) > 0) && (count < 30))
		{
			MTXDBG(CRITICAL,_("Draining I/O Queue, current length %i\n"),g_async_queue_length(io_data_queue));
			count++;
		}

	}
	get_table(int_name,new_text,NULL); /* Load the new one in it's place */
	gtk_list_store_set(GTK_LIST_STORE(model),&iter, FILENAME_COL, new_text,-1);
	if (restart_tickler)
		start_tickler(RTV_TICKLER);

	cfgfile = cfg_open_file(firmware->profile_filename);
	if (!cfgfile)
	{
		g_free(int_name);
		g_free(old);
		EXIT();
		return FALSE;
	}
	g_hash_table_foreach((GHashTable *)DATA_GET(global_data,"lookuptables"),update_lt_config,cfgfile);
	if (g_strrstr(firmware->profile_filename,"mtx"))
		cfg_write_file(cfgfile, firmware->profile_filename);
	else
	{
		project = (const gchar *)DATA_GET(global_data,"project_name");
		vector = g_strsplit(firmware->profile_filename,PSEP,-1);
		if (!project)
			project = DEFAULT_PROJECT;
		new_name = g_build_filename(HOME(),"mtx",project,INTERROGATOR_DATA_DIR,"Profiles",vector[g_strv_length(vector)-2],vector[g_strv_length(vector)-1],NULL);
		g_strfreev(vector);
		cfg_write_file(cfgfile, new_name);
		g_free(firmware->profile_filename);
		firmware->profile_filename=g_strdup(new_name);
		g_free(new_name);
	}
	cfg_free(cfgfile);

	/*printf("internal name %s, old table %s, new table %s\n",int_name,old,new_text);*/
	g_free(int_name);
	g_free(old);
	EXIT();
	return TRUE;
}


/*!
  \brief updates the interrogation profile with the new default lookuptable
  \param key is the lookuptable name
  \param value is the pointer to LookupTable object
  \param data is the pointer to ConfigFile structure
  */
G_MODULE_EXPORT void update_lt_config(gpointer key, gpointer value, gpointer data)
{
	ENTER();
	ConfigFile *cfgfile = (ConfigFile *)data;
	LookupTable *lookuptable = (LookupTable *)value;
	/*printf("updating %s, %s, %s\n",cfgfile->filename,(gchar *)key, lookuptable->filename);*/
	cfg_write_string(cfgfile,"lookuptables",(gchar *)key,lookuptable->filename);
	EXIT();
	return;
}


/*!
  \brief dump_hash() is a debug function to dump the contents of the str_2_enum
  hashtable to check for errors or problems
  \param key is the key name in the hashtable
  \param value is the value (enumeration value) in the hashtable
  \param user_data is unused...
  */
G_MODULE_EXPORT void dump_lookuptables(gpointer key, gpointer value, gpointer user_data)
{
	LookupTable *table;
	ENTER();
	table = (LookupTable *)value;
	printf(_(": dump_hash()\n\tKey %s, Value %p, %s\n"),(gchar *)key, value,table->filename);
	EXIT();
	return;
}


/*!
  \brief row_activated() is a handler to expand hte combo box on double click of a row
  \param treeview is the treeview widget
  \param path is the path within the tree they clicked on
  \param column is hte column they clicked on
  \param user_data is unused...
  */
G_MODULE_EXPORT void row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data)
{
	ENTER();
	printf("A row was activated, should popdown the combo if possible...\n");
	EXIT();
	return;
}


G_MODULE_EXPORT void started(GtkCellRenderer *renderer, GtkCellEditable *editable, gchar *path, gpointer data)
{
	ENTER();
	printf("combo editing-started signal!\n");
	if (GTK_IS_ENTRY(editable))
		printf("Its an entry\n");
	else if (GTK_IS_COMBO_BOX(editable))
	{
		printf("Its a combo box\n");
		g_signal_emit_by_name(editable,"popup");
	}
	else
		printf("Not sure what widget this is...\n");
	EXIT();
	return;
}

