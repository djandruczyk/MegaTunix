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
#include <lookuptables.h>
#include <stdlib.h>
#include <structures.h>


void load_lookuptables(void *ptr)
{
	struct Canidate *canidate = ptr;
	gchar * filename = NULL;
	gboolean status = FALSE;;

	/* MAT table first... */
	filename = g_strconcat(DATA_DIR,"/",LOOKUPTABLE_DIR,"/",canidate->mat_tbl_name,NULL);
	if (g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		status = load_table(MAT,filename);
	if (!status)
	{
		filename = g_strconcat(g_get_home_dir(),"/.MegaTunix/",LOOKUPTABLE_DIR,"/",canidate->mat_tbl_name,NULL);
		if (g_file_test(filename,G_FILE_TEST_IS_REGULAR))
			status = load_table(MAT,filename);
	}
	if (!status)
		dbg_func(__FILE__": load_lookuptables(), FAILURE loading MAT lookuptable\n",CRITICAL);

	/* CLT table next... */
	filename = g_strconcat(DATA_DIR,"/",LOOKUPTABLE_DIR,"/",canidate->clt_tbl_name,NULL);
	if (g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		status = load_table(CLT,filename);
	if (!status)
	{
		filename = g_strconcat(g_get_home_dir(),"/.MegaTunix/",LOOKUPTABLE_DIR,"/",canidate->clt_tbl_name,NULL);
		if (g_file_test(filename,G_FILE_TEST_IS_REGULAR))
			status = load_table(CLT,filename);
	}
	if (!status)
		dbg_func(__FILE__": load_lookuptables(), FAILURE loading CLT lookuptable\n",CRITICAL);

}

gboolean load_table(TableType ttype,gchar * filename)
{
	GIOStatus status;
	GIOChannel *iochannel;
	gboolean go = TRUE;
	gchar * str = NULL;
	gchar * tmp = NULL;
	gchar * end = NULL;
	GString *a_line; 
	extern gint matfactor[];
	extern gint cltfactor[];
	gint tmparray[256];
	gint i = 0;

	iochannel = g_io_channel_new_file(filename,"r", NULL);
	status = g_io_channel_seek_position(iochannel,0,G_SEEK_SET,NULL);
	if (status != G_IO_STATUS_NORMAL)
		dbg_func(__FILE__": load_lookuptables() Eror seeking to beginning of the file\n",CRITICAL);
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
	if (ttype == MAT)
		for (i=0;i<256;i++)
			matfactor[i]=tmparray[i];
	else if (ttype == CLT)
		for (i=0;i<256;i++)
			cltfactor[i]=tmparray[i];
	else
	{
		dbg_func(__FILE__": load_table() table type is out of range\n",CRITICAL);
		return FALSE;
	}

	return TRUE;
}
