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
  \file src/rtv_processor.c
  \ingroup CoreMtx
  \brief The routines for processing Runtime/Realtime variables from the ECU
  \author David Andruczyk
  */

#include <assert.h>
#include <conversions.h>
#include <debugging.h>
#include <firmware.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <lookuptables.h>
#include <math.h>
#include <mem_mgmt.h>
#include <mtxmatheval.h>
#include <multi_expr_loader.h>
#include <notifications.h>
#include <plugin.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>

extern gconstpointer *global_data;

/*!
  \brief process_rt_vars() processes incoming realtime variables. It's a pretty
  complex function so read the sourcecode.. ;)
  \param incoming is the pointer to the raw incoming data
  \param len how many bytes in the raw incoming block
  */
G_MODULE_EXPORT void process_rt_vars(void *incoming, gint len)
{
	static GMutex *rtv_mutex = NULL;
	static Rtv_Map *rtv_map = NULL;
	static Firmware_Details *firmware = NULL;
	gint mtx_temp_units;
	guchar *raw_realtime = (guchar *)incoming;
	gconstpointer * object = NULL;
	GList * list= NULL;
	guint i = 0;
	guint j = 0;
	gfloat x = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gfloat result = 0.0;
	gfloat tmpf = 0.0;
	gfloat *adder = NULL;
	gfloat *multiplier = NULL;
	gboolean temp_dep = FALSE;
	GTimeVal timeval;
	GArray *history = NULL;
	gchar *special = NULL;
	GHashTable *hash = NULL;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!rtv_map)
		rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");

	g_return_if_fail(rtv_mutex);
	g_return_if_fail(rtv_map);
	g_return_if_fail(firmware);
	g_return_if_fail(incoming);
	/* Store timestamps in ringbuffer */

	/* Backup current rtv copy */
	memcpy(firmware->rt_data_last,firmware->rt_data,len);
	/* Needed for Socket Mode */
	memcpy(firmware->rt_data,incoming,len);
	mtx_temp_units = (GINT)DATA_GET(global_data,"mtx_temp_units");
	g_get_current_time(&timeval);
	g_array_append_val(rtv_map->ts_array,timeval);
	if (rtv_map->ts_array->len%250 == 0)
	{
		GTimeVal curr;
		GTimeVal begin;
		gint hours = 0;
		gint minutes = 0;
		gint seconds = 0;
		curr = g_array_index(rtv_map->ts_array,GTimeVal, rtv_map->ts_array->len-1);
		begin = g_array_index(rtv_map->ts_array,GTimeVal,0);
		tmpf = curr.tv_sec-begin.tv_sec-((curr.tv_usec-begin.tv_usec)/1000000);
		hours = tmpf > 3600.0 ? floor(tmpf/3600.0) : 0;
		tmpf -= hours*3600.0;
		minutes = tmpf > 60.0 ? floor(tmpf/60.0) : 0;
		tmpf -= minutes*60.0;
		seconds = (GINT)tmpf;

		thread_update_logbar("dlog_view",NULL,g_strdup_printf(_("Currently %i samples stored, Total Logged Time (HH:MM:SS) (%02i:%02i:%02i)\n"),rtv_map->ts_array->len,hours,minutes,seconds),FALSE,FALSE);
	}

	for (i=0;i<rtv_map->rtvars_size;i++)
	{
		/* Get list of derived vars for raw offset "i" */
		list = (GList *)g_hash_table_lookup(rtv_map->offset_hash,GINT_TO_POINTER(i));
		if (list == NULL) /* no derived vars for this variable */
		{
			continue;
		}
		list = g_list_first(list);
		for (j=0;j<g_list_length(list);j++)
		{
			history = NULL;
			special = NULL;
			hash = NULL;
			temp_dep = FALSE;
			object=(gconstpointer *)g_list_nth_data(list,j);
			/*
			printf("Dumping datalist for objects\n");
			g_dataset_foreach(object,dump_datalist,NULL);
			*/
			 
			if (!object)
			{
				MTXDBG(COMPLEX_EXPR|CRITICAL,_("Object bound to list at offset %i is invalid!!!!\n"),i);
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
			offset = (GINT)DATA_GET(object,"offset");
			size = (DataSize)(GINT)DATA_GET(object,"size");
			if (DATA_GET(object,"fromecu_complex"))
			{
				tmpf = handle_complex_expr(object,incoming,UPLOAD);
				goto store_it;
			}

			if (DATA_GET(object,"lookuptable"))
			{
				MTXDBG(COMPLEX_EXPR,_("Getting Lookuptable for var using offset %i\n"),offset);
				x = lookup_data(object,raw_realtime[offset]);
			}
			else
			{
				MTXDBG(COMPLEX_EXPR,_("No Lookuptable needed for var using offset %i\n"),offset);
				x = _get_sized_data((guint8 *)incoming,offset,size,firmware->bigendian);
			}

			/* MS Simple math without the complex math... */
			multiplier = NULL;
			adder = NULL;
			multiplier = (gfloat *)DATA_GET(object,"fromecu_mult");
			adder = (gfloat *)DATA_GET(object,"fromecu_add");
			if ((multiplier) && (adder))
				tmpf = (x + (*adder)) * (*multiplier);
			else if (multiplier)
				tmpf = x * (*multiplier);
			else
				tmpf = x;

store_it:
			if (temp_dep)
			{
				MTXDBG(COMPLEX_EXPR,_("Var at offset %i is temp dependant.\n"),offset);
				result = temp_to_host(tmpf);
			}
			else
				result = tmpf;
			/* Get history array and current index point */
			history = (GArray *)DATA_GET(object,"history");
			/* Store data in history buffer */
			g_mutex_lock(rtv_mutex);
			g_array_append_val(history,result);
			/*printf("array size %i, current index %i, appended %f, readback %f previous %f\n",history->len,history->len-1,result,g_array_index(history, gfloat, history->len-1),g_array_index(history, gfloat, history->len-2));*/
			g_mutex_unlock(rtv_mutex);

			/*printf("Result of %s is %f\n",(gchar *)DATA_GET(object,"internal_names"),result);*/

		}
	}
	EXIT();
	return;
}


