/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/rtv_map_loader.c
  \ingroup CoreMtx
  \brief Handles loading ECU runtime map files

  Right now this currrently fits all ECU types however there it may make sense
  to generalize this more aside from just dependancy handling so plugins can 
  be leveraged more for ECU specific goodness
  \author David Andruczyk
  */

#include <apicheck.h>
#include <api-versions.h>
#include <debugging.h>
#include <defines.h>
#include <firmware.h>
#include <glade/glade-xml.h>
#include <init.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <multi_expr_loader.h>
#include <getfiles.h>
#include <keyparser.h>
#include <mtxmatheval.h>
#include <notifications.h>
#include <plugin.h>
#include <rtv_map_loader.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <xmlbase.h>

extern gconstpointer *global_data;

/*!
  \brief load_realtime_map_pf() loads the realtime map specified in the detected
  firmware's interrogation profile, and sets up the necessary arrays for storage
  of data coming from the ECU (temporary arrays for the last 50 or so entries)
  */
G_MODULE_EXPORT gboolean load_realtime_map_pf(void )
{
	gchar * filename = NULL;
	GtkWidget *dialog = NULL;
	gboolean xml_result = FALSE;
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	Rtv_Map *rtv_map = NULL;
	gchar *pathstub = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	MTXDBG(RTMLOADER,_("Entered"));
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,FALSE);

	if ((!DATA_GET(global_data,"interrogated")) && 
			(DATA_GET(global_data,"connected")))
	{
		EXIT();
		return FALSE;
	}

	set_title(g_strdup(_("Loading Realtime Map...")));
	pathstub = g_build_filename(REALTIME_MAPS_DATA_DIR,firmware->rtv_map_file,NULL);
	filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,"xml");
	g_free(pathstub);
	if (!filename)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("File \"%s.xml\" not found!!, exiting function\n"),firmware->rtv_map_file);
		set_title(g_strdup(_("ERROR RT Map XML file DOES NOT EXIST!!!")));
		dialog = gtk_message_dialog_new_with_markup(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,_("\n<b>MegaTunix</b> Realtime Variable map \"%s.xml\" for this firmware doesn't appear to be installed correctly!\n\nDid you forget to run <i>\"sudo make install\"</i>??\n You should notify the author of this bug\n\n"),firmware->rtv_map_file);

		g_signal_connect(G_OBJECT(dialog),"response", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"delete_event", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"destroy_event", G_CALLBACK(gtk_main_quit), dialog);
		gtk_widget_show_all(dialog);
		gtk_main();
		if (global_data)
		{
			g_dataset_destroy(global_data);
			g_free(global_data);
		}
		exit(-1);
	}
	LIBXML_TEST_VERSION
	doc = xmlReadFile (filename, NULL, 0);
	g_free(filename);
	if (doc == NULL)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("error: could not parse file %s\n"),filename);
		EXIT();
		return FALSE;
	}
	rtv_map = g_new0(Rtv_Map, 1);
	/* This should free to values with g_list_free, but it causes a fault*/
	rtv_map->offset_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,NULL);
	rtv_map->rtv_list = g_ptr_array_new();
	rtv_map->rtv_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	rtv_map->rtvars_size = firmware->rtvars_size;
	rtv_map->ts_array = g_array_sized_new(FALSE,TRUE, sizeof(GTimeVal),4096);
	rtv_map->derived_total = 0; /* Will be incremented for each one loaded*/
	root_element = xmlDocGetRootElement(doc);
	xml_result = load_rtv_xml_elements(root_element,rtv_map);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	if (xml_result == FALSE)
	{
		set_title(g_strdup(_("ERROR RT Map XML Parse Errors!")));
		MTXDBG(RTMLOADER|CRITICAL,_("Parsing errors with realtime map XML!\n"));
	}
	else
	{
		DATA_SET_FULL(global_data,"rtv_map",rtv_map,dealloc_rtv_map);
		set_title(g_strdup(_("RT Map XML Loaded OK!")));
		DATA_SET(global_data,"rtvars_loaded",GINT_TO_POINTER(TRUE));
	}
	EXIT();
	return TRUE;
}


