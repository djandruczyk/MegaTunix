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
#include <termios.h>
#include <unistd.h>
#include <3d_vetable.h>

extern GStaticMutex serio_mutex;
extern gint dbg_lvl;
extern GObject *global_data;

/*!
 \brief update_comms_status updates the Gui with the results of the comms
 test.  This is decoupled from the comms_test due to threading constraints.
 \see comms_test
 */
void update_comms_status(void)
{
	extern gboolean connected;
	extern GHashTable *dynamic_widgets;
	GtkWidget *widget = NULL;

	if (NULL != (widget = g_hash_table_lookup(dynamic_widgets,"runtime_connected_label")))
		gtk_widget_set_sensitive(GTK_WIDGET(widget),connected);
	if (NULL != (widget = g_hash_table_lookup(dynamic_widgets,"ww_connected_label")))
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
	static gint errcount = 0;
	extern Serial_Params *serial_params;
	extern gboolean connected;

	if (dbg_lvl & SERIAL_RD)
		dbg_func(g_strdup(__FILE__": comms_test()\n\t Entered...\n"));
	if (!serial_params)
		return FALSE;

	if (dbg_lvl & SERIAL_RD)
		dbg_func(g_strdup(__FILE__": comms_test()\n\tRequesting ECU Clock (\"C\" cmd)\n"));
	if (write(serial_params->fd,"C",1) != 1)
	{
		err_text = (gchar *)g_strerror(errno);
		if (dbg_lvl & (SERIAL_RD|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": comms_test()\n\tError writing \"C\" to the ecu, ERROR \"%s\" in comms_test()\n",err_text));
		thread_update_logbar("comms_view","warning",g_strdup_printf("Error writing \"C\" to the ecu, ERROR \"%s\" in comms_test()\n",err_text),FALSE,FALSE);
		connected = FALSE;
		return connected;
	}
	result = read_data(1,NULL);
	if (!result) /* Failure,  Attempt MS-II method */
	{
		if (write(serial_params->fd,"c",1) != 1)
		{
			err_text = (gchar *)g_strerror(errno);
			if (dbg_lvl & (SERIAL_RD|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": comms_test()\n\tError writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n",err_text));
			thread_update_logbar("comms_view","warning",g_strdup_printf("Error writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n",err_text),FALSE,FALSE);
			connected = FALSE;
			return connected;
		}
		result = read_data(2,NULL);
	}
	if (result)	/* Success */
	{
		connected = TRUE;
		errcount=0;
		if (dbg_lvl & SERIAL_RD)
			dbg_func(g_strdup(__FILE__": comms_test()\n\tECU Comms Test Successfull\n"));
		queue_function(g_strdup("kill_conn_warning"));
		thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup("ECU Connected..."));
		thread_update_logbar("comms_view","info",g_strdup_printf("ECU Comms Test Successfull\n"),FALSE,FALSE);

	}
	else
	{
		/* An I/O Error occurred with the MegaSquirt ECU  */
		connected = FALSE;
		errcount++;
		if (errcount > 2 )
			queue_function(g_strdup("conn_warning"));
		thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup_printf("COMMS ISSUES: Check COMMS tab"));
		if (dbg_lvl & (SERIAL_RD|IO_PROCESS))
			dbg_func(g_strdup(__FILE__": comms_test()\n\tI/O with ECU Timeout\n"));
		thread_update_logbar("comms_view","warning",g_strdup_printf("I/O with ECU Timeout\n"),FALSE,FALSE);
	}
	return connected;
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
	guint8 *sent_data = NULL;
	WriteMode mode = MTX_CMD_WRITE;
	extern GList ***ve_widgets;
	extern gboolean paused_handlers;


	if (!output)
		return;

	mode = (WriteMode)OBJ_GET(output->object,"mode");
	if (mode == MTX_CHUNK_WRITE)
	{
		sent_data = (guint8 *)OBJ_GET(output->object,"data");
		if (sent_data)
			g_free(sent_data);
		return;
	}
	page = (gint)OBJ_GET(output->object,"page");
	offset = (gint)OBJ_GET(output->object,"offset");
	paused_handlers = TRUE;

	/*printf ("page %i, offset %i\n",data->page,data->offset); */
	for (i=0;i<g_list_length(ve_widgets[page][offset]);i++)
	{
		if ((gint)OBJ_GET(g_list_nth_data(ve_widgets[page][offset],i),"dl_type") != DEFERRED)
		{
			/*printf("updating widget %s\n",(gchar *)glade_get_widget_name(g_list_nth_data(ve_widgets[page][offset],i))); */
			update_widget(g_list_nth_data(ve_widgets[page][offset],i),NULL);
		}
		/*	else
		printf("NOT updating widget %s because it's defered\n",(gchar *)glade_get_widget_name(g_list_nth_data(ve_widgets[page][offset],i)));
		*/
	}

	update_ve3d_if_necessary(page,offset);

	paused_handlers = FALSE;
	/* We check to see if the last burn copy of the VE/constants matches 
	 * the currently set, if so take away the "burn now" notification.
	 * avoid unnecessary burns to the FLASH 
	 */

	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;

		if(memcmp(ecu_data_last[i],ecu_data[i],firmware->page_params[i]->length) != 0)
		{
			set_group_color(RED,"burners");
			return;
		}
	}
	set_group_color(BLACK,"burners");

	return;
}


