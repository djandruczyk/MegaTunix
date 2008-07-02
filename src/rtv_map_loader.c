/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <apicheck.h>
#include <api-versions.h>
#include <assert.h>
#include <assert.h>
#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <dep_loader.h>
#include <multi_expr_loader.h>
#include <enums.h>
#include <firmware.h>
#include <getfiles.h>
#include <keyparser.h>
#include <notifications.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <rtv_map_loader.h>
#include <string.h>
#include <stringmatch.h>

Rtv_Map *rtv_map = NULL;
gboolean rtvars_loaded = FALSE;
extern gint dbg_lvl;
extern GObject *global_data;


/*!
 \brief load_realtime_map_pf() loads the realtime map specified in the detected
 firmware's interrogation profile, and sets up the necessary arrays for storage
 of data coming from the ECU (temporary arrays for the last 50 or so entries)
 */
EXPORT gboolean load_realtime_map_pf(void )
{
	ConfigFile *cfgfile;
	extern Firmware_Details *firmware;
	gchar * filename = NULL;
	gchar *tmpbuf = NULL;
	gint derived_total = 0;
	gint num_keys = 0;
	gint num_keytypes = 0;
	gchar ** keys = NULL;
	gchar **vector = NULL;
	gint *keytypes = NULL;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint tmpi = 0;
	gint major = 0;
	gint minor = 0;
	gint offset = 0;
	gchar * section = NULL;
	GObject * object = NULL;
	GList * list = NULL;
	GArray *history = NULL;
	extern gboolean interrogated;
	extern gboolean connected;
	extern volatile gboolean offline;

	rtvars_loaded = FALSE;

	if (!((interrogated) && ((connected) || (offline))))
		return FALSE;

	set_title(g_strdup("Loading RT Map..."));
	filename = get_file(g_strconcat(REALTIME_MAPS_DATA_DIR,PSEP,firmware->rtv_map_file,NULL),g_strdup("rtv_map"));
	if (!filename)
	{
		if (dbg_lvl & (RTMLOADER|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\t File \"%s.rtv_map\" not found!!, exiting function\n",firmware->rtv_map_file));
		set_title(g_strdup("ERROR RT Map file DOES NOT EXIST!!!"));
		return FALSE;
	}
	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
	{
		if (dbg_lvl & (RTMLOADER|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tCan't find realtime vars map file %s\n\n",filename));
		g_free(filename);
		set_title(g_strdup("ERROR RT Map file could NOT be opened!!!"));
		return FALSE;
	}
	get_file_api(cfgfile,&major,&minor);
	if ((major != RTV_MAP_MAJOR_API) || (minor != RTV_MAP_MINOR_API))
	{
		if (dbg_lvl & (RTMLOADER|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tRealTimeMap profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n",major,minor,RTV_MAP_MAJOR_API,RTV_MAP_MINOR_API,filename));
		g_free(filename);
		set_title(g_strdup("ERROR RT Map API MISMATCH!!!"));
		return FALSE;
	}
	else
	{
		if (dbg_lvl & RTMLOADER)
			dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tLoading realtime map from %s\n",filename));
	}
	g_free(filename);

	/* If file found we continue... */
	if(!cfg_read_string(cfgfile,"realtime_map","applicable_firmwares",&tmpbuf))
	{
		if (dbg_lvl & (RTMLOADER|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_realtime_map_pf()\n\tCan't find \"applicable_firmwares\" key, ABORTING!!\n"));
		cfg_free(cfgfile);
		g_free(cfgfile);
		set_title(g_strdup("ERROR RT Map missing data!!!"));
		return FALSE;
	}
	if (strstr(tmpbuf,firmware->name) == NULL)	
	{
		if (dbg_lvl & (RTMLOADER|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tFirmware signature \"%s\"\n\tis NOT found in this file:\n\t(%s)\n\tPotential firmware choices are \"%s\", ABORT!\n\n",firmware->actual_signature,cfgfile->filename,tmpbuf));
		cfg_free(cfgfile);
		g_free(cfgfile);
		g_free(tmpbuf);
		set_title(g_strdup("ERROR RT Map signature MISMATCH!!!"));
		return FALSE;

	}
	g_free(tmpbuf);
	/* OK, basic checks passed,  start loading data into
	 * the main mapping structures...
	 */
	if(!cfg_read_int(cfgfile,"realtime_map","derived_total",&derived_total))
	{
		if (dbg_lvl & (RTMLOADER|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_realtime_map_pf()\n\tcan't find \"derived_total\" in the \"[realtime_map]\" section\n"));
	}

	tmpbuf = NULL;
	rtv_map = g_new0(Rtv_Map, 1);
	cfg_read_string(cfgfile,"realtime_map","applicable_signatures",&rtv_map->applicable_signatures);
	cfg_read_string(cfgfile,"realtime_map","raw_list",&tmpbuf);
	if (tmpbuf)
	{
		rtv_map->raw_list = parse_keys(tmpbuf,&num_keys,",");
		g_free(tmpbuf);
	}
	rtv_map->offset_hash = g_hash_table_new(g_direct_hash,g_direct_equal);
	rtv_map->rtv_list = g_array_new(FALSE,TRUE,sizeof(GObject *));
	rtv_map->rtv_hash = g_hash_table_new(g_str_hash,g_str_equal);
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
			if (dbg_lvl & (RTMLOADER|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tCan't find \"keys\" in the \"[%s]\" section, ABORTING!!!\n\n ",section));
			g_free(section);
			set_title(g_strdup("ERROR RT Map missing data problem!!!"));
			return FALSE;
		}
		else
		{
			keys = parse_keys(tmpbuf,&num_keys,",");
			g_free(tmpbuf);
		}
		/* Get key TYPE list and parse */
		if(!cfg_read_string(cfgfile,section,"key_types",&tmpbuf))
		{
			if (dbg_lvl & (RTMLOADER|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tCan't find \"key_types\" in the \"[%s]\" section, ABORTING!!\n\n",section));
			g_free(section);
		set_title(g_strdup("ERROR RT Map missing data problem!!!"));
			return FALSE;
		}
		else
		{
			keytypes = parse_keytypes(tmpbuf, &num_keytypes,",");
			g_free(tmpbuf);
		}
		if (num_keytypes != num_keys)
		{
			if (dbg_lvl & (RTMLOADER|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tNumber of keys (%i) and keytypes(%i)\n\tdoes NOT match for: \"%s\", ABORTING!!!\n\n",num_keys,num_keytypes,section));
			g_free(section);
			g_free(keytypes);
			g_strfreev(keys);
		set_title(g_strdup("ERROR RT Map key/data problem!!!"));
			return FALSE;
		}
		if (!cfg_read_int(cfgfile,section,"offset",&offset))
		{
			if (dbg_lvl & (RTMLOADER|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tCan't find \"offset\" in the \"[%s]\" section, ABORTING!!!\n\n",section));
			g_free(section);
			g_free(keytypes);
			g_strfreev(keys);
			set_title(g_strdup("ERROR RT Map offset missing!!!"));
			return FALSE;
		}
		/* Create object to hold all the data. (dynamically)*/
		object = g_object_new(GTK_TYPE_INVISIBLE,NULL);
		g_object_ref(object);
		gtk_object_sink(GTK_OBJECT(object));
		/* History Array */
		history = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
		OBJ_SET(object,"current_index",GINT_TO_POINTER(-1));
		/* Assume default size of 8 bit unsigned */
		OBJ_SET(object,"size",GINT_TO_POINTER(MTX_U08));
		/* bind hostory array to object for future retrieval */
		OBJ_SET(object,"history",(gpointer)history);

		if (cfg_read_string(cfgfile,section,"depend_on",&tmpbuf))
		{
	                load_dependancies(G_OBJECT(object),cfgfile,section);
			g_free(tmpbuf);
		}
		if (cfg_read_string(cfgfile,section,"multi_expr_keys",&tmpbuf))
		{
			load_multi_expressions(G_OBJECT(object),cfgfile,section);
			g_free(tmpbuf);
		}
		for (j=0;j<num_keys;j++)
		{
			switch((DataType)keytypes[j])
			{
				case MTX_INT:
					if (cfg_read_int(cfgfile,section,keys[j],&tmpi))
					{
						if (dbg_lvl & RTMLOADER)
							dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tbinding INT \"%s\",\"%i\" to widget \"%s\"\n",keys[j],tmpi,section));
						OBJ_SET(object,
								keys[j],
								GINT_TO_POINTER(tmpi));
					}
					else
					{
						if (dbg_lvl & (RTMLOADER|CRITICAL))
							dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tMTX_INT: read of key \"%s\" from section \"%s\" failed\n",keys[j],section));
					}
					break;
				case MTX_ENUM:
					if (cfg_read_string(cfgfile,section,keys[j],&tmpbuf))
					{
						tmpi = translate_string(tmpbuf);
						if (dbg_lvl & RTMLOADER)
							dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tbinding ENUM \"%s\",\"%i\" to widget \"%s\"\n",keys[j],tmpi,section));
						OBJ_SET(object,
								keys[j],
								GINT_TO_POINTER(tmpi));
						g_free(tmpbuf);
					}
					else
					{
						if (dbg_lvl & (RTMLOADER|CRITICAL))
							dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tMTX_ENUM: read of key \"%s\" from section \"%s\" failed\n",keys[j],section));
					}
					break;
				case MTX_BOOL:
					if (cfg_read_boolean(cfgfile,section,keys[j],&tmpi))
					{
						if (dbg_lvl & RTMLOADER)
							dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tbinding BOOL \"%s\",\"%i\" to widget \"%s\"\n",keys[j],tmpi,section));
						OBJ_SET(object,
								keys[j],
								GINT_TO_POINTER(tmpi));
						if (strstr(keys[j],"complex_expr") != NULL)
							load_complex_params(object,cfgfile,section);
					}
					else
					{
						if (dbg_lvl & (RTMLOADER|CRITICAL))
							dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tMTX_BOOL: read of key \"%s\" from section \"%s\" failed\n",keys[j],section));
					}
					break;
				case MTX_STRING:
					if(cfg_read_string(cfgfile,section,keys[j],&tmpbuf))
					{
						if (dbg_lvl & RTMLOADER)
							dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tbinding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n",keys[j],tmpbuf,section));
						OBJ_SET(object,
								keys[j],
								g_strdup(tmpbuf));
						if (strstr(keys[j],"internal_names") != NULL)
						{
							vector = g_strsplit(tmpbuf,",",-1); 
							for(k=0;k<g_strv_length(vector);k++) 
								g_hash_table_insert(rtv_map->rtv_hash,g_strdup(vector[k]),(gpointer)object);
							g_strfreev(vector);
						}
						g_free(tmpbuf);
					}
					else
					{
						if (dbg_lvl & (RTMLOADER|CRITICAL))
							dbg_func(g_strdup_printf(__FILE__": load_realtime_map_pf()\n\tMTX_STRING: read of key \"%s\" from section \"%s\" failed\n",keys[j],section));
					}
					break;
				default:
					break;

			}
		}
		list = g_hash_table_lookup(rtv_map->offset_hash,GINT_TO_POINTER(offset));
		list = g_list_prepend(list,(gpointer)object);
		g_hash_table_replace(rtv_map->offset_hash,GINT_TO_POINTER(offset),list);
		g_array_append_val(rtv_map->rtv_list,object);

		g_free(section);
		g_free(keytypes);
		g_strfreev(keys);
	}
	cfg_free(cfgfile);
	g_free(cfgfile);
	if (dbg_lvl & RTMLOADER)
		dbg_func(g_strdup(__FILE__": load_realtime_map_pf()\n\t All is well, leaving...\n\n"));
	rtvars_loaded = TRUE;
	set_title(g_strdup("RT Map loaded..."));
	return TRUE;
}


