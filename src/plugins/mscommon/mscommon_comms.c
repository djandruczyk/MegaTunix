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
  \file src/plugins/mscommon/mscommon_comms.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS specific communication functions
  \author David Andruczyk
  */

#include <datamgmt.h>
#include <errno.h>
#include <firmware.h>
#include <mscommon_comms.h>
#include <mscommon_plugin.h>
#include <mtxsocket.h>
#include <serialio.h>
#include <stdio.h>
#include <string.h>


extern gconstpointer *global_data;

/*!
 \brief queue_burn_ecu_flash() issues the commands to the ECU to 
 burn the contents of RAM to flash.
 \param page is the MTX page (not  to be confused with the ECU physical page)
 to be burnt
 */
G_MODULE_EXPORT void queue_burn_ecu_flash(gint page)
{
	OutputData *output = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (DATA_GET(global_data,"offline"))
	{
		EXIT();
		return;
	}

	output = initialize_outputdata_f();
	DATA_SET(output->data,"canID", GINT_TO_POINTER(firmware->canID));
	DATA_SET(output->data,"page", GINT_TO_POINTER(page));
	DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
	io_cmd_f(firmware->burn_command,output);
	DATA_SET(global_data,"last_page",GINT_TO_POINTER(page));
	EXIT();
	return;
}


/*!
 \brief queue_ms1_page_change() issues the commands to the ECU to 
 change the ECU page
 \param page is the page to be read into ECU RAM from flash by the ECU
 */
G_MODULE_EXPORT void queue_ms1_page_change(gint page)
{
	OutputData *output = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (DATA_GET(global_data,"offline"))
	{
		EXIT();
		return;
	}

	output = initialize_outputdata_f();
	DATA_SET(output->data,"page", GINT_TO_POINTER(page));
	DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
	io_cmd_f(firmware->page_command,output);
	DATA_SET(global_data,"last_page",GINT_TO_POINTER(page));
	EXIT();
	return;
}



/*! 
 \brief comms_test sends the clock_request command to the ECU and
 checks the response.  if nothing comes back, MegaTunix assumes the ecu isn't
 connected or powered down. NO Gui updates are done from this function as it
 gets called from a thread. 
 */
G_MODULE_EXPORT gint comms_test(void)
{
	static gint errcount = 0;
	gboolean result = FALSE;
	gchar * err_text = NULL;
	gint len = 0;
	gint res = 0;
	const guint8 ms3_comms_check[7] = {0x00, 0x01, 0x63, 0x06, 0xb9, 0xdf, 0x6f};
	Serial_Params *serial_params = NULL;
	extern gconstpointer *global_data;

	ENTER();
	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return TRUE;
	}
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	g_return_val_if_fail(serial_params, FALSE);
	MTXDBG(SERIAL_RD,_("Entered\n"));

	if ((GINT)DATA_GET(global_data,"ecu_baud") < 115200)
	{
		MTXDBG(SERIAL_RD,_("Requesting MS-1 ECU Clock\n"));
		/*printf("MS-1 comms test\n");*/
		if (!write_wrapper_f(serial_params->fd,"C",1,&len))
		{
			err_text = (gchar *)g_strerror(errno);
			MTXDBG(SERIAL_WR|CRITICAL,_("Error writing \"C\" to the ecu, ERROR \"%s\" in comms_test()\n"),err_text);
			thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("Error writing \"C\" to the ecu, ERROR \"%s\" in comms_test()\n"),err_text),FALSE,FALSE);
			DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
			EXIT();
			return FALSE;
		}
		result = read_data_f(1,NULL,FALSE);
	}
	else
	{
		MTXDBG(SERIAL_RD,_("Requesting MS2/3 ECU Clock\n"));
		/*printf("MS-2 comms test\n");*/
		if (g_ascii_strcasecmp((gchar *)DATA_GET(global_data,"ecu_persona"),"MS3-1.1") == 0)
			res = write_wrapper_f(serial_params->fd,&ms3_comms_check,7,&len);
		else
			res = write_wrapper_f(serial_params->fd,"c",1,&len);
		if (!res)
		{
			err_text = (gchar *)g_strerror(errno);
			MTXDBG(SERIAL_WR|CRITICAL,_("Error writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n"),err_text);
			thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("Error writing \"c\" (MS-II clock test) to the ecu, ERROR \"%s\" in comms_test()\n"),err_text),FALSE,FALSE);
			DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
			EXIT();
			return FALSE;
		}
		result = read_data_f(2,NULL,FALSE);
	}
	if (result)     /* Success */
	{
		DATA_SET(global_data,"connected",GINT_TO_POINTER(TRUE));
		errcount=0;
		MTXDBG(SERIAL_RD,_("ECU Comms Test Successful\n"));
		queue_function_f("kill_conn_warning");
		thread_update_widget_f("titlebar",MTX_TITLE,g_strdup(_("ECU Connected...")));
		thread_update_logbar_f("comms_view","info",g_strdup_printf(_("ECU Comms Test Successful\n")),FALSE,FALSE);
		EXIT();
		return TRUE;

	}
	else
	{
		/* An I/O Error occurred with the MegaSquirt ECU  */
		DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
		errcount++;
		if (errcount > 5 )
			queue_function_f("conn_warning");
		thread_update_widget_f("titlebar",MTX_TITLE,g_strdup_printf(_("COMMS ISSUES: Check COMMS tab")));
		MTXDBG(SERIAL_RD|IO_PROCESS,_("I/O with ECU Timeout\n"));
		thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("I/O with ECU Timeout\n")),FALSE,FALSE);
		EXIT();
		return FALSE;
	}
	EXIT();
	return FALSE;
}


