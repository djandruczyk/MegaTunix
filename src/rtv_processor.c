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
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <dep_processor.h>
#include <enums.h>
#include <firmware.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <lookuptables.h>
#include <math.h>
#include <mtxmatheval.h>
#include <multi_expr_loader.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <widgetmgmt.h>


extern GStaticMutex rtv_mutex;
extern Firmware_Details *firmware;
extern gconstpointer *global_data;

/*!
 \brief process_rt_vars() processes incoming realtime variables. It's a pretty
 complex function so read the sourcecode.. ;)
 \param incoming (void *) pointer to the raw incoming data
 */
G_MODULE_EXPORT void process_rt_vars(void *incoming)
{
	extern Rtv_Map *rtv_map;
	gint temp_units;
	guchar *raw_realtime = incoming;
	gconstpointer * object = NULL;
	gchar * expr = NULL;
	GList * list= NULL;
	guint i = 0;
	guint j = 0;
	gfloat x = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gfloat result = 0.0;
	gfloat tmpf = 0.0;
	gboolean temp_dep = FALSE;
	void *evaluator = NULL;
	GTimeVal timeval;
	GArray *history = NULL;
	gchar *special = NULL;
	GHashTable *hash = NULL;
	GTimeVal curr;
	GTimeVal begin;
	gint hours = 0;
	gint minutes = 0;
	gint seconds = 0;

	if (!incoming)
		printf(_("ERROR, INPUT IS NULL!!!!\n"));
	/* Store timestamps in ringbuffer */

	/* Backup current rtv copy */
	memcpy(firmware->rt_data_last,firmware->rt_data,firmware->rtvars_size);
	memcpy(firmware->rt_data,incoming,firmware->rtvars_size);
	temp_units = (GINT)DATA_GET(global_data,"temp_units");
	g_get_current_time(&timeval);
	g_array_append_val(rtv_map->ts_array,timeval);
	if (rtv_map->ts_array->len%250 == 0)
	{
		curr = g_array_index(rtv_map->ts_array,GTimeVal, rtv_map->ts_array->len-1);
		begin = g_array_index(rtv_map->ts_array,GTimeVal,0);
		tmpf = curr.tv_sec-begin.tv_sec-((curr.tv_usec-begin.tv_usec)/1000000);
		hours = tmpf > 3600.0 ? floor(tmpf/3600.0) : 0;
		tmpf -= hours*3600.0;
		minutes = tmpf > 60.0 ? floor(tmpf/60.0) : 0;
		tmpf -= minutes*60.0;
		seconds = (gint)tmpf;

		thread_update_logbar("dlog_view",NULL,g_strdup_printf(_("Currently %i samples stored, Total Logged Time (HH:MM:SS) (%02i:%02i:%02i)\n"),rtv_map->ts_array->len,hours,minutes,seconds),FALSE,FALSE);
	}

	for (i=0;i<rtv_map->rtvars_size;i++)
	{
		/* Get list of derived vars for raw offset "i" */
		list = g_hash_table_lookup(rtv_map->offset_hash,GINT_TO_POINTER(i));
		if (list == NULL) /* no derived vars for this variable */
			continue;
		list = g_list_first(list);
		for (j=0;j<g_list_length(list);j++)
		{
			history = NULL;
			special = NULL;
			hash = NULL;
			object=(gconstpointer *)g_list_nth_data(list,j);
/*			printf("Dumping datalist for objects\n");
			 g_datalist_foreach(object,dump_datalist,NULL);
			 */
			if (!object)
			{
				dbg_func(COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": rtv_processor()\n\t Object bound to list at offset %i is invalid!!!!\n",i));
				continue;
			}
			special = (gchar *)DATA_GET(object,"special");
			if (special)
			{
				tmpf = handle_special(object,special);
				goto store_it;
			}
			temp_dep = (GBOOLEAN)DATA_GET(object,"temp_dep");
			hash = (GHashTable *)DATA_GET(object,"multi_expr_hash");
			if (hash)
			{
				tmpf = handle_multi_expression(object,raw_realtime,hash);
				goto store_it;
			}
			evaluator = (void *)DATA_GET(object,"ul_evaluator");
			if (!evaluator)
			{
				expr = DATA_GET(object,"ul_conv_expr");
				if (expr == NULL)
				{
					dbg_func(COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": process_rt_vars()\n\t \"ul_conv_expr\" was NULL for control \"%s\", EXITING!\n",(gchar *)DATA_GET(object,"internal_names")));
					exit (-3);
				}
				evaluator = evaluator_create(expr);
				if (!evaluator)
				{
					dbg_func(COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": rtv_processor()\n\t Creating of evaluator for function \"%s\" FAILED!!!\n\n",expr));
				}
				assert(evaluator);
				DATA_SET_FULL(object,"ul_evaluator",evaluator,evaluator_destroy);
			}
			else
				assert(evaluator);
			offset = (GINT)DATA_GET(object,"offset");
			size = (DataSize)DATA_GET(object,"size");
			if (DATA_GET(object,"complex_expr"))
			{
				tmpf = handle_complex_expr(object,incoming,UPLOAD);
				goto store_it;
			}

			if (DATA_GET(object,"lookuptable"))
			{
				/*dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": process_rt_vars()\n\tgetting Lookuptable for var using offset %i\n",offset));*/
				x = lookup_data(object,raw_realtime[offset]);
			}
			else
			{
				/*dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": process_rt_vars()\n\tNo Lookuptable needed for var using offset %i\n",offset));*/
				x = _get_sized_data((guint8 *)incoming,0,offset,size,firmware->bigendian);
			}

			/*dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": process_rt_vars()\n\texpression is %s\n",evaluator_get_string(evaluator))); */
			tmpf = evaluator_evaluate_x(evaluator,x);
