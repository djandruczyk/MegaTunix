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

#include <args.h>
#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <fileio.h>
#include <gtk/gtk.h>
#include <listmgmt.h>
#include <notifications.h>
#include <offline.h>
#include <tabloader.h>

extern GdkColor red;
extern GdkColor black;
extern GdkColor green;
static volatile gboolean warning_present = FALSE;
static GtkWidget *warning_dialog;
extern gint dbg_lvl;
extern GObject *global_data;



/*!
 \brief set_group_color() sets all the widgets in the passed group to 
 the color passed.
 \param color (GuiColor enumeration) the color to set the widgets to
 \param group (gchar *) textual name of the group of controls to alter color
 \see set_widget_color
 */
void set_group_color(GuiColor color, gchar *group)
{
	g_list_foreach(get_list(group), set_widget_color,(gpointer)color);
}


/*!
 \brief set_reqfuel_color() sets all the widgets in the reqfuel group as 
 defined by the page number passed to the color passed.
 \param color (GuiColor enumeration) the color to set the widgets to
 \param table_num (gint) the table number to determien the right group of 
 controls to change the color on
 \see set_widget_color
 */
void set_reqfuel_color(GuiColor color, gint table_num)
{
	gchar *name = NULL;
	name = g_strdup_printf("reqfuel_%i_ctrl",table_num);
	g_list_foreach(get_list(name), set_widget_color,(gpointer)color);
	g_free(name);
}


/*!
 \brief set_widget_color() sets all the widgets in the passed group to 
 the color passed.
 \param widget (gpointer) the widget to  change color
 \param color (GuiColor) enumeration of the color to switch to..
 */
void set_widget_color(gpointer widget, gpointer color)
{
	switch ((GuiColor)color)
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
		case GREEN:
			if (GTK_IS_BUTTON(widget))
			{
				gtk_widget_modify_fg(GTK_BIN(widget)->child,
						GTK_STATE_NORMAL,&green);
				gtk_widget_modify_fg(GTK_BIN(widget)->child,
						GTK_STATE_PRELIGHT,&green);
			}
			else if (GTK_IS_LABEL(widget))
			{
				gtk_widget_modify_fg(widget,
						GTK_STATE_NORMAL,&green);
				gtk_widget_modify_fg(widget,
						GTK_STATE_PRELIGHT,&green);
			}
			else
			{	
				gtk_widget_modify_text(GTK_WIDGET(widget),
						GTK_STATE_NORMAL,&green);
				gtk_widget_modify_text(GTK_WIDGET(widget),
						GTK_STATE_INSENSITIVE,&green);
			}
			break;
		default:
			break;
	}
}


/*!
 \brief update_logbar() updates the logbar passed with the text passed to it
 \param view_name (gchar *) textual name of the textview the text is supposed 
 to go to. (required)
 \param tagname (gchar *) textual tagname to be used to set attributes on the
 text (optional, can be NULL)
 \param message (gchar *) message to display (required)
 \param count (gboolean) flag to show a running count or not
 \param clear (gboolean) if set, clear display before displaying text
 */
void  update_logbar(
		gchar * view_name, 
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
	GtkWidget * widget = NULL;
	extern GHashTable *dynamic_widgets;
	extern volatile gboolean leaving;

	if (leaving)
	{
		g_free(message);
		return;
	}

	widget = (GtkWidget *)g_hash_table_lookup(dynamic_widgets,view_name);

	if (!widget)
	{
		g_free(message);
		return;
	}
	if (!GTK_IS_OBJECT(widget))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": update_logbar()\n\t Textview name passed: \"%s\" wasn't registered, not updating\n",view_name));
		g_free(message);
		return;
	}

	/* Add the message to the end of the textview */
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
	if (!textbuffer)
	{
		g_free(message);
		return;
	}
	gtk_text_buffer_get_end_iter (textbuffer, &iter);
	result = OBJ_GET(widget,"counter");	
	if (result == NULL)
		counter = 0;
	else
		counter = (gint)result;

	if (count)
	{
		counter++;
		tmpbuf = g_strdup_printf(" %i. ",counter);
		OBJ_SET(widget,"counter",GINT_TO_POINTER(counter));	
	}

	if (tagname == NULL)
	{
		if (count) /* if TRUE, display counter, else don't */
			gtk_text_buffer_insert(textbuffer,&iter,tmpbuf,-1);
		gtk_text_buffer_insert(textbuffer,&iter,message,-1);
	}
	else
	{
		if (count) /* if TRUE, display counter, else don't */
			gtk_text_buffer_insert_with_tags_by_name(textbuffer,
					&iter,tmpbuf,-1,tagname,NULL);

		gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,
				message,-1,tagname,NULL);
	}

	g_free(message);
	/* Get it's parent (the scrolled window) and slide it to the
	 * bottom so the new message is visible... 
	 */
	parent = gtk_widget_get_parent(widget);
	if (parent != NULL)
	{
		adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(parent));
		adj->value = adj->upper;
	}

	if (tmpbuf)
		g_free(tmpbuf);
	return;	
}


/*!
 \brief conn_warning() displays a warning message when connection is 
 either lost or not detected with the ECU
 */
