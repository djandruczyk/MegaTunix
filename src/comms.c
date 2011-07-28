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

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */

#include <dataio.h>
#include <comms.h>
#include <debugging.h>
#include <defines.h>
#include <firmware.h>
#include <notifications.h>
#include <plugin.h>
#include <serialio.h>
#include <stdio.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <widgetmgmt.h>

extern gconstpointer *global_data;

/*!
 \brief Updates the Gui with the results of the comms
 test.  This is decoupled from the comms_test due to threading constraints.
 \see comms_test
 */

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT void update_comms_status(void)
{
	GtkWidget *widget = NULL;

	if (NULL != (widget = lookup_widget("runtime_connected_label")))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),(GBOOLEAN)DATA_GET(global_data,"connected"));
	if (NULL != (widget = lookup_widget("ww_connected_label")))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),(GBOOLEAN)DATA_GET(global_data,"connected"));
	return;
}


/*!
 \brief write_data() physically sends the data to the ECU.
 \param message (Io_Message *) a pointer to an Io_Message structure
 */

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT gboolean write_data(Io_Message *message)
{
	static GMutex *serio_mutex = NULL;
	static Serial_Params *serial_params = NULL;
	static Firmware_Details *firmware = NULL;
	static gfloat *factor = NULL;
	OutputData *output = message->payload;
	gint res = 0;
	gchar * err_text = NULL;
	guint i = 0;
	gint j = 0;
	gint len = 0;
	gboolean notifies = FALSE;
	gint notif_divisor = 32;
	WriteMode mode = MTX_CMD_WRITE;
	gboolean retval = TRUE;
	DBlock *block = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	static void (*store_new_block)(gpointer) = NULL;
	static void (*set_ecu_data)(gpointer,gint *) = NULL;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	if (!serial_params)
		serial_params = DATA_GET(global_data,"serial_params");
	if (!serio_mutex)
		serio_mutex = DATA_GET(global_data,"serio_mutex");
	if (!factor)
		factor = DATA_GET(global_data,"sleep_correction");
	if (!set_ecu_data)
		get_symbol("set_ecu_data",(void*)&set_ecu_data);
	if (!store_new_block)
		get_symbol("store_new_block",(void*)&store_new_block);

	g_return_val_if_fail(firmware,FALSE);
	g_return_val_if_fail(serial_params,FALSE);
	g_return_val_if_fail(serio_mutex,FALSE);
	g_return_val_if_fail(factor,FALSE);
	g_return_val_if_fail(set_ecu_data,FALSE);
	g_return_val_if_fail(store_new_block,FALSE);

	g_static_mutex_lock(&mutex);
	g_mutex_lock(serio_mutex);

	if (output)
		mode = (WriteMode)DATA_GET(output->data,"mode");

	if (DATA_GET(global_data,"offline"))
	{
		switch (mode)
		{
			case MTX_SIMPLE_WRITE:
				set_ecu_data(output->data,NULL);
				break;
			case MTX_CHUNK_WRITE:
				store_new_block(output->data);
				break;
			case MTX_CMD_WRITE:
				break;
		}
		g_mutex_unlock(serio_mutex);
		g_static_mutex_unlock(&mutex);
		return TRUE;		/* can't write anything if offline */

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
	}
	if (!DATA_GET(global_data,"connected"))
	{
		g_mutex_unlock(serio_mutex);
		g_static_mutex_unlock(&mutex);
		return FALSE;		/* can't write anything if disconnected */

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
	}

	for (i=0;i<message->sequence->len;i++)
	{
		block = g_array_index(message->sequence,DBlock *,i);
		/*	printf("Block pulled\n");*/

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
		if (block->type == ACTION)
		{
			/*		printf("Block type of ACTION!\n");*/

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
			if (block->action == SLEEP)
			{
				/*			printf("Sleeping for %i usec\n", block->arg);*/

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
				dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tSleeping for %i microseconds \n",block->arg));
				g_usleep((*factor)*block->arg);
			}
		}
		else if (block->type == DATA)
		{
			/*		printf("Block type of DATA!\n");*/

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
			if (block->len > 100)
				notifies = TRUE;
			for (j=0;j<block->len;j++)
			{
				/*printf("comms.c data[%i] is %i\n",j,block->data[j]);*/

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
				if ((notifies) && ((j % notif_divisor) == 0))
					thread_update_widget("info_label",MTX_LABEL,g_strdup_printf(_("<b>Sending %i of %i bytes</b>"),j,block->len));
				if (i == 0)
					dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%.2X\", (\"%c\")\n",i,j+1,block->len,block->data[j], (gchar)block->data[j]));
				else
					dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%.2X\"\n",i,j+1,block->len,block->data[j]));
				/*printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%i\"\n",i,j+1,block->len,block->data[j]);*/

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
				res = write_wrapper(serial_params->fd,&(block->data[j]),1, &len);	/* Send write command */

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
				if (!res)
				{
					dbg_func(SERIAL_WR|CRITICAL,g_strdup_printf(__FILE__": write_data()\n\tError writing block offset %i, value %i ERROR \"%s\"!!!\n",j,block->data[j],err_text));
					retval = FALSE;
				}
				if (firmware->capabilities & MS2)
					g_usleep((*factor)*firmware->interchardelay*1000);
			}
		}
	}
	if (notifies)
	{
		thread_update_widget("info_label",MTX_LABEL,g_strdup("<b>Transfer Completed</b>"));
		gdk_threads_add_timeout(2000,(GSourceFunc)reset_infolabel,NULL);
	}
	/* If sucessfull update ecu_data as well, this way, current 
	 * and pending match, in the case of a failed write, the 
	 * update_write_status() function will catch it and rollback as needed
	 */

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
	if ((output) && (retval))
	{
		if (mode == MTX_SIMPLE_WRITE)
			set_ecu_data(output->data,NULL);
		else if (mode == MTX_CHUNK_WRITE)
			store_new_block(output->data);
	}

	g_mutex_unlock(serio_mutex);
	g_static_mutex_unlock(&mutex);
	return retval;
}