gboolean load_rtv_xml_elements(xmlNode *a_node, Rtv_Map *map)
{
	xmlNode *cur_node = NULL;

	ENTER();
	/* Iterate down... */
	for (cur_node = a_node; cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"api") == 0)
				if (!xml_api_check(cur_node,RTV_MAP_MAJOR_API,RTV_MAP_MINOR_API))
				{
					MTXDBG(CRITICAL,_("API mismatch, won't load this file!!\n"));
					EXIT();
					return FALSE;
				}
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"realtime_map") == 0)
				load_rtv_defaults(cur_node,map);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"derived") == 0)
				load_derived_var(cur_node,map);
		}
		if (!load_rtv_xml_elements(cur_node->children,map))
		{
			EXIT();
			return FALSE;
		}
	}
	EXIT();
	return TRUE;
}


void load_rtv_defaults(xmlNode *node, Rtv_Map *map)
{
	xmlNode *cur_node = NULL;
	gchar * tmpbuf = NULL;

	ENTER();
	MTXDBG(RTMLOADER,_("load_rtv_defaults!\n"));
	if (!node->children)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("ERROR, load_derived_var, xml node is empty!!\n"));
		EXIT();
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"applicable_signatures") == 0)
				generic_xml_gchar_import(cur_node,&map->applicable_signatures);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"raw_list") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				map->raw_list = parse_keys(tmpbuf,NULL,",");
				g_free(tmpbuf);
			}
		}
		cur_node = cur_node->next; /* Iterate, iterate, iterate... */
	}
	EXIT();
	return;
}