store_it:
			if (temp_dep)
			{
				/*dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": process_rt_vars()\n\tvar at offset %i is temp dependant.\n",offset));*/
				if (temp_units == CELSIUS)
					result = (tmpf-32.0)*(5.0/9.0);
				else
					result = tmpf;
			}
			else
				result = tmpf;
			/* Get history array and current index point */
			history = (GArray *)DATA_GET(object,"history");
			/* Store data in history buffer */
			g_static_mutex_lock(&rtv_mutex);
			g_array_append_val(history,result);
			/*printf("array size %i, current index %i, appended %f, readback %f previous %f\n",history->len,history->len-1,result,g_array_index(history, gfloat, history->len-1),g_array_index(history, gfloat, history->len-2));*/
			g_static_mutex_unlock(&rtv_mutex);

			/*printf("Result of %s is %f\n",(gchar *)DATA_GET(object,"internal_names"),result);*/

		}
	}
	return;
}


/*!
 \brief handle_complex_expr() handles a complex mathematcial expression for
 an variable represented by a gconstpointer.
 \param object (gconstpointer *) pointer to the object containing the conversion 
 expression and other relevant data
 \param incoming (void *) pointer to the raw data
 \param type (ConvType) enumeration stating if this is an upload or
 download conversion
 \returns a float of the result of the mathematical expression
 */