EXPORT void conn_warning(void)
{
	gchar *buff = NULL;
	CmdLineArgs *args = OBJ_GET(global_data,"args");

	if ((args->be_quiet) || (warning_present))
		return;
	buff = g_strdup("The MegaSquirt ECU appears to be currently disconnected.  This means that either one of the following occurred:\n   1. Wrong Comm port is selected on the Communications Tab\n   2. The MegaSquirt serial link is not plugged in\n   3. The MegaSquirt ECU does not have adequate power.\n   4. The MegaSquirt ECU is in bootloader mode.\n\nSuggest checking the serial settings on the Communications page first, and then check the Serial Status log at the bottom of that page. If the Serial Status log says \"I/O with MegaSquirt Timeout\", it means one of a few possible problems:\n   1. You selected the wrong COM port (older systems came with two, most newer ones only have one, try the other one...)\n   2. Faulty cable to the MegaSquirt unit. (Should be a straight thru DB9 Male/Female cable)\n   3. The cable is OK, but the MS doesn't have adequate power.\n   4. If you have the ECU in bootloader mode, none of the lights lite up, and a terminal program will show a \"Boot>\" prompt Disconnect the Boot jumper and power cycle the ECU.\n\nIf it says \"Serial Port NOT Opened, Can NOT Test ECU Communications\", this can mean one of two things:\n   1. The COM port doesn't exist on your computer,\n\t\t\t\t\tOR\n   2. You don't have permission to open the port. (/dev/ttySx).\nUNIX ONLY: Change the permissions on /dev/ttyS* to 666 (\"chmod 666 /dev/ttyS*\" as root), NOTE: This has potential security implications. Check the Unix/Linux system documentation regarding \"security\" for more information...\n");
	warn_user(buff);
	g_free(buff);
}


/*!
 \brief kill_conn_warning() removes the no connection warning message.
  Takes no parameters.
 */
EXPORT void kill_conn_warning()
{
	if (!warning_present)
		return;
	if (GTK_IS_WIDGET(warning_dialog))
		close_dialog(warning_dialog,warning_dialog);
}


/*!
 \brief warn_user() displays a warning message on the screen as a error dialog
 \param message (gchar *) the text to display
 */
void warn_user(gchar *message)
{
	extern gboolean interrogated;
	CmdLineArgs *args = OBJ_GET(global_data,"args");
	if ((args->be_quiet))
		return;

	warning_dialog = gtk_message_dialog_new(NULL,0,GTK_MESSAGE_ERROR,
			GTK_BUTTONS_NONE,message);
	if (!interrogated)
		gtk_dialog_add_buttons(GTK_DIALOG(warning_dialog),"Go to Offline mode", GTK_RESPONSE_ACCEPT,"_Close", GTK_RESPONSE_CLOSE,NULL);
	else
		gtk_dialog_add_buttons(GTK_DIALOG(warning_dialog),"_Close", GTK_RESPONSE_CLOSE,NULL);
			

	g_signal_connect (G_OBJECT(warning_dialog),
			"response",
			G_CALLBACK (get_response),
			warning_dialog);

	warning_present = TRUE;
	gtk_widget_show_all(warning_dialog);
}


gboolean get_response(GtkWidget *widget, gpointer data)
{
	gint response = (gint)data;
	if (response == GTK_RESPONSE_ACCEPT)
	{
		close_dialog(widget,NULL);
		set_title(g_strdup("Offline Mode..."));
		set_offline_mode();
	}
	if (response == GTK_RESPONSE_CLOSE)
		close_dialog(widget,NULL);
	return TRUE;
}


/*!
 \brief close_dialog() is a handler to close the dialog and reset the flag
 showing if it was up or not so it prevents displaying the error multiple
 times
 \param widget (GtkWidget *) widget to destroy
 \param data (gpointer) unused
 */
gboolean close_dialog(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(widget);
	g_timeout_add(2000,set_warning_flag,NULL);
	warning_dialog = NULL;
	return TRUE;
}



/*!
 \brief set_title() appends text to the titlebar of the application to
 give user notifications...
 \param text (gchar *) text to append, (static strings only please)
 */
void set_title(gchar * text)
{
	extern GtkWidget *main_window;
	gchar * tmpbuf = NULL;
	extern volatile gboolean leaving;
	extern GHashTable *dynamic_widgets;
	static GtkWidget *info_label = NULL;

	if ((!main_window) || (leaving))
		return;
	if (!info_label)
		info_label = (GtkWidget *)g_hash_table_lookup(dynamic_widgets,"info_label");
	tmpbuf = g_strconcat("MegaTunix ",VERSION,",   ",text,NULL);

	gtk_window_set_title(GTK_WINDOW(main_window),tmpbuf);
	g_free(tmpbuf);
	if (GTK_IS_WIDGET(info_label))
	{
		tmpbuf = g_markup_printf_escaped("<big>%s</big>",text);
		gtk_label_set_markup(GTK_LABEL(info_label),tmpbuf);
		g_free(tmpbuf);
	}
	g_free(text);
}

gboolean set_warning_flag(gpointer user_data)
{
	warning_present = FALSE;
	return FALSE;
}
