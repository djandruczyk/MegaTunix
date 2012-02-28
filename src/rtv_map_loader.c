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
	Firmware_Details *firmware = NULL;

	MTXDBG(RTMLOADER,_("Entered"));
	firmware = DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,FALSE);

	if ((!DATA_GET(global_data,"interrogated")) && 
			(DATA_GET(global_data,"connected")))
		return FALSE;

	gdk_threads_enter();
	set_title(g_strdup(_("Loading Realtime Map...")));
	filename = get_file(g_build_path(PSEP,REALTIME_MAPS_DATA_DIR,firmware->rtv_map_file,NULL),g_strdup("xml"));
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
		gdk_threads_leave();
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
		DATA_SET(global_data,"rtv_map",rtv_map);
		set_title(g_strdup(_("RT Map XML Loaded OK!")));
		DATA_SET(global_data,"rtvars_loaded",GINT_TO_POINTER(TRUE));
	}
	gdk_threads_leave();
	return TRUE;
}


gboolean load_rtv_xml_elements(xmlNode *a_node, Rtv_Map *map)
{
	xmlNode *cur_node = NULL;

	/* Iterate down... */
	for (cur_node = a_node; cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"api") == 0)
				if (!xml_api_check(cur_node,RTV_MAP_MAJOR_API,RTV_MAP_MINOR_API))
				{
					MTXDBG(CRITICAL,_("API mismatch, won't load this file!!\n"));
					return FALSE;
				}
			if (g_strcasecmp((gchar *)cur_node->name,"realtime_map") == 0)
				load_rtv_defaults(cur_node,map);
			if (g_strcasecmp((gchar *)cur_node->name,"derived") == 0)
				load_derived_var(cur_node,map);
		}
		if (!load_rtv_xml_elements(cur_node->children,map))
			return FALSE;
	}
	return TRUE;
}


void load_rtv_defaults(xmlNode *node, Rtv_Map *map)
{
	xmlNode *cur_node = NULL;
	gchar * tmpbuf = NULL;

	MTXDBG(RTMLOADER,_("load_rtv_defaults!\n"));
	if (!node->children)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("ERROR, load_derived_var, xml node is empty!!\n"));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"applicable_signatures") == 0)
				generic_xml_gchar_import(cur_node,&map->applicable_signatures);
			if (g_strcasecmp((gchar *)cur_node->name,"raw_list") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				map->raw_list = parse_keys(tmpbuf,NULL,",");
				g_free(tmpbuf);
			}
		}
		cur_node = cur_node->next; /* Iterate, iterate, iterate... */
	}
}


