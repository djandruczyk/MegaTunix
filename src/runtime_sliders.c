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
  \file src/runtime_sliders.c
  \ingroup CoreMtx
  \brief Handles the Runtime sliders present on the runtime tab as well as the
  VE/Spark/Afr/Boost 3D displays
  \author David Andruczyk
  */

#include <api-versions.h>
#include <conversions.h>
#include <debugging.h>
#include <getfiles.h>
#include <glade/glade-xml.h>
#include <init.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <listmgmt.h>
#include <notifications.h>
#include <progress.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <runtime_sliders.h>
#include <stdio.h>
#include <stdlib.h>
#include <warmwizard_gui.h>
#include <widgetmgmt.h>
#include <xmlbase.h>

extern gconstpointer *global_data;

/*!
  \brief load_rt_sliders() is called to load up the runtime slider 
  configuration from the file specified in the firmware's interrogation 
  profile, and populate the gui with the newly created sliders.
  */
G_MODULE_EXPORT void load_rt_sliders(void)
{
	GHashTable *rt_sliders = NULL;
	gchar *filename = NULL;
	xmlDoc * doc = NULL;
	xmlNode *root_element = NULL;
	gboolean res = FALSE;
	gchar *pathstub = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!DATA_GET(global_data,"interrogated"))
	{
		MTXDBG(CRITICAL,_("ERROR, NOT interrogated, returning!\n"));
		EXIT();
		return;
	}

	if (DATA_GET(global_data,"leaving"))
	{
		MTXDBG(CRITICAL,_("ERROR, \"leaving\" set, returning!\n"));
		EXIT();
		return;
	}
	if (!DATA_GET(global_data,"rtvars_loaded"))
	{
		MTXDBG(CRITICAL,_("CRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n"));
		EXIT();
		return;
	}
	set_title(g_strdup(_("Loading RT Sliders...")));
	rt_sliders = (GHashTable *)DATA_GET(global_data,"rt_sliders");
	if (!rt_sliders)
	{
		rt_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);
		DATA_SET_FULL(global_data,"rt_sliders",rt_sliders,g_hash_table_destroy);
	}
	pathstub = g_build_filename(RTSLIDERS_DATA_DIR,firmware->sliders_map_file,NULL);
	filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,"xml");
	g_free(pathstub);
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
	res = load_rts_xml_elements(root_element,"rt_rts",rt_sliders,0,RUNTIME_TAB);
	if (!res)
		MTXDBG(CRITICAL,_("Runtime Sliders XML parse/load failure\n"));
	xmlFreeDoc(doc);
	xmlCleanupParser();

	EXIT();
	return;
}


/*!
  \brief load_ww_sliders() is called to load up the runtime slider 
  configurations from the file specified in the firmware's interrogation 
  profile, and populate the gui with the newly created sliders.
  */
G_MODULE_EXPORT void load_ww_sliders(void)
{
	GHashTable *ww_sliders = NULL;
	gchar *filename = NULL;
	xmlDoc * doc = NULL;
	xmlNode *root_element = NULL;
	gboolean res = FALSE;
	gchar *pathstub = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!DATA_GET(global_data,"interrogated"))
	{
		MTXDBG(CRITICAL,_("ERROR, NOT interrogated, returning!\n"));
		EXIT();
		return;
	}

	if (DATA_GET(global_data,"leaving"))
	{
		MTXDBG(CRITICAL,_("ERROR, \"leaving\" set, returning!\n"));
		EXIT();
		return;
	}
	if (!DATA_GET(global_data,"rtvars_loaded"))
	{
		MTXDBG(CRITICAL,_("CRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n"));
		EXIT();
		return;
	}
	set_title(g_strdup(_("Loading RT Sliders...")));
	ww_sliders = (GHashTable *)DATA_GET(global_data,"ww_sliders");
	if (!ww_sliders)
	{
		ww_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);
		DATA_SET_FULL(global_data,"ww_sliders",ww_sliders,g_hash_table_destroy);
	}

	pathstub = g_build_filename(RTSLIDERS_DATA_DIR,firmware->sliders_map_file,NULL);
	filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,"xml");
	g_free(pathstub);
	LIBXML_TEST_VERSION
		doc = xmlReadFile(filename, NULL, 0);
	g_free(filename);
	if (doc == NULL)
	{
		printf(_("error: could not parse file %s\n"),filename);
		EXIT();
		return;
	}
	res = load_rts_xml_elements(root_element,"ww_rts",ww_sliders,0,WARMUP_WIZ_TAB);
	if (!res)
		MTXDBG(CRITICAL,_("Warmup Wizard Sliders XML parse/load failure\n"));
	xmlFreeDoc(doc);
	xmlCleanupParser();

	EXIT();
	return;
}


