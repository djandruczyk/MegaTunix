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
#include <datalogging_const.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <fileio.h>
#include <glib.h>
#include <gui_handlers.h>
#include <math.h>
#include <ms_structures.h>
#include <notifications.h>
#include <structures.h>
#include <sys/types.h>
#include <unistd.h>


/* global vars (owned here...) */
gchar *delimiter;
gfloat cumu = 0.0;
gint logging_mode = CUSTOM_LOG;

/* External global vars */
extern gint ready;
extern struct Runtime_Common *runtime;
extern GdkColor white;
extern struct RtvMap *rtv_map;

/* Static vars to all functions in this file... */
static gboolean logging_active = FALSE;
static gboolean header_needed = FALSE;
static GTimeVal now;
static GTimeVal last;


/* mt_classic[] and mt_full[] are arrays laid out like the datalogging
 * screen, insert a "1" where you want the button selected for that mode
 * otherwise use a zero...
 */
static const gboolean mt_classic[] =
{
	FALSE,	TRUE,	TRUE,	TRUE,	FALSE,	
	FALSE,	FALSE,	FALSE,	FALSE,	FALSE,	
	FALSE,	FALSE,	FALSE,	FALSE,	FALSE,	
	FALSE,	TRUE,	FALSE,	FALSE,	FALSE,	
	TRUE,	FALSE,	TRUE,	FALSE,	FALSE,	
	TRUE,	TRUE,	TRUE,	TRUE,	FALSE,	
	TRUE,	FALSE,	FALSE,	FALSE,	FALSE,	
	FALSE,	FALSE,	FALSE,	FALSE,	FALSE,	
	FALSE,	FALSE 
};

static const gboolean mt_full[] = 
{
	TRUE,	TRUE,	TRUE,	TRUE,	FALSE,	
	FALSE,	FALSE,	FALSE,	FALSE,	FALSE,	
	FALSE,	FALSE,	FALSE,	FALSE,	FALSE,	
	TRUE,	TRUE,	FALSE,	TRUE,	TRUE,	
	TRUE,	FALSE,	TRUE,	FALSE,	FALSE,	
	TRUE,	TRUE,	TRUE,	TRUE,	TRUE,	
	TRUE,	TRUE,	TRUE,	TRUE,	TRUE,	
	TRUE,	FALSE,	FALSE,	FALSE,	TRUE,	
	TRUE,	TRUE 
}; 


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
	extern GHashTable *dynamic_widgets;
	extern gint preferred_delimiter;

	vbox = g_hash_table_lookup(dynamic_widgets,"dlog_logable_vars_vbox1");
	table_rows = ceil((float)rtv_map->derived_total/(float)TABLE_COLS);
	table = gtk_table_new(table_rows,TABLE_COLS,TRUE);
