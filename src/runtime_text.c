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


#include <args.h>
#include <apicheck.h>
#include <api-versions.h>
#include <configfile.h>
#include <debugging.h>
#include <firmware.h>
#include <getfiles.h>
#include <glade/glade-xml.h>
#include <glib.h>
#include <gui_handlers.h>
#include <init.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <runtime_status.h>
#include <runtime_text.h>
#include <stdio.h>
#include <stdlib.h>
#include <watches.h>
#include <widgetmgmt.h>
#include <xmlbase.h>

extern gconstpointer *global_data;

/*!
 \brief load_rt_text_pf() is called to load up the runtime text configurations
 from the file specified in the firmware's interrogation profile, and populate
 a new window with the runtiem vars text value box.
 */
EXPORT void load_rt_text_pf()
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
	CmdLineArgs *args = DATA_GET(global_data,"args");
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	extern volatile gboolean leaving;
	extern gboolean rtvars_loaded;
	extern Firmware_Details *firmware;
	extern gboolean interrogated;

	if (leaving)
		return;
	if (!(interrogated))
		return;
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	if ((!main_xml) || (leaving))
		return;

	if (rtvars_loaded == FALSE) 
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": load_rt_text_pf()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n"));
		return;
	}
	gdk_threads_enter();
	set_title(g_strdup(_("Loading RT Text...")));

	filename = get_file(g_strconcat(RTTEXT_DATA_DIR,PSEP,firmware->rtt_map_file,NULL),g_strdup("xml"));
	if (!filename)
	{
		dbg_func(RTMLOADER|CRITICAL,g_strdup_printf(__FILE__": load_rt_text_pf()\n\t File \"%s.xml\" not found!!, exiting function\n",firmware->rtt_map_file));
		set_title(g_strdup(_("ERROR RunTimeText Map XML file DOES NOT EXIST!!!")));
		gdk_threads_leave();
		return; 
	}

	/* Create window */
	xml = glade_xml_new(main_xml->filename,"rtt_window",NULL);
	window = glade_xml_get_widget(xml,"rtt_window");
	register_widget("rtt_window",window);
	x = (GINT)DATA_GET(global_data,"rtt_x_origin");
	y = (GINT)DATA_GET(global_data,"rtt_y_origin");
	gtk_window_move(GTK_WINDOW(window),x,y);
	gtk_window_set_default_size(GTK_WINDOW(window),1,1);
	g_object_set(window, "resizable", TRUE, NULL);
	parent = glade_xml_get_widget(xml,"rtt_vbox");
	glade_xml_signal_autoconnect(xml);

	LIBXML_TEST_VERSION

		doc = xmlReadFile(filename, NULL, 0);
	g_free(filename);
	if (doc == NULL)
	{
		printf(_("error: could not parse file %s\n"),filename);
		gdk_threads_leave();
		return;
	}

	/*Get the root element node */
	store = gtk_list_store_new(RTT_NUM_COLS,G_TYPE_POINTER,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_FLOAT);
	DATA_SET_FULL(global_data,"rtt_model",store,gtk_list_store_clear);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(parent),treeview,TRUE,TRUE,0);
	setup_rtt_treeview(treeview);

	root_element = xmlDocGetRootElement(doc);
	xml_result = load_rtt_xml_elements(root_element,store,parent);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	if (xml_result == FALSE)
		gtk_widget_destroy(window);
	else if ((!args->hide_rttext) && (xml_result))
		gtk_widget_show_all(window);

	set_title(g_strdup(_("RT Text Loaded...")));
	gdk_threads_leave();
	return;
}


gboolean load_rtt_xml_elements(xmlNode *a_node, GtkListStore *store, GtkWidget *parent)
{
	xmlNode *cur_node = NULL;

	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"api") == 0)
				if (!xml_api_check(cur_node,RT_TEXT_MAJOR_API,RT_TEXT_MINOR_API))
				{
					dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_rtt_xml_elements()\n\tAPI mismatch, won't load this file!!\n"));
					return FALSE;
				}
			if (g_strcasecmp((gchar *)cur_node->name,"rtt") == 0)
				load_rtt(cur_node,store,parent);
		}
		if (!load_rtt_xml_elements(cur_node->children,store,parent))
			return FALSE;
	}
	return TRUE;
}


