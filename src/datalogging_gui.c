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
gint max_logables = 0;
GtkWidget *dlog_view;
GtkWidget *logables_table;
GtkWidget *delim_table;
GtkWidget *tab_delimiter_button;
struct Logables logables;

/* External global vars */
extern gint ready;
extern struct Runtime_Common *runtime;
struct DynamicButtons buttons;
struct DynamicLabels labels;
extern GdkColor white;
extern struct RtvMap *rtv_map;

/* Static vars to all functions in this file... */
static gint total_logables = 0;
static gint offset_list[MAX_LOGABLES];
static gint size_list[MAX_LOGABLES];
static gboolean logging_active = FALSE;
static gboolean header_needed = FALSE;
static GtkWidget *file_selection;
static GtkWidget *format_table;
static GHashTable *custom_ord_hash;
static GHashTable *classic_ord_hash;
static GHashTable *full_ord_hash;
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
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget *button;
	GtkWidget *label;
	gint table_rows = 0;
	GObject * object = NULL;
	gchar * dlog_name = NULL;
	extern GHashTable *dynamic_widgets;

	vbox = g_hash_table_lookup(dynamic_widgets,"dlog_logable_vars_vbox1");
	max_logables = rtv_map->derived_total;
	table_rows = ceil((float)max_logables/(float)TABLE_COLS);
	table = gtk_table_new(table_rows,TABLE_COLS,TRUE);
//	logables_table = table;
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox),table,TRUE,TRUE,0);

	j = 0;	
	k = 0;
	for (i=0;i<max_logables;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *,i);
		dlog_name = (gchar *)g_object_get_data(object,"dlog_gui_name");
		button = gtk_check_button_new_with_label(dlog_name);
		//button = gtk_check_button_new();
		//label = gtk_label_new(NULL);
		//gtk_label_set_markup(GTK_LABEL(label),dlog_name);
		//gtk_button_set_label(GTK_BUTTON(button),gtk_label_get_label(GTK_LABEL(label)));
		//gtk_tooltips_set_tip(tip,button,logable_names_tips[i],NULL);

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
	gchar * tmpbuf;
	GtkWidget * widget = NULL;
	if (logging_active)
		return;   /* Logging already running ... */

	gtk_widget_set_sensitive(logables_table,FALSE);
	gtk_widget_set_sensitive(delim_table,FALSE);
	gtk_widget_set_sensitive(format_table,FALSE);
	gtk_widget_set_sensitive(file_selection,FALSE);
	header_needed = TRUE;
	logging_active = TRUE;
	tmpbuf = g_strdup_printf("DataLogging Started...\n");
	update_logbar("dlog_view",NULL,tmpbuf,TRUE,FALSE);
	g_free(tmpbuf);
	widget = gtk_button_new();
	g_object_set_data(G_OBJECT(widget),"handler",
			GINT_TO_POINTER(START_REALTIME));
	std_button_handler(widget,NULL);
	gtk_widget_destroy(widget);
	return;
}

void stop_datalogging()
{
	gchar *tmpbuf = NULL;
	logging_active = FALSE;
	if (logging_mode == CUSTOM_LOG)
	{
		gtk_widget_set_sensitive(logables_table,TRUE);
		gtk_widget_set_sensitive(delim_table,TRUE);
	}
	gtk_widget_set_sensitive(format_table,TRUE);
	gtk_widget_set_sensitive(file_selection,TRUE);
	tmpbuf = g_strdup_printf("DataLogging Stopped...\n");
	update_logbar("dlog_view",NULL,tmpbuf,TRUE,FALSE);
	g_free(tmpbuf);
	return;
}


void clear_logables(void)
{
	gint i = 0;
	/* Uncheck all logable choices */
	for (i=0;i<max_logables;i++)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(logables.widgets[i]),
				FALSE);
}

gboolean log_value_set(GtkWidget * widget, gpointer data)
{
	gint index = 0;
	gint size = 0;
	gint classic_ord = -1;
	gint full_ord = -1;
	gint i = 0;

	index = (gint)g_object_get_data(G_OBJECT(widget),"index");
	size = (gint)g_object_get_data(G_OBJECT(widget),"size");
	classic_ord = (gint)g_object_get_data(G_OBJECT(widget),
			"mt_classic_order");
	full_ord = (gint)g_object_get_data(G_OBJECT(widget),
			"mt_full_order");

	/* Set state in array so total count can be determined... */
	logables.index[index] = gtk_toggle_button_get_active 
		(GTK_TOGGLE_BUTTON (widget));

	/* insert into hash table for ordered printout (megatune compat) */
	if (logables.index[index])
	{
		g_hash_table_insert(classic_ord_hash,
				GINT_TO_POINTER(classic_ord),(gpointer)index+1);
		g_hash_table_insert(full_ord_hash,
				GINT_TO_POINTER(full_ord),(gpointer)index+1);
		g_hash_table_insert(custom_ord_hash,
				GINT_TO_POINTER(index),(gpointer)index+1);
	}
	else
	{
		g_hash_table_remove(classic_ord_hash,
				GINT_TO_POINTER(classic_ord));
		g_hash_table_remove(full_ord_hash,
				GINT_TO_POINTER(full_ord));
		g_hash_table_remove(custom_ord_hash,
				GINT_TO_POINTER(index));
	}

	total_logables = 0;
	// Update total count....
	for (i=0;i<max_logables;i++)
	{
		if (logables.index[i])
			total_logables++;
	}

	return TRUE;
}