/*!
  \brief handle_complex_expr() handles a complex mathematcial expression for
  an variable represented by a gconstpointer.
  \param object is the pointer to the object containing the conversion 
  expression and other relevant data
  \param incoming is the pointer to the raw data
  \param type is the enumeration stating if this is an upload or
  download conversion
  \returns a float of the result of the mathematical expression
  */
G_MODULE_EXPORT gfloat handle_complex_expr(gconstpointer *object, void * incoming,ConvType type)
{
	static gdouble (*common_rtv_processor)(gconstpointer *, gchar *, ComplexExprType);
	static Firmware_Details *firmware = NULL;
	gchar **symbols = NULL;
	gint *expr_types = NULL;
	guchar *raw_data = (guchar *)incoming;
	gchar * expr = NULL;
	gint total_symbols = 0;
	gint i = 0;
	DataSize size = MTX_U08;
	gint offset = 0;
	guint bitmask = 0;
	guint bitshift = 0;
	void * evaluator = NULL;
	gchar **names = NULL;
	gdouble * values = NULL;
	gchar * tmpbuf = NULL;
	const gchar *name = NULL;
	gdouble lower_limit = 0;
	gdouble upper_limit = 0;
	gdouble result = 0.0;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!common_rtv_processor)
		get_symbol("common_rtv_processor",(void **)&common_rtv_processor);
	g_return_val_if_fail(firmware,0.0);
	g_return_val_if_fail(common_rtv_processor,0.0);

	symbols = (gchar **)DATA_GET(object,"expr_symbols");
	expr_types = (gint *)DATA_GET(object,"expr_types");
	total_symbols = (GINT)DATA_GET(object,"total_symbols");
	if (DATA_GET(object,"real_lower"))
		lower_limit = strtod((gchar *)DATA_GET(object,"real_lower"),NULL);
	else
		lower_limit = -G_MAXDOUBLE;
	if (DATA_GET(object,"real_upper"))
		upper_limit = strtod((gchar *)DATA_GET(object,"real_upper"),NULL);
	else
		upper_limit = G_MAXDOUBLE;

	names = g_new0(gchar *, total_symbols);
	values = g_new0(gdouble, total_symbols);
	name = (gchar *)DATA_GET(object,"name");

	for (i=0;i<total_symbols;i++)
	{
		offset = 0;
		bitmask = 0;
		bitshift = 0;
		switch ((ComplexExprType)expr_types[i])
		{
			case ECU_EMB_BIT:
				size = MTX_U08;
				names[i] = g_strdup(symbols[i]);
				values[i] = common_rtv_processor(object,symbols[i],ECU_EMB_BIT);
				MTXDBG(COMPLEX_EXPR,_("Embedded bit, name: %s, value %f\n"),names[i],values[i]);
				break;
			case ECU_VAR:
				names[i] = g_strdup(symbols[i]);
				values[i] = (gdouble)common_rtv_processor(object,symbols[i], ECU_VAR);
				MTXDBG(COMPLEX_EXPR,_("VE Variable, name: %s, value %f\n"),names[i],values[i]);
				break;
			case RAW_VAR:
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_size",symbols[i]);
				size = (DataSize)(GINT) DATA_GET(object,tmpbuf);
				g_free(tmpbuf);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)_get_sized_data(raw_data,offset,size,firmware->bigendian);
				MTXDBG(COMPLEX_EXPR,_("RAW Variable, name: %s, value %f\n"),names[i],values[i]);
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
				values[i]=(gdouble)(((_get_sized_data(raw_data,offset,size,firmware->bigendian)) & bitmask) >> bitshift);
				MTXDBG(COMPLEX_EXPR,_("RAW Embedded Bit, name: %s, value %f\n"),names[i],values[i]);
				break;
			default:
				MTXDBG(COMPLEX_EXPR|CRITICAL,_("UNDEFINED Variable, this will cause a crash!!!!\n"));
				break;
		}

	}
	if (type == UPLOAD)
	{
		evaluator = (void *)DATA_GET(object,"ul_evaluator");
		if (!evaluator)
		{
			expr = (gchar *)DATA_GET(object,"fromecu_conv_expr");
			evaluator = evaluator_create(expr);
			if (!evaluator)
				MTXDBG(COMPLEX_EXPR|CRITICAL,_("Unable to create evaluator for expression \"%s\", expect a crash\n"),expr);

			DATA_SET_FULL(object,"ul_evaluator",evaluator,evaluator_destroy);
		}
	}
	else if (type == DOWNLOAD)
	{
		evaluator = (void *)DATA_GET(object,"dl_evaluator");
		if (!evaluator)
		{
			expr = (gchar *)DATA_GET(object,"toecu_conv_expr");
			evaluator = evaluator_create(expr);
			if (!evaluator)
				MTXDBG(COMPLEX_EXPR|CRITICAL,_("Unable to create evaluator for expression \"%s\", expect a crash\n"),expr);
			DATA_SET_FULL(object,"dl_evaluator",evaluator,evaluator_destroy);
		}
	}
	else
	{
		MTXDBG(COMPLEX_EXPR|CRITICAL,_("Evaluator type undefined for %s\n"),(name == NULL ? "undefined":name));
	}
	if (!evaluator)
	{
		MTXDBG(COMPLEX_EXPR|CRITICAL,_("No Evaluator for %s, sym %s\n"),(name == NULL ? "undefined":name),symbols[0]);
		exit (-1);
	}

	assert(evaluator);

	result = evaluator_evaluate(evaluator,total_symbols,names,values);
	if (result < lower_limit)
		result = lower_limit;
	if (result > upper_limit)
		result = upper_limit;

	MTXDBG(COMPLEX_EXPR,_("Total symbols is %i\n"),total_symbols);
	for (i=0;i<total_symbols;i++)
	{
		MTXDBG(COMPLEX_EXPR,_("Key %s value %f\n"),names[i],values[i]);
		g_free(names[i]);
	}
	MTXDBG(COMPLEX_EXPR,_("Expression is %s\n"),evaluator_get_string(evaluator));
	g_free(names);
	g_free(values);
	EXIT();
	return result;
}



