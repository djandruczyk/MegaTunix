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

#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>

gboolean check_dependancy(GObject *object )
{
	gint i = 0;
	gint page = 0;
	gint offset = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	gint bitval = 0;
	gchar ** deps = NULL;
	gint type = 0;
	gint num_deps = 0;
	extern guchar *ms_data[MAX_SUPPORTED_PAGES];

	num_deps = (gint)g_object_get_data(object,"num_deps");
	deps = g_object_get_data(object,"deps");
	for (i=0;i<num_deps;i++)
	{
		type = (gint)g_object_get_data(object,g_strdup_printf("%s_type",deps[i]));
		if (type == VE_EMB_BIT)
		{
			page = (gint)g_object_get_data(object,g_strdup_printf("%s_page",deps[i]));
			offset = (gint)g_object_get_data(object,g_strdup_printf("%s_offset",deps[i]));
			bitshift = (gint)g_object_get_data(object,g_strdup_printf("%s_bitshift",deps[i]));
			bitmask = (gint)g_object_get_data(object,g_strdup_printf("%s_bitmask",deps[i]));
			bitval = (gint)g_object_get_data(object,g_strdup_printf("%s_bitval",deps[i]));
			if (!(((ms_data[page][offset]) & bitmask) >> bitshift) == bitval)	
				return FALSE;
		}
/*		else if (type == VE_VAR)
		{
			page = (gint)g_object_get_data(object,g_strdup_printf("%s_page",deps[i]));
			offset = (gint)g_object_get_data(object,g_strdup_printf("%s_offset",deps[i]));
			value = (gint)g_object_get_data(object,g_strdup_printf("%s_offset",deps[i]));
		}
*/
	}
	return TRUE;
}
