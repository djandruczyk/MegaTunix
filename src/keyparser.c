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
#include <keyparser.h>
#include <stringmatch.h>


gchar ** parse_keys(gchar * string, gint * count, gchar *delimiter)
{
	gchar **result = NULL;	
	gint i = 0;
	if (!string)
	{
		dbg_func(__FILE__": parse_keys()\n\t String passed was NULL\n",CRITICAL);
		*count = 0;
		return NULL;
	}

	result = g_strsplit(string,delimiter,0);
	while (result[i])
		i++;
	*count = i;	
	return result;
}

gint * parse_keytypes(gchar * string, gint * count, gchar *delimiter)
{
	gchar **vector = NULL;	
	gint *keytypes = NULL;
	gint i = 0;
	gint ct = 0;

	if (!string)
	{
		dbg_func(__FILE__": parse_keytypes()\n\t String passed was NULL\n",CRITICAL);
		*count = 0;
		return 0;
	}
	vector = g_strsplit(string,delimiter,0);
	while (vector[ct])
		ct++;

	keytypes = (gint *)g_malloc0(ct*sizeof(gint));	
	while (vector[i])
	{
		dbg_func(g_strdup_printf(__FILE__": parse_keytypes()\n\ttrying to translate %s\n",vector[i]),TABLOADER);
		keytypes[i] = translate_string(vector[i]);
		dbg_func(g_strdup_printf(__FILE__": parse_keytypes()\n\ttranslated value of %s is %i\n",vector[i],keytypes[i]),TABLOADER);
		i++;
	}
	g_strfreev(vector);
	*count = i;	
	return keytypes;

}
