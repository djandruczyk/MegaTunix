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
#include <dataio.h>
#include <defines.h>
#include <3d_vetable.h>
#include <comms.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <fcntl.h>
#include <firmware.h>
#include <gui_handlers.h>
#include <init.h>
#include <notifications.h>
#include <plugin.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <threads.h>
#include <timeout_handlers.h>
#include <unistd.h>
#include <widgetmgmt.h>

extern gconstpointer *global_data;

/*!
 \brief update_comms_status updates the Gui with the results of the comms
 test.  This is decoupled from the comms_test due to threading constraints.
 \see comms_test
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
 \param message (Io_Message *) a pointer to a Io_Message
 */
G_MODULE_EXPORT gboolean write_data(Io_Message *message)
{
	static GMutex *serio_mutex = NULL;
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
	Serial_Params *serial_params;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	Firmware_Details *firmware = NULL;
	static void (*store_new_block)(gconstpointer *) = NULL;
	static void (*set_ecu_data)(gconstpointer *) = NULL;

	firmware = DATA_GET(global_data,"firmware");
	serial_params = DATA_GET(global_data,"serial_params");
	if (!serio_mutex)
		serio_mutex = DATA_GET(global_data,"serio_mutex");
	if (!set_ecu_data)
		get_symbol("set_ecu_data",(void*)&set_ecu_data);
	if (!set_ecu_data)
		dbg_func(CRITICAL|SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tFunction pointer for \"set_ecu_data\" was NOT found in plugins, BUG!\n"));
	if (!store_new_block)
		get_symbol("store_new_block",(void*)&store_new_block);
	if (!store_new_block)
		dbg_func(CRITICAL|SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tFunction pointer for \"store_new_block\" was NOT found in plugins, BUG!\n"));

	g_static_mutex_lock(&mutex);
	g_mutex_lock(serio_mutex);

	if (output)
		mode = (WriteMode)DATA_GET(output->data,"mode");

	if (DATA_GET(global_data,"offline"))
	{
		switch (mode)
		{
			case MTX_SIMPLE_WRITE:
				set_ecu_data(output->data);
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
	}
	if (!DATA_GET(global_data,"connected"))
	{
		g_mutex_unlock(serio_mutex);
		g_static_mutex_unlock(&mutex);
		return FALSE;		/* can't write anything if disconnected */
	}

	for (i=0;i<message->sequence->len;i++)
	{
		block = g_array_index(message->sequence,DBlock *,i);
		/*	printf("Block pulled\n");*/
		if (block->type == ACTION)
		{
			/*		printf("Block type of ACTION!\n");*/
			if (block->action == SLEEP)
			{
				/*			printf("Sleeping for %i usec\n", block->arg);*/
				dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tSleeping for %i microseconds \n",block->arg));
				g_usleep(block->arg);
			}
		}
		else if (block->type == DATA)
		{
			/*		printf("Block type of DATA!\n");*/
			if (block->len > 100)
				notifies = TRUE;
			for (j=0;j<block->len;j++)
			{
				/*printf("comms.c data[%i] is %i\n",j,block->data[j]);*/
				if ((notifies) && ((j % notif_divisor) == 0))
					thread_update_widget("info_label",MTX_LABEL,g_strdup_printf(_("<b>Sending %i of %i bytes</b>"),j,block->len));
				if (i == 0)
					dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%i\", (\"%c\")\n",i,j+1,block->len,block->data[j], (gchar)block->data[j]));
				else
					dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%i\"\n",i,j+1,block->len,block->data[j]));
				/*printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%i\"\n",i,j+1,block->len,block->data[j]);*/
				res = write_wrapper(serial_params->fd,&(block->data[j]),1, &len);	/* Send write command */
				if (!res)
				{
					dbg_func(SERIAL_WR|CRITICAL,g_strdup_printf(__FILE__": write_data()\n\tError writing block offset %i, value %i ERROR \"%s\"!!!\n",j,block->data[j],err_text));
					retval = FALSE;
				}
				if (firmware)
					if (firmware->capabilities & MS2)
						g_usleep(firmware->interchardelay*1000);
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
	if ((output) && (retval))
	{
		if (mode == MTX_SIMPLE_WRITE)
			set_ecu_data(output->data);
		else if (mode == MTX_CHUNK_WRITE)
			store_new_block(output->data);
	}

	g_mutex_unlock(serio_mutex);
	g_static_mutex_unlock(&mutex);
	return retval;
}


G_MODULE_EXPORT gboolean enumerate_dev(GtkWidget *widget, gpointer data)
{
	gint i = 0;
	gint result = 0;
	GDir *a_dir = NULL;
	GDir *b_dir = NULL;
	GList *found = NULL;
	GHashTable *hash = NULL;
	const gchar * entry = NULL;
	gchar *ports = NULL;
	GtkWidget *dialog = NULL;
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
			if (!g_hash_table_lookup(hash,(gconstpointer)entry))
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
		entry = g_strdup("MegaTunix detected the following devices:\n\n");
		for (i=0;i<g_list_length(found);i++)
			entry = g_strconcat(entry,g_list_nth_data(found,i),"\n",NULL);
		entry = g_strconcat(entry,"\nWould you like to add these to the ports to be scanned?",NULL);
		dialog = gtk_message_dialog_new (top,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				entry,NULL);
		result = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (result == GTK_RESPONSE_YES)
		{
			ports = (gchar *)DATA_GET(global_data,"potential_ports");
			for (i=0;i<g_list_length(found);i++)
				ports = g_strconcat(ports,",/dev/",g_list_nth_data(found,i),NULL);
			DATA_SET_FULL(global_data,"potential_ports",ports,g_free);
		}
	}
	for (i=0;i<g_list_length(found);i++)
		g_free(g_list_nth_data(found,i));
	g_list_free(found);
	
	return TRUE;

}