G_MODULE_EXPORT gfloat handle_complex_expr(gconstpointer *object, void * incoming,ConvType type)
{
	gchar **symbols = NULL;
	gint *expr_types = NULL;
	guchar *raw_data = incoming;
	gint total_symbols = 0;
	gint i = 0;
	gint page = 0;
	DataSize size = MTX_U08;
	gint offset = 0;
	guint bitmask = 0;
	guint bitshift = 0;
	gint canID = 0;
	void * evaluator = NULL;
	gchar **names = NULL;
	gdouble * values = NULL;
	gchar * tmpbuf = NULL;
	gdouble lower_limit = 0;
	gdouble upper_limit = 0;
	gdouble result = 0.0;


	symbols = (gchar **)DATA_GET(object,"expr_symbols");
	expr_types = (gint *)DATA_GET(object,"expr_types");
	total_symbols = (GINT)DATA_GET(object,"total_symbols");
	if (DATA_GET(object,"real_lower"))
		lower_limit = strtod(DATA_GET(object,"real_lower"),NULL);
	else
		lower_limit = -G_MAXDOUBLE;
	if (DATA_GET(object,"real_upper"))
		upper_limit = strtod(DATA_GET(object,"real_upper"),NULL);
	else
		upper_limit = G_MAXDOUBLE;

	names = g_new0(gchar *, total_symbols);
	values = g_new0(gdouble, total_symbols);

	for (i=0;i<total_symbols;i++)
	{
		page = 0;
		offset = 0;
		bitmask = 0;
		bitshift = 0;
		switch ((ComplexExprType)expr_types[i])
		{
			case VE_EMB_BIT:
				size = MTX_U08;

				tmpbuf = g_strdup_printf("%s_page",symbols[i]);
				page = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_canID",symbols[i]);
				canID = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_bitmask",symbols[i]);
				bitmask = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				bitshift = get_bitshift(bitmask);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)(((get_ecu_data(canID,page,offset,size)) & bitmask) >> bitshift);
				/*
				   printf("raw ecu at page %i, offset %i is %i\n",page,offset,get_ecu_data(canID,page,offset,size));
				   printf("value masked by %i, shifted by %i is %i\n",bitmask,bitshift,(get_ecu_data(canID,page,offset,size) & bitmask) >> bitshift);
				 */
				dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\t Embedded bit, name: %s, value %f\n",names[i],values[i]));
				break;
			case VE_VAR:
				tmpbuf = g_strdup_printf("%s_page",symbols[i]);
				page = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_canID",symbols[i]);
				canID = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_size",symbols[i]);
				size = (DataSize) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)get_ecu_data(canID,page,offset,size);
				dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\t VE Variable, name: %s, value %f\n",names[i],values[i]));
				break;
			case RAW_VAR:
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_size",symbols[i]);
				size = (DataSize) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)_get_sized_data(raw_data,0,offset,size,firmware->bigendian);
				dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\t RAW Variable, name: %s, value %f\n",names[i],values[i]));
				break;
			case RAW_EMB_BIT:
				size = MTX_U08;
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_bitmask",symbols[i]);
				bitmask = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				bitshift = get_bitshift(bitmask);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)(((_get_sized_data(raw_data,0,offset,size,firmware->bigendian)) & bitmask) >> bitshift);
				dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\t RAW Embedded Bit, name: %s, value %f\n",names[i],values[i]));
				break;
			default:
				dbg_func(COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": handle_complex_expr()\n\t UNDEFINE Variable, this will cause a crash!!!!\n"));
				break;
		}

	}
	if (type == UPLOAD)
	{
		evaluator = (void *)DATA_GET(object,"ul_evaluator");
		if (!evaluator)
		{
			evaluator = evaluator_create(DATA_GET(object,"ul_conv_expr"));
			DATA_SET_FULL(object,"ul_evaluator",evaluator,evaluator_destroy);

		}
	}
	else if (type == DOWNLOAD)
	{
		evaluator = (void *)DATA_GET(object,"dl_evaluator");
		if (!evaluator)
		{
			evaluator = evaluator_create(DATA_GET(object,"dl_conv_expr"));
			DATA_SET_FULL(object,"dl_evaluator",evaluator,evaluator_destroy);
		}
	}
	else
	{
		dbg_func(COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": handle_complex_expr()\n\tevaluator type undefined for %s\n",(gchar *)glade_get_widget_name(GTK_WIDGET(object))));
	}
	if (!evaluator)
	{
		dbg_func(COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": handle_complex_expr()\n\tevaluator missing for %s\n",(gchar *)glade_get_widget_name(GTK_WIDGET(object))));
		exit (-1);
	}

	assert(evaluator);

	result = evaluator_evaluate(evaluator,total_symbols,names,values);
	if (result < lower_limit)
		result = lower_limit;
	if (result > upper_limit)
		result = upper_limit;
	for (i=0;i<total_symbols;i++)
	{
		dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\tkey %s value %f\n",names[i],values[i]));
		g_free(names[i]);
	}
	dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\texpression is %s\n",evaluator_get_string(evaluator)));
	g_free(names);
	g_free(values);
	return result;
}



