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
  \file src/plugins/mscommon/mscommon_helpers.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS common helper functions referred to via comm.xml
  \author David Andruczyk
  */

#include <args.h>
#include <datamgmt.h>
#include <firmware.h>
#include <mscommon_comms.h>
#include <mscommon_helpers.h>
#include <mscommon_plugin.h>
#include <mtxsocket.h>
#include <serialio.h>
#include <stdio.h>

extern gconstpointer *global_data;

/*!
  \brief post function to fire up the TCP/IP sockets
  */
G_MODULE_EXPORT void startup_tcpip_sockets_pf(void)
{
	CmdLineArgs *args = NULL;

	ENTER();
	args = (CmdLineArgs *)DATA_GET(global_data,"args");
	if (args->network_mode)
	{
		EXIT();
		return;
	}
	if ((DATA_GET(global_data,"interrogated")) && (!DATA_GET(global_data,"offline")))
		open_tcpip_sockets();
	EXIT();
	return;
}


/*!
  \brief post function to issue a read of all ECU data
  */
G_MODULE_EXPORT void spawn_read_all_pf(void)
{
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!firmware)
	{
		EXIT();
		return;
	}

	set_title_f(g_strdup(_("Queuing read of all ECU data...")));
	io_cmd_f(firmware->get_all_command,NULL);
	EXIT();
	return;
}


/*!
  \brief  Burns all outstanding ECU pages to flash
  \param data is a pointer to a Command structure to be passed on the last
  call out
  \param func
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean burn_all_helper(void *data, FuncCall func)
{
	OutputData *output = NULL;
	Command *command = NULL;
	gint last_page = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	last_page = (GINT)DATA_GET(global_data,"last_page");

	if (last_page == -1)
	{
		command = (Command *)data;
		io_cmd_f(NULL,command->post_functions);
		EXIT();
		return TRUE;
	}
	/*printf("burn all helper\n");*/
	if ((firmware->capabilities & MS2) && (firmware->capabilities & MS1))
	{
		EXIT();
		return FALSE;
	}
	if (!DATA_GET(global_data,"offline"))
	{
		/* MS2 extra is slightly different as it's paged like MS1 */
		if (((firmware->capabilities & MS2_E) || (firmware->capabilities & MS1) || (firmware->capabilities & MS1_E)))
		{
			/*		printf("paged burn\n");*/
			output = initialize_outputdata_f();
			DATA_SET(output->data,"page",GINT_TO_POINTER(last_page));
			DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[last_page]->phys_ecu_page));
			DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
			DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
			io_cmd_f(firmware->burn_command,output);
		}
		else if ((firmware->capabilities & MS2) && (!(firmware->capabilities & MS2_E)))
		{
			/* MS2 std allows all pages to be in ram at will*/
			for (gint i=0;i<firmware->total_pages;i++)
			{
				if (!firmware->page_params[i]->dl_by_default)
					continue;
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(i));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[i]->phys_ecu_page));
				DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->burn_command,output);
			}
		}
	}
	command = (Command *)data;
	io_cmd_f(NULL,command->post_functions);
	EXIT();
	return TRUE;
}


/*!
  \brief function to do the actual read calls from the ECU
  \param data is a pointer to the Command structure
  \param func is an enumeration for the type of function
  \returns TRUE on success, FASE otherwise
  */
