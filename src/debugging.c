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
#include <glib/gprintf.h>


guint dbg_lvl = 0;

void dbg_func(gchar *str, Dbg_Class class)
{
	if ((dbg_lvl & class))
	{
		g_fprintf(stderr,str);
	}

}
