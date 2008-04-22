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
#include <getfiles.h>
#include <gui_handlers.h>
#include <keyparser.h>
#include <logviewer_core.h>
#include <logviewer_gui.h>
#include <math.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <string.h>
#include <tabloader.h>
#include <timeout_handlers.h>

Log_Info *log_info = NULL;
extern gint dbg_lvl;
extern GObject *global_data;



/*! 
 \brief select_datalog_for_import() loads a datalog file for playback
 \param widget Calling widget
 \param data  unused
 */

EXPORT gboolean select_datalog_for_import(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	extern GtkWidget *main_window;
	extern GHashTable *dynamic_widgets;

	reset_logviewer_state();
	free_log_info();


	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_Datalogs");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Choose a datalog to view");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("dlog_view",g_strdup("warning"),g_strdup("NO FILE opened for logviewing!\n"),FALSE,FALSE);
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "r+",NULL);
	if (!iochannel)
	{
		update_logbar("dlog_view",g_strdup("warning"),g_strdup("File open FAILURE! \n"),FALSE,FALSE);
		return FALSE;
	}

	update_logbar("dlog_view",NULL,g_strdup("DataLog ViewFile Opened\n"),FALSE,FALSE);
	load_logviewer_file(iochannel);
	g_io_channel_shutdown(iochannel,FALSE,NULL);
	g_io_channel_unref(iochannel);

	update_logbar("dlog_view",NULL,g_strdup("LogView File Closed\n"),FALSE,FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_controls_hbox"),TRUE);
	free_mtxfileio(fileio);
	return TRUE;
}


/*! 
 \brief load_logviewer_file() loads a datalog file for playback
 \param iochannel The IO channel representing the source file
 */
void load_logviewer_file(GIOChannel *iochannel)
{
	if (!iochannel)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": load_logviewer_file()\n\tIo_File pointer NULL,returning!!\n"));
		return;
	}
	log_info = initialize_log_info();
	read_log_header(iochannel, log_info);
	read_log_data(iochannel, log_info);
	populate_limits(log_info);
	return;
}


/*!
 \brief initialixe_log_info() alocates, and sets to sane defaults the fields
 of the log_info struture
 \returns a pointer to an allocated Log_Info structure
 */
Log_Info * initialize_log_info(void)
{
	Log_Info *log_info = NULL;
	log_info = g_malloc0(sizeof(Log_Info));
	log_info->field_count = 0;
	log_info->delimiter = NULL;
	log_info->signature = NULL;
	log_info->log_list = g_array_new(FALSE,FALSE,sizeof(GObject *));
	return log_info;
}

/*!
 \brief read_log_header() First we read the first line,  try to determine 
 if the delimiter is a COMMA, or a TAB. 
 \param iochannel (GIOChannel *) iochannel that represents the input file
 \param log_info (Log_Info *)the Log_Info structure
 */
void read_log_header(GIOChannel *iochannel, Log_Info *log_info )
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
	extern gboolean offline;
	extern Rtv_Map *rtv_map;

read_again:
	status = g_io_channel_read_line_string(iochannel,a_line,NULL,NULL); 

	if (status == G_IO_STATUS_NORMAL) /* good read */
	{
		/* This searchjed for a quoted string which should be the 
		 * ecu signature.  pre 0.9.15 versions of megatunix shoved the
		 * internal name ofthe firmware in there which is a problem as
		 * it makes the logs locked to megatunix which is a bad thing 
		 * as it hurts interoperability.  0.9.16+ changes this to use 
		 * the REAL signature returned by the firmware. 
		 */
		if (g_strrstr(a_line->str,"\"") != NULL)
		{
			log_info->signature = g_strdup(g_strstrip(g_strdelimit(a_line->str,"\"\n\r",' ')));
			printf("LOG signature is \"%s\"\n",log_info->signature);
			if (offline)
			{
				printf("rtv_map->applicable_signatures is \"%s\"\n",rtv_map->applicable_signatures);
				if (strstr(rtv_map->applicable_signatures,log_info->signature) != NULL)
					printf("Good this firmware is compatible with the firmware we're using\n");
				else
					printf("mismatch between datalog and current firmware\n");
			}
			goto read_again;
		}

		if (g_strrstr(a_line->str,",") != NULL)
			delimiter = g_strdup(",");
		else if (g_strrstr(a_line->str,"\t") != NULL)
			delimiter = g_strdup("\t");

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
			g_object_ref(object);
			gtk_object_sink(GTK_OBJECT(object));
			array = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
			OBJ_SET(object,"data_array",(gpointer)array);
			g_free(OBJ_GET(object,"lview_name"));
			OBJ_SET(object,"lview_name",g_strdup(g_strstrip(fields[i])));
			g_array_append_val(log_info->log_list,object);
		}
		/* Enable parameter selection button */
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button"), TRUE);
		OBJ_SET(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea"),"log_info",(gpointer)log_info);

	}
	g_free(delimiter);

}


