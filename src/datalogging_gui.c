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
  \file src/datalogging_gui.c
  \ingroup CoreMtx
  \brief All functions specific to the datalogging abilities of Megatunix
 
  Megatunix is ALWAYS recording data once it connects to the ECU, this log
  can be flushed/written as needed as includes ALL FIELDS, in addition the user
  can select a custom datalog of the default fields, all fields or his own
  specific fields and log that.  The third type is CLI specified autolog dump
  which takes the internal auto-recording log and dumps that to a file 
  periodically,  the filename and path are specified as CLI options at 
  megatunix startup
  \author David Andruczyk
  */

#include <args.h>
#include <datalogging_gui.h>
#include <debugging.h>
#include <getfiles.h>
#include <gui_handlers.h>
#include <listmgmt.h>
#include <math.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <widgetmgmt.h>

/* External global vars */
extern gconstpointer *global_data;

/* Static vars to all functions in this file... */
static GList *object_list = NULL;
static gint last_len = 0;


/*!
  \brief populate_dlog_choices_pf() is called when the datalogging tab is loaded
  by glade AFTER the realtime variable definitions have been loaded and 
  processed.  All of the logable variables are then placed here for user 
  selecting during datalogging.
  */
G_MODULE_EXPORT void populate_dlog_choices(void)
{
	guint i,j,k;
	GList *list = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *table = NULL;
	GtkWidget *button = NULL;
	GtkWidget *label = NULL;
	gint table_rows = 0;
	gconstpointer * object = NULL;
	gchar * dlog_name = NULL;
	gchar * tooltip = NULL;
	Rtv_Map *rtv_map = NULL;

	ENTER();

	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return;
	}
	if (!((DATA_GET(global_data,"connected")) && 
				(DATA_GET(global_data,"interrogated"))))
	{
		EXIT();
		return;
	}
	if (!DATA_GET(global_data,"rtvars_loaded"))
	{
		MTXDBG(CRITICAL,_("CRITICAL ERROR, Realtime Variable definitions are NOT LOADED!!!\n"));
		EXIT();
		return;
	}
	set_title(g_strdup(_("Populating Datalogger...")));

	vbox = lookup_widget("dlog_logable_vars_vbox1");
	if (!GTK_IS_WIDGET(vbox))
	{
		printf(_("datalogger windows not present, returning\n"));
		EXIT();
		return;
	}
	table_rows = ceil((float)rtv_map->derived_total/(float)TABLE_COLS);
	table = gtk_table_new(table_rows,TABLE_COLS,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),0);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox),table,TRUE,TRUE,0);

	/* Update status of the delimiter buttons... */

	switch ((GINT)DATA_GET(global_data,"preferred_delimiter"))
	{
		case COMMA:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget("dlog_comma_delimit_radio_button")),TRUE);
			gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(lookup_widget("dlog_comma_delimit_radio_button")));
			break;
		case TAB:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget("dlog_tab_delimit_radio_button")),TRUE);
			gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(lookup_widget("dlog_tab_delimit_radio_button")));
			break;
		default:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget("dlog_comma_delimit_radio_button")),TRUE);
			gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(lookup_widget("dlog_comma_delimit_radio_button")));
			break;

	}
	j = 0;	
	k = 0;
	/* Put into GList and sort it */
	for (i=0;i<rtv_map->derived_total;i++)
		list = g_list_prepend(list,(gpointer)g_ptr_array_index(rtv_map->rtv_list,i));
	list = g_list_sort_with_data(list,list_object_sort,(gpointer)"dlog_gui_name");

	for (i=0;i<rtv_map->derived_total;i++)
	{
		tooltip = NULL;
		dlog_name = NULL;
		//object = g_ptr_array_index(rtv_map->rtv_list,i);
		object = (gconstpointer *)g_list_nth_data(list,i);
		dlog_name = (gchar *)DATA_GET(object,"dlog_gui_name");
		button = gtk_check_button_new();
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),dlog_name);
		gtk_container_add(GTK_CONTAINER(button),label);
		tooltip = (gchar *)(DATA_GET(object,"tooltip"));
		if (tooltip)
			gtk_widget_set_tooltip_text(button,tooltip);
		/* Bind button to the object, Done so that we can set the state
		 * of the buttons from elsewhere... 
		 */
		DATA_SET(object,"dlog_button",(gpointer)button);

		/* Bind object to the button */
		OBJ_SET(button,"object",(gpointer)object);

		g_signal_connect(G_OBJECT(button),"toggled",
				G_CALLBACK(log_value_set),
				NULL);
		if ((GINT)DATA_GET(object,"log_by_default") == 1)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		gtk_table_attach (GTK_TABLE (table), button, j, j+1, k, k+1,
				(GtkAttachOptions) (GTK_EXPAND|GTK_FILL|GTK_SHRINK),
				(GtkAttachOptions) (GTK_FILL|GTK_SHRINK),
				//(GtkAttachOptions) (GTK_FILL|GTK_SHRINK),
				//(GtkAttachOptions) (GTK_FILL|GTK_SHRINK),
				0, 0);
		j++;

		if (j == TABLE_COLS)
		{
			k++;
			j = 0;
		} 
	}
	g_list_free(list);
	gtk_widget_show_all(vbox);
	set_title(g_strdup(_("Datalogger Ready...")));
	EXIT();
	return;
}


