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

#include <binreloc.h>
#include <config.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <getfiles.h>
#include <structures.h>



/*!
 \brief get_files() returns a list of files located at the pathstub passed
 this function will first search starting from ~/.MegaTunix+pathstub and
 then in the system path of $PREFIX/share/MegaTunix/+pathstub, it'll return
 the list as a vector char array. (free with g_strfreev)
 \param pathstub (gchar *) partial path to search for files
 \param extension (gchar *) extension to search for 
 \returns vector char array of filenames or NULL if none found
 */
gchar ** get_files(gchar *pathstub, gchar * extension)
{
	gchar *path = NULL;
	gchar *parent = NULL;
	gchar *list = NULL;
	gchar * filename = NULL;
	GDir *dir = NULL;

	path = g_strconcat(HOME(),"/.MegaTunix/",pathstub,NULL);
	dir = g_dir_open(path,0,NULL);
	if (!dir)
	{
		g_free(path);
		goto syspath;
	}
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		if (!g_str_has_suffix(filename,extension))
		{
			filename = (gchar *)g_dir_read_name(dir);
			continue;
		}

		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
			list = g_strconcat(list,",",path,filename,NULL);

		filename = (gchar *)g_dir_read_name(dir);
		
	}
	g_free(path);
	g_dir_close(dir);

	syspath:
	parent = gbr_find_data_dir(DATA_DIR);
	path = g_strconcat(parent,"/",pathstub,NULL);
	g_free(parent);
	dir = g_dir_open(path,0,NULL);
	if (!dir)
	{
		g_free(path);
		goto finish;
	}
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		if (!g_str_has_suffix(filename,extension))
		{
			filename = (gchar *)g_dir_read_name(dir);
			continue;
		}
		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
			list = g_strconcat(list,",",path,filename,NULL);
		filename = (gchar *)g_dir_read_name(dir);
	}
	g_free(path);
	g_dir_close(dir);

	finish:
	if (!list)
	{
		dbg_func(g_strdup(__FILE__": get_files()\n\t File list was NULL\n"),CRITICAL);
		return NULL;
	}
	return (g_strsplit(list,",",0));
}


/*!
 \brief get_file() gets a single file defnied by pathstub, first searching in
 ~/.MegaTunix+pathstub, and then in $PREFIX/share/MegaTunix/+pathstub,
 \param pathstub (gchar *) partial path to filename
 \param extension (gchar *) extension wanted..
 \returns filename if found or NULL if not found
 */
gchar * get_file(gchar *pathstub,gchar *extension)
{
	gchar * filename = NULL;
	gchar *dir = NULL;
	gchar *ext = NULL;
	if (extension)
		ext = g_strconcat(".",extension,NULL);
	else
		ext = g_strdup("");


	filename = g_strconcat(HOME(),"/.MegaTunix/",pathstub,ext,NULL);
	if (g_file_test(filename,(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)))
	{
		g_free(ext);
		return filename;
	}
	else 
	{
		g_free(filename);
		dir = gbr_find_data_dir(DATA_DIR);
		filename = g_strconcat(dir,"/",pathstub,ext,NULL);
		g_free(dir);
		g_free(ext);
		if (g_file_test(filename,(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)))
			return filename;
		else
			return NULL;
	}
	return NULL;
}
