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

extern GObject *global_data;

/*!
 \brief load_rt_text_pf() is called to load up the runtime text configurations
 from the file specified in the firmware's interrogation profile, and populate
 a new window with the runtiem vars text value box.
 */
EXPORT void load_rt_text_pf()
{
	GHashTable *rtt_hash = NULL;
	GtkWidget *window = NULL;
	GtkWidget *parent = NULL;
	gchar *filename = NULL;
	GladeXML *main_xml = NULL;
	GladeXML *xml = NULL;
	gboolean xml_result = FALSE;
	CmdLineArgs *args = OBJ_GET(global_data,"args");
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	extern volatile gboolean leaving;
	extern gboolean rtvars_loaded;
	extern Firmware_Details *firmware;
	extern gboolean connected;
	extern gboolean interrogated;

	if (leaving)
		return;
	if (!((connected) && (interrogated)))
		return;
	main_xml = (GladeXML *)OBJ_GET(global_data,"main_xml");
	if ((!main_xml) || (leaving))
		return;

	if (rtvars_loaded == FALSE) 
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": load_rt_text_pf()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n"));
		return;
	}
	set_title(g_strdup("Loading RT Text..."));
	if (!rtt_hash)
		rtt_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	OBJ_SET(global_data,"rtt_hash",(gpointer)rtt_hash);

	filename = get_file(g_strconcat(RTTEXT_DATA_DIR,PSEP,firmware->rtt_map_file,NULL),g_strdup("xml"));
	if (!filename)
	{
		dbg_func(RTMLOADER|CRITICAL,g_strdup_printf(__FILE__": load_rt_text_pf()\n\t File \"%s.xml\" not found!!, exiting function\n",firmware->rtt_map_file));
		set_title(g_strdup("ERROR RunTimeText Map XML file DOES NOT EXIST!!!"));
		return; 
	}

	/* Create window */
	xml = glade_xml_new(main_xml->filename,"rtt_window",NULL);
	window = glade_xml_get_widget(xml,"rtt_window");
	parent = glade_xml_get_widget(xml,"rtt_vbox");
	glade_xml_signal_autoconnect(xml);

	LIBXML_TEST_VERSION

	doc = xmlReadFile(filename, NULL, 0);
	g_free(filename);
	if (doc == NULL)
	{
		printf("error: could not parse file %s\n",filename);
		return;
	}

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	xml_result = load_rtt_xml_elements(root_element,rtt_hash,parent);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	if (xml_result == FALSE)
		gtk_widget_destroy(window);
	else if ((!args->hide_rttext) && (xml_result))
		gtk_widget_show_all(window);

	set_title(g_strdup("RT Text Loaded..."));
	return;
}


gboolean load_rtt_xml_elements(xmlNode *a_node, GHashTable *hash, GtkWidget *parent)
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
				load_rtt(cur_node,hash,parent);
		}
		if (!load_rtt_xml_elements(cur_node->children,hash,parent))
			return FALSE;
	}
	return TRUE;
}


void load_rtt(xmlNode *node,GHashTable *hash,GtkWidget *parent)
{
	gchar *int_name = NULL;
	gchar *source = NULL;
	Rt_Text *rt_text = NULL;
	xmlNode *cur_node = NULL;

	if (!node->children)
	{
		printf("ERROR, load_potential_args, xml node is empty!!\n");
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
		rt_text = add_rtt(parent,int_name,source,TRUE);
	if (rt_text)
	{
		if (!g_hash_table_lookup(hash,int_name))
			g_hash_table_insert(hash,
					g_strdup(int_name),
					(gpointer)rt_text);
	}
	if (int_name)
		g_free(int_name);
	if (source)
		g_free(source);
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
	GObject *object = NULL;

	rtt = g_malloc0(sizeof(Rt_Text));

	if (!rtv_map)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": add_rtt()\n\tBad things man, rtv_map is null!!\n"));
		return NULL;
	}

	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!G_IS_OBJECT(object))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": add_rtt()\n\tBad things man, object doesn't exist for %s\n",source));
		return NULL;
	}

	rtt->show_prefix = show_prefix;
	rtt->ctrl_name = g_strdup(ctrl_name);
	rtt->friendly_name = (gchar *) OBJ_GET(object,"dlog_gui_name");
	rtt->history = (GArray *) OBJ_GET(object,"history");
	rtt->object = object;

	hbox = gtk_hbox_new(FALSE,5);

	if (show_prefix)
	{
		label = gtk_label_new(NULL);
		rtt->name_label = label;
		gtk_label_set_markup(GTK_LABEL(label),rtt->friendly_name);
		gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
	}

	label = gtk_label_new(NULL);
	set_fixed_size(label,6);
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
 \brief add_custom_rtt() creates the rt_text from the passed data, and attaches
 it the the gui.
 \param label (GtkWidget *) parent widget
 \param ctrl_name (gchar *) name of the rt_text as defined in the config file
 \param source (gchar *) data source for this rt_text 
 \returns a Struct Rt_Text *
 */
