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
  \file src/runtime_status.c
  \ingroup CoreMtx
  \brief Handles the Runtime status present on the runtime tab as well as the
  floating window
  \author David Andruczyk
  */

#include <args.h>
#include <api-versions.h>
#include <core_gui.h>
#include <debugging.h>
#include <glade/glade-xml.h>
#include <getfiles.h>
#include <gui_handlers.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <listmgmt.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <runtime_status.h>
#include <stdio.h>
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
	gchar *pathstub = NULL;
	CmdLineArgs *args =  NULL;
	
	ENTER();
	args = (CmdLineArgs *)DATA_GET(global_data,"args");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(firmware);
	g_return_if_fail(args);

	if (!(DATA_GET(global_data,"interrogated")))
	{
		EXIT();
		return;
	}
	if (!firmware->status_map_file)
	{
		MTXDBG(CRITICAL,_("firmware->status_map_file is UNDEFINED, exiting status window creation routine!!!!\n"));
		EXIT();
		return;
	}

	set_title(g_strdup(_("Loading RT Status...")));
	pathstub = g_build_filename(RTSTATUS_DATA_DIR,firmware->status_map_file,NULL);
	filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,"xml");
	g_free(pathstub);
	if (!filename)
	{
		MTXDBG(CRITICAL,_("File \"%s.xml\" not found!!, exiting function\n"),firmware->status_map_file);
		set_title(g_strdup(_("ERROR RT Statusfile DOES NOT EXIST!!!")));
		EXIT();
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
	if (g_ascii_strcasecmp(firmware->actual_signature,DATA_GET(global_data,"last_signature")) == 0)
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
		EXIT();
		return;
	}

	root_element = xmlDocGetRootElement(doc);
	xml_result = load_status_xml_elements(root_element,parent);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	if (xml_result == FALSE)
		gtk_widget_destroy(window);
	else if (args->hide_status)
		gtk_widget_hide(window);
	else
		gtk_widget_show_all(window);

	set_title(g_strdup(_("RT Status Loaded...")));
	EXIT();
	return;
}


/*!
  \brief Traverses the runtime status XML config file and creates the RTS 
  structures and adds them to the parent
  \param a_node is the XML node
  \param parent is the parent widget thisrts entity shouldbe placed in
  \returns FALSE at EOF, TRUE otherwise
  */
G_MODULE_EXPORT gboolean load_status_xml_elements(xmlNode *a_node, GtkWidget *parent)
{
	xmlNode *cur_node = NULL;

	ENTER();
	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"api") == 0)
				if (!xml_api_check(cur_node,RT_STATUS_MAJOR_API,RT_STATUS_MINOR_API))
				{
					MTXDBG(CRITICAL,_("API mismatch, won't load this file!!\n"));
					EXIT();
					return FALSE;
				}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"status") == 0)
				load_status(cur_node,parent);
		}
		if (!load_status_xml_elements(cur_node->children,parent))
		{
			EXIT();
			return FALSE;
		}
	}
	EXIT();
	return TRUE;
}


/*!
  \brief loads the runtime status XML
  \param node is the XML node the start from
  \param parent is the parent container widget
  */
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

	ENTER();
	if (!node->children)
	{
		printf(_("ERROR, load_potential_args, xml node is empty!!\n"));
		EXIT();
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"label") == 0)
				generic_xml_gchar_import(cur_node,&txt);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"active_fg") == 0)
				generic_xml_gchar_import(cur_node,&active_fg);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"inactive_fg") == 0)
				generic_xml_gchar_import(cur_node,&inactive_fg);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"source") == 0)
				generic_xml_gchar_import(cur_node,&source);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"bind_to_list") == 0)
				generic_xml_gchar_import(cur_node,&bind_to_list);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"bitval") == 0)
				generic_xml_gint_import(cur_node,&bitval);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"bitmask") == 0)
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
			OBJ_SET_FULL(label,"source",g_strdup(source),g_free);
			g_free(source);
		}
		if (bind_to_list)
		{
			bind_to_lists(label, bind_to_list);
			g_free(bind_to_list);
		}
		gtk_box_pack_start(GTK_BOX(parent),frame,TRUE,TRUE,0);
	}
	EXIT();
	return;
}


