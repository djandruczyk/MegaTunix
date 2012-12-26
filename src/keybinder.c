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
  \file src/keybinder.c
  \ingroup CoreMtx
  \brief Deals with initializing an object with the info from the config file

  This file deals with extracting out the information needed from a config
  file and storing it within the object in a way that it's easily accessed
  \author David Andruczyk
  */

#include <defines.h>
#include <debugging.h>
#include <rtv_map_loader.h>
#include <string.h>
#include <stringmatch.h>

/*!
  \brief Binds remaing keys in cfgfile to the object
  \param object is the pointer to object where we bind the data to
  \param cfgfile is the pointer to configfile  where we pull the data from
  \param section is the section name within cfgfile to gt the data from
  \param keys is the array of char *'s, list of keys to read and store
  \param num_keys is the number of elements in the keys array
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

	ENTER();
	for (i=0;i<num_keys;i++)
	{
		keytype = (DataType)translate_string(keys[i]);
		switch(keytype)
		{
			case MTX_INT:
				if (cfg_read_int(cfgfile,section,keys[i],&tmpi))
				{
					MTXDBG(KEYPARSER,_("Binding INT \"%s\",\"%i\" to widget \"%s\"\n"),keys[i],tmpi,section);
					OBJ_SET(object,
							keys[i],
							GINT_TO_POINTER(tmpi));	
				}
				else
					MTXDBG(KEYPARSER|CRITICAL,_("MTX_INT: read of key \"%s\" from section \"%s\"  of file \"%s\" failed\n"),keys[i],section,cfgfile->filename);
				break;
			case MTX_ENUM:
				if (cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					tmpi = translate_string(tmpbuf);
					MTXDBG(KEYPARSER,_("Binding ENUM \"%s\",\"%i\" to widget \"%s\"\n"),keys[i],tmpi,section);
					OBJ_SET(object,
							keys[i],
							GINT_TO_POINTER(tmpi));	
					g_free(tmpbuf);
				}
				else
					MTXDBG(KEYPARSER|CRITICAL,_("MTX_ENUM: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n"),keys[i],section,cfgfile->filename);
				break;
			case MTX_BOOL:
				if (cfg_read_boolean(cfgfile,section,keys[i],&tmpi))
				{
					MTXDBG(KEYPARSER,_("Binding BOOL \"%s\",\"%i\" to widget \"%s\"\n"),keys[i],tmpi,section);
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
					MTXDBG(KEYPARSER|CRITICAL,_("MTX_BOOL: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n"),keys[i],section,cfgfile->filename);
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
					MTXDBG(KEYPARSER|CRITICAL,_("MTX_FLOAT: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n"),keys[i],section,cfgfile->filename);
				break;

			case MTX_STRING:
				if(cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					MTXDBG(KEYPARSER,_("Binding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n"),keys[i],tmpbuf,section);
					tmpstr = (gchar *)DATA_GET(object,keys[i]);
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
					MTXDBG(KEYPARSER|CRITICAL,_("MTX_STRING: read of key \"%s\" from section \"%s\" of file \"%s\" failed\n"),keys[i],section,cfgfile->filename);
				break;
			default:
				break;

		}
	}

	EXIT();
	return;
}