/*!
  \brief loads runtime status XML elements by iterating down recursively 
  through the passed XML node
  \param a_node is the current XML node we are traversing
  \param prefix is the prefix we are searching for in the XML (section)
  \param hash is the pointer to hashtable to store the created RTS structure
  \param table_num is the table number within this tab
  \param tab_id is the Tab identifier enumeration
  \returns FALSE when at end of file, TRUE otherwise
  */
G_MODULE_EXPORT gboolean load_rts_xml_elements(xmlNode *a_node, const gchar *prefix, GHashTable *hash, gint table_num, TabIdent tab_id)
{
	xmlNode *cur_node = NULL;

	ENTER();
	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"api") == 0)
				if (!xml_api_check(cur_node,RT_SLIDERS_MAJOR_API,RT_SLIDERS_MINOR_API))
				{
					MTXDBG(CRITICAL,_("API mismatch, won't load this file!!\n"));
					EXIT();
					return FALSE;
				}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,prefix) == 0)
				load_rts(cur_node,hash,table_num,tab_id);
		}
		if (!load_rts_xml_elements(cur_node->children,prefix,hash,table_num,tab_id))
		{
			EXIT();
			return FALSE;
		}
	}
	EXIT();
	return TRUE;
}


/*!
  \brief loads runtime status details from XML, creates the slider and inserts
  it into the hash table
  \param node is the current XML node
  \param hash is the hashtable to stick the resulting RTS structure into
  \param table_num is the table number on this tab
  \param tab_id is the Tab identification enumeration
  */
G_MODULE_EXPORT void load_rts(xmlNode *node, GHashTable *hash, gint table_num, TabIdent tab_id)
{
	gchar *slider_name = NULL;
	gchar *source = NULL;
	gint row = 0;
	gint table = 0;
	Rt_Slider *slider = NULL;
	xmlNode *cur_node = NULL;

	ENTER();
	if (!node->children)
	{
		printf(_("ERROR, load_rts, xml node is empty!!\n"));
		EXIT();
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"slider_name") == 0)
				generic_xml_gchar_import(cur_node,&slider_name);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"source") == 0)
				generic_xml_gchar_import(cur_node,&source);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"row") == 0)
				generic_xml_gint_import(cur_node,&row);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"table") == 0)
				generic_xml_gint_import(cur_node,&table);
		}
		cur_node = cur_node->next;
	}
	if ((slider_name) && (source))
		slider = add_slider(slider_name,table,table_num,row,source,tab_id);
	if (slider)
	{
		if (g_hash_table_lookup(hash,slider_name) == NULL)
			g_hash_table_insert(hash,g_strdup(slider_name),(gpointer)slider);
	}
	g_free(slider_name);
	g_free(source);
	EXIT();
	return;
}


/*!
  \brief load_ve3d_sliders() is called from 3d_vetable.c to load up the sliders
  specific to the 3D Table views. 
  \param table_num is the table number passed to load sliders for
  */