G_MODULE_EXPORT gboolean read_ve_const(void *data, FuncCall func)
{
	gint last_page;
	OutputData *output = NULL;
	Command *command = NULL;
	gint i = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	last_page = (GINT)DATA_GET(global_data,"last_page");
	switch (func)
	{
		case MS1_VECONST:

			if (!DATA_GET(global_data,"offline"))
			{
				g_list_foreach(get_list_f("get_data_buttons"),set_widget_sensitive_f,GINT_TO_POINTER(FALSE));

				for (i=0;i<firmware->total_pages;i++)
					if (firmware->page_params[i]->needs_burn)
						queue_burn_ecu_flash(i);
				for (i=0;i<firmware->total_pages;i++)
				{
					if (!firmware->page_params[i]->dl_by_default)
						continue;
					queue_ms1_page_change(i);
					output = initialize_outputdata_f();
					DATA_SET(output->data,"page",GINT_TO_POINTER(i));
					DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[i]->phys_ecu_page));
					DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
					io_cmd_f(firmware->read_command,output);
				}
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS2_VECONST:
			if (!DATA_GET(global_data,"offline"))
			{
				g_list_foreach(get_list_f("get_data_buttons"),set_widget_sensitive_f,GINT_TO_POINTER(FALSE));
				for (i=0;i<firmware->total_pages;i++)
					if (firmware->page_params[i]->needs_burn)
						queue_burn_ecu_flash(i);
				for (i=0;i<firmware->total_pages;i++)
				{
					if (!firmware->page_params[i]->dl_by_default)
						continue;
					output = initialize_outputdata_f();
					DATA_SET(output->data,"page",GINT_TO_POINTER(i));
					DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[i]->phys_ecu_page));
					DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
					DATA_SET(output->data,"offset", GINT_TO_POINTER(0));
					DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(firmware->page_params[i]->length));
					DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
					io_cmd_f(firmware->read_command,output);
				}
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS2_E_COMPOSITEMON:
			if (!DATA_GET(global_data,"offline"))
			{
				if (firmware->capabilities & MS2_E)
				{
					for (i=0;i<firmware->total_pages;i++)
						if (firmware->page_params[i]->needs_burn)
							queue_burn_ecu_flash(i);
				}
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->compositemon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->compositemon_page]->phys_ecu_page));
				DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
				DATA_SET(output->data,"offset", GINT_TO_POINTER(0));
				DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(firmware->page_params[firmware->compositemon_page]->length));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->read_command,output);
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS2_E_TRIGMON:
			if (!DATA_GET(global_data,"offline"))
			{
				if (firmware->capabilities & MS2_E)
				{
					for (i=0;i<firmware->total_pages;i++)
						if (firmware->page_params[i]->needs_burn)
							queue_burn_ecu_flash(i);
				}
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->trigmon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->trigmon_page]->phys_ecu_page));
				DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
				DATA_SET(output->data,"offset", GINT_TO_POINTER(0));
				DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(firmware->page_params[firmware->trigmon_page]->length));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->read_command,output);
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS2_E_TOOTHMON:
			if (!DATA_GET(global_data,"offline"))
			{
				if (firmware->capabilities & MS2_E)
				{
					for (i=0;i<firmware->total_pages;i++)
						if (firmware->page_params[i]->needs_burn)
							queue_burn_ecu_flash(i);
				}
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->toothmon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->toothmon_page]->phys_ecu_page));
				DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
				DATA_SET(output->data,"offset", GINT_TO_POINTER(0));
				DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(firmware->page_params[firmware->toothmon_page]->length));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->read_command,output);
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS1_E_TRIGMON:
			if (!DATA_GET(global_data,"offline"))
			{
				for (i=0;i<firmware->total_pages;i++)
					if (firmware->page_params[i]->needs_burn)
						queue_burn_ecu_flash(i);
				queue_ms1_page_change(firmware->trigmon_page);
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->trigmon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->trigmon_page]->phys_ecu_page));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->read_command,output);
				command = (Command *)data;
				io_cmd_f(NULL,command->post_functions);
			}
			break;
		case MS1_E_TOOTHMON:
			if (!DATA_GET(global_data,"offline"))
			{
				for (i=0;i<firmware->total_pages;i++)
					if (firmware->page_params[i]->needs_burn)
						queue_burn_ecu_flash(i);
				queue_ms1_page_change(firmware->toothmon_page);
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->toothmon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->toothmon_page]->phys_ecu_page));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->read_command,output);
				command = (Command *)data;
				io_cmd_f(NULL,command->post_functions);
			}
			break;
		default:
			break;
	}
	EXIT();
	return TRUE;
}


