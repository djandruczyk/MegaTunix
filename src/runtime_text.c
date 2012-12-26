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

/*! @file src/runtime_text.c
  \ingroup CoreMtx
  \brief Handles the Runtime Text floating window
  \author David Andruczyk
  */

#include <args.h>
#include <api-versions.h>
#include <debugging.h>
#include <getfiles.h>
#include <glade/glade-xml.h>
#include <init.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <runtime_text.h>
#include <stdio.h>
#include <stdlib.h>
#include <widgetmgmt.h>
#include <xmlbase.h>

extern gconstpointer *global_data;

/*!
  \brief load_rt_text_pf() is called to load up the runtime text configurations
  from the file specified in the firmware's interrogation profile, and populate
  a new window with the runtiem vars text value box.
  */
G_MODULE_EXPORT void load_rt_text_pf(void)
{
	GtkWidget *treeview = NULL; 
	GtkWidget *window = NULL;
	GtkWidget *parent = NULL;
	gint x = 0;
	gint y = 0;
	GtkListStore *store = NULL;
	gchar *filename = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	gboolean xml_result = FALSE;
	CmdLineArgs *args = (CmdLineArgs *)DATA_GET(global_data,"args");
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	Firmware_Details *firmware = NULL;
	gchar *pathstub = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!(DATA_GET(global_data,"interrogated")))
	{
		EXIT();
		return;
	}
	if (!firmware->rtt_map_file)
	{
		MTXDBG(CRITICAL,_("Firmware->rtt_map_file is UNDEFINED, exiting runtime text window creation routine!!!!\n"));
		EXIT();
		return;
	}

	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!main_xml) || (DATA_GET(global_data,"leaving")))
	{
		EXIT();
		return;
	}

	if (!DATA_GET(global_data,"rtvars_loaded"))
	{
		MTXDBG(CRITICAL,_("CRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n"));
		EXIT();
		return;
	}
	set_title(g_strdup(_("Loading RT Text...")));

	pathstub = g_build_filename(RTTEXT_DATA_DIR,firmware->rtt_map_file,NULL);
	filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,"xml");
	g_free(pathstub);
	if (!filename)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("File \"%s.xml\" not found!!, exiting function\n"),firmware->rtt_map_file);
		set_title(g_strdup(_("ERROR RunTimeText Map XML file DOES NOT EXIST!!!")));
		EXIT();
		return; 
	}

	/* Create window */
	xml = glade_xml_new(main_xml->filename,"rtt_window",NULL);
	window = glade_xml_get_widget(xml,"rtt_window");
	register_widget("rtt_window",window);
	x = (GINT)DATA_GET(global_data,"rtt_x_origin");
	y = (GINT)DATA_GET(global_data,"rtt_y_origin");
	gtk_window_move(GTK_WINDOW(window),x,y);
	gtk_window_set_default_size(GTK_WINDOW(window),-1,-1);
	g_object_set(window, "resizable", TRUE, NULL);
	parent = glade_xml_get_widget(xml,"rtt_vbox");
	glade_xml_signal_autoconnect(xml);

	LIBXML_TEST_VERSION

		doc = xmlReadFile(filename, NULL, 0);
	g_free(filename);
	if (doc == NULL)
	{
		printf(_("error: could not parse file %s\n"),filename);
		EXIT();
		return;
	}

	/*Get the root element node */
	store = gtk_list_store_new(RTT_NUM_COLS,G_TYPE_POINTER,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_FLOAT,G_TYPE_STRING,G_TYPE_STRING);
	DATA_SET(global_data,"rtt_model",store);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(parent),treeview,TRUE,TRUE,0);
	setup_rtt_treeview(treeview);

	root_element = xmlDocGetRootElement(doc);
	xml_result = load_rtt_xml_elements(root_element,store,parent);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	if (!xml_result)
		gtk_widget_destroy(window);
	else if (args->hide_rttext)
		gtk_widget_hide(window);
	else
		gtk_widget_show_all(window);

	set_title(g_strdup(_("RT Text Loaded...")));
	EXIT();
	return;
}


/*!
  \brief loads the runtime text XML elements
  \param a_node is the XML node
  \param store is the Liststoreto stick the built RTT object into
  \param parent is the container for the RTT display
  \returns FALSE when at EOF, otherwise TRUE
  */
