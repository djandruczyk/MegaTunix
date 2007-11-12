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
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <fcntl.h>
#include <gui_handlers.h>
#include <notifications.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <structures.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <termios.h>
#include <unistd.h>
#include <3d_vetable.h>

extern GStaticMutex serio_mutex;
extern gint dbg_lvl;

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
		thread_update_logbar("comms_view","warning",g_strdup_printf("Error writing \"C\" to the ecu, ERROR \"%s\" in comms_test()\n",err_text),TRUE,FALSE);
		connected = FALSE;
		return connected;
	}
	result = handle_ecu_data(C_TEST,NULL);
	if (!result) // Failure,  Attempt MS-II method
	{
		if (write(serial_params->fd,"c",1) != 1)
		{
			err_text = (gchar *)g_strerror(errno);
			if (dbg_lvl & (SERIAL_RD|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": comms_test()\n\tError writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n",err_text));
			thread_update_logbar("comms_view","warning",g_strdup_printf("Error writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n",err_text),TRUE,FALSE);
			connected = FALSE;
			return connected;
		}
		result = handle_ecu_data(C_TEST,NULL);
	}
	if (result)	// Success
	{
		connected = TRUE;
		errcount=0;
		if (dbg_lvl & SERIAL_RD)
			dbg_func(g_strdup(__FILE__": comms_test()\n\tECU Comms Test Successfull\n"));
		queue_function(g_strdup("kill_conn_warning"));
		thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup("ECU Connected..."));
		thread_update_logbar("comms_view",NULL,g_strdup_printf("ECU Comms Test Successfull\n"),TRUE,FALSE);

	}
	else
	{
		// An I/O Error occurred with the MegaSquirt ECU 
		connected = FALSE;
		errcount++;
		if (errcount > 2 )
			queue_function(g_strdup("conn_warning"));
		thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup_printf("COMMS ISSUES: Check COMMS tab"));
		if (dbg_lvl & (SERIAL_RD|IO_PROCESS))
			dbg_func(g_strdup(__FILE__": comms_test()\n\tI/O with ECU Timeout\n"));
		thread_update_logbar("comms_view","warning",g_strdup_printf("I/O with ECU Timeout\n"),TRUE,FALSE);
	}
	return connected;
}


/*!
 \brief update_write_status() checks the differences between the current ECU
 data snapshot and the last one, if there are any differences (things need to
 be burnt) then it turns all the widgets in the "burners" group to RED
 \param data (Output_Data *) pointer to data sent to ECU used to
 update other widgets that refer to that Page/Offset
 */
void update_write_status(Output_Data *data)
{
	extern gint **ms_data;
	extern gint **ms_data_last;
	gint i = 0;
	extern Firmware_Details *firmware;
	extern GList ***ve_widgets;
	extern gboolean paused_handlers;


	if ((data->data) && (data->mode == MTX_CHUNK_WRITE))
	{
		g_free(data->data);
		return;
	}

	paused_handlers = TRUE;

	//printf ("page %i, offset %i\n",data->page,data->offset);
	for (i=0;i<g_list_length(ve_widgets[data->page][data->offset]);i++)
	{
		if ((gint)g_object_get_data(G_OBJECT(g_list_nth_data(ve_widgets[data->page][data->offset],i)),"dl_type") != DEFERRED)
		{
			//			printf("updating widget %s\n",(gchar *)glade_get_widget_name(g_list_nth_data(ve_widgets[data->page][data->offset],i)));
			update_widget(g_list_nth_data(ve_widgets[data->page][data->offset],i),NULL);
		}
		//		else
		//printf("NOT updating widget %s because it's defered\n",(gchar *)glade_get_widget_name(g_list_nth_data(ve_widgets[data->page][data->offset],i)));
	}

	update_ve3d_if_necessary(data->page,data->offset);

	paused_handlers = FALSE;
	/* We check to see if the last burn copy of the VE/constants matches 
	 * the currently set, if so take away the "burn now" notification.
	 * avoid unnecessary burns to the FLASH 
	 */

	for (i=0;i<firmware->total_pages;i++)
	{

		if(memcmp(ms_data_last[i],ms_data[i],sizeof(gint)*firmware->page_params[i]->length) != 0)
		{
			set_group_color(RED,"burners");
			return;
		}
	}
	/* If pchunk dwrite data bound,  FREE IT */
	set_group_color(BLACK,"burners");

	return;
}


/*!
 \brief writeto_ecu() physiclaly sends the data to the ECU.
 \param message (Io_Message *) a pointer to a Io_Message
 */