/*!
  \brief post functions to enable the TTM buttons
  */
G_MODULE_EXPORT void enable_ttm_buttons_pf(void)
{
	ENTER();
	g_list_foreach(get_list_f("ttm_buttons"),set_widget_sensitive_f,GINT_TO_POINTER(TRUE));
	EXIT();
	return;
}


/*!
  \brief handles a simple read result
  \param data is a pointer to a Io_Message structure
  \param func is an enum representing the action we need to handle
  */
G_MODULE_EXPORT void simple_read_hf(void * data, FuncCall func)
{
	static guint16 lastcount = 0;
	static gboolean just_starting = TRUE;
	Io_Message *message  = NULL;
	OutputData *output  = NULL;
	gint count = 0;
	gchar *tmpbuf = NULL;
	gint page = -1;
	gint canID = -1;
	gint tmpi = 0;
	gint adder = 0;
	gboolean check_header = FALSE;
	gint base_offset = 0;
	guint16 curcount = 0;
	guint8 *ptr8 = NULL;
	guint16 *ptr16 = NULL;
	Firmware_Details *firmware = NULL;
	Serial_Params *serial_params = NULL;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(serial_params);
	g_return_if_fail(firmware);

	message = (Io_Message *)data;
	output = (OutputData *)message->payload;
	if (firmware->capabilities & MS3_NEWSERIAL)
	{
		check_header = TRUE;
		adder = 7;
		base_offset = 3;
	}

	switch (func)
	{
		case WRITE_VERIFY:
			printf(_("MS2_WRITE_VERIFY not written yet\n"));
			break;
		case MISMATCH_COUNT:
			printf(_("MS2_MISMATCH_COUNT not written yet\n"));
			break;
		case MS1_CLOCK:
			printf(_("MS1_CLOCK not written yet\n"));
			break;
		case MS2_CLOCK:
			printf(_("MS2_CLOCK not written yet\n"));
			break;
		case NUM_REV:
			if (DATA_GET(global_data,"offline"))
				break;
			count = read_data_f(-1,&message->recv_buf,FALSE);
			if (count > adder)
			{
				ptr8 = (guchar *)message->recv_buf;
				firmware->ecu_revision=(gint)ptr8[0+base_offset];
				ecu_info_update(firmware);
			}
			break;
		case TEXT_REV:
			if (DATA_GET(global_data,"offline"))
				break;
			count = read_data_f(-1,&message->recv_buf,FALSE);
			if (count > adder)
			{
				if (check_header)
				{
					if (message->recv_buf[2] & 0x80)
					{
						MTXDBG(CRITICAL,_("TextRev read error: specifically 0X%x\n"),message->recv_buf[2]);
						flush_serial_f(serial_params->fd,BOTH);
						break;
					}
				}
				firmware->txt_rev_len = count-adder;
				firmware->text_revision = g_markup_escape_text((const gchar *)message->recv_buf+base_offset,count-adder);
				ecu_info_update(firmware);
			}
			break;
		case SIGNATURE:
			if (DATA_GET(global_data,"offline"))
				break;
			count = read_data_f(-1,&message->recv_buf,FALSE);
			if (count > adder)
			{
				if (check_header)
				{
					if (message->recv_buf[2] & 0x80)
					{
						MTXDBG(CRITICAL,_("Signature read error: specifically 0X%x\n"),message->recv_buf[2]);
						flush_serial_f(serial_params->fd,BOTH);
						break;
					}
				}
				firmware->signature_len = count-adder;
				firmware->actual_signature = g_markup_escape_text((const gchar *)message->recv_buf+base_offset,count-adder);
				ecu_info_update(firmware);
			}
			break;
		case MS1_VECONST:
		case MS2_VECONST:
			page = (GINT)DATA_GET(output->data,"page");
			canID = (GINT)DATA_GET(output->data,"canID");
			count = read_data_f(firmware->page_params[page]->length+adder,&message->recv_buf,TRUE);
			if ((count-adder) != firmware->page_params[page]->length)
				break;
			if (check_header)
			{
				if (message->recv_buf[2] & 0x80)
				{
					MTXDBG(CRITICAL,_("MS1/2_VECONST read error: specifically 0X%x\n"),message->recv_buf[2]);
					flush_serial_f(serial_params->fd,BOTH);
					break;
				}
			}
			ms_store_new_block(canID,page,0,
					((guint8 *)message->recv_buf)+base_offset,
					firmware->page_params[page]->length);
			ms_backup_current_data(canID,page);
			tmpi = (GINT)DATA_GET(global_data,"ve_goodread_count");
			DATA_SET(global_data,"ve_goodread_count",GINT_TO_POINTER(++tmpi));
			break;
		case MS1_RT_VARS:
			count = read_data_f(firmware->rtvars_size+adder,&message->recv_buf,TRUE);
			if ((count-adder) != firmware->rtvars_size)
				break;
			if (check_header)
			{
				if (message->recv_buf[2] & 0x80)
				{
					MTXDBG(CRITICAL,_("MS1_RT_VARS read error: specifically 0X%x\n"),message->recv_buf[2]);
					flush_serial_f(serial_params->fd,BOTH);
					break;
				}
			}
			ptr8 = (guchar *)message->recv_buf+base_offset;
			/* Test for MS reset */
			if (just_starting)
			{
				lastcount = ptr8[0];
				just_starting = FALSE;
			}
			/* Check for clock jump from the MS, a 
			 * jump in time from the MS clock indicates 
			 * a reset due to power and/or noise.
			 */
			if ((lastcount - ptr8[0] > 1) && \
					(lastcount - ptr8[0] != 255))
			{
				tmpi = (GINT)DATA_GET(global_data,"reset_count");
				DATA_SET(global_data,"reset_count",GINT_TO_POINTER(++tmpi));
				printf(_("ECU Reset detected!, lastcount %i, current %i\n"),lastcount,ptr8[0]);
				gdk_beep();
			}
			else
			{
				tmpi = (GINT)DATA_GET(global_data,"rt_goodread_count");
				DATA_SET(global_data,"rt_goodread_count",GINT_TO_POINTER(++tmpi));
			}
			lastcount = ptr8[0];
			/* Feed raw buffer over to post_process(void)
			 * as a void * and pass it a pointer to the new
			 * area for the parsed data...
			 */
			process_rt_vars_f(((guint8 *)message->recv_buf)+base_offset,firmware->rtvars_size);
			break;
		case MS2_RT_VARS:
			page = (GINT)DATA_GET(output->data,"page");
			canID = (GINT)DATA_GET(output->data,"canID");
			count = read_data_f(firmware->rtvars_size+adder,&message->recv_buf,TRUE);
			if ((count-adder) != firmware->rtvars_size)
				break;
			if (check_header)
			{
				if (message->recv_buf[2] & 0x80)
				{
					MTXDBG(CRITICAL,_("MS2_RT_VARS read error: specifically 0X%x\n"),message->recv_buf[2]);
					flush_serial_f(serial_params->fd,BOTH);
					break;
				}
			}
			ms_store_new_block(canID,page,0,
					((guint8 *)message->recv_buf)+base_offset,
					firmware->page_params[page]->length);
			ms_backup_current_data(canID,page);
			ptr16 = (guint16 *)message->recv_buf+base_offset;
			/* Test for MS reset */
			if (just_starting)
			{
				if (firmware->bigendian)
				{
					lastcount = GUINT16_TO_BE(ptr16[0]);
					curcount = GUINT16_TO_BE(ptr16[0]);
				}
				else
				{
					lastcount = GUINT16_TO_LE(ptr16[0]);
					curcount = GUINT16_TO_LE(ptr16[0]);
				}
				just_starting = FALSE;
			}
			else
			{
				if (firmware->bigendian)
					curcount = GUINT16_TO_BE(ptr16[0]);
				else
					curcount = GUINT16_TO_LE(ptr16[0]);
			}
			/* Check for clock jump from the MS, a 
			 * jump in time from the MS clock indicates 
			 * a reset due to power and/or noise.
			 */
			if ((lastcount - curcount > 1) && \
					(lastcount - curcount != 65535))
			{
				tmpi = (GINT)DATA_GET(global_data,"reset_count");
				DATA_SET(global_data,"reset_count",GINT_TO_POINTER(++tmpi));
				printf(_("MS2 rtvars reset detected, lastcount is %i, current %i"),lastcount,curcount);
				gdk_beep();
			}
			else
			{
				tmpi = (GINT)DATA_GET(global_data,"rt_goodread_count");
				DATA_SET(global_data,"rt_goodread_count",GINT_TO_POINTER(++tmpi));
			}
			lastcount = curcount;
			/* Feed raw buffer over to post_process(void)
			 * as a void * and pass it a pointer to the new
			 * area for the parsed data...
			 */
			process_rt_vars_f(((guint8 *)message->recv_buf)+base_offset,firmware->rtvars_size);
			break;
		case MS2_BOOTLOADER:
			printf(_("MS2_BOOTLOADER not written yet\n"));
			break;
		case MS1_GETERROR:
			DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
			DATA_SET(global_data,"force_page_change",GINT_TO_POINTER(TRUE));
			count = read_data_f(-1,&message->recv_buf,FALSE);
			if (count <= 10)
			{
				thread_update_logbar_f("error_status_view",NULL,g_strdup(_("No ECU Errors were reported....\n")),FALSE,FALSE);
				break;
			}
			if (g_utf8_validate(((gchar *)message->recv_buf)+1,count-1,NULL))
			{
				thread_update_logbar_f("error_status_view",NULL,g_strndup(((gchar *)message->recv_buf+7)+1,count-8),FALSE,FALSE);
				tmpbuf = g_strndup(((gchar *)message->recv_buf)+1,count-1);
				MTXDBG(IO_PROCESS|SERIAL_RD,_("ECU ERROR string: \"%s\"\n"),tmpbuf);
				g_free(tmpbuf);
			}
			else
				thread_update_logbar_f("error_status_view",NULL,g_strdup("The data came back as gibberish, please try again...\n"),FALSE,FALSE);
			break;
		default:
			break;
	}
	EXIT();
	return;
}