void load_rtt(xmlNode *node,GtkListStore *store,GtkWidget *parent)
{
	gchar *int_name = NULL;
	gchar *source = NULL;
	Rt_Text *rt_text = NULL;
	GtkTreeIter iter;
	xmlNode *cur_node = NULL;

	if (!node->children)
	{
		printf(_("ERROR, load_potential_args, xml node is empty!!\n"));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"internal_name") == 0)
				generic_xml_gchar_import(cur_node,&int_name);
			if (g_strcasecmp((gchar *)cur_node->name,"datasource") == 0)
				generic_xml_gchar_import(cur_node,&source);
		}
		cur_node = cur_node->next;
	}
	if ((int_name) && (source))
		rt_text = create_rtt(int_name,source,TRUE);
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
}


/*!
 \brief create_rtt() creates the rt_text from the passed data, and attaches
 it the the gui.
 \param parent (GtkWidget *) parent widget
 \param ctrl_name (gchar *) name of the rt_text as defined in the config file
 \param source (gchar *) data source for this rt_text 
 \returns a Struct Rt_Text *
 */
Rt_Text * create_rtt(gchar *ctrl_name, gchar *source, gboolean show_prefix)
{
	Rt_Text *rtt = NULL;
	extern Rtv_Map *rtv_map;
	gconstpointer *object = NULL;

	rtt = g_malloc0(sizeof(Rt_Text));

	if (!rtv_map)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": create_rtt()\n\tBad things man, rtv_map is null!!\n"));
		return NULL;
	}
	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!(object))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": create_rtt()\n\tBad things man, object doesn't exist for %s\n",source));
		return NULL;
	}

	rtt->show_prefix = show_prefix;
	rtt->ctrl_name = g_strdup(ctrl_name);
	rtt->friendly_name = (gchar *) DATA_GET(object,"dlog_gui_name");
	rtt->object = object;

	return rtt;
}


/*!
 \brief add_rtt() creates the rt_text from the passed data, and attaches
 it the the gui.
 \param parent (GtkWidget *) parent widget
 \param ctrl_name (gchar *) name of the rt_text as defined in the config file
 \param source (gchar *) data source for this rt_text 
 \returns a Struct Rt_Text *
 */
Rt_Text * add_rtt(GtkWidget *parent, gchar *ctrl_name, gchar *source, gboolean show_prefix)
{
	Rt_Text *rtt = NULL;
	GtkWidget *label = NULL;
	GtkWidget *hbox = NULL;
	extern Rtv_Map *rtv_map;
	gconstpointer *object = NULL;

	rtt = g_malloc0(sizeof(Rt_Text));

	if (!rtv_map)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": add_rtt()\n\tBad things man, rtv_map is null!!\n"));
		return NULL;
	}

	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!(object))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": add_rtt()\n\tBad things man, object doesn't exist for %s\n",source));
		return NULL;
	}

	rtt->show_prefix = show_prefix;
	rtt->ctrl_name = g_strdup(ctrl_name);
	rtt->friendly_name = (gchar *) DATA_GET(object,"dlog_gui_name");
	rtt->markup = (GBOOLEAN)OBJ_GET(parent,"markup");
	rtt->label_prefix = OBJ_GET(parent,"label_prefix");
	rtt->label_suffix = OBJ_GET(parent,"label_suffix");
	rtt->object = object;
	

	hbox = gtk_hbox_new(FALSE,5);

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

	//set_fixed_size(label,6);
	rtt->textval = label;
	if (show_prefix)
		gtk_misc_set_alignment(GTK_MISC(label),1,0.5);
	else
		gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

	gtk_box_pack_start(GTK_BOX(parent),hbox,TRUE,TRUE,0);

	rtt->parent = hbox;
	gtk_widget_show_all(rtt->parent);

	return rtt;
}


/*!
 \brief add_additional_rtt() is called as a post function for Tab loading
 to add an RTT on a normal widget tab. (AE wizard currently)
 \param widget, pointer to widget containing the data needed
 */
EXPORT void add_additional_rtt(GtkWidget *widget)
{
	GHashTable *rtt_hash = NULL;
	gchar * ctrl_name = NULL;
	gchar * source = NULL;
	Rt_Text *rt_text = NULL;
	gboolean show_prefix = FALSE;

	rtt_hash = DATA_GET(global_data,"rtt_hash");
	ctrl_name = OBJ_GET(widget,"ctrl_name");
	source = OBJ_GET(widget,"source");
	show_prefix = (GBOOLEAN)OBJ_GET(widget,"show_prefix");

	if (!rtt_hash)
	{
		rtt_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_rtt);
		DATA_SET_FULL(global_data,"rtt_hash",(gpointer)rtt_hash,g_hash_table_destroy);
	}

	if ((rtt_hash) && (ctrl_name) && (source))
		rt_text = add_rtt(widget,ctrl_name,source,show_prefix);

	if (rt_text)
	{
		if (!g_hash_table_lookup(rtt_hash,ctrl_name))
			g_hash_table_insert(rtt_hash,
					g_strdup(ctrl_name),
					(gpointer)rt_text);
	}
	return;
}