G_MODULE_EXPORT void load_ve3d_sliders(gint table_num)
{
	gchar *filename = NULL;
	xmlDoc * doc = NULL;
	xmlNode *root_element = NULL;
	gboolean res = FALSE;
	GHashTable **ve3d_sliders;
	gchar *pathstub = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!(DATA_GET(global_data,"rtvars_loaded")) || 
			(!(DATA_GET(global_data,"tabs_loaded"))))
	{
		MTXDBG(CRITICAL,_("CRITICAL ERROR, Tabs not loaded OR Realtime Variable definitions NOT LOADED!!!\n"));
		EXIT();
		return;
	}

	ve3d_sliders = (GHashTable **)DATA_GET(global_data,"ve3d_sliders");
	if (!ve3d_sliders)
		ve3d_sliders = g_new0(GHashTable *,firmware->total_tables);
	DATA_SET(global_data,"ve3d_sliders",ve3d_sliders);

	if (!ve3d_sliders[table_num])
		ve3d_sliders[table_num] = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);

	pathstub = g_build_filename(RTSLIDERS_DATA_DIR,firmware->sliders_map_file,NULL);
	filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,"xml");
	g_free(pathstub);
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
	res = load_rts_xml_elements(root_element,"ve3d_rts",ve3d_sliders[table_num],table_num,VE3D_VIEWER_TAB);
	if (!res)
		MTXDBG(CRITICAL,_("Runtime Sliders XML parse/load failure\n"));
	xmlFreeDoc(doc);
	xmlCleanupParser();

	EXIT();
	return;
}


/*!
  \brief add_slider() creates the slider from the passed data, and attaches
  it the the gui.
  \param ctrl_name is the name of the slider as defined in the config file
  \param tbl is the table number to bind this slider to
  \param table_num is  the table_num from the firmware that this slider is
  bound to. (used for the sliders on the 3D view)
  \param row is the row of the table (tbl) that this slider goes on
  \param source is the data source for this slider 
  \param ident is the enumeration of the page this slider goes on
  \returns a Struct Rt_Slider *
  */