/*!
  \brief handle_complex_expr_obj() handles a complex mathematcial expression for
  an variable represented by a gconstpointer.
  \param object is the pointer to the object containing the conversion 
  expression and other relevant data
  \param incoming is the pointer to the raw data
  \param type is the enumeration stating if this is an upload or
  download conversion
  \returns a float of the result of the mathematical expression
  */
G_MODULE_EXPORT gfloat handle_complex_expr_obj(GObject *object, void * incoming,ConvType type)
{
	static gdouble (*common_rtv_processor_obj)(GObject *, gchar *, ComplexExprType);
	static Firmware_Details *firmware = NULL;
	gchar **symbols = NULL;
	gint *expr_types = NULL;
	guchar *raw_data = (guchar *)incoming;
	gint total_symbols = 0;
	gint i = 0;
	DataSize size = MTX_U08;
	gint offset = 0;
	guint bitmask = 0;
	guint bitshift = 0;
	void * evaluator = NULL;
	gchar **names = NULL;
	gdouble * values = NULL;
	gchar * tmpbuf = NULL;
	const gchar * name = NULL;
	gdouble lower_limit = 0;
	gdouble upper_limit = 0;
	gdouble result = 0.0;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!common_rtv_processor_obj)
		get_symbol("common_rtv_processor_obj",(void **)&common_rtv_processor_obj);
	g_return_val_if_fail(firmware,0.0);
	g_return_val_if_fail(common_rtv_processor_obj,0.0);

	symbols = (gchar **)OBJ_GET(object,"expr_symbols");
	expr_types = (gint *)OBJ_GET(object,"expr_types");
	total_symbols = (GINT)OBJ_GET(object,"total_symbols");
	if (OBJ_GET(object,"real_lower"))
		lower_limit = strtod((gchar *)OBJ_GET(object,"real_lower"),NULL);
	else
		lower_limit = -G_MAXDOUBLE;
	if (OBJ_GET(object,"real_upper"))
		upper_limit = strtod((gchar *)OBJ_GET(object,"real_upper"),NULL);
	else
		upper_limit = G_MAXDOUBLE;

	names = g_new0(gchar *, total_symbols);
	values = g_new0(gdouble, total_symbols);
	name = glade_get_widget_name(GTK_WIDGET(object));

	for (i=0;i<total_symbols;i++)
	{
		offset = 0;
		bitmask = 0;
		bitshift = 0;
		switch ((ComplexExprType)expr_types[i])
		{
			case ECU_EMB_BIT:
				size = MTX_U08;
				names[i] = g_strdup(symbols[i]);
				values[i] = common_rtv_processor_obj(object,symbols[i],ECU_EMB_BIT);
				MTXDBG(COMPLEX_EXPR,_("ECU Embedded bit, name: %s, value %f\n"),names[i],values[i]);
				break;
			case ECU_VAR:
				names[i] = g_strdup(symbols[i]);
				values[i] = (gdouble)common_rtv_processor_obj(object,symbols[i], ECU_VAR);
				MTXDBG(COMPLEX_EXPR,_("ECU Variable, name: %s, value %f\n"),names[i],values[i]);
				break;
			case RAW_VAR:
				tmpbuf = g_strdup_printf("%s_offset",symbols[i]);
				offset = (GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("%s_size",symbols[i]);
				size = (DataSize)(GINT) OBJ_GET(object,tmpbuf);
				g_free(tmpbuf);
				names[i]=g_strdup(symbols[i]);
				values[i]=(gdouble)_get_sized_data(raw_data,offset,size,firmware->bigendian);
				MTXDBG(COMPLEX_EXPR,_("RAW Variable, name: %s, value %f\n"),names[i],values[i]);
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
				values[i]=(gdouble)(((_get_sized_data(raw_data,offset,size,firmware->bigendian)) & bitmask) >> bitshift);
				MTXDBG(COMPLEX_EXPR,_("RAW Embedded Bit, name: %s, value %f\n"),names[i],values[i]);
				break;
			default:
				MTXDBG(COMPLEX_EXPR|CRITICAL,_("UNDEFINED Variable, this will cause a crash!!!!\n"));
				break;
		}

	}
	if (type == UPLOAD)
	{
		evaluator = (void *)OBJ_GET(object,"ul_evaluator");
		if (!evaluator)
		{
			evaluator = evaluator_create((gchar *)OBJ_GET(object,"fromecu_conv_expr"));
			if (!evaluator)
				MTXDBG(COMPLEX_EXPR|CRITICAL,_("Unable to create evaluator for expression \"%s\", expect a crash\n"),(gchar *)OBJ_GET(object,"fromecu_conv_expr"));
			OBJ_SET_FULL(object,"ul_evaluator",evaluator,evaluator_destroy);
		}
	}
	else if (type == DOWNLOAD)
	{
		evaluator = (void *)OBJ_GET(object,"dl_evaluator");
		if (!evaluator)
		{
			evaluator = evaluator_create((gchar *)OBJ_GET(object,"toecu_conv_expr"));
			if (!evaluator)
				MTXDBG(COMPLEX_EXPR|CRITICAL,_("Unable to create evaluator for expression \"%s\", expect a crash\n"),(gchar *)OBJ_GET(object,"toecu_conv_expr"));
			OBJ_SET_FULL(object,"dl_evaluator",evaluator,evaluator_destroy);
		}
	}
	else
	{
		MTXDBG(COMPLEX_EXPR|CRITICAL,_("Evaluator type undefined for %s\n"),(name == NULL ? "undefined":name));
	}
	if (!evaluator)
	{
		MTXDBG(COMPLEX_EXPR|CRITICAL,_("Evaluator missing for %s\n"),(name == NULL ? "undefined":name));
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
		MTXDBG(COMPLEX_EXPR,_("Key %s value %f\n"),names[i],values[i]);
		g_free(names[i]);
	}
	MTXDBG(COMPLEX_EXPR,_("Expression is %s\n"),evaluator_get_string(evaluator));
	g_free(names);
	g_free(values);
	EXIT();
	return result;
}