/*!
 \brief load_complex_params() loads the necessary parameters from the config
 file for a complex conversion
 \param object (GObject *) the place where the data loaded is bound to
 \param cfgfile (ConfigFile *) configfile pointer to read from
 \param section (gchar *) section to read from in the config file
 */
void load_complex_params(GObject *object, ConfigFile *cfgfile, gchar * section)
{
	gchar *tmpbuf = NULL;
	gchar **expr_symbols = NULL;
	gint *expr_types = NULL;
	gint total_symbols = 0;
	gint total_symtypes = 0;
	gchar * name = NULL;
	gint tmpi;
	gint i = 0;
	extern Firmware_Details *firmware;

	if (!cfg_read_string(cfgfile,section,"expr_symbols",&tmpbuf))
	{
		if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": load_complex_params()\n\tRead of \"expr_symbols\" from section \"[%s]\" failed ABORTING!!!\n\n",section));
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
		if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": load_complex_params()\n\tRead of \"expr_types\" from section \"[%s]\" failed, ABORTING!!!\n\n",section));
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
		if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": load_complex_params()\n\tNumber of symbols(%i) and symbol types(%i)\n\tare different, ABORTING!!!\n\n",total_symbols,total_symtypes));
		g_free(expr_types);
		g_free(expr_symbols);
		return;
	}
	/* Store the lists as well so DO NOT DEALLOCATE THEM!!! */
	OBJ_SET(object,"expr_types",(gpointer)expr_types);
	OBJ_SET(object,"expr_symbols",(gpointer)expr_symbols);
	OBJ_SET(object,"total_symbols",GINT_TO_POINTER(total_symbols));
	for (i=0;i<total_symbols;i++)
	{
		switch ((ComplexExprType)expr_types[i])
		{
			case VE_EMB_BIT:
				/* VE Table embedded bitfield 4 params */
				name=NULL;
				name=g_strdup_printf("%s_page",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
				{
					if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__": load_compex_params()\n\tVE_EMB_BIT, failure looking for:%s\n",name));
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
				{
					if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
					dbg_func(g_strdup_printf(__FILE__": load_compex_params()\n\tVE_EMB_BIT, failure looking for:%s\n",name));
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_canID",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					tmpi = firmware->canID;
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_bitmask",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
				{
					if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__": load_compex_params()\n\tVE_EMB_BIT, failure looking for:%s\n",name));
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_bitshift",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
				{
					if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__": load_compex_params()\n\tVE_EMB_BIT, failure looking for:%s\n",name));
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				break;
			case VE_VAR:
				/* VE table std variable,  page/offset only */
				name=NULL;
				name=g_strdup_printf("%s_page",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
				{
					if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__": load_compex_params()\n\tVE_VAR, failure looking for:%s\n",name));
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
				{
					if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__": load_compex_params()\n\tVE_VAR, failure looking for:%s\n",name));
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_canID",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					tmpi = firmware->canID;

				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_size",expr_symbols[i]);
				if (!cfg_read_string(cfgfile,section,name,&tmpbuf))
					tmpi = MTX_U08;
				else
				{
					tmpi = translate_string(tmpbuf);
					g_free(tmpbuf);
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				break;
			case RAW_VAR:
				/* RAW variable */
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
				{
					if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__": load_compex_params()\n\tRAW_VAR, failure looking for:%s\n",name));
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				name=NULL;
				name=g_strdup_printf("%s_size",expr_symbols[i]);
				if (!cfg_read_string(cfgfile,section,name,&tmpbuf))
				{
					if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__": load_compex_params()\n\tRAW_VAR, failure looking for:%s\n",name));
					tmpi = MTX_U08;
				}
				else
				{
					tmpi = translate_string(tmpbuf);
					g_free(tmpbuf);
				}
				OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
				g_free(name);
				break;
			default:
				if (dbg_lvl & (RTMLOADER|COMPLEX_EXPR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_complex_params(), expr_type is UNDEFINED, this will cause a crash!!\n"));

		}
	}

}
