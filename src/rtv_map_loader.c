/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
#include <multi_expr_loader.h>
#include <getfiles.h>
#include <keyparser.h>
#include <notifications.h>
#include <plugin.h>
#include <rtv_map_loader.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>

/*!
  \brief load_realtime_map_pf() loads the realtime map specified in the detected
  firmware's interrogation profile, and sets up the necessary arrays for storage
  of data coming from the ECU (temporary arrays for the last 50 or so entries)
  */
G_MODULE_EXPORT gboolean load_realtime_map_pf(void )
{
	GtkWidget *dialog = NULL;
	ConfigFile *cfgfile = NULL;
	gchar * filename = NULL;
	gchar *tmpbuf = NULL;
	gint derived_total = 0;
	gint num_keys = 0;
	gchar ** keys = NULL;
	gchar **vector = NULL;
	DataType keytype = MTX_INT;
	gint i = 0;
	gint j = 0;
	guint k = 0;
	gint tmpi = 0;
	gint major = 0;
	gint minor = 0;
	gint offset = 0;
	gfloat tmpf = 0.0;
	gfloat *newfloat = NULL;
	gchar * section = NULL;
	gconstpointer *object = NULL;
	GList * list = NULL;
	GArray *history = NULL;
	Rtv_Map *rtv_map = NULL;
	extern gconstpointer *global_data;
	Firmware_Details *firmware = NULL;
	static void (*load_deps)(gconstpointer *,ConfigFile *,const gchar *, const gchar *) = NULL;

	MTXDBG(RTMLOADER,_("Entered"));
	firmware = DATA_GET(global_data,"firmware");

	if ((!DATA_GET(global_data,"interrogated")) && 
			(DATA_GET(global_data,"connected")))
		return FALSE;

	gdk_threads_enter();
	set_title(g_strdup(_("Loading Realtime Map...")));
	filename = get_file(g_build_path(PSEP,REALTIME_MAPS_DATA_DIR,firmware->rtv_map_file,NULL),g_strdup("rtv_map"));
	if (!filename)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("File \"%s.rtv_map\" not found!!, exiting function\n"),firmware->rtv_map_file);
		set_title(g_strdup(_("ERROR RT Map file DOES NOT EXIST!!!")));
		dialog = gtk_message_dialog_new_with_markup(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,_("\n<b>MegaTunix</b> Realtime Variable map \"%s.rtv_map\" for this firmware doesn't appear to be installed correctly!\n\nDid you forget to run <i>\"sudo make install\"</i>??\n You should notify the author of this bug\n\n"),firmware->rtv_map_file);

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
	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
	{
		MTXDBG(RTMLOADER|CRITICAL,_("Can't find realtime vars map file %s\n\n"),filename);
		g_free(filename);
		set_title(g_strdup(_("ERROR RT Map file could NOT be opened!!!")));
		gdk_threads_leave();
		return FALSE;
	}
	get_file_api(cfgfile,&major,&minor);
	if ((major != RTV_MAP_MAJOR_API) || (minor != RTV_MAP_MINOR_API))
	{
		MTXDBG(RTMLOADER|CRITICAL,_("RealTimeMap profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n"),major,minor,RTV_MAP_MAJOR_API,RTV_MAP_MINOR_API,filename);
		g_free(filename);
		set_title(g_strdup(_("ERROR RT Map API MISMATCH!!!")));
		gdk_threads_leave();
		return FALSE;
	}
	else
		MTXDBG(RTMLOADER,_("Loading realtime map from %s\n"),filename);
	g_free(filename);

	/* If file found we continue... */
	if(!cfg_read_string(cfgfile,"realtime_map","applicable_firmwares",&tmpbuf))
	{
		MTXDBG(RTMLOADER|CRITICAL,_("Can't find \"applicable_firmwares\" key, ABORTING!!\n"));
		cfg_free(cfgfile);
		set_title(g_strdup(_("ERROR RT Map missing data!!!")));
		gdk_threads_leave();
		return FALSE;
	}
	if (strstr(tmpbuf,firmware->name) == NULL)	
	{
		MTXDBG(RTMLOADER|CRITICAL,_("Firmware signature \"%s\"\n\tis NOT found in this file:(%s) Potential firmware choices are \"%s\", ABORT!\n\n"),firmware->actual_signature,cfgfile->filename,tmpbuf);
		cfg_free(cfgfile);
		g_free(tmpbuf);
		set_title(g_strdup(_("ERROR RT Map signature MISMATCH!!!")));
		gdk_threads_leave();
		return FALSE;

	}
	g_free(tmpbuf);
	/* OK, basic checks passed,  start loading data into
	 * the main mapping structures...
	 */
	if(!cfg_read_int(cfgfile,"realtime_map","derived_total",&derived_total))
	{
		MTXDBG(RTMLOADER|CRITICAL,_("Can't find \"derived_total\" in the \"[realtime_map]\" section\n"));
	}

	tmpbuf = NULL;
	rtv_map = g_new0(Rtv_Map, 1);
	DATA_SET(global_data,"rtv_map",rtv_map);
	cfg_read_string(cfgfile,"realtime_map","applicable_revisions",&rtv_map->applicable_revisions);
	cfg_read_string(cfgfile,"realtime_map","raw_list",&tmpbuf);
	if (tmpbuf)
	{
		rtv_map->raw_list = parse_keys(tmpbuf,&num_keys,",");
		g_free(tmpbuf);
	}
	/* This should free to values with g_list_free, but it causes a fault*/
	rtv_map->offset_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,NULL);
	rtv_map->rtv_list = g_ptr_array_new();
	rtv_map->rtv_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	rtv_map->rtvars_size = firmware->rtvars_size;
	rtv_map->derived_total = derived_total;
	rtv_map->ts_array = g_array_sized_new(FALSE,TRUE, sizeof(GTimeVal),4096);

	/* Load em up.. */
	for (i=0;i<derived_total;i++)
	{
		section = g_strdup_printf("derived_%i",i);
		/* Get key list and parse */
		if(!cfg_read_string(cfgfile,section,"keys",&tmpbuf))
		{
			MTXDBG(RTMLOADER|CRITICAL,_("Can't find \"keys\" in the \"[%s]\" section, ABORTING!!!\n"),section);
			g_free(section);
			set_title(g_strdup(_("ERROR RT Map missing data problem!!!")));
			gdk_threads_leave();
			return FALSE;
		}
		else
		{
			keys = parse_keys(tmpbuf,&num_keys,",");
			g_free(tmpbuf);
		}
		if (!cfg_read_int(cfgfile,section,"offset",&offset))
		{
			MTXDBG(RTMLOADER|CRITICAL,_("Can't find \"offset\" in the \"[%s]\" section, ABORTING!!!\n\n"),section);
			g_free(section);
			g_strfreev(keys);
			set_title(g_strdup(_("ERROR RT Map offset missing!!!")));
			gdk_threads_leave();
			return FALSE;
		}
		object = g_new0(gconstpointer, 1);
		history = NULL;
		/* Create object to hold all the data. (dynamically)*/
		/* Assume default size of 8 bit unsigned */
		DATA_SET(object,"size",GINT_TO_POINTER(MTX_U08));
		/* Index */
		DATA_SET(object,"index",GINT_TO_POINTER(i));
		/* Object name */
		DATA_SET_FULL(object,"name",g_strdup(section),g_free);
		/* History Array */
		history = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
		/* bind history array to object for future retrieval */
		DATA_SET(object,"history",(gpointer)history);

		if (cfg_read_string(cfgfile,section,"depend_on",&tmpbuf))
		{
			if (!load_deps)
				get_symbol("load_dependancies",(void *)&load_deps);
			if (load_deps)
				load_deps(object,cfgfile,section,"depend_on");
			else
				MTXDBG(CRITICAL|RTMLOADER,_("Function pointer for \"load_dependancies\" could NOT be found in plugins, BUG!\n"));
			g_free(tmpbuf);
		}
		if (cfg_read_string(cfgfile,section,"multi_expr_keys",&tmpbuf))
		{
			load_multi_expressions(object,cfgfile,section);
			g_free(tmpbuf);
		}
		for (j=0;j<num_keys;j++)
		{
			keytype = translate_string(keys[j]);
			switch((DataType)keytype)
			{
				case MTX_INT:
					if (cfg_read_int(cfgfile,section,keys[j],&tmpi))
					{
						MTXDBG(RTMLOADER,_("Binding INT \"%s\",\"%i\" to widget \"%s\"\n"),keys[j],tmpi,section);
						DATA_SET(object,
								keys[j],
								GINT_TO_POINTER(tmpi));
					}
					else
						MTXDBG(RTMLOADER|CRITICAL,_("MTX_INT: read of key \"%s\" from section \"%s\" failed\n"),keys[j],section);
					break;
				case MTX_ENUM:
					if (cfg_read_string(cfgfile,section,keys[j],&tmpbuf))
					{
						tmpi = translate_string(tmpbuf);
						MTXDBG(RTMLOADER,_("Binding ENUM \"%s\",\"%i\" to widget \"%s\"\n"),keys[j],tmpi,section);
						DATA_SET(object,
								keys[j],
								GINT_TO_POINTER(tmpi));
						g_free(tmpbuf);
					}
					else
						MTXDBG(RTMLOADER|CRITICAL,_("MTX_ENUM: read of key \"%s\" from section \"%s\" failed\n"),keys[j],section);
					break;
				case MTX_BOOL:
					if (cfg_read_boolean(cfgfile,section,keys[j],&tmpi))
					{
						if (tmpi == 0)
							tmpi = -1;
						MTXDBG(RTMLOADER,("Binding BOOL \"%s\",\"%i\" to widget \"%s\"\n"),keys[j],tmpi,section);
						DATA_SET(object,
								keys[j],
								GINT_TO_POINTER(tmpi));
						if (strstr(keys[j],"fromecu_complex") != NULL)
							load_complex_params(object,cfgfile,section);
					}
					else
						MTXDBG(RTMLOADER|CRITICAL,_("MTX_BOOL: read of key \"%s\" from section \"%s\" failed\n"),keys[j],section);
					break;
				case MTX_FLOAT:
					if (cfg_read_float(cfgfile,section,keys[j],&tmpf))
					{
						newfloat = NULL;
						newfloat = g_new0(gfloat, 1);
						*newfloat = tmpf;
						DATA_SET_FULL(object,
								keys[j],
								(gpointer)newfloat,g_free);
					}
					else
						MTXDBG(RTMLOADER|CRITICAL,_("MTX_FLOAT: read of key \"%s\" from section \"%s\" failed\n"),keys[j],section);
					break;

				case MTX_STRING:
					if(cfg_read_string(cfgfile,section,keys[j],&tmpbuf))
					{
						MTXDBG(RTMLOADER,_("Binding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n"),keys[j],tmpbuf,section);
						if ((strstr(keys[j],"dlog_gui_name")) || (strstr(keys[j],"tooltip")))
						{
							DATA_SET_FULL(object,
									keys[j],
									g_strdup(_(tmpbuf)),
									g_free);
						}
						else
						{
							DATA_SET_FULL(object,
									keys[j],
									g_strdup(tmpbuf),
									g_free);
						}
						g_free(tmpbuf);
					}
					else
						MTXDBG(RTMLOADER|CRITICAL,_("MTX_STRING: read of key \"%s\" from section \"%s\" failed\n"),keys[j],section);
					break;
				case MTX_UNKNOWN:
					MTXDBG(RTMLOADER|CRITICAL,_("MTX_UNKNWON: key \"%s\" DON'T KNOW HOW TO HANDLE, missing stringmatch association!\n"),keys[j]);
					break;

				default:
					break;

			}
		}
		if(cfg_read_string(cfgfile,section,"internal_names",&tmpbuf))
		{
			vector = g_strsplit(tmpbuf,",",-1); 
			for(k=0;k<g_strv_length(vector);k++) 
				g_hash_table_insert(rtv_map->rtv_hash,g_strdup(vector[k]),(gpointer)object);
			g_strfreev(vector);
			g_free(tmpbuf);
		}
		/*DATA_SET_FULL(object,"keys",g_strdupv(keys),g_strfreev);*/
		list = g_hash_table_lookup(rtv_map->offset_hash,GINT_TO_POINTER(offset));
		list = g_list_prepend(list,(gpointer)object);
		g_hash_table_replace(rtv_map->offset_hash,GINT_TO_POINTER(offset),(gpointer)list);
		g_ptr_array_add(rtv_map->rtv_list,object);
		g_free(section);
		g_strfreev(keys);
		/*g_datalist_foreach(object,dump_datalist,NULL);*/
	}
	cfg_free(cfgfile);
	DATA_SET(global_data,"rtvars_loaded",GINT_TO_POINTER(TRUE));
	set_title(g_strdup(_("RT Map loaded...")));
	gdk_threads_leave();
	MTXDBG(RTMLOADER,_("Leaving"));
	return TRUE;
}