/*!
 \brief handle_complex_expr_obj() handles a complex mathematcial expression for
 an variable represented by a gconstpointer.
 \param object (gconstpointer *) pointer to the object containing the conversion 
 expression and other relevant data
 \param incoming (void *) pointer to the raw data
 \param type (ConvType) enumeration stating if this is an upload or
 download conversion
 \returns a float of the result of the mathematical expression
 */
G_MODULE_EXPORT gfloat handle_complex_expr_obj(GObject *object, void * incoming,ConvType type)
{
	gchar **symbols = NULL;
	gint *expr_types = NULL;
	guchar *raw_data = incoming;
	gint total_symbols = 0;
	gint i = 0;
	gint page = 0;
	DataSize size = MTX_U08;
	gint offset = 0;
	guint bitmask = 0;
	guint bitshift = 0;
	gint canID = 0;
	void * evaluator = NULL;
	gchar **names = NULL;
	gdouble * values = NULL;
	gchar * tmpbuf = NULL;
	gdouble lower_limit = 0;
	gdouble upper_limit = 0;
	gdouble result = 0.0;


	symbols = (gchar **)OBJ_GET(object,"expr_symbols");
	expr_types = (gint *)OBJ_GET(object,"expr_types");
	total_symbols = (GINT)OBJ_GET(object,"total_symbols");
	if (OBJ_GET(object,"real_lower"))
		lower_limit = strtod(OBJ_GET(object,"real_lower"),NULL);
	else
		lower_limit = -G_MAXDOUBLE;
	if (OBJ_GET(object,"real_upper"))
		upper_limit = strtod(OBJ_GET(object,"real_upper"),NULL);
	else
		upper_limit = G_MAXDOUBLE;

	names = g_new0(gchar *, total_symbols);
	values = g_new0(gdouble, total_symbols);

	for (i=0;i<total_symbols;i++)
	{
		page = 0;
		offset = 0;
		bitmask = 0;
		bitshift = 0;
		switch ((ComplexExprType)expr_types[i])
		{
			case VE_EMB_BIT:
				size = MTX_U08;

				tmpbuf = g_strdup_printf("%s_page",symbols[i]);
				page = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_canID",symbols[i]);
				canID = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_bitmask",symbols[i]);
				bitmask = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				bitshift = get_bitshift(bitmask);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)(((get_ecu_data(canID,page,offset,size)) & bitmask) >> bitshift);
				/*
				   printf("raw ecu at page %i, offset %i is %i\n",page,offset,get_ecu_data(canID,page,offset,size));
				   printf("value masked by %i, shifted by %i is %i\n",bitmask,bitshift,(get_ecu_data(canID,page,offset,size) & bitmask) >> bitshift);
				 */
				dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\t Embedded bit, name: %s, value %f\n",names[i],values[i]));
				break;
			case VE_VAR:
				tmpbuf = g_strdup_printf("%s_page",symbols[i]);
				page = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_canID",symbols[i]);
				canID = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_size",symbols[i]);
				size = (DataSize) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)get_ecu_data(canID,page,offset,size);
				dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\t VE Variable, name: %s, value %f\n",names[i],values[i]));
				break;
			case RAW_VAR:
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_size",symbols[i]);
				size = (DataSize) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)_get_sized_data(raw_data,0,offset,size,firmware->bigendian);
				dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\t RAW Variable, name: %s, value %f\n",names[i],values[i]));
				break;
			case RAW_EMB_BIT:
				size = MTX_U08;
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_bitmask",symbols[i]);
				bitmask = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				bitshift = get_bitshift(bitmask);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)(((_get_sized_data(raw_data,0,offset,size,firmware->bigendian)) & bitmask) >> bitshift);
				dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\t RAW Embedded Bit, name: %s, value %f\n",names[i],values[i]));
				break;
			default:
				dbg_func(COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": handle_complex_expr()\n\t UNDEFINE Variable, this will cause a crash!!!!\n"));
				break;
		}

	}
	if (type == UPLOAD)
	{
		evaluator = (void *)OBJ_GET(object,"ul_evaluator");
		if (!evaluator)
		{
			evaluator = evaluator_create(OBJ_GET(object,"ul_conv_expr"));
			OBJ_SET_FULL(object,"ul_evaluator",evaluator,evaluator_destroy);

		}
	}
	else if (type == DOWNLOAD)
	{
		evaluator = (void *)OBJ_GET(object,"dl_evaluator");
		if (!evaluator)
		{
			evaluator = evaluator_create(OBJ_GET(object,"dl_conv_expr"));
			OBJ_SET_FULL(object,"dl_evaluator",evaluator,evaluator_destroy);
		}
	}
	else
	{
		dbg_func(COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": handle_complex_expr()\n\tevaluator type undefined for %s\n",(gchar *)glade_get_widget_name(GTK_WIDGET(object))));
	}
	if (!evaluator)
	{
		dbg_func(COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": handle_complex_expr()\n\tevaluator missing for %s\n",(gchar *)glade_get_widget_name(GTK_WIDGET(object))));
		exit (-1);
	}

	assert(evaluator);

	result = evaluator_evaluate(evaluator,total_symbols,names,values);
	if (result < lower_limit)
		result = lower_limit;
	if (result > upper_limit)
		result = upper_limit;
	for (i=0;i<total_symbols;i++)
	{
		dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\tkey %s value %f\n",names[i],values[i]));
		g_free(names[i]);
	}
	dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_complex_expr()\n\texpression is %s\n",evaluator_get_string(evaluator)));
	g_free(names);
	g_free(values);
	return result;
}

