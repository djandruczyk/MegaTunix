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


#include <apicheck.h>
#include <api-versions.h>
#include <configfile.h>
#include <debugging.h>
#include <defines.h>
#include <firmware.h>
#include <getfiles.h>
#include <glade/glade-xml.h>
#include <glib.h>
#include <init.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <listmgmt.h>
#include <notifications.h>
#include <progress.h>
#include <rtv_map_loader.h>
#include <runtime_sliders.h>
#include <stdio.h>
#include <stdlib.h>
#include <widgetmgmt.h>
#include <xmlbase.h>

static GtkSizeGroup *size_group_left = NULL;
static GtkSizeGroup *size_group_right = NULL;
extern gconstpointer *global_data;


/*!
 \brief load_sliders_pf() is called to load up the runtime slider configurations
 from the file specified in the firmware's interrogation profile, and populate
 the gui with the newly created sliders.
 */
G_MODULE_EXPORT void load_sliders_pf(void)
{
	extern Firmware_Details *firmware;
	GHashTable *rt_sliders = NULL;
	GHashTable *ww_sliders = NULL;
	gchar *filename = NULL;
	xmlDoc * doc = NULL;
	xmlNode *root_element = NULL;
	gboolean res = FALSE;

	if (!DATA_GET(global_data,"interrogated"))
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": load_sliders_pf()\n\tERROR, NOT connected and not interrogated, returning!\n\n"));
		return;
	}

	if ((!DATA_GET(global_data,"tabs_loaded")) || 
			(DATA_GET(global_data,"leaving")))
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": load_sliders_pf()\n\tERROR, tabs not loaded or leaving, returning!\n\n"));
		return;
	}
	if (!(DATA_GET(global_data,"rtvars_loaded")) || 
			(!DATA_GET(global_data,"tabs_loaded")))
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": load_sliders_pf()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n"));
		return;
	}
	gdk_threads_enter();
	set_title(g_strdup(_("Loading RT Sliders...")));
	rt_sliders = DATA_GET(global_data,"rt_sliders");
	ww_sliders = DATA_GET(global_data,"ww_sliders");
	if (!rt_sliders)
	{
		rt_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);
		DATA_SET_FULL(global_data,"rt_sliders",rt_sliders,g_hash_table_destroy);
	}
	if (!ww_sliders)
	{
		ww_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);
		DATA_SET_FULL(global_data,"ww_sliders",ww_sliders,g_hash_table_destroy);
	}


	filename = get_file(g_strconcat(RTSLIDERS_DATA_DIR,PSEP,firmware->sliders_map_file,NULL),g_strdup("xml"));
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
	size_group_left = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	size_group_right = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	res = load_rts_xml_elements(root_element,"rt_rts",rt_sliders,0,RUNTIME_TAB);
	if (!res)
		dbg_func(CRITICAL,g_strdup(__FILE__": load_sliders_pf()\n\tRuntime Sliders XML parse/load failure\n"));
	size_group_left = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	size_group_right = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	res = load_rts_xml_elements(root_element,"ww_rts",ww_sliders,0,WARMUP_WIZ_TAB);
	if (!res)
		dbg_func(CRITICAL,g_strdup(__FILE__": load_sliders_pf()\n\tWarmup Wizard Sliders XML parse/load failure\n"));
	xmlFreeDoc(doc);
	xmlCleanupParser();

	gdk_threads_leave();
	return;
}

G_MODULE_EXPORT gboolean load_rts_xml_elements(xmlNode *a_node, const gchar *prefix, GHashTable *hash, gint table_num, TabIdent tab_id)
{
	xmlNode *cur_node = NULL;

	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"api") == 0)
				if (!xml_api_check(cur_node,RT_SLIDERS_MAJOR_API,RT_SLIDERS_MINOR_API))
				{
					dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_rtt_xml_elements()\n\tAPI mismatch, won't load this file!!\n"));
					return FALSE;
				}
			if (g_strcasecmp((gchar *)cur_node->name,prefix) == 0)
				load_rts(cur_node,hash,table_num,tab_id);
		}
		if (!load_rts_xml_elements(cur_node->children,prefix,hash,table_num,tab_id))
			return FALSE;
	}
	return TRUE;
}

