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

#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <rtv_map_loader.h>
#include <structures.h>
#include <tabloader.h>

void realtime_map_load(void *ptr, gchar *basename)
{
	ConfigFile *cfgfile;
	struct Firmware_Details *firmware = ptr;
	gchar * filename = NULL;
	gchar *tmpbuf = NULL;

	filename = g_strconcat(DATA_DIR,"/",REALTIME_MAP_DIR,"/",basename,NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		if(!cfg_read_string(cfgfile,"realtime_map","name",&tmpbuf))
			dbg_func(__FILE__": realtime_map_load(), can't find firmware name\n",CRITICAL);
		if (g_strcasecmp(tmpbuf,firmware->name) != 0)	
		{
			dbg_func(g_strdup_printf(__FILE__": realtime_map_load(), firmware name(%s) in this file(%s) does NOT match firmware name(%s) of loaded firmware, ABORT!\n",tmpbuf,filename,firmware->name),CRITICAL);
			cfg_free(cfgfile);
			g_free(cfgfile);
			g_free(filename);
			return;
			
		}
		
		
	}
	
	
	
}