G_MODULE_EXPORT gboolean load_rtt_xml_elements(xmlNode *a_node, GtkListStore *store, GtkWidget *parent)
{
	xmlNode *cur_node = NULL;
	ENTER();

	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"api") == 0)
				if (!xml_api_check(cur_node,RT_TEXT_MAJOR_API,RT_TEXT_MINOR_API))
				{
					MTXDBG(CRITICAL,_("API mismatch, won't load this file!!\n"));
					EXIT();
					return FALSE;
				}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"rtt") == 0)
				load_rtt(cur_node,store,parent);
		}
		if (!load_rtt_xml_elements(cur_node->children,store,parent))
		{
			EXIT();
			return FALSE;
		}
	}
	EXIT();
	return TRUE;
}

/*
  \brief load the RTT threshold details at this XML node, creates the 
  RTTthreshold object and returns it to the caller
  \param node is the pointer to XML node
  \returns pointer to a Rtt_Thresold object
  */

G_MODULE_EXPORT Rtt_Threshold *load_rtt_threshold(xmlNode *node)
{
	Rtt_Threshold *thresh = NULL;
	xmlNode *cur_node = NULL;
	GdkColor color;
	ENTER();
	if (!node->children)
	{
		printf(_("ERROR, load_rtt_threshold, xml node is empty!!\n"));
		EXIT();
		return NULL;
	}
	cur_node = node->children;
	thresh = g_new0(Rtt_Threshold,1);
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"low") == 0)
				generic_xml_gfloat_import(cur_node,&thresh->low);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"high") == 0)
				generic_xml_gfloat_import(cur_node,&thresh->high);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"fg_color") == 0)
			{
				generic_xml_color_import(cur_node,&color);
				thresh->fg = gdk_color_to_string(&color);
			}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"bg_color") == 0)
			{
				generic_xml_color_import(cur_node,&color);
				thresh->bg = gdk_color_to_string(&color);
			}
		}
		cur_node = cur_node->next;
	}
	/*
	printf("low, %f, high %f, bg (%s), fg (%s)\n",thresh->low,thresh->high,thresh->bg,thresh->fg);
	*/
	EXIT();
	return thresh;
}


/*
  \brief Frees memory of an Rtt_Threshold object
  \param data is hte pointer to the Rtt_Threshold object
  */
void rtt_thresh_free(gpointer data)
{
	Rtt_Threshold *thresh = (Rtt_Threshold *)data;
	ENTER();
	g_free(thresh->fg);
	g_free(thresh->bg);
	g_free(thresh);
	EXIT();
	return;
}


/*
  \brief load the RTT specifics at this XML node, creates the RTT object and 
  stores it in the ListStore
  \param node is the pointer to XML node
  \param store is the pointer to ListStore where we save it
  \param parent is the Parent widget
  */
G_MODULE_EXPORT void load_rtt(xmlNode *node,GtkListStore *store,GtkWidget *parent)
{
	gchar *int_name = NULL;
	gchar *source = NULL;
	Rt_Text *rt_text = NULL;
	GtkTreeIter iter;
	xmlNode *cur_node = NULL;
	Rtt_Threshold *thresh = NULL;
	GPtrArray *thresh_array = g_ptr_array_new_with_free_func(rtt_thresh_free);

	ENTER();
	if (!node->children)
	{
		printf(_("ERROR, load_rtt, xml node is empty!!\n"));
		EXIT();
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"internal_name") == 0)
				generic_xml_gchar_import(cur_node,&int_name);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"datasource") == 0)
				generic_xml_gchar_import(cur_node,&source);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"threshold") == 0)
			{
				thresh = load_rtt_threshold(cur_node);
				if (thresh)
					g_ptr_array_add(thresh_array, thresh);
			}

		}
		cur_node = cur_node->next;
	}
	if ((int_name) && (source))
		rt_text = create_rtt(int_name,source,TRUE);
	g_return_if_fail(rt_text);
	if (thresh_array->len > 0)
		rt_text->thresholds = thresh_array;
	else
		g_ptr_array_unref(thresh_array);

	if (rt_text)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				COL_RTT_OBJECT,(gpointer)rt_text,
				COL_RTT_INT_NAME,rt_text->ctrl_name,
				COL_RTT_DATA,"",
				COL_RTT_LAST,-0.1,-1);	
	}
	if (int_name)
		g_free(int_name);
	if (source)
		g_free(source);
	EXIT();
	return;
}


