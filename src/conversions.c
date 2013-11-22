/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*! 
  \file src/conversions.c
  \ingroup CoreMtx
  \brief Handlers for value conversion to/From ECU units
  
  These take care of all the fancy matho converting between ECU units and 
  Real world units, taking into account things like the local Temperature scale
  in use, and other factors like interdependant variables.
  \author David Andruczyk
  */

#include <assert.h>
#include <conversions.h>
#include <debugging.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <listmgmt.h>
#include <lookuptables.h>
#include <mtxmatheval.h>
#include <plugin.h>
#include <rtv_processor.h>
#include <stdio.h>
#include <stdlib.h>

extern gconstpointer *global_data;

/*!
  \brief convert_before_download() converts the value passed using the
  conversions bound to the widget
  \param widget is the pointer to the widget to extract the conversion info from
  \param value is the "real world" value from the tuning gui before
  translation to ECU-units
  \returns the integer ECU-units form after conversion
  */
G_MODULE_EXPORT gint convert_before_download(GtkWidget *widget, gfloat value)
{
	static GMutex mutex;
	gint return_value = 0;
	gint tmpi = 0;
	gint dl_type = IMMEDIATE;
	gchar * conv_expr = NULL;
	void *evaluator = NULL;
	DataSize size = MTX_U08;
	float lower = 0.0;
	float upper = 0.0;
	gfloat *multiplier = NULL;
	gfloat *adder = NULL;
	gchar *table = NULL;
	GHashTable *mhash = NULL;
	GHashTable *ahash = NULL;
	GHashTable *lhash = NULL;
	gchar *key_list = NULL;
	gchar *mult_list = NULL;
	gchar *add_list = NULL;
	gchar *table_list = NULL;
	gchar **keys = NULL;
	gchar **mults = NULL;
	gchar **adds = NULL;
	gchar **tables = NULL;
	gchar *tmpbuf = NULL;
	const gchar *name = NULL;
	gchar * source_key = NULL;
	gchar * hash_key = NULL;
	gint *algorithm = NULL;
	GHashTable *sources_hash = NULL;

	ENTER();

	sources_hash = (GHashTable *)DATA_GET(global_data,"sources_hash");

	g_mutex_lock(&mutex);

	name = glade_get_widget_name(widget);

	if (!OBJ_GET(widget,"size"))
		printf(__FILE__"%s %s\n",_(": convert_before_download, FATAL ERROR, size undefined for widget %s "),(name == NULL ? "undefined" : name));

	size = (DataSize)(GINT)OBJ_GET(widget,"size");
	dl_type = (GINT)OBJ_GET(widget,"dl_type");
	if (OBJ_GET(widget,"raw_lower"))
		lower = (gfloat)strtol((gchar *)OBJ_GET(widget,"raw_lower"),NULL,10);
	else
		lower = (gfloat)get_extreme_from_size(size,LOWER);
	if (OBJ_GET(widget,"raw_upper"))
		upper = (gfloat)strtol((gchar *)OBJ_GET(widget,"raw_upper"),NULL,10);
	else
		upper = (gfloat)get_extreme_from_size(size,UPPER);

	/* MULTI EXPRESSION ONLY, i.e. different math conversions that depend on a named source_key! */
	if (OBJ_GET(widget,"multi_expr_keys"))
	{
		gint table_num = 0;
		//if ((!OBJ_GET(widget,"mhash")) && (!OBJ_GET(widget,"ahash")))
		if ((!OBJ_GET(widget,"mhash")) && 
				(!OBJ_GET(widget,"ahash")) &&
				(!OBJ_GET(widget,"lhash")))
		{
			mhash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
			ahash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
			lhash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
			key_list = (gchar *)OBJ_GET(widget,"multi_expr_keys");
			mult_list = (gchar *)OBJ_GET(widget,"fromecu_mults");
			add_list = (gchar *)OBJ_GET(widget,"fromecu_addds");
			table_list = (gchar *)OBJ_GET(widget,"lookuptables");
			if (table_list)
				tables = g_strsplit(table_list,",",-1);
			keys = g_strsplit(key_list,",",-1);
			mults = g_strsplit(mult_list,",",-1);
			adds = g_strsplit(add_list,",",-1);
			for (int i=0;i<MIN(g_strv_length(keys),g_strv_length(mults));i++)
			{
				multiplier = (gfloat *)g_new0(gfloat, 1);
				*multiplier = (gfloat)g_strtod(mults[i],NULL);
				g_hash_table_insert(mhash,g_strdup(keys[i]),multiplier);
				adder = (gfloat *)g_new0(gfloat, 1);
				*adder = (gfloat)g_strtod(adds[i],NULL);
				g_hash_table_insert(ahash,g_strdup(keys[i]),adder);
				if ((table_list) && (tables[i]))
					g_hash_table_insert(lhash,g_strdup(keys[i]),g_strdup(tables[i]));
			}
			g_strfreev(keys);
			g_strfreev(mults);
			g_strfreev(adds);
			if (table_list)
				g_strfreev(tables);

			OBJ_SET_FULL(widget,"mhash",mhash,g_hash_table_destroy);
			OBJ_SET_FULL(widget,"ahash",ahash,g_hash_table_destroy);
			OBJ_SET_FULL(widget,"lhash",lhash,g_hash_table_destroy);
		}
		mhash = (GHashTable *)OBJ_GET(widget,"mhash");
		ahash = (GHashTable *)OBJ_GET(widget,"ahash");
		lhash = (GHashTable *)OBJ_GET(widget,"lhash");
		source_key = (gchar *)OBJ_GET(widget,"source_key");
		if (!source_key)
			printf(_("big problem, source key is undefined!!\n"));
		hash_key = (gchar *)g_hash_table_lookup(sources_hash,source_key);
		tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
		if (tmpbuf)
			table_num = (GINT)strtol(tmpbuf,NULL,10);
		if (table_num == -1) /* Not a table */
		{
			if (!hash_key)
			{
				multiplier = (gfloat *)g_hash_table_lookup(mhash,"DEFAULT");
				adder = (gfloat *)g_hash_table_lookup(ahash,"DEFAULT");
				table = (gchar *)g_hash_table_lookup(lhash,"DEFAULT");
			}
			else
			{
				multiplier = (gfloat *)g_hash_table_lookup(mhash,(gchar *)hash_key);
				adder = (gfloat *)g_hash_table_lookup(ahash,(gchar *)hash_key);
				table = (gchar *)g_hash_table_lookup(lhash,(gchar *)hash_key);
			}
		}
		else /* This is a 3d table */
		{
			if((GINT)OBJ_GET(widget,"ignore_algorithm"))
			{
				if (!hash_key)
				{
					multiplier = (gfloat *)g_hash_table_lookup(mhash,"DEFAULT");
					adder = (gfloat *)g_hash_table_lookup(ahash,"DEFAULT");
					table = (gchar *)g_hash_table_lookup(lhash,"DEFAULT");
				}
				else
				{
					multiplier = (gfloat *)g_hash_table_lookup(mhash,hash_key);
					adder = (gfloat *)g_hash_table_lookup(ahash,hash_key);
					table = (gchar *)g_hash_table_lookup(lhash,hash_key);
				}
			}
			else
			{
				algorithm = (gint *)DATA_GET(global_data,"algorithm");
				switch (algorithm[table_num])
				{
					case SPEED_DENSITY:
						if (!hash_key)
						{
							multiplier = (gfloat *)g_hash_table_lookup(mhash,"DEFAULT");
							adder = (gfloat *)g_hash_table_lookup(ahash,"DEFAULT");
							table = (gchar *)g_hash_table_lookup(lhash,"DEFAULT");
						}
						else
						{
							multiplier = (gfloat *)g_hash_table_lookup(mhash,hash_key);
							adder = (gfloat *)g_hash_table_lookup(ahash,hash_key);
							table = (gchar *)g_hash_table_lookup(lhash,hash_key);
						}
						break;
					case ALPHA_N:
						multiplier = (gfloat *)g_hash_table_lookup(mhash,"DEFAULT");
						adder = (gfloat *)g_hash_table_lookup(ahash,"DEFAULT");
						table = (gchar *)g_hash_table_lookup(lhash,"DEFAULT");
						break;
					case MAF:
						multiplier = (gfloat *)g_hash_table_lookup(mhash,"AFM_VOLTS");
						adder = (gfloat *)g_hash_table_lookup(ahash,"AFM_VOLTS");
						table = (gchar *)g_hash_table_lookup(lhash,"DEFAULT");
						break;
				}
			}
		}
		/* Reverse calc due to this being TO the ecu */
		if ((multiplier) && (adder))
			return_value = (GINT)((value/(*multiplier)) - (*adder));
		else if (multiplier)
			return_value = (GINT)(value/(*multiplier));
		else
			return_value = (GINT)value;

		tmpi = return_value;
		if (table)
			return_value = (GINT)direct_reverse_lookup(table,tmpi);
	}
	else /* NON Multi Expression */
	{
		conv_expr = (gchar *)OBJ_GET(widget,"toecu_conv_expr");

		/* Expression is NOT multi expression but has more complex math*/
		if (conv_expr)
		{
			evaluator = (void *)OBJ_GET(widget,"dl_evaluator");
			if (!evaluator)
			{
				evaluator = evaluator_create(conv_expr);
				assert(evaluator);
				OBJ_SET_FULL(widget,"dl_evaluator",(gpointer)evaluator,evaluator_destroy);
			}
			return_value = (GINT)evaluator_evaluate_x(evaluator,value); 
		}
		else
		{
			multiplier = (gfloat *)OBJ_GET(widget,"fromecu_mult");
			adder = (gfloat *)OBJ_GET(widget,"fromecu_add");
			/* Handle all cases of with or without multiplier/adder*/
			if ((multiplier) && (adder))
				return_value = (GINT)((value/(*multiplier)) - (*adder));
			else if (multiplier)
				return_value = (GINT)(value/(*multiplier));
			else
				return_value = (GINT)value;
		}
		tmpi = return_value;
		if (OBJ_GET(widget,"lookuptable"))
			return_value = (GINT)reverse_lookup_obj(G_OBJECT(widget),tmpi);
	}

	name = glade_get_widget_name(widget);
	MTXDBG(CONVERSIONS,_("Widget %s raw %.2f, sent %i\n"),(name == NULL ? "undefined" : name),value,return_value);
	if ((gfloat)return_value > upper)
	{
		if (dl_type != IGNORED)
			MTXDBG(CONVERSIONS|CRITICAL,_("WARNING value clamped at %f (%f <- %i -> %f)!!\n"),upper,lower,return_value,upper);
		return_value = upper;
	}
	if ((gfloat)return_value < lower)
	{
		if (dl_type != IGNORED)
			MTXDBG(CONVERSIONS|CRITICAL,_("WARNING value clamped at %f (%f <- %i -> %f)!!\n"),lower,lower,return_value,upper);
		return_value = lower;
	}


	g_mutex_unlock(&mutex);
	EXIT();
	return (return_value);
}