G_MODULE_EXPORT Rt_Slider * add_slider(gchar *ctrl_name, gint tbl, gint table_num, gint row, gchar *source, TabIdent ident)
{
	GtkSizeGroup *size_group_left = NULL;
	GtkSizeGroup *size_group_right = NULL;
	Rt_Slider *slider = NULL;
	GtkWidget *label = NULL;
	GtkWidget *pbar = NULL;
	GtkWidget *table = NULL;
	GtkWidget *hbox = NULL;
	gchar * name = NULL;
	Rtv_Map *rtv_map = NULL;
	gconstpointer *object = NULL;

	ENTER();
	size_group_left = DATA_GET(global_data,"size_group_left");
	if (!size_group_left)
	{
		size_group_left = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		DATA_SET_FULL(global_data,"size_group_left",size_group_left,g_object_unref);
	}
	size_group_right = DATA_GET(global_data,"size_group_right");
	if (!size_group_right)
	{
		size_group_right = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		DATA_SET_FULL(global_data,"size_group_right",size_group_right,g_object_unref);
	}
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!(object))
	{
		MTXDBG(CRITICAL,_("Request to create slider for non-existant datasource \"%s\"\n"),source);
		EXIT();
		return NULL;
	}

	if (ident == RUNTIME_TAB)
		name = g_strdup_printf("runtime_rt_table%i",tbl);
	else if (ident == WARMUP_WIZ_TAB)
		name = g_strdup_printf("ww_rt_table%i",tbl);
	else if (ident == VE3D_VIEWER_TAB)
		name = g_strdup_printf("ve3d_rt_table%i_%i",tbl,table_num);
	else
	{
		MTXDBG(CRITICAL,_("Page ident passed is not handled, ERROR, widget add aborted\n"));
		EXIT();
		return NULL;
	}
	table = lookup_widget(name);
	if (!table)
	{
		MTXDBG(CRITICAL,_("Table \"%s\" was not found, RuntimeSlider map or runtime datamap has a typo\n"),name);
		g_free(name);
		EXIT();
		return NULL;
	}
	g_free(name);

	slider = (Rt_Slider *)g_malloc0(sizeof(Rt_Slider));
	slider->ctrl_name = g_strdup(ctrl_name);
	slider->tbl = tbl;
	slider->table_num = table_num;
	slider->row = row;
	slider->last = 0.0;
	slider->type = MTX_PROGRESS;
	slider->friendly_name = (gchar *) DATA_GET(object,"dlog_gui_name");
	slider->temp_dep = (GBOOLEAN)DATA_GET(object,"temp_dep");
	if ((gchar *)DATA_GET(object,"real_lower"))
		slider->lower = (GINT)strtol((gchar *)DATA_GET(object,"real_lower"),NULL,10);
	else
		MTXDBG(CRITICAL,_("No \"real_lower\" value defined for control name %s, datasource %s\n"),ctrl_name,source);
	if ((gchar *)DATA_GET(object,"real_upper"))
		slider->upper = (GINT)strtol((gchar *)DATA_GET(object,"real_upper"),NULL,10);
	else
		MTXDBG(CRITICAL,_("No \"real_upper\" value defined for control name %s, datasource %s\n"),ctrl_name,source);
	slider->history = (GArray *) DATA_GET(object,"history");
	slider->object = object;
	hbox = gtk_hbox_new(FALSE,5);

	label = gtk_label_new(NULL);
	slider->label = label;
	gtk_label_set_markup(GTK_LABEL(label),slider->friendly_name);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

	label = gtk_entry_new();
	gtk_entry_set_has_frame(GTK_ENTRY(label),FALSE);
	gtk_entry_set_width_chars(GTK_ENTRY(label),6);
	gtk_entry_set_alignment(GTK_ENTRY(label),1);
	gtk_editable_set_editable(GTK_EDITABLE(label),FALSE);
	/* PRELIGHT seems to not give the box as NORMAL does, not sure why */
	gtk_widget_modify_base(GTK_WIDGET(label),GTK_STATE_NORMAL,&gtk_widget_get_style(slider->label)->bg[GTK_STATE_PRELIGHT]);

	slider->textval = label;
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

	if ((ident == RUNTIME_TAB) || (ident == VE3D_VIEWER_TAB))
	{
		if (((tbl+1) % 2) == 0)
			gtk_size_group_add_widget(size_group_right,hbox);
		else
			gtk_size_group_add_widget(size_group_left,hbox);
	}

	gtk_table_attach (GTK_TABLE (table),hbox,
			0,2,slider->row,(slider->row)+1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);

	pbar = mtx_progress_bar_new();
	/* 1.1 Seconds peak hold time */
	mtx_progress_bar_set_hold_time(MTX_PROGRESS_BAR(pbar),1100);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);

	gtk_table_attach (GTK_TABLE (table),pbar,
			2,3,slider->row,(slider->row)+1,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 0, 0);
	slider->pbar = pbar;

	slider->parent = table;
	gtk_widget_show_all(slider->parent);

	EXIT();
	return slider;
}


/*!
  \brief register_rt_range() creates the slider from the passed data, 
  and attaches it the the gui. This is called during gui tab loading to embed
  sliders into regular tabs.
  \param widget is the widget defined in Gui datamap file. Used
  to load all the necessary attributes to stick the control in the right place.
  */
