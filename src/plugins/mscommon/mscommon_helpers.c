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

#include <args.h>
#include <config.h>
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <firmware.h>
#include <mscommon_comms.h>
#include <mscommon_helpers.h>
#include <mscommon_plugin.h>
#include <mtxsocket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern gconstpointer *global_data;


G_MODULE_EXPORT void startup_tcpip_sockets_pf(void)
{
	CmdLineArgs *args = NULL;

	args = DATA_GET(global_data,"args");
	if (args->network_mode)
		return;
	if ((DATA_GET(global_data,"interrogated")) && (!DATA_GET(global_data,"offline")))
		open_tcpip_sockets();
}

G_MODULE_EXPORT void spawn_read_ve_const_pf(void)
{
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	if (!firmware)
		return;

	gdk_threads_enter();
	set_title_f(g_strdup(_("Queuing read of all ECU data...")));
	gdk_threads_leave();
	io_cmd_f(firmware->get_all_command,NULL);
}


G_MODULE_EXPORT gboolean burn_all_helper(void *data, XmlCmdType type)
{
	OutputData *output = NULL;
	Command *command = NULL;
	gint i = 0;
	gint last_page = 0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	last_page = (gint)DATA_GET(global_data,"last_page");

	if (last_page == -1)
	{
		command = (Command *)data;
		io_cmd_f(NULL,command->post_functions);
		return TRUE;
	}
	/*printf("burn all helper\n");*/
	if ((type != MS2) && (type != MS1))
		return FALSE;
	if (!DATA_GET(global_data,"offline"))
	{
		/* MS2 extra is slightly different as it's paged like MS1 */
		if (((firmware->capabilities & MS2_E) || (firmware->capabilities & MS1) || (firmware->capabilities & MSNS_E)))
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
			for (i=0;i<firmware->total_pages;i++)
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
	return TRUE;
}

G_MODULE_EXPORT gboolean read_ve_const(void *data, XmlCmdType type)
{
	gint last_page;
	OutputData *output = NULL;
	Command *command = NULL;
	gint i = 0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	last_page = (gint)DATA_GET(global_data,"last_page");
	switch (type)
	{
		case MS1_VECONST:

			if (!DATA_GET(global_data,"offline"))
			{
				g_list_foreach(get_list_f("get_data_buttons"),set_widget_sensitive_f,GINT_TO_POINTER(FALSE));
				if (DATA_GET(global_data,"outstanding_data"))
					queue_burn_ecu_flash(last_page);
				for (i=0;i<firmware->total_pages;i++)
				{
					if (!firmware->page_params[i]->dl_by_default)
						continue;
					queue_ms1_page_change(i);
					output = initialize_outputdata_f();
					DATA_SET(output->data,"page",GINT_TO_POINTER(i));
					DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[i]->phys_ecu_page));
					DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
					io_cmd_f(firmware->ve_command,output);
				}
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS2_VECONST:
			if (!DATA_GET(global_data,"offline"))
			{
				g_list_foreach(get_list_f("get_data_buttons"),set_widget_sensitive_f,GINT_TO_POINTER(FALSE));
				if ((firmware->capabilities & MS2_E) && 
						(DATA_GET(global_data,"outstanding_data")))
					queue_burn_ecu_flash(last_page);
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
					io_cmd_f(firmware->ve_command,output);
				}
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS2_E_COMPOSITEMON:
			if (!DATA_GET(global_data,"offline"))
			{
				if ((firmware->capabilities & MS2_E) && 
						(DATA_GET(global_data,"outstanding_data")))
					queue_burn_ecu_flash(last_page);
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->compositemon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->compositemon_page]->phys_ecu_page));
				DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
				DATA_SET(output->data,"offset", GINT_TO_POINTER(0));
				DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(firmware->page_params[firmware->compositemon_page]->length));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->ve_command,output);
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS2_E_TRIGMON:
			if (!DATA_GET(global_data,"offline"))
			{
				if ((firmware->capabilities & MS2_E) && 
						(DATA_GET(global_data,"outstanding_data")))
					queue_burn_ecu_flash(last_page);
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->trigmon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->trigmon_page]->phys_ecu_page));
				DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
				DATA_SET(output->data,"offset", GINT_TO_POINTER(0));
				DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(firmware->page_params[firmware->trigmon_page]->length));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->ve_command,output);
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS2_E_TOOTHMON:
			if (!DATA_GET(global_data,"offline"))
			{
				if ((firmware->capabilities & MS2_E) && 
						(DATA_GET(global_data,"outstanding_data")))
					queue_burn_ecu_flash(last_page);
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->toothmon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->toothmon_page]->phys_ecu_page));
				DATA_SET(output->data,"canID",GINT_TO_POINTER(firmware->canID));
				DATA_SET(output->data,"offset", GINT_TO_POINTER(0));
				DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(firmware->page_params[firmware->toothmon_page]->length));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->ve_command,output);
			}
			command = (Command *)data;
			io_cmd_f(NULL,command->post_functions);
			break;
		case MS1_E_TRIGMON:
			if (!DATA_GET(global_data,"offline"))
			{
				if (DATA_GET(global_data,"outstanding_data"))
					queue_burn_ecu_flash(last_page);
				queue_ms1_page_change(firmware->trigmon_page);
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->trigmon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->trigmon_page]->phys_ecu_page));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->ve_command,output);
				command = (Command *)data;
				io_cmd_f(NULL,command->post_functions);
			}
			break;
		case MS1_E_TOOTHMON:
			if (!DATA_GET(global_data,"offline"))
			{
				if (DATA_GET(global_data,"outstanding_data"))
					queue_burn_ecu_flash(last_page);
				queue_ms1_page_change(firmware->toothmon_page);
				output = initialize_outputdata_f();
				DATA_SET(output->data,"page",GINT_TO_POINTER(firmware->toothmon_page));
				DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(firmware->page_params[firmware->toothmon_page]->phys_ecu_page));
				DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
				io_cmd_f(firmware->ve_command,output);
				command = (Command *)data;
				io_cmd_f(NULL,command->post_functions);
			}
			break;
		default:
			break;
	}
	return TRUE;
}