void write_log_header(void *ptr)
{
	gint i = 0;
	gint j = 0;
	gint total_logables = 0;
	gsize count = 0;
	gint index = -1;
	GString *output;
	struct Io_File *iofile = NULL;
	if (ptr != NULL)
		iofile = (struct Io_File *)ptr;
	else
	{
		dbg_func(__FILE__": write_log_header()\n\tIo_File pointer was undefined, returning NOW...\n",CRITICAL);
		return;
	}

	output = g_string_sized_new(64); /* pre-allccate for 64 chars */

	// Get total number of logables....
	for (i=0;i<max_logables;i++)
		if (logables.index[i])
			total_logables++;

	for (i=0;i<max_logables;i++)
	{
		index = -1;
		if (logging_mode == MT_CLASSIC_LOG)
		{
			index = (gint)g_hash_table_lookup(classic_ord_hash,
					GINT_TO_POINTER(i));
			if (index == 0)
				continue;
			index -= 1;
			output = g_string_append(output, 
					mt_classic_names[index]);
			offset_list[j] = logging_offset_map[index];
			size_list[j] = logging_datasizes_map[index];
			j++;
		}
		else if (logging_mode == MT_FULL_LOG)
		{
			index = (gint)g_hash_table_lookup(full_ord_hash,
					GINT_TO_POINTER(i));
			if (index == 0)
				continue;
			index -= 1;
			output = g_string_append(output, 
					mt_full_names[index]);
			offset_list[j] = logging_offset_map[index];
			size_list[j] = logging_datasizes_map[index];
			j++;
		}
		else
		{
			index = (gint)g_hash_table_lookup(custom_ord_hash,
					GINT_TO_POINTER(i));
			if (index == 0)
				continue;
			index -= 1;
			//g_printf("i %i, index %i\n",i,index);
			output = g_string_append(output, 
					g_strdelimit(g_strdup(logable_names[index])," ",'_'));
			offset_list[j] = logging_offset_map[index];
			size_list[j] = logging_datasizes_map[index];
			j++;
		}

		if (j < (total_logables))
			output = g_string_append(output,delimiter);
	}
	output = g_string_append(output,"\r\n");
	g_io_channel_write_chars(iofile->iochannel,output->str,output->len,&count,NULL);
	g_string_free(output,TRUE);

}

void run_datalog(void)
{
	gint i = 0;
	gint offset = 0;
	gint size = 0;
	gint begin = FALSE;
	gsize count = 0;
	void *data;
	GString *output;
	struct Io_File *iofile = NULL;
	guchar * uchar_ptr = (guchar *)runtime;
	gshort * short_ptr = (gshort *)runtime;
	gfloat * float_ptr = (gfloat *)runtime;
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


	output = g_string_sized_new(64); /*64 char initial size */

	if (header_needed)
	{
		write_log_header((void *)iofile);
		begin = TRUE;
		header_needed = FALSE;
	}
	for(i=0;i<total_logables;i++)
	{
		offset = offset_list[i];
		size = size_list[i];
		switch (offset)
		{
			case 99:
				/* Special Hi-Res clock to be logged */
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
			default:
				switch (size)
				{
					case FLOAT:
						g_string_append_printf(
								output,"%.3f",(float)float_ptr[offset/FLOAT]);
						break;
					case SHORT:
						g_string_append_printf(
								output,"%i",short_ptr[offset/SHORT]);
						break;
					case UCHAR:
						g_string_append_printf(
								output,"%i",(guchar)uchar_ptr[offset]);
						break;
					default:
						dbg_func(g_strdup_printf(__FILE__": run_datalog()\n\tSIZE not defined (%i), log corruption likely!!\n",i),CRITICAL);
						break;
				}

				break;
		}
		/* Print delimiter to log here so there isnt an extra
		 * char at the end fo the line 
		 */
		if (i < (total_logables-1))
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