/*!
  \brief Enumerates the contents of /dev to be used when looking for a 
  serial port device on Linux or OS-X
  \param widget, unused
  \param data, unused
  \returns TRUE
  */

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT gboolean enumerate_dev(GtkWidget *widget, gpointer data)
{
#ifndef __WIN32__
	gint i = 0;
	gint result = 0;
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

	dialog = gtk_message_dialog_new (top,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			"Going to start serial port scan, please make sure USB/Serial adapter is unplugged, then click on the \"OK\" Button");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	/* Read list of dev before device plugin */

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
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

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */

	/* Read list of dev AFTER device plugin */

/*! @file comms.c
 *
 * @brief ...
 *
 *
 */
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
			check = gtk_check_button_new_with_label(g_list_nth_data(found,i));
			gtk_box_pack_start(GTK_BOX(parent),check,TRUE,TRUE,0);
			buttons = g_list_append(buttons,check);
		}
		label = gtk_label_new(_("Please select the ports that you wish\nto be used to locate your ECU\n"));
		gtk_box_pack_start(GTK_BOX(parent),label,TRUE,TRUE,0);
		gtk_widget_show_all(parent);
		result = gtk_dialog_run (GTK_DIALOG (dialog));
		if (result == GTK_RESPONSE_APPLY)
		{
			ports = (gchar *)DATA_GET(global_data,"potential_ports");
			for (i=0;i<g_list_length(found);i++)
			{
				check = g_list_nth_data(buttons,i);
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

	return TRUE;
#else
	return TRUE;
#endif
}


/*!
  \brief Checks the potential ports config var for the existance of this
  port, if so, return TRUE else return FALSE
  \param name,  port name to search for...
  */

/*! @file comms.c
 *
 * @brief ...
 *
 *
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
	ports = (gchar *)DATA_GET(global_data,"potential_ports");
	if (g_strrstr(ports,searchstr))
		retval = TRUE;
	g_free(searchstr);
	return retval;
}