/*!
  \brief start_datalogging() enables logging and if RT vars aren't running it
  starts them.
  */
G_MODULE_EXPORT void start_datalogging(void)
{
	ENTER();

	if (DATA_GET(global_data,"logging_active"))
	{
		EXIT();
		return;   /* Logging already running ... */
	}
	if (DATA_GET(global_data,"offline"))
	{
		EXIT();
		return;
	}
	if (lookup_widget("dlog_logable_vars_vbox1"))
		gtk_widget_set_sensitive(lookup_widget("dlog_logable_vars_vbox1"),FALSE);
	if (lookup_widget("dlog_format_delimit_hbox1"))
		gtk_widget_set_sensitive(lookup_widget("dlog_format_delimit_hbox1"),FALSE);
	if (lookup_widget("dlog_select_log_button"))
		gtk_widget_set_sensitive(lookup_widget("dlog_select_log_button"),FALSE);

	DATA_SET(global_data,"datalogging_header_needed",GINT_TO_POINTER(TRUE));
	DATA_SET(global_data,"logging_active",GINT_TO_POINTER(TRUE));
	update_logbar("dlog_view",NULL,_("DataLogging Started...\n"),FALSE,FALSE,FALSE);

	if (!DATA_GET(global_data,"offline"))
		start_tickler(RTV_TICKLER);
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	EXIT();
	return;
}


/*!
  \brief stop_datalogging() stops the datalog process. It DOES not stop realtime
  variable readback though
  */
G_MODULE_EXPORT void stop_datalogging(void)
{
	ENTER();

	GIOChannel *iochannel = NULL;
	if (!DATA_GET(global_data,"logging_active"))
	{
		EXIT();
		return;
	}

	if (DATA_GET(global_data,"offline"))
	{
		EXIT();
		return;
	}
	DATA_SET(global_data,"logging_active",NULL);

	if (lookup_widget("dlog_logable_vars_vbox1"))
		gtk_widget_set_sensitive(lookup_widget("dlog_logable_vars_vbox1"),TRUE);
	if (lookup_widget("dlog_format_delimit_hbox1"))
		gtk_widget_set_sensitive(lookup_widget("dlog_format_delimit_hbox1"),TRUE);
	if (lookup_widget("dlog_select_log_button"))
		gtk_widget_set_sensitive(lookup_widget("dlog_select_log_button"),TRUE);
	if (lookup_widget("dlog_stop_logging_button"))
		gtk_widget_set_sensitive(lookup_widget("dlog_stop_logging_button"),FALSE);
	if (lookup_widget("dlog_start_logging_button"))
		gtk_widget_set_sensitive(lookup_widget("dlog_start_logging_button"),FALSE);
	gtk_label_set_text(GTK_LABEL(lookup_widget("dlog_file_label")),"No Log Selected Yet");


	update_logbar("dlog_view",NULL,_("DataLogging Stopped...\n"),FALSE,FALSE,FALSE);
	iochannel = (GIOChannel *) OBJ_GET(lookup_widget("dlog_select_log_button"),"data");
	if (iochannel)
	{
		g_io_channel_shutdown(iochannel,TRUE,NULL);
		g_io_channel_unref(iochannel);
	}

	g_list_free(object_list);
	object_list = NULL;
	last_len = 0;
	OBJ_SET(lookup_widget("dlog_select_log_button"),"data",NULL);

	EXIT();
	return;
}



