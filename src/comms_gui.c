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
  \file src/comms_gui.c
  \ingroup CoreMtx
  \brief Holds handlers for updating/resetting error counters related to
  communications as well as the /dev enumeration functions for Unix class
  OS's
  \author David Andruczyk
  */

#include <comms_gui.h>
#include <debugging.h>
#include <serialio.h>
#include <widgetmgmt.h>

extern GdkColor black;
extern GdkColor white;
extern GdkColor red;
extern GdkColor black;
extern gconstpointer *global_data;


/*!
  \brief reset_errcounts() resets the error counters
  \param widget is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean reset_errcounts(GtkWidget *widget)
{
	Serial_Params *serial_params = NULL;

	ENTER();

	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	DATA_SET(global_data,"ve_goodread_count",GINT_TO_POINTER(0));
	DATA_SET(global_data,"rt_goodread_count",GINT_TO_POINTER(0));
	DATA_SET(global_data,"reset_count",GINT_TO_POINTER(0));
	EXIT();
	return TRUE;
}


/*
 * \brief wrapper for update_errcounts
 * */
G_MODULE_EXPORT gboolean update_errcounts_wrapper(gpointer data)
{
	ENTER();
	g_idle_add(update_errcounts,data);
	EXIT();
	return TRUE;
}

/*!
  \brief update_errcounts() updates the text entries on the gui with the 
  current status, error and I/O counters
  \returns TRUE
  */
G_MODULE_EXPORT gboolean update_errcounts(gpointer data)
{
	static gboolean pf_red = FALSE;
	gchar *tmpbuf = NULL;
	gint tmp = 0;
	GtkWidget * widget = NULL;
	Serial_Params *serial_params = NULL;

	ENTER();

	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	tmpbuf = g_strdup_printf("%i",(GINT)DATA_GET(global_data,"ve_goodread_count"));
	widget = lookup_widget("runtime_good_ve_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("comms_vecount_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",(GINT)DATA_GET(global_data,"rt_goodread_count"));
	widget = lookup_widget("comms_rtcount_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("runtime_good_rt_read_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",(GINT)DATA_GET(global_data,"reset_count"));
	widget = lookup_widget("comms_reset_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("runtime_hardreset_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",serial_params->errcount);
	widget = lookup_widget("comms_sioerr_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("runtime_sioerr_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	EXIT();
	return FALSE;
}


/*!
  \brief Enumerates the contents of /dev to be used when looking for a 
  serial port device on Linux or OS-X
  \param widget is unused
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean enumerate_dev(GtkWidget *widget, gpointer data)
{
#ifndef __WIN32__
	guint i = 0;
	GDir *a_dir = NULL;
	GDir *b_dir = NULL;
	GList *found = NULL;
	GHashTable *hash = NULL;
	const gchar * entry = NULL;
	gchar *ports = NULL;
	gchar *tmpbuf = NULL;
	GtkWidget *dialog = NULL;
	GtkWidget *check = NULL;
	GtkWidget *label = NULL;
	GtkWidget *parent = NULL;
	GList *buttons = NULL;
	GError *err = NULL;
	GtkWindow *top = (GtkWindow *)lookup_widget("main_window");

	ENTER();

	dialog = gtk_message_dialog_new (top,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			"Going to start serial port scan, please make sure USB/Serial adapter is unplugged, then click on the \"OK\" Button");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	/* Read list of dev before device plugin */
	b_dir = g_dir_open("/dev",0,&err);
	if (b_dir)
	{
		hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		while ((entry = g_dir_read_name(b_dir)) != NULL)
			g_hash_table_insert(hash,g_strdup(entry),GINT_TO_POINTER(1));
	}

	dialog = gtk_message_dialog_new (top,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			"Please plugin the USB->serial adapter and click the \"OK\" Button");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_usleep(1000000);	/* 1 Sec */

	/* Read list of dev AFTER device plugin */
	a_dir = g_dir_open("/dev",0,&err);
	if (a_dir)
	{
		while ((entry = g_dir_read_name(a_dir)) != NULL)
			if ((!g_hash_table_lookup(hash,(gconstpointer)entry)) && (!g_strrstr(entry,"tty.")) && (!check_potential_ports(entry)))
				found = g_list_prepend(found,g_strdup(entry));
	}
	g_hash_table_destroy(hash);
	g_dir_close(a_dir);
	g_dir_close(b_dir);
	if (g_list_length(found) == 0)
	{
		dialog = gtk_message_dialog_new (top,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING,
				GTK_BUTTONS_OK,
				"No new devices were detected! You may wish to check if you need to install a driver first for the device.  Google is probably your best place to start.");

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}
	else
	{
		dialog = gtk_dialog_new_with_buttons ("Ports Detected!",
				top,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_STOCK_APPLY,
				GTK_RESPONSE_APPLY,
				NULL);
		parent = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
		gtk_container_set_border_width(GTK_CONTAINER(parent),5);
		label = gtk_label_new("MegaTunix detected the following devices:");
		gtk_box_pack_start(GTK_BOX(parent),label,FALSE,TRUE,0);
		for (i=0;i<g_list_length(found);i++)
		{
			check = gtk_check_button_new_with_label((const gchar *)g_list_nth_data(found,i));
			gtk_box_pack_start(GTK_BOX(parent),check,TRUE,TRUE,0);
			buttons = g_list_append(buttons,check);
		}
		label = gtk_label_new(_("Please select the ports that you wish\nto be used to locate your ECU\n"));
		gtk_box_pack_start(GTK_BOX(parent),label,TRUE,TRUE,0);
		gtk_widget_show_all(parent);
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));
		if (result == GTK_RESPONSE_APPLY)
		{
			ports = (gchar *)DATA_GET(global_data,"potential_ports");
			for (i=0;i<g_list_length(found);i++)
			{
				check = (GtkWidget *)g_list_nth_data(buttons,i);
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)))
					tmpbuf = g_strdup_printf("/dev/%s,%s",(gchar *)g_list_nth_data(found,i),ports);
			}
			DATA_SET_FULL(global_data,"potential_ports",tmpbuf,g_free);
		}
		gtk_widget_destroy (dialog);
	}
	for (i=0;i<g_list_length(found);i++)
		g_free(g_list_nth_data(found,i));
	g_list_free(found);

	EXIT();
	return TRUE;
#else
	EXIT();
	return TRUE;
#endif
}


/*!
  \brief Checks the potential ports config var for the existance of this
  port, if so, return TRUE else return FALSE
  \param name is the port name to search for...
  */
gboolean check_potential_ports(const gchar *name)
{
	gchar * ports = NULL;
	gboolean retval = FALSE;
#ifdef __WIN32__
	gchar *searchstr = g_strdup_printf("%s",name);
#else
	gchar *searchstr = g_strdup_printf("/dev/%s",name);
#endif

	ENTER();

	ports = (gchar *)DATA_GET(global_data,"potential_ports");
	if (g_strrstr(ports,searchstr))
		retval = TRUE;
	g_free(searchstr);
	EXIT();
	return retval;
}