/*!
 \brief ms_table_write() gets called to send a block of lookuptable values to the ECU
 \param page is the MTX page in which the value refers to.
 \param num_bytes is the length of block to sent
 \param data is the block of data to be sent which better damn well be
 in ECU byte order if there is an endianness thing..
 */
G_MODULE_EXPORT void ms_table_write(gint page, gint num_bytes, guint8 * data)
{
	static GMutex mutex;
	OutputData *output = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_mutex_lock(&mutex);

	MTXDBG(SERIAL_WR,_("Sending page %i, num_bytes %i, data %p\n"),page,num_bytes,data);

	output = initialize_outputdata_f();
	DATA_SET(output->data,"page", GINT_TO_POINTER(page));
	DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
	DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(num_bytes));
	DATA_SET(output->data,"data", (gpointer)data);
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CHUNK_WRITE));

	/* save it otherwise the burn checker can miss it due to a potential
	 * race condition
	 */
	ms_store_new_block(0,page,0,data,num_bytes);

	if (firmware->multi_page)
		ms_handle_page_change(page,(GINT)DATA_GET(global_data,"last_page"));
	output->queue_update = TRUE;
	io_cmd_f(firmware->table_write_command,output);

	g_mutex_unlock(&mutex);
	EXIT();
	return;
}


/*!
  \brief cis passed the current/last pages and  determines if the ECU
  requires a  burn to flash or not, if so, it injects the command to do so
  \param page is the current page
  \param last is the page  used  for the last command
  */
G_MODULE_EXPORT void ms_handle_page_change(gint page, gint last)
{
	guint8 **ecu_data = NULL;
	guint8 **ecu_data_last = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;
	ecu_data_last = firmware->ecu_data_last;

	/*printf("handle page change!, page %i, last %i\n",page,last);
	 */

	if (last == -1)  /* First Write of the day, burn not needed... */
	{
		queue_ms1_page_change(page);
		EXIT();
		return;
	}
	if ((page == last) && (!DATA_GET(global_data,"force_page_change")))
	{
		/*printf("page == last and force_page_change is not set\n");
		 */
		EXIT();
		return;
	}
	/* If current page is NOT a dl_by_default page, but the last one WAS
	 * then a burn is required otherwise settings will be lost in the
	 * last
	 */
	if ((!firmware->page_params[page]->dl_by_default) && (firmware->page_params[last]->dl_by_default))
	{
		/*printf("current was not dl by default  but last was,  burning\n");
		 */
		queue_burn_ecu_flash(last);
		if (firmware->capabilities & MS1)
			queue_ms1_page_change(page);
		EXIT();
		return;
	}
	/* If current page is NOT a dl_by_default page, OR the last one was
	 * not then a burn is NOT required.
	 */
	if ((!firmware->page_params[page]->dl_by_default) || (!firmware->page_params[last]->dl_by_default))
	{
		/*printf("current is not dl by default or last was not as well\n");
		 */
		if ((page != last) && (firmware->capabilities & MS1))
		{
			/*printf("page diff and MS1, changing page\n");
			 */
			queue_ms1_page_change(page);
		}
		EXIT();
		return;
	}
	/* If current and last pages are DIFFERENT,  do a memory buffer scan
	 * to see if previous and last match,  if so return, otherwise burn
	 * then change page
	 */
	if (((page != last) && (((memcmp(ecu_data_last[last],ecu_data[last],firmware->page_params[last]->length) != 0)) || ((memcmp(ecu_data_last[page],ecu_data[page],firmware->page_params[page]->length) != 0)))))
	{
		/*printf("page and last don't match AND there's a ram difference, burning, before changing\n");
		 */
		queue_burn_ecu_flash(last);
		if (firmware->capabilities & MS1)
			queue_ms1_page_change(page);
	}
	else if ((page != last) && (firmware->capabilities & MS1))
	{
		/*printf("page and last don't match AND there's a NOT a RAM difference, changing page\n");
		 */
		queue_ms1_page_change(page);
	}
	EXIT();
	return;
}


/*!
 \brief chunk_write() gets called to send a block of data to the ECU.  
 This function has an ECU agnostic interface and is for sending 
 arbritrary blocks of data to the ECU. This function extracts the 
 important things from the passed ptr and sends to the real function 
 which is ecu specific.
 \param data is the pointer to the object containing the really important 
 things like thecanID, page  and offset
 \param num_bytes is how many bytes in the next var to send
 \param block is a pointer to the buffer to write
 */
G_MODULE_EXPORT void chunk_write(gpointer data, gint num_bytes, guint8 * block)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	GtkWidget *widget = (GtkWidget *)data;
	gconstpointer *gptr = (gconstpointer *)data;

	ENTER();
	if (GTK_IS_WIDGET(widget))
	{
		canID = (GINT)OBJ_GET(widget,"canID");
		page = (GINT)OBJ_GET(widget,"page");
		offset = (GINT)OBJ_GET(widget,"offset");
	}
	else
	{
		canID = (GINT)DATA_GET(gptr,"canID");
		page = (GINT)DATA_GET(gptr,"page");
		offset = (GINT)DATA_GET(gptr,"offset");
	}
	ms_chunk_write(canID, page, offset, num_bytes, block);
	EXIT();
	return;
}

/*!
 \brief ecu_chunk_write() is an abstraction wrapper
 \param canID is the CAN identifier (0-14)
 \param page is the MTX page in which the value refers to.
 \param offset is the offset from the beginning of the page that this data
 refers to.
 \param num_bytes is the length of block to sent
 \param block is the the block of data to be sent which better damn well be
 in ECU byte order if there is an endianness thing..
 */
