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
	gchar **vector = NULL;
	gchar * tmpbuf = NULL;
	GDir *dir = NULL;


	path = g_build_filename(HOME(), ".MegaTunix",pathstub,NULL);
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

		/* Create name of file and store temporarily */
		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
		{
			tmpbuf = g_strconcat(list,",",path,filename,NULL);
			g_free(list);
			list = tmpbuf;
		}

		filename = (gchar *)g_dir_read_name(dir);
		
	}
	g_free(path);
	g_dir_close(dir);

	syspath:
	parent = gbr_find_data_dir(DATA_DIR);
	path = g_build_filename(parent,pathstub,NULL);
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

		/* Create name of file and store temporarily */
		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
		{
			tmpbuf = g_strconcat(list,",",path,filename,NULL);
			g_free(list);
			list = tmpbuf;
		}

		filename = (gchar *)g_dir_read_name(dir);
	}
	g_free(path);
	g_dir_close(dir);

	finish:
	g_free(pathstub);
	g_free(extension);
	if (!list)
	{
		dbg_func(g_strdup(__FILE__": get_files()\n\t File list was NULL\n"),CRITICAL);
		return NULL;
	}
	vector = g_strsplit(list,",",0);
	g_free(list);
	return (vector);
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
	gchar *filename = NULL;
	gchar *dir = NULL;
	gchar *ext = NULL;
	gchar *file = NULL;
	if (extension)
		ext = g_strconcat(".",extension,NULL);
	else
		ext = g_strdup("");

	file = g_strconcat(pathstub,ext,NULL);

	g_free(ext);

	filename = g_build_filename(HOME(), ".MegaTunix",file,NULL);
	if (g_file_test(filename,(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)))
	{
		g_free(pathstub);
		g_free(extension);
		g_free(file);
		return filename;
	}
	else 
	{
		g_free(filename);
		dir = gbr_find_data_dir(DATA_DIR);
		filename = g_build_filename(dir,file,NULL);

		g_free(dir);
		g_free(file);

		if (g_file_test(filename,(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)))
		{
			g_free(pathstub);
			g_free(extension);
			return filename;
		}
		else
			g_free(filename);
	}
	g_free(pathstub);
	g_free(extension);
	return NULL;
}
