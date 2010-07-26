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
#include <defines.h>
#include <3d_vetable.h>
#include <comms.h>
#include <dataio.h>
#include <datamgmt.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <fcntl.h>
#include <firmware.h>
#include <gui_handlers.h>
#include <init.h>
#include <notifications.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <unistd.h>
#include <widgetmgmt.h>

extern GStaticMutex serio_mutex;
extern GObject *global_data;
volatile gboolean outstanding_data = FALSE;

/*!
 \brief update_comms_status updates the Gui with the results of the comms
 test.  This is decoupled from the comms_test due to threading constraints.
 \see comms_test
 */
void update_comms_status(void)
{
	extern gboolean connected;
	GtkWidget *widget = NULL;

	if (NULL != (widget = lookup_widget("runtime_connected_label")))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),connected);
	if (NULL != (widget = lookup_widget("ww_connected_label")))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),connected);
	return;
}

/*! 
 \brief comms_test sends the clock_request command ("C") to the ECU and
 checks the response.  if nothing comes back, MegaTunix assumes the ecu isn't
 connected or powered down. NO Gui updates are done from this function as it
 gets called from a thread. update_comms_status is dispatched after this
 function ends from the main context to update the GUI.
 \see update_comms_status
 */

gint comms_test()
{
	gboolean result = FALSE;
	gchar * err_text = NULL;
	gint len = 0;
	static gint errcount = 0;
	extern Serial_Params *serial_params;
	extern gboolean connected;

/*	printf("comms test\n"); */
	dbg_func(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\t Entered...\n"));
	if (!serial_params)
		return FALSE;

	dbg_func(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tRequesting ECU Clock (\"C\" cmd)\n"));
	/*printf("sending \"C\"\n"); */
	if (!write_wrapper(serial_params->fd,"C",1,&len))
	{
		err_text = (gchar *)g_strerror(errno);
		dbg_func(SERIAL_RD|CRITICAL,g_strdup_printf(__FILE__": comms_test()\n\tError writing \"C\" to the ecu, ERROR \"%s\" in comms_test()\n",err_text));
		thread_update_logbar("comms_view","warning",g_strdup_printf(_("Error writing \"C\" to the ecu, ERROR \"%s\" in comms_test()\n"),err_text),FALSE,FALSE);
		connected = FALSE;
		return connected;
	}

/*	printf("reading \n"); */
	result = read_data(1,NULL,FALSE);
/*	printf("read %i bytes \n",result); */
	if (!result) /* Failure,  Attempt MS-II method */
	{
		if (!write_wrapper(serial_params->fd,"c",1,&len))
		{
			err_text = (gchar *)g_strerror(errno);
			dbg_func(SERIAL_RD|CRITICAL,g_strdup_printf(__FILE__": comms_test()\n\tError writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n",err_text));
			thread_update_logbar("comms_view","warning",g_strdup_printf(_("Error writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n"),err_text),FALSE,FALSE);
			connected = FALSE;
			return connected;
		}
		result = read_data(2,NULL,FALSE);
	}
	if (result)	/* Success */
	{
		connected = TRUE;
		errcount=0;
		dbg_func(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tECU Comms Test Successfull\n"));
		queue_function("kill_conn_warning");
		thread_update_widget("titlebar",MTX_TITLE,g_strdup(_("ECU Connected...")));
		thread_update_logbar("comms_view","info",g_strdup_printf(_("ECU Comms Test Successfull\n")),FALSE,FALSE);

	}
	else
	{
		/* An I/O Error occurred with the MegaSquirt ECU  */
		connected = FALSE;
		errcount++;
		if (errcount > 5 )
			queue_function("conn_warning");
		thread_update_widget("titlebar",MTX_TITLE,g_strdup_printf(_("COMMS ISSUES: Check COMMS tab")));
		dbg_func(SERIAL_RD|IO_PROCESS,g_strdup(__FILE__": comms_test()\n\tI/O with ECU Timeout\n"));
		thread_update_logbar("comms_view","warning",g_strdup_printf(_("I/O with ECU Timeout\n")),FALSE,FALSE);
	}
	return connected;
}

/*!
 \brief send_to_slaves() sends messages to a thread talking to the slave
 clients to trigger them to update their GUI with appropriate changes
 \param data (OutputData *) pointer to data sent to ECU used to
 update other widgets that refer to that Page/Offset
 */

EXPORT void send_to_slaves(void *data)
{
	Io_Message *message = (Io_Message *)data;
	OutputData *output = (OutputData *)message->payload;
	extern GAsyncQueue *slave_msg_queue;
	SlaveMessage *msg = NULL;

	if (!output) /* If no data, don't bother the slaves */
		return;
	if (!(gboolean)OBJ_GET(global_data,"network_access"))
		return;

	msg = g_new0(SlaveMessage, 1);
	msg->page = (guint8)(GINT)OBJ_GET(output->object,"page");
	msg->offset = (guint16)(GINT)OBJ_GET(output->object,"offset");
	msg->length = (guint16)(GINT)OBJ_GET(output->object,"num_bytes");
	msg->size = (DataSize)OBJ_GET(output->object,"size");
	msg->mode = (WriteMode)OBJ_GET(output->object,"mode");
	if (msg->mode == MTX_CHUNK_WRITE)
		msg->data = g_memdup(OBJ_GET(output->object,"data"), msg->length);
	else if (msg->mode == MTX_SIMPLE_WRITE)
		msg->value = (GINT)OBJ_GET(output->object,"value");
	else
	{
		printf(_("Non simple/chunk write command, not notifying peers\n"));
		g_free(msg);
		return;
	}

	/*	printf("Sending message to peer(s)\n");*/
	g_async_queue_ref(slave_msg_queue);
        g_async_queue_push(slave_msg_queue,(gpointer)msg);
        g_async_queue_unref(slave_msg_queue);
	return;
}

/*!
 \brief update_write_status() checks the differences between the current ECU
 data snapshot and the last one, if there are any differences (things need to
 be burnt) then it turns all the widgets in the "burners" group to RED
 \param data (OutputData *) pointer to data sent to ECU used to
 update other widgets that refer to that Page/Offset
 */
EXPORT void update_write_status(void *data)
{
	Io_Message *message = (Io_Message *)data;
	OutputData *output = (OutputData *)message->payload;
	extern Firmware_Details *firmware;
	guint8 **ecu_data = firmware->ecu_data;
	guint8 **ecu_data_last = firmware->ecu_data_last;
	gint i = 0;
	gint page = 0;
	gint offset = 0;
	gint length = 0;
	guint8 *sent_data = NULL;
	WriteMode mode = MTX_CMD_WRITE;
	gint z = 0;
	extern gboolean paused_handlers;
	extern volatile gboolean offline;

	if (!output)
		goto red_or_black;
	else
	{
		page = (GINT)OBJ_GET(output->object,"page");
		offset = (GINT)OBJ_GET(output->object,"offset");
		length = (GINT)OBJ_GET(output->object,"num_bytes");
		mode = (WriteMode)OBJ_GET(output->object,"mode");

		if (!message->status) /* Bad write! */
		{
			dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": update_write_status()\n\tWRITE failed, rolling back!\n"));
			memcpy(ecu_data[page]+offset, ecu_data_last[page]+offset,length);
		}
	}
	if (mode == MTX_CHUNK_WRITE)
	{
		sent_data = (guint8 *)OBJ_GET(output->object,"data");
		if (sent_data)
			g_free(sent_data);
	}

	if (output->queue_update)
	{
		paused_handlers = TRUE;
		for (i=0;i<firmware->total_tables;i++)
		{
			/* This at least only recalcs the limits on one... */
			if (((firmware->table_params[i]->x_page == page) ||
						(firmware->table_params[i]->y_page == page) ||
						(firmware->table_params[i]->z_page == page)) && (firmware->table_params[i]->color_update == FALSE))
			{
				recalc_table_limits(0,i);
				if ((firmware->table_params[i]->last_z_maxval != firmware->table_params[i]->z_maxval) || (firmware->table_params[i]->last_z_minval != firmware->table_params[i]->z_minval))
					firmware->table_params[i]->color_update = TRUE;
				else
					firmware->table_params[i]->color_update = FALSE;
			}
		}

		gdk_threads_enter();
		for (z=offset;z<offset+length;z++)
			refresh_widgets_at_offset(page,z);
		gdk_threads_leave();

		paused_handlers = FALSE;
	}
	/* We check to see if the last burn copy of the VE/constants matches 
	 * the currently set, if so take away the "burn now" notification.
	 * avoid unnecessary burns to the FLASH 
	 */

	if (offline)
		return;
red_or_black:
	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;

		if(memcmp(ecu_data_last[i],ecu_data[i],firmware->page_params[i]->length) != 0)
		{
			gdk_threads_enter();
			set_group_color(RED,"burners");
			gdk_threads_leave();
			outstanding_data = TRUE;
			return;
		}
	}
	outstanding_data = FALSE;
	gdk_threads_enter();
	set_group_color(BLACK,"burners");
	gdk_threads_leave();
	return;
}