G_MODULE_EXPORT void ecu_chunk_write(gint canID, gint page, gint offset, gint num_bytes, guint8 * block)
{
	/* Should check if firmware is chunk capable first though and fallback
	 * as needed
	 */
	ENTER();
	ms_chunk_write(canID,page,offset,num_bytes,block);
	EXIT();
	return;
}


/*!
 \brief ms_chunk_write() gets called to send a block of values to the ECU.
 \param canID is the CAN identifier (0-14)
 \param page is the MTX page in which the value refers to.
 \param offset is the offset from the beginning of the page that this data
 refers to.
 \param num_bytes is the length of block to sent
 \param block is the the block of data to be sent which better damn well be
 in ECU byte order if there is an endianness thing..
 */
G_MODULE_EXPORT void ms_chunk_write(gint canID, gint page, gint offset, gint num_bytes, guint8 * block)
{
	OutputData *output = NULL;
	Firmware_Details *firmware = NULL;
	gint adder = 0;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (firmware->capabilities & MS3_NEWSERIAL)
		adder = 6;

	MTXDBG(SERIAL_WR,_("Sending canID %i, page %i, offset %i, num_bytes %i, data %p\n"),canID,page,offset,num_bytes,block);
	output = initialize_outputdata_f();
	DATA_SET(output->data,"canID", GINT_TO_POINTER(canID));
	DATA_SET(output->data,"page", GINT_TO_POINTER(page));
	DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
	DATA_SET(output->data,"offset", GINT_TO_POINTER(offset));
	DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(num_bytes+adder));
	DATA_SET_FULL(output->data,"data", (gpointer)block, g_free);
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CHUNK_WRITE));

	/* save it otherwise the burn checker can miss it due to a potential
	 * race condition
	 */
	ms_store_new_block(canID,page,offset,block,num_bytes);

	if (firmware->multi_page)
		ms_handle_page_change(page,(GINT)DATA_GET(global_data,"last_page"));
	output->queue_update = TRUE;
	io_cmd_f(firmware->chunk_write_command,output);
	DATA_SET(global_data,"last_page",GINT_TO_POINTER(page));
	EXIT();
	return;
}


/*!
 \brief send_to_ecu() gets called to send a value to the ECU.  This function
 is has an ECU agnostic interface and is for sending single 8-32 bit bits of 
 data to the ECU. This one extracts the important things from the passed ptr
 and sends to the real function which is ecu specific
 \param data is a pointer to the  object holding the important bits
 \param value is the new value to send
 \param queue_update is a flag to trigger other wisgets on this address to
 update
 */
G_MODULE_EXPORT void send_to_ecu(gpointer data, gint value, gboolean queue_update)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	GtkWidget *widget = (GtkWidget *)data;
	gconstpointer *gptr = (gconstpointer *)data;

	ENTER();
	if (GTK_IS_WIDGET(widget))
	{
		canID = (GINT)OBJ_GET(widget,"canID");
		page = (GINT)OBJ_GET(widget,"page");
		offset = (GINT)OBJ_GET(widget,"offset");
		size = (DataSize)(GINT)OBJ_GET(widget,"size");
	}
	else
	{
		canID = (GINT)DATA_GET(gptr,"canID");
		page = (GINT)DATA_GET(gptr,"page");
		offset = (GINT)DATA_GET(gptr,"offset");
		size = (DataSize)(GINT)DATA_GET(gptr,"size");
	}
	ms_send_to_ecu(canID,page,offset,size,value,queue_update);
	EXIT();
	return;
}


/*!
 \brief ms_send_to_ecu() gets called to send a value to the ECU.  This function
 will check if the value sent is NOT the reqfuel_offset (that has special
 interdependancy issues) and then will check if there are more than 1 widgets
 that are associated with this page/offset and update those widgets before
 sending the value to the ECU.
 \param canID is the CAN identifier
 \param page is the page in which the value refers to.
 \param offset is the offset from the beginning of the page that this data
 refers to.
 \param size is the size enumeration for this value
 \param value is the the value that should be sent to the ECU At page.offset
 \param queue_update if true queues a gui update, used to prevent
 a horrible stall when doing an ECU restore or batch load...
 */
