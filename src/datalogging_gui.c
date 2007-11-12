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
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <fileio.h>
#include <getfiles.h>
#include <glib.h>
#include <gui_handlers.h>
#include <math.h>
#include <notifications.h>
#include <structures.h>
#include <sys/types.h>
#include <timeout_handlers.h>
#include <unistd.h>


/* global vars (owned here...) */
gchar *delimiter;
gboolean begin = TRUE;

/* External global vars */
extern gint ready;
extern gint dbg_lvl;
extern Rtv_Map *rtv_map;

/* Static vars to all functions in this file... */
static gboolean logging_active = FALSE;
static gboolean header_needed = FALSE;


/*!
 \brief populate_dlog_choices() is called when the datalogging tab is loaded
 by glade AFTER the realtime variable definitions have been loaded and 
 processed.  All of the logable variables are then placed here to user 
 selecting during datalogging.
 */
void populate_dlog_choices()
{
	gint i,j,k;
	GtkWidget *vbox = NULL;
	GtkWidget *table = NULL;
	GtkWidget *button = NULL;
	GtkWidget *label = NULL;
	gint table_rows = 0;
	GObject * object = NULL;
	gchar * dlog_name = NULL;
	gchar * tooltip = NULL;
	extern GHashTable *dynamic_widgets;
	extern GtkTooltips *tip;
	extern gint preferred_delimiter;
	extern gboolean tabs_loaded;
	extern gboolean rtvars_loaded;
	extern volatile gboolean leaving;

	if ((!tabs_loaded) || (leaving))
		return;
	if (!rtvars_loaded)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": populate_dlog_choices()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n"));
		return;
	}

	vbox = g_hash_table_lookup(dynamic_widgets,"dlog_logable_vars_vbox1");
	table_rows = ceil((float)rtv_map->derived_total/(float)TABLE_COLS);
	table = gtk_table_new(table_rows,TABLE_COLS,TRUE);
	//	logables_table = table;
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,0);

	// Update status of the delimiter buttons...

	switch (preferred_delimiter)
	{
		case COMMA:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_hash_table_lookup(dynamic_widgets,"dlog_comma_delimit_radio_button")),TRUE);
			gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(g_hash_table_lookup(dynamic_widgets,"dlog_comma_delimit_radio_button")));
			break;
		case TAB:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_hash_table_lookup(dynamic_widgets,"dlog_tab_delimit_radio_button")),TRUE);
			gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(g_hash_table_lookup(dynamic_widgets,"dlog_tab_delimit_radio_button")));
			break;
		default:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_hash_table_lookup(dynamic_widgets,"dlog_comma_delimit_radio_button")),TRUE);
			gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(g_hash_table_lookup(dynamic_widgets,"dlog_comma_delimit_radio_button")));
			break;

	}
	j = 0;	
	k = 0;
	for (i=0;i<rtv_map->derived_total;i++)
	{
		tooltip = NULL;
		dlog_name = NULL;
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		dlog_name = g_object_get_data(object,"dlog_gui_name");
		button = gtk_check_button_new();
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),dlog_name);
		gtk_container_add(GTK_CONTAINER(button),label);
		tooltip = g_strdup(g_object_get_data(object,"tooltip"));
		if (tooltip)
			gtk_tooltips_set_tip(tip,button,tooltip,NULL);
		g_free(tooltip);

		// Bind button to the object, Done so that we can set the state
		// of the buttons from elsewhere... 
		g_object_set_data(G_OBJECT(object),"dlog_button",
				(gpointer)button);
		// Bind object to the button 
		g_object_set_data(G_OBJECT(button),"object",
				(gpointer)object);
		g_signal_connect(G_OBJECT(button),"toggled",
				G_CALLBACK(log_value_set),
				NULL);
		if ((gboolean)g_object_get_data(object,"log_by_default")==TRUE)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		gtk_table_attach (GTK_TABLE (table), button, j, j+1, k, k+1,
				(GtkAttachOptions) (GTK_FILL|GTK_SHRINK),
				(GtkAttachOptions) (GTK_FILL|GTK_SHRINK),
				0, 0);
		j++;

		if (j == 5)
		{
			k++;
			j = 0;
		} 
	}
	gtk_widget_show_all(vbox);
	return;
}