void load_derived_var(xmlNode *node, Rtv_Map *map)
{
	guint i = 0;
	gconstpointer *object = NULL;
	GArray *history = NULL;
	gfloat *newfloat = NULL;
	GList *list = NULL;
	gfloat tmpf = 0.0;
	gint tmpi = 0;
	gint offset = -1;
	DataSize size = MTX_U08;
	gint log_by_default = 0;
	gchar **vector = NULL;
	gchar * tmpbuf = NULL;
	gchar * dlog_field_name = NULL;
	gchar * dlog_gui_name = NULL;
	gchar * internal_names = NULL;
	gchar * tooltip = NULL;

	ENTER();
	MTXDBG(RTMLOADER,_("Load derived variable!\n"));
	if (!node->children)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("ERROR, load_derived_var, xml node is empty!!\n"));
		EXIT();
		return;
	}
	if (!generic_xml_gchar_find(node,"tooltip",&tooltip))
		MTXDBG(RTMLOADER|CRITICAL,_("Missing Key value \"tooltip\" from RTV XML\n"));
	if (!generic_xml_gchar_find(node,"dlog_gui_name",&dlog_gui_name))
		MTXDBG(RTMLOADER|CRITICAL,_("Missing Key value \"dlog_gui_name\" from RTV XML\n"));
	if (!generic_xml_gchar_find(node,"dlog_field_name",&dlog_field_name))
		MTXDBG(RTMLOADER|CRITICAL,_("Missing Key value \"dlog_field_name\" from RTV XML\n"));
	if (!generic_xml_gchar_find(node,"internal_names",&internal_names))
		MTXDBG(RTMLOADER|CRITICAL,_("Missing Key value \"internal_names\" from RTV XML\n"));
	if (!generic_xml_gint_find(node,"offset",&offset))
		MTXDBG(RTMLOADER|CRITICAL,_("Missing Key value \"offset\" from RTV XML\n"));
	if (!generic_xml_gboolean_find(node,"log_by_default",&log_by_default))
		MTXDBG(RTMLOADER|CRITICAL,_("Missing Key value \"log_by_default\" from RTV XML\n"));
	if (generic_xml_gchar_find(node,"size",&tmpbuf))
	{
		size = (DataSize)translate_string(tmpbuf);
		g_free(tmpbuf);
	}
	else
		size = MTX_U08;
	if ((!internal_names) || (!dlog_gui_name) || (!dlog_field_name) || \
			(!tooltip) || (offset < 0))
	{
		MTXDBG(RTMLOADER|CRITICAL,_("MINIMUMS for RTV var NOT MET, skipping this one!!\n"));
		EXIT();
		return;
	}
	object = g_new0(gconstpointer, 1);
	history = NULL;
	/* Create object to hold all the data. (dynamically)*/
	/* Index */
	DATA_SET(object,"index",GINT_TO_POINTER(map->derived_total));
	map->derived_total++;
	/* History Array */
	history = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
	/* bind history array to object for future retrieval */
	DATA_SET(object,"history",(gpointer)history);

	DATA_SET_FULL(object,"tooltip",g_strdup(_(tooltip)),g_free);
	g_free(tooltip);
	DATA_SET_FULL(object,"dlog_gui_name",g_strdup(_(dlog_gui_name)),g_free);
	g_free(dlog_gui_name);
	DATA_SET_FULL(object,"dlog_field_name",g_strdup(dlog_field_name),g_free);
	g_free(dlog_field_name);
	DATA_SET_FULL(object,"internal_names",g_strdup(internal_names),g_free);
	vector = g_strsplit(internal_names,",",-1); 
	for(i=0;i<g_strv_length(vector);i++) 
		g_hash_table_insert(map->rtv_hash,g_strdup(vector[i]),(gpointer)object);
	g_strfreev(vector);
	g_free(internal_names);
	DATA_SET(object,"size",GINT_TO_POINTER(size));
	DATA_SET(object,"offset",GINT_TO_POINTER(offset));
	DATA_SET(object,"log_by_default",GINT_TO_POINTER(log_by_default));

	if (generic_xml_gint_find(node,"precision",&tmpi))
		DATA_SET(object,"precision",GINT_TO_POINTER(tmpi));
	if (generic_xml_gboolean_find(node,"temp_dep",&tmpi))
		DATA_SET(object,"temp_dep",GINT_TO_POINTER(tmpi));
	if (generic_xml_gchar_find(node,"real_upper",&tmpbuf))
	{
		DATA_SET_FULL(object,"real_upper",g_strdup(tmpbuf),g_free);
		g_free(tmpbuf);
	}
	if (generic_xml_gchar_find(node,"real_lower",&tmpbuf))
	{
		DATA_SET_FULL(object,"real_lower",g_strdup(tmpbuf),g_free);
		g_free(tmpbuf);
	}
	if (generic_xml_gfloat_find(node,"fromecu_mult",&tmpf))
	{
		newfloat = g_new0(gfloat, 1);
		*newfloat = tmpf;
		DATA_SET_FULL(object,"fromecu_mult",(gpointer)newfloat,g_free);
	}
	if (generic_xml_gfloat_find(node,"fromecu_add",&tmpf))
	{
		newfloat = g_new0(gfloat, 1);
		*newfloat = tmpf;
		DATA_SET_FULL(object,"fromecu_add",(gpointer)newfloat,g_free);
	}
	if (generic_xml_gchar_find(node,"lookuptable",&tmpbuf))
	{
		DATA_SET_FULL(object,"lookuptable",g_strdup(tmpbuf),g_free);
		g_free(tmpbuf);
	}
	if (generic_xml_gchar_find(node,"alt_lookuptable",&tmpbuf))
	{
		DATA_SET_FULL(object,"alt_lookuptable",g_strdup(tmpbuf),g_free);
		g_free(tmpbuf);
	}
	if (generic_xml_gchar_find(node,"special",&tmpbuf))
	{
		DATA_SET_FULL(object,"special",g_strdup(tmpbuf),g_free);
		g_free(tmpbuf);
	}
	if (generic_xml_gchar_find(node,"depend_on",&tmpbuf))
	{
		DATA_SET_FULL(object,"depend_on",g_strdup(tmpbuf),g_free);
		g_free(tmpbuf);
		load_rtv_xml_dependencies(object,node);
	}
	if (generic_xml_gboolean_find(node,"fromecu_complex",&tmpi))
	{
		DATA_SET(object,"fromecu_complex",GINT_TO_POINTER(tmpi));
		load_rtv_xml_complex_expression(object,node);
	}
	if (generic_xml_gchar_find(node,"multi_expr_keys",&tmpbuf))
	{
		DATA_SET_FULL(object,"multi_expr_keys",g_strdup(tmpbuf),g_free);
		g_free(tmpbuf);
		load_rtv_xml_multi_expressions(object,node);
	}

	list = (GList *)g_hash_table_lookup(map->offset_hash,GINT_TO_POINTER(offset));
	list = g_list_prepend(list,(gpointer)object);
	g_hash_table_replace(map->offset_hash,GINT_TO_POINTER(offset),(gpointer)list);
	g_ptr_array_add(map->rtv_list,object);

	EXIT();
	return;
}


void load_rtv_xml_dependencies(gconstpointer *object, xmlNode *node)
{
	static void (*load_deps)(gconstpointer *,xmlNode *, const gchar *) = NULL;
	ENTER();
	if (!load_deps)
		get_symbol("load_dependencies",(void **)&load_deps);
	g_return_if_fail(load_deps);
	load_deps(object,node,"depend_on");
	EXIT();
	return;
}


