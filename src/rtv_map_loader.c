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

#include <assert.h>
#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <rtv_map_loader.h>
#include <string.h>
#include <stringmatch.h>
#include <structures.h>
#include <tabloader.h>

GArray *raw_array;

gboolean load_realtime_map(void )
{
	ConfigFile *cfgfile;
	extern struct Firmware_Details *firmware;
	gchar * filename = NULL;
	gchar *tmpbuf = NULL;
	gint raw_total = 0;
	gint derived_total = 0;
	gint num_keys = 0;
	gint num_keytypes = 0;
	gchar ** keys = NULL;
	gint *keytypes = NULL;
	gint i = 0;
	gint j = 0;
	gint tmpi = 0;
	gint offset = 0;
	gchar * section = NULL;
	GObject * object = NULL;
	GList * list = NULL;
	
	filename = g_strconcat(DATA_DIR,"/",REALTIME_MAP_DIR,"/",firmware->rtv_map_file,".rtv_map",NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		if(!cfg_read_string(cfgfile,"realtime_map","firmware_name",&tmpbuf))
			dbg_func(__FILE__": realtime_map_load(), can't find firmware name\n",CRITICAL);
		if (g_strcasecmp(tmpbuf,firmware->name) != 0)	
		{
			dbg_func(g_strdup_printf(__FILE__": realtime_map_load(), firmware name(%s) in this file(%s) does NOT match firmware name(%s) of loaded firmware, ABORT!\n",tmpbuf,filename,firmware->name),CRITICAL);
			cfg_free(cfgfile);
			g_free(cfgfile);
			g_free(filename);
			return FALSE;

		}
		/* OK, basic checks passed,  start loading data into
		 * the main mapping structures...
		 */
		if(!cfg_read_int(cfgfile,"realtime_map","raw_total",&raw_total))
			dbg_func(__FILE__": realtime_map_load(), can't find \"raw_total\" in the \"[realtime_map]\" section\n",CRITICAL);
		if(!cfg_read_int(cfgfile,"realtime_map","derived_total",&derived_total))
			dbg_func(__FILE__": realtime_map_load(), can't find \"derived_total\" in the \"[realtime_map]\" section\n",CRITICAL);

		raw_array = g_array_sized_new(FALSE,TRUE,sizeof(GList *),raw_total);
		list = NULL;
		for (i=0;i<raw_total;i++)
			g_array_insert_val(raw_array,i,list);

		for (i=0;i<derived_total;i++)
		{
			section = g_strdup_printf("derived_%i",i);
			/* Get key list and parse */
			if(!cfg_read_string(cfgfile,section,"keys",&tmpbuf))
			{
				dbg_func(g_strdup_printf(__FILE__": realtime_map_load(), can't find \"keys\" in the \"[derived_%i]\" section\n",i),CRITICAL);
				return FALSE;
			}
			else
			{
				keys = parse_keys(tmpbuf,&num_keys);
				g_free(tmpbuf);
			}
			/* Get key TYPE list and parse */
			if(!cfg_read_string(cfgfile,section,"key_types",&tmpbuf))
			{
				dbg_func(g_strdup_printf(__FILE__": realtime_map_load(), can't find \"key_types\" in the \"[derived_%i]\" section\n",i),CRITICAL);
				return FALSE;
			}
			else
			{
				keytypes = parse_keytypes(tmpbuf, &num_keytypes);
				g_free(tmpbuf);
			}
			if (num_keytypes != num_keys)
			{
				dbg_func(g_strdup_printf(__FILE__": Number of keys (%i) and keytypes(%i) does NOT match for: \"%s\", CRITICAL!!!\n",num_keys,num_keytypes,section),CRITICAL);
				g_free(keytypes);
				g_strfreev(keys);
				return FALSE;
			}
			if (!cfg_read_int(cfgfile,section,"offset",&offset))
				dbg_func(g_strdup_printf(__FILE__": Offset not defined for: \"%s\", CRITICAL!!!\n",section),CRITICAL);
	
				
			object = g_object_new(GTK_TYPE_INVISIBLE,NULL);
			for (j=0;j<num_keys;j++)
			{
				switch((DataType)keytypes[j])
				{
					case MTX_INT:
						if (cfg_read_int(cfgfile,section,keys[j],&tmpi))
						{
							dbg_func(g_strdup_printf(__FILE__": bind_data() binding INT \"%s\",\"%i\" to widget \"%s\"\n",keys[j],tmpi,section),TABLOADER);
							g_object_set_data(object,
									g_strdup(keys[j]),
									GINT_TO_POINTER(tmpi));
						}
						else
							dbg_func(g_strdup_printf(__FILE__": bind_data(), MTX_INT: read of key \"%s\" from section \"%s\" failed\n",keys[j],section),CRITICAL);
						break;
					case MTX_ENUM:
						if (cfg_read_string(cfgfile,section,keys[j],&tmpbuf))
						{
							tmpi = translate_string(tmpbuf);
							dbg_func(g_strdup_printf(__FILE__": bind_data() binding ENUM \"%s\",\"%i\" to widget \"%s\"\n",keys[j],tmpi,section),TABLOADER);
							g_object_set_data(object,
									g_strdup(keys[j]),
									GINT_TO_POINTER(tmpi));
							g_free(tmpbuf);
						}
						else
							dbg_func(g_strdup_printf(__FILE__": bind_data(), MTX_ENUM: read of key \"%s\" from section \"%s\" failed\n",keys[j],section),CRITICAL);
						break;
					case MTX_BOOL:
						if (cfg_read_boolean(cfgfile,section,keys[j],&tmpi))
						{
							dbg_func(g_strdup_printf(__FILE__": bind_data() binding BOOL \"%s\",\"%i\" to widget \"%s\"\n",keys[j],tmpi,section),TABLOADER);
							g_object_set_data(object,
									g_strdup(keys[j]),
									GINT_TO_POINTER(tmpi));
							if (strstr(keys[j],"complex_expr") != NULL)
								load_complex_params(object,cfgfile,section);
						}
						else
							dbg_func(g_strdup_printf(__FILE__": bind_data(), MTX_BOOL: read of key \"%s\" from section \"%s\" failed\n",keys[j],section),CRITICAL);
						break;
					case MTX_STRING:
						if(cfg_read_string(cfgfile,section,keys[j],&tmpbuf))
						{
							dbg_func(g_strdup_printf(__FILE__": bind_data() binding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n",keys[j],tmpbuf,section),TABLOADER);
							g_object_set_data(object,
									g_strdup(keys[j]),
									g_strdup(tmpbuf));
							g_free(tmpbuf);
						}
						else
							dbg_func(g_strdup_printf(__FILE__": bind_data(), MTX_STRING: read of key \"%s\" from section \"%s\" failed\n",keys[j],section),CRITICAL);
						break;

				}
			}
			list = g_array_index(raw_array,GList *,offset);
			list = g_list_append(list,(gpointer)object);
			raw_array = g_array_insert_val(raw_array,offset,list);

			g_free(section);
			g_free(keytypes);
			g_strfreev(keys);
		}

	}
	return TRUE;
}

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

	if (!cfg_read_string(cfgfile,section,"expr_symbols",&tmpbuf))
	{
		dbg_func(g_strdup_printf(__FILE__": load_complex_params(), read of \"expr_symbols\" from section \"%s\" failed\n",section),CRITICAL);
		g_free(tmpbuf);
		return;
	}
	else
	{
		expr_symbols = parse_keys(tmpbuf, &total_symbols);	
		g_free(tmpbuf);
	}
	if (!cfg_read_string(cfgfile,section,"expr_types",&tmpbuf))
	{
		dbg_func(g_strdup_printf(__FILE__": load_complex_params(), read of \"expr_types\" from section \"%s\" failed\n",section),CRITICAL);
		g_free(tmpbuf);
		return;
	}
	else
	{
		expr_types = parse_keytypes(tmpbuf, &total_symtypes);	
		g_free(tmpbuf);
	}
	if (total_symbols!=total_symtypes)
	{
		dbg_func(g_strdup_printf(__FILE__": load_complex_params(), Number of symbols(%i) and symbol types(%i) are different, ABORTING\n",total_symbols,total_symtypes),CRITICAL);
		g_free(expr_types);
		g_free(expr_symbols);
		return;
	}
	/* Store the lists as well so DO NOT DEALLOCATE THEM!!! */
	g_object_set_data(object,"expr_types",(gpointer)expr_types);
	g_object_set_data(object,"expr_symbols",(gpointer)expr_symbols);
	for (i=0;i<total_symbols;i++)
	{
		switch ((ComplexExprType)expr_types[i])
		{
			case VE_EMB_BIT:
				/* VE Table embedded bitfield 4 params */
				name=NULL;
				name=g_strdup_printf("%s_page",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					dbg_func(g_strdup_printf(__FILE__": load_compex_params(), VE_EMB+BIT, failure looking for:\n\t%s\n",name),CRITICAL);
				 g_object_set_data(object,name,GINT_TO_POINTER(tmpi));
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					dbg_func(g_strdup_printf(__FILE__": load_compex_params(), VE_EMB+BIT, failure looking for:\n\t%s\n",name),CRITICAL);
				 g_object_set_data(object,name,GINT_TO_POINTER(tmpi));
				name=NULL;
				name=g_strdup_printf("%s_bitmask",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					dbg_func(g_strdup_printf(__FILE__": load_compex_params(), VE_EMB+BIT, failure looking for:\n\t%s\n",name),CRITICAL);
				 g_object_set_data(object,name,GINT_TO_POINTER(tmpi));
				name=NULL;
				name=g_strdup_printf("%s_bitshift",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					dbg_func(g_strdup_printf(__FILE__": load_compex_params(), VE_EMB+BIT, failure looking for:\n\t%s\n",name),CRITICAL);
				 g_object_set_data(object,name,GINT_TO_POINTER(tmpi));
				name=NULL;
				break;
			case VE_VAR:
				/* VE table std variable,  page/offset only */
				name=NULL;
				name=g_strdup_printf("%s_page",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					dbg_func(g_strdup_printf(__FILE__": load_compex_params(), VE_EMB+BIT, failure looking for:\n\t%s\n",name),CRITICAL);
				 g_object_set_data(object,name,GINT_TO_POINTER(tmpi));
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					dbg_func(g_strdup_printf(__FILE__": load_compex_params(), VE_EMB+BIT, failure looking for:\n\t%s\n",name),CRITICAL);
				 g_object_set_data(object,name,GINT_TO_POINTER(tmpi));
				name=NULL;
				break;
			case RAW_VAR:
				/* RAW variable */
				name=NULL;
				name=g_strdup_printf("%s_offset",expr_symbols[i]);
				if (!cfg_read_int(cfgfile,section,name,&tmpi))
					dbg_func(g_strdup_printf(__FILE__": load_compex_params(), VE_EMB+BIT, failure looking for:\n\t%s\n",name),CRITICAL);
				 g_object_set_data(object,name,GINT_TO_POINTER(tmpi));
				name=NULL;
				break;

		}
	}
	
}