/*!
  \brief handle_multi_expression() is used to handle RT Vars that take
  multiple possible conversions based on ECU state
  \param object is the object representing this derived variable
  \param raw_realtime is the pointer to the the raw realtime vars array
  \param hash is the pointer to hashtable of MultiExpression structures
  \returns the result of the multi_expression calc
  */
G_MODULE_EXPORT gfloat handle_multi_expression(gconstpointer *object,guchar* raw_realtime,GHashTable *hash)
{
	MultiExpr *multi = NULL;
	gint offset = 0;
	gfloat result = 0.0;
	gfloat x = 0.0;
	gfloat *multiplier = NULL;
	gfloat *adder = NULL;
	gchar *key = NULL;
	gchar *hash_key = NULL;
	GHashTable *sources_hash = NULL;

	ENTER();
	sources_hash = (GHashTable *)DATA_GET(global_data,"sources_hash");
	if (!(object))
	{
		MTXDBG(COMPLEX_EXPR,_("ERROR: multi_expression object is NULL!\n"));
		EXIT();
		return 0.0;
	}
	key = (gchar *)DATA_GET(object,"source_key");
	if (!key)
	{
		MTXDBG(COMPLEX_EXPR,_("ERROR: multi_expression source key is NULL!\n"));
		EXIT();
		return 0.0;
	}
	hash_key  = (gchar *)g_hash_table_lookup(sources_hash,key);
	if (!hash_key)
	{
		MTXDBG(COMPLEX_EXPR,_("ERROR: multi_expression hash key is NULL!\n"));
		printf(_(": ERROR: multi_expression hash key is NULL!\n"));
		EXIT();
		return 0.0;
	}
	multi = (MultiExpr *)g_hash_table_lookup(hash,hash_key);
	if (!multi)
	{
		MTXDBG(COMPLEX_EXPR,_("multi-expression data structure NOT found for key \"%s\"\n"),hash_key);
		EXIT();
		return 0.0;
	}

	offset = (GINT)DATA_GET(object,"offset");
	if (multi->lookuptable)
		x = direct_lookup_data(multi->lookuptable,raw_realtime[offset]);
	else
		x = (float)raw_realtime[offset];

	multiplier = multi->fromecu_mult;
	adder = multi->fromecu_add;
	if ((multiplier) && (adder))
		result = (x + (*adder)) * (*multiplier);
	else if (multiplier)
		result = (x * (*multiplier));
	else
		result = x;
	EXIT();
	return result;
}