void load_derived_var(xmlNode *node, Rtv_Map *map)
{
	gint i = 0;
	gconstpointer *object = NULL;
	GArray *history = NULL;
	gfloat *newfloat = NULL;
	GList *list = NULL;
	xmlNode *cur_node = NULL;
	gint offset = 0;
	gint precision = 0;
	gfloat fromecu_mult = 1.0;
	gfloat fromecu_add = 0.0;
	DataSize size = MTX_U08;
	gint temp_dep = 0;
	gint log_by_default = 0;
	gchar **vector = NULL;
	gchar * lookuptable = NULL;
	gchar * real_lower = NULL;
	gchar * real_upper = NULL;
	gchar * tmpbuf = NULL;
	gchar * special = NULL;
	gchar * dlog_field_name = NULL;
	gchar * dlog_gui_name = NULL;
	gchar * internal_names = NULL;
	gchar * tooltip = NULL;
	gboolean has_deps = FALSE;
	gboolean complex_expression = FALSE;
	gboolean multiple_expression = FALSE;

	MTXDBG(RTMLOADER,_("Load derived variable!\n"));
	if (!node->children)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("ERROR, load_derived_var, xml node is empty!!\n"));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			/* Minimum Required Fields */
			if (g_strcasecmp((gchar *)cur_node->name,"tooltip") == 0)
				generic_xml_gchar_import(cur_node,&tooltip);
			if (g_strcasecmp((gchar *)cur_node->name,"dlog_gui_name") == 0)
				generic_xml_gchar_import(cur_node,&dlog_gui_name);
			if (g_strcasecmp((gchar *)cur_node->name,"dlog_field_name") == 0)
				generic_xml_gchar_import(cur_node,&dlog_field_name);
			if (g_strcasecmp((gchar *)cur_node->name,"internal_names") == 0)
				generic_xml_gchar_import(cur_node,&internal_names);
			if (g_strcasecmp((gchar *)cur_node->name,"real_upper") == 0)
				generic_xml_gchar_import(cur_node,&real_upper);
			if (g_strcasecmp((gchar *)cur_node->name,"real_lower") == 0)
				generic_xml_gchar_import(cur_node,&real_lower);
			if (g_strcasecmp((gchar *)cur_node->name,"offset") == 0)
				generic_xml_gint_import(cur_node,&offset);
			if (g_strcasecmp((gchar *)cur_node->name,"log_by_default") == 0)
				generic_xml_gboolean_import(cur_node,&log_by_default);
			if (g_strcasecmp((gchar *)cur_node->name,"size") == 0)
			{
				generic_xml_gchar_import(cur_node,&tmpbuf);
				size = translate_string(tmpbuf);
				g_free(tmpbuf);
			}
			/* Common, yet Optional Fields */
			if (g_strcasecmp((gchar *)cur_node->name,"fromecu_mult") == 0)
				generic_xml_gfloat_import(cur_node,&fromecu_mult);
			if (g_strcasecmp((gchar *)cur_node->name,"fromecu_add") == 0)
				generic_xml_gfloat_import(cur_node,&fromecu_add);
			if (g_strcasecmp((gchar *)cur_node->name,"precision") == 0)
				generic_xml_gint_import(cur_node,&precision);
			if (g_strcasecmp((gchar *)cur_node->name,"temp_dep") == 0)
				generic_xml_gboolean_import(cur_node,&temp_dep);
			if (g_strcasecmp((gchar *)cur_node->name,"lookuptable") == 0)
				generic_xml_gchar_import(cur_node,&lookuptable);
			/* Hack for special values (HR clock?) */
			if (g_strcasecmp((gchar *)cur_node->name,"special") == 0)
				generic_xml_gchar_import(cur_node,&special);
			if (g_strcasecmp((gchar *)cur_node->name,"depend_on") == 0)
				has_deps = TRUE;
			if (g_strcasecmp((gchar *)cur_node->name,"fromecu_complex") == 0)
				complex_expression = TRUE;
			if (g_strcasecmp((gchar *)cur_node->name,"multi_expr_keys") == 0)
				multiple_expression = TRUE;
		}
		cur_node = cur_node->next;
	}
	/* Validate minimum needed */
	if ((internal_names) && (dlog_gui_name) && (dlog_field_name) && \
			(tooltip) && (offset >= 0) && \
			(real_lower) && (real_upper))
			
	{
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

		DATA_SET(object,"precision",GINT_TO_POINTER(precision));
		DATA_SET(object,"offset",GINT_TO_POINTER(offset));
		DATA_SET(object,"size",GINT_TO_POINTER(size));
		DATA_SET(object,"temp_dep",GINT_TO_POINTER(temp_dep));
		DATA_SET(object,"log_by_default",GINT_TO_POINTER(log_by_default));
		if (internal_names)
		{
			DATA_SET_FULL(object,"internal_names",g_strdup(internal_names),g_free);
			vector = g_strsplit(internal_names,",",-1); 
			for(i=0;i<g_strv_length(vector);i++) 
				g_hash_table_insert(map->rtv_hash,g_strdup(vector[i]),(gpointer)object);
			g_strfreev(vector);
			g_free(internal_names);
		}
		if (dlog_field_name)
		{
			DATA_SET_FULL(object,"dlog_field_name",g_strdup(dlog_field_name),g_free);
			g_free(dlog_field_name);
		}
		/* Translate if needed  */
		if (dlog_gui_name)
		{
			DATA_SET_FULL(object,"dlog_gui_name",g_strdup(_(dlog_gui_name)),g_free);
			g_free(dlog_gui_name);
		}
		if (tooltip)
		{
			DATA_SET_FULL(object,"tooltip",g_strdup(_(tooltip)),g_free);
			g_free(tooltip);
		}
		if (lookuptable)
		{
			DATA_SET_FULL(object,"lookuptable",g_strdup(lookuptable),g_free);
			g_free(lookuptable);
		}
		/* Oddball one... */
		if (special)
		{
			DATA_SET_FULL(object,"special",g_strdup(special),g_free);
			g_free(special);
		}
		/* OPTIONAL PARAMETERS */
		if (real_lower)
		{
			DATA_SET_FULL(object,"real_lower",g_strdup(real_lower),g_free);
			g_free(real_lower);
		}
		if (real_upper)
		{
			DATA_SET_FULL(object,"real_upper",g_strdup(real_upper),g_free);
			g_free(real_upper);
		}
		/* Floats need some special handling to avoid leaks */
		newfloat = g_new0(gfloat, 1);
		*newfloat = fromecu_mult;
		DATA_SET_FULL(object,"fromecu_mult",(gpointer)newfloat,g_free);
		newfloat = g_new0(gfloat, 1);
		*newfloat = fromecu_add;
		DATA_SET_FULL(object,"fromecu_add",(gpointer)newfloat,g_free);

		if (has_deps)
			load_rtv_xml_dependancies(object,node);
		if (complex_expression)
			load_rtv_xml_complex_expression(object,node);
		if (multiple_expression)
			printf("This rtv var has multiple_expressions!, not written yet\n");
		list = g_hash_table_lookup(map->offset_hash,GINT_TO_POINTER(offset));
		list = g_list_prepend(list,(gpointer)object);
		g_hash_table_replace(map->offset_hash,GINT_TO_POINTER(offset),(gpointer)list);
		g_ptr_array_add(map->rtv_list,object);
	}
	else
		MTXDBG(RTMLOADER|CRITICAL,_("Derived variable doesn't meet the basic criteria!\n"));
}