//	logables_table = table;
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox),table,TRUE,TRUE,0);

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
		case SPACE:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_hash_table_lookup(dynamic_widgets,"dlog_space_delimit_radio_button")),TRUE);
			gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(g_hash_table_lookup(dynamic_widgets,"dlog_space_delimit_radio_button")));
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
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		dlog_name = (gchar *)g_object_get_data(object,"dlog_gui_name");
		button = gtk_check_button_new();
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),dlog_name);
		gtk_container_add(GTK_CONTAINER(button),label);
		//gtk_tooltips_set_tip(tip,button,logable_names_tips[i],NULL);

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
		gtk_table_attach (GTK_TABLE (table), button, j, j+1, k, k+1,
				(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
				(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
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

void start_datalogging(void)
{
	GtkWidget * widget = NULL;
	extern GHashTable *dynamic_widgets;

	if (logging_active)
		return;   /* Logging already running ... */

	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_logables_vars_vbox1"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_format_delimit_hbox1"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button"),FALSE);
	header_needed = TRUE;
	logging_active = TRUE;
	update_logbar("dlog_view",NULL,"DataLogging Started...\n",TRUE,FALSE);

	widget = gtk_button_new();
	g_object_set_data(G_OBJECT(widget),"handler",
			GINT_TO_POINTER(START_REALTIME));
	std_button_handler(widget,NULL);
	gtk_widget_destroy(widget);
	return;
}

void stop_datalogging()
{
	extern GHashTable *dynamic_widgets;

	logging_active = FALSE;

	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_format_delimit_hbox1"),TRUE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_format_delimit_hbox1"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button"),TRUE);
	update_logbar("dlog_view",NULL,"DataLogging Stopped...\n",TRUE,FALSE);
	return;
}


void clear_logables(void)
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

void write_log_header(void *ptr)
{
	gint i = 0;
	gint j = 0;
	gint total_logables = 0;
	gsize count = 0;
	GString *output;
	struct Io_File *iofile = NULL;
	GObject * object = NULL;
	gchar * string = NULL;
	if (ptr != NULL)
		iofile = (struct Io_File *)ptr;
	else
	{
		dbg_func(__FILE__": write_log_header()\n\tIo_File pointer was undefined, returning NOW...\n",CRITICAL);
		return;
	}
	/* Count total logable variables */
	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		if((gboolean)g_object_get_data(object,"being_logged"))
			total_logables++;
	}
	output = g_string_sized_new(64); /* pre-allccate for 64 chars */

	for (i=0;i<rtv_map->derived_total;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		if((gboolean)g_object_get_data(object,"being_logged"))
		{
			string = (gchar *)g_object_get_data(object,"dlog_field_name");
			output = g_string_append(output,string); 
			j++;
			if (j < total_logables)
				output = g_string_append(output,delimiter);
		}
	}
	output = g_string_append(output,"\r\n");
	g_io_channel_write_chars(iofile->iochannel,output->str,output->len,&count,NULL);
	g_string_free(output,TRUE);

}

void run_datalog(void)
{
	gint i = 0;
	gint j = 0;
	gint begin = FALSE;
	gsize count = 0;
	void *data;
	gint total_logables = 0;
	GString *output;
	struct Io_File *iofile = NULL;
	GObject *object = NULL;
	gfloat value = 0.0;
	gfloat *history = NULL;
	gint hist_position = 0;
	extern GHashTable *dynamic_widgets;

	if (!logging_active) /* Logging isn't enabled.... */
		return;

	data =  g_object_get_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_close_log_button")),"data");
	if (data != NULL)
		iofile = (struct Io_File *)data;
	else
	{
		dbg_func(__FILE__": run_datalog()\n\tIo_File undefined, returning NOW!!!\n",CRITICAL);
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
		write_log_header((void *)iofile);
		begin = TRUE;
		header_needed = FALSE;
	}
	j = 0;
	for(i=0;i<rtv_map->derived_total;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		if (!((gboolean)g_object_get_data(object,"being_logged")))
			continue;

		history = (gfloat *)g_object_get_data(object,"history");
		hist_position = (gint)g_object_get_data(object,"hist_position");
		value = history[hist_position];
		if ((gboolean)g_object_get_data(object,"is_float"))
			g_string_append_printf(output,"%.3f",value);
		else
			g_string_append_printf(output,"%i",(gint)value);
		j++;


		/*
		// Special Hi-Res clock to be logged 
		if (begin == TRUE)
		{	
		g_get_current_time(&now);
		last.tv_sec = now.tv_sec;
		last.tv_usec = now.tv_usec;
		begin = FALSE;
		output = g_string_append(output,"0.0");
		}
		else
		{
		g_get_current_time(&now);
		cumu += (now.tv_sec-last.tv_sec)+
		((double)(now.tv_usec-last.tv_usec)/1000000.0);
		last.tv_sec = now.tv_sec;
		last.tv_usec = now.tv_usec;
		g_string_append_printf(
		output,"%.3f",cumu);

		}
		break;
		 */

		/* Print delimiter to log here so there isnt an extra
		 * char at the end fo the line 
		 */
		if (j < total_logables)
			output = g_string_append(output,delimiter);
	}
	output = g_string_append(output,"\r\n");
	g_io_channel_write_chars(iofile->iochannel,output->str,output->len,&count,NULL);
	g_string_free(output,TRUE);

}

gboolean set_logging_mode(GtkWidget * widget, gpointer *data)
{
	gint handler = 0;

	if (GTK_IS_OBJECT(widget))
                handler = (ToggleButton)g_object_get_data(G_OBJECT(widget),"handler");
	if (!ready)
		return FALSE;

	if (GTK_TOGGLE_BUTTON(widget)->active) /* its pressed */
	{
		clear_logables();
		switch((ToggleButton)handler)
		{
			case MT_CLASSIC_LOG:
/*
				logging_mode = MT_CLASSIC_LOG;
				clear_logables();
				gtk_widget_set_sensitive(
						logables_table,FALSE);
				gtk_toggle_button_set_active(
						GTK_TOGGLE_BUTTON
						(tab_delimiter_button),
						TRUE);
				gtk_widget_set_sensitive(
						delim_table,FALSE);

				for (i=0;i<max_logables;i++)
				{
					if (mt_classic[i] == 1)
					{
						gtk_toggle_button_set_active(
								GTK_TOGGLE_BUTTON
								(logables.widgets[i]),
								TRUE);
					}
				}
*/
				break;
			case MT_FULL_LOG:
/*
				logging_mode = MT_FULL_LOG;
				clear_logables();
				gtk_widget_set_sensitive(
						logables_table,FALSE);
				gtk_toggle_button_set_active(
						GTK_TOGGLE_BUTTON
						(tab_delimiter_button),
						TRUE);
				gtk_widget_set_sensitive(
						delim_table,FALSE);

				for (i=0;i<max_logables;i++)
				{
					if (mt_full[i] == 1)
					{
						gtk_toggle_button_set_active(
								GTK_TOGGLE_BUTTON
								(logables.widgets[i]),
								TRUE);
					}
				}
*/
				break;
			case CUSTOM_LOG:
/*
				logging_mode = CUSTOM_LOG;
				clear_logables();
				gtk_widget_set_sensitive(
						logables_table,TRUE);
				gtk_widget_set_sensitive(
						delim_table,TRUE);
*/
				break;
			default:
				break;
		}
	}
	return TRUE;
}