/*!
  \brief convert_after_upload() converts the ECU-units data to the real world
  units for display on the GUI
  \param widget is the pointer to the widget to extract the conversion 
  info from to perform the necessary math
  \returns the real world value for the GUI
  */
G_MODULE_EXPORT gfloat convert_after_upload(GtkWidget * widget)
{
	static GMutex mutex;
	static gint (*get_ecu_data_f)(gpointer);
	static void (*send_to_ecu_f)(gpointer, gint, gboolean) = NULL;
	gfloat return_value = 0.0;
	gint dl_type = IMMEDIATE;
	gchar * conv_expr = NULL;
	void *evaluator = NULL;
	gint tmpi = 0;
	DataSize size = MTX_U08;
	gfloat lower = 0.0;
	gfloat upper = 0.0;
	gboolean fromecu_complex = FALSE;
	GHashTable *mhash = NULL;
	GHashTable *ahash = NULL;
	GHashTable *lhash = NULL;
	gchar *key_list = NULL;
	gchar *mult_list = NULL;
	gchar *add_list = NULL;
	gchar *table_list = NULL;
	gchar **keys = NULL;
	gchar **mults = NULL;
	gchar **adds = NULL;
	gchar **tables = NULL;
	gchar * tmpbuf = NULL;
	gchar * source_key = NULL;
	gchar * hash_key = NULL;
	gchar * table = NULL;
	const gchar *name = NULL;
	gfloat *multiplier = NULL;
	gfloat *adder = NULL;
	gint *algorithm = NULL;
	GHashTable *sources_hash = NULL;
	extern gconstpointer *global_data;

	ENTER();

	if (!get_ecu_data_f)
		get_symbol("get_ecu_data",(void **)&get_ecu_data_f);
	if (!send_to_ecu_f)
		get_symbol("send_to_ecu",(void **)&send_to_ecu_f);
	g_return_val_if_fail(get_ecu_data_f,0.0);
	g_return_val_if_fail(send_to_ecu_f,0.0);
	name = glade_get_widget_name(widget);

	g_mutex_lock(&mutex);

	size = (DataSize)(GINT)OBJ_GET(widget,"size");
	dl_type = (GINT)OBJ_GET(widget,"dl_type");
	if (size == 0)
	{
		printf(_("BIG PROBLEM, size undefined! widget %s, default to U08 \n"),(name == NULL ? "undefined" : name));
		size = MTX_U08;
	}

	if (OBJ_GET(widget,"raw_lower"))
		lower = (gfloat)strtol((gchar *)OBJ_GET(widget,"raw_lower"),NULL,10);
	else
		lower = (gfloat)get_extreme_from_size(size,LOWER);
	if (OBJ_GET(widget,"raw_upper"))
		upper = (gfloat)strtol((gchar *)OBJ_GET(widget,"raw_upper"),NULL,10);
	else
		upper = (gfloat)get_extreme_from_size(size,UPPER);

	fromecu_complex = (GBOOLEAN)OBJ_GET(widget,"fromecu_complex");
	if (fromecu_complex)
	{
		g_mutex_unlock(&mutex);
		/*printf("Complex upload conversion for widget at page %i, offset %i, name %s\n",(GINT)OBJ_GET(widget,"page"),(GINT)OBJ_GET(widget,"offset"),(name == NULL ? "undefined" : name));
		  */
		return handle_complex_expr_obj(G_OBJECT(widget),NULL,UPLOAD);
	}

	if (OBJ_GET(widget,"lookuptable"))
		tmpi = lookup_data_obj(G_OBJECT(widget),get_ecu_data_f(widget));
	else
		tmpi = get_ecu_data_f(widget);
	if ((gfloat)tmpi < lower)
	{
		if (dl_type != IGNORED)
			MTXDBG(CONVERSIONS|CRITICAL,_("WARNING RAW value out of range for widget %s, clamped at %.1f (%.1f <- %i -> %.1f), updating ECU with valid value within limits!!\n"),(name == NULL ? "undefined" : name),lower,lower,tmpi,upper);
		tmpi = (gint)lower;
		send_to_ecu_f(widget,tmpi,TRUE);
	}
	if ((gfloat)tmpi > upper)
	{
		if (dl_type != IGNORED)
			MTXDBG(CONVERSIONS|CRITICAL,_("WARNING RAW value out of range for widget %s, clamped at %.1f (%.1f <- %i -> %.1f), updating ECU with valid value within limits!!\n"),(name == NULL ? "undefined" : name),upper,lower,tmpi,upper);
		tmpi = (gint)upper;
		send_to_ecu_f(widget,tmpi,TRUE);
	}
	/* MULTI EXPRESSION ONLY! */
	if (OBJ_GET(widget,"multi_expr_keys"))
	{
		gint table_num = -1;
		sources_hash = (GHashTable *)DATA_GET(global_data,"sources_hash");
		//if ((!OBJ_GET(widget,"mhash")) && (!OBJ_GET(widget,"ahash")))
		if ((!OBJ_GET(widget,"mhash")) && 
				(!OBJ_GET(widget,"ahash")) &&
				(!OBJ_GET(widget,"lhash")))
		{
			mhash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
			ahash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
			lhash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
			key_list = (gchar *)OBJ_GET(widget,"multi_expr_keys");
			mult_list = (gchar *)OBJ_GET(widget,"fromecu_mults");
			add_list = (gchar *)OBJ_GET(widget,"fromecu_adds");
			table_list = (gchar *)OBJ_GET(widget,"lookuptables");
			if (!mult_list)
				MTXDBG(CRITICAL,_("BUG, widget %s is multi_expression but doesn't have fromecu_mults defined!\n"),(name == NULL ? "undefined" : name));
			if (!add_list)
				MTXDBG(CRITICAL,_("BUG, widget %s is multi_expression but doesn't have fromecu_adds defined!\n"),(name == NULL ? "undefined" : name));
			keys = g_strsplit(key_list,",",-1);
			mults = g_strsplit(mult_list,",",-1);
			adds = g_strsplit(add_list,",",-1);
			if (table_list)
				tables = g_strsplit(table_list,",",-1);
			for (int i=0;i<MIN(g_strv_length(keys),g_strv_length(mults));i++)
			{
				multiplier = (gfloat *)g_new0(gfloat, 1);
				*multiplier = (gfloat)g_strtod(mults[i],NULL);
				g_hash_table_insert(mhash,g_strdup(keys[i]),multiplier);
				adder = (gfloat *)g_new0(gfloat, 1);
				*adder = (gfloat)g_strtod(adds[i],NULL);
				g_hash_table_insert(ahash,g_strdup(keys[i]),adder);
				if ((table_list) && (tables[i]))
					g_hash_table_insert(lhash,g_strdup(keys[i]),g_strdup(tables[i]));
			}
			g_strfreev(keys);
			g_strfreev(mults);
			g_strfreev(adds);
			if (table_list)
				g_strfreev(tables);

			OBJ_SET_FULL(widget,"mhash",mhash,g_hash_table_destroy);
			OBJ_SET_FULL(widget,"ahash",ahash,g_hash_table_destroy);
			OBJ_SET_FULL(widget,"lhash",lhash,g_hash_table_destroy);
		}
		mhash = (GHashTable *)OBJ_GET(widget,"mhash");
		ahash = (GHashTable *)OBJ_GET(widget,"ahash");
		lhash = (GHashTable *)OBJ_GET(widget,"lhash");
		source_key = (gchar *)OBJ_GET(widget,"source_key");
		if (!source_key)
			printf(_("big problem, source key is undefined!!\n"));
		hash_key = (gchar *)g_hash_table_lookup(sources_hash,source_key);
		tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
		if (tmpbuf)
			table_num = (GINT)strtol(tmpbuf,NULL,10);
		if (table_num == -1)
		{
			if (!hash_key)
			{
				multiplier = (gfloat *)g_hash_table_lookup(mhash,"DEFAULT");
				adder = (gfloat *)g_hash_table_lookup(ahash,"DEFAULT");
				table = (gchar *)g_hash_table_lookup(lhash,"DEFAULT");
			}
			else
			{
				multiplier = (gfloat *)g_hash_table_lookup(mhash,(gchar *)hash_key);
				adder = (gfloat *)g_hash_table_lookup(ahash,(gchar *)hash_key);
				table = (gchar *)g_hash_table_lookup(lhash,(gchar *)hash_key);
			}
		}
		else /* This is a 3d table */
		{
			if((GINT)OBJ_GET(widget,"ignore_algorithm"))
			{
				if (!hash_key)
				{
					multiplier = (gfloat *)g_hash_table_lookup(mhash,"DEFAULT");
					adder = (gfloat *)g_hash_table_lookup(ahash,"DEFAULT");
					table = (gchar *)g_hash_table_lookup(lhash,"DEFAULT");
				}
				else
				{
					multiplier = (gfloat *)g_hash_table_lookup(mhash,hash_key);
					adder = (gfloat *)g_hash_table_lookup(ahash,hash_key);
					table = (gchar *)g_hash_table_lookup(lhash,hash_key);
				}
			}
			else
			{
				algorithm = (gint *)DATA_GET(global_data,"algorithm");
				switch (algorithm[table_num])
				{
					case SPEED_DENSITY:
						if (!hash_key)
						{
							multiplier = (gfloat *)g_hash_table_lookup(mhash,"DEFAULT");
							adder = (gfloat *)g_hash_table_lookup(ahash,"DEFAULT");
							table = (gchar *)g_hash_table_lookup(lhash,"DEFAULT");
						}
						else
						{
							multiplier = (gfloat *)g_hash_table_lookup(mhash,hash_key);
							adder = (gfloat *)g_hash_table_lookup(ahash,hash_key);
							table = (gchar *)g_hash_table_lookup(lhash,hash_key);
						}
						break;
					case ALPHA_N:
						multiplier = (gfloat *)g_hash_table_lookup(mhash,"DEFAULT");
						adder = (gfloat *)g_hash_table_lookup(ahash,"DEFAULT");
						table = (gchar *)g_hash_table_lookup(lhash,"DEFAULT");
						break;
					case MAF:
						multiplier = (gfloat *)g_hash_table_lookup(mhash,"AFM_VOLTS");
						adder = (gfloat *)g_hash_table_lookup(ahash,"AFM_VOLTS");
						table = (gchar *)g_hash_table_lookup(lhash,"AFM_VOLTS");
						break;
				}
			}
		}
		if (table)
			tmpi = direct_lookup_data(table,tmpi);
		if ((multiplier) && (adder))
			return_value = (((gfloat)tmpi + (*adder)) * (*multiplier));
		else if (multiplier)
			return_value = (gfloat)tmpi * (*multiplier);
		else
			return_value = (gfloat)tmpi;
	}
	else
	{
		conv_expr = (gchar *)OBJ_GET(widget,"fromecu_conv_expr");
		if (conv_expr)
		{
			evaluator = (void *)OBJ_GET(widget,"ul_evaluator");
			if (!evaluator)
			{
				evaluator = evaluator_create(conv_expr);
				assert(evaluator);
				OBJ_SET_FULL(widget,"ul_evaluator",(gpointer)evaluator,evaluator_destroy);
			}
			return_value = evaluator_evaluate_x(evaluator,tmpi);
		}
		else
		{
			multiplier = (gfloat *)OBJ_GET(widget,"fromecu_mult");
			adder = (gfloat *)OBJ_GET(widget,"fromecu_add");
			if ((multiplier) && (adder))
				return_value = (((gfloat)tmpi + (*adder)) * (*multiplier));
			else if (multiplier)
				return_value = (gfloat)tmpi * (*multiplier);
			else
				return_value = (gfloat)tmpi;
		}

	}
	g_mutex_unlock(&mutex);
	EXIT();
	return (return_value);
}