G_MODULE_EXPORT void ms_send_to_ecu(gint canID, gint page, gint offset, DataSize size, gint value, gboolean queue_update)
{
	OutputData *output = NULL;
	guint8 *data = NULL;
	guint16 u16 = 0;
	gint16 s16 = 0;
	guint32 u32 = 0;
	gint32 s32 = 0;
	gint adder = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (firmware->capabilities & MS3_NEWSERIAL)
		adder = 6;

	MTXDBG(SERIAL_WR,_("Sending canID %i, page %i, offset %i, value %i\n"),canID,page,offset,value);

	switch (size)
	{
		case MTX_CHAR:
		case MTX_S08:
		case MTX_U08:
			/*              printf("8 bit var %i at offset %i\n",value,offset);*/
			break;
		case MTX_S16:
		case MTX_U16:
			/*              printf("16 bit var %i at offset %i\n",value,offset);*/
			break;
		case MTX_S32:
		case MTX_U32:
			/*              printf("32 bit var %i at offset %i\n",value,offset);*/
			break;
		default:
			printf(_("ms_send_to_ecu() ERROR!!! Size undefined for variable at canID %i, page %i, offset %i\n"),canID,page,offset);
	}
	/* If the ecu is multi-page, run the handler to take care of queing
	 * burns and/or page changing
	 */
	if (firmware->multi_page)
		ms_handle_page_change(page,(GINT)DATA_GET(global_data,"last_page"));
	/* VERY special case for busted as MS-1 which can only accept 8 bit
	   writes
	   */
	if ((firmware->capabilities & MS1) && ((size == MTX_U16) || (size == MTX_S16)))
	{
		/* First byte */
		output = initialize_outputdata_f();
		output->queue_update = queue_update;
		DATA_SET(output->data,"canID", GINT_TO_POINTER(canID));
		DATA_SET(output->data,"page", GINT_TO_POINTER(page));
		DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
		DATA_SET(output->data,"offset", GINT_TO_POINTER(offset));
		DATA_SET(output->data,"value", GINT_TO_POINTER((value & 0xff00) >> 8));
		DATA_SET(output->data,"size", GINT_TO_POINTER(MTX_U08));
		DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(get_multiplier_f(MTX_U08)));
		DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_SIMPLE_WRITE));
		io_cmd_f(firmware->write_command,output);
		/* Second byte */
		output = initialize_outputdata_f();
		output->queue_update = queue_update;
		DATA_SET(output->data,"canID", GINT_TO_POINTER(canID));
		DATA_SET(output->data,"page", GINT_TO_POINTER(page));
		DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
		DATA_SET(output->data,"offset", GINT_TO_POINTER(offset+1));
		DATA_SET(output->data,"value", GINT_TO_POINTER(value & 0xff));
		DATA_SET(output->data,"size", GINT_TO_POINTER(MTX_U08));
		DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(get_multiplier_f(MTX_U08)));
		DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_SIMPLE_WRITE));
		io_cmd_f(firmware->write_command,output);
	}
	else
	{
		/* Normal 8 bit stuff */
		output = initialize_outputdata_f();
		output->queue_update = queue_update;
		DATA_SET(output->data,"canID", GINT_TO_POINTER(canID));
		DATA_SET(output->data,"page", GINT_TO_POINTER(page));
		DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
		DATA_SET(output->data,"offset", GINT_TO_POINTER(offset));
		DATA_SET(output->data,"value", GINT_TO_POINTER(value));
		DATA_SET(output->data,"size", GINT_TO_POINTER(size));
		DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(get_multiplier_f(size)+adder));
		DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_SIMPLE_WRITE));
		/* SPECIAL case for MS2, as it's write always assume a "datablock"
		 * and it doesn't have a simple easy write api due to it's use of 
		 * different sized vars,  hence the extra complexity.
		 */
		if (firmware->capabilities & MS2)
		{
			/* Get memory */
			data = g_new0(guint8,get_multiplier_f(size));
			switch (size)
			{
				case MTX_CHAR:
				case MTX_U08:
					data[0] = (guint8)value;
					break;
				case MTX_S08:
					data[0] = (gint8)value;
					break;
				case MTX_U16:
					if (firmware->bigendian)
						u16 = GUINT16_TO_BE((guint16)value);
					else
						u16 = GUINT16_TO_LE((guint16)value);
					data[0] = (guint8)u16;
					data[1] = (guint8)((guint16)u16 >> 8);
					break;
				case MTX_S16:
					if (firmware->bigendian)
						s16 = GINT16_TO_BE((gint16)value);
					else
						s16 = GINT16_TO_LE((gint16)value);
					data[0] = (guint8)s16;
					data[1] = (guint8)((gint16)s16 >> 8);
					break;
				case MTX_S32:
					if (firmware->bigendian)
						s32 = GINT32_TO_BE((gint32)value);
					else
						s32 = GINT32_TO_LE((gint32)value);
					data[0] = (guint8)s32;
					data[1] = (guint8)((gint32)s32 >> 8);
					data[2] = (guint8)((gint32)s32 >> 16);
					data[3] = (guint8)((gint32)s32 >> 24);
					break;
				case MTX_U32:
					if (firmware->bigendian)
						u32 = GUINT32_TO_BE((guint32)value);
					else
						u32 = GUINT32_TO_LE((guint32)value);
					data[0] = (guint8)u32;
					data[1] = (guint8)((guint32)u32 >> 8);
					data[2] = (guint8)((guint32)u32 >> 16);
					data[3] = (guint8)((guint32)u32 >> 24);
					break;
				default:
					break;
			}
			DATA_SET_FULL(output->data,"data", (gpointer)data,g_free);
		}
		io_cmd_f(firmware->write_command,output);
	}

	/* Set it here otherwise there's a risk of a missed burn due to 
	 * a potential race condition in the burn checker
	 */
	ms_set_ecu_data(canID,page,offset,size,value);

	DATA_SET(global_data,"last_page",GINT_TO_POINTER(page));
	EXIT();
	return;
}


/*!
 \brief send_to_slaves() sends messages to a thread talking to the slave
 clients to trigger them to update their GUI with appropriate changes
 \param data is a pointer to an OutputData structure which contains the
 necessary info to pass to the slave 
 */