/*!
 \brief Copies current state to backup state
 */
G_MODULE_EXPORT void post_burn_pf(void)
{
	gint i = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	/* sync temp buffer with current burned settings */
	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;
		ms_backup_current_data(firmware->canID,i);
	}

	MTXDBG(SERIAL_WR,_("Burn to Flash Completed\n"));

	EXIT();
	return;
}


/*!
 \brief Copies current state to backup state
 */
G_MODULE_EXPORT void post_single_burn_pf(void *data)
{
	Io_Message *message = (Io_Message *)data;
	OutputData *output = (OutputData *)message->payload;
	Firmware_Details *firmware = NULL;
	gint page = 0;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	page = (GINT)DATA_GET(output->data,"page");

	/* sync temp buffer with current burned settings */
	if (!firmware->page_params[page]->dl_by_default)
	{
		EXIT();
		return;
	}
	ms_backup_current_data(firmware->canID,page);

	MTXDBG(SERIAL_WR,_("Burn to Flash Completed\n"));

	EXIT();
	return;
}


/*!
 \brief Updates General tab ECU info box
 */
G_MODULE_EXPORT void ecu_info_update(Firmware_Details *firmware)
{
	gchar *tmpbuf = NULL;
	ENTER();
	g_return_if_fail(firmware);
	tmpbuf = g_strdup_printf("<b>Firmware Version:</b> %s\n<b>Firmware Signature:</b> %s\n<b>Numeric Version:</b> %.1f",firmware->text_revision,firmware->actual_signature,firmware->ecu_revision/10.0);
	thread_update_widget_f("ecu_info_label",MTX_LABEL,g_strdup(tmpbuf));
	g_free(tmpbuf);

}
