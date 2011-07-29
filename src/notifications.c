/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*! @file src/notifications.c
 *
 * @brief ...
 *
 *
 */

#include <args.h>
#include <debugging.h>
#include <gui_handlers.h>
#include <listmgmt.h>
#include <notifications.h>
#include <offline.h>
#include <widgetmgmt.h>

extern GdkColor red;
extern GdkColor black;
extern GdkColor green;
static volatile gboolean warning_present = FALSE;
static GtkWidget *warning_dialog;
extern gconstpointer *global_data;

/*!
 \brief set_group_color() sets all the widgets in the passed group to 
 the color passed.
 \param color, the color to set the widgets to.
 \param group, textual name of the group of controls to alter color
 \see set_widget_color
 */
G_MODULE_EXPORT void set_group_color(GuiColor color, const gchar *group)
{
	g_list_foreach(get_list(group), set_widget_color,(gpointer)color);
}


/*!
 \brief set_reqfuel_color() sets all the widgets in the reqfuel group as 
 defined by the page number passed to the color passed.
 \param color, the color to set the widgets to
 \param table_num, the table number to determine the right group of 
 controls to change the color on
 \see set_widget_color
 */
G_MODULE_EXPORT void set_reqfuel_color(GuiColor color, gint table_num)
{
	gchar *name = NULL;
	name = g_strdup_printf("interdep_%i_ctrl",table_num);
	g_list_foreach(get_list(name), set_widget_color,(gpointer)color);
	g_free(name);
}


/*!
 \brief set_widget_color() sets all the widgets in the passed group to 
 the color passed.
 \param widget, the widget to  change color
 \param color, enumeration of the color to switch to..
 */
G_MODULE_EXPORT void set_widget_color(gpointer widget, gpointer color)
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
 \param view_name, textual name of the textview the text is supposed 
 to go to. (required)
 \param tagname, textual tagname to be used to set attributes on the
 text (optional, can be NULL)
 \param message, message to display (required)
 \param count, flag to show a running count or not
 \param clear, if set, clear display before displaying text
 */
G_MODULE_EXPORT void  update_logbar(
		const gchar * view_name, 
		const gchar * tagname, 
		gchar * message,
		gboolean count,
		gboolean clear,
		gboolean free)
{
	GtkTextIter iter;
	GtkTextBuffer * textbuffer = NULL;
	GtkWidget *parent = NULL;
	GtkAdjustment * adj = NULL;
	gint counter = -1;
	gchar *tmpbuf = NULL;
	gpointer result = NULL;
	GtkWidget * widget = NULL;

	widget = (GtkWidget *)lookup_widget(view_name);

	if ((DATA_GET(global_data,"leaving")) || (!widget))
		return;

	if (!GTK_IS_OBJECT(widget))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": update_logbar()\n\t Textview name passed: \"%s\" wasn't registered, not updating\n",view_name));
		return;
	}

	/* Add the message to the end of the textview */
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
	if (!textbuffer)
		return;

	gtk_text_buffer_get_end_iter (textbuffer, &iter);
	result = OBJ_GET(widget,"counter");	
	if (result == NULL)
		counter = 0;
	else
		counter = (GINT)result;

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
		gtk_text_buffer_insert(textbuffer,&iter,(const gchar *)message,-1);
	}
	else
	{
		if (count) /* if TRUE, display counter, else don't */
			gtk_text_buffer_insert_with_tags_by_name(textbuffer,
					&iter,tmpbuf,-1,tagname,NULL);

		gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,
				(const gchar *)message,-1,tagname,NULL);
	}

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
	if (free)
		g_free(message);
	return;	
}


/*!
 \brief conn_warning() displays a warning message when connection is 
 either lost or not detected with the ECU
 */
G_MODULE_EXPORT void conn_warning(void)
{
	CmdLineArgs *args = DATA_GET(global_data,"args");

	if ((args->be_quiet) || (warning_present))
		return;
	warn_user(_("The ECU appears to be currently disconnected.  This means that either one of the following occurred:\n   1. MegaTunix couldn't determine the correct comm port to use\n   2. The serial link is not plugged in\n   3. The ECU does not have adequate power.\n   4. The ECU is in bootloader mode.\n\nTry power cycling your ECU and verifying all connections are in order."));
}


/*!
 \brief kill_conn_warning() removes the no connection warning message.
  Takes no parameters.
 */
G_MODULE_EXPORT void kill_conn_warning(void)
{
	if (!warning_present)
		return;
	if (GTK_IS_WIDGET(warning_dialog))
	{
		gtk_widget_destroy(warning_dialog);
		warning_dialog = NULL;
		warning_present = FALSE;
	}
}


/*!
 \brief warn_user() displays a warning message on the screen as a error dialog
 \param message, the text to display
 */