G_MODULE_EXPORT void enable_ttm_buttons_pf(void)
{
	gdk_threads_enter();
	g_list_foreach(get_list_f("ttm_buttons"),set_widget_sensitive_f,GINT_TO_POINTER(TRUE));
	gdk_threads_leave();
}


G_MODULE_EXPORT void simple_read_pf(void * data, XmlCmdType type)
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
	guint16 curcount = 0;
	guint8 *ptr8 = NULL;
	guint16 *ptr16 = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	message = (Io_Message *)data;
	output = (OutputData *)message->payload;

	switch (type)
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
			ptr8 = (guchar *)message->recv_buf;
			firmware->ecu_revision=(gint)ptr8[0];
			if (count > 0)
				thread_update_widget_f("ecu_revision_entry",MTX_ENTRY,g_strdup_printf("%.1f",((gint)ptr8[0]/10.0)));
			else
				thread_update_widget_f("ecu_revision_entry",MTX_ENTRY,g_strdup(""));
			break;
		case TEXT_REV:
			if (DATA_GET(global_data,"offline"))
				break;
			count = read_data_f(-1,&message->recv_buf,FALSE);
			if (count > 0)
			{
				thread_update_widget_f("text_version_entry",MTX_ENTRY,g_strndup(message->recv_buf,count));
				 firmware->txt_rev_len = count;
				firmware->text_revision = g_strndup(message->recv_buf,count);
			}
			break;
		case SIGNATURE:
			if (DATA_GET(global_data,"offline"))
				break;
			 count = read_data_f(-1,&message->recv_buf,FALSE);
                         if (count > 0)
			 {
				 thread_update_widget_f("ecu_signature_entry",MTX_ENTRY,g_strndup(message->recv_buf,count));
				 firmware->signature_len = count;
				 firmware->actual_signature = g_strndup(message->recv_buf,count);
			 }
			break;
		case MS1_VECONST:
		case MS2_VECONST:
			page = (GINT)DATA_GET(output->data,"page");
			canID = (GINT)DATA_GET(output->data,"canID");
			count = read_data_f(firmware->page_params[page]->length,&message->recv_buf,TRUE);
			if (count != firmware->page_params[page]->length)
				break;
			ms_store_new_block(canID,page,0,
					message->recv_buf,
					firmware->page_params[page]->length);
			ms_backup_current_data(canID,page);
			tmpi = (GINT)DATA_GET(global_data,"ve_goodread_count");
			DATA_SET(global_data,"ve_goodread_count",GINT_TO_POINTER(++tmpi));
			break;
		case MS1_RT_VARS:
			count = read_data_f(firmware->rtvars_size,&message->recv_buf,TRUE);
			if (count != firmware->rtvars_size)
				break;
			ptr8 = (guchar *)message->recv_buf;
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
				printf(_("MS1 Reset detected!, lastcount %i, current %i\n"),lastcount,ptr8[0]);
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
			process_rt_vars_f((void *)message->recv_buf);
			break;
		case MS2_RT_VARS:
			page = (GINT)DATA_GET(output->data,"page");
			canID = (GINT)DATA_GET(output->data,"canID");
			count = read_data_f(firmware->rtvars_size,&message->recv_buf,TRUE);
			if (count != firmware->rtvars_size)
				break;
			ms_store_new_block(canID,page,0,
					message->recv_buf,
					firmware->page_params[page]->length);
			ms_backup_current_data(canID,page);
			ptr16 = (guint16 *)message->recv_buf;
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
			process_rt_vars_f((void *)message->recv_buf);
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
				dbg_func_f(IO_PROCESS|SERIAL_RD,g_strdup_printf(__FILE__"\tECU  ERROR string: \"%s\"\n",tmpbuf));
				g_free(tmpbuf);
			}
			else
				thread_update_logbar_f("error_status_view",NULL,g_strdup("The data came back as gibberish, please try again...\n"),FALSE,FALSE);
			break;
		default:
			break;
	}
}

/*!
 \brief post_burn_pf() handles post burn ecu data mgmt
 */
G_MODULE_EXPORT void post_burn_pf(void)
{
	gint i = 0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	/* sync temp buffer with current burned settings */
	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;
		ms_backup_current_data(firmware->canID,i);
	}

	dbg_func_f(SERIAL_WR,g_strdup(__FILE__": post_burn_pf()\n\tBurn to Flash Completed\n"));

	return;
}


G_MODULE_EXPORT void post_single_burn_pf(void *data)
{
	Io_Message *message = (Io_Message *)data;
	OutputData *output = (OutputData *)message->payload;
	Firmware_Details *firmware = NULL;
	gint page = 0;

	firmware = DATA_GET(global_data,"firmware");
	page = (GINT)DATA_GET(output->data,"page");

	/* sync temp buffer with current burned settings */
	if (!firmware->page_params[page]->dl_by_default)
		return;
	ms_backup_current_data(firmware->canID,page);

	dbg_func_f(SERIAL_WR,g_strdup(__FILE__": post_single_burn_pf()\n\tBurn to Flash Completed\n"));

	return;
}