/*!
  \brief load_complex_params() loads the necessary parameters from the config
  file for a complex conversion
  \param object is the place where the data loaded is bound to
  \param cfgfile is the configfile pointer to read from
  \param section is the section to read from in the config file
  */
G_MODULE_EXPORT void load_complex_params(gconstpointer *object, ConfigFile *cfgfile, gchar * section)
{
	static void (*common_rtv_loader)(gconstpointer *,ConfigFile *,gchar * section, gchar *, ComplexExprType);
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
	if (!common_rtv_loader)
		get_symbol("common_rtv_loader",(void *)&common_rtv_loader);

	g_return_if_fail(firmware);
	g_return_if_fail(common_rtv_loader);
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
		MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("Read of \"expr_types\" from section \"[%s]\" failed, ABORTING!!!\n"),section);
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
		MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("Number of symbols(%i) and symbol types(%i)\n\tare different, ABORTING!!!\n"),total_symbols,total_symtypes);
		g_free(expr_types);
		g_strfreev(expr_symbols);
		return;
	}
	/* Store the lists as well so DO NOT DEALLOCATE THEM!!! */
	DATA_SET_FULL(object,"expr_types",(gpointer)expr_types,g_free);
	DATA_SET_FULL(object,"expr_symbols",(gpointer)expr_symbols,g_strfreev);
	DATA_SET(object,"total_symbols",GINT_TO_POINTER(total_symbols));
	for (i=0;i<total_symbols;i++)
	{
		switch ((ComplexExprType)expr_types[i])
		{
			case ECU_EMB_BIT:
				common_rtv_loader(object,cfgfile,section,expr_symbols[i],ECU_EMB_BIT);
				break;
			case ECU_VAR:
				common_rtv_loader(object,cfgfile,section,expr_symbols[i],ECU_VAR);
				break;
			case RAW_VAR:
				/* RAW variable */
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("RAW_VAR, failure looking for:%s\n"),name);
				DATA_SET(object,name,GINT_TO_POINTER(tmpi));
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
				DATA_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				break;
			case RAW_EMB_BIT:
				/* RAW data embedded bitfield 2 params */
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("RAW_EMB_BIT, failure looking for:%s\n"),name);
				DATA_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_bitmask",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
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