G_MODULE_EXPORT void register_rt_range(GtkWidget * widget)
{
	gconstpointer *object = NULL;
	Rtv_Map *rtv_map = NULL;
	GHashTable *rt_sliders = NULL;
	GHashTable *aw_sliders = NULL;
	GHashTable *ww_sliders = NULL;
	GHashTable *enr_sliders = NULL;
	gchar * source = NULL;
	const gchar *name = NULL;
	TabIdent ident;
	Rt_Slider *slider = NULL;

	ENTER();
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	source = (gchar *)OBJ_GET(widget,"source");
	ident = (TabIdent)(GINT)OBJ_GET(widget,"tab_ident");
	name = glade_get_widget_name(widget);
		
	g_return_if_fail(rtv_map);
	g_return_if_fail(source);


	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,source);

	rt_sliders = (GHashTable *)DATA_GET(global_data,"rt_sliders");
	aw_sliders = (GHashTable *)DATA_GET(global_data,"aw_sliders");
	ww_sliders = (GHashTable *)DATA_GET(global_data,"ww_sliders");
	enr_sliders = (GHashTable *)DATA_GET(global_data,"enr_sliders");
	if (!rt_sliders)
	{
		rt_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);
		DATA_SET_FULL(global_data,"rt_sliders",rt_sliders,g_hash_table_destroy);
	}
	if (!aw_sliders)
	{
		aw_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);
		DATA_SET_FULL(global_data,"aw_sliders",aw_sliders,g_hash_table_destroy);
	}
	if (!ww_sliders)
	{
		ww_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);
		DATA_SET_FULL(global_data,"ww_sliders",ww_sliders,g_hash_table_destroy);
	}
	if (!enr_sliders)
	{
		enr_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);
		DATA_SET_FULL(global_data,"enr_sliders",enr_sliders,g_hash_table_destroy);
	}
	
	if  (!(object))
	{
		MTXDBG(CRITICAL,_("ERROR! There is no datasource named \"%s\", Check config of widget %s\n"),source,(name == NULL ? "undefined":name));
		EXIT();
		return;
	}
	slider = (Rt_Slider *)g_malloc0(sizeof(Rt_Slider));
	slider->ctrl_name = g_strdup((name == NULL ? "undefined":name));
	slider->tbl = -1;
	slider->table_num = -1;
	slider->row = -1;
	slider->history = (GArray *) DATA_GET(object,"history");
	slider->friendly_name = (gchar *) DATA_GET(object,"dlog_gui_name");
	slider->temp_dep = (GBOOLEAN)DATA_GET(object,"temp_dep");
	if ((gchar *)DATA_GET(object,"real_lower"))
		slider->lower = (GINT)strtol((gchar *)DATA_GET(object,"real_lower"),NULL,10);
	else
		printf(_("No \"real_lower\" value defined for control name %s, datasource %s\n"),slider->ctrl_name,source);
	if ((gchar *)DATA_GET(object,"real_upper"))
		slider->upper = (GINT)strtol((gchar *)DATA_GET(object,"real_upper"),NULL,10);
	else
		printf(_("No \"real_upper\" value defined for control name %s, datasource %s\n"),slider->ctrl_name,source);
	slider->object = object;
	slider->textval = NULL;
	if (GTK_IS_SCALE(widget))
	{
		slider->type = MTX_RANGE;
		slider->pbar = widget;
	}
	/* generic container (Table/box HOLDING a mtx pbar */
	else if (GTK_IS_CONTAINER(widget))
	{
		GtkProgressBarOrientation orient;
		/* We don't like GTK+'s progress bar, so rip it out and 
		 * stick in my custom version instead.  Get the orientation
		 * first...
		 */
		orient = (GtkProgressBarOrientation)(GINT)OBJ_GET(widget,"orientation");
		slider->pbar = mtx_progress_bar_new();
		mtx_progress_bar_set_hold_time(MTX_PROGRESS_BAR(slider->pbar),(GINT)DATA_GET(global_data,"pbar_hold_time"));
		gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(slider->pbar),orient);
		gtk_container_add(GTK_CONTAINER(widget),slider->pbar);
		slider->type = MTX_PROGRESS;
	}

	switch (ident)
	{
		case RUNTIME_TAB:
			printf("inserted slider into runtime tab!\n");
			g_hash_table_insert(rt_sliders,g_strdup(slider->ctrl_name),(gpointer)slider);
			break;
		case ENRICHMENTS_TAB:
			g_hash_table_insert(enr_sliders,g_strdup(slider->ctrl_name),(gpointer)slider);
			break;
		case WARMUP_WIZ_TAB:
			g_hash_table_insert(ww_sliders,g_strdup(slider->ctrl_name),(gpointer)slider);
			break;
		case ACCEL_WIZ_TAB:
			g_hash_table_insert(aw_sliders,g_strdup(slider->ctrl_name),(gpointer)slider);
			break;
		default:
			break;
	}
	EXIT();
	return;
}


/*!
  \brief free_ve3d_sliders() frees the sliders associated with the table_num
  passed to it.
  \param table_num is the table_number to free the sliders for
  \returns FALSE
  */
