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
#include <structures.h>
#include <tabloader.h>

GArray *raw_array;

void realtime_map_load(void *ptr, gchar *basename)
{
	ConfigFile *cfgfile;
	struct Firmware_Details *firmware = ptr;
	gchar * filename = NULL;
	gchar *tmpbuf = NULL;
	gint raw_total = 0;
	gint derived_total = 0;
	gint num_keys = 0;
	gint num_keytypes = 0;
	gchar ** keys = NULL;
	gint i = 0;
	gchar * section = NULL;
	gint keytypes[50]; /* bad idea... */
	

	filename = g_strconcat(DATA_DIR,"/",REALTIME_MAP_DIR,"/",basename,NULL);
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
			return;
			
		}
		/* OK, basic checks passed,  start loading data into
		 * the main mapping structures...
		 */
		if(!cfg_read_int(cfgfile,"realtime_map","raw_total",&raw_total))
			dbg_func(__FILE__": realtime_map_load(), can't find \"raw_total\" in the \"[realtime_map]\" section\n",CRITICAL);
		if(!cfg_read_int(cfgfile,"realtime_map","derived_total",&derived_total))
			dbg_func(__FILE__": realtime_map_load(), can't find \"derived_total\" in the \"[realtime_map]\" section\n",CRITICAL);

		raw_array = g_array_sized_new(FALSE,TRUE,sizeof(GList *),raw_total);
//		for (i=0;i<raw_total;i++)
//			g_array_insert_val(raw_array,i,NULL);
			
		for (i=0;i<derived_total;i++)
		{
			section = g_strdup_printf("derived_%i",i);
			/* Get key list and parse */
			if(!cfg_read_string(cfgfile,section,"keys",&tmpbuf))
				dbg_func(g_strdup_printf(__FILE__": realtime_map_load(), can't find \"keys\" in the \"[derived_%i]\" section\n",i),CRITICAL);
			else
			{
				keys = parse_keys(tmpbuf,&num_keys);
				g_free(tmpbuf);
			}
			/* Get key TYPE list and parse */
			if(!cfg_read_string(cfgfile,section,"key_types",&tmpbuf))
				dbg_func(g_strdup_printf(__FILE__": realtime_map_load(), can't find \"key_types\" in the \"[derived_%i]\" section\n",i),CRITICAL);
			else
			{
				parse_keytypes(tmpbuf,keytypes, &num_keytypes);
				g_free(tmpbuf);
			}

			
			g_free(section);
		}
		
	}
}