G_MODULE_EXPORT void load_rts(xmlNode *node,GHashTable *hash,gint table_num,TabIdent tab_id)
{
	gchar *slider_name = NULL;
	gchar *source = NULL;
	gint row = 0;
	gint table = 0;
	Rt_Slider *slider = NULL;
	xmlNode *cur_node = NULL;

	if (!node->children)
	{
		printf(_("ERROR, load_rts, xml node is empty!!\n"));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"slider_name") == 0)
				generic_xml_gchar_import(cur_node,&slider_name);
			if (g_strcasecmp((gchar *)cur_node->name,"source") == 0)
				generic_xml_gchar_import(cur_node,&source);
			if (g_strcasecmp((gchar *)cur_node->name,"row") == 0)
				generic_xml_gint_import(cur_node,&row);
			if (g_strcasecmp((gchar *)cur_node->name,"table") == 0)
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
}


/*!
 \brief load_ve3d_sliders() is called from 3d_vetable.c to load up the sliders
 specific to the 3D Table views. 
 \param table_num (gint) the table number passed to load sliders for
 */
G_MODULE_EXPORT void load_ve3d_sliders(gint table_num)
{
	extern Firmware_Details *firmware;
	gchar *filename = NULL;
	xmlDoc * doc = NULL;
	xmlNode *root_element = NULL;
	gboolean res = FALSE;
	GHashTable **ve3d_sliders;

	if (!(DATA_GET(global_data,"rtvars_loaded")) || 
			(!(DATA_GET(global_data,"tabs_loaded"))))
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": load_sliders_pf()\n\tCRITICAL ERROR, Tabs not loaded OR Realtime Variable definitions NOT LOADED!!!\n\n"));
		return;
	}

	ve3d_sliders = DATA_GET(global_data,"ve3d_sliders");
	if (!ve3d_sliders)
		ve3d_sliders = g_new0(GHashTable *,firmware->total_tables);
	DATA_SET(global_data,"ve3d_sliders",ve3d_sliders);

	if (!ve3d_sliders[table_num])
		ve3d_sliders[table_num] = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,dealloc_slider);

	filename = get_file(g_strconcat(RTSLIDERS_DATA_DIR,PSEP,firmware->sliders_map_file,NULL),g_strdup("xml"));
	LIBXML_TEST_VERSION
		doc = xmlReadFile(filename, NULL, 0);
	g_free(filename);
	if (doc == NULL)
	{
		printf(_("error: could not parse file %s\n"),filename);
		return;
	}
	root_element = xmlDocGetRootElement(doc);
	size_group_left = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	size_group_right = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	res = load_rts_xml_elements(root_element,"ve3d_rts",ve3d_sliders[table_num],table_num,VE3D_VIEWER_TAB);
	if (!res)
		dbg_func(CRITICAL,g_strdup(__FILE__": load_sliders_pf()\n\tRuntime Sliders XML parse/load failure\n"));
	xmlFreeDoc(doc);
	xmlCleanupParser();

	return;
}


/*!
 \brief add_slider() creates the slider from the passed data, and attaches
 it the the gui.
 \param ctrl_name (gchar *) name of the slider as defined in the config file
 \param tbl (gint) table number to bind this slider to
 \param table_num (gint) the table_num from the firmware that this slider is
 bound to. (used for the sliders on the 3D view)
 \param row (gint) row of the table (tbl) that this slider goes on
 \param source (gchar *) data source for this slider 
 \param ident (TabIdent) enumeration of the page this slider goes on
 \returns a Struct Rt_Slider *
 */