/*!
  \brief create_rtt() creates the rt_text from the passed data, and attaches
  it the the gui.
  \param ctrl_name is the name of the rt_text as defined in the config file
  \param source is the data source for this rt_text 
  \param show_prefix is a flag to hide or show the prefix
  \returns a pointer to a Rt_Text structure
  \see Rt_Text
  */
G_MODULE_EXPORT Rt_Text * create_rtt(gchar *ctrl_name, gchar *source, gboolean show_prefix)
{
	Rt_Text *rtt = NULL;
	Rtv_Map *rtv_map = NULL;
	gconstpointer *object = NULL;

	ENTER();
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	if (!rtv_map)
	{
		MTXDBG(CRITICAL,_("Bad things man, rtv_map is null!!\n"));
		EXIT();
		return NULL;
	}
	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!(object))
	{
		MTXDBG(CRITICAL,_("Bad things man, object doesn't exist for %s\n"),source);
		EXIT();
		return NULL;
	}

	rtt = (Rt_Text *)g_malloc0(sizeof(Rt_Text));
	rtt->show_prefix = show_prefix;
	rtt->ctrl_name = g_strdup(ctrl_name);
	rtt->friendly_name = (gchar *) DATA_GET(object,"dlog_gui_name");
	rtt->object = object;

	EXIT();
	return rtt;
}


/*!
  \brief add_rtt() creates the rt_text from the passed data, and attaches
  it the the gui.
  \param parent is the parent widget
  \param ctrl_name is the name of the rt_text as defined in the config file
  \returns a populated pointer to a Rt_Text structure
  */
G_MODULE_EXPORT Rt_Text * add_rtt(GtkWidget *parent, gchar *ctrl_name)
{
	Rt_Text *rtt = NULL;
	GtkWidget *label = NULL;
	GtkWidget *hbox = NULL;
	Rtv_Map *rtv_map = NULL;
	gconstpointer *object = NULL;
	gchar * source = NULL;
	gboolean show_prefix = FALSE;

	ENTER();
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	source = (gchar *)OBJ_GET(parent,"source");
	show_prefix = (GBOOLEAN)OBJ_GET(parent,"show_prefix");

	g_return_val_if_fail(rtv_map,NULL);
	g_return_val_if_fail(parent,NULL);
	g_return_val_if_fail(ctrl_name,NULL);
	g_return_val_if_fail(source,NULL);


	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!(object))
	{
		MTXDBG(CRITICAL,_("ERROR, Object doesn't exist for datasource: %s\n"),source);
		EXIT();
		return NULL;
	}

	rtt = (Rt_Text *)g_malloc0(sizeof(Rt_Text));
	rtt->show_prefix = show_prefix;
	rtt->ctrl_name = g_strdup(ctrl_name);
	rtt->friendly_name = (gchar *) DATA_GET(object,"dlog_gui_name");
	rtt->markup = (GBOOLEAN)OBJ_GET(parent,"markup");
	rtt->label_prefix = g_strdup((gchar *)OBJ_GET(parent,"label_prefix"));
	rtt->label_suffix = g_strdup((gchar *)OBJ_GET(parent,"label_suffix"));
	rtt->object = object;
	
	hbox = gtk_hbox_new(FALSE,2);

	/* Static prefix label.... */
	if (show_prefix)
	{
		label = gtk_label_new(NULL);
		rtt->name_label = label;
		gtk_label_set_markup(GTK_LABEL(label),rtt->friendly_name);
		gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
	}

	/* Value label */
	label = gtk_label_new(NULL);
	gtk_label_set_width_chars(GTK_LABEL(label),5);

	//set_fixed_size(label,11);
	rtt->textval = label;
	if (show_prefix)
		gtk_misc_set_alignment(GTK_MISC(label),1,0.5);
	else
		gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

	gtk_box_pack_start(GTK_BOX(parent),hbox,FALSE,FALSE,0);

	rtt->parent = hbox;
	gtk_widget_show_all(rtt->parent);

	EXIT();
	return rtt;
}


/*!
  \brief add_additional_rtt() is called as a post function for Tab loading
  to add an RTT on a normal widget tab. (AE wizard currently)
  \param widget is the pointer to widget containing the data needed
  */