/*!
  \brief convert_temps() changes the values of controls based on the currently
  selected temperature scale.  It works for labels, spinbuttons, etc...
  \param widget is the pointer to the widget that contains the necessary
  paramaters regarding temperature (Alt label, etc)
  \param units is the temp scale selected 
  \see TempUnits
  */
G_MODULE_EXPORT void convert_temps(gpointer widget, gpointer units)
{
	static void (*update_widget_f)(gpointer, gpointer);
	static gboolean (*check_deps)(gconstpointer *) = NULL;
	gconstpointer *dep_obj = NULL;
	gfloat upper = 0.0;
	gfloat lower = 0.0;
	gfloat value = 0.0;
	gchar * text = NULL;
	const gchar  *name = NULL;
	GtkAdjustment * adj = NULL;
	gboolean state = FALSE;
	gint widget_temp = -1;
	/*extern GdkColor black;*/
	extern gconstpointer *global_data;

	ENTER();

	/* If this widgt depends on anything call check_dependancy which will
	 * return TRUE/FALSE.  True if what it depends on is in the matching
	 * state, FALSE otherwise...
	 */
	if ((!widget) || (DATA_GET(global_data,"leaving")))
		return;
	if (!check_deps)
		if (!get_symbol("check_dependencies",(void **)&check_deps))
			MTXDBG(CRITICAL|CONVERSIONS,_("Can NOT locate \"check_dependencies\" function pointer in plugins, BUG!\n"));
	if (!update_widget_f)
		if(!get_symbol("update_widget",(void **)&update_widget_f))
			MTXDBG(CRITICAL|CONVERSIONS,_("Can NOT locate \"update_widget\" function pointer in plugins, BUG!\n"));
	dep_obj = (gconstpointer *)OBJ_GET(widget,"dep_object");
	widget_temp = (GINT)OBJ_GET(widget,"widget_temp");
	name = glade_get_widget_name((GtkWidget *)widget);
	if (dep_obj)
	{
		if (check_deps)
			state = check_deps(dep_obj);
		else
			MTXDBG(CRITICAL|CONVERSIONS,_("Widget %s has dependant object bound but can't locate function ptr for \"check_dependencies\" from plugins, BUG!\n"),(name == NULL ? "undefined" : name));
	}
	switch ((TempUnits)(GINT)units)
	{
		case FAHRENHEIT:
			if (GTK_IS_LABEL(widget))
			{
				if ((dep_obj) && (state))	
					text = (gchar *)OBJ_GET(widget,"alt_f_label");
				else
					text = (gchar *)OBJ_GET(widget,"f_label");
				gtk_label_set_text(GTK_LABEL(widget),text);
				//gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));

			}
			if ((GTK_IS_SPIN_BUTTON(widget)) && (widget_temp != FAHRENHEIT))
			{

				adj = (GtkAdjustment *) gtk_spin_button_get_adjustment(
						GTK_SPIN_BUTTON(widget));
				upper = gtk_adjustment_get_upper(adj);
				value = gtk_adjustment_get_value(adj);
				lower = gtk_adjustment_get_lower(adj);
				if (widget_temp == CELSIUS)
				{
					gtk_adjustment_set_value(adj, c_to_f(value));
					gtk_adjustment_set_lower(adj, c_to_f(lower));
					gtk_adjustment_set_upper(adj, c_to_f(upper));
				}
				else /* Previous is kelvin */
				{
					gtk_adjustment_set_value(adj, k_to_f(value));
					gtk_adjustment_set_lower(adj, k_to_f(lower));
					gtk_adjustment_set_upper(adj, k_to_f(upper));
				}

				gtk_adjustment_changed(adj);
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
				update_widget_f(widget,NULL);
			}
			if ((GTK_IS_ENTRY(widget)) && (widget_temp != FAHRENHEIT))
			{
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
				update_widget_f(widget,NULL);
			}
			if ((GTK_IS_RANGE(widget)) && (widget_temp != FAHRENHEIT))
			{
				adj = (GtkAdjustment *) gtk_range_get_adjustment(
						GTK_RANGE(widget));
				upper = gtk_adjustment_get_upper(adj);
				lower = gtk_adjustment_get_lower(adj);
				value = gtk_adjustment_get_value(adj);
				if (widget_temp == CELSIUS)
				{
					gtk_adjustment_set_value(adj, c_to_f(value));
					gtk_adjustment_set_lower(adj, c_to_f(lower));
					gtk_adjustment_set_upper(adj, c_to_f(upper));
				}
				else /* Previous is kelvin */
				{
					gtk_adjustment_set_value(adj, k_to_f(value));
					gtk_adjustment_set_lower(adj, k_to_f(lower));
					gtk_adjustment_set_upper(adj, k_to_f(upper));
				}

				gtk_range_set_adjustment(GTK_RANGE(widget),adj);
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
			}
			break;
		case CELSIUS:
			/*printf("fahr %s\n",(name == NULL ? "undefined" : name));*/
			if (GTK_IS_LABEL(widget))
			{
				if ((dep_obj) && (state))	
					text = (gchar *)OBJ_GET(widget,"alt_c_label");
				else
					text = (gchar *)OBJ_GET(widget,"c_label");
				gtk_label_set_text(GTK_LABEL(widget),text);
				//gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));

			}
			if ((GTK_IS_SPIN_BUTTON(widget)) && (widget_temp != CELSIUS))
			{

				adj = (GtkAdjustment *) gtk_spin_button_get_adjustment(
						GTK_SPIN_BUTTON(widget));
				upper = gtk_adjustment_get_upper(adj);
				value = gtk_adjustment_get_value(adj);
				lower = gtk_adjustment_get_lower(adj);
				if (widget_temp == FAHRENHEIT)
				{
					gtk_adjustment_set_value(adj, f_to_c(value));
					gtk_adjustment_set_lower(adj, f_to_c(lower));
					gtk_adjustment_set_upper(adj, f_to_c(upper));
				}
				else /* Previous is kelvin */
				{
					gtk_adjustment_set_value(adj, k_to_c(value));
					gtk_adjustment_set_lower(adj, k_to_c(lower));
					gtk_adjustment_set_upper(adj, k_to_c(upper));
				}

				gtk_adjustment_changed(adj);
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
				update_widget_f(widget,NULL);
			}
			if ((GTK_IS_ENTRY(widget)) && (widget_temp != CELSIUS))
			{
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
				update_widget_f(widget,NULL);
			}
			if ((GTK_IS_RANGE(widget)) && (widget_temp != CELSIUS))
			{
				adj = (GtkAdjustment *) gtk_range_get_adjustment(
						GTK_RANGE(widget));
				upper = gtk_adjustment_get_upper(adj);
				lower = gtk_adjustment_get_lower(adj);
				value = gtk_adjustment_get_value(adj);
				if (widget_temp == FAHRENHEIT)
				{
					gtk_adjustment_set_value(adj, f_to_c(value));
					gtk_adjustment_set_lower(adj, f_to_c(lower));
					gtk_adjustment_set_upper(adj, f_to_c(upper));
				}
				else /* Previous is kelvin */
				{
					gtk_adjustment_set_value(adj, k_to_c(value));
					gtk_adjustment_set_lower(adj, k_to_c(lower));
					gtk_adjustment_set_upper(adj, k_to_c(upper));
				}

				gtk_range_set_adjustment(GTK_RANGE(widget),adj);
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
			}
			break;
		case KELVIN:
			/*printf("fahr %s\n",(name == NULL ? "undefined" : name));*/
			if (GTK_IS_LABEL(widget))
			{
				if ((dep_obj) && (state))	
					text = (gchar *)OBJ_GET(widget,"alt_k_label");
				else
					text = (gchar *)OBJ_GET(widget,"k_label");
				gtk_label_set_text(GTK_LABEL(widget),text);
				//gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));

			}
			if ((GTK_IS_SPIN_BUTTON(widget)) && (widget_temp != KELVIN))
			{

				adj = (GtkAdjustment *) gtk_spin_button_get_adjustment(
						GTK_SPIN_BUTTON(widget));
				upper = gtk_adjustment_get_upper(adj);
				value = gtk_adjustment_get_value(adj);
				lower = gtk_adjustment_get_lower(adj);
				if (widget_temp == FAHRENHEIT)
				{
					gtk_adjustment_set_value(adj, f_to_k(value));
					gtk_adjustment_set_lower(adj, f_to_k(lower));
					gtk_adjustment_set_upper(adj, f_to_k(upper));
				}
				else /* Previous is celsius */
				{
					gtk_adjustment_set_value(adj, c_to_k(value));
					gtk_adjustment_set_lower(adj, c_to_k(lower));
					gtk_adjustment_set_upper(adj, c_to_k(upper));
				}

				gtk_adjustment_changed(adj);
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
				update_widget_f(widget,NULL);
			}
			if ((GTK_IS_ENTRY(widget)) && (widget_temp != KELVIN))
			{
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
				update_widget_f(widget,NULL);
			}
			if ((GTK_IS_RANGE(widget)) && (widget_temp != KELVIN))
			{
				adj = (GtkAdjustment *) gtk_range_get_adjustment(
						GTK_RANGE(widget));
				upper = gtk_adjustment_get_upper(adj);
				lower = gtk_adjustment_get_lower(adj);
				value = gtk_adjustment_get_value(adj);
				if (widget_temp == FAHRENHEIT)
				{
					gtk_adjustment_set_value(adj, f_to_k(value));
					gtk_adjustment_set_lower(adj, f_to_k(lower));
					gtk_adjustment_set_upper(adj, f_to_k(upper));
				}
				else /* Previous is celsius */
				{
					gtk_adjustment_set_value(adj, c_to_k(value));
					gtk_adjustment_set_lower(adj, c_to_k(lower));
					gtk_adjustment_set_upper(adj, c_to_k(upper));
				}

				gtk_range_set_adjustment(GTK_RANGE(widget),adj);
				OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
			}
			break;
	}
	EXIT();
}


