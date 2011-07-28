/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */

#include <plugin.h>
#include <rtv_processor.h>
#include <stdio.h>
#include <watches.h>


static GHashTable *watch_hash;
/*!
 \brief fire_off_rtv_watches_pf() Trolls through the watch list and if
 conditions are met, calls the corresponding fucntion(s)
 */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT void fire_off_rtv_watches_pf(void)
{
	if (watch_hash)
	{
		gdk_threads_enter();
		g_hash_table_foreach(watch_hash,process_watches,NULL);
		gdk_threads_leave();
	}
}


/*!
  \brief Creates a watch that monitors for a single bit to be a specific state,
  when it is this watch fires.
  \param varname, an internal name ofthe variable we want to watch for a bit
  change
  \param bit, bit to watch from 0-7
  \param state, the state you want the bit to be, i.e. TRUE or FALSE
  \param one_shot, If true, this watch is called only once then expires
  \param fname, function name to call when this watch fires
  \param user_data, pointer to data to pass to the function when this watch
  fires
  \returns ID for this watch so it can be cancelled whe nno longer used.
  */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT guint32 create_single_bit_state_watch(const gchar * varname, gint bit, gboolean state, gboolean one_shot,const gchar *fname, gpointer user_data)
{
	DataWatch *watch = NULL;

	watch = g_new0(DataWatch,1);
	watch->style = SINGLE_BIT_STATE;
	watch->varname = g_strdup(varname);
	watch->bit= bit;
	watch->state = state;
	watch->function = g_strdup(fname);
	watch->user_data = user_data;
	watch->id = g_random_int();
	watch->one_shot = one_shot;
	get_symbol(watch->function,(void *)&watch->func);
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,watch_destroy);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
	return watch->id;
}


/*!
  \brief Creates a watch that monitors for a single bit to change within a
  variable.
  \param varname, an internal name ofthe variable we want to watch for a bit
  change
  \param bit, bit to watch from 0-7
  \param state, the state you want the bit to be, i.e. TRUE or FALSE
  \param one_shot, If true, this watch is called only once then expires
  \param fname, function name to call when this watch fires
  \param user_data, pointer to data to pass to the function when this watch
  fires
  \returns ID for this watch so it can be cancelled whe nno longer used.
  */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT guint32 create_single_bit_change_watch(const gchar * varname, gint bit,gboolean one_shot,const gchar *fname, gpointer user_data)
{
	DataWatch *watch = NULL;

	watch = g_new0(DataWatch,1);
	watch->style = SINGLE_BIT_CHANGE;
	watch->varname = g_strdup(varname);
	watch->bit= bit;
	watch->function = g_strdup(fname);
	watch->user_data = user_data;
	watch->id = g_random_int();
	watch->one_shot = one_shot;
	get_symbol(watch->function,(void *)&watch->func);
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,watch_destroy);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
	return watch->id;
}


/*!
  \brief Creates a watch that monitors for a variable's value to change.
  \param varname, an internal name ofthe variable we want to watch for changes
  \param one_shot, if TRUE, this watch is called only once then expires
  \param fname, function name to call when this watch fires
  \param user_data, pointer to data to pass to the function when this watch
  fires
  \returns ID for this watch so it can be cancelled whe nno longer used.
  */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT guint32 create_value_change_watch(const gchar * varname, gboolean one_shot,const gchar *fname, gpointer user_data)
{
	DataWatch *watch = NULL;

	watch = g_new0(DataWatch,1);
	watch->style = VALUE_CHANGE;
	watch->varname = g_strdup(varname);
	watch->function = g_strdup(fname);
	watch->user_data = user_data;
	watch->id = g_random_int();
	watch->one_shot = one_shot;
	get_symbol(watch->function,(void *)&watch->func);
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,watch_destroy);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
	return watch->id;
}


/*!
  \brief Creates a watch that monitors for multiple variable's values to change.
  \param varnames, A CSV list of internal names of the variables we want 
  to watch for changes in value
  \param one_shot, if TRUE, this watch is called only once then expires
  \param fname, function name to call when this watch fires
  \param user_data, pointer to data to pass to the function when this watch
  fires
  \returns ID for this watch so it can be cancelled whe nno longer used.
  */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT guint32 create_multi_value_watch(gchar ** varnames, gboolean one_shot,const gchar *fname, gpointer user_data)
{
	DataWatch *watch = NULL;

	watch = g_new0(DataWatch,1);
	watch->style = MULTI_VALUE;
	watch->num_vars = g_strv_length(varnames);
	watch->vals = g_new0(gfloat,watch->num_vars);
	watch->varnames = g_strdupv(varnames);
	watch->function = g_strdup(fname);
	watch->user_data = user_data;
	watch->id = g_random_int();
	watch->one_shot = one_shot;
	get_symbol(watch->function,(void *)&watch->func);
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,watch_destroy);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
	return watch->id;
}


/*!
  \brief destroys a watch given the pointer passed
  \param data, pointer to the DataWatch structure we need to destroy
  */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT void watch_destroy(gpointer data)
{
	DataWatch *watch = (DataWatch *)data;
	/*printf("destroying watch %ui\n",watch->id);*/

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
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


/*!
  \brief Removes a watch for the list of active watches
  \param watch_id, the watch identifier as returned by any of the 
  create_*_watch functions
  */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT void remove_watch(guint32 watch_id)
{
	g_hash_table_remove(watch_hash,GINT_TO_POINTER(watch_id));
}


/*!
  \brief iterates over the has hof active watches and if they have fired
  call the corresponding watch function passing in the pointer to the 
  DataWatch structure as the argument
  \param key, unused
  \param value, pointer to DataWatch structure
  \param data, unused
  */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT void process_watches(gpointer key, gpointer value, gpointer data)
{
	DataWatch * watch = (DataWatch *)value;
	gfloat tmpf = 0.0;
	guint8 tmpi = 0;
	guint8 tmpi2 = 0;
	gint i = 0;
	/*printf("process watches running\n");*/

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
	switch (watch->style)
	{
		case SINGLE_BIT_STATE:
			/*! 
			   When bit becomes this state, fire watch function. 
			   This will fire each time new vars come in unless 
			   watch is set to run only once, thus it will 
			   evaporate after 1 run 
			 */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
			/*printf("single bit state\n");*/

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
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
			/*!
			  When bit CHANGES from previous state, i.e. only 
			  fire when it changes, but if it's stable, 
			  don't fire repeatedly 
			 */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
			/*printf("single bit change\n");*/

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
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
			/*!
			  If value changes at ALL from previous value, then
			  fire watch. (useful for gauges/dash/warmup 2d stuff)
			 */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
			/*printf("value change\n");*/

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
			lookup_current_value(watch->varname, &(watch->val));
			/* If it's a one-shot, fire it no matter what... */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
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

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
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


/*!
  \brief checks if a watch ID is in the active list of watches
  \param id, WatchID as returned by any of the create_*_watch functions
  \returns TRUE if the ID is valid, FALSE otherwise
  */

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT gboolean watch_active(guint32 id)
{
	/*printf("watch_active call for watch %ui\n",id);*/

/*! @file watches.c
 *
 * @brief ...
 *
 *
 */
	if (g_hash_table_lookup(watch_hash,GINT_TO_POINTER(id)))
		return TRUE;
	else
		return FALSE;
}