G_MODULE_EXPORT void add_additional_rtt(GtkWidget *widget)
{
	GHashTable *rtt_hash = NULL;
	gchar * ctrl_name = NULL;
	Rt_Text *rt_text = NULL;

	ENTER();
	rtt_hash = (GHashTable *)DATA_GET(global_data,"rtt_hash");
	ctrl_name = (gchar *)OBJ_GET(widget,"ctrl_name");

	if (!rtt_hash)
	{
		rtt_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_rtt);
		DATA_SET_FULL(global_data,"rtt_hash",(gpointer)rtt_hash,g_hash_table_destroy);
	}

	if ((rtt_hash) && (ctrl_name))
		rt_text = add_rtt(widget,ctrl_name);

	if (rt_text)
	{
		if (!g_hash_table_lookup(rtt_hash,ctrl_name))
			g_hash_table_insert(rtt_hash,
					g_strdup(ctrl_name),
					(gpointer)rt_text);
	}
	EXIT();
	return;
}



/*!
  \brief rtt_update_values() is called for each runtime text to update
  it's label (label is periodic and not every time due to pango
  speed problems)
  \param key is the unused
  \param value is the pointer to Rt_Slider
  \param data is unused
  */
G_MODULE_EXPORT void rtt_update_values(gpointer key, gpointer value, gpointer data)
{
	static GRand *rand = NULL;
	static GMutex *rtv_mutex = NULL;
	Rt_Text *rtt = NULL;
	gint count = 0;
	gint last_upd = 0;
	gint precision = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;
	gchar * tmpbuf2 = NULL;

	ENTER();
	rtt = (Rt_Text *)value;
	if (!rtt)
	{
		EXIT();
		return;
	}
	if (!rand)
		rand = g_rand_new();
	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");

	count = rtt->count;
	last_upd = rtt->last_upd;
	history = (GArray *)DATA_GET(rtt->object,"history");
	precision = (GINT)DATA_GET(rtt->object,"precision");

	if (!history)
	{
		EXIT();
		return;
	}
	if ((GINT)history->len-2 <= 0)
	{
		EXIT();
		return;
	}
	g_mutex_lock(rtv_mutex);
	current = g_array_index(history, gfloat, history->len-1);
	previous = g_array_index(history, gfloat, history->len-2);
	g_mutex_unlock(rtv_mutex);

	if (GTK_IS_WIDGET(gtk_widget_get_window(GTK_WIDGET(rtt->textval))))
		if (!gdk_window_is_viewable(gtk_widget_get_window(GTK_WIDGET(rtt->textval))))
		{
			EXIT();
			return;
		}

	if ((current != previous) 
			|| (DATA_GET(global_data,"forced_update")) 
			|| (rtt->textval && ((abs(count-last_upd)%g_rand_int_range(rand,25,50)) == 0)))
	{
		tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);
		if (rtt->markup)
		{
			tmpbuf2 = g_strconcat(rtt->label_prefix,tmpbuf,rtt->label_suffix,NULL);
			gtk_label_set_markup(GTK_LABEL(rtt->textval),tmpbuf2);
			g_free(tmpbuf2);
		}
		else
			gtk_label_set_text(GTK_LABEL(rtt->textval),tmpbuf);
		g_free(tmpbuf);
		last_upd = count;
	}

	if (last_upd > 5000)
		last_upd = 0;
	count++;
	if (count > 5000)
		count = 0;
	rtt->count = count;
	rtt->last_upd = last_upd;
	EXIT();
	return;
}


/*!
  \brief Sets up the runtime text treeview
  \param treeview is the pointer to the treeview to setup
  */
G_MODULE_EXPORT void setup_rtt_treeview(GtkWidget *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *parent = gtk_widget_get_parent(treeview);
	GtkStyle * style = NULL;

	ENTER();
	style = gtk_widget_get_style(parent);

	renderer = gtk_cell_renderer_text_new();
	/*gtk_cell_renderer_set_fixed_size(GTK_CELL_RENDERER(renderer),-1, 1);*/
	g_object_set(renderer, "background-gdk", &style->bg[GTK_STATE_NORMAL], NULL);
	column = gtk_tree_view_column_new_with_attributes("",renderer, "text", COL_RTT_INT_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	/*gtk_cell_renderer_set_fixed_size(GTK_CELL_RENDERER(renderer),65, 1);*/
	g_object_set(renderer, "background-gdk", &style->bg[GTK_STATE_NORMAL], NULL);
	column = gtk_tree_view_column_new_with_attributes("",renderer, 
			"markup", COL_RTT_DATA, 
			"background", COL_RTT_BGCOLOR, 
			"foreground",COL_RTT_FGCOLOR, NULL);
	g_object_set(column, "alignment", 1.0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview), TRUE);
	EXIT();
	return;
}


