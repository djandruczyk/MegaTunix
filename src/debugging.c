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
#include <debugging.h>
#include <enums.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <time.h>


gint dbg_lvl = 0;

static FILE * dbgfile = NULL;
GStaticMutex dbg_mutex = G_STATIC_MUTEX_INIT;

/*!
 \brief open_debugfile() opens the file that holds debugging information.
 The path defaults to the current working directory.
 */
void open_debugfile()
{
	gchar * filename = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;

	g_static_mutex_lock(&dbg_mutex);
	if(!dbgfile)
	{
		filename = g_build_filename(HOME(), "MTXlog.txt",NULL);
		dbgfile = fopen(filename,"w");
		g_free(filename);
		if (dbgfile)
		{
			t = g_malloc(sizeof(time_t));
			time(t);
			tm = localtime(t);
			g_free(t);
			g_fprintf(dbgfile,"Logfile opened for appending on %i-%.2i-%i at %.2i:%.2i \n",1+(tm->tm_mon),tm->tm_mday,1900+(tm->tm_year),tm->tm_hour,tm->tm_min);
		}
	}
	g_static_mutex_unlock(&dbg_mutex);
}


void close_debugfile()
{
	g_static_mutex_lock(&dbg_mutex);
	if (dbgfile)
		fclose(dbgfile);
	g_static_mutex_unlock(&dbg_mutex);
}

/*!
 \brief dbg_func() writes debugggin output to the console based on if the
 passed debug level is marked in the current debugging mask.
 \param str (gchar *) message to print out
 \param class (Dbg_Class enumeration) the debug level
 */
void dbg_func(gchar *str, Dbg_Class class)
{
	extern gboolean use_timestamps;
	static struct tm *tm = NULL;
	static time_t *t = NULL;
	extern gboolean leaving;

//	if (leaving)
//		return;
	if (!dbgfile)
	{
		g_free(str);
		return;
	}

	g_static_mutex_lock(&dbg_mutex);

	if ((dbgfile) && (dbg_lvl & class))
	{
		if (use_timestamps)
		{
			if (!t)
				t = g_malloc(sizeof(time_t));
			time(t);
			tm = localtime(t);
			g_fprintf(dbgfile,"%.2i:%.2i:%.2i  %s",tm->tm_hour,tm->tm_min,tm->tm_sec,str);
		}
		else
			g_fprintf(dbgfile,str);
		fflush(dbgfile);
		g_free(str);
	}
	g_static_mutex_unlock(&dbg_mutex);
}