G_MODULE_EXPORT void send_to_slaves(void *data)
{
	static GAsyncQueue *slave_msg_queue = NULL;
	Io_Message *message = (Io_Message *)data;
	OutputData *output = (OutputData *)message->payload;
	SlaveMessage *msg = NULL;

	ENTER();
	if (!slave_msg_queue)
		slave_msg_queue = (GAsyncQueue *)DATA_GET(global_data,"slave_msg_queue");
	if (!(GBOOLEAN)DATA_GET(global_data,"network_access"))
	{
		EXIT();
		return;
	}
	if (!output) /* If no data, don't bother the slaves */
	{
		EXIT();
		return;
	}
	if (!slave_msg_queue)
	{
		EXIT();
		return;
	}
	msg = g_new0(SlaveMessage, 1);
	msg->page = (guint8)(GINT)DATA_GET(output->data,"page");
	msg->offset = (guint16)(GINT)DATA_GET(output->data,"offset");
	msg->length = (guint16)(GINT)DATA_GET(output->data,"num_bytes");
	msg->size = (DataSize)(GINT)DATA_GET(output->data,"size");
	msg->mode = (WriteMode)(GINT)DATA_GET(output->data,"mode");
	msg->type = MTX_DATA_CHANGED;
	if (msg->mode == MTX_CHUNK_WRITE)
		msg->data = g_memdup(DATA_GET(output->data,"data"), msg->length);
	else if (msg->mode == MTX_SIMPLE_WRITE)
		msg->value = (GINT)DATA_GET(output->data,"value");
	else
	{
		printf(_("Non simple/chunk write command, not notifying peers\n"));
		g_free(msg);
		EXIT();
		return;
	}

	/*      printf("Sending message to peer(s)\n");*/
	g_async_queue_ref(slave_msg_queue);
	g_async_queue_push(slave_msg_queue,(gpointer)msg);
	g_async_queue_unref(slave_msg_queue);
	EXIT();
	return;
}


/*!
  \brief sends a message to connected slaves to change the color of the
  widgets in the passed group name
  \param clr is an enumeration representing the color
  \param groupname is the group to manipulate
  \see GuiColor
  */
G_MODULE_EXPORT void slaves_set_color(GuiColor clr, const gchar *groupname)
{
	static GAsyncQueue *slave_msg_queue = NULL;
	SlaveMessage *msg = NULL;

	ENTER();
	if (!slave_msg_queue)
		slave_msg_queue = (GAsyncQueue *)DATA_GET(global_data,"slave_msg_queue");
	if (!(GBOOLEAN)DATA_GET(global_data,"network_access"))
	{
		EXIT();
		return;
	}

	msg = g_new0(SlaveMessage, 1);
	msg->type = MTX_STATUS_CHANGED;
	msg->action = GROUP_SET_COLOR;
	msg->value = (guint8)clr;
	msg->data = g_strdup(groupname);
	msg->length = (guint16)(GINT)strlen(groupname);

	/*      printf("Sending message to peer(s)\n");*/
	g_async_queue_ref(slave_msg_queue);
	g_async_queue_push(slave_msg_queue,(gpointer)msg);
	g_async_queue_unref(slave_msg_queue);
	EXIT();
	return;
}


/*!
 \brief update_write_status() checks the differences between the current ECU
 data snapshot and the last one, if there are any differences (things need to
 be burnt) then it turns all the widgets in the "burners" group to RED
 \param data is a pointer to data sent to ECU used to
 update other widgets that refer to that Page/Offset
 */
G_MODULE_EXPORT void update_write_status(void *data)
{
	Io_Message *message = (Io_Message *)data;
	OutputData *output = (OutputData *)message->payload;
	guint8 **ecu_data = NULL;
	guint8 **ecu_data_last = NULL;
	gint i = 0;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint length = 0;
	gchar * tmpbuf = NULL;
	WriteMode mode = MTX_CMD_WRITE;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;
	ecu_data_last = firmware->ecu_data_last;

	if (!output)
		goto red_or_black;
	else
	{
		canID = (GINT)DATA_GET(output->data,"canID");
		page = (GINT)DATA_GET(output->data,"page");
		offset = (GINT)DATA_GET(output->data,"offset");
		length = (GINT)DATA_GET(output->data,"num_bytes");
		mode = (WriteMode)(GINT)DATA_GET(output->data,"mode");

		if (!message->status) /* Bad write! */
		{
			MTXDBG(SERIAL_WR,_("WRITE failed, rolling back!\n"));
			memcpy(ecu_data[page]+offset, ecu_data_last[page]+offset,length);
		}
	}
	if (output->queue_update)
	{
		if ((GINT)DATA_GET(global_data,"mtx_color_scale") == AUTO_COLOR_SCALE)
		{
			for (i=0;i<firmware->total_tables;i++)
			{
				// This at least only recalcs the limits on one... 
				if (firmware->table_params[i]->z_page == page)
				{
					recalc_table_limits_f(canID,i);
					if ((firmware->table_params[i]->last_z_maxval != firmware->table_params[i]->z_maxval) || (firmware->table_params[i]->last_z_minval != firmware->table_params[i]->z_minval))
					{
						tmpbuf = g_strdup_printf("table%i_color_id",i);
						if (!DATA_GET(global_data,tmpbuf))
						{
							guint id = g_timeout_add(200,(GSourceFunc)table_color_refresh_wrapper_f,GINT_TO_POINTER(i));
							DATA_SET(global_data,tmpbuf,GINT_TO_POINTER(id));
						}
						g_free(tmpbuf);
					}
				}
			}
		}

		if (mode == MTX_CHUNK_WRITE)
			thread_refresh_widget_range_f(page,offset,length);
		else
		{
			for (gint z=offset;z<offset+length;z++)
			{
				/*printf("refreshing widgets at page %i, offset %i\n",page,z);*/
				thread_refresh_widgets_at_offset_f(page,z);
			}
		}
		DATA_SET(global_data,"paused_handlers",GINT_TO_POINTER(FALSE));
	}
	/* We check to see if the last burn copy of the VE/constants matches 
	 * the currently set, if so take away the "burn now" notification.
	 * avoid unnecessary burns to the FLASH 
	 */

	if (DATA_GET(global_data,"offline"))
	{
		EXIT();
		return;
	}
red_or_black:
	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;

		if(memcmp(ecu_data_last[i],ecu_data[i],firmware->page_params[i]->length) != 0)
		{
			firmware->page_params[i]->needs_burn = TRUE;
			thread_set_group_color_f(RED,"burners");
			slaves_set_color(RED,"burners");
			EXIT();
			return;
		}
		else
			firmware->page_params[i]->needs_burn = FALSE;
	}
	thread_set_group_color_f(BLACK,"burners");
	slaves_set_color(BLACK,"burners");
	EXIT();
	return;
}