Rt_Text * add_custom_rtt(GtkWidget *label, gchar *ctrl_name, gchar *source, gboolean show_prefix)
{
	Rt_Text *rtt = NULL;
	extern Rtv_Map *rtv_map;
	GObject *object = NULL;

	rtt = g_malloc0(sizeof(Rt_Text));

	if (!rtv_map)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": add_rtt()\n\tBad things man, rtv_map is null!!\n"));
		return NULL;
	}

	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!G_IS_OBJECT(object))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": add_rtt()\n\tBad things man, object doesn't exist for %s\n",source));
		return NULL;
	}

	rtt->show_prefix = show_prefix;
	rtt->ctrl_name = g_strdup(ctrl_name);
	rtt->friendly_name = (gchar *) OBJ_GET(object,"dlog_gui_name");
	rtt->history = (GArray *) OBJ_GET(object,"history");
	rtt->object = object;
	if (OBJ_GET(label,"markup"))
	{
		rtt->markup = TRUE;
		OBJ_SET(object,"label_prefix",OBJ_GET(label,"label_prefix"));
		OBJ_SET(object,"label_suffix",OBJ_GET(label,"label_suffix"));
	}
	else
		rtt->markup = FALSE;

	rtt->textval = label;
	if (show_prefix)
		gtk_misc_set_alignment(GTK_MISC(label),1,0.5);
	else
		gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
	gtk_widget_show_all(rtt->textval);

	return rtt;
}


/*!
 \brief add_additional_rtt() is called as a post function for Tab loading
 to add an RTT on a normal widget tab. (AE wizard currently)
 \param widget, pointer to widget containing the data needed
 */
EXPORT void add_additional_rtt(GtkWidget *widget)
{
	gchar * ctrl_name = NULL;
	gchar * source = NULL;
	gboolean markup = FALSE;
	GHashTable *rtt_hash = NULL;
	Rt_Text *rt_text = NULL;
	gboolean show_prefix = FALSE;

	rtt_hash = OBJ_GET(global_data,"rtt_hash");
	ctrl_name = OBJ_GET(widget,"ctrl_name");
	source = OBJ_GET(widget,"source");
	show_prefix = (gboolean)OBJ_GET(widget,"show_prefix");
	markup = (gboolean)OBJ_GET(widget,"markup");

	if ((rtt_hash) && (ctrl_name) && (source) && (!markup))
		rt_text = add_rtt(widget,ctrl_name,source,show_prefix);
	if ((rtt_hash) && (ctrl_name) && (source) && (markup))
		rt_text = add_custom_rtt(widget,ctrl_name,source,show_prefix);
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
	Rt_Text *rtt = (Rt_Text *)value;
	gint count = rtt->count;
	gint last_upd = rtt->last_upd;
	gint current_index = 0;
	gint precision = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;
	gchar * tmpbuf2 = NULL;
	extern gboolean forced_update;
	extern GStaticMutex rtv_mutex;

	history = (GArray *)OBJ_GET(rtt->object,"history");
	current_index = (gint)OBJ_GET(rtt->object,"current_index");
	precision = (gint)OBJ_GET(rtt->object,"precision");

	if (!history)
		return;
	if (current_index < 0)
		return;
	dbg_func(MUTEX,g_strdup_printf(__FILE__": rtt_update_values() before lock rtv_mutex\n"));
	g_static_mutex_lock(&rtv_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": rtt_update_values() after lock rtv_mutex\n"));
	current = g_array_index(history, gfloat, current_index);
	if (current_index > 0)
		current_index-=1;
	previous = g_array_index(history, gfloat, current_index);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": rtt_update_values() before UNlock rtv_mutex\n"));
	g_static_mutex_unlock(&rtv_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": rtt_update_values() after UNlock rtv_mutex\n"));

	if (GTK_IS_WIDGET(GTK_WIDGET(rtt->textval)->window))
		if (!gdk_window_is_viewable(GTK_WIDGET(rtt->textval)->window))
			return;

	if ((current != previous) || (forced_update))
	{
		/* If changed by more than 5% or has been at least 5 
		 * times withot an update or forced_update is set
		 * */
		/*if ((rtt->textval) && ((abs(count-last_upd) > 2) || (forced_update)))*/
		{
			tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);
			if (rtt->markup)
			{
				tmpbuf2 = g_strconcat(OBJ_GET(rtt->object,"label_prefix"),tmpbuf,OBJ_GET(rtt->object,"label_suffix"),NULL);
				gtk_label_set_markup(GTK_LABEL(rtt->textval),tmpbuf2);
				g_free(tmpbuf2);
				g_free(tmpbuf);
			}
			else
			{
				gtk_label_set_text(GTK_LABEL(rtt->textval),tmpbuf);
				g_free(tmpbuf);
			}
			last_upd = count;
		}
	}
	else if (rtt->textval && ((abs(count-last_upd)%30) == 0))
	{
		tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);
		if (rtt->markup)
		{
			tmpbuf2 = g_strconcat(OBJ_GET(rtt->object,"label_prefix"),tmpbuf,OBJ_GET(rtt->object,"label_suffix"),NULL);
			gtk_label_set_markup(GTK_LABEL(rtt->textval),tmpbuf2);
			g_free(tmpbuf2);
			g_free(tmpbuf);
		}
		else
		{
			gtk_label_set_text(GTK_LABEL(rtt->textval),tmpbuf);
			g_free(tmpbuf);
		}

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
