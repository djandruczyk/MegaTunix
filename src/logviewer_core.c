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
	read_log_header(iofile->iochannel, log_info);
	allocate_buffers(log_info);
	read_log_data(iofile->iochannel, log_info);
	close_file(iofile);
	return;
}

/* Initializer routine for the log_info datastructure */
void initialize_log_info(void *ptr)
{
	struct Log_Info *log_info;
	log_info = ptr;
	log_info->field_count = 0;
	log_info->delimiter = NULL;
	log_info->fields = NULL;
	log_info->fields_data = g_array_new(FALSE,FALSE,sizeof(GArray *));
	return;
}

/* First we read the first line,  try to determine if the delimiter
 * is a COMMA, TAB or a SPACE.
 * If we detect SPACE delimiting,  we need to read one dataline because
 * the variable names can be space delimited and we need to figure out
 * how to discern thespace inthe name with the space in between variable names
 * This may be UGLY...
 */
void read_log_header(GIOChannel *iochannel, void *ptr)
{
	GString *a_line = g_string_new("\0");
	GIOStatus  status = G_IO_STATUS_ERROR;
	gchar *delimiter = NULL;
	struct Log_Info *log_info = ptr;
	extern struct DynamicButtons buttons;
	extern GtkWidget *lv_darea;	

	status = g_io_channel_read_line_string(iochannel,a_line,NULL,NULL); 

	if (status == G_IO_STATUS_NORMAL) /* good read */
	{
		if (g_strrstr(a_line->str,",") != NULL)
			delimiter = g_strdup(",");
		else if (g_strrstr(a_line->str,"\t") != NULL)
			delimiter = g_strdup("\t");
		else	
			delimiter = g_strdup(" ");

		/* Store delimiter in structure */
		log_info->delimiter = g_strdup(delimiter);
		/* Store field names as well... 
		 * log_info->fields is a string vector (char **)
		 * that is NULL terminated thanks to g_strsplit()
		 */
		log_info->fields = g_strsplit(a_line->str,delimiter,0);

		log_info->field_count = 0;
		/* Get total count of fields in there too... */
		while (log_info->fields[log_info->field_count] != NULL)
			log_info->field_count++;
		/* Enable parameter selection button */
		gtk_widget_set_sensitive(buttons.logplay_sel_parm_but, TRUE);
		g_object_set_data(G_OBJECT(lv_darea),"log_info",(gpointer)log_info);
		
	}
	g_free(delimiter);
	
}

void allocate_buffers(void *ptr)
{
	gint i = 0;
	struct Log_Info *log_info = ptr;
	GArray *data_array;
	
	/* This looks weird but it should work pretty well.
	 * On Log_Info initialization we create a GArray. This 
	 * array contains pointers to the arrays of data for EACH variable
	 * in the logfile.  This makes it pretty easy to insert things
	 * and should be pretty fast. (well I hope so..)
	 */
	for (i=0;i<log_info->field_count;i++)
	{
		data_array = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
		g_array_insert_val(log_info->fields_data,i,data_array);
	}
}

void read_log_data(GIOChannel *iochannel, void *ptr)
{
	GString *a_line = g_string_new("\0");
	//GIOStatus  status = G_IO_STATUS_ERROR;
	struct Log_Info *log_info = ptr;
	gchar **data;
	gint i = 0;
	GArray *tmp_array;
	gfloat val = 0.0;

	while(g_io_channel_read_line_string(iochannel,a_line,NULL,NULL) 
			== G_IO_STATUS_NORMAL) 
	{
		data = g_strsplit(a_line->str,log_info->delimiter,0);
		for (i=0;i<(log_info->field_count);i++)
		{
			tmp_array = g_array_index(log_info->fields_data,GArray *,i);
			val = (gfloat)g_ascii_strtod(g_strdup(data[i]),NULL);
			g_array_append_val(tmp_array,val);

			//printf("data[%i]=%s\n",i,data[i]);
		}
		g_strfreev(data);
	}
}
	