void load_rtv_xml_dependancies(gconstpointer *object, xmlNode *node)
{
	static void (*load_deps)(gconstpointer *,ConfigFile *,const gchar *, const gchar *) = NULL;
	if (!load_deps)
		get_symbol("load_dependancies",(void *)&load_deps);
	g_return_if_fail(load_deps);
	load_deps(object,node,"depend_on");
}

/*
void shit()
{
	static void (*load_deps)(gconstpointer *,ConfigFile *,const gchar *, const gchar *) = NULL;

	get_symbol("load_dependancies",(void *)&load_deps);
		load_deps(object,cfgfile,section,"depend_on");
	load_multi_expressions(object,cfgfile,section);
	*/

void load_rtv_xml_complex_expression(gconstpointer *object, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	static void (*common_rtv_loader)(gconstpointer *,xmlNode *, gchar *, ComplexExprType);
	static Firmware_Details *firmware = NULL;
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

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	if (!common_rtv_loader)
		get_symbol("common_rtv_loader",(void *)&common_rtv_loader);

	g_return_if_fail(firmware);
	g_return_if_fail(common_rtv_loader);

	if (!node->children)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("ERROR, load_derived_var, xml node is empty!!\n"));
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
	if (total_symbols != total_symtypes)
	{
		MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("Number of symbols(%i) and symbol types(%i)\n\tare different, ABORTING!!!\n"),total_symbols,total_symtypes);
		g_free(expr_types);
		g_strfreev(expr_symbols);
		return;
	}
	// Store the lists as well so DO NOT DEALLOCATE THEM!!! 
	DATA_SET_FULL(object,"expr_types",(gpointer)expr_types,g_free);
	DATA_SET_FULL(object,"expr_symbols",(gpointer)expr_symbols,g_strfreev);
	DATA_SET(object,"total_symbols",GINT_TO_POINTER(total_symbols));

	for (i=0;i<total_symbols;i++)
	{
		switch ((ComplexExprType)expr_types[i])
		{
			case ECU_EMB_BIT:
				printf("ECU_EMB_BIT loader not written yet\n");
				common_rtv_loader(object,node,expr_symbols[i],ECU_EMB_BIT);
				break;
			case ECU_VAR:
				printf("ECU_EMB_BIT loader not written yet\n");
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
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("expr_type %i is UNDEFINED, this will cause a crash!!\n"),expr_types[i]);
		}
	}
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

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	if (!common_rtv_loader_obj)
		get_symbol("common_rtv_loader_obj",(void *)&common_rtv_loader_obj);


	if (!cfg_read_string(cfgfile,section,"expr_symbols",&tmpbuf))
	{
		MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("Read of \"expr_symbols\" from section \"[%s]\" failed ABORTING!!!\n\n"),section);
		g_free(tmpbuf);
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
}