/*!
  \brief reset_temps() calls the convert_temps function for each widget in
  the "temperature" list
  \param type is the temp scale now selected
  \see TempUnits
  */
G_MODULE_EXPORT void reset_temps(gpointer type)
{
	ENTER();
	g_list_foreach(get_list("temperature"),convert_temps,type);
	EXIT();
	return;
}


/*!
  \brief temp_to_host Converts and ECU temperature into host units
  \param in is the input temperature in ECU scale
  \return temp in host (mtx user) scale
  */
G_MODULE_EXPORT gdouble temp_to_host(gdouble in)
{
	static Firmware_Details *firmware = NULL;
	gdouble res = 0.0;

	ENTER();

	TempUnits mtx_temp_units = (TempUnits)(GINT)DATA_GET(global_data,"mtx_temp_units");

	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,in);
	g_return_val_if_fail(DATA_GET(global_data,"mtx_temp_units"),in);

	if (firmware->ecu_temp_units == mtx_temp_units)
		res = in;
	else if ((firmware->ecu_temp_units == CELSIUS) && (mtx_temp_units == FAHRENHEIT))
		res = c_to_f(in);
	else if ((firmware->ecu_temp_units == CELSIUS) && (mtx_temp_units == KELVIN))
		res = c_to_k(in);
	else if ((firmware->ecu_temp_units == FAHRENHEIT) && (mtx_temp_units == KELVIN))
		res = f_to_k(in);
	else if ((firmware->ecu_temp_units == FAHRENHEIT) && (mtx_temp_units == CELSIUS))
		res = f_to_c(in);
	else if ((firmware->ecu_temp_units == KELVIN) && (mtx_temp_units == CELSIUS))
		res = k_to_c(in);
	else if ((firmware->ecu_temp_units == KELVIN) && (mtx_temp_units == FAHRENHEIT))
		res = k_to_f(in);
	else 
	{
		printf("temp_to_host(): BUG couldn't figure out temp conversion!\n");
		res = in;
	}
	return res;
	EXIT();
}