/*!
  \brief kludgy ugly monster that gets enabled whene hte serial I/O error
  crosses sa magic threshold, This basically closes the port and begins a 
  search for a valid device, and will exit when it does, or is cancelled
  \param data is unused
  */
G_MODULE_EXPORT void *serial_repair_thread(gpointer data)
{
	/* We got sent here because of one of the following occurred:
	 * Serial port isn't opened yet (app just fired up)
	 * Serial I/O errors (missing data, or failures reading/writing)
	 *  - This includes things like pulling the RS232 cable out of the ECU
	 * Serial port disappeared (i.e. device hot unplugged)
	 *  - This includes unplugging the USB side of a USB->Serial adapter
	 *    or going out of bluetooth range, for a BT serial device
	 *
	 * Thus we need to handle all possible conditions cleanly
	 */
	static gboolean serial_is_open = FALSE; /* Assume never opened */
	static GAsyncQueue *io_repair_queue = NULL;
	gchar * potential_ports = NULL;
	gint len = 0;
	gboolean autodetect = FALSE;
	guint8 buf [1024];
	gchar ** vector = NULL;
	guint i = 0;
	Serial_Params *serial_params = NULL;
	void (*unlock_serial_f)(void) = NULL;
	void (*close_serial_f)(void) = NULL;
	gboolean (*open_serial_f)(const gchar *,gboolean) = NULL;
	gboolean (*lock_serial_f)(const gchar *) = NULL;
	void (*setup_serial_params_f)(void) = NULL;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	get_symbol_f("setup_serial_params",(void **)&setup_serial_params_f);
	get_symbol_f("open_serial",(void **)&open_serial_f);
	get_symbol_f("close_serial",(void **)&close_serial_f);
	get_symbol_f("lock_serial",(void **)&lock_serial_f);
	get_symbol_f("unlock_serial",(void **)&unlock_serial_f);

	g_return_val_if_fail(setup_serial_params_f,NULL);
	g_return_val_if_fail(open_serial_f,NULL);
	g_return_val_if_fail(close_serial_f,NULL);
	g_return_val_if_fail(lock_serial_f,NULL);
	g_return_val_if_fail(unlock_serial_f,NULL);

	MTXDBG(THREADS,_("serial repair thread created!\n"));

	if (DATA_GET(global_data,"offline"))
	{
		g_timeout_add(100,(GSourceFunc)queue_function_f,(gpointer)"kill_conn_warning");
		MTXDBG(THREADS,_("Thread exiting, offline mode!\n"));
		g_thread_exit(0);
	}

	if (!io_repair_queue)
		io_repair_queue = (GAsyncQueue *)DATA_GET(global_data,"io_repair_queue");
	/* IF serial_is_open is true, then the port was ALREADY opened 
	 * previously but some error occurred that sent us down here. Thus
	 * first do a simple comms test, if that succeeds, then just cleanup 
	 * and return,  if not, close the port and essentially start over.
	 */
	if (serial_is_open == TRUE)
	{
		MTXDBG(SERIAL_RD|SERIAL_WR,_("Port considered open, but throwing errors\n"));
		i = 0;
		while (i <= 2)
		{
			MTXDBG(SERIAL_RD|SERIAL_WR,_("Calling comms_test, attempt %i\n"),i);
			if (comms_test())
			{
				MTXDBG(THREADS,_("Thread exiting, successfull comms test!\n"));
				g_thread_exit(0);
			}
			i++;
		}
		close_serial_f();
		unlock_serial_f();
		serial_is_open = FALSE;
		/* Fall through */
	}
	/* App just started, no connection yet*/
	while (!serial_is_open) 	
	{
		/* If "leaving" flag set, EXIT now */
		if (DATA_GET(global_data,"leaving"))
			g_thread_exit(0);
		MTXDBG(SERIAL_RD|SERIAL_WR,_("Port NOT considered open yet.\n"));
		autodetect = (GBOOLEAN) DATA_GET(global_data,"autodetect_port");
		if (!autodetect) /* User thinks he/she is S M A R T */
		{
			potential_ports = (gchar *)DATA_GET(global_data, "override_port");
			if (potential_ports == NULL)
				potential_ports = (gchar *)DATA_GET(global_data,"potential_ports");
		}
		else	/* Auto mode */
			potential_ports = (gchar *)DATA_GET(global_data,"potential_ports");
		vector = g_strsplit(potential_ports,",",-1);
		for (i=0;i<g_strv_length(vector);i++)
		{
			if (DATA_GET(global_data,"leaving"))
			{
				g_strfreev(vector);
				g_thread_exit(0);
			}
			/* Message queue used to exit immediately */
			if (g_async_queue_try_pop(io_repair_queue))
			{
				g_timeout_add(300,(GSourceFunc)queue_function_f,(gpointer)"kill_conn_warning");
				MTXDBG(THREADS,_("Thread exiting, told to!\n"));
				g_thread_exit(0);
			}
			if (!g_file_test(vector[i],G_FILE_TEST_EXISTS))
			{
				MTXDBG(SERIAL_RD|SERIAL_WR,_("Port %s does NOT exist\n"),vector[i]);

				/* Wait 100 ms to avoid deadlocking */
				g_usleep(100000);
				continue;
			}
			/* Wait 100 ms to avoid deadlocking */
			g_usleep(100000);
			MTXDBG(SERIAL_RD|SERIAL_WR,_("Attempting to open port %s\n"),vector[i]);
			thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Attempting to open port %s\n"),vector[i]),FALSE,FALSE);
			if (lock_serial_f(vector[i]))
			{
				if (open_serial_f(vector[i],FALSE))
				{
					if (autodetect)
						thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
					MTXDBG(SERIAL_RD|SERIAL_WR,_("Port %s opened\n"),vector[i]);
					setup_serial_params_f();
					/* read out any junk in buffer and toss it */
					read_wrapper_f(serial_params->fd,(guint8 *)&buf,1024,&len);

					thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Searching for ECU\n")),FALSE,FALSE);
					MTXDBG(SERIAL_RD|SERIAL_WR,_("Performing ECU comms test via port %s.\n"),vector[i]);
					if (comms_test())
					{       /* We have a winner !!  Abort loop */
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Search successfull\n")),FALSE,FALSE);
						serial_is_open = TRUE;
						thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
						break;
					}
					else
					{  
						MTXDBG(SERIAL_RD|SERIAL_WR,_("COMMS test failed, no ECU found, closing port %s.\n"),vector[i]);
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("No ECU found...\n")),FALSE,FALSE);
						close_serial_f();
						unlock_serial_f();
						/*g_usleep(100000);*/
					}
				}
				g_usleep(100000);
			}
			else
			{
				MTXDBG(SERIAL_RD|SERIAL_WR,_("Port %s is open by another application\n"),vector[i]);
				thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("Port %s is open by another application\n"),vector[i]),FALSE,FALSE);
			}
		}
		queue_function_f("conn_warning");
		g_strfreev(vector);
	}
	if (serial_is_open)
		queue_function_f("kill_conn_warning");
	MTXDBG(THREADS,_("Thread exiting, device found!\n"));
	g_thread_exit(0);
	EXIT();
	return NULL;
}


