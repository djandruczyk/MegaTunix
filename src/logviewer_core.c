/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/logviewer_core.c
  \ingroup CoreMtx
  \brief The legacy MegaTunix Logviewer core functions
  This needs to be rewritten to use the new stripchart widgets
  \author David Andruczyk
  */

#include <debugging.h>
#include <getfiles.h>
#include <gui_handlers.h>
#include <keyparser.h>
#include <logviewer_gui.h>
#include <math.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <stdlib.h>
#include <string.h>
#include <stripchart.h>
#include <timeout_handlers.h>
#include <widgetmgmt.h>

extern gconstpointer *global_data;

/*!
  \brief Creates a stripchart widget (i.e. accel wizard)
  \param parent is the container for stripchart widget
  */
G_MODULE_EXPORT void create_stripchart(GtkWidget *parent)
{
	GtkWidget *chart = NULL;
	gchar ** sources = NULL;
	gchar * tmpbuf = NULL;
	guint i = 0;
	gconstpointer *object = NULL;
	gint min = 0;
	gint max = 0;
	gint precision = 0;
	gchar * name = NULL;
	DataSize size = MTX_U08;
	Rtv_Map *rtv_map;

	ENTER();
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	tmpbuf = (gchar *)OBJ_GET(parent,"sources");
	if (tmpbuf);
		sources = g_strsplit(tmpbuf,",",-1);
	chart = mtx_stripchart_new();
	gtk_container_add(GTK_CONTAINER(parent), chart);
	//gtk_widget_realize(chart);
	for (i=0;i<g_strv_length(sources);i++)
	{
		if (!rtv_map)
			continue;
		object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,sources[i]);
		if (!object)
			continue;
		if ((gchar *)DATA_GET(object,"dlog_gui_name"))
			name = (gchar *)DATA_GET(object,"dlog_gui_name");
		else
			name = g_strdup("undefined!\n");
		if (DATA_GET(object,"real_lower"))
			min = (GINT)strtol((gchar *)DATA_GET(object,"real_lower"),NULL,10);
		else
			min = get_extreme_from_size(size,LOWER);
		if (DATA_GET(object,"real_upper"))
			max = (GINT)strtol((gchar *)DATA_GET(object,"real_upper"),NULL,10);
		else
			max = get_extreme_from_size(size,UPPER);
		if (DATA_GET(object,"precision"))
			precision = (GINT)DATA_GET(object,"precision");
		else
			precision = 0;
		mtx_stripchart_add_trace(MTX_STRIPCHART(chart),(gfloat)min,(gfloat)max,precision,name,NULL);
	}
	create_rtv_multi_value_historical_watch(sources,FALSE,"update_stripchart_data",chart);
	g_strfreev(sources);
	gtk_widget_show_all(parent);
	EXIT();
	return;
}


/*!
  \brief updates a stripchart widget with new values
  \param watch is the pointer to the watch containing the new data
  */
G_MODULE_EXPORT void update_stripchart_data(RtvWatch* watch)
{
	ENTER();
	mtx_stripchart_set_n_values(MTX_STRIPCHART(watch->user_data),watch->count,watch->hist_vals);
	EXIT();
	return;
}


/*! 
  \brief select_datalog_for_import() loads a datalog file for playback
  \param widget is the Calling widget
  \param data is unused
  */
