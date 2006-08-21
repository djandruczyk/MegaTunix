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

	if (connected)
		update_logbar("comms_view",NULL,g_strdup("ECU Comms Test Successfull\n"),TRUE,FALSE);
	else
		update_logbar("comms_view","warning",g_strdup("I/O with ECU Timeout\n"),TRUE,FALSE);

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

void comms_test()
{
	gboolean result = FALSE;
	gint count = 0;
	extern struct Serial_Params *serial_params;
	extern gboolean connected;

	/* If serial control struct exists, 
	 * but we don't have connected status, try to reset connection */
	
	if (!serial_params->open)
	{
		dbg_func(g_strdup(__FILE__": comms_test()\n\tSerial Port is NOT opened can NOT check ecu comms...\n"),CRITICAL);
		return;
	}
	/* Try toggling the control lines */
	toggle_serial_control_lines();

	/* Flush the toilet.... */
	flush_serial(serial_params->fd, TCIOFLUSH);	

	while ((write(serial_params->fd,"C",1) != 1) && (count < 4 ))
	{
		g_usleep(1000);
		dbg_func(g_strdup(__FILE__": comms_test()\n\tError writing \"C\" to the ecu in comms_test()\n"),CRITICAL);
		count++;
	}
	if (count >= 4)
	{
		connected = FALSE;
		flush_serial(serial_params->fd, TCIOFLUSH);
		dbg_func(g_strdup(__FILE__": comms_test()\n\tFour attempts made to talk to ECU, no response received\n"),CRITICAL);
		return;
	}
	dbg_func(g_strdup(__FILE__": comms_test()\n\tRequesting ECU Clock (\"C\" cmd)\n"),SERIAL_RD);
	result = handle_ecu_data(C_TEST,NULL);
	if (result)	// Success
	{
		// COMMS test succeeded 
		connected = TRUE;
		dbg_func(g_strdup(__FILE__": comms_test()\n\tECU Comms Test Successfull\n"),SERIAL_RD);
		queue_function(g_strdup("kill_conn_warning"));
		thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup("ECU Connected..."));

	}
	else
	{
		// An I/O Error occurred with the MegaSquirt ECU 
		connected = FALSE;
		queue_function(g_strdup("conn_warning"));
		thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup_printf("COMMS ISSUES: Check COMMS tab"));
		dbg_func(g_strdup(__FILE__": comms_test()\n\tI/O with ECU Timeout\n"),IO_PROCESS);
	}
	/* Flush the toilet again.... */
	flush_serial(serial_params->fd, TCIOFLUSH);
	return;
}


/*!
 \brief update_write_status() checks the differences between the current ECU
 data snapshot and the last one, if there are any differences (things need to
 be burnt) then it turns all the widgets in the "burners" group to RED
 \param data (struct Output_Data *) pointer to data sent to ECU used to
 update other widgets that refer to that Page/Offset
 */