/*!
 \brief populate_limits() scans the datalog data and sets the minimum and 
 maximum values based on the span of the data in the file
 \param log_info (Log_Info *) pointer to log info structure
 */
void populate_limits(Log_Info *log_info)
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
		array = (GArray *)OBJ_GET(object,"data_array");
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
		OBJ_SET(object,"lower_limit", GINT_TO_POINTER(tmpi));
		tmpi = ceil(upper) + 1.0;
		OBJ_SET(object,"upper_limit", GINT_TO_POINTER(tmpi));

	}
}


/*! 
 \brief read_log_data() reads the log data and sticks it into the arrays in
 the log_info structure
 \param iochannel (GIOChannel *) data source 
 \param log_info (Log_Info *) pointer to log information struct
 */
void read_log_data(GIOChannel *iochannel, Log_Info *log_info)
{
	GString *a_line = g_string_new("\0");
	gchar **data = NULL;
	gint i = 0;
	gint x = 0;
	GArray *tmp_array = NULL;
	gfloat val = 0.0;
	GObject *object = NULL;

	while(g_io_channel_read_line_string(iochannel,a_line,NULL,NULL) == G_IO_STATUS_NORMAL) 
	{
		if (g_strrstr(a_line->str,"MARK"))
		{
			/* Should insert some sort of marker at this index
			 * in the data arrays... 
			 */
			continue;
		}
		data = g_strsplit(a_line->str,log_info->delimiter,0);
		if ((!g_ascii_isdigit(data[0][0])) && (!g_ascii_isdigit(data[0][1])))
		{
			printf("non numerical line detected\n");
			printf("text %s\n",data[0]);
			g_strfreev(data);
			continue;
		}
		if (g_strv_length(data) != log_info->field_count)
		{
			printf("Datalog error, field count assertion failure\nExpected %i fields, got %i instead\n",log_info->field_count,g_strv_length(data));
			g_strfreev(data);
			continue;
		}

		for (i=0;i<(log_info->field_count);i++)
		{
			object = g_array_index(log_info->log_list,GObject *, i);
			tmp_array = (GArray *)OBJ_GET(object,"data_array");
			val = (gfloat)g_ascii_strtod(data[i],NULL);
			g_array_append_val(tmp_array,val);

			/*printf("data[%i]=%s\n",i,data[i]);*/
			if (x == 0) /* only check fir first line */
			{
				if (g_strrstr(data[i], ".") == NULL)
					OBJ_SET(object,"is_float", GINT_TO_POINTER(FALSE));
				else
					OBJ_SET(object,"is_float", GINT_TO_POINTER(TRUE));
			}
		}
		g_strfreev(data);
		x++;
	}
}

/*!
 \brief free_log_info frees the data allocated by a datalog import, 
 should be done when switching logfiles
 */
void free_log_info()
{
	extern Log_Info *log_info;
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
		array = (GArray *)OBJ_GET(object,"data_array");
		if (array)
			g_array_free(array,TRUE);
	}
	if (log_info->delimiter)
		g_free(log_info->delimiter);
	g_free(log_info);
	log_info = NULL;

	return;
}


/*!
 *\brief changes scroll speed for logviewer playback
 */
EXPORT gboolean logviewer_scroll_speed_change(GtkWidget *widget, gpointer data)
{
	gfloat tmpf = 0.0;
	extern gint playback_id;

	tmpf = gtk_range_get_value(GTK_RANGE(widget));
	OBJ_SET(global_data,"lv_scroll_delay", GINT_TO_POINTER((gint)tmpf));
	if (playback_id > 0)
	{
		stop_tickler(LV_PLAYBACK_TICKLER);
		start_tickler(LV_PLAYBACK_TICKLER);
	}

	return TRUE;
}