/*!
  \brief handle_special() is used to handle special derived variables that
  DO NOT use any data fromthe realtime variables.  In this case it's only to
  create the high resoluation clock variable.
  \param object is the object representing this derived variable
  \param handler_name is the string name of special handler case to be done
  \returns the result of the special calc
  */
G_MODULE_EXPORT gfloat handle_special(gconstpointer *object,gchar *handler_name)
{
	static GTimer *timer = NULL;

	ENTER();
	if (g_ascii_strcasecmp(handler_name,"hr_clock")==0)
	{
		if (!timer)
			timer = DATA_GET(global_data,"mtx_uptime_timer");
		if (!timer)
		{
			MTXDBG(COMPLEX_EXPR|CRITICAL,_("\"mtx_uptime_timer\" pointer not found, unable to set high res clock for datalog!\n"));
			EXIT();
			return 0.0;
		}
		EXIT();
		return g_timer_elapsed(timer,NULL);
	}
	else
	{
		MTXDBG(CRITICAL,_("Handler name is not recognized, \"%s\"\n"),handler_name);
	}
	EXIT();
	return 0.0;
}


/*!
  \brief lookup_current_index() gets the current index of the derived
  variable requested by name.
  \param internal_name is the name of the variable to get the data for.
  \param value is where to put the value
  \returns TRUE on successful lookup, FALSE on failure
  */