/*!
  \brief update_rtstatus() updates all of the runtime status 
  indicators on the gui
  */
G_MODULE_EXPORT gboolean update_rtstatus(void)
{
	static gint count = 0;
	static gboolean conn_status = FALSE;
	gboolean curr_conn_status = (GBOOLEAN)DATA_GET(global_data,"connected");

	ENTER();
	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return FALSE;
	}

	if (!DATA_GET(global_data,"interrogated"))
	{
		EXIT();
		return FALSE;
	}

	count++;
	if (conn_status != curr_conn_status)
	{
		g_list_foreach(get_list("connected_widgets"),set_widget_sensitive,GINT_TO_POINTER(curr_conn_status));
		set_connected_icons_state(curr_conn_status);
		conn_status = curr_conn_status;
		DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	}

	g_list_foreach(get_list("runtime_status"),rt_update_status,NULL);
	g_list_foreach(get_list("ww_status"),rt_update_status,NULL);

	if (count > 60 )
		count = 0;

	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(FALSE));
	EXIT();
	return FALSE;
}


/*!
  \brief reset_runtime_statue() sets all of the status indicators to OFF
  to reset the display
  */
G_MODULE_EXPORT void reset_runtime_status(void)
{
	ENTER();
	/* Runtime screen */
	g_list_foreach(get_list("runtime_status"),set_widget_sensitive,GINT_TO_POINTER(FALSE));
	/* Warmup Wizard screen */
	g_list_foreach(get_list("ww_status"),set_widget_sensitive,GINT_TO_POINTER(FALSE));
	EXIT();
	return;
}


/*!
  \brief rt_update_status() updates the bitfield based status lights on the 
  runtime/warmupwizard displays
  \param key is the pointer to a widget
  \param data is unused
  */
G_MODULE_EXPORT void rt_update_status(gpointer key, gpointer data)
{
	static gconstpointer *object = NULL;
	static GArray * history = NULL;
	static gchar * source = NULL;
	static const gchar * last_source = "";
	GtkWidget *widget = (GtkWidget *) key;
	gint bitval = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	gint value = 0;
	gfloat tmpf = 0.0;
	gint previous_value = 0;
	Rtv_Map *rtv_map = NULL;
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	ENTER();
	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return;
	}

	g_return_if_fail(GTK_IS_WIDGET(widget));
	g_return_if_fail(rtv_map);
	g_return_if_fail(rtv_map->rtv_hash);

	source = (gchar *)OBJ_GET(widget,"source");
	if (!source)
	{
		EXIT();
		return;
	}
	if ((g_ascii_strcasecmp(source,last_source) != 0))
	{
		object = NULL;
		object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,source);
		if (!object)
		{
			EXIT();
			return;
		}
		history = (GArray *)DATA_GET(object,"history");
		if (!history)
		{
			EXIT();
			return;
		}
	}
	if (!DATA_GET(global_data,"connected"))
	{
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
		EXIT();
		return;
	}

	if (lookup_current_value(source,&tmpf))
		value = (GINT) tmpf;
	else
		MTXDBG(CRITICAL,_("COULD NOT get current value for %s\n"),source);

	previous_value = (GINT)OBJ_GET(widget,"last");
	OBJ_SET(widget,"last",GINT_TO_POINTER(value));

	bitval = (GINT)OBJ_GET(widget,"bitval");
	bitmask = (GINT)OBJ_GET(widget,"bitmask");
	bitshift = get_bitshift(bitmask);

	/* if the value hasn't changed, don't bother continuing */
	if (((value & bitmask) == (previous_value & bitmask)) && 
			(!DATA_GET(global_data,"forced_update")))
	{
		EXIT();
		return;	
	}

	if (((value & bitmask) >> bitshift) == bitval) /* enable it */
		gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);
	else	/* disable it.. */
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
	last_source = source;
	EXIT();
	return;
}