G_MODULE_EXPORT gboolean free_ve3d_sliders(gint table_num)
{
	gchar * widget = NULL;
	GHashTable **tables = NULL;

	ENTER();
	tables = (GHashTable **)DATA_GET(global_data,"ve3d_sliders");
	g_hash_table_destroy(tables[table_num]);
	tables[table_num] = NULL;

	widget = g_strdup_printf("ve3d_rt_table0_%i",table_num);
	deregister_widget(widget);
	g_free(widget);

	widget = g_strdup_printf("ve3d_rt_table1_%i",table_num);
	deregister_widget(widget);
	g_free(widget);
	EXIT();
	return FALSE;
}


/*!
  \brief Runtime slider button handler, not used yet.
  \param widget is unused
  \param event is unused
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean rtslider_button_handler(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*!
  \brief Runtime slider motion handler, not used yet.
  \param widget is unused
  \param event is unused
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean rtslider_motion_handler(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*!
  \brief Accel enrichment slider limit checker, This takes care fo the special 
  0/100 cases where controls should be enabled/disabled as appropriate
  \param widget is the slider/range
  \param data is unused
  \returns FALSE to let other signals run
  */
G_MODULE_EXPORT gboolean ae_slider_check_limits(GtkWidget *widget, gpointer data)
{
	gboolean mapae_ctrl_state = FALSE;
	gboolean tpsae_ctrl_state = FALSE;
	GHashTable *widget_group_states = NULL;
 	gfloat value = gtk_range_get_value(GTK_RANGE(widget));
	ENTER();

	if (value == 0)
		tpsae_ctrl_state = FALSE;
	else
		tpsae_ctrl_state = TRUE;

	if (value == 100)
		mapae_ctrl_state = FALSE;
	else
		mapae_ctrl_state = TRUE;

	widget_group_states = (GHashTable *)DATA_GET(global_data,"widget_group_states");
	g_hash_table_insert(widget_group_states,g_strdup("tps_ae_ctrls"),GINT_TO_POINTER(tpsae_ctrl_state));
	g_hash_table_insert(widget_group_states,g_strdup("map_ae_ctrls"),GINT_TO_POINTER(mapae_ctrl_state));
	g_list_foreach(get_list("tps_ae_ctrls"),alter_widget_state,NULL);
	g_list_foreach(get_list("map_ae_ctrls"),alter_widget_state,NULL);
	EXIT();
	return FALSE;
}


/*!
  \brief Updates the runtime sliders on whichever tab is active, by calling
  rt_update_values for each element of the hashtable
  \param data is unused
  */
G_MODULE_EXPORT gboolean update_rtsliders(gpointer data)
{
	gfloat coolant = 0.0;
	static gfloat last_coolant = 0.0;
	GHashTable **ve3d_sliders;
	GHashTable *hash;
	TabIdent active_page;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ve3d_sliders = (GHashTable **)DATA_GET(global_data,"ve3d_sliders");

	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return FALSE;
	}

	active_page = (TabIdent)(GINT)DATA_GET(global_data,"active_page");
	/* Update all the dynamic RT Sliders */
	if (active_page == RUNTIME_TAB) /* Runtime display is visible */
		if ((hash = (GHashTable *)DATA_GET(global_data,"rt_sliders")))
			g_hash_table_foreach(hash,rt_update_values,NULL);
	if (active_page == ENRICHMENTS_TAB)     /* Enrichments display is up */
		if ((hash = (GHashTable *)DATA_GET(global_data,"enr_sliders")))
			g_hash_table_foreach(hash,rt_update_values,NULL);
	if (active_page == ACCEL_WIZ_TAB)       /* Enrichments display is up */
		if ((hash = (GHashTable *)DATA_GET(global_data,"aw_sliders")))
			g_hash_table_foreach(hash,rt_update_values,NULL);
	if (active_page == WARMUP_WIZ_TAB)      /* Warmup wizard is visible */
	{
		if ((hash = (GHashTable *)DATA_GET(global_data,"ww_sliders")))
			g_hash_table_foreach(hash,rt_update_values,NULL);

		if (!lookup_current_value("cltdeg",&coolant))
			MTXDBG(CRITICAL,_("Error getting current value of \"cltdeg\" from datasource\n"));
		if ((coolant != last_coolant) || 
				(DATA_GET(global_data,"rt_forced_update")))
			warmwizard_update_status(coolant);
		last_coolant = coolant;
	}
	if (ve3d_sliders)       
	{                       
		for (gint i=0;i<firmware->total_tables;i++)
		{               
			if (ve3d_sliders[i])
				g_hash_table_foreach(ve3d_sliders[i],rt_update_values,NULL);
		}               
	}
	EXIT();
	return FALSE;            
}