G_MODULE_EXPORT gboolean select_datalog_for_import(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;

	ENTER();
	reset_logviewer_state();
	free_log_info((Log_Info *)DATA_GET(global_data,"log_info"));

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup(DATALOG_DATA_DIR);
	fileio->project = (const gchar *)DATA_GET(global_data,"project_name");
	fileio->parent = lookup_widget("main_window");
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Choose a datalog to view");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("dlog_view","warning",_("NO FILE opened for logviewing!\n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "r+",NULL);
	if (!iochannel)
	{
		update_logbar("dlog_view","warning",_("File open FAILURE! \n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}

	update_logbar("dlog_view",NULL,_("DataLog ViewFile Opened\n"),FALSE,FALSE,FALSE);
	load_logviewer_file(iochannel);
	g_io_channel_shutdown(iochannel,FALSE,NULL);
	g_io_channel_unref(iochannel);

	update_logbar("dlog_view",NULL,_("LogView File Closed\n"),FALSE,FALSE,FALSE);
	gtk_widget_set_sensitive(lookup_widget("logviewer_controls_hbox"),TRUE);
	enable_playback_controls(TRUE);
	free_mtxfileio(fileio);
	EXIT();
	return TRUE;
}


/*! 
  \brief load_logviewer_file() loads a datalog file for playback
  \param iochannel is the The IO channel representing the source file
  */
G_MODULE_EXPORT void load_logviewer_file(GIOChannel *iochannel)
{
	Log_Info *log_info = NULL;
	ENTER();
	if (!iochannel)
	{
		MTXDBG(CRITICAL,_("IO Channel pointer is NULL, returning!!\n"));
		EXIT();
		return;
	}
	log_info = initialize_log_info();
	DATA_SET(global_data,"log_info",log_info);
	read_log_header(iochannel, log_info);
	read_log_data(iochannel, log_info);
	populate_limits(log_info);
	EXIT();
	return;
}


/*!
  \brief initialixe_log_info() alocates, and sets to sane defaults the fields
  of the log_info struture
  \returns a pointer to an allocated Log_Info structure
  */
G_MODULE_EXPORT Log_Info * initialize_log_info(void)
{
	Log_Info *log_info = NULL;
	ENTER();
	log_info = (Log_Info *)g_malloc0(sizeof(Log_Info));
	log_info->field_count = 0;
	log_info->delimiter = NULL;
	log_info->signature = NULL;
	log_info->log_list = g_ptr_array_new();
	EXIT();
	return log_info;
}

/*!
  \brief read_log_header() First we read the first line,  try to determine 
  if the delimiter is a COMMA, or a TAB. 
  \param iochannel is the iochannel that represents the input file
  \param log_info is the the Log_Info structure
  */
G_MODULE_EXPORT void read_log_header(GIOChannel *iochannel, Log_Info *log_info )
{
	GString *a_line = g_string_new("\0");
	GIOStatus  status = G_IO_STATUS_ERROR;
	gchar *delimiter = NULL;
	gchar **fields = NULL;
	gint num_fields = 0;
	GArray *array = NULL;
	gconstpointer *object = NULL;
	Rtv_Map *rtv_map;
	extern gconstpointer *global_data;
	
	ENTER();
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

read_again:
	status = g_io_channel_read_line_string(iochannel,a_line,NULL,NULL); 

	if (status == G_IO_STATUS_NORMAL) /* good read */
	{
		/* This searched for a quoted string which should be the 
		 * ecu signature.  pre 0.9.15 versions of megatunix shoved the
		 * internal name of the firmware in there which is a problem as
		 * it makes the logs locked to megatunix which is a bad thing 
		 * as it hurts interoperability.  0.9.16+ changes this to use 
		 * the REAL signature returned by the firmware. 
		 */
		if (g_strrstr(a_line->str,"\"") != NULL)
		{
			log_info->signature = g_strdup(g_strstrip(g_strdelimit(a_line->str,"\"\n\r",' ')));
			/*printf(_("LOG signature is \"%s\"\n"),log_info->signature);*/
			if (DATA_GET(global_data,"offline"))
			{
				printf("rtv_map->applicable_signatures is \"%s\"\n",rtv_map->applicable_signatures);
				if (rtv_map->applicable_signatures)
				{
					if (strstr(rtv_map->applicable_signatures,log_info->signature) != NULL)
						printf(_("Good this firmware is compatible with the firmware we're using\n"));
					else
						printf(_("mismatch between datalog and current firmware, playback via full gui will probably not work like you expected\n"));
				}
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
		 * that is NULL terminated thanks to g_strsplit(void)
		 */
		fields = parse_keys(a_line->str,&num_fields,delimiter);

		log_info->field_count = num_fields;
		/* Create objects, arrays and storage points... */
		for (gint i=0;i<num_fields;i++)
		{
			array = NULL;
			object = g_new0(gconstpointer, 1);
			array = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),4096);
			DATA_SET(object,"data_array",(gpointer)array);
			g_free(DATA_GET(object,"lview_name"));
			DATA_SET_FULL(object,"lview_name",g_strdup(g_strstrip(fields[i])),g_free);
			g_ptr_array_add(log_info->log_list,object);
		}
		/* Enable parameter selection button */
		gtk_widget_set_sensitive(lookup_widget("logviewer_select_params_button"), TRUE);
		OBJ_SET(lookup_widget("logviewer_trace_darea"),"log_info",(gpointer)log_info);

	}
	g_free(delimiter);
	EXIT();
	return;
}


/*!
  \brief populate_limits() scans the datalog data and sets the minimum and 
  maximum values based on the span of the data in the file
  \param log_info is the pointer to log info structure
  */
G_MODULE_EXPORT void populate_limits(Log_Info *log_info)
{
	guint i = 0;
	gint j = 0;
	gconstpointer * object = NULL;
	GArray *array = NULL;
	gfloat val = 0.0;
	gfloat lower = 0.0;
	gfloat upper = 0.0;
	gint tmpi = 0;
	gint len = 0;

	ENTER();
	for (i=0;i<log_info->field_count;i++)
	{
		object = NULL;
		array = NULL;
		lower = 0.0;
		upper = 0.0;
		tmpi = 0;
		len = 0;
		object = (gconstpointer *)g_ptr_array_index(log_info->log_list, i);
		array = (GArray *)DATA_GET(object,"data_array");
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
		DATA_SET_FULL(object,"real_lower", (gpointer)g_strdup_printf("%i",tmpi),g_free);
		tmpi = ceil(upper) + 1.0;
		DATA_SET_FULL(object,"real_upper", (gpointer)g_strdup_printf("%i",tmpi),g_free);

	}
	EXIT();
	return;
}


/*! 
  \brief read_log_data() reads the log data and sticks it into the arrays in
  the log_info structure
  \param iochannel is the data source 
  \param log_info is the pointer to log information struct
  */
G_MODULE_EXPORT void read_log_data(GIOChannel *iochannel, Log_Info *log_info)
{
	GString *a_line = g_string_new("\0");
	gchar **data = NULL;
	guint i = 0;
	gint x = 0;
	gint precision = 0;
	gchar ** vector = NULL;
	GArray *tmp_array = NULL;
	gfloat val = 0.0;
	gconstpointer *object = NULL;

	ENTER();
	while(g_io_channel_read_line_string(iochannel,a_line,NULL,NULL) == G_IO_STATUS_NORMAL) 
	{
		if (g_strrstr(a_line->str,"MARK"))
		{
			/* Should insert some sort of marker at this index
			 * in the data arrays... 
			 */
			printf(_("MARK found in logfile. MTX doesn't do anything with these yet...!\n"));
			continue;
		}
		data = g_strsplit(a_line->str,log_info->delimiter,0);
		if ((!g_ascii_isdigit(data[0][0])) && (!g_ascii_isdigit(data[0][1])))
		{
			printf(_("non numerical line detected\n"));
			printf("text %s\n",data[0]);
			g_strfreev(data);
			continue;
		}
		if (g_strv_length(data) != log_info->field_count)
		{
			printf(_("Datalog error, field count assertion failure\nExpected %i fields, got %i instead, tossing this record!\n"),log_info->field_count,g_strv_length(data));
			g_strfreev(data);
			continue;
		}

		for (i=0;i<(log_info->field_count);i++)
		{
			object = (gconstpointer *)g_ptr_array_index(log_info->log_list, i);
			tmp_array = (GArray *)DATA_GET(object,"data_array");
			val = (gfloat)g_ascii_strtod(g_strdelimit(data[i],",.",'.'),NULL);
			g_array_append_val(tmp_array,val);

			/*printf("data[%i]=%s\n",i,data[i]);*/
			if (x == 0) /* only check first line */
			{
				if (g_strrstr(data[i], ".") != NULL)
				{
					vector = g_strsplit(data[i],".",-1);
					precision = strlen(vector[1]);
					g_strfreev(vector);
					DATA_SET(object,"precision",GINT_TO_POINTER(precision));
				}
			}

		}
		g_strfreev(data);
		x++;
	}
	EXIT();
	return;
}

/*!
  \brief free_log_info frees the data allocated by a datalog import, 
  should be done when switching logfiles
  \param log_info is the pointer to log information structure
  */
G_MODULE_EXPORT void free_log_info(Log_Info *log_info)
{
	guint i = 0;
	gconstpointer *object = NULL;
	GArray *array = NULL;
	
	ENTER();
	if (!log_info)
	{
		EXIT();
		return;
	}

	for (i=0;i<log_info->field_count;i++)
	{
		object = NULL;
		object = (gconstpointer *)g_ptr_array_index(log_info->log_list,i);
		if (!object)
			continue;
		array = (GArray *)DATA_GET(object,"data_array");
		g_free(DATA_GET(object,"lview_name"));
		if (array)
			g_array_free(array,TRUE);
	}
	if (log_info->delimiter)
		g_free(log_info->delimiter);
	g_free(log_info);
	log_info = NULL;
	DATA_SET(global_data,"log_info",NULL);
	EXIT();
	return;
}


/*!
  \brief changes scroll speed for logviewer playback
  \param widget is the pointer to the range widget representing the scroll speed
  \param data is unused
  \returns TRUE to indicate the event is handled
  */
G_MODULE_EXPORT gboolean logviewer_scroll_speed_change(GtkWidget *widget, gpointer data)
{
	gfloat tmpf = 0.0;
	extern gconstpointer *global_data;

	ENTER();
	tmpf = gtk_range_get_value(GTK_RANGE(widget));
	DATA_SET(global_data,"lv_scroll_delay", GINT_TO_POINTER((GINT)tmpf));
	if (DATA_GET(global_data,"playback_id"))
	{
		stop_tickler(LV_PLAYBACK_TICKLER);
		start_tickler(LV_PLAYBACK_TICKLER);
	}
	EXIT();
	return TRUE;
}
