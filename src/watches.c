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
EXPORT void fire_off_rtv_watches_pf()
{
	if (watch_hash)
		g_hash_table_foreach(watch_hash,process_watches,NULL);
}

guint32 create_bit_watch(gchar * varname, gint bit, gboolean state, gchar *fname)
{
	DataWatch *watch = NULL;
	watch = g_new0(DataWatch,1);
	watch->style = SINGLE_BIT;
	watch->varname = g_strdup(varname);
	watch->bit = bit;
	watch->state = state;
	watch->function = g_strdup(fname);
	watch->id = g_random_int();
	if (!watch_hash)
		watch_hash = g_hash_table_new_full(g_int_hash,g_int_equal,NULL,watch_destroy);
	g_hash_table_insert(watch_hash,GINT_TO_POINTER(watch->id),watch);
	return watch->id;
}


void watch_destroy(gpointer data)
{
	DataWatch *watch = (DataWatch *)data;
	if (watch->varname)
		g_free(watch->varname);
	if (watch->function)
		g_free(watch->function);
	g_free(watch);
}

void process_watches(gpointer key, gpointer value, gpointer data)
{
	DataWatch * watch = (DataWatch *)value;
	gfloat tmpf = 0.0;
	guint8 tmpi = 0;
	printf("key id %ui\n",GPOINTER_TO_UINT(key));
	switch (watch->style)
	{
		case SINGLE_BIT:
			lookup_current_value(watch->varname, &tmpf);
			tmpi = (guint8)tmpf;
			if ((tmpi & watch->bit) == watch->state)
				printf("should call %s\n",watch->function);
			break;
		default:
			break;

	}
}