/*!
  \brief rt_update_values() is called for each runtime slider to update
  it's position and label (label is periodic and not every time due to pango
  speed problems)
  \param key is unused
  \param value is the pointer to Rt_Slider
  \param data is unused
  */
G_MODULE_EXPORT void rt_update_values(gpointer key, gpointer value, gpointer data)
{
	static GRand *rand = NULL;
	static GMutex *rtv_mutex = NULL;
	Rt_Slider *slider = (Rt_Slider *)value;
	gint count = slider->count;
	gint last_upd = slider->last_upd;
	gfloat upper = 0.0;
	gfloat lower = 0.0;
	gint precision = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;

	ENTER();
	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return;
	}

	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");
	if (!rand)
		rand = g_rand_new();
	history = (GArray *)DATA_GET(slider->object,"history");
	if (!history)
	{
		EXIT();
		return;
	}
	if ((GINT)history->len-1 <= 0)
	{
		EXIT();
		return;
	}
	precision = (GINT)DATA_GET(slider->object,"precision");
	g_mutex_lock(rtv_mutex);
	/*printf("runtime_gui history length is %i, current index %i\n",history->len,history->len-1);*/
	current = g_array_index(history, gfloat, history->len-1);
	previous = slider->last;
	slider->last = current;
	g_mutex_unlock(rtv_mutex);

	if (slider->temp_dep)
	{
		upper = temp_to_host((gfloat)slider->upper);
		lower = temp_to_host((gfloat)slider->lower);
	}
	else
	{
		upper = (gfloat)slider->upper;
		lower = (gfloat)slider->lower;
	}

	
	if ((current != previous) || 
			(DATA_GET(global_data,"rt_forced_update")))
	{
		gfloat percentage = (current-lower)/(upper-lower);
		gfloat tmpf = percentage <= 1.0 ? percentage : 1.0;
		tmpf = tmpf >= 0.0 ? tmpf : 0.0;
		switch (slider->type)
		{
			case MTX_PROGRESS:
				mtx_progress_bar_set_fraction(MTX_PROGRESS_BAR
						(slider->pbar),
						tmpf);
				break;
			case MTX_RANGE:
				gtk_range_set_value(GTK_RANGE(slider->pbar),current);
				break;
			default:
				break;
		}


		/* If changed by more than 5% or has been at least 5 
		 * times withot an update or rt_forced_update is set
		 * */
		if ((slider->textval) && ((abs(count-last_upd) > 2) || 
					(DATA_GET(global_data,"rt_forced_update"))))
		{
			tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);

			//gtk_label_set_text(GTK_LABEL(slider->textval),tmpbuf);
			gtk_entry_set_text(GTK_ENTRY(slider->textval),tmpbuf);
			g_free(tmpbuf);
			last_upd = count;
		}
		slider->last_percentage = percentage;
	}
	else if (slider->textval && ((abs(count-last_upd)%g_rand_int_range(rand,25,50)) == 0))
	{
		tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);

		//gtk_label_set_text(GTK_LABEL(slider->textval),tmpbuf);
		gtk_entry_set_text(GTK_ENTRY(slider->textval),tmpbuf);
		g_free(tmpbuf);
		last_upd = count;
	}

	if (last_upd > 5000)
		last_upd = 0;
	count++;
	if (count > 5000)
		count = 0;
	slider->count = count;
	slider->last_upd = last_upd;
	EXIT();
	return;
}