/*!
  \brief Converts temps to the scale that the ECU expects
  \param in is the input temp in host (mtx user) scale
  \returns temp in ECU scale
  */
G_MODULE_EXPORT gdouble temp_to_ecu(gdouble in)
{
	gdouble res = 0.0;
	static Firmware_Details *firmware = NULL;
	TempUnits mtx_temp_units = (TempUnits)(GINT)DATA_GET(global_data,"mtx_temp_units");

	ENTER();

	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,in);
	g_return_val_if_fail(DATA_GET(global_data,"mtx_temp_units"),in);

	if(firmware->ecu_temp_units == mtx_temp_units)
		res = in;
	else if ((firmware->ecu_temp_units == CELSIUS) && (mtx_temp_units == FAHRENHEIT))
		res = f_to_c(in);
	else if ((firmware->ecu_temp_units == CELSIUS) && (mtx_temp_units == KELVIN))
		res = k_to_c(in);
	else if ((firmware->ecu_temp_units == FAHRENHEIT) && (mtx_temp_units == KELVIN))
		res = k_to_f(in);
	else if ((firmware->ecu_temp_units == FAHRENHEIT) && (mtx_temp_units == CELSIUS))
		res = c_to_f(in);
	else if ((firmware->ecu_temp_units == KELVIN) && (mtx_temp_units == CELSIUS))
		res = c_to_k(in);
	else if ((firmware->ecu_temp_units == KELVIN) && (mtx_temp_units == FAHRENHEIT))
		res = f_to_k(in);
	else
	{
		printf("temp_to_ecu(): BUG couldn't figure out temp conversion!\n");
		res = in;
	}
	return res;
	EXIT();
}