/*!
 \brief rtt_update_values() is called for each runtime text to update
 it's label (label is periodic and not every time due to pango
 speed problems)
 \param key (gpointer) unused
 \param value (gpointer) pointer to Rt_Slider
 \param data (gpointer) unused
 */
void rtt_update_values(gpointer key, gpointer value, gpointer data)
{
	Rt_Text *rtt = NULL;
	gint count = 0;
	gint last_upd = 0;
	gint precision = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;
	gchar * tmpbuf2 = NULL;
	static GRand *rand = NULL;
	extern gboolean forced_update;
	extern GStaticMutex rtv_mutex;

	rtt = (Rt_Text *)value;
	if (!rtt)
		return;
	if (!rand)
		rand = g_rand_new();

	count = rtt->count;
	last_upd = rtt->last_upd;
	history = (GArray *)DATA_GET(rtt->object,"history");
	precision = (GINT)DATA_GET(rtt->object,"precision");

	if (!history)
		return;
	if ((gint)history->len-2 <= 0)
		return;
	g_static_mutex_lock(&rtv_mutex);
	current = g_array_index(history, gfloat, history->len-1);
	previous = g_array_index(history, gfloat, history->len-2);
	g_static_mutex_unlock(&rtv_mutex);

	if (GTK_IS_WIDGET(GTK_WIDGET(rtt->textval)->window))
		if (!gdk_window_is_viewable(GTK_WIDGET(rtt->textval)->window))
			return;

	if ((current != previous) 
			|| (forced_update) 
			|| (rtt->textval && ((abs(count-last_upd)%g_rand_int_range(rand,25,50)) == 0)))
	{
		tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);
		gdk_threads_enter();
		if (rtt->markup)
		{
			tmpbuf2 = g_strconcat(rtt->label_prefix,tmpbuf,rtt->label_suffix,NULL);
			gtk_label_set_markup(GTK_LABEL(rtt->textval),tmpbuf2);
			g_free(tmpbuf2);
		}
		else
			gtk_label_set_text(GTK_LABEL(rtt->textval),tmpbuf);
		gdk_threads_leave();
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
	return;
}


void setup_rtt_treeview(GtkWidget *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *parent = gtk_widget_get_parent(treeview);
	GtkStyle * style = NULL;

	style = gtk_widget_get_style(parent);

	renderer = gtk_cell_renderer_text_new();
//	gtk_cell_renderer_set_fixed_size(GTK_CELL_RENDERER(renderer),-1, 1);
	g_object_set(renderer, "background-gdk", &style->bg[GTK_STATE_NORMAL], NULL);
	column = gtk_tree_view_column_new_with_attributes("",renderer, "text", COL_RTT_INT_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
//	gtk_cell_renderer_set_fixed_size(GTK_CELL_RENDERER(renderer),-1, 1);
	g_object_set(renderer, "background-gdk", &style->bg[GTK_STATE_NORMAL], NULL);
	column = gtk_tree_view_column_new_with_attributes("",renderer, "markup", COL_RTT_DATA,  NULL);
	g_object_set(column, "alignment", 1.0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview), TRUE);
	
}


gboolean rtt_foreach(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,gpointer user_data)
{
	Rt_Text *rtt = NULL;
	gint count = 0;
	gint last_upd = 0;
	gint precision = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;
	extern gboolean forced_update;
	extern GStaticMutex rtv_mutex;

	gtk_tree_model_get (model, iter,
			COL_RTT_OBJECT, &rtt,
			COL_RTT_LAST, &previous,
			-1);

	if (!(rtt->object))
		return FALSE;
	history = (GArray *)DATA_GET(rtt->object,"history");
	precision = (GINT)DATA_GET(rtt->object,"precision");

	if (!history)
		return FALSE;
	if ((gint)history->len-1 <= 0)
		return FALSE;
	g_static_mutex_lock(&rtv_mutex);
	current = g_array_index(history, gfloat, history->len-1);
	g_static_mutex_unlock(&rtv_mutex);

	if ((current != previous) || (forced_update))
	{
		tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);
		gdk_threads_enter();
		gtk_list_store_set(GTK_LIST_STORE(model), iter,
				COL_RTT_DATA, tmpbuf,
			        COL_RTT_LAST, current,	-1);
		gdk_threads_leave();
		g_free(tmpbuf);
		last_upd = count;
	}
	return FALSE;
}