G_MODULE_EXPORT Rt_Slider *  add_slider(gchar *ctrl_name, gint tbl, gint table_num, gint row, gchar *source, TabIdent ident)
{
	Rt_Slider *slider = NULL;
	GtkWidget *label = NULL;
	GtkWidget *pbar = NULL;
	GtkWidget *table = NULL;
	GtkWidget *hbox = NULL;
	gchar * name = NULL;
	extern Rtv_Map *rtv_map;
	gconstpointer *object = NULL;


	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!(object))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": ERROR!: add_slider()\n\t Request to create slider for non-existant datasource \"%s\"\n",source));
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
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": add_slider()\n\tpage ident passed is not handled, ERROR, widget add aborted\n"));
		return NULL;
	}
	table = lookup_widget(name);
	if (!table)
	{
		//		dbg_func(CRITICAL,g_strdup_printf(__FILE__": add_slider()\n\t table \"%s\" was not found, RuntimeSlider map or runtime datamap has a typo\n",name));
		g_free(name);
		return NULL;
	}
	g_free(name);

	slider = g_malloc0(sizeof(Rt_Slider));
	slider->ctrl_name = g_strdup(ctrl_name);
	slider->tbl = tbl;
	slider->table_num = table_num;
	slider->row = row;
	slider->last = 0.0;
	slider->class = MTX_PROGRESS;
	slider->friendly_name = (gchar *) DATA_GET(object,"dlog_gui_name");
	if ((gchar *)DATA_GET(object,"real_lower"))
		slider->lower = (gint)strtol(DATA_GET(object,"real_lower"),NULL,10);
	else
		dbg_func(CRITICAL,g_strdup_printf(_(__FILE__"No \"real_lower\" value defined for control name %s, datasource %s\n"),ctrl_name,source));
	if ((gchar *)DATA_GET(object,"real_upper"))
		slider->upper = (gint)strtol(DATA_GET(object,"real_upper"),NULL,10);
	else
		dbg_func(CRITICAL,g_strdup_printf(_(__FILE__"No \"real_upper\" value defined for control name %s, datasource %s\n"),ctrl_name,source));
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
	gtk_entry_set_width_chars(GTK_ENTRY(label),5);
	gtk_entry_set_alignment(GTK_ENTRY(label),1);
	gtk_entry_set_editable(GTK_ENTRY(label),FALSE);
	gtk_widget_modify_base(GTK_WIDGET(label),GTK_STATE_NORMAL,&table->style->bg[GTK_STATE_NORMAL]);

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

	return slider;
}


/*!
 \brief register_rt_range() creates the slider from the passed data, 
 and attaches it the the gui. This is called during gui tab loading to embed
 sliders into regular tabs.
 \param widget (GtkWidget *) name of widget defined in Gui datamap file. Used
 to load al lthe necessary attributes to stick the control in the right place.
 */
