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
#include <glade/glade-xml.h>
#include <getfiles.h>
#include <firmware.h>
#include <glib.h>
#include <gui_handlers.h>
#include <keybinder.h>
#include <keyparser.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <listmgmt.h>
#include <notifications.h>
#include <runtime_status.h>
#include <tabloader.h>
#include <widgetmgmt.h>
#include <xmlbase.h>



extern gconstpointer *global_data;

/*!
 \brief load_status_pf() is called to create the ECU status window, load the 
 settings from the StatusMapFile.
 */
G_MODULE_EXPORT void load_status_pf(void)
{
	gchar *filename = NULL;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	GtkWidget * window;
	GtkWidget * parent;
	GladeXML *xml = NULL;
	gboolean xml_result = FALSE;
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	GladeXML *main_xml;
	Firmware_Details *firmware = NULL;
	CmdLineArgs *args =  NULL;
	
	args = DATA_GET(global_data,"args");
	firmware = DATA_GET(global_data,"firmware");

	g_return_if_fail(firmware);
	g_return_if_fail(args);

	if (!(DATA_GET(global_data,"interrogated")))
		return;
	if (!firmware->status_map_file)
	{
		//dbg_func(CRITICAL,g_strdup_printf(__FILE__": firmware->status_map_file is UNDEFINED,\n\texiting status window creation routine!!!!\n"));
		return;
	}

	gdk_threads_enter();
	set_title(g_strdup(_("Loading RT Status...")));
	filename = get_file(g_strconcat(RTSTATUS_DATA_DIR,PSEP,firmware->status_map_file,NULL),g_strdup("xml"));
	if (!filename)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_runtime_status()\n\t File \"%s.xml\" not found!!, exiting function\n",firmware->status_map_file));
		set_title(g_strdup(_("ERROR RT Statusfile DOES NOT EXIST!!!")));
		gdk_threads_leave();
		return;
	}
	main_xml = (GladeXML *)DATA_GET(global_data,"main_xml");
	xml = glade_xml_new(main_xml->filename,"status_window",NULL);
	window = glade_xml_get_widget(xml,"status_window");
	register_widget("status_window",window);
	gtk_window_set_focus_on_map((GtkWindow *)window,FALSE);
	gtk_window_set_title(GTK_WINDOW(window),_("ECU Status"));
	x = (GINT)DATA_GET(global_data,"status_x_origin");
	y = (GINT)DATA_GET(global_data,"status_y_origin");
	gtk_window_move(GTK_WINDOW(window),x,y);
	w = (GINT)DATA_GET(global_data,"status_width");
	h = (GINT)DATA_GET(global_data,"status_height");
	gtk_window_set_default_size(GTK_WINDOW(window),w,h);
	/*
	if (g_strcasecmp(firmware->actual_signature,DATA_GET(global_data,"last_signature")) == 0)
		gtk_window_set_default_size(GTK_WINDOW(window),w,h);
	else
		gtk_window_set_default_size(GTK_WINDOW(window),-1,-1);
		*/
//	gtk_window_resize(GTK_WINDOW(window),w,h);
//	g_object_set(window, "resizable", FALSE, NULL);
	parent = glade_xml_get_widget(xml,"status_vbox");
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

	root_element = xmlDocGetRootElement(doc);
	xml_result = load_status_xml_elements(root_element,parent);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	if (xml_result == FALSE)
		gtk_widget_destroy(window);
	else if ((!args->hide_status) && (xml_result))
		gtk_widget_show_all(window);

	set_title(g_strdup(_("RT Status Loaded...")));
	gdk_threads_leave();
	return;
}


G_MODULE_EXPORT gboolean load_status_xml_elements(xmlNode *a_node, GtkWidget *parent)
{
	xmlNode *cur_node = NULL;

	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"api") == 0)
				if (!xml_api_check(cur_node,RT_STATUS_MAJOR_API,RT_STATUS_MINOR_API))
				{
					dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_status_xml_elements()\n\tAPI mismatch, won't load this file!!\n"));
					return FALSE;
				}
			if (g_strcasecmp((gchar *)cur_node->name,"status") == 0)
				load_status(cur_node,parent);
		}
		if (!load_status_xml_elements(cur_node->children,parent))
			return FALSE;
	}
	return TRUE;
}


G_MODULE_EXPORT void load_status(xmlNode *node,GtkWidget *parent)
{
	gchar *txt = NULL;
	gchar *active_fg = NULL;
	gchar *inactive_fg = NULL;
	gchar *bind_to_list = NULL;
	gchar *source = NULL;
	gint bitval = -1;
	gint bitmask = -1;
	GtkWidget *label = NULL;
	GtkWidget *frame = NULL;
	GdkColor color;
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
			if (g_strcasecmp((gchar *)cur_node->name,"label") == 0)
				generic_xml_gchar_import(cur_node,&txt);
			if (g_strcasecmp((gchar *)cur_node->name,"active_fg") == 0)
				generic_xml_gchar_import(cur_node,&active_fg);
			if (g_strcasecmp((gchar *)cur_node->name,"inactive_fg") == 0)
				generic_xml_gchar_import(cur_node,&inactive_fg);
			if (g_strcasecmp((gchar *)cur_node->name,"source") == 0)
				generic_xml_gchar_import(cur_node,&source);
			if (g_strcasecmp((gchar *)cur_node->name,"bind_to_list") == 0)
				generic_xml_gchar_import(cur_node,&bind_to_list);
			if (g_strcasecmp((gchar *)cur_node->name,"bitval") == 0)
				generic_xml_gint_import(cur_node,&bitval);
			if (g_strcasecmp((gchar *)cur_node->name,"bitmask") == 0)
				generic_xml_gint_import(cur_node,&bitmask);
		}
		cur_node = cur_node->next;
	}
	/* Minimum requirements */
	if ((txt) && (active_fg) && (inactive_fg) && (bind_to_list))
	{
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_ETCHED_IN);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),_(txt));
		gtk_container_add(GTK_CONTAINER(frame),label);
		gtk_widget_set_sensitive(GTK_WIDGET(label),FALSE);
		gdk_color_parse(active_fg,&color);
		gtk_widget_modify_fg(label,GTK_STATE_NORMAL,&color);
		gdk_color_parse(inactive_fg,&color);
		gtk_widget_modify_fg(label,GTK_STATE_INSENSITIVE,&color);
		g_free(txt);
		g_free(active_fg);
		g_free(inactive_fg);
		/* For controls based on ECU data */
		if ((bitval >= 0) && (bitmask >= 0) && (source))
		{
			OBJ_SET(label,"bitval",GINT_TO_POINTER(bitval));
			OBJ_SET(label,"bitmask",GINT_TO_POINTER(bitmask));
			OBJ_SET(label,"source",g_strdup(source));
			g_free(source);
		}
		if (bind_to_list)
		{
			bind_to_lists(label, bind_to_list);
			g_free(bind_to_list);
		}
		gtk_box_pack_start(GTK_BOX(parent),frame,TRUE,TRUE,0);
	}
}

