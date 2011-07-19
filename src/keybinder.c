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

#include <defines.h>
#include <debugging.h>
#include <rtv_map_loader.h>
#include <string.h>
#include <stringmatch.h>




/*!
  \brief Binds remaing keys in cfgfile to the object
  */
G_MODULE_EXPORT void bind_keys(GObject *object, ConfigFile *cfgfile, gchar *section, gchar ** keys, gint num_keys)
{
	gint i = 0;
	gint tmpi = 0;
	gfloat tmpf = 0.0;
	gfloat *newfloat = NULL;
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
					if (tmpi)
					{
						OBJ_SET(object,
								keys[i],
								GINT_TO_POINTER(tmpi));	
						if (strstr(keys[i],"fromecu_complex"))
							load_complex_params_obj(object,cfgfile,section);
					}
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_BOOL: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;
			case MTX_FLOAT:
				if (cfg_read_float(cfgfile,section,keys[i],&tmpf))
				{
					newfloat = NULL;
					newfloat = g_new0(gfloat, 1);
					*newfloat = tmpf;
					OBJ_SET_FULL(object,
							keys[i],
							(gpointer)newfloat,g_free);
				}
				else
					dbg_func(KEYPARSER|CRITICAL,g_strdup_printf(__FILE__": bind_keys()\n\tMTX_FLOAT: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n",keys[i],section,cfgfile->filename));
				break;

			case MTX_STRING:
				if(cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					dbg_func(KEYPARSER,g_strdup_printf(__FILE__": bind_keys()\n\tbinding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n",keys[i],tmpbuf,section));
					tmpstr = DATA_GET(object,keys[i]);
					/* If data already on widget, append
					 * new data and store */
					if ((tmpstr) && (g_strrstr(keys[i],"bind_to_list")!= NULL))
					{
						tmpstr = g_strconcat(tmpstr,",",tmpbuf,NULL);
						OBJ_SET_FULL(object,
								keys[i],
								g_strdup(tmpstr),
								g_free);
						g_free(tmpstr);
					}
					else
						OBJ_SET_FULL(object,keys[i],g_strdup(tmpbuf),g_free);
								
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
