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
#include <fileio.h>
#include <gui_handlers.h>
#include <keyparser.h>
#include <logviewer_core.h>
#include <logviewer_gui.h>
#include <ms_structures.h>
#include <string.h>
#include <structures.h>
#include <tabloader.h>

struct Log_Info *log_info = NULL;

void load_logviewer_file(void *ptr)
{
	struct Io_File *iofile = NULL;

	if (ptr != NULL)
		iofile = (struct Io_File*) ptr;
	else
	{
		dbg_func(__FILE__": load_logviewer_file()\n\tIo_File pointer NULL,returning!!\n",CRITICAL);
		return;
	}
	log_info = g_malloc0(sizeof(struct Log_Info));
	initialize_log_info(log_info);
	read_log_header(iofile->iochannel, log_info);
	read_log_data(iofile->iochannel, log_info);
	populate_limits(log_info);
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
	log_info->log_list = g_array_new(FALSE,FALSE,sizeof(GObject *));
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
	gchar **fields = NULL;
	gint num_fields = 0;
	GArray *array = NULL;
	GObject *object = NULL;
	gint i = 0;
	struct Log_Info *log_info = ptr;
	extern GHashTable *dynamic_widgets;

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
		fields = parse_keys(a_line->str,&num_fields,delimiter);

		log_info->field_count = num_fields;
		/* Create objects, arrays and storage points... */
		for (i=0;i<num_fields;i++)
		{
			array = NULL;
			object = NULL;
			object = g_object_new(GTK_TYPE_INVISIBLE,NULL);
			array = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
			g_object_set_data(G_OBJECT(object),"data_array",(gpointer)array);
			g_object_set_data(G_OBJECT(object),"lview_name",g_strdup(g_strstrip(fields[i])));
			g_array_append_val(log_info->log_list,object);
		}
		/* Enable parameter selection button */
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button"), TRUE);
		g_object_set_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea")),"log_info",(gpointer)log_info);

	}
	g_free(delimiter);

}


void populate_limits(void *ptr)
{
	struct Log_Info *log_info = NULL;
	gint i = 0;
	gint j = 0;
	log_info = ptr;
	GObject * object = NULL;
	GArray *array = NULL;
	gfloat val = 0.0;
	gfloat lower = 0.0;
	gfloat upper = 0.0;
	gint tmpi = 0;
	gint len = 0;

	for (i=0;i<log_info->field_count;i++)
	{
		object = NULL;
		array = NULL;
		lower = 0.0;
		upper = 0.0;
		tmpi = 0;
		len = 0;
		object = g_array_index(log_info->log_list,GObject *, i);
		array = (GArray *)g_object_get_data(object,"data_array");
		len = array->len;
		for (j=0;j<len;j++)
		{
			val = g_array_index(array,gfloat, j);
			if (val < lower)
				lower = val;
			if (val >upper )
				upper = val;

		}
		tmpi = floor(lower) -1.0;
		g_object_set_data(object,"lower_limit", GINT_TO_POINTER(tmpi));
		tmpi = ceil(upper) + 1.0;
		g_object_set_data(object,"upper_limit", GINT_TO_POINTER(tmpi));

	}
}


void read_log_data(GIOChannel *iochannel, void *ptr)
{
	GString *a_line = g_string_new("\0");
	//GIOStatus  status = G_IO_STATUS_ERROR;
	struct Log_Info *log_info = ptr;
	gchar **data = NULL;
	gint i = 0;
	GArray *tmp_array = NULL;
	gfloat val = 0.0;
	GObject *object = NULL;

	while(g_io_channel_read_line_string(iochannel,a_line,NULL,NULL) == G_IO_STATUS_NORMAL) 
	{
		if (g_strrstr(a_line->str,"MARK"))
			continue;
		data = g_strsplit(a_line->str,log_info->delimiter,0);
		for (i=0;i<(log_info->field_count);i++)
		{
			object = g_array_index(log_info->log_list,GObject *, i);
			tmp_array = (GArray *)g_object_get_data(object,"data_array");
			val = (gfloat)g_ascii_strtod(g_strdup(data[i]),NULL);
			g_array_append_val(tmp_array,val);

			//printf("data[%i]=%s\n",i,data[i]);
		}
		g_strfreev(data);
	}
}

/*!
 * \brief frees data allocated by a datalog import, should be done when
 * switching logfiles
 */
void free_log_info()
{
	extern struct Log_Info *log_info;
	gint i = 0;
	GObject *object = NULL;
	GArray *array = NULL;
	
	if (!log_info)
		return;

	for (i=0;i<log_info->field_count;i++)
	{
		object = NULL;
		object = g_array_index(log_info->log_list,GObject *,i);
		if (!object)
			continue;
		array = (GArray *)g_object_get_data(object,"data_array");
		if (array)
			g_array_free(array,TRUE);
	}
	if (log_info->delimiter)
		g_free(log_info->delimiter);
	g_free(log_info);
	log_info = NULL;

	return;
}
