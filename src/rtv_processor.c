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
#include <glade/glade.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <rtv_processor.h>
#include <stdlib.h>
#include <structures.h>


void process_rt_vars(void *incoming)
{
	extern struct RtvMap *rtv_map;
	extern struct Firmware_Details *firmware;
	extern gint temp_units;
	guchar *raw_realtime = incoming;
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
			evaluator = (void *)g_object_get_data(object,"evaluator");
			if (!evaluator)
			{
				evaluator = evaluator_create(g_object_get_data(object,"conv_expr"));
				assert(evaluator);
				g_object_set_data(object,"evaluator",evaluator);
			}
			else
				assert(evaluator);
			offset = (gint)g_object_get_data(object,"offset");
			if (g_object_get_data(object,"complex_expr"))
			{
				result = handle_complex_expr(object,incoming);
				printf("Result of COMPLEX %s is %f\n",(gchar *)g_object_get_data(object,"internal_name"),result);
				continue;
			}

			if (g_object_get_data(object,"lookuptable"))
				x = lookup_data(object,raw_realtime[offset]);
			else
				x = raw_realtime[offset];


			tmpf = evaluator_evaluate_x(evaluator,x);
			if (temp_units == CELSIUS)
				result = (tmpf-32)*(5.0/9.0);
			else
				result = tmpf;
			printf("Result of %s is %f\n",(gchar *)g_object_get_data(object,"internal_name"),result);

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

gdouble handle_complex_expr(GObject *object, void * incoming)
{
	extern guchar *ms_data[MAX_SUPPORTED_PAGES];
	gchar **symbols = NULL;
	gchar **expr_types = NULL;
	guchar *raw_data = incoming;
	gint total_symbols = 0;
	gint i = 0;
	gint page = 0;
	gint offset = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	void * evaluator = NULL;
	gchar **names = NULL;
	gdouble * values = NULL;
	gchar * tmpbuf = NULL;
	gdouble result = 0.0;


	symbols = (gchar **)g_object_get_data(object,"expr_symbols");
	expr_types = (gchar **)g_object_get_data(object,"expr_types");
	total_symbols = (gint)g_object_get_data(object,"total_symbols");

	names = g_malloc0(total_symbols*sizeof(gchar *));
	values = g_malloc0(total_symbols*sizeof(gdouble));

	for (i=0;i<total_symbols;i++)
	{
		page = 0;
		offset = 0;
		bitmask = 0;
		bitshift = 0;
		switch ((ComplexExprType)expr_types[i])
		{
			case VE_EMB_BIT:
				tmpbuf = g_strdup_printf("%s_page",symbols[i]);
				page = (gint) g_object_get_data(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (gint) g_object_get_data(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_bitmask",symbols[i]);
				bitmask = (gint) g_object_get_data(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_bitshift",symbols[i]);
				bitshift = (gint) g_object_get_data(object,tmpbuf);
				g_free(tmpbuf);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)(((ms_data[page][offset])&bitmask) >> bitshift);
				dbg_func(g_strdup_printf(__FILE__": handle_complex_expr()\n\t Embedded bit, name: %s, value %f\n",names[i],values[i]),COMPLEX_EXPR);
				break;
			case VE_VAR:
				tmpbuf = g_strdup_printf("%s_page",symbols[i]);
				page = (gint) g_object_get_data(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (gint) g_object_get_data(object,tmpbuf);
				g_free(tmpbuf);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)ms_data[page][offset];
				dbg_func(g_strdup_printf(__FILE__": handle_complex_expr()\n\t VE Variable, name: %s, value %f\n",names[i],values[i]),COMPLEX_EXPR);
				break;
			case RAW_VAR:
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (gint) g_object_get_data(object,tmpbuf);
				g_free(tmpbuf);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)raw_data[offset];
				dbg_func(g_strdup_printf(__FILE__": handle_complex_expr()\n\t RAW Variable, name: %s, value %f\n",names[i],values[i]),COMPLEX_EXPR);
		}

	}
	evaluator = g_object_get_data(object,"evaluator");
	if (!evaluator)
	{
		evaluator = evaluator_create(g_object_get_data(object,"conv_expr"));
		if (!evaluator)
		{
			dbg_func(g_strdup_printf(__FILE__": handle_complex_expr()\n\t evaluator missing for %s,\n\texpression is %s\n",(gchar *)glade_get_widget_name(GTK_WIDGET(object)),(gchar *)g_object_get_data(object,"conv_expr")),CRITICAL);
			exit (-1);
		}
		else
			g_object_set_data(object,"evaluator",evaluator);
	}
	else
		assert(evaluator);
	result = evaluator_evaluate(evaluator,total_symbols,names,values);
	for (i=0;i<total_symbols;i++)
	{
		dbg_func(g_strdup_printf(__FILE__": handle_complex_expr()\n\tkey %s value %f\n",names[i],values[i]),COMPLEX_EXPR);
		g_free(names[i]);
	}
	dbg_func(g_strdup_printf(__FILE__": handle_complex_expr()\n\texpression is %s\n",evaluator_get_string(evaluator)),COMPLEX_EXPR);
	g_free(names);
	g_free(values);
	return result;
}