void load_rtv_xml_complex_expression(gconstpointer *object, xmlNode *node)
{
	static void (*common_rtv_loader)(gconstpointer *,xmlNode *, gchar *, ComplexExprType);
	static Firmware_Details *firmware = NULL;
	void *evaluator = NULL;
	gchar *tmpbuf = NULL;
	gchar * name = NULL;
	gint tmpi;
	gint i = 0;
	gchar * fromecu_conv_expr = NULL;
	gchar * raw_expr_symbols = NULL;
	gchar * raw_expr_types = NULL;
	gchar **expr_symbols = NULL;
	gint *expr_types = NULL;
	gint total_symbols = 0;
	gint total_symtypes = 0;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!common_rtv_loader)
		get_symbol("common_rtv_loader",(void **)&common_rtv_loader);

	g_return_if_fail(firmware);
	g_return_if_fail(common_rtv_loader);

	if (!node->children)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("ERROR, load_derived_var, xml node is empty!!\n"));
		EXIT();
		return;
	}
	if (!generic_xml_gchar_find(node,"fromecu_conv_expr",&fromecu_conv_expr))
		MTXDBG(RTMLOADER|CRITICAL,_("Missing important key \"fromecu_conv_expr\" from rtv XML!\n"));
	if (!generic_xml_gchar_find(node,"expr_symbols",&raw_expr_symbols))
		MTXDBG(RTMLOADER|CRITICAL,_("Missing important key \"expr_symbols\" from rtv XML!\n"));
	if (!generic_xml_gchar_find(node,"expr_types",&raw_expr_types))
		MTXDBG(RTMLOADER|CRITICAL,_("Missing important key \"expr_types\" from rtv XML!\n"));

	expr_symbols = parse_keys(raw_expr_symbols, &total_symbols,",");	
	expr_types = parse_keytypes(raw_expr_types, &total_symtypes,",");	
	g_free(raw_expr_symbols);
	g_free(raw_expr_types);
	if (total_symbols != total_symtypes)
	{
		MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("Number of symbols(%i) and symbol types(%i)\n\tare different, ABORTING!!!\n"),total_symbols,total_symtypes);
		g_free(expr_types);
		g_strfreev(expr_symbols);
		g_free(fromecu_conv_expr);
		EXIT();
		return;
	}
	DATA_SET_FULL(object,"fromecu_conv_expr",g_strdup(fromecu_conv_expr),g_free);
	evaluator = evaluator_create(fromecu_conv_expr);
	if (!evaluator)
		MTXDBG(COMPLEX_EXPR|CRITICAL,_("Unable to create evaluator for expression \"%s\", expect a crash\n"),fromecu_conv_expr);
	else
		DATA_SET_FULL(object,"ul_evaluator",evaluator,evaluator_destroy);

	g_free(fromecu_conv_expr);
	// Store the lists as well so DO NOT DEALLOCATE THEM!!! 
	DATA_SET_FULL(object,"expr_types",(gpointer)expr_types,g_free);
	DATA_SET_FULL(object,"expr_symbols",(gpointer)expr_symbols,g_strfreev);
	DATA_SET(object,"total_symbols",GINT_TO_POINTER(total_symbols));

	for (i=0;i<total_symbols;i++)
	{
		switch ((ComplexExprType)expr_types[i])
		{
			case ECU_EMB_BIT:
				// ECU generic (non RTV) variable bit(s)
				common_rtv_loader(object,node,expr_symbols[i],ECU_EMB_BIT);
				break;
			case ECU_VAR:
				// ECU generic (non RTV) variable
				common_rtv_loader(object,node,expr_symbols[i],ECU_VAR);
				break;
			case RAW_VAR:
				// RAW variable 
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!generic_xml_gint_find(node,name,&tmpi))
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("RAW_VAR, failure looking for:%s\n"),name);
				DATA_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=g_strdup_printf("%s_size",expr_symbols[i]);
				if (!generic_xml_gchar_find(node,name,&tmpbuf))
				{
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("RAW_VAR, failure looking for:%s\n"),name);
					tmpi = MTX_U08;
				}
				else
				{
					tmpi = translate_string(tmpbuf);
					g_free(tmpbuf);
					tmpbuf = NULL;
				}
				DATA_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				break;
			case RAW_EMB_BIT:
				// RAW data embedded bitfield 2 params 
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!generic_xml_gint_find(node,name,&tmpi))
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("RAW_EMB_BIT, failure looking for:%s\n"),name);
				DATA_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_bitmask",expr_symbols[i]);
				if (!generic_xml_gint_find(node,name,&tmpi))
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_(__FILE__": load_compex_params()\n\tRAW_EMB_BIT, failure looking for:%s\n"),name);
				DATA_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				break;

			default:
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("expr_type %i (%s) is UNDEFINED, this will cause a crash!!\n"),expr_types[i],expr_symbols[i]);
		}
	}
	EXIT();
	return;
}


