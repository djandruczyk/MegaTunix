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
  \file src/notifications.c
  \ingroup CoreMtx
  \brief Where all the notification messages come from...
  \author David Andruczyk
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
  \param color is the the color to set the widgets to.
  \param group is the textual name of the group of controls to alter color
  \see set_widget_color
  */
G_MODULE_EXPORT void set_group_color(GuiColor color, const gchar *group)
{
	ENTER();
	g_list_foreach(get_list(group), set_widget_color,(gpointer)color);
	EXIT();
	return;
}


/*!
  \brief set_reqfuel_color() sets all the widgets in the reqfuel group as 
  defined by the page number passed to the color passed.
  \param color is the the color to set the widgets to
  \param table_num is the the table number to determine the right group of 
  controls to change the color on
  \see set_widget_color
  */
G_MODULE_EXPORT void set_reqfuel_color(GuiColor color, gint table_num)
{
	gchar *name = NULL;
	ENTER();
	name = g_strdup_printf("interdep_%i_ctrl",table_num);
	g_list_foreach(get_list(name), set_widget_color,(gpointer)color);
	g_free(name);
	EXIT();
	return;
}


/*!
  \brief set_widget_color() sets all the widgets in the passed group to 
  the color passed.
  \param widget is the the widget to  change color
  \param color is the enumeration of the color to switch to..
  */
G_MODULE_EXPORT void set_widget_color(gpointer w_ptr, gpointer c_ptr)
{
	GtkWidget *widget = (GtkWidget *)w_ptr;
	GuiColor color = (GuiColor)(GINT)c_ptr;
	ENTER();
	switch (color)
	{
		case RED:
			if (GTK_IS_BUTTON(widget))
			{
				gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(widget)),
						GTK_STATE_NORMAL,&red);
				gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(widget)),
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
				gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(widget)),
						GTK_STATE_NORMAL,&black);
				gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(widget)),
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
				gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(widget)),
						GTK_STATE_NORMAL,&green);
				gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(widget)),
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
	EXIT();
	return;
}


/*!
  \brief update_logbar() updates the logbar passed with the text passed to it
  \param view_name is the textual name of the textview the text is supposed 
  to go to. (required)
  \param tag_name is the textual tag name to be used to set attributes on the
  text (optional, can be NULL)
  \param message is the message to display (required)
  \param count is the flag to show a running count or not
  \param clear if set, clear display before displaying text
  \param free if set, free the message passed to it
  */
G_MODULE_EXPORT void  update_logbar(
		const gchar * view_name, 
		const gchar * tag_name, 
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

	ENTER();
	g_return_if_fail(message);
	widget = (GtkWidget *)lookup_widget(view_name);

	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return;
	}

	if (!GTK_IS_WIDGET(widget))
	{
		MTXDBG(CRITICAL,_("Textview name passed: \"%s\" wasn't registered, not updating\n"),view_name);
		EXIT();
		return;
	}

	/* Add the message to the end of the textview */
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
	if (!textbuffer)
	{
		EXIT();
		return;
	}

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

	if (tag_name == NULL)
	{
		if (count) /* if TRUE, display counter, else don't */
			gtk_text_buffer_insert(textbuffer,&iter,tmpbuf,-1);
		gtk_text_buffer_insert(textbuffer,&iter,(const gchar *)message,-1);
	}
	else
	{
		if (count) /* if TRUE, display counter, else don't */
			gtk_text_buffer_insert_with_tags_by_name(textbuffer,
					&iter,tmpbuf,-1,tag_name,NULL);

		gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,
				(const gchar *)message,-1,tag_name,NULL);
	}

	/* Get it's parent (the scrolled window) and slide it to the
	 * bottom so the new message is visible... 
	 */
	parent = gtk_widget_get_parent(widget);
	if (GTK_IS_WIDGET(parent))
	{
		adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(parent));
		gtk_adjustment_set_value(adj,gtk_adjustment_get_upper(adj));
	}

	if (tmpbuf)
		g_free(tmpbuf);
	if (free)
		g_free(message);
	EXIT();
	return;	
}


/*!
  \brief conn_warning() displays a warning message when connection is 
  either lost or not detected with the ECU
  */
G_MODULE_EXPORT void conn_warning(void)
{
	CmdLineArgs *args = (CmdLineArgs *)DATA_GET(global_data,"args");

	ENTER();
	if ((args->be_quiet) || (warning_present))
	{
		EXIT();
		return;
	}
	warn_user(_("The ECU appears to be currently disconnected.  This means that either one of the following occurred:\n   1. MegaTunix couldn't determine the correct comm port to use\n   2. The serial link is not plugged in\n   3. The ECU does not have adequate power.\n   4. The ECU is in bootloader mode.\n\nTry power cycling your ECU and verifying all connections are in order."));
	EXIT();
	return;
}


/*!
  \brief kill_conn_warning() removes the no connection warning message.
  Takes no parameters.
  */
