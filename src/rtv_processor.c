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
	extern gint temp_units;
	unsigned char *raw_realtime = incoming;
	GObject * object = NULL;
	gint len =  0;
	GList * list= NULL;
	gint i = 0;
	gint j = 0;
	gfloat x = 0;
	gint offset = 0;
	gfloat result = 0.0;
	gfloat tmpf = 0.0;
	void *evaluator = NULL;
	
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
			object=(GObject *)g_list_nth_data(list,j);
			offset = (gint)g_object_get_data(object,"offset");
			if (g_object_get_data(object,"complex_expr"))
			{
				handle_complex_expr(object);
				break;
			}

			if (g_object_get_data(object,"lookuptable"))
				x = lookup_data(object,offset);
			else
				x = raw_realtime[offset];

			evaluator = (void *)g_object_get_data(object,"evaluator");
			assert(evaluator);
			tmpf = evaluator_evaluate_x(evaluator,x);
			if (temp_units == CELSIUS)
				result = (tmpf-32)*(5.0/9.0);
			else
				result = tmpf;

		}
	}
	return;
}

gfloat lookup_data(GObject *object, gint offset)
{
	extern GHashTable *lookuptables;
	gint *lookuptable = NULL;
	gchar *table = NULL;

	table = (gchar *)g_object_get_data(object,"lookuptable");
	lookuptable = (gint *)g_hash_table_lookup(lookuptables,table);	
	//assert(lookuptable);
	if (!lookuptable)
		return 0;
	return lookuptable[offset];
}

void handle_complex_expr(GObject *object)
{
	return;
}