/*! 
  \brief log_value_set() gets called when a variable is selected for 
  logging so that it can be marked as being logged
  \param widget is the pointer to toggle button widget
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean log_value_set(GtkWidget * widget, gpointer data)
{
	gconstpointer *object = NULL;
	gboolean state = FALSE;

	ENTER();

	state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widget));
                
	/* get object from widget */
	object = (gconstpointer *)OBJ_GET(widget,"object");
	DATA_SET(object,"being_logged",GINT_TO_POINTER(state));

	EXIT();
	return TRUE;
}


/*!
  \brief write_log_header() writes the top line of the datalog with field names
  \param iochannel is the pointer to the datalog output channel 
  \param override if true ALL variables are logged, if FALSE
  only selected variabels are logged
  */
G_MODULE_EXPORT void write_log_header(GIOChannel *iochannel, gboolean override)
{
	GList *list = NULL;
	guint i = 0;
	guint j = 0;
	guint total_logables = 0;
	gsize count = 0;
	GString *output;
	gconstpointer * object = NULL;
	gchar * string = NULL;
	Firmware_Details *firmware = NULL;
	Rtv_Map *rtv_map = NULL;

	ENTER();

	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!iochannel)
	{
		MTXDBG(CRITICAL,_("IOChannel pointer was undefined, returning NOW...\n"));
		EXIT();
		return;
	}
	/* Count total logable variables */
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list,i);
		if((override) || ((GBOOLEAN)DATA_GET(object,"being_logged")))
			list = g_list_prepend(list,object);
	}
	list = g_list_reverse(list);
	output = g_string_sized_new(64); /* pre-allccate for 64 chars */

	string = g_strdup_printf("\"%s\"\r\n",firmware->actual_signature);
	output = g_string_append(output,string); 
	g_free(string);
	total_logables = g_list_length(list);
	for (i=0;i<total_logables;i++)
	{
		object = (gconstpointer *)g_list_nth_data(list,i);
		/* If space delimited, QUOTE the header names */
		string = (gchar *)DATA_GET(object,"dlog_field_name");
		output = g_string_append(output,string); 

		j++;
		if (j < total_logables)
			output = g_string_append(output,(const gchar *)DATA_GET(global_data,"delimiter"));
	}
	output = g_string_append(output,"\r\n");
	g_io_channel_write_chars(iochannel,output->str,output->len,&count,NULL);
	g_string_free(output,TRUE);

}


/*!
  \brief run_datalog() gets called periodically to log whatever is currently selected
  to the datalog file
  */
