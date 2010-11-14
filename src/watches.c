/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <comms_gui.h>
#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <firmware.h>
#include <math.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <stdlib.h>
#include <watches.h>


static GHashTable *watch_hash;
/*!
 \brief fire_off_rtv_watches_pf() Trolls through the watch list and if
 conditions are met, calls the corresponding fucntion(s)
 */
EXPORT void fire_off_rtv_watches_pf(void)
{
	if (watch_hash)
	{
		gdk_threads_enter();
		g_hash_table_foreach(watch_hash,process_watches,NULL);
		gdk_threads_leave();
	}
}

guint32 create_single_bit_state_watch(gchar * varname, gint bit, gboolean state, gboolean one_shot,gchar *fname, gpointer user_data)
{
	DataWatch *watch = NULL;
	GModule *module = NULL;

	watch = g_new0(DataWatch,1);
	watch->style = SINGLE_BIT_STATE;
	watch->varname = g_strdup(varname);
	watch->bit= bit;
	watch->state = state;
	watch->function = g_strdup(fname);
	watch->user_data = user_data;
	watch->id = g_random_int();
	watch->one_shot = one_shot;
	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (module)
		g_module_symbol(module,watch->function, (void *)&watch->func);
	g_module_close(module);
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,watch_destroy);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
	return watch->id;
}

guint32 create_single_bit_change_watch(gchar * varname, gint bit,gboolean one_shot,gchar *fname, gpointer user_data)
{
	DataWatch *watch = NULL;
	GModule *module = NULL;

	watch = g_new0(DataWatch,1);
	watch->style = SINGLE_BIT_CHANGE;
	watch->varname = g_strdup(varname);
	watch->bit= bit;
	watch->function = g_strdup(fname);
	watch->user_data = user_data;
	watch->id = g_random_int();
	watch->one_shot = one_shot;
	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (module)
		g_module_symbol(module,watch->function, (void *)&watch->func);
	g_module_close(module);
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,watch_destroy);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
	return watch->id;
}


guint32 create_value_change_watch(gchar * varname, gboolean one_shot,gchar *fname, gpointer user_data)
{
	DataWatch *watch = NULL;
	GModule *module = NULL;

	watch = g_new0(DataWatch,1);
	watch->style = VALUE_CHANGE;
	watch->varname = g_strdup(varname);
	watch->function = g_strdup(fname);
	watch->user_data = user_data;
	watch->id = g_random_int();
	watch->one_shot = one_shot;
	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (module)
		g_module_symbol(module,watch->function, (void *)&watch->func);
	g_module_close(module);
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,watch_destroy);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
	return watch->id;
}


guint32 create_multi_value_watch(gchar ** varnames, gboolean one_shot,gchar *fname, gpointer user_data)
{
	DataWatch *watch = NULL;
	GModule *module = NULL;
	gint i = 0;

	watch = g_new0(DataWatch,1);
	watch->style = MULTI_VALUE;
	watch->num_vars = g_strv_length(varnames);
	watch->vals = g_new0(gfloat,watch->num_vars);
	watch->varnames = g_new0(gchar *, watch->num_vars);
	for (i=0;i<watch->num_vars;i++)
		watch->varnames[i] = g_strdup(varnames[i]);
	watch->function = g_strdup(fname);
	watch->user_data = user_data;
	watch->id = g_random_int();
	watch->one_shot = one_shot;
	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (module)
		g_module_symbol(module,watch->function, (void *)&watch->func);
	g_module_close(module);
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,watch_destroy);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
	return watch->id;
}


void watch_destroy(gpointer data)
{
	DataWatch *watch = (DataWatch *)data;
	/*printf("destroying watch %ui\n",watch->id);*/
	if (watch->varname)
		g_free(watch->varname);
	if (watch->vals)
		g_free(watch->vals);
	if (watch->varnames)
		g_strfreev(watch->varnames);
	if (watch->function)
		g_free(watch->function);
	g_free(watch);
}


void remove_watch(guint32 watch_id)
{
	g_hash_table_remove(watch_hash,GINT_TO_POINTER(watch_id));
}


void process_watches(gpointer key, gpointer value, gpointer data)
{
	DataWatch * watch = (DataWatch *)value;
	gfloat tmpf = 0.0;
	guint8 tmpi = 0;
	guint8 tmpi2 = 0;
	gint i = 0;
	/*printf("process watches running\n");*/
	switch (watch->style)
	{
		case SINGLE_BIT_STATE:
		/* When bit becomes this state, fire watch function. This will
		 * fire each time new vars come in unless watch is set to
		 * run only once, thus it will evaporate after 1 run */
			/*printf("single bit state\n");*/
			lookup_current_value(watch->varname, &tmpf);
			tmpi = (guint8)tmpf;
			if (((tmpi & (1 << watch->bit)) >> watch->bit) == watch->state)
			{
				tmpi = ((tmpi & (1 << watch->bit)) >> watch->bit);
				watch->val = tmpi;
				watch->func(watch);
				if (watch->one_shot)
					remove_watch(watch->id);
			}
			break;
		case SINGLE_BIT_CHANGE:
		/* When bit CHANGES from previous state, i.e.  only fire when
		 * it changes, but if it's stable, don't fire repeatedly 
		 */
			/*printf("single bit change\n");*/
			lookup_current_value(watch->varname, &tmpf);
			tmpi = (guint8)tmpf;
			lookup_previous_value(watch->varname, &tmpf);
			tmpi2 = (guint8)tmpf;
			if (((tmpi & (1 << watch->bit)) >> watch->bit) != ((tmpi2 & (1 << watch->bit)) >> watch->bit))
			{
				tmpi = ((tmpi & (1 << watch->bit)) >> watch->bit);
				watch->val = (gfloat)tmpi;
				watch->func(watch);
				if (watch->one_shot)
					remove_watch(watch->id);
			}
			break;
		case VALUE_CHANGE:
			/*printf("value change\n");*/
			/* If value changes at ALL from previous value, then
			 * fire watch. (useful for gauges/dash/warmup 2d stuff)
			 */
			lookup_current_value(watch->varname, &(watch->val));
			/* If it's a one-shot, fire it no matter what... */
			if (watch->one_shot)
			{
				watch->func(watch);
				remove_watch(watch->id);
				break;
			}

			if (watch->val != watch->last_val)
			{
				watch->last_val = watch->val;
				watch->func(watch);
			}
			else
				watch->last_val = watch->val;
			break;
		case MULTI_VALUE:
			/*printf("multi value change\n");*/
			for (i=0;i<watch->num_vars;i++)
			{
				if (watch->varnames[i])
					lookup_current_value(watch->varnames[i], &watch->vals[i]);
				else
					watch->vals[i]=0.0;
			}
			watch->func(watch);
			if (watch->one_shot)
			{
				remove_watch(watch->id);
				break;
			}
			break;
		default:
			break;
	}
}


gboolean watch_active(guint32 id)
{
	/*printf("watch_active call for watch %ui\n",id);*/
	if (g_hash_table_lookup(watch_hash,GINT_TO_POINTER(id)))
		return TRUE;
	else
		return FALSE;
}