void writeto_ecu(Io_Message *message)
{
	extern gboolean connected;
	Output_Data *output = message->payload;

	gint page = output->page;
	gint truepgnum = message->truepgnum;
	gint offset = output->offset;
	gint value = output->value;
	gboolean ign_parm = output->ign_parm;
	gint highbyte = 0;
	gint lowbyte = 0;
	gboolean twopart = 0;
	gint res = 0;
	gint count = 0;
	gchar * err_text = NULL;
	gint i = 0;
	char *lbuff = NULL;
	gchar * write_cmd = NULL;
	extern Firmware_Details *firmware;
	extern Serial_Params *serial_params;
	extern gint **ms_data;
	extern volatile gboolean offline;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&serio_mutex);
	g_static_mutex_lock(&mutex);

	if (offline)
	{
		//printf ("OFFLINE writing value at %i,%i [%i]\n",page,offset,value);
		switch (output->mode)
		{
			case MTX_SIMPLE_WRITE:
				ms_data[page][offset] = value;
				break;
			case MTX_CHUNK_WRITE:
				for (i=0;i<output->len;i++)
					ms_data[page][offset+i] = output->data[i];
				break;
		}
		g_static_mutex_unlock(&serio_mutex);
		g_static_mutex_unlock(&mutex);
		return;		/* can't write anything if offline */
	}
	if (!connected)
	{
		g_static_mutex_unlock(&serio_mutex);
		g_static_mutex_unlock(&mutex);
		return;		/* can't write anything if disconnected */
	}
	if ((!firmware->multi_page) && (page > 0))
	{
		if (dbg_lvl & (SERIAL_WR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tCRITICAL ERROR, Firmware is NOT multi-page, yet page is greater than ZERO, ABORTING WRITE!!!\n"));
		g_static_mutex_unlock(&serio_mutex);
		g_static_mutex_unlock(&mutex);
		return;
	}

	if (output->mode == MTX_SIMPLE_WRITE)
	{
		if (dbg_lvl & SERIAL_WR)
			dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSerial Write, Page, %i, Mem Offset %i, Value %i\n",page,offset,value));
	}
	else if (output->mode == MTX_CHUNK_WRITE)
	{
		if (dbg_lvl & SERIAL_WR)
			dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tCHUNK Write, Page, %i, Mem Offset %i, length %i\n",page,offset,output->len));
	}
	else
	{
		if (dbg_lvl & (SERIAL_WR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tNO output mode defined, aborting!!\n"));
		g_static_mutex_unlock(&serio_mutex);
		g_static_mutex_unlock(&mutex);
		return;
	}

	if ((value > 255) && (output->mode == MTX_SIMPLE_WRITE))
	{
		if (dbg_lvl & SERIAL_WR)
			dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tLarge value being sent: %i, to page %i, offset %i\n",value,page,offset));

		highbyte = (value & 0xff00) >> 8;
		lowbyte = value & 0x00ff;
		twopart = TRUE;
		if (dbg_lvl & SERIAL_WR)
			dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tHighbyte: %i, Lowbyte %i\n",highbyte,lowbyte));
	}
	if ((value < 0) && (output->mode == MTX_SIMPLE_WRITE))
	{
		if (dbg_lvl & (SERIAL_WR|CRITICAL))
			dbg_func(g_strdup(__FILE__": writeto_ecu()\n\tWARNING!!, value sent is below 0\n"));
		g_static_mutex_unlock(&serio_mutex);
		g_static_mutex_unlock(&mutex);
		return;
	}


	g_static_mutex_unlock(&serio_mutex);
	if ((firmware->multi_page ) && (message->need_page_change)) 
		set_ms_page(truepgnum);
	g_static_mutex_lock(&serio_mutex);

	if (dbg_lvl & SERIAL_WR)
		dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tIgnition param %i\n",ign_parm));

	if ((ign_parm) && (output->mode == MTX_SIMPLE_WRITE))
		write_cmd = g_strdup("J");
	else
		write_cmd = g_strdup(firmware->write_cmd);

	if (output->mode == MTX_SIMPLE_WRITE)
	{
		if (twopart)
		{
			count = 3;
			lbuff = g_new0(char,count);
			lbuff[0]=offset;
			lbuff[1]=highbyte;
			lbuff[2]=lowbyte;
			if (dbg_lvl & SERIAL_WR)
				dbg_func(g_strdup(__FILE__": writeto_ecu()\n\tSending 16 bit value to ECU\n"));
		}
		else
		{
			count = 2;
			lbuff = g_new0(char,count);
			lbuff[0]=offset;
			lbuff[1]=value;
			if (dbg_lvl & SERIAL_WR)
				dbg_func(g_strdup(__FILE__": writeto_ecu()\n\tSending 8 bit value to ECU\n"));
		}

		res = write (serial_params->fd,write_cmd,1);	/* Send write command */
		if (res != 1 )
		{
			err_text = (gchar *)g_strerror(errno);
			if (dbg_lvl & (SERIAL_WR|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending write command \"%s\" FAILED, ERROR \"%s\"!!!\n",write_cmd,err_text));
		}
		else
		{
			if (dbg_lvl & SERIAL_WR)
				dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending of write command \"%s\" to ECU succeeded\n",write_cmd));
		}
		res = write (serial_params->fd,lbuff,count);	/* Send offset+data */
		if (res != count )
		{
			err_text = (gchar *)g_strerror(errno);
			if (dbg_lvl & (SERIAL_WR|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending offset+data FAILED, ERROR \"%s\"!!!\n",err_text));
		}
		else
		{
			if (dbg_lvl & SERIAL_WR)
				dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending of offset+data to ECU succeeded\n"));
		}

		g_free(lbuff);
		ms_data[page][offset] = value;

		g_free(write_cmd);
		g_usleep(5000);

	}
	else if (output->mode == MTX_CHUNK_WRITE)
	{
		/* Initiate chunk write */
		res = write (serial_params->fd,firmware->chunk_write_cmd,1);
		if (res != 1 )
		{
			err_text = (gchar *)g_strerror(errno);
			if (dbg_lvl & (SERIAL_WR|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending chunkwrite command \"%s\" FAILED, ERROR \"%s\"!!!\n",firmware->chunk_write_cmd,err_text));
		}
		else
		{
			if (dbg_lvl & SERIAL_WR)
				dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tChunk write of \"%s\" to ECU succeeded\n",firmware->chunk_write_cmd));
		}

		/* count is len+2 cause we need two bytes for offset and len*/
		count = output->len+2;
		lbuff = g_new0(char,count);
		lbuff[0]=offset;
		lbuff[1]=output->len;
		for(i=0;i<output->len;i++)
		{
			ms_data[page][offset+i] = output->data[i];
			lbuff[2+i] = output->data[i];
		}
		res = write (serial_params->fd,lbuff,count);	/* Send write command */
		if (res != count )
		{
			err_text = (gchar *)g_strerror(errno);
			if (dbg_lvl & (SERIAL_WR|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending chunkwrite offset+count+data of length \"%i\", (%i) FAILED, ERROR \"%s\"!!!\n",count,res,err_text));
		}
		else
		{
			if (dbg_lvl & SERIAL_WR)
				dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tChunk write of offset+count+data to ECU succeeded\n"));
		}
		g_free(lbuff);
		g_usleep(100000);
	}

	/*is this really needed??? */

	g_static_mutex_unlock(&serio_mutex);
	g_static_mutex_unlock(&mutex);
	return;
}