/*!
 \brief start_datalogging() enables logging and if RT vars aren't running it
 starts them.
 */
void start_datalogging(void)
{
	extern GHashTable *dynamic_widgets;
	extern volatile gboolean offline;
	extern gboolean forced_update;

	if (logging_active)
		return;   /* Logging already running ... */
	if (g_hash_table_lookup(dynamic_widgets,"dlog_logable_vars_vbox1"))
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_logable_vars_vbox1"),FALSE);
	if (g_hash_table_lookup(dynamic_widgets,"dlog_format_delimit_hbox1"))
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_format_delimit_hbox1"),FALSE);
	if (g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button"))
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button"),FALSE);

	header_needed = TRUE;
	logging_active = TRUE;
	update_logbar("dlog_view",NULL,g_strdup("DataLogging Started...\n"),TRUE,FALSE);

	if (!offline)
		start_tickler(RTV_TICKLER);
	forced_update=TRUE;
	return;
}


/*!
 \brief stop_datalogging() stops the datalog process. It DOES not stop realtime
 variable readback though
 */
void stop_datalogging()
{
	extern GHashTable *dynamic_widgets;
	GIOChannel *iochannel = NULL;
	if (!logging_active)
		return;

	logging_active = FALSE;

	if (g_hash_table_lookup(dynamic_widgets,"dlog_logable_vars_vbox1"))
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_logable_vars_vbox1"),TRUE);
	if (g_hash_table_lookup(dynamic_widgets,"dlog_format_delimit_hbox1"))
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_format_delimit_hbox1"),TRUE);
	if (g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button"))
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button"),TRUE);
	if (g_hash_table_lookup(dynamic_widgets,"dlog_stop_logging_button"))
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_stop_logging_button"),FALSE);
	if (g_hash_table_lookup(dynamic_widgets,"dlog_start_logging_button"))
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_start_logging_button"),FALSE);
	gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,"dlog_file_label")),"No Log Selected Yet");


	update_logbar("dlog_view",NULL,g_strdup("DataLogging Stopped...\n"),TRUE,FALSE);
	iochannel = (GIOChannel *) g_object_get_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button")),"data");
	if (iochannel)
		g_io_channel_shutdown(iochannel,TRUE,NULL);

	g_object_set_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button")),"data",NULL);

	return;
}



/*! 
 \brief log_value_set() gets called when a variable is selected for 
 logging so that it can be marked as being logged
 */
gboolean log_value_set(GtkWidget * widget, gpointer data)
{
	GObject *object = NULL;
	gboolean state = FALSE;

	state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widget));
                
	/* get object from widget */
	object = (GObject *)g_object_get_data(G_OBJECT(widget),"object");
	g_object_set_data(object,"being_logged",GINT_TO_POINTER(state));

	return TRUE;
}


/*!
 \brief write_log_header() writes the top line of the datalog with field names
 \param iofile (Io_File *) pointer to the datalog output file 
 \param override (gboolean),  if true ALL variabels are logged, if FALSE
 only selected variabels are logged
 */
void write_log_header(GIOChannel *iochannel, gboolean override)
{
	gint i = 0;
	gint j = 0;
	gint total_logables = 0;
	gsize count = 0;
	GString *output;
	GObject * object = NULL;
	gchar * string = NULL;
	extern gint preferred_delimiter;
	extern Firmware_Details *firmware;
	if (!iochannel)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": write_log_header()\n\tIOChannel pointer was undefined, returning NOW...\n"));
		return;
	}
	/* Count total logable variables */
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		if((override) || ((gboolean)g_object_get_data(object,"being_logged")))
			total_logables++;
	}
	output = g_string_sized_new(64); /* pre-allccate for 64 chars */

	string = g_strdup_printf("\"%s\"\r\n",firmware->name);
	output = g_string_append(output,string); 
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		if((override) || ((gboolean)g_object_get_data(object,"being_logged")))
		{
			/* If space delimited, QUOTE the header names */
			string = (gchar *)g_object_get_data(object,"dlog_field_name");
			output = g_string_append(output,string); 

			j++;
			if (j < total_logables)
				output = g_string_append(output,delimiter);
		}
	}
	output = g_string_append(output,"\r\n");
	g_io_channel_write_chars(iochannel,output->str,output->len,&count,NULL);
	g_string_free(output,TRUE);

}