/*!
 \brief burn_ecu_flash() issues the commands to the ECU to burn the contents
 of RAM to flash.
 */
void burn_ms1_ecu_flash()
{
	Io_Message *message = NULL;
	Command *command = NULL;
	GHashTable *commands_hash = NULL;
	extern Firmware_Details * firmware;
	extern volatile gboolean offline;
	extern GAsyncQueue *pf_dispatch_queue;

	if (offline)
		return;
	commands_hash = OBJ_GET(global_data,"commands_hash");
	command = g_hash_table_lookup(commands_hash,firmware->burn_command);
	message = initialize_io_message();
	message->command = command;
	build_output_string(message,command,NULL);
	write_data(message);
	g_async_queue_ref(pf_dispatch_queue);
	g_async_queue_push(pf_dispatch_queue,(gpointer)message);
	g_async_queue_unref(pf_dispatch_queue);

}



/*!
 \brief set_ms_page() is called to change the current page being accessed in
 the firmware. set_ms_page will check to see if any outstanding data has 
 been sent to the current page, but NOT burned to flash befpre changing pages
 in that case it will burn the flash before changing the page. 
 \param ms_page (guint8) the page to set to
 */
void set_ms_page(guint8 ms_page)
{
	extern Serial_Params *serial_params;
	extern Firmware_Details *firmware;
	guint8 **ecu_data = firmware->ecu_data;
	guint8 **ecu_data_last = firmware->ecu_data_last;
	extern gboolean force_page_change;
	static gint last_page = -1;
	gint res = 0;
	gchar * err_text = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	/*printf("fed_page %i, last_page %i\n",ms_page,last_page); */
	g_static_mutex_lock(&serio_mutex);
	g_static_mutex_lock(&mutex);

	/* put int to make sure page is SET on FIRST RUN after interrogation.
	 * Found that wihtout it was getting a corrupted first page
	 */
	if (last_page == -1)
		goto force_change;

	/* If current page is NOT a dl_by_default page, and last page WAS,
	 * then force a burn, otherwise data will be lost. 
	 */
	if ((ms_page > firmware->ro_above) && (last_page <= firmware->ro_above))
	{
		g_static_mutex_unlock(&serio_mutex);
		burn_ms1_ecu_flash();
		g_static_mutex_lock(&serio_mutex);
		goto force_change;
	}
	/* If current OR last page is NOT a dl_by_default page,  then
	 * skip burning and move on. 
	 */
	if ((ms_page > firmware->ro_above) || (last_page > firmware->ro_above))
		goto skip_change;

	if (((ms_page != last_page) && (((memcmp(ecu_data_last[last_page],ecu_data[last_page],firmware->page_params[last_page]->length) != 0)) || ((memcmp(ecu_data_last[ms_page],ecu_data[ms_page],firmware->page_params[ms_page]->length) != 0)))))
	{
		g_static_mutex_unlock(&serio_mutex);
		burn_ms1_ecu_flash();
		g_static_mutex_lock(&serio_mutex);
	}
skip_change:
	if ((ms_page == last_page) && (!force_page_change))
	{
		/*	printf("no need to change the page again as it's already %i\n",ms_page); */
		g_static_mutex_unlock(&serio_mutex);
		g_static_mutex_unlock(&mutex);
		return;
	}

force_change:
	if (dbg_lvl & SERIAL_WR)
		dbg_func(g_strdup_printf(__FILE__": set_ms_page()\n\tSetting Page to \"%i\" with \"%s\" command...\n",ms_page,firmware->page_cmd));

	res = write(serial_params->fd,firmware->page_cmd,1);
	if (res != 1)
	{
		err_text = (gchar *)g_strerror(errno);
		if (dbg_lvl & (SERIAL_WR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": set_ms_page()\n\tFAILURE sending \"%s\" (change page) command to ECU, ERROR \"%s\" \n",firmware->page_cmd,err_text));
	}
	res = write(serial_params->fd,&ms_page,1);
	g_usleep(100000);
	if (res != 1)
	{
		err_text = (gchar *)g_strerror(errno);
		if (dbg_lvl & (SERIAL_WR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": set_ms_page()\n\tFAILURE changing page on ECU to %i, ERROR \"%s\"\n",ms_page,err_text));
	}

	last_page = ms_page;

	g_static_mutex_unlock(&serio_mutex);
	g_static_mutex_unlock(&mutex);
	force_page_change = FALSE;
	return;

}