G_MODULE_EXPORT gboolean run_datalog(void)
{
	guint i = 0;
	guint j = 0;
	guint k = 0;
	gint base = 0;
	gsize count = 0;
	gint cur_len = 0;
	guint total_logables = 0;
	GString *output;
	GIOChannel *iochannel = NULL;
	gconstpointer *object = NULL;
	gfloat value = 0.0;
	GArray *history = NULL;
	GArray *array = NULL;
	gint precision = 0;
	gchar *tmpbuf = NULL;
	Rtv_Map *rtv_map = NULL;

	ENTER();

	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	if (!((DATA_GET(global_data,"connected")) && (DATA_GET(global_data,"interrogated"))))
	{
		EXIT();
		return FALSE;
	}

	if (!DATA_GET(global_data,"logging_active")) /* Logging isn't enabled.... */
	{
		EXIT();
		return FALSE;
	}

	iochannel = (GIOChannel *) OBJ_GET(lookup_widget("dlog_select_log_button"),"data");
	if (!iochannel)
	{
		MTXDBG(CRITICAL,_("IO Channel undefined, returning NOW!!!\n"));
		EXIT();
		return FALSE;
	}

	if (!object_list)
	{
		for (i=0;i<rtv_map->derived_total;i++)
		{
			object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list,i);
			if((GBOOLEAN)DATA_GET(object,"being_logged"))
				object_list = g_list_prepend(object_list,object);
		}
		object_list = g_list_reverse(object_list);
	}

	output = g_string_sized_new(1024); /* 1024 char initial size */

	if (DATA_GET(global_data,"datalogging_header_needed"))
	{
		write_log_header(iochannel, FALSE);
		DATA_SET(global_data,"datalogging_header_needed",NULL);
		DATA_SET(global_data,"begin",GINT_TO_POINTER(TRUE));
	}
	
	/* Get current array length */
	array = (GArray *)DATA_GET(g_list_nth_data(object_list,0),"history");
	cur_len = array->len;
	if (cur_len > last_len)
	{
		base = last_len;
		count = cur_len-last_len;
	}
	else	/* Arrays have been reset! */
	{
		base = 0;
		count = cur_len;
	}
	total_logables = g_list_length(object_list);
	for(j=base;j<base+count;j++)
	{
		k = 0;
		for(i=0;i<total_logables;i++)
		{
			object = (gconstpointer *)g_list_nth_data(object_list,i);
			history = (GArray *)DATA_GET(object,"history");
			precision = (GINT)DATA_GET(object,"precision");
			if ((GINT)j <= 0)
				value = 0.0;
			else
				value = g_array_index(history, gfloat, j);

			tmpbuf = g_strdelimit(g_strdup_printf("%1$.*2$f",value,precision),",",'.');
			g_string_append(output,tmpbuf);
			g_free(tmpbuf);
			k++;

			/* Print delimiter to log here so there isnt an extra
			 * char at the end fo the line 
			 */
			if (k < total_logables)
				output = g_string_append(output,(const gchar *)DATA_GET(global_data,"delimiter"));
		}
		output = g_string_append(output,"\r\n");
	}
	last_len = cur_len;
	g_io_channel_write_chars(iochannel,output->str,output->len,&count,NULL);
	g_string_free(output,TRUE);
	EXIT();
	return FALSE;
}


/*!
  \brief dlog_select_all() selects all variables for logging
  */
G_MODULE_EXPORT void dlog_select_all(void)
{
	guint i = 0;
	gconstpointer * object = NULL;
	GtkWidget *button = NULL;
	Rtv_Map *rtv_map = NULL;

	ENTER();

	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	/* Check all logable choices */
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = NULL;
		button = NULL;
		object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list, i);
		button = (GtkWidget *)DATA_GET(object,"dlog_button");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
	}
}


/*!
  \brief dlog_deselect_all() resets the logged choices to having NONE selected
  */
G_MODULE_EXPORT void dlog_deselect_all(void)
{
	guint i = 0;
	GtkWidget * button = NULL;
	gconstpointer * object = NULL;
	Rtv_Map *rtv_map = NULL;

	ENTER();

	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	/* Uncheck all logable choices */
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = NULL;
		button = NULL;
		object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list, i);
		button = (GtkWidget *)DATA_GET(object,"dlog_button");
		if (GTK_IS_TOGGLE_BUTTON(button))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),FALSE);
		/*
		else
		{
			printf("bad object!\n");
			g_dataset_foreach(&object,dump_datalist,NULL);
		}
		*/
	}
}