/*!
 \brief run_datalog() gets called each time data arrives after rtvar 
 processing and logs the selected values to the file
 */
void run_datalog(void)
{
	gint i = 0;
	gint j = 0;
	gsize count = 0;
	gint total_logables = 0;
	GString *output;
	GIOChannel *iochannel = NULL;
	GObject *object = NULL;
	gfloat value = 0.0;
	GArray *history = NULL;
	gint current_index = 0;
	gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
	gchar *tmpbuf = NULL;
	extern GHashTable *dynamic_widgets;

	if (!logging_active) /* Logging isn't enabled.... */
		return;

	iochannel = (GIOChannel *) g_object_get_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button")),"data");
	if (!iochannel)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": run_datalog()\n\tIo_File undefined, returning NOW!!!\n"));
		return;
	}

	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		if((gboolean)g_object_get_data(object,"being_logged"))
			total_logables++;
	}

	output = g_string_sized_new(64); /*64 char initial size */

	if (header_needed)
	{
		write_log_header(iochannel, FALSE);
		header_needed = FALSE;
		begin = TRUE;
	}
	j = 0;
	for(i=0;i<rtv_map->derived_total;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		if (!((gboolean)g_object_get_data(object,"being_logged")))
			continue;

		history = (GArray *)g_object_get_data(object,"history");
		current_index = (gint)g_object_get_data(object,"current_index");
		value = g_array_index(history, gfloat, current_index);
		if ((gboolean)g_object_get_data(object,"is_float"))
		{
//			printf("value %.3f\n",value);
			tmpbuf = g_ascii_formatd(buf,G_ASCII_DTOSTR_BUF_SIZE,"%.3f",value);
			g_string_append(output,tmpbuf);
		}
		else
			g_string_append_printf(output,"%i",(gint)value);
		j++;

		/* Print delimiter to log here so there isnt an extra
		 * char at the end fo the line 
		 */
		if (j < total_logables)
			output = g_string_append(output,delimiter);
	}
	output = g_string_append(output,"\r\n");
	g_io_channel_write_chars(iochannel,output->str,output->len,&count,NULL);
	g_string_free(output,TRUE);

}


/*!
 \brief dlog_select_all() selects all variables for logging
 */
void dlog_select_all()
{
	gint i = 0;
	GObject * object = NULL;
	GtkWidget *button = NULL;

	/* Check all logable choices */
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = NULL;
		button = NULL;
		object = g_array_index(rtv_map->rtv_list,GObject *, i);
		button = (GtkWidget *)g_object_get_data(object,"dlog_button");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
	}
}


/*!
 \brief dlog_deselect_all() resets the logged choices to having NONE selected
 */
void dlog_deselect_all(void)
{
	gint i = 0;
	GtkWidget * button = NULL;
	GObject * object = NULL;

	/* Uncheck all logable choices */
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = NULL;
		button = NULL;
		object = g_array_index(rtv_map->rtv_list,GObject *, i);
		button = (GtkWidget *)g_object_get_data(object,"dlog_button");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),FALSE);
	}
}

/*!
 \brief dlog_select_defaults() resets the logged choices to the ones defined 
 in the RealtimeMap file
 */
void dlog_select_defaults(void)
{
	gint i = 0;
	GtkWidget * button = NULL;
	GObject * object = NULL;
	gboolean state=FALSE;

	/* Uncheck all logable choices */
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = NULL;
		button = NULL;
		state = FALSE;
		object = g_array_index(rtv_map->rtv_list,GObject *, i);
		button = (GtkWidget *)g_object_get_data(object,"dlog_button");
		state = (gboolean)g_object_get_data(object,"log_by_default");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),state);
	}
}