/*!
  \brief Celsius to Fahrenheit conversion
  \param in is the input temp in Celsius
  \returns temp in Fahrenheit
  */
G_MODULE_EXPORT gdouble c_to_f(gdouble in)
{
	ENTER();
	EXIT();
	return ((in *(9.0/5.0))+32.0);
}


/*!
  \brief Celsius to Kelvin conversion
  \param in is the input temp in Celsius
  \returns temp in Kelvin
  */
G_MODULE_EXPORT gdouble c_to_k(gdouble in)
{
	ENTER();
	EXIT();
	return (in+273.0);
}


/*!
  \brief Fahrenheit to Celsius conversion
  \param in is the input temp in Fahrenheit
  \returns temp in Celsius
  */
G_MODULE_EXPORT gdouble f_to_c(gdouble in)
{
	ENTER();
	EXIT();
	return ((in-32.0)*(5.0/9.0));
}


/*!
  \brief Fahrenheit to Kelvin conversion
  \param in is the input temp in Fahrenheit
  \returns temp in Kelvin
  */
G_MODULE_EXPORT gdouble f_to_k(gdouble in)
{
	ENTER();
	EXIT();
	return ((in-32.0)*(5.0/9.0))+273;
}


/*!
  \brief Kelvin to Fahrenheit conversion
  \param in is the input temp in Kevin
  \returns temp in Fahrenheit
  */