G_MODULE_EXPORT void warn_user(const gchar *message)
{
	CmdLineArgs *args = DATA_GET(global_data,"args");
	if ((args->be_quiet))
		return;

	warning_dialog = gtk_message_dialog_new((GtkWindow *)lookup_widget("main_window"),0,GTK_MESSAGE_ERROR,GTK_BUTTONS_NONE,"%s",message);

	if (!DATA_GET(global_data,"interrogated"))
		gtk_dialog_add_buttons(GTK_DIALOG(warning_dialog),(const gchar *)"Exit Megatunix",GTK_RESPONSE_CLOSE,(const gchar *)"Go to Offline mode", GTK_RESPONSE_ACCEPT,NULL);
	else
		gtk_dialog_add_buttons(GTK_DIALOG(warning_dialog),"_Close", GTK_RESPONSE_CANCEL,NULL);


	g_signal_connect (G_OBJECT(warning_dialog),
			"response",
			G_CALLBACK (get_response),
			warning_dialog);

	g_signal_connect_swapped (G_OBJECT(warning_dialog),
			"response",
			G_CALLBACK (gtk_widget_destroy),
			warning_dialog);

	warning_present = TRUE;
	gtk_widget_show_all(warning_dialog);
}



/*!
 \brief error_msg() displays a warning message on the screen as a error dialog
 \param message, the text to display
 */
G_MODULE_EXPORT void error_msg(const gchar *message)
{
	GtkWidget *dialog = NULL;
	gint result = 0;
	dialog = gtk_message_dialog_new((GtkWindow *)lookup_widget("main_window"),0,GTK_MESSAGE_ERROR,GTK_BUTTONS_NONE,"%s",message);

	gtk_dialog_add_buttons(GTK_DIALOG(dialog),(const gchar *)"Exit Megatunix",GTK_RESPONSE_CLOSE,(const gchar *)"Ignore at my peril!", GTK_RESPONSE_ACCEPT,NULL);

	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
		case GTK_RESPONSE_CLOSE:
			gtk_widget_destroy(dialog);
			leave(NULL,NULL);
			break;
		case GTK_RESPONSE_ACCEPT:
			gtk_widget_destroy(dialog);
	}
	return;
}


/*!
  \brief Retrieves the response from the offline mode dialog box choice
  \param widget, widget the user clicked on
  \param data, the response as an integer
  \returns TRUE
  */
G_MODULE_EXPORT gboolean get_response(GtkWidget *widget, gpointer data)
{
	gint response = (GINT)data;
	if (response == GTK_RESPONSE_ACCEPT)
	{
		set_title(g_strdup(_("Offline Mode...")));
		g_timeout_add(100,(GSourceFunc)set_offline_mode,NULL);
	}
	if (response == GTK_RESPONSE_CLOSE)
		leave(NULL,NULL);
	gdk_threads_add_timeout(1500,set_warning_flag,NULL);
	return TRUE;
}


/*!
 \brief reset_infolabel() resets infolabel text to "Ready"
 \param data, unused
 \returns FALSE
 */
G_MODULE_EXPORT gboolean reset_infolabel(gpointer data)
{
	static GtkWidget *info_label = NULL;
	if (!info_label)
		info_label = (GtkWidget *)lookup_widget("info_label");
	if (GTK_IS_WIDGET(info_label))
		gtk_label_set_markup(GTK_LABEL(info_label),"<b>Ready...</b>");
	return FALSE;
}

/*!
 \brief set_title() appends text to the titlebar of the application to
 give user notifications...
 \param text, text to append, dynamic strings only
 */
G_MODULE_EXPORT void set_title(gchar * text)
{
	gchar * tmpbuf = NULL;
	static GtkWidget *info_label = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	if ((!lookup_widget("main_window")) || (DATA_GET(global_data,"leaving")))
		return;
	if (!info_label)
		info_label = (GtkWidget *)lookup_widget("info_label");

	if (firmware)
	{
		if (firmware->actual_signature)
			tmpbuf = g_strdup_printf("MegaTunix %s,   (%s)   %s",VERSION,firmware->actual_signature,text);
		else
			tmpbuf = g_strconcat("MegaTunix ",VERSION,",   ",text,NULL);
	}
	else
		tmpbuf = g_strconcat("MegaTunix ",VERSION,",   ",text,NULL);

	gtk_window_set_title(GTK_WINDOW(lookup_widget("main_window")),tmpbuf);
	g_free(tmpbuf);
	if (info_label)
	{
		if (GTK_IS_WIDGET(info_label))
		{
			tmpbuf = g_markup_printf_escaped("<b>%s</b>",text);
			gtk_label_set_markup(GTK_LABEL(info_label),tmpbuf);
			g_free(tmpbuf);
		}
	}
	g_free(text);
}


/*!
  \brief enables the static warning present flag
  \param user_Data, unused
  \returns FALSE
  */
G_MODULE_EXPORT gboolean set_warning_flag(gpointer user_data)
{
	warning_present = FALSE;
	return FALSE;
}
