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

#include <3d_vetable.h>
#include <comms.h>
#include <config.h>
#include <dataio.h>
#include <datamgmt.h>
#include <defines.h>
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

	dbg_func(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\t Entered...\n"));
	if (!serial_params)
		return FALSE;

	dbg_func(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tRequesting ECU Clock (\"C\" cmd)\n"));
	if (!write_wrapper(serial_params->fd,"C",1,&len))
	{
		err_text = (gchar *)g_strerror(errno);
		dbg_func(SERIAL_RD|CRITICAL,g_strdup_printf(__FILE__": comms_test()\n\tError writing \"C\" to the ecu, ERROR \"%s\" in comms_test()\n",err_text));
		thread_update_logbar("comms_view","warning",g_strdup_printf("Error writing \"C\" to the ecu, ERROR \"%s\" in comms_test()\n",err_text),FALSE,FALSE);
		connected = FALSE;
		return connected;
	}
	result = read_data(1,NULL,FALSE);
	if (!result) /* Failure,  Attempt MS-II method */
	{
		if (!write_wrapper(serial_params->fd,"c",1,&len))
		{
			err_text = (gchar *)g_strerror(errno);
			dbg_func(SERIAL_RD|CRITICAL,g_strdup_printf(__FILE__": comms_test()\n\tError writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n",err_text));
			thread_update_logbar("comms_view","warning",g_strdup_printf("Error writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n",err_text),FALSE,FALSE);
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
		queue_function(g_strdup("kill_conn_warning"));
		thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup("ECU Connected..."));
		thread_update_logbar("comms_view","info",g_strdup_printf("ECU Comms Test Successfull\n"),FALSE,FALSE);

	}
	else
	{
		/* An I/O Error occurred with the MegaSquirt ECU  */
		connected = FALSE;
		errcount++;
		if (errcount > 5 )
			queue_function(g_strdup("conn_warning"));
		thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup_printf("COMMS ISSUES: Check COMMS tab"));
		dbg_func(SERIAL_RD|IO_PROCESS,g_strdup(__FILE__": comms_test()\n\tI/O with ECU Timeout\n"));
		thread_update_logbar("comms_view","warning",g_strdup_printf("I/O with ECU Timeout\n"),FALSE,FALSE);
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

	msg = g_new0(SlaveMessage, 1);
	msg->page = (guint8)(gint)OBJ_GET(output->object,"page");
	msg->offset = (guint16)(gint)OBJ_GET(output->object,"offset");
	msg->size = (DataSize)(gint)OBJ_GET(output->object,"size");
	msg->length = (guint16)(gint)OBJ_GET(output->object,"num_bytes");
	msg->mode = (WriteMode)OBJ_GET(output->object,"mode");
	if (msg->mode == MTX_CHUNK_WRITE)
		msg->data = g_memdup(OBJ_GET(output->object,"data"), msg->length);
	else if (msg->mode == MTX_SIMPLE_WRITE)
		msg->value = (gint)OBJ_GET(output->object,"value");
	else
	{
		printf("Non simple/chunk write command, not notifying slaves\n");
		g_free(msg);
		return;
	}

	/*	printf("Sending message to slave(s)\n");*/
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
	gint z = 0;
	gint page = 0;
	gint offset = 0;
	gint length = 0;
	guint8 *sent_data = NULL;
	WriteMode mode = MTX_CMD_WRITE;
	extern gboolean paused_handlers;

	if (!output)
		goto red_or_black;
	else
	{
		page = (gint)OBJ_GET(output->object,"page");
		offset = (gint)OBJ_GET(output->object,"offset");
		length = (gint)OBJ_GET(output->object,"num_bytes");
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
	paused_handlers = TRUE;

	/*printf ("page %i, offset %i\n",data->page,data->offset); */
	/*printf("WRITE STATUS, page %i, offset %i\n",page,offset);*/
	for (z=offset;z<offset+length;z++)
		refresh_widgets_at_offset(page,z);

	paused_handlers = FALSE;
	/* We check to see if the last burn copy of the VE/constants matches 
	 * the currently set, if so take away the "burn now" notification.
	 * avoid unnecessary burns to the FLASH 
	 */

red_or_black:
	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;

		if(memcmp(ecu_data_last[i],ecu_data[i],firmware->page_params[i]->length) != 0)
		{
			set_group_color(RED,"burners");
			outstanding_data = TRUE;
			return;
		}
	}
	outstanding_data = FALSE;
	set_group_color(BLACK,"burners");
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
		canID = (gint)OBJ_GET(output->object,"canID");
		page = (gint)OBJ_GET(output->object,"page");
		offset = (gint)OBJ_GET(output->object,"offset");
		size = (DataSize)OBJ_GET(output->object,"size");
		value = (gint)OBJ_GET(output->object,"value");
		data = (guint8 *)OBJ_GET(output->object,"data");
		num_bytes = (gint)OBJ_GET(output->object,"num_bytes");
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
				g_usleep(block->arg);
			}
		}
		else if (block->type == DATA)
		{
	/*		printf("Block type of DATA!\n");*/
			for (j=0;j<block->len;j++)
			{
				/*printf("comms.c data[%i] is %i\n",j,block->data[j]);*/
				if (i == 0)
					dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%i\", (\"%c\")\n",i,j+1,block->len,block->data[j], (gchar)block->data[j]));
				else
					dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%i\"\n",i,j+1,block->len,block->data[j]));
				/*printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%i\"\n",i,j+1,block->len,block->data[j]);*/
				res = write_wrapper(serial_params->fd,&(block->data[j]),1, &len);	/* Send write command */
				if (!res)
				{
					dbg_func(SERIAL_WR|CRITICAL,g_strdup_printf(__FILE__": write_data()\n\tError writing block  offset %i, value %i ERROR \"%s\"!!!\n",j,block->data[j],err_text));
					retval = FALSE;
				}
				if (firmware->capabilities & MS2)
					g_usleep(firmware->interchardelay*1000);
			}
		}

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