/*!
 \brief write_data() physiclaly sends the data to the ECU.
 \param message (Io_Message *) a pointer to a Io_Message
 */
void write_data(Io_Message *message)
{
	extern gboolean connected;
	OutputData *output = message->payload;

	gint res = 0;
	gchar * err_text = NULL;
	gint i = 0;
	gint j = 0;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gint value = 0;
	WriteMode mode = MTX_CMD_WRITE;
	guint8 *data = NULL;
	gint num_bytes = 0;
	DBlock *block = NULL;
	/*gint j = 0;
	gchar * tmpbuf = NULL;*/
	extern Serial_Params *serial_params;
	extern Firmware_Details *firmware;
	extern volatile gboolean offline;
	//static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	//g_static_mutex_lock(&serio_mutex);
	//g_static_mutex_lock(&mutex);
	
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

		if ((firmware->multi_page ) && 
				(output->need_page_change) && 
				(!(firmware->capabilities & MS2))) 
		{
//			g_static_mutex_unlock(&serio_mutex);
			set_ms_page(firmware->page_params[page]->truepgnum);
//			g_static_mutex_lock(&serio_mutex);
		}
	}

	if (offline)
	{
		//printf ("OFFLINE writing value at %i,%i [%i]\n",page,offset,value); 
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
//		g_static_mutex_unlock(&serio_mutex);
//		g_static_mutex_unlock(&mutex);
		return;		/* can't write anything if offline */
	}
	if (!connected)
	{
//		g_static_mutex_unlock(&serio_mutex);
//		g_static_mutex_unlock(&mutex);
		return;		/* can't write anything if disconnected */
	}

	for (i=0;i<message->sequence->len;i++)
	{
		block = g_array_index(message->sequence,DBlock *,i);
		if (block->type == ACTION)
		{
			if (block->action == SLEEP)
				g_usleep(block->arg);
		}
		else if (block->type == DATA)
		{
			for (j=0;j<block->len;j++)
			{
//				printf("comms.c data[%i] is %i\n",j,block->data[j]);
				if (dbg_lvl & (SERIAL_WR))
					dbg_func(g_strdup_printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%i\"\n",i,j+1,block->len,block->data[j]));
//				printf(__FILE__": write_data()\n\tWriting argument %i byte %i of %i, \"%i\"\n",i,j+1,block->len,block->data[j]);
				res = write (serial_params->fd,&(block->data[j]),1);	/* Send write command */
				if (res != 1)
					if (dbg_lvl & (SERIAL_WR|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__": write_data()\n\tError writing block  offset %i, value %i ERROR \"%s\"!!!\n",j,block->data[j],err_text));
				if (firmware->capabilities & MS2)
					g_usleep(firmware->interchardelay*1000);

			}
		}

	}
	if (output)
	{
		if (mode == MTX_SIMPLE_WRITE)
			set_ecu_data(canID,page,offset,size,value);
		else if (mode == MTX_CHUNK_WRITE)
			store_new_block(canID,page,offset,data,num_bytes);
	}

//	g_static_mutex_unlock(&serio_mutex);
//	g_static_mutex_unlock(&mutex);
	return;
}