/*!
 \brief signal_read_rtvars() sends io message to I/O core to tell ms to send 
 back runtime vars
 */
G_MODULE_EXPORT void signal_read_rtvars(void)
{
	OutputData *output = NULL;
	extern gconstpointer *global_data;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);

	/* MS2 */
	if (firmware->capabilities & MS2)
	{
		output = initialize_outputdata_f();
		DATA_SET(output->data,"canID", GINT_TO_POINTER(firmware->canID));
		DATA_SET(output->data,"page", GINT_TO_POINTER(firmware->ms2_rt_page));
		DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->ms2_rt_page));
		DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
		io_cmd_f(firmware->rt_command,output);
	}
	else /* MS1 */
		io_cmd_f(firmware->rt_command,NULL);
	EXIT();
	return;
}


/*! 
 \brief build_output_message() is called when doing output to the ECU, to 
 append the needed data together into one nice blob for sending
 \param message is a pointer to the Io_Message structure to finish up
 \param command is a pointer to the Command strucutre which has the info on how
 to assemble the message properly
 \param data is a poiinter to the  OutputData structure which contains the 
 source info needed by this function to make the proper output stream
 */
G_MODULE_EXPORT void build_output_message(Io_Message *message, Command *command, gpointer data)
{
	guint i = 0;
	guint j = 0;
	gint v = 0;
	gint len = 0;
	gint offset = 0;
	gint num_bytes = 0;
	gint adder = 0;
	gint base_offset = 0;
	gint total_len = 0;
	guint8 * buffer = NULL;
	unsigned long crc32 = 0;
	OutputData *output = NULL;
	PotentialArg * arg = NULL;
	guint8 *sent_data = NULL;
	DBlock *block = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);

	if (data)
		output = (OutputData *)data;

	message->sequence = g_array_new(FALSE,TRUE,sizeof(DBlock *));

	/* Base command */
	block = g_new0(DBlock, 1);
	block->type = DATA;
	block->data = (guint8 *)g_strdup(command->base);
	if (!command->base)
		block->len = 0;
	else
		block->len = strlen(command->base);
	total_len += block->len;
	g_array_append_val(message->sequence,block);

	/* Arguments */
	for (i=0;i<command->args->len;i++)
	{
		arg = g_array_index(command->args,PotentialArg *, i);
		block = g_new0(DBlock, 1);
		if (arg->type == ACTION)
		{
			/*printf("build_output_string(): ACTION being created!\n");*/
			block->type = ACTION;
			block->action = arg->action;
			block->arg = arg->action_arg;
			g_array_append_val(message->sequence,block);
			continue;
		}
		if (arg->type == STATIC_STRING)
		{
			block->type = DATA;
			block->data = (guint8 *)g_strdup(arg->static_string);
			block->len = strlen(arg->static_string);
			total_len += block->len;
			g_array_append_val(message->sequence,block);
			continue;
		}
		if (!output)
			continue;
		switch (arg->size)
		{
			case MTX_U08:
			case MTX_S08:
			case MTX_CHAR:
				/*printf("8 bit arg %i, name \"%s\"\n",i,arg->internal_name);*/
				block->type = DATA;
				v = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("value %i\n",v);*/
				block->data = g_new0(guint8, 1);
				block->data[0 + base_offset] = (guint8)v;
				block->len = 1;
				total_len += block->len;
				break;
			case MTX_U16:
			case MTX_S16:
				/*printf("16 bit arg %i, name \"%s\"\n",i,arg->internal_name);*/
				block->type = DATA;
				v = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("value %i\n",v);*/
				block->data = g_new0(guint8,2);
				block->data[0 + base_offset] = (v >> 8 ) & 0xff;
				block->data[1 + base_offset] = v & 0xff;
				block->len = 2;
				total_len += block->len;
				break;
			case MTX_U32:
			case MTX_S32:
				/*                              printf("32 bit arg %i, name \"%s\"\n",i,arg->internal_name);*/
				block->type = DATA;
				v = (GINT)DATA_GET(output->data,arg->internal_name);
				/*                              printf("value %i\n",v); */
				block->data = g_new0(guint8,4);
				block->data[0 + base_offset] = (v > 24 ) & 0xff;
				block->data[1 + base_offset] = (v > 16 ) & 0xff;
				block->data[2 + base_offset] = (v > 8 ) & 0xff;
				block->data[3 + base_offset] = v & 0xff;
				block->len = 4;
				total_len += block->len;
				break;
			case MTX_UNDEF:
				/*printf("arg %i, name \"%s\"\n",i,arg->internal_name);*/
				block->type = DATA;
				if (!arg->internal_name)
					printf(_("ERROR, MTX_UNDEF, donno what to do!!\n"));
				sent_data = (guint8 *)DATA_GET(output->data,arg->internal_name);
				num_bytes = (GINT)DATA_GET(output->data,"num_bytes");
				block->data = (guint8 *)g_memdup(sent_data, num_bytes);
				block->len = num_bytes;
				total_len += block->len;
				/*
				   for (j=0;j<len;j++)
				   {
				   printf("sent_data[%i] is %i\n",j,sent_data[j]);
				   printf("block->data[%i] is %i\n",j,block->data[j]);
				   }
				 */
		}
		g_array_append_val(message->sequence,block);
	}
	if (firmware->capabilities & MS3_NEWSERIAL)
	{
		/* Append length prefix, calc CRC3 and add it */
		buffer = g_new0(guint8, total_len + 6 );
		buffer[0] = (total_len >> 8) & 0xff;
		buffer[1] = total_len & 0xff;
		offset = 2;
		for (i=0;i<message->sequence->len;i++)
		{
			block = g_array_index(message->sequence,DBlock *,i);
			if (block->type == DATA)
			{
				for (j=0;j<block->len;j++)
					buffer[offset++] = block->data[j];
			}
		}
		crc32 = crc32_computebuf(0, &buffer[2], total_len);
		buffer[offset + 0] = ((crc32 >> 24) & 0xff);
		buffer[offset + 1] = ((crc32 >> 16) & 0xff);
		buffer[offset + 2] = ((crc32 >> 8) & 0xff);
		buffer[offset + 3] = (crc32 & 0xff);
		DATA_SET(message->data,"burst_write",GINT_TO_POINTER(TRUE));
		DATA_SET_FULL(message->data,"burst_buffer",buffer,g_free);
		DATA_SET(message->data,"burst_len",GINT_TO_POINTER(total_len + 6));
	}
}