/*!
 \brief burn_ecu_flash() issues the commands to the ECU to burn the contents
 of RAM to flash.
 */
void burn_ecu_flash()
{
	extern gint **ms_data;
	extern gint **ms_data_last;
	gint res = 0;
	gint i = 0;
	gchar * err_text = NULL;
	extern Firmware_Details * firmware;
	extern Serial_Params *serial_params;
	extern volatile gboolean offline;
	extern gboolean connected;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&serio_mutex);
	g_static_mutex_lock(&mutex);

	if (offline)
		goto copyover;

	if (!connected)
	{
		if (dbg_lvl & (SERIAL_WR|CRITICAL))
			dbg_func(g_strdup(__FILE__": burn_ms_flahs()\n\t NOT connected, can't burn flash, returning immediately\n"));
		g_static_mutex_unlock(&serio_mutex);
		g_static_mutex_unlock(&mutex);
		return;
	}
	g_static_mutex_unlock(&serio_mutex);
	flush_serial(serial_params->fd, TCIOFLUSH);
	g_static_mutex_lock(&serio_mutex);
	res = write (serial_params->fd,firmware->burn_cmd,1);  /* Send Burn command */
	if (res != 1)
	{
		err_text = (gchar *)g_strerror(errno);
		if (dbg_lvl & (SERIAL_WR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": burn_ecu_flash()\n\tBurn Failure, ERROR \"%s\"\n",err_text));
	}
	g_usleep(250000);

	if (dbg_lvl & SERIAL_WR)
		dbg_func(g_strdup(__FILE__": burn_ecu_flash()\n\tBurn to Flash\n"));
	g_static_mutex_unlock(&serio_mutex);
	flush_serial(serial_params->fd, TCIOFLUSH);
	g_static_mutex_lock(&serio_mutex);
copyover:
	/* sync temp buffer with current burned settings */
	for (i=0;i<firmware->total_pages;i++)
		memcpy(ms_data_last[i],ms_data[i],sizeof(gint)*firmware->page_params[i]->length);

	g_static_mutex_unlock(&serio_mutex);
	g_static_mutex_unlock(&mutex);
	return;
}


/*!
 \brief readfrom_ecu() reads arbritrary data from the ECU.  Data is actually
 written in this function to trigger the ECU to send back a block of data, 
 and then a handler is kicked off to handle the incoming data
 \see handle_ecu_data
 \param message (Io_Message *) pointer to a Io_Message
 */