G_MODULE_EXPORT gdouble k_to_f(gdouble in)
{
	ENTER();
	EXIT();
	return (((in-273) *(9.0/5.0))+32.0);
}


/*!
  \brief Kelvin to Celsius conversion
  \param in is the input temp in Kelvin
  \returns temp in Celsius
  */
G_MODULE_EXPORT gdouble k_to_c(gdouble in)
{
	ENTER();
	EXIT();
	return (in-273.0);
}


/*!
  \brief Calculates the result based on the passed multiplier, adder, conversion cdirection (to/from ecu) and raw input)
  \param in is the input value
  \param mult is the pointer to multiplier
  \param add is the pointer to adder
  \param dir is the enumeration defining the conversion direction, i.e. TO/FROM Ecu
  \see ConvDir
  \returns the calculated result
  */
G_MODULE_EXPORT gfloat calc_value(gfloat in, gfloat *mult, gfloat *add, ConvDir dir)
{
	gfloat result = 0.0;

	ENTER();

	g_return_val_if_fail(((dir == FROMECU)||(dir == TOECU)),0.0);
	if (dir == FROMECU)
	{
		if ((mult) && (add))
			result = (in * (*mult)) + (*add);
		else if (mult)
			result = (in * (*mult));
		else
			result = in;
	}
	else if (dir == TOECU)
	{
		if ((mult) && (add))
			result = (in - (*add))/(*mult);
		else if (mult)
			result = in/(*mult);
		else
			result = in;
	}
	EXIT();
	return result;
}