EXPORT gboolean select_datalog_for_export(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	extern GHashTable *dynamic_widgets;
	extern GtkWidget *main_window;

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_Datalogs");
	fileio->title = g_strdup("Choose a filename for datalog export");
	fileio->parent = main_window;
	fileio->on_top =TRUE;
	fileio->default_filename= g_strdup("Untitled.log");
	fileio->default_extension= g_strdup("log");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("dlog_view",g_strdup("warning"),g_strdup("NO FILE opened for normal datalogging!\n"),TRUE,FALSE);
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "a+",NULL);
	if (!iochannel)
	{
		update_logbar("dlog_view",g_strdup("warning"),g_strdup("File open FAILURE! \n"),TRUE,FALSE);
		return FALSE;
	}

	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_stop_logging_button"),TRUE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_start_logging_button"),TRUE);
	g_object_set_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button")),"data",(gpointer)iochannel);
	gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,"dlog_file_label")),g_filename_to_utf8(filename,-1,NULL,NULL,NULL));
	update_logbar("dlog_view",NULL,g_strdup("DataLog File Opened\n"),TRUE,FALSE);

	free_mtxfileio(fileio);
	return TRUE;
}



EXPORT gboolean internal_datalog_dump(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	extern GtkWidget *main_window;

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_Datalogs");
	fileio->title = g_strdup("Choose a filename for internal datalog export");
	fileio->parent = main_window;
	fileio->on_top =TRUE;
	fileio->default_filename= g_strdup("Untitled.log");
	fileio->default_extension= g_strdup("log");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("dlog_view",g_strdup("warning"),g_strdup("NO FILE opened for internal log export,  aborting dump!\n"),TRUE,FALSE);
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "a+",NULL);
	if (iochannel)
		update_logbar("dlog_view",NULL,g_strdup("File opened successfully for internal log dump\n"),TRUE,FALSE);
	dump_log_to_disk(iochannel);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	update_logbar("dlog_view",NULL,g_strdup("Internal datalog successfully dumped to disk\n"),TRUE,FALSE);
	free_mtxfileio(fileio);
	return TRUE;

}

/*!
 \brief dump_log_to_disk() dumps the contents of the RTV arrays to disk as a
 datalog file
 */
void dump_log_to_disk(GIOChannel *iochannel)
{
	gint i = 0;
	gint x = 0;
	gint j = 0;
	gsize count = 0;
	GString *output;
	GObject * object = NULL;
	GArray **histories;
	gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
	gchar *tmpbuf = NULL;
	gboolean *is_floats;
	gfloat value = 0.0;
	extern gint realtime_id;
	gboolean restart_tickler = FALSE;

	if (realtime_id)
	{
		restart_tickler = TRUE;
		stop_tickler(RTV_TICKLER);
	}

	output = g_string_sized_new(1024); 

	write_log_header(iochannel, TRUE);

	histories = g_new0(GArray *,rtv_map->derived_total);
	is_floats = g_new0(gboolean ,rtv_map->derived_total);

	for(i=0;i<rtv_map->derived_total;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		histories[i] = (GArray *)g_object_get_data(object,"history");
		is_floats[i] = (gboolean)g_object_get_data(object,"is_float");
	}

	for (x=0;x<rtv_map->ts_array->len;x++)
	{
		j = 0;
		for(i=0;i<rtv_map->derived_total;i++)
		{
			value = g_array_index(histories[i], gfloat, x);
			if (is_floats[i])
			{
				tmpbuf = g_ascii_formatd(buf,G_ASCII_DTOSTR_BUF_SIZE,"%.3f",value);
				g_string_append(output,tmpbuf);
			}
			else
				g_string_append_printf(output,"%i",(gint)value);
			j++;

			/* Print delimiter to log here so there isnt an extra
			 * char at the end fo the line 
			 */
			if (j < rtv_map->derived_total)
				output = g_string_append(output,delimiter);
		}
		output = g_string_append(output,"\r\n");
	}
	g_io_channel_write_chars(iochannel,output->str,output->len,&count,NULL);
	g_free(is_floats);
	g_free(histories);
	g_string_free(output,TRUE);
	if (restart_tickler)
		start_tickler(RTV_TICKLER);

}