G_MODULE_EXPORT gboolean lookup_current_index(const gchar *internal_name, gint *value)
{
	static GMutex *rtv_mutex = NULL;
	Rtv_Map *rtv_map = NULL;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	ENTER();
	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");
	*value = 0;
	if (!internal_name)
	{
		EXIT();
		return FALSE;
	}

	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
	{
		EXIT();
		return FALSE;
	}

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
	{
		EXIT();
		return FALSE;
	}

	*value = history->len;
	EXIT();
	return TRUE;
}


/*!
  \brief lookup_current_value() gets the current value of the derived
  variable requested by name.
  \param internal_name is the name of the variable to get the data for.
  \param value is where to put the value
  \returns TRUE on successful lookup, FALSE on failure
  */
G_MODULE_EXPORT gboolean lookup_current_value(const gchar *internal_name, gfloat *value)
{
	static GMutex *rtv_mutex = NULL;
	Rtv_Map *rtv_map = NULL;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	ENTER();
	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");
	*value = 0.0;
	if (!internal_name)
	{
		EXIT();
		return FALSE;
	}

	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
	{
		EXIT();
		return FALSE;
	}

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
	{
		EXIT();
		return FALSE;
	}

	if ((GINT)history->len-1 <= 0)
	{
		EXIT();
		return TRUE;
	}

	g_mutex_lock(rtv_mutex);
	*value = g_array_index(history,gfloat,history->len-1);
	g_mutex_unlock(rtv_mutex);
	EXIT();
	return TRUE;
}