/*!
  \brief dlog_select_defaults() resets the logged choices to the ones defined 
  in the RealtimeMap file
  */
G_MODULE_EXPORT void dlog_select_defaults(void)
{
	guint i = 0;
	GtkWidget * button = NULL;
	gconstpointer * object = NULL;
	gboolean state=FALSE;
	Rtv_Map *rtv_map = NULL;

	ENTER();

	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	/* Uncheck all logable choices */
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = NULL;
		button = NULL;
		state = FALSE;
		object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list, i);
		button = (GtkWidget *)DATA_GET(object,"dlog_button");
		state = (GINT)DATA_GET(object,"log_by_default");
		if (state == -1)
			state = 0;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),state);
	}
}


/*!
  \brief Selects a datalog to export
  \param widget is unused
  \param data is unused
  \returns FALSE on problem, TRUE otherwise
  */
G_MODULE_EXPORT gboolean select_datalog_for_export(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	t = (time_t *)g_malloc(sizeof(time_t));
	time(t);
	tm = localtime(t);
	g_free(t);

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup(DATALOG_DATA_DIR);
	fileio->project = (const gchar *)DATA_GET(global_data,"project_name");
	fileio->title = g_strdup("Choose a filename for datalog export");
	fileio->parent = lookup_widget("main_window");
	fileio->on_top =TRUE;
	fileio->default_filename = g_strdup_printf("%s-%.4i_%.2i_%.2i-%.2i%.2i.log",g_strdelimit(firmware->name," ,",'_'),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min);
	fileio->default_extension= g_strdup("log");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("dlog_view","warning",_("NO FILE opened for normal datalogging!\n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "a+",NULL);
	if (!iochannel)
	{
		update_logbar("dlog_view","warning",_("File open FAILURE!\n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}

	gtk_widget_set_sensitive(lookup_widget("dlog_stop_logging_button"),TRUE);
	gtk_widget_set_sensitive(lookup_widget("dlog_start_logging_button"),TRUE);
	OBJ_SET(lookup_widget("dlog_select_log_button"),"data",(gpointer)iochannel);
	gtk_label_set_text(GTK_LABEL(lookup_widget("dlog_file_label")),g_filename_to_utf8(filename,-1,NULL,NULL,NULL));
	update_logbar("dlog_view",NULL,_("DataLog File Opened\n"),FALSE,FALSE,FALSE);

	free_mtxfileio(fileio);
	EXIT();
	return TRUE;
}


/*!
  \brief dumps the automatic datalog blob to disk on its periodic basis
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean autolog_dump(gpointer data)
{
	CmdLineArgs *args = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	gchar * tmpbuf = NULL;
	static gint dlog_index = 0;

	ENTER();

	args = (CmdLineArgs *)DATA_GET(global_data,"args");

	filename = g_strdup_printf("%s%s%s_%.3i.log",args->autolog_dump_dir,PSEP,args->autolog_basename,dlog_index);
		
	iochannel = g_io_channel_new_file(filename, "a+",NULL);
	dump_log_to_disk(iochannel);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	dlog_index++;
	tmpbuf = g_strdup_printf(_("Autolog dump (log number %i) successfully completed.\n"),dlog_index);
	thread_update_logbar("dlog_view",NULL,tmpbuf,FALSE,FALSE);
	g_free(filename);
	EXIT();
	return TRUE;
}


/*!
  \brief dumps the internally running datalog to disk
  \param widget is unused
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean internal_datalog_dump(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	t = (time_t *)g_malloc(sizeof(time_t));
	time(t);
	tm = localtime(t);
	g_free(t);

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup(DATALOG_DATA_DIR);
	fileio->project = (const gchar *)DATA_GET(global_data,"project_name");
	fileio->title = g_strdup("Choose a filename for internal datalog export");
	fileio->parent = lookup_widget("main_window");
	fileio->on_top =TRUE;
	fileio->default_filename = g_strdup_printf("%s-%.4i_%.2i_%.2i-%.2i%.2i.log",g_strdelimit(firmware->name," ,",'_'),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min);
	fileio->default_extension= g_strdup("log");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("dlog_view","warning",_("NO FILE opened for internal log export, aborting dump!\n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "a+",NULL);
	if (iochannel)
		update_logbar("dlog_view",NULL,_("File opened successfully for internal log dump\n"),FALSE,FALSE,FALSE);
	dump_log_to_disk(iochannel);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	update_logbar("dlog_view",NULL,_("Internal datalog successfully dumped to disk\n"),FALSE,FALSE,FALSE);
	free_mtxfileio(fileio);
	g_free(filename);
	EXIT();
	return TRUE;

}

/*!
  \brief dump_log_to_disk() dumps the contents of the RTV arrays to disk as a
  datalog file
  \param iochannel is the pointer to io channel for the log stream
  */
G_MODULE_EXPORT void dump_log_to_disk(GIOChannel *iochannel)
{
	guint i = 0;
	guint x = 0;
	guint j = 0;
	guint notif_divisor = 128;
	gboolean notifies = FALSE;
	gsize count = 0;
	GString *output;
	gconstpointer * object = NULL;
	GArray **histories = NULL;
	gint *precisions = NULL;
	gchar *tmpbuf = NULL;
	gchar *msg = NULL;
	gfloat value = 0.0;
	gboolean restart_tickler = FALSE;
	GtkWidget * info_label = NULL;
	Rtv_Map *rtv_map = NULL;

	ENTER();

	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	if (DATA_GET(global_data,"realtime_id"))
	{
		restart_tickler = TRUE;
		stop_tickler(RTV_TICKLER);
	}

	output = g_string_sized_new(4096); 

	write_log_header(iochannel, TRUE);

	histories = g_new0(GArray *,rtv_map->derived_total);
	precisions = g_new0(gint ,rtv_map->derived_total);
	info_label = lookup_widget("info_label");
	if (GTK_IS_LABEL(info_label))
		notifies = TRUE;

	for(i=0;i<rtv_map->derived_total;i++)
	{
		object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list,i);
		histories[i] = (GArray *)DATA_GET(object,"history");
		precisions[i] = (GINT)DATA_GET(object,"precision");
	}

	for (x=0;x<rtv_map->ts_array->len;x++)
	{
		j = 0;
		for(i=0;i<rtv_map->derived_total;i++)
		{
			value = g_array_index(histories[i], gfloat, x);
			/*tmpbuf = g_ascii_formatd(buf,G_ASCII_DTOSTR_BUF_SIZE,"%1$.*2$f",value,precisions[i]);*/
			tmpbuf = g_strdelimit(g_strdup_printf("%1$.*2$f",value,precisions[i]),",",'.');
			g_string_append(output,tmpbuf);
			g_free(tmpbuf);
			j++;

			/* Print delimiter to log here so there isnt an extra
			 * char at the end fo the line 
			 */
			if (j < rtv_map->derived_total)
				output = g_string_append(output,(const gchar *)DATA_GET(global_data,"delimiter"));
		}
		output = g_string_append(output,"\r\n");
		if (notifies && ((x % notif_divisor) == 0))
		{
			msg = g_strdup_printf(_("Flushing Datalog %i of %i records"),x,rtv_map->ts_array->len);
			gtk_label_set_text(GTK_LABEL(info_label),msg);
			g_free(msg);
		}
	}
	g_usleep(100000);
	if (notifies)
		gtk_label_set_text(GTK_LABEL(info_label),"Datalog buffer written to disk");
	g_io_channel_write_chars(iochannel,output->str,output->len,&count,NULL);
	g_free(precisions);
	g_free(histories);
	g_string_free(output,TRUE);
	if (restart_tickler)
		start_tickler(RTV_TICKLER);

}