/*!
 \brief handle_multi_expression() is used to handle RT Vars that take
 multiple possible conversions based on ECU state
 \param object (gconstpointer *) object representing this derived variable
 \param handler_name (gchar *) string name of special handler case to be done
 */
G_MODULE_EXPORT gfloat handle_multi_expression(gconstpointer *object,guchar* raw_realtime,GHashTable *hash)
{
	MultiExpr *multi = NULL;
	gint offset = 0;
	gfloat result = 0.0;
	gfloat x = 0.0;
	gchar *key = NULL;
	gchar *hash_key = NULL;
	extern GHashTable *sources_hash;

	if (!(object))
	{
		dbg_func(COMPLEX_EXPR,g_strdup_printf("__FILE__ ERROR: multi_expression object is NULL!\n"));
		return 0.0;
	}
	key = (gchar *)DATA_GET(object,"source_key");
	if (!key)
	{
		dbg_func(COMPLEX_EXPR,g_strdup_printf("__FILE__ ERROR: multi_expression source key is NULL!\n"));
		return 0.0;
	}
	hash_key  = (gchar *)g_hash_table_lookup(sources_hash,key);
	if (!hash_key)
	{
		dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": ERROR: multi_expression hash key is NULL!\n"));
		printf(_(": ERROR: multi_expression hash key is NULL!\n"));
		return 0.0;
	}
	multi = (MultiExpr *)g_hash_table_lookup(hash,hash_key);
	if (!multi)
	{
		dbg_func(COMPLEX_EXPR,g_strdup_printf(__FILE__": handle_multi_expression()\n\t data struct NOT found for key \"%s\"\n",hash_key));
		return 0.0;
	}

	offset = (GINT)DATA_GET(object,"offset");
	if (multi->lookuptable)
		x = direct_lookup_data(multi->lookuptable,raw_realtime[offset]);
	else
		x = (float)raw_realtime[offset];

	 result = evaluator_evaluate_x(multi->ul_eval,x);

	 return result;
}




/*!
 \brief handle_special() is used to handle special derived variables that
 DO NOT use any data fromthe realtime variables.  In this case it's only to
 create the high resoluation clock variable.
 \param object (gconstpointer *) object representing this derived variable
 \param handler_name (gchar *) string name of special handler case to be done
 */