/*! 
  \brief stub function for the plugin
  */
G_MODULE_EXPORT gboolean setup_rtv(void)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*! 
  \brief stub function for the plugin
  */
G_MODULE_EXPORT gboolean teardown_rtv(void)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*----------------------------------------------------------------------------*\
 *  NAME:
 *     Crc32_ComputeBuf() - computes the CRC-32 value of a memory buffer
 *  DESCRIPTION:
 *     Computes or accumulates the CRC-32 value for a memory buffer.
 *     The 'inCrc32' gives a previously accumulated CRC-32 value to allow
 *     a CRC to be generated for multiple sequential buffer-fuls of data.
 *     The 'inCrc32' for the first buffer must be zero.
 *  ARGUMENTS:
 *     inCrc32 - accumulated CRC-32 value, must be 0 on first call
 *     buf     - buffer to compute CRC-32 value for
 *     bufLen  - number of bytes in buffer
 *  RETURNS:
 *     crc32 - computed CRC-32 value
 *  ERRORS:
 *     (no errors are possible)
 \*----------------------------------------------------------------------------*/
G_MODULE_EXPORT unsigned long crc32_computebuf( unsigned long inCrc32, const void *buf,size_t bufLen )
{
	static const unsigned long crcTable[256] = {
		0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,
		0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,
		0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,
		0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,
		0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,
		0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
		0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,
		0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
		0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,
		0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
		0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,
		0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
		0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
		0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,
		0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,
		0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
		0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,
		0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
		0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,
		0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
		0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,
		0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,
		0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,
		0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
		0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,
		0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
		0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,
		0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,
		0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,
		0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
		0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,
		0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
		0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,
		0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,
		0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,
		0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
		0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D };
	unsigned long crc32 = 0;
	unsigned char *byteBuf = NULL;
	size_t i = 0;

	ENTER();
	/** accumulate crc32 for buffer **/
	crc32 = inCrc32 ^ 0xFFFFFFFF;
	byteBuf = (unsigned char*) buf;
	for (i=0; i < bufLen; i++) {
		crc32 = (crc32 >> 8) ^ crcTable[ (crc32 ^ byteBuf[i]) & 0xFF ];
	}                                                                                                               
	EXIT();
	return( crc32 ^ 0xFFFFFFFF );
}

