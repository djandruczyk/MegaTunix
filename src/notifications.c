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
#include <globals.h>
#include <gtk/gtk.h>
#include <notifications.h>
#include <structures.h>

extern GdkColor red;
extern GdkColor black;
extern struct DynamicButtons buttons;
extern struct DynamicLabels labels;
extern struct DynamicSpinners spinners;
static gboolean warning_present = FALSE;


void set_store_buttons_state(GuiState state)
{
	switch (state)
	{
		case RED:
			/* Let user know to burn vars byt turnign button text red */
			gtk_widget_modify_fg(GTK_BIN(
						buttons.const_store_but)->child,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.const_store_but)->child,
					GTK_STATE_PRELIGHT,&red);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.enrich_store_but)->child,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.enrich_store_but)->child,
					GTK_STATE_PRELIGHT,&red);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.vetable_store_but)->child,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.vetable_store_but)->child,
					GTK_STATE_PRELIGHT,&red);
			break;
		case BLACK:
			gtk_widget_modify_fg(GTK_BIN(
						buttons.const_store_but)->child,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.const_store_but)->child,
					GTK_STATE_PRELIGHT,&black);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.enrich_store_but)->child,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.enrich_store_but)->child,
					GTK_STATE_PRELIGHT,&black);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.vetable_store_but)->child,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_fg(GTK_BIN(
						buttons.vetable_store_but)->child,
					GTK_STATE_PRELIGHT,&black);
			break;
	}
}

void update_logbar(GtkWidget *view, gchar * tagname, gchar * message)
{
	GtkTextIter iter;
	GtkTextBuffer * textbuffer;
	GtkWidget *parent;
	GtkAdjustment * adj;
	gint counter;
	gchar *tmpbuf;
	gpointer result;

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
		gtk_adjustment_changed(GTK_ADJUSTMENT(adj));
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

void squirt_cyl_inj_set_state(GuiState state)
{
	switch (state)
	{
		case RED:
			gtk_widget_modify_fg(labels.squirts_lab,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_fg(labels.cylinders_lab,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_text(spinners.cylinders_spin,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_text(spinners.inj_per_cycle_spin,
					GTK_STATE_NORMAL,&red);
			break;
		case BLACK:
			gtk_widget_modify_fg(labels.squirts_lab,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_fg(labels.cylinders_lab,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_text(spinners.cylinders_spin,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_text(spinners.inj_per_cycle_spin,
					GTK_STATE_NORMAL,&black);
			break;
	}
}

void interdep_state(GuiState state, gint page)
{
	switch (state)
	{
		case RED:
			gtk_widget_modify_fg(labels.squirts_lab,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_fg(labels.injectors_lab,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_fg(labels.cylinders_lab,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_text(spinners.req_fuel_total_spin,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_text(spinners.inj_per_cycle_spin,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_text(spinners.cylinders_spin,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_text(spinners.injectors_spin,
					GTK_STATE_NORMAL,&red);
			gtk_widget_modify_text(
					spinners.req_fuel_per_squirt_spin,
					GTK_STATE_INSENSITIVE,&red);
			break;
		case BLACK:
			gtk_widget_modify_fg(labels.squirts_lab,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_fg(labels.injectors_lab,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_fg(labels.cylinders_lab,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_text(spinners.req_fuel_total_spin,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_text(spinners.inj_per_cycle_spin,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_text(spinners.cylinders_spin,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_text(spinners.injectors_spin,
					GTK_STATE_NORMAL,&black);
			gtk_widget_modify_text(
					spinners.req_fuel_per_squirt_spin,
					GTK_STATE_INSENSITIVE,&black);

			break;
	}
}

void warn_file_not_empty(void)
{
	gchar *buff;
	buff = g_strdup("The Log/VEX file you selected is NOT empty.  If you wish to overwrite this file, just hit \"Close\" to dismiss this warning, and select \"Clear Log/VEX File\" button.  If you do NOT clear out the logfile, the information already present in it will most likely confuse whatever program you use to process it.  If you do NOT wish to use this logfile, Close this warning and click on \"Select Logfile\" and either select a new empty file, or type in the name of your choice.");
	warn_user(buff);
	g_free(buff);
}