G_MODULE_EXPORT gfloat handle_special(gconstpointer *object,gchar *handler_name)
{
	static GTimeVal now;
	static GTimeVal last;
	static gfloat cumu = 0.0;

	if (g_strcasecmp(handler_name,"hr_clock")==0)
	{
		if (DATA_GET(global_data,"begin"))
		{       
			g_get_current_time(&now);
			last.tv_sec = now.tv_sec;
			last.tv_usec = now.tv_usec;
			DATA_SET(global_data,"begin",GINT_TO_POINTER(FALSE));
			return 0.0;
		}
		else
		{
			g_get_current_time(&now);
			cumu += (now.tv_sec-last.tv_sec)+
				((double)(now.tv_usec-last.tv_usec)/1000000.0);
			last.tv_sec = now.tv_sec;
			last.tv_usec = now.tv_usec;
			return cumu;
		}

	}
	else
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": handle_special()\n\t handler name is not recognized, \"%s\"\n",handler_name));
	}
	return 0.0;
}


/*!
 \brief lookup_current_value() gets the current value of the derived
 variable requested by name.
 \param internal_name (gchar *) name of the variable to get the data for.
 \param value (gflaot *) where to put the value
 \returns TRUE on successful lookup, FALSE on failure
 */
G_MODULE_EXPORT gboolean lookup_current_value(const gchar *internal_name, gfloat *value)
{
	extern Rtv_Map *rtv_map;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	
	*value = 0.0;
	if (!internal_name)
		return FALSE;

	object = g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
		return FALSE;

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
		return FALSE;

	if ((gint)history->len-1 <= 0)
		return TRUE;

	g_static_mutex_lock(&rtv_mutex);
	*value = g_array_index(history,gfloat,history->len-1);
	g_static_mutex_unlock(&rtv_mutex);
	return TRUE;
}


/*!
 \brief lookup_previous_value() gets the current value of the derived
 variable requested by name.
 \param internal_name (gchar *) name of the variable to get the data for.
 \param value (gflaot *) where to put the value
 \returns TRUE on successful lookup, FALSE on failure
 */
G_MODULE_EXPORT gboolean lookup_previous_value(const gchar *internal_name, gfloat *value)
{
	extern Rtv_Map *rtv_map;
	gconstpointer * object = NULL;
	GArray * history = NULL;

	*value = 0.0;
	if (!internal_name)
		return FALSE;

	object = g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
		return FALSE;

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
		return FALSE;

	if ((gint)history->len-2 <= 0)
		return TRUE;

	g_static_mutex_lock(&rtv_mutex);
	*value = g_array_index(history,gfloat,history->len-2);
	g_static_mutex_unlock(&rtv_mutex);
	return TRUE;
}


/*!
 \brief lookup_previous_nth_value() gets the nth previosu value of the derived
 variable requested by name. i.e. if n = 0 it gets current,  n=5 means
 5 samples "back in time"
 \param internal_name (gchar *) name of the variable to get the data for.
 \param value (gflaot *) where to put the value
 \returns TRUE on successful lookup, FALSE on failure
 */
G_MODULE_EXPORT gboolean lookup_previous_nth_value(const gchar *internal_name, gint n, gfloat *value)
{
	extern Rtv_Map *rtv_map;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	gint index = 0;

	*value = 0.0;
	if (!internal_name)
		return FALSE;

	object = g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
		return FALSE;

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
		return FALSE;

	if ((gint)history->len-n <= 0)
		return TRUE;

	g_static_mutex_lock(&rtv_mutex);
	if (index > n)
		index -= n;  /* get PREVIOUS nth one */
	*value = g_array_index(history,gfloat,index);
	g_static_mutex_unlock(&rtv_mutex);
	history = (GArray *)DATA_GET(object,"history");
	return TRUE;
}


/*!
 \brief lookup_previous_n_values() gets the n previous values of the derived
 variable requested by name. i.e. if n = 0 it gets current,  n=5 means
 5 samples "back in time"
 \param internal_name (gchar *) name of the variable to get the data for.
 \param value (gflaot *) where to put the value
 \returns TRUE on successful lookup, FALSE on failure
 */
