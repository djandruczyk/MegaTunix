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

#include <defines.h>
#include <fileio.h>
#include <gtk/gtk.h>
#include <notifications.h>
#include <stdio.h>
#include <structures.h>

extern GdkColor red;
extern GdkColor black;
extern struct DynamicButtons buttons;
extern struct DynamicLabels labels;
extern struct DynamicSpinners spinners;
static gboolean warning_present = FALSE;
extern GtkWidget *tools_view;
extern GtkWidget *dlog_view;
GList *store_widgets;	/* list of widgets that have color change attributes*/
GList *interdep_widgets;/* list of widgets that have color change attributes*/
GList *reqfuel_widgets;	/* list of widgets that have color change attributes*/


void set_store_buttons_state(GuiState state)
{
	g_list_foreach(store_widgets, set_widget_color,(gpointer)state);
}

void set_interdep_state(GuiState state)
{
	g_list_foreach(interdep_widgets, set_widget_color,(gpointer)state);
}

void set_reqfuel_state(GuiState state)
{
	g_list_foreach(reqfuel_widgets, set_widget_color,(gpointer)state);
}

void set_widget_color(gpointer widget, gpointer state)
{
	switch ((GuiState)state)
	{
		case RED:
			if (GTK_IS_BUTTON(widget))
			{
				gtk_widget_modify_fg(GTK_BIN(widget)->child,
						GTK_STATE_NORMAL,&red);
				gtk_widget_modify_fg(GTK_BIN(widget)->child,
						GTK_STATE_PRELIGHT,&red);
			}
			else if (GTK_IS_LABEL(widget))
			{
				gtk_widget_modify_fg(widget,
						GTK_STATE_NORMAL,&red);
				gtk_widget_modify_fg(widget,
						GTK_STATE_PRELIGHT,&red);
			}
			else
			{
				gtk_widget_modify_text(GTK_WIDGET(widget),
						GTK_STATE_NORMAL,&red);
				gtk_widget_modify_text(GTK_WIDGET(widget),
						GTK_STATE_INSENSITIVE,&red);
			}
			break;
		case BLACK:
			if (GTK_IS_BUTTON(widget))
			{
				gtk_widget_modify_fg(GTK_BIN(widget)->child,
						GTK_STATE_NORMAL,&black);
				gtk_widget_modify_fg(GTK_BIN(widget)->child,
						GTK_STATE_PRELIGHT,&black);
			}
			else if (GTK_IS_LABEL(widget))
			{
				gtk_widget_modify_fg(widget,
						GTK_STATE_NORMAL,&black);
				gtk_widget_modify_fg(widget,
						GTK_STATE_PRELIGHT,&black);
			}
			else
			{	
				gtk_widget_modify_text(GTK_WIDGET(widget),
						GTK_STATE_NORMAL,&black);
				gtk_widget_modify_text(GTK_WIDGET(widget),
						GTK_STATE_INSENSITIVE,&black);
			}
			break;
	}
}

void 
 update_logbar(
		GtkWidget *view, 
		gchar * tagname, 
		gchar * message,
		gboolean count,
		gboolean clear)
{
	GtkTextIter iter;
	GtkTextBuffer * textbuffer = NULL;
	GtkWidget *parent = NULL;
	GtkAdjustment * adj = NULL;
	gint counter = -1;
	gchar *tmpbuf = NULL;
	gpointer result = NULL;

	/* Add the message to the end of the textview */
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_get_end_iter (textbuffer, &iter);
	result = g_object_get_data(G_OBJECT(view),"counter");	
	if (result == NULL)
		counter = 0;
	else
		counter = (gint)result;

	counter++;
	tmpbuf = g_strdup_printf("%i. ",counter);
	g_object_set_data(G_OBJECT(view),"counter",GINT_TO_POINTER(counter));	

	if (count) /* if TRUE, display counter, else don't */
		gtk_text_buffer_insert(textbuffer,&iter,tmpbuf,-1);
	if (tagname == NULL)
		gtk_text_buffer_insert(textbuffer,&iter,message,-1);
	else
		gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,
				message,-1,tagname,NULL);

	/* Get it's parent (the scrolled window) and slide it to the
	 * bottom so the new message is visible... 
	 */
	parent = gtk_widget_get_parent(view);
	if (parent != NULL)
	{
		adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(parent));
		adj->value = adj->upper;
		gtk_adjustment_value_changed(GTK_ADJUSTMENT(adj));
	}

	return;	
}

