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
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <getfiles.h>
#include <lookuptables.h>
#include <stdlib.h>
#include <structures.h>

GHashTable *lookuptables = NULL;

void load_lookuptables(void *ptr)
{
	struct Canidate *canidate = ptr;
	g_hash_table_foreach(canidate->lookuptables,get_table,NULL);
}

void get_table(gpointer table_name, gpointer fname, gpointer user_data)
{
	gboolean status = FALSE;
	gchar * filename = NULL;
	
	filename = get_file(g_strconcat(LOOKUPTABLE_DIR,"/",(gchar *)fname,NULL));
	if (filename)
		status = load_table(table_name,filename);
	g_free(filename);
	if (!status)
	{
		dbg_func(g_strdup_printf(__FILE__": load_lookuptables()\n\tFAILURE loading \"%s\" lookuptable, EXITING!!\n",(gchar *)table_name),CRITICAL);
		exit (-2);
	}

}

gboolean load_table(gchar *table_name, gchar *filename)
{
	GIOStatus status;
	GIOChannel *iochannel;
	gboolean go = TRUE;
	gchar * str = NULL;
	gchar * tmp = NULL;
	gchar * end = NULL;
	GString *a_line; 
	gint *array = NULL;
	gint tmparray[2048]; // bad idea being static!!
	gint i = 0;

	iochannel = g_io_channel_new_file(filename,"r", NULL);
	status = g_io_channel_seek_position(iochannel,0,G_SEEK_SET,NULL);
	if (status != G_IO_STATUS_NORMAL)
		dbg_func(__FILE__": load_lookuptables()\n\tError seeking to beginning of the file\n",CRITICAL);
	while (go)	
	{
		a_line = g_string_new("\0");
		status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
		if (status == G_IO_STATUS_EOF)
			go = FALSE;
		else
		{
			str = g_strchug(g_strdup(a_line->str));
			if (g_str_has_prefix(str,"DB"))
			{
				str+=2; // move 2 places in	
				end = g_strrstr(str,"T");
				tmp = g_strndup(str,end-str);
				tmparray[i]=atoi(tmp);
				i++;
			}
		}
		g_string_free(a_line,TRUE);
	}

	array = g_memdup(&tmparray,i*sizeof(gint));
	if (!lookuptables)
		lookuptables = g_hash_table_new(g_str_hash,g_str_equal);
	g_hash_table_insert(lookuptables,table_name,array);

	return TRUE;
}
