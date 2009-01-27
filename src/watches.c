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

GStaticMutex watch_mutex = G_STATIC_MUTEX_INIT;

static GHashTable *watch_hash;
/*!
 \brief fire_off_rtv_watches_pf() Trolls through the watch list and if
 conditions are met, calls the corresponding fucntion(s)
 */
EXPORT void fire_off_rtv_watches_pf()
{
	if (watch_hash)
	{
//		g_static_mutex_lock(&watch_mutex);
		g_hash_table_foreach(watch_hash,process_watches,NULL);
//		g_static_mutex_unlock(&watch_mutex);
	}
}

guint32 create_single_bit_watch(gchar * varname, gint bit, gboolean state, gchar *fname, gpointer user_data)
{
	DataWatch *watch = NULL;
	watch = g_new0(DataWatch,1);
	watch->style = SINGLE_BIT;
	watch->varname = g_strdup(varname);
	watch->bit= bit;
	watch->state = state;
	watch->function = g_strdup(fname);
	watch->user_data = user_data;
	watch->id = g_random_int();
//	g_static_mutex_lock(&watch_mutex);
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,watch_destroy);
	//printf("Create watch %ui\n",watch->id);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
//	g_static_mutex_unlock(&watch_mutex);
	return watch->id;
}


void watch_destroy(gpointer data)
{
	DataWatch *watch = (DataWatch *)data;
	//printf("destroying watch %ui\n",watch->id);
	if (watch->varname)
		g_free(watch->varname);
	if (watch->function)
		g_free(watch->function);
	g_free(watch);
}


void remove_watch(guint32 watch_id)
{
//	g_static_mutex_lock(&watch_mutex);
	//printf("remove watch %ui\n",watch_id);
	g_hash_table_remove(watch_hash,GINT_TO_POINTER(watch_id));
	//printf("watch removed %ui\n",watch_id);
//	g_static_mutex_unlock(&watch_mutex);
}


void process_watches(gpointer key, gpointer value, gpointer data)
{
	DataWatch * watch = (DataWatch *)value;
	gfloat tmpf = 0.0;
	guint8 tmpi = 0;
	GModule *module = NULL;
	void (*func) (gpointer);
	//printf("key id %ui\n",GPOINTER_TO_UINT(key));
	switch (watch->style)
	{
		case SINGLE_BIT:
			lookup_current_value(watch->varname, &tmpf);
			tmpi = (guint8)tmpf;
			if (((tmpi & (1 << watch->bit)) >> watch->bit) == watch->state)
			{
				module = g_module_open(NULL,G_MODULE_BIND_LAZY);
				if (module)
					g_module_symbol(module,watch->function, (void *)&func);
				g_module_close(module);
				//printf("Fire watch handler %ui\n",watch->id);
				func(watch->user_data);

				remove_watch(watch->id);
			}
			break;
		default:
			break;

	}
}


gboolean watch_active(guint32 id)
{
	//printf("watch_active call for watch %ui\n",id);
	if (g_hash_table_lookup(watch_hash,GINT_TO_POINTER(id)))
		return TRUE;
	else
		return FALSE;
}