G_MODULE_EXPORT void register_rt_range(GtkWidget * widget)
{
	gconstpointer * object = NULL;
	extern Rtv_Map *rtv_map;
	GtkWidget *parent = NULL;
	GtkProgressBarOrientation orient;
	GHashTable *rt_sliders = NULL;
	GHashTable *aw_sliders = NULL;
	GHashTable *ww_sliders = NULL;
	GHashTable *enr_sliders = NULL;
	Rt_Slider *slider = g_malloc0(sizeof(Rt_Slider));
	gchar * source = (gchar *)OBJ_GET(widget,"source");
	TabIdent ident = (TabIdent)OBJ_GET(widget,"tab_ident");
		
	if (!rtv_map)
		return;
	object = g_hash_table_lookup(rtv_map->rtv_hash,source);

	rt_sliders = DATA_GET(global_data,"rt_sliders");
	aw_sliders = DATA_GET(global_data,"aw_sliders");
	ww_sliders = DATA_GET(global_data,"ww_sliders");
	enr_sliders = DATA_GET(global_data,"enr_sliders");
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
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": register_rt_range()\n\t ERROR! There is no datasource named \"%s\", Check config of widget %s\n",source,glade_get_widget_name(widget)));
		return;
	}
	slider->ctrl_name = g_strdup(glade_get_widget_name(widget));
	slider->tbl = -1;
	slider->table_num = -1;
	slider->row = -1;
	slider->history = (GArray *) DATA_GET(object,"history");
	slider->friendly_name = (gchar *) DATA_GET(object,"dlog_gui_name");
	if ((gchar *)DATA_GET(object,"real_lower"))
		slider->lower = (gint)strtol(DATA_GET(object,"real_lower"),NULL,10);
	else
		printf(_("No \"real_lower\" value defined for control name %s, datasource %s\n"),slider->ctrl_name,source);
	if ((gchar *)DATA_GET(object,"real_upper"))
		slider->upper = (gint)strtol(DATA_GET(object,"real_upper"),NULL,10);
	else
		printf(_("No \"real_upper\" value defined for control name %s, datasource %s\n"),slider->ctrl_name,source);
	slider->object = object;
	slider->textval = NULL;
	if (GTK_IS_SCALE(widget))
		slider->class = MTX_RANGE;
	else if (GTK_IS_PROGRESS(widget))
	{
		/* We don't like GTK+'s progress bar, so rip it out and 
		 * stick in my custom version instead.  Get the orientation
		 * first...
		 */
		orient = gtk_progress_bar_get_orientation(GTK_PROGRESS_BAR(widget));
		parent = gtk_widget_get_parent(widget);
		gtk_widget_destroy(widget);
		widget = mtx_progress_bar_new();
		/* 1.1 Seconds peak hold time */
		mtx_progress_bar_set_hold_time(MTX_PROGRESS_BAR(widget),1100);
		gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(widget),
				orient);
		gtk_container_add(GTK_CONTAINER(parent),widget);
		slider->class = MTX_PROGRESS;
	}
	slider->pbar = widget;

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


}


/*!
 \brief free_ve3d_sliders() frees the sliders associated with the table_num
 passed to it.
 \param table_num (gint) the table_number to free the sliders for
 \returns FALSE
 */
G_MODULE_EXPORT gboolean free_ve3d_sliders(gint table_num)
{
	gchar * widget = NULL;
	GHashTable **tables = NULL;

	tables = DATA_GET(global_data,"ve3d_sliders");
	g_hash_table_destroy(tables[table_num]);
	tables[table_num] = NULL;

	widget = g_strdup_printf("ve3d_rt_table0_%i",table_num);
	deregister_widget(widget);
	g_free(widget);

	widget = g_strdup_printf("ve3d_rt_table1_%i",table_num);
	deregister_widget(widget);
	g_free(widget);
	return FALSE;
}


G_MODULE_EXPORT gboolean rtslider_button_handler(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	return TRUE;
}


G_MODULE_EXPORT gboolean rtslider_motion_handler(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	return TRUE;
}


G_MODULE_EXPORT gboolean ae_slider_check_limits(GtkWidget *widget, gpointer data)
{
	gboolean mapae_ctrl_state = FALSE;
	gboolean tpsae_ctrl_state = FALSE;
	extern GHashTable *widget_group_states;
	gfloat value = gtk_range_get_value(GTK_RANGE(widget));

	if (value == 0)
		tpsae_ctrl_state = FALSE;
	else
		tpsae_ctrl_state = TRUE;

	if (value == 100)
		mapae_ctrl_state = FALSE;
	else
		mapae_ctrl_state = TRUE;

	g_hash_table_insert(widget_group_states,g_strdup("tps_ae_ctrls"),GINT_TO_POINTER(tpsae_ctrl_state));
	g_hash_table_insert(widget_group_states,g_strdup("map_ae_ctrls"),GINT_TO_POINTER(mapae_ctrl_state));
	g_list_foreach(get_list("tps_ae_ctrls"),alter_widget_state,NULL);
	g_list_foreach(get_list("map_ae_ctrls"),alter_widget_state,NULL);
	return FALSE;

}

