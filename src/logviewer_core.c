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
#include <fileio.h>
#include <glib/gprintf.h>
#include <gui_handlers.h>
#include <logviewer_core.h>
#include <logviewer_gui.h>
#include <ms_structures.h>
#include <structures.h>



void load_logviewer_file(void *ptr)
{
	struct Io_File *iofile = NULL;
	//gchar *tmpbuf = NULL;
	//GIOStatus status = 0;
	struct Log_Info *log_info = NULL;
	

	if (ptr != NULL)
		iofile = (struct Io_File*) ptr;
	else
        {
                g_fprintf(stderr,__FILE__": load_logviewer_file() pointer null\n");
                return;
        }
	log_info = g_malloc0(sizeof(struct Log_Info));
	initialize_log_info(log_info);
	read_logviewer_header(iofile->iochannel, log_info);
	close_file(iofile);
	printf("Not quite implemented yet....\n");
	return;
}

/* Initializer routine for the log_info datastructure */
void initialize_log_info(void *ptr)
{
	struct Log_Info *log_info;
	log_info = ptr;
	log_info->field_count = 0;
	log_info->fields_hash = g_hash_table_new(NULL,NULL);
	log_info->delim = NULL;
	log_info->fields = NULL;
	return;
}

/* First we read the first line,  try to determine if the delimiter
 * is a COMMA, TAB or a SPACE.
 * If we detect SPACE delimiting,  we need to read one dataline because
 * the variable names can be space delimited and we need to figure out
 * how to discern thespace inthe name with the space in between variable names
 * This may be UGLY...
 */
void read_logviewer_header(GIOChannel *iochannel, void *ptr)
{
	GString *a_line = g_string_new("\0");
	GIOStatus  status = G_IO_STATUS_ERROR;
	gchar *delim = NULL;
	struct Log_Info *log_info = ptr;
	extern struct DynamicButtons buttons;
	extern GtkWidget *lv_darea;	

	status = g_io_channel_read_line_string(iochannel,a_line,NULL,NULL); 

	if (status == G_IO_STATUS_NORMAL) /* good read */
	{
		if (g_strrstr(a_line->str,",") != NULL)
			delim = g_strdup(",");
		else if (g_strrstr(a_line->str,"\t") != NULL)
			delim = g_strdup("\t");
		else	
			delim = g_strdup(" ");

		/* Store delimiter in structure */
		log_info->delim = g_strdup(delim);
		/* Store field names as well... 
		 * log_info->fields is a string vector (char **)
		 * that is NULL terminated thanks to g_strsplit()
		 */
		log_info->fields = g_strsplit(a_line->str,delim,0);

		log_info->field_count = 0;
		/* Get total count of fields in there too... */
		while (log_info->fields[log_info->field_count] != NULL)
			log_info->field_count++;
		/* Enable parameter selection button */
		gtk_widget_set_sensitive(buttons.logplay_sel_parm_but, TRUE);
		g_object_set_data(G_OBJECT(lv_darea),"log_info",(gpointer)log_info);
		
	}
	g_free(delim);
	
}