void readfrom_ecu(Io_Message *message)
{
	gint result = 0;
	extern Serial_Params *serial_params;
	extern Firmware_Details *firmware;
	extern gboolean connected;
	extern gchar *handler_types[];
	gchar *err_text = NULL;
	extern volatile gboolean offline;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	if(serial_params->open == FALSE)
		return;

	if (offline)
		return;


	if ((firmware->multi_page ) && (message->need_page_change)) 
		set_ms_page(message->truepgnum);

	g_static_mutex_lock(&mutex);

	/* Flush serial port... */
	flush_serial(serial_params->fd, TCIOFLUSH);
	g_static_mutex_lock(&serio_mutex);
	result = write(serial_params->fd,
			message->out_str,
			message->out_len);
	if (result != message->out_len)	
	{
		err_text = (gchar *)g_strerror(errno);
		if (dbg_lvl & (SERIAL_RD|SERIAL_WR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\twrite command to ECU failed, ERROR \"%s\"\n",err_text));
	}

	else
		if (dbg_lvl & (SERIAL_WR|SERIAL_RD))
			dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\tSent %s to the ECU\n",message->out_str));

	if (message->handler == RAW_MEMORY_DUMP)
	{
		result = write(serial_params->fd,&message->offset,1);
		if (result < 1)
		{
			err_text = (gchar *)g_strerror(errno);
			if (dbg_lvl & (SERIAL_WR|SERIAL_RD|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\twrite of offset for raw mem cmd to ECU failed, ERROR \"%s\"\n",err_text));
		}
		else
		{
			if (dbg_lvl & (SERIAL_WR|SERIAL_RD))
				dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\twrite of offset of \"%i\" for raw mem cmd succeeded\n",message->offset));
		}
	}

	if (message->handler != -1)
	{
		g_static_mutex_unlock(&serio_mutex);
		result = handle_ecu_data(message->handler,message);
		g_static_mutex_lock(&serio_mutex);
	}
	else
	{
		if (dbg_lvl & (SERIAL_RD|SERIAL_WR|CRITICAL))
			dbg_func(g_strdup(__FILE__": readfrom_ecu()\n\t message->handler is undefined, author brainfart, EXITING!\n"));
		g_static_mutex_unlock(&serio_mutex);
		g_static_mutex_unlock(&mutex);
		exit (-1);
	}
	if (result)
	{
		connected = TRUE;
		if (dbg_lvl & (SERIAL_WR|SERIAL_RD))
			dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\tDone Reading %s\n",handler_types[message->handler]));

	}
	else
	{
		connected = FALSE;
		serial_params->errcount++;
		if (dbg_lvl & (SERIAL_WR|SERIAL_RD))
			dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\tError reading data: %s\n",g_strerror(errno)));
	}
	g_static_mutex_unlock(&serio_mutex);
	g_static_mutex_unlock(&mutex);
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
	extern Firmware_Details *firmware;
	extern Serial_Params *serial_params;
	extern gint **ms_data;
	extern gint **ms_data_last;
	extern gboolean force_page_change;
	static gint last_page = -1;
	gint res = 0;
	gchar * err_text = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	//printf("fed_page %i, last_page %i\n",ms_page,last_page);
	g_static_mutex_lock(&serio_mutex);
	g_static_mutex_lock(&mutex);

	/* put int to make sure page is SET on FIRST RUN after interrogation.
	 * Found that wihtout it was getting a corrupted first page
	 */
	if (last_page == -1)
		goto forceburn;

	if ((ms_page > firmware->ro_above) || (last_page > firmware->ro_above))
		goto skipburn;
	//	printf("last page %i, ms_page %i, memcpy results for last page %i, memcmp results for current page %i\n",last_page, ms_page, memcmp(ms_data_last[last_page],ms_data[last_page],sizeof(gint)*firmware->page_params[last_page]->length),memcmp(ms_data_last[ms_page],ms_data[ms_page],sizeof(gint)*firmware->page_params[ms_page]->length));

	if (((ms_page != last_page) && (((memcmp(ms_data_last[last_page],ms_data[last_page],sizeof(gint)*firmware->page_params[last_page]->length) != 0)) || ((memcmp(ms_data_last[ms_page],ms_data[ms_page],sizeof(gint)*firmware->page_params[ms_page]->length) != 0)))))
	{
		g_static_mutex_unlock(&serio_mutex);
		burn_ecu_flash();
		g_static_mutex_lock(&serio_mutex);
	}
skipburn:
	if ((ms_page == last_page) && (!force_page_change))
	{
		//	printf("no need to change the page again as it's already %i\n",ms_page);
		g_static_mutex_unlock(&serio_mutex);
		g_static_mutex_unlock(&mutex);
		return;
	}

forceburn:
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