/*!
  \brief load_complex_params_obj() loads the necessary parameters from 
  the config file for a complex conversion
  \param object is the place where the data loaded is bound to
  \param cfgfile is the configfile pointer to read from
  \param section is the section to read from in the config file
  */
G_MODULE_EXPORT void load_complex_params_obj(GObject *object, ConfigFile *cfgfile, gchar * section)
{
	static void (*common_rtv_loader_obj)(GObject *,ConfigFile *,gchar * section, gchar *, ComplexExprType);
	static Firmware_Details *firmware = NULL;
	gchar *tmpbuf = NULL;
	gchar **expr_symbols = NULL;
	gint *expr_types = NULL;
	gint total_symbols = 0;
	gint total_symtypes = 0;
	gchar * name = NULL;
	gint tmpi;
	gint i = 0;
	extern gconstpointer *global_data;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!common_rtv_loader_obj)
		get_symbol("common_rtv_loader_obj",(void **)&common_rtv_loader_obj);


	if (!cfg_read_string(cfgfile,section,"expr_symbols",&tmpbuf))
	{
		MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("Read of \"expr_symbols\" from section \"[%s]\" failed ABORTING!!!\n\n"),section);
		g_free(tmpbuf);
		EXIT();
		return;
	}
	else
	{
		expr_symbols = parse_keys(tmpbuf, &total_symbols,",");	
		g_free(tmpbuf);
	}
	if (!cfg_read_string(cfgfile,section,"expr_types",&tmpbuf))
	{
		MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("Read of \"expr_types\" from section \"[%s]\" failed, ABORTING!!!\n\n"),section);
		g_strfreev(expr_symbols);
		g_free(tmpbuf);
		EXIT();
		return;
	}
	else
	{
		expr_types = parse_keytypes(tmpbuf, &total_symtypes,",");	
		g_free(tmpbuf);
	}
	if (total_symbols!=total_symtypes)
	{
		MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("Number of symbols(%i) and symbol types(%i)\n\tare different, ABORTING!!!\n\n"),total_symbols,total_symtypes);
		g_free(expr_types);
		g_strfreev(expr_symbols);
		EXIT();
		return;
	}
	/* Store the lists as well so DO NOT DEALLOCATE THEM!!! */
	OBJ_SET_FULL(object,"expr_types",(gpointer)expr_types,g_free);
	OBJ_SET_FULL(object,"expr_symbols",(gpointer)expr_symbols,g_strfreev);
	OBJ_SET(object,"total_symbols",GINT_TO_POINTER(total_symbols));
	for (i=0;i<total_symbols;i++)
	{
		switch ((ComplexExprType)expr_types[i])
		{
			case ECU_EMB_BIT:
				common_rtv_loader_obj(object,cfgfile,section,expr_symbols[i],ECU_EMB_BIT);
				break;
			case ECU_VAR:
				common_rtv_loader_obj(object,cfgfile,section,expr_symbols[i],ECU_VAR);
				break;
			case RAW_VAR:
				/* RAW variable */
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("RAW_VAR, failure looking for:%s\n"),name);
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_size",expr_symbols[i]);
				if (!cfg_read_string(cfgfile,section,name,&tmpbuf))
				{
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("RAW_VAR, failure looking for:%s\n"),name);
					tmpi = MTX_U08;
				}
				else
				{
					tmpi = translate_string(tmpbuf);
					g_free(tmpbuf);
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				break;
			case RAW_EMB_BIT:
				/* RAW data embedded bitfield 2 params */
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("RAW_EMB_BIT, failure looking for:%s\n"),name);
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_bitmask",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("RAW_EMB_BIT, failure looking for:%s\n"),name);
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				break;

			default:
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("expr_type is UNDEFINED, this will cause a crash!!\n"));
		}
	}
	EXIT();
	return;
}