G_MODULE_EXPORT gboolean lookup_previous_n_values(const gchar *internal_name, gint n, gfloat *values)
{
	extern Rtv_Map *rtv_map;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	gint index = 0;
	gint i = 0;

	/* Set default in case of failure */
	for (i=0;i<n;i++)
		values[i] = 0.0;
	if (!internal_name)
		return FALSE;

	object = g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
		return FALSE;

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
		return FALSE;

	if ((gint)history->len-n <= 0)
		return TRUE;

	g_static_mutex_lock(&rtv_mutex);
	index = history->len-1;
	if (index > n)
	{
		for (i=0;i<n;i++)
		{
			index--;  /* get PREVIOUS nth one */
			values[i] = g_array_index(history,gfloat,index);
		}
	}
	g_static_mutex_unlock(&rtv_mutex);
	return TRUE;
}


/*!
 \brief lookup_previous_n_x_m_values() gets previous data
 \param internal_name (gchar *) name of the variable to get the data for.
 \param n (gint) number of campels we want
 \param skip (gint) number to SKIP between samples
 \param value (gfloat *) where to put the value
 \returns TRUE on successful lookup, FALSE on failure
 */
G_MODULE_EXPORT gboolean lookup_previous_n_skip_x_values(const gchar *internal_name, gint n, gint skip, gfloat *values)
{
	extern Rtv_Map *rtv_map;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	gint index = 0;
	gint i = 0;

	for (i=0;i<n;i++)
		values[i] = 0.0;
	if (!internal_name)
		return FALSE;

	object = g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
		return FALSE;

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
		return FALSE;

	if ((gint)history->len-(n*skip) <= 0)
		return TRUE;

	g_static_mutex_lock(&rtv_mutex);
	index = history->len-1;
	if (index > (n*skip))
	{
		for (i=0;i<n;i++)
		{
			index-=skip;  /* get PREVIOUS nth one */
			values[i] = g_array_index(history,gfloat,index);
		}
	}
	g_static_mutex_unlock(&rtv_mutex);
	return TRUE;
}


/*!
 \brief lookup_precision() gets the current precision of the derived
 variable requested by name.
 \param internal_name (gchar *) name of the variable to get the data for.
 \param value (gint *) where to put the value
 \returns TRUE on successful lookup, FALSE on failure
 */
G_MODULE_EXPORT gboolean lookup_precision(const gchar *internal_name, gint *precision)
{
	extern Rtv_Map *rtv_map;
	gconstpointer * object = NULL;

	if (!internal_name)
	{
		*precision = 0;
		return FALSE;
	}
	object = g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
	{
		*precision = 0;
		return FALSE;
	}
	*precision = (GINT)DATA_GET(object,"precision");
	return TRUE;
}


/*!
 \brief flush_rt_arrays() flushed the history buffers for all the realtime
 variables
 */
G_MODULE_EXPORT void flush_rt_arrays(void)
{
	extern Rtv_Map *rtv_map;
	GArray *history = NULL;
	guint i = 0;
	guint j = 0;
	gconstpointer * object = NULL;
	GList *list = NULL;

	/* Flush and recreate the timestamp array */
	g_array_free(rtv_map->ts_array,TRUE);
	rtv_map->ts_array = g_array_sized_new(FALSE,TRUE,sizeof(GTimeVal),4096);

	for (i=0;i<rtv_map->rtvars_size;i++)
	{
		/* Get list of derived vars for raw offset "i" */
		list = g_hash_table_lookup(rtv_map->offset_hash,GINT_TO_POINTER(i));
		if (list == NULL) /* no derived vars for this variable */
			continue;
		list = g_list_first(list);
		for (j=0;j<g_list_length(list);j++)
		{
			object=(gconstpointer *)g_list_nth_data(list,j);
			if (!(object))
				continue;
			g_static_mutex_lock(&rtv_mutex);
			history = (GArray *)DATA_GET(object,"history");
			/* TRuncate array,  but don't free/recreate as it
			 * makes the logviewer explode!
			 */
			g_array_free(history,TRUE);
			history = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
			DATA_SET(object,"history",(gpointer)history);
			g_static_mutex_unlock(&rtv_mutex);
	                /* bind history array to object for future retrieval */
		}

	}
	update_logbar("dlog_view","warning",_("Realtime Variables History buffers flushed...\n"),FALSE,FALSE,FALSE);

}