/*!
  \brief lookup_previous_value() gets the current value of the derived
  variable requested by name.
  \param internal_name is the name of the variable to get the data for.
  \param value is where to put the value
  \returns TRUE on successful lookup, FALSE on failure
  */
G_MODULE_EXPORT gboolean lookup_previous_value(const gchar *internal_name, gfloat *value)
{
	static GMutex *rtv_mutex = NULL;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	Rtv_Map *rtv_map = NULL;
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	ENTER();
	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");
	*value = 0.0;
	if (!internal_name)
	{
		EXIT();
		return FALSE;
	}

	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
	{
		EXIT();
		return FALSE;
	}

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
	{
		EXIT();
		return FALSE;
	}

	if ((GINT)history->len-2 <= 0)
	{
		EXIT();
		return TRUE;
	}

	g_mutex_lock(rtv_mutex);
	*value = g_array_index(history,gfloat,history->len-2);
	g_mutex_unlock(rtv_mutex);
	EXIT();
	return TRUE;
}


/*!
  \brief lookup_previous_nth_value() gets the nth previosu value of the derived
  variable requested by name. i.e. if n = 0 it gets current,  n=4 means
  5 samples "back in time"
  \param internal_name is the name of the variable to get the data for.
  \param n how far back in time to go in samples from current
  \param value is where to put the value
  \returns TRUE on successful lookup, FALSE on failure
  */
G_MODULE_EXPORT gboolean lookup_previous_nth_value(const gchar *internal_name, gint n, gfloat *value)
{
	static GMutex *rtv_mutex = NULL;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	gint index = 0;
	Rtv_Map *rtv_map = NULL;
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	ENTER();
	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");
	*value = 0.0;
	if (!internal_name)
	{
		EXIT();
		return FALSE;
	}

	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
	{
		EXIT();
		return FALSE;
	}

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
	{
		EXIT();
		return FALSE;
	}

	if ((GINT)history->len-n <= 0)
	{
		EXIT();
		return TRUE;
	}

	g_mutex_lock(rtv_mutex);
	if (index > n)
		index -= n;  /* get PREVIOUS nth one */
	*value = g_array_index(history,gfloat,index);
	g_mutex_unlock(rtv_mutex);
	history = (GArray *)DATA_GET(object,"history");
	EXIT();
	return TRUE;
}


/*!
  \brief lookup_previous_n_values() gets the n previous values of the derived
  variable requested by name. i.e. if n = 1 it gets current,  n=5 means
  5 samples "back in time"
  \param internal_name is name of the variable to get the data for.
  \param n how many samples from now to get
  \param values is where to put the values
  \returns TRUE on successful lookup, FALSE on failure
  */
G_MODULE_EXPORT gboolean lookup_previous_n_values(const gchar *internal_name, gint n, gfloat *values)
{
	static GMutex *rtv_mutex = NULL;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	gint index = 0;
	gint i = 0;
	Rtv_Map *rtv_map = NULL;
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	ENTER();
	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");
	/* Set default in case of failure */
	for (i=0;i<n;i++)
		values[i] = 0.0;
	if (!internal_name)
	{
		EXIT();
		return FALSE;
	}

	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
	{
		EXIT();
		return FALSE;
	}

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
	{
		EXIT();
		return FALSE;
	}

	if ((GINT)history->len-n <= 0)
	{
		EXIT();
		return TRUE;
	}

	g_mutex_lock(rtv_mutex);
	index = history->len-1;
	if (index > n)
	{
		for (i=0;i<n;i++)
		{
			index--;  /* get PREVIOUS nth one */
			values[i] = g_array_index(history,gfloat,index);
		}
	}
	g_mutex_unlock(rtv_mutex);
	EXIT();
	return TRUE;
}


/*!
  \brief lookup_previous_n_skip_xvalues() gets previous data
  \param internal_name is the name of the variable to get the data for.
  \param n is the number of samples we want
  \param skip is the number to SKIP between samples
  \param values is where to put the value
  \returns TRUE on successful lookup, FALSE on failure
  */
