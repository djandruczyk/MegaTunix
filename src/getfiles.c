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
#include <defines.h>
#include <enums.h>
#include <getfiles.h>
#include <structures.h>


gchar ** get_files(gchar *pathstub)
{
	gchar *path = NULL;
	gchar *list = NULL;
	gchar * filename = NULL;
	GDir *dir = NULL;

	path = g_strconcat(g_get_home_dir(),"/.MegaTunix/",pathstub,NULL);
	dir = g_dir_open(path,0,NULL);
	if (!dir)
	{
		g_free(path);
		goto syspath;
	}
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
			list = g_strconcat(list,",",path,filename,NULL);

		filename = (gchar *)g_dir_read_name(dir);
		
	}
	g_free(path);
	g_dir_close(dir);

	syspath:
	path = g_strconcat(DATA_DIR,"/",pathstub,NULL);
	dir = g_dir_open(path,0,NULL);
	if (!dir)
	{
		g_free(path);
		goto finish;
	}
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
			list = g_strconcat(list,",",path,filename,NULL);
		filename = (gchar *)g_dir_read_name(dir);
	}
	g_free(path);
	g_dir_close(dir);

	finish:
	return (g_strsplit(list,",",0));
}