void no_ms_connection(void)
{
	gchar *buff;
	buff = g_strdup("The MegaSquirt ECU appears to be currently disconnected.  This means that either one of the following occurred:\n   1. Wrong Comm port is selected on the Communications Tab\n   2. The MegaSquirt serial link is not plugged in\n   3. The MegaSquirt ECU does not have adequate power.\n\nSuggest checking the serial settings on the Communications page first, and then check the Serial Status log at the bottom of that page. If the Serial Status log says \"I/O with MegaSquirt Timeout\", it means one of a few possible problems:\n   1. You selected the wrong COM port (older systems came with two, most newer ones only have one, try the other one...)\n   2. Faulty cable to the MegaSquirt unit. (Should be a straight thru DB9 Male/Female cable)\n   3. The cable is OK, but the MS doesn't have adequate power.\n\nIf it says \"Serial Port NOT Opened, Can NOT Test ECU Communications\", this can mean one of two things:\n   1. The COM port doesn't exist on your computer,\n\t\t\t\t\tOR\n   2. You don't have permission to open the port. (/dev/ttySx).\nChange the permissions on /dev/ttyS* to 666 (\"chmod 666 /dev/ttyS*\" as root), NOTE: This has potential security implications. Check the Unix/Linux system documentation regarding \"security\" for more information...");
	if (!warning_present)
		warn_user(buff);
	g_free(buff);
}

void warn_user(gchar *message)
{
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(NULL,0,GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,message);
	g_signal_connect (G_OBJECT(dialog),
			"delete_event",
			G_CALLBACK (close_dialog),
			dialog);
	g_signal_connect (G_OBJECT(dialog),
			"destroy_event",
			G_CALLBACK (close_dialog),
			dialog);
	g_signal_connect (G_OBJECT(dialog),
			"response",
			G_CALLBACK (close_dialog),
			dialog);

	warning_present = TRUE;
	gtk_widget_show_all(dialog);

}

gint close_dialog(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(widget);
	warning_present = FALSE;
	return TRUE;
}

void warn_input_file_not_exist(FileIoType iotype, gchar * filename)
{
	gchar *buff = NULL;
	gchar *function = NULL;
	gchar *choice = NULL;
	gchar *lbar_msg = NULL;
	switch (iotype)
        {
                case VE_IMPORT:
                        function = g_strdup("VE Table Load");
                        choice = g_strdup("\"Import VE Table (s)\"");
			lbar_msg = g_strdup_printf("The selected file %s does NOT exist\nImporting a VE Table requires a valid VEX file\n",filename);
			update_logbar(tools_view,"warning",lbar_msg,TRUE,FALSE);
                        break;
                case DATALOG_IMPORT:
                        function = g_strdup("DataLog Load");
                        choice = g_strdup("\"Select Log File\"");
			lbar_msg = g_strdup_printf("The selected file %s does NOPT exist\nImporting a DataLog for viewing requires a valid datalog file\n",filename);
			update_logbar(dlog_view,"warning",lbar_msg,TRUE,FALSE);
                        break;
                case FULL_RESTORE:
                        function = g_strdup("MegaSquirt Restore");
                        choice = g_strdup("\"Restore All MS Parameters\"");
			lbar_msg = g_strdup_printf("The selected file %s does NOT exist\nA Full restore of MegaSquirt parameters requires a valid backup file\n",filename);
			update_logbar(tools_view,"warning",lbar_msg,TRUE,FALSE);
                        break;
                default:
                        break;
        }

        buff = g_strdup_printf("The file you selected does NOT exist. The %s function requires a valid input file to work, please close this dialog by clicking on \"Close\" to dismiss this warning, and select the %s button and select a valid file.",function,choice);
        warn_user(buff);
	g_free(buff);
	if (lbar_msg)
		g_free(lbar_msg);
	if (function)
		g_free(function);
	if (choice)
		g_free(choice);
	return;
}
gboolean warn_file_not_empty(FileIoType iotype,gchar * filename)
{
	GtkWidget *dialog;
	gchar *buff = NULL;
	gchar *filetype = NULL;
	gint result = 0;
	gboolean truncated = FALSE;

	switch (iotype)
	{
		case DATALOG_EXPORT:
			filetype = g_strdup("DataLog");
			break;
		case FULL_BACKUP:
			filetype = g_strdup("MegaSquirt Backup");
			break;
		case VE_EXPORT:
			filetype = g_strdup("VE Table Export");
			break;
		default:
			break;
	}

	buff = g_strdup_printf("The %s file (%s) you selected already exists and contains data.  If you wish to overwrite this file, just hit \"Truncate\" to overwrite this file, otherwise select \"Cancel\"",filetype, filename);

	dialog = gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_NONE,buff);

	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			"Truncate File",iotype,
			GTK_STOCK_CANCEL,GTK_RESPONSE_NONE,NULL);

	gtk_widget_show_all(dialog);

	if (buff)
		g_free(buff);
	if (filetype)
		g_free(filetype);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result >= 0)
	{
		truncate_file(result,filename);
		truncated = TRUE;
	}
	else
		truncated = FALSE;
	gtk_widget_destroy(dialog);

	return truncated;
}
