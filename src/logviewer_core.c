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


/*! 
 \brief load_logviewer_file() loads a datalog file for playback
 \param ptr (void *) pointer to an Io_File
 */
void load_logviewer_file(struct Io_File *iofile)
{
	if (!iofile)
	{
		dbg_func(__FILE__": load_logviewer_file()\n\tIo_File pointer NULL,returning!!\n",CRITICAL);
		return;
	}
	log_info = initialize_log_info();
	read_log_header(iofile->iochannel, log_info);
	read_log_data(iofile->iochannel, log_info);
	populate_limits(log_info);
	close_file(iofile);
	return;
}


/*!
 \brielf initialixe_log_info() alocates, and sets to sane defaults the fields
 of the log_info struture
 \returns a pointer to an allocated Log_Info structure
 */
struct Log_Info * initialize_log_info(void)
{
	struct Log_Info *log_info = NULL;
	log_info = g_malloc0(sizeof(struct Log_Info));
	log_info->field_count = 0;
	log_info->delimiter = NULL;
	log_info->log_list = g_array_new(FALSE,FALSE,sizeof(GObject *));
	return log_info;
}

/*!
 \brief read_log_header() First we read the first line,  try to determine 
 if the delimiter is a COMMA, TAB or a SPACE. If we detect SPACE delimiting,  
 we need to read one dataline because the variable names can be space 
 delimited and we need to figure out how to discern thespace inthe name with the space in between variable names
 \param iochannel (GIOChannel *) iochannel that represents the input file
 \param ptr (void *) pointer to the Log_Info structure
 */
void read_log_header(GIOChannel *iochannel, struct Log_Info *log_info )
{
	GString *a_line = g_string_new("\0");
	GIOStatus  status = G_IO_STATUS_ERROR;
	gchar *delimiter = NULL;
	gchar **fields = NULL;
	gint num_fields = 0;
	GArray *array = NULL;
	GObject *object = NULL;
	gint i = 0;
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


/*!
 \brief populate_limits() scans the datalog data and sets the minimum and 
 maximum values based on the span of the data in the file
 \param log_info (struct Log_Info *) pointer to log info structure
 */
void populate_limits(struct Log_Info *log_info)
{
	gint i = 0;
	gint j = 0;
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


/*! 
 \brief read_log_data() reads the log data and sticks it into the arrays in
 the log_info structure
 \param iochannel (GIOChannel *) data source 
 \param log_info (struct Log_Info *) pointer to log information struct
 */
void read_log_data(GIOChannel *iochannel, struct Log_Info *log_info)
{
	GString *a_line = g_string_new("\0");
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
 \brief free_log_info frees the data allocated by a datalog import, 
 should be done when switching logfiles
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