/*!
  \brief updates each RTT with new data as appropriate
  \param model is the pointer to TreeModel
  \param path is the pointer to item in the model
  \param iter is the iterator
  \param user_data is unused
  \returns FALSE
  */
G_MODULE_EXPORT gboolean rtt_foreach(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,gpointer user_data)
{
	static GMutex *rtv_mutex = NULL;
	Rt_Text *rtt = NULL;
	Rtt_Threshold *thresh = NULL;
	gint count = 0;
	gboolean in_thresh = FALSE;
	gint precision = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;

	ENTER();
	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");

	gtk_tree_model_get (model, iter,
			COL_RTT_OBJECT, &rtt,
			COL_RTT_LAST, &previous,
			-1);

	if (!(rtt->object))
	{
		EXIT();
		return FALSE;
	}
	history = (GArray *)DATA_GET(rtt->object,"history");
	precision = (GINT)DATA_GET(rtt->object,"precision");

	if (!history)
	{
		EXIT();
		return FALSE;
	}
	if ((GINT)history->len-1 <= 0)
	{
		EXIT();
		return FALSE;
	}
	g_mutex_lock(rtv_mutex);
	current = g_array_index(history, gfloat, history->len-1);
	g_mutex_unlock(rtv_mutex);

	if ((current != previous) || (DATA_GET(global_data,"forced_update")))
	{
		tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);
		if (rtt->thresholds)
		{
			/* Need to check for a proper match */
			for (guint i=0;i<rtt->thresholds->len;i++)
			{
				thresh = (Rtt_Threshold *)g_ptr_array_index(rtt->thresholds,i);
				if ((current >thresh->low) && (current <= thresh->high))
				{
					in_thresh = TRUE;
					gtk_list_store_set(GTK_LIST_STORE(model), iter,
							COL_RTT_DATA, tmpbuf,
							COL_RTT_BGCOLOR, thresh->bg,
							COL_RTT_FGCOLOR, thresh->fg,
							COL_RTT_LAST, current,	-1);
					break;
				}
			}
			/* Value wasn't within threshold range, render without colors */
			if (!in_thresh)
				goto not_in_thresh;
		}
		else
		{
not_in_thresh:
			gtk_list_store_set(GTK_LIST_STORE(model), iter,
					COL_RTT_DATA, tmpbuf,
					COL_RTT_BGCOLOR, NULL,
					COL_RTT_FGCOLOR, NULL,
					COL_RTT_LAST, current,	-1);
		}

		g_free(tmpbuf);
	}
	EXIT();
	return FALSE;
}


/*!
  \brief calls the rtt_foreach handler to update each runtime text entry
  \param data is unused
  \returns TRUE on success, FALSE if Mtx is shutting down
  */
G_MODULE_EXPORT gboolean update_rttext(gpointer data)
{       
	static GMutex *rtt_mutex = NULL;
	static GtkTreeModel *rtt_model = NULL;
	static GHashTable *rtt_hash = NULL;

	ENTER();
	/* Return false (cancel timeout) if leaving */
	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return FALSE;
	}

	if (!rtt_mutex)
		rtt_mutex = (GMutex *)DATA_GET(global_data,"rtt_mutex");
	if (!rtt_model)
		rtt_model = (GtkTreeModel *)DATA_GET(global_data,"rtt_model");
	if (!rtt_hash)
		rtt_hash = (GHashTable *)DATA_GET(global_data,"rtt_hash");

	g_mutex_lock(rtt_mutex);
	/* Silently return if not yet ready */
	if (rtt_model)
		gtk_tree_model_foreach(rtt_model,rtt_foreach,NULL);
	if (rtt_hash)
		g_hash_table_foreach(rtt_hash,rtt_update_values,NULL);
	g_mutex_unlock(rtt_mutex);
	EXIT();
	return FALSE;
}