G_MODULE_EXPORT void kill_conn_warning(void)
{
	ENTER();
	if (!warning_present)
	{
		EXIT();
		return;
	}
	if (GTK_IS_WIDGET(warning_dialog))
	{
		gtk_widget_destroy(warning_dialog);
		warning_dialog = NULL;
		warning_present = FALSE;
	}
	EXIT();
	return;
}


/*!
  \brief warn_user() displays a warning message on the screen as a error dialog
  \param message is the text to display
  */
G_MODULE_EXPORT void warn_user(const gchar *message)
{
	CmdLineArgs *args = (CmdLineArgs *)DATA_GET(global_data,"args");
	ENTER();
	if ((args->be_quiet))
	{
		EXIT();
		return;
	}

	warning_dialog = gtk_message_dialog_new((GtkWindow *)lookup_widget("main_window"),(GtkDialogFlags)0,GTK_MESSAGE_ERROR,GTK_BUTTONS_NONE,"%s",message);

	if (!DATA_GET(global_data,"interrogated"))
		gtk_dialog_add_buttons(GTK_DIALOG(warning_dialog),(const gchar *)"Exit Megatunix",GTK_RESPONSE_CLOSE,(const gchar *)"Go to Offline mode", GTK_RESPONSE_ACCEPT,NULL);
	else
		gtk_dialog_add_buttons(GTK_DIALOG(warning_dialog),"_Close", GTK_RESPONSE_CANCEL,NULL);


	g_signal_connect (G_OBJECT(warning_dialog),
			"response",
			G_CALLBACK (get_response),
			NULL);

	g_signal_connect_swapped (G_OBJECT(warning_dialog),
			"response",
			G_CALLBACK (gtk_widget_destroy),
			warning_dialog);

	warning_present = TRUE;
	gtk_widget_show_all(warning_dialog);
	EXIT();
	return;
}



/*!
  \brief error_msg() displays a warning message on the screen as a error dialog
  \param message is the text to display
  */
G_MODULE_EXPORT void error_msg(const gchar *message)
{
	GtkWidget *dialog = NULL;
	gint result = 0;
	ENTER();
	dialog = gtk_message_dialog_new((GtkWindow *)lookup_widget("main_window"),(GtkDialogFlags)0,GTK_MESSAGE_ERROR,GTK_BUTTONS_NONE,"%s",message);

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
	EXIT();
	return;
}


/*!
  \brief Retrieves the response from the offline mode dialog box choice
  \param widget is the widget the user clicked on
  \param data is the response as an integer
  \returns TRUE
  */
G_MODULE_EXPORT void get_response(GtkWidget *widget, gint response, gpointer data)
{
	ENTER();
	if (response == GTK_RESPONSE_ACCEPT)
	{
		set_title(g_strdup(_("Offline Mode...")));
		set_offline_mode();
	}
	if (response == GTK_RESPONSE_CLOSE)
		leave(NULL,NULL);
	/* This might need a timeout (1500ms) +idle to delay */
	set_warning_flag(NULL);
	EXIT();
	return;
}


/*!
  \brief wrapper for reset_infolabel()  to run via g_idle_add
  */
G_MODULE_EXPORT gboolean reset_infolabel_wrapper(gpointer data)
{
	ENTER();
	g_idle_add(reset_infolabel,data);
	EXIT();
	return FALSE;
}

/*!
  \brief reset_infolabel() resets infolabel text to "Ready"
  \param data is unused
  \returns FALSE
  */
G_MODULE_EXPORT gboolean reset_infolabel(gpointer data)
{
	static GtkWidget *info_label = NULL;
	ENTER();
	if (!info_label)
		info_label = (GtkWidget *)lookup_widget("info_label");
	if (GTK_IS_WIDGET(info_label))
		gtk_label_set_markup(GTK_LABEL(info_label),"<b>Ready...</b>");
	EXIT();
	return FALSE;
}

/*!
  \brief set_title() appends text to the titlebar of the application to
  give user notifications...
  \param text is the text to append, dynamic strings only
  */
G_MODULE_EXPORT void set_title(gchar * text)
{
	gchar * tmpbuf = NULL;
	static GtkWidget *info_label = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if ((!lookup_widget("main_window")) || (DATA_GET(global_data,"leaving")))
	{
		EXIT();
		return;
	}
	if (!info_label)
		info_label = (GtkWidget *)lookup_widget("info_label");

	if (firmware)
	{
		if (firmware->actual_signature)
			tmpbuf = g_strdup_printf("MegaTunix %s,   (%s)   %s",GIT_COMMIT,firmware->actual_signature,text);
		else
			tmpbuf = g_strconcat("MegaTunix ",GIT_COMMIT,",   ",text,NULL);
	}
	else
		tmpbuf = g_strconcat("MegaTunix ",GIT_COMMIT,",   ",text,NULL);

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
	EXIT();
	return;
}


/*!
  \brief enables the static warning present flag
  \param user_data is unused
  \returns FALSE
  */
G_MODULE_EXPORT gboolean set_warning_flag(gpointer user_data)
{
	ENTER();
	warning_present = FALSE;
	EXIT();
	return FALSE;
}