void update_write_status(struct Output_Data *data)
{
	extern gint **ms_data;
	extern gint **ms_data_last;
	gint i = 0;
	extern struct Firmware_Details *firmware;
	extern GList ***ve_widgets;
	extern gboolean paused_handlers;

	paused_handlers = TRUE;
	for (i=0;i<g_list_length(ve_widgets[data->page][data->offset]);i++)
	{
		if ((gint)g_object_get_data(G_OBJECT(g_list_nth_data(ve_widgets[data->page][data->offset],i)),"dl_type") != DEFERRED)
			update_widget(g_list_nth_data(ve_widgets[data->page][data->offset],i),NULL);
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

	set_group_color(BLACK,"burners");

	return;
}


/*!
 \brief writeto_ecu() physiclaly sends the data to the ECU.
 \param message (struct Io_Message *) a pointer to a struct Io_Message
 */
void writeto_ecu(struct Io_Message *message)
{
	extern gboolean connected;
	struct Output_Data *data = message->payload;

	gint page = data->page;
	gint offset = data->offset;
	gint value = data->value;
	gboolean ign_parm = data->ign_parm;
	gint highbyte = 0;
	gint lowbyte = 0;
	gboolean twopart = 0;
	gint res = 0;
	gint count = 0;
	char lbuff[3] = {0, 0, 0};
	gchar * write_cmd = NULL;
	extern struct Firmware_Details *firmware;
	extern struct Serial_Params *serial_params;
	extern gint **ms_data;
	extern gboolean offline;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	if (offline)
	{
		ms_data[page][offset] = value;
		g_static_mutex_unlock(&mutex);
		return;		/* can't write anything if offline */
	}
	if (!connected)
	{
		g_static_mutex_unlock(&mutex);
		return;		/* can't write anything if disconnected */
	}
	if ((!firmware->multi_page) && (page > 0))
		dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tCRITICAL ERROR, Firmware is NOT multi-page, yet page is greater than ZERO!!!\n"),CRITICAL);


	dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSerial Write, Page, %i, Mem Offset %i, Value %i\n",page,offset,value),SERIAL_WR);

	if (value > 255)
	{
		dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tLarge value being sent: %i, to page %i, offset %i\n",value,page,offset),SERIAL_WR);

		highbyte = (value & 0xff00) >> 8;
		lowbyte = value & 0x00ff;
		twopart = TRUE;
		dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tHighbyte: %i, Lowbyte %i\n",highbyte,lowbyte),SERIAL_WR);
	}
	if (value < 0)
	{
		dbg_func(g_strdup(__FILE__": writeto_ecu()\n\tWARNING!!, value sent is below 0\n"),CRITICAL);
		g_static_mutex_unlock(&mutex);
		return;
	}

	if ((firmware->multi_page ) && (message->need_page_change)) 
		set_ms_page(page);

	dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tIgnition param %i\n",ign_parm),SERIAL_WR);

	if (ign_parm)
		write_cmd = g_strdup("J");
	else
		write_cmd = g_strdup(firmware->write_cmd);

	lbuff[0]=offset;
	if (twopart)
	{
		lbuff[1]=highbyte;
		lbuff[2]=lowbyte;
		count = 3;
		dbg_func(g_strdup(__FILE__": writeto_ecu()\n\tSending 16 bit value to ECU\n"),SERIAL_WR);
	}
	else
	{
		lbuff[1]=value;
		count = 2;
		dbg_func(g_strdup(__FILE__": writeto_ecu()\n\tSending 8 bit value to ECU\n"),SERIAL_WR);
	}

	res = write (serial_params->fd,write_cmd,1);	/* Send write command */
	if (res != 1 )
		dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending write command \"%s\" FAILED!!!\n",write_cmd),CRITICAL);
	else
		dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending of write command \"%s\" to ECU succeeded\n",write_cmd),SERIAL_WR);
	res = write (serial_params->fd,lbuff,count);	/* Send offset+data */
	if (res != count )
		dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending offset+data FAILED!!!\n"),CRITICAL);
	else
		dbg_func(g_strdup_printf(__FILE__": writeto_ecu()\n\tSending of offset+data to ECU succeeded\n"),SERIAL_WR);

	ms_data[page][offset] = value;
	g_usleep(5000);

	g_free(write_cmd);

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
	extern struct Firmware_Details * firmware;
	extern struct Serial_Params *serial_params;
	extern gboolean offline;
	extern gboolean connected;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	if (offline)
		goto copyover;

	if (!connected)
	{
		dbg_func(g_strdup(__FILE__": burn_ms_flahs()\n\t NOT connected, can't burn flash, returning immediately\n"),CRITICAL);
		return;
	}
	flush_serial(serial_params->fd, TCIOFLUSH);

	res = write (serial_params->fd,firmware->burn_cmd,1);  /* Send Burn command */
	if (res != 1)
	{
		dbg_func(g_strdup_printf(__FILE__": burn_ecu_flash()\n\tBurn Failure, write command failed!!%i\n",res),CRITICAL);
	}
	g_usleep(100000);

	dbg_func(g_strdup_printf(__FILE__": burn_ecu_flash()\n\tBurn to Flash\n"),SERIAL_WR);

	flush_serial(serial_params->fd, TCIOFLUSH);
copyover:
	/* sync temp buffer with current burned settings */
	for (i=0;i<firmware->total_pages;i++)
		memcpy(ms_data_last[i],ms_data[i],sizeof(gint)*firmware->page_params[i]->length);

	g_static_mutex_unlock(&mutex);
	return;
}


/*!
 \brief readfrom_ecu() reads arbritrary data from the ECU.  Data is actually
 written in this function to trigger the ECU to send back a block of data, 
 and then a handler is kicked off to handle the incoming data
 \see handle_ecu_data
 \param message (struct Io_Message *) pointer to a struct Io_Message
 */
void readfrom_ecu(struct Io_Message *message)
{
	gint result = 0;
	extern struct Serial_Params *serial_params;
	extern struct Firmware_Details *firmware;
	extern gboolean connected;
	extern gchar *handler_types[];
	extern gboolean offline;

	if(serial_params->open == FALSE)
		return;

	if (offline)
		return;

	/* Flush serial port... */
	flush_serial(serial_params->fd, TCIOFLUSH);

	if ((firmware->multi_page ) && (message->need_page_change)) 
		set_ms_page(message->page);

	result = write(serial_params->fd,
			message->out_str,
			message->out_len);
	if (result != message->out_len)	
		dbg_func(g_strdup(__FILE__": readfrom_ecu()\n\twrite command to ECU failed\n"),CRITICAL);

	else
		dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\tSent %s to the ECU\n",message->out_str),SERIAL_WR);

	if (message->handler == RAW_MEMORY_DUMP)
	{
		result = write(serial_params->fd,&message->offset,1);
		if (result == -1)
			dbg_func(g_strdup(__FILE__": readfrom_ecu()\n\twrite of offset for raw mem cmd to ECU failed\n"),CRITICAL);
		else if (result != 1)	
			dbg_func(g_strdup(__FILE__": readfrom_ecu()\n\twrite of offset for raw mem cmd to ECU failed\n"),CRITICAL);
		else
			dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\twrite of offset of \"%i\" for raw mem cmd succeeded\n",message->offset),SERIAL_WR);
	}

	if (message->handler != -1)
		result = handle_ecu_data(message->handler,message);
	else
	{
		dbg_func(g_strdup(__FILE__": readfrom_ecu()\n\t message->handler is undefined, author brainfart, EXITING!\n"),CRITICAL);
		exit (-1);
	}
	if (result)
	{
		connected = TRUE;
		dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\tDone Reading %s\n",handler_types[message->handler]),SERIAL_RD);

	}
	else
	{
		connected = FALSE;
		serial_params->errcount++;
		dbg_func(g_strdup_printf(__FILE__": readfrom_ecu()\n\tError reading data: %s\n",g_strerror(errno)),IO_PROCESS);
	}
}

