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
#include <widgetmgmt.h>


extern GObject *global_data;


void bind_keys(GObject *object, ConfigFile *cfgfile, gchar *section, gchar ** keys, gint num_keys)
{
	gint i = 0;
	gint tmpi = 0;
	gchar * tmpbuf = NULL;
	gchar * tmpstr = NULL;
	DataType keytype = MTX_STRING;

	for (i=0;i<num_keys;i++)
	{
		keytype = translate_string(keys[i]);
		switch(keytype)
		{
			case MTX_INT:
				if (cfg_read_int(cfgfile,section,keys[i],&tmpi))
				{
					dbg_func(KEYPARSER,g_strdup_printf(__FILE__": bind_keys()\n\tbinding INT \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section));
					OBJ_SET(object,
							keys[i],
							GINT_TO_POINTER(tmpi));	
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_INT: read of key \"%s\" from section \"%s\"  of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;
			case MTX_ENUM:
				if (cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					tmpi = translate_string(tmpbuf);
					dbg_func(KEYPARSER,g_strdup_printf(__FILE__": bind_keys()\n\tbinding ENUM \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section));
					OBJ_SET(object,
							keys[i],
							GINT_TO_POINTER(tmpi));	
					g_free(tmpbuf);
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_ENUM: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;
			case MTX_BOOL:
				if (cfg_read_boolean(cfgfile,section,keys[i],&tmpi))
				{
					dbg_func(KEYPARSER,g_strdup_printf(__FILE__": bind_keys()\n\tbinding BOOL \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section));
					OBJ_SET(object,
							keys[i],
							GINT_TO_POINTER(tmpi));	
					if (strstr(keys[i],"ul_complex"))
						load_complex_params(object,cfgfile,section);
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_BOOL: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;
			case MTX_STRING:
				if(cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					dbg_func(KEYPARSER,g_strdup_printf(__FILE__": bind_keys()\n\tbinding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n",keys[i],tmpbuf,section));
					tmpstr = OBJ_GET(object,keys[i]);
					/* If data already on widget, append
					 * new data and store */
					if ((tmpstr) && (g_strrstr(keys[i],"bind_to_list")!= NULL))
					{
						tmpstr = g_strconcat(tmpstr,",",tmpbuf,NULL);
						g_free(OBJ_GET(object,keys[i]));
						OBJ_SET(object,
								keys[i],
								g_strdup(tmpstr));
						g_free(tmpstr);
					}
					else
						OBJ_SET(object,keys[i],g_strdup(tmpbuf));
								
					g_free(tmpbuf);
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_STRING: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;
			default:
				break;

		}
	}

	return;
}
/*
void bind_keys(GObject *object, ConfigFile *cfgfile, gchar *section, gchar ** keys, gint * keytypes, gint num_keys)
{
	gint i = 0;
	gint tmpi = 0;
	gchar * tmpbuf = NULL;
	gchar * tmpstr = NULL;

	for (i=0;i<num_keys;i++)
	{
		switch((DataType)keytypes[i])
		{
			case MTX_INT:
				if (cfg_read_int(cfgfile,section,keys[i],&tmpi))
				{
					dbg_func(KEYPARSER,g_strdup_printf(__FILE__": bind_keys()\n\tbinding INT \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section));
					OBJ_SET(object,
							keys[i],
							GINT_TO_POINTER(tmpi));	
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_INT: read of key \"%s\" from section \"%s\"  of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;
			case MTX_ENUM:
				if (cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					tmpi = translate_string(tmpbuf);
					dbg_func(KEYPARSER,g_strdup_printf(__FILE__": bind_keys()\n\tbinding ENUM \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section));
					OBJ_SET(object,
							keys[i],
							GINT_TO_POINTER(tmpi));	
					g_free(tmpbuf);
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_ENUM: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;
			case MTX_BOOL:
				if (cfg_read_boolean(cfgfile,section,keys[i],&tmpi))
				{
					dbg_func(KEYPARSER,g_strdup_printf(__FILE__": bind_keys()\n\tbinding BOOL \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section));
					OBJ_SET(object,
							keys[i],
							GINT_TO_POINTER(tmpi));	
					if (strstr(keys[i],"ul_complex"))
						load_complex_params(object,cfgfile,section);
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_BOOL: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;
			case MTX_STRING:
				if(cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					dbg_func(KEYPARSER,g_strdup_printf(__FILE__": bind_keys()\n\tbinding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n",keys[i],tmpbuf,section));
					tmpstr = OBJ_GET(object,keys[i]);
					// If data already on widget, append
					// new data and store 
					if ((tmpstr) && (g_strrstr(keys[i],"bind_to_list")!= NULL))
					{
						tmpstr = g_strconcat(tmpstr,",",tmpbuf,NULL);
						g_free(OBJ_GET(object,keys[i]));
						OBJ_SET(object,
								keys[i],
								g_strdup(tmpstr));
						g_free(tmpstr);
					}
					else
						OBJ_SET(object,keys[i],g_strdup(tmpbuf));
								
					g_free(tmpbuf);
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_STRING: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;
			default:
				break;

		}
	}

	return;
}
*/