/*!
 \brief queue_burn_ecu_flash() issues the commands to the ECU to burn the contents
 of RAM to flash.
 */
void queue_burn_ecu_flash(gint page)
{
	extern Firmware_Details * firmware;
	extern volatile gboolean offline;
	OutputData *output = NULL;

	if (offline)
		return;

	output = initialize_outputdata();
	OBJ_SET(output->object,"canID", GINT_TO_POINTER(firmware->canID));
	OBJ_SET(output->object,"page", GINT_TO_POINTER(page));
	OBJ_SET(output->object,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
	OBJ_SET(output->object,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
	io_cmd(firmware->burn_command,output);
}


/*!
 \brief write_data() physiclaly sends the data to the ECU.
 \param message (Io_Message *) a pointer to a Io_Message
 */
gboolean write_data(Io_Message *message)
{
	extern gboolean connected;
	OutputData *output = message->payload;

	gint res = 0;
	gchar * err_text = NULL;
	guint i = 0;
	gint j = 0;
	gint len = 0;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gboolean notifies = FALSE;
	gint notif_divisor = 32;
	DataSize size = MTX_U08;
	gint value = 0;
	WriteMode mode = MTX_CMD_WRITE;
	guint8 *data = NULL;
	gint num_bytes = 0;
	gboolean retval = TRUE;
	DBlock *block = NULL;
	extern Serial_Params *serial_params;
	extern Firmware_Details *firmware;
	extern volatile gboolean offline;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() before lock write_data mutex\n"));
	g_static_mutex_lock(&mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() after lock write_data mutex\n"));
	dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() before lock serio_mutex\n"));
	g_static_mutex_lock(&serio_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() after lock serio_mutex\n"));

	if (output)
	{
		canID = (GINT)OBJ_GET(output->object,"canID");
		page = (GINT)OBJ_GET(output->object,"page");
		offset = (GINT)OBJ_GET(output->object,"offset");
		value = (GINT)OBJ_GET(output->object,"value");
		num_bytes = (GINT)OBJ_GET(output->object,"num_bytes");
		size = (DataSize)OBJ_GET(output->object,"size");
		data = (guint8 *)OBJ_GET(output->object,"data");
		mode = (WriteMode)OBJ_GET(output->object,"mode");
	}
	if (offline)
	{
		/*printf ("OFFLINE writing value at %i,%i [%i]\n",page,offset,value); */
		switch (mode)
		{
			case MTX_SIMPLE_WRITE:
				set_ecu_data(canID,page,offset,size,value);
				break;
			case MTX_CHUNK_WRITE:
				store_new_block(canID,page,offset,data,num_bytes);
				break;
			case MTX_CMD_WRITE:
				break;
		}
		dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() before UNlock serio_mutex\n"));
		g_static_mutex_unlock(&serio_mutex);
		dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() after UNlock serio_mutex\n"));
		dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() before UNlock write_data mutex\n"));
		g_static_mutex_unlock(&mutex);
		dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() after UNlock write_data mutex\n"));
		return TRUE;		/* can't write anything if offline */
	}
	if (!connected)
	{
		dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() before UNlock serio_mutex\n"));
		g_static_mutex_unlock(&serio_mutex);
		dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() after UNlock serio_mutex\n"));
		dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() before UNlock write_data mutex\n"));
		g_static_mutex_unlock(&mutex);
		dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() after UNlock write_data mutex\n"));
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
					thread_update_widget("info_label",MTX_LABEL,g_strdup_printf(_("Sending %i of %i bytes"),j,block->len));
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
				if (firmware->capabilities & MS2)
					g_usleep(firmware->interchardelay*1000);
			}
		}

	}
	if (notifies)
	{
		thread_update_widget("info_label",MTX_LABEL,g_strdup("Transfer Completed"));
		gdk_threads_add_timeout(2000,(GtkFunction)reset_infolabel,NULL);
	}
	/* If sucessfull update ecu_data as well, this way, current 
	 * and pending match, in the case of a failed write, the 
	 * update_write_status() function will catch it and rollback as needed
	 */
	if ((output) && (retval))
	{
		if (mode == MTX_SIMPLE_WRITE)
			set_ecu_data(canID,page,offset,size,value);
		else if (mode == MTX_CHUNK_WRITE)
			store_new_block(canID,page,offset,data,num_bytes);
	}

	dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() before UNlock serio_mutex\n"));
	g_static_mutex_unlock(&serio_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() after UNlock serio_mutex\n"));
	dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() before UNlock write_data mutex\n"));
	g_static_mutex_unlock(&mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": write_data() after UNlock write_data mutex\n"));
	return retval;
}


EXPORT gboolean enumerate_dev(GtkWidget *widget, gpointer data)
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
			ports = (gchar *)OBJ_GET(global_data,"potential_ports");
			for (i=0;i<g_list_length(found);i++)
				ports = g_strconcat(ports,",/dev/",g_list_nth_data(found,i),NULL);
			OBJ_SET(global_data,"potential_ports",ports);
		}
	}
	for (i=0;i<g_list_length(found);i++)
		g_free(g_list_nth_data(found,i));
	g_list_free(found);
	
	return TRUE;

}