G_MODULE_EXPORT gboolean lookup_previous_n_skip_x_values(const gchar *internal_name, gint n, gint skip, gfloat *values)
{
	static GMutex *rtv_mutex = NULL;
	gconstpointer * object = NULL;
	GArray * history = NULL;
	gint index = 0;
	gint i = 0;
	Rtv_Map *rtv_map = NULL;
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	ENTER();
	if (!rtv_mutex)
		rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");
	for (i=0;i<n;i++)
		values[i] = 0.0;
	if (!internal_name)
	{
		EXIT();
		return FALSE;
	}

	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
	{
		EXIT();
		return FALSE;
	}

	history = (GArray *)DATA_GET(object,"history");
	if (!history)
	{
		EXIT();
		return FALSE;
	}

	if ((GINT)history->len-(n*skip) <= 0)
	{
		EXIT();
		return TRUE;
	}

	g_mutex_lock(rtv_mutex);
	index = history->len-1;
	if (index > (n*skip))
	{
		for (i=0;i<n;i++)
		{
			index-=skip;  /* get PREVIOUS nth one */
			values[i] = g_array_index(history,gfloat,index);
		}
	}
	g_mutex_unlock(rtv_mutex);
	EXIT();
	return TRUE;
}


/*!
  \brief lookup_precision() gets the current precision of the derived
  variable requested by name.
  \param internal_name is the name of the variable to get the data for.
  \param precision is  where to put the precision
  \returns TRUE on successful lookup, FALSE on failure
  */
G_MODULE_EXPORT gboolean lookup_precision(const gchar *internal_name, gint *precision)
{
	gconstpointer * object = NULL;
	Rtv_Map *rtv_map = NULL;
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	ENTER();
	if (!internal_name)
	{
		*precision = 0;
		EXIT();
		return FALSE;
	}
	object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,internal_name);
	if (!object)
	{
		*precision = 0;
		EXIT();
		return FALSE;
	}
	*precision = (GINT)DATA_GET(object,"precision");
	EXIT();
	return TRUE;
}


/*!
  \brief flush_rt_arrays() flushed the history buffers for all the realtime
  variables
  */
G_MODULE_EXPORT void flush_rt_arrays(void)
{
	GMutex *rtv_mutex = NULL;
	GArray *history = NULL;
	guint i = 0;
	guint j = 0;
	gconstpointer * object = NULL;
	GList *list = NULL;
	Rtv_Map *rtv_map = NULL;
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	rtv_mutex = (GMutex *)DATA_GET(global_data,"rtv_mutex");

	ENTER();
	/* Flush and recreate the timestamp array */
	g_array_free(rtv_map->ts_array,TRUE);
	rtv_map->ts_array = g_array_sized_new(FALSE,TRUE,sizeof(GTimeVal),4096);

	for (i=0;i<rtv_map->rtvars_size;i++)
	{
		/* Get list of derived vars for raw offset "i" */
		list = (GList *)g_hash_table_lookup(rtv_map->offset_hash,GINT_TO_POINTER(i));
		if (list == NULL) /* no derived vars for this variable */
			continue;
		list = g_list_first(list);
		for (j=0;j<g_list_length(list);j++)
		{
			object=(gconstpointer *)g_list_nth_data(list,j);
			if (!(object))
				continue;
			g_mutex_lock(rtv_mutex);
			history = (GArray *)DATA_GET(object,"history");
			/* Truncate array,  but don't free/recreate as it
			 * makes the logviewer explode!
			 */
			g_array_free(history,TRUE);
			history = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
			DATA_SET(object,"history",(gpointer)history);
			g_mutex_unlock(rtv_mutex);
	                /* bind history array to object for future retrieval */
		}
	}
	thread_update_logbar("dlog_view","warning",g_strdup(_("Realtime Variables History buffers flushed...\n")),FALSE,FALSE);
	EXIT();
	return;
}
