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
#include <rtv_processor.h>
#include <structures.h>


void process_rt_vars(void *incoming)
{
	extern struct RtvMap *rtv_map;
	extern struct Firmware_Details *firmware;
	gint len =  0;
	GList * list= NULL;
	gint i = 0;
	gint j = 0;
	
	len = firmware->rtvars_size;
	if (len != rtv_map->raw_total)
	{
		dbg_func(g_strdup_printf(__FILE__": process_rt_vars(): length of buffer(%i) and relatime map raw_length(%i) DO NOT match, critical ERROR!\n",len,rtv_map->raw_total),CRITICAL);
		return;
	}
	for (i=0;i<len;i++)
	{
		/* Get list of derived vars for raw offset "i" */
		list = g_array_index(rtv_map->rtv_array,GList *,i);
		if (list == NULL) /* no derived vars for this variable */
			continue;
		list = g_list_first(list);
		for (j=0;j<g_list_length(list);j++)
		{
			
		}
	}
	return;
}
