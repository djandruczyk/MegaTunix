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

#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <keybinder.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <string.h>
#include <stringmatch.h>
#include <structures.h>
#include <widgetmgmt.h>


//	load_keys(widget,cfgfile,section,keys,keytypes,num_keys);
void bind_keys(GObject *object, ConfigFile *cfgfile, gchar *section, gchar ** keys, gint * keytypes, gint num_keys)
{
	gint i = 0;
	gint tmpi = 0;
	gchar * tmpbuf = NULL;

	for (i=0;i<num_keys;i++)
	{
		switch((DataType)keytypes[i])
		{
			case MTX_INT:
				if (cfg_read_int(cfgfile,section,keys[i],&tmpi))
				{
					dbg_func(g_strdup_printf(__FILE__": bind_keys()\n\tbinding INT \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section),TABLOADER);
					g_object_set_data(object,
							g_strdup(keys[i]),
							GINT_TO_POINTER(tmpi));	
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_keys()\n\tMTX_INT: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;
			case MTX_ENUM:
				if (cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					tmpi = translate_string(tmpbuf);
					dbg_func(g_strdup_printf(__FILE__": bind_keys()\n\tbinding ENUM \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section),TABLOADER);
					g_object_set_data(object,
							g_strdup(keys[i]),
							GINT_TO_POINTER(tmpi));	
					g_free(tmpbuf);
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_keys()\n\tMTX_ENUM: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;
			case MTX_BOOL:
				if (cfg_read_boolean(cfgfile,section,keys[i],&tmpi))
				{
					dbg_func(g_strdup_printf(__FILE__": bind_keys()\n\tbinding BOOL \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section),TABLOADER);
					g_object_set_data(object,
							g_strdup(keys[i]),
							GINT_TO_POINTER(tmpi));	
					if (strstr(keys[i],"ul_complex"))
						load_complex_params(object,cfgfile,section);
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_keys()\n\tMTX_BOOL: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;
			case MTX_STRING:
				if(cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					dbg_func(g_strdup_printf(__FILE__": bind_keys()\n\tbinding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n",keys[i],tmpbuf,section),TABLOADER);
					g_object_set_data(object,
							g_strdup(keys[i]),
							g_strdup(tmpbuf));
					g_free(tmpbuf);
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_keys()\n\tMTX_STRING: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;

		}
	}

	return;
}
