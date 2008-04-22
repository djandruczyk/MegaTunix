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
#include <conversions.h>
#include <dataio.h>
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <firmware.h>
#include <init.h>
#include <listmgmt.h>
#include <mode_select.h>
#include <notifications.h>
#include <rtv_processor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <widgetmgmt.h>


extern gint dbg_lvl;
extern GObject *global_data;


EXPORT void enable_interrogation_button_cb(void)
{
	extern GHashTable *dynamic_widgets;
	extern volatile gboolean offline;
	extern gboolean interrogated;

	if ((!offline) && (!interrogated))
		gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets, "interrogate_button")),TRUE);
}


EXPORT void start_statuscounts_cb(void)
{
	start_tickler(SCOUNTS_TICKLER);
}


EXPORT void enable_reboot_button_cb(void)
{
	extern GHashTable *dynamic_widgets;
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"error_status_reboot_button"),TRUE);
}


EXPORT void spawn_read_ve_const_cb(void)
{
	extern Firmware_Details *firmware;
	if (!firmware)
		return;
	
	io_cmd(firmware->get_all_command,NULL);
}


EXPORT gboolean ms2_burn_all_helper(void *data, XmlCmdType type)
{
	extern Firmware_Details *firmware;
	extern volatile gboolean offline;
	OutputData *output = NULL;
	Command *command = NULL;
	gint i = 0;
	if (type != MS2_STD)
		return FALSE;
	if (!offline)
	{
		for (i=0;i<firmware->total_pages;i++)
		{
			if (!firmware->page_params[i]->dl_by_default)
				continue;
			output = initialize_outputdata();
			OBJ_SET(output->object,"page",GINT_TO_POINTER(i));
			OBJ_SET(output->object,"truepgnum",GINT_TO_POINTER(firmware->page_params[i]->truepgnum));
			OBJ_SET(output->object,"canID",GINT_TO_POINTER(firmware->canID));
			OBJ_SET(output->object,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
			output->need_page_change = FALSE;
			io_cmd(firmware->burn_command,output);
		}
	}
	command = (Command *)data;
	io_cmd(NULL,command->post_functions);
	return TRUE;
}

EXPORT gboolean read_ve_const(void *data, XmlCmdType type)
{
	extern Firmware_Details *firmware;
	extern volatile gboolean offline;
	OutputData *output = NULL;
	Command *command = NULL;
	gint i = 0;

	g_list_foreach(get_list("get_data_buttons"),set_widget_sensitive,GINT_TO_POINTER(FALSE));
	switch (type)
	{
		case MS1_VECONST:

			if (!offline)
			{
				for (i=0;i<firmware->total_pages;i++)
				{
					if (!firmware->page_params[i]->dl_by_default)
						continue;
					output = initialize_outputdata();
					OBJ_SET(output->object,"page",GINT_TO_POINTER(i));
					OBJ_SET(output->object,"truepgnum",GINT_TO_POINTER(firmware->page_params[i]->truepgnum));
					OBJ_SET(output->object,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
					output->need_page_change = TRUE;
					io_cmd(firmware->ve_command,output);
				}
			}
			command = (Command *)data;
			io_cmd(NULL,command->post_functions);
			break;
		case MS2_VECONST:
			if (!offline)
			{
				for (i=0;i<firmware->total_pages;i++)
				{
					if (!firmware->page_params[i]->dl_by_default)
						continue;
					output = initialize_outputdata();
					OBJ_SET(output->object,"page",GINT_TO_POINTER(i));
					OBJ_SET(output->object,"truepgnum",GINT_TO_POINTER(firmware->page_params[i]->truepgnum));
					OBJ_SET(output->object,"canID",GINT_TO_POINTER(firmware->canID));
					OBJ_SET(output->object,"offset", GINT_TO_POINTER(0));
					OBJ_SET(output->object,"num_bytes", GINT_TO_POINTER(firmware->page_params[i]->length));
					OBJ_SET(output->object,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
					output->need_page_change = FALSE;
					io_cmd(firmware->ve_command,output);
				}
			}
			command = (Command *)data;
			io_cmd(NULL,command->post_functions);
			break;
		case MS1_E_TRIGMON:
			if (!offline)
			{
				output = initialize_outputdata();
				OBJ_SET(output->object,"page",GINT_TO_POINTER(firmware->trigmon_page));
				OBJ_SET(output->object,"truepgnum",GINT_TO_POINTER(firmware->page_params[firmware->trigmon_page]->truepgnum));
				output->need_page_change = TRUE;
				io_cmd(firmware->ve_command,output);
				command = (Command *)data;
				io_cmd(NULL,command->post_functions);
			}
			break;
		case MS1_E_TOOTHMON:
			if (!offline)
			{
				output = initialize_outputdata();
				OBJ_SET(output->object,"page",GINT_TO_POINTER(firmware->toothmon_page));
				OBJ_SET(output->object,"truepgnum",GINT_TO_POINTER(firmware->page_params[firmware->toothmon_page]->truepgnum));
				output->need_page_change = TRUE;
				io_cmd(firmware->ve_command,output);
				command = (Command *)data;
				io_cmd(NULL,command->post_functions);
			}
			break;
		default:
			break;
	}
	return TRUE;
}


EXPORT void enable_get_data_buttons_cb(void)
{
	g_list_foreach(get_list("get_data_buttons"),set_widget_sensitive,GINT_TO_POINTER(TRUE));
}


EXPORT void enable_ttm_buttons_cb(void)
{
	g_list_foreach(get_list("ttm_buttons"),set_widget_sensitive,GINT_TO_POINTER(TRUE));
}


EXPORT void conditional_start_rtv_tickler_cb(void)
{
	static gboolean just_starting = TRUE;

	if (just_starting)
	{
		start_tickler(RTV_TICKLER);
		just_starting = FALSE;
	}
}


EXPORT void set_store_black_cb(void)
{
	gint j = 0;
	extern Firmware_Details *firmware;

	set_group_color(BLACK,"burners");
	for (j=0;j<firmware->total_tables;j++)
		set_reqfuel_color(BLACK,j);
}

EXPORT void enable_3d_buttons_cb(void)
{
	g_list_foreach(get_list("3d_buttons"),set_widget_sensitive,GINT_TO_POINTER(TRUE));
}


EXPORT void disable_burner_buttons_cb(void)
{
	g_list_foreach(get_list("burners"),set_widget_sensitive,GINT_TO_POINTER(FALSE));
}

EXPORT void reset_temps_cb(void)
{
	reset_temps(OBJ_GET(global_data,"temp_units"));
}

EXPORT void simple_read_cb(void * data, XmlCmdType type)
{
	Io_Message *message  = NULL;
	OutputData *output  = NULL;
	gint count = 0;
	gchar *tmpbuf = NULL;
	gint page = -1;
	gint canID = -1;
	static gint lastcount = 0;
	extern Firmware_Details *firmware;
	extern gint ms_ve_goodread_count;
	extern gint ms_reset_count;
	extern gint ms_goodread_count;
	guint8 *ptr8 = NULL;
	guint16 *ptr16 = NULL;
	static gboolean just_starting = TRUE;
	extern gboolean forced_update;
	extern gboolean force_page_change;


	message = (Io_Message *)data;
	output = (OutputData *)message->payload;

	switch (type)
	{
		case WRITE_VERIFY:
			printf("MS2_WRITE_VERIFY not written yet\n");
			break;
		case MISMATCH_COUNT:
			printf("MS2_MISMATCH_COUNT not written yet\n");
			break;
		case MS1_CLOCK:
			printf("MS1_CLOCK not written yet\n");
			break;
		case MS2_CLOCK:
			printf("MS2_CLOCK not written yet\n");
			break;
		case REVISION:
			printf("REVISON not written yet\n");
			break;
		case SIGNATURE:
			printf("SIGNATURE not written yet\n");
			break;
		case MS1_VECONST:
		case MS2_VECONST:
			page = (gint)OBJ_GET(output->object,"page");
			canID = (gint)OBJ_GET(output->object,"canID");
			count = read_data(firmware->page_params[page]->length,&message->recv_buf);
			if (count != firmware->page_params[page]->length)
				break;
			store_new_block(canID,page,0,
					message->recv_buf,
					firmware->page_params[page]->length);
			backup_current_data(0,page);
			ms_ve_goodread_count++;
			break;
		case MS1_RT_VARS:
			count = read_data(firmware->rtvars_size,&message->recv_buf);
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
					(lastcount - ptr8[0] > 255))
			{
				ms_reset_count++;
				gdk_beep();
			}
			else
				ms_goodread_count++;
			lastcount = ptr8[0];
			/* Feed raw buffer over to post_process()
			 * as a void * and pass it a pointer to the new
			 * area for the parsed data...
			 */
			process_rt_vars((void *)message->recv_buf);
			break;
		case MS2_RT_VARS:
			count = read_data(firmware->rtvars_size,&message->recv_buf);
			if (count != firmware->rtvars_size)
				break;
			ptr16 = (guint16 *)message->recv_buf;
			/* Test for MS reset */
			if (just_starting)
			{
				lastcount = ptr16[0];
				just_starting = FALSE;
			}
			/* Check for clock jump from the MS, a 
			 * jump in time from the MS clock indicates 
			 * a reset due to power and/or noise.
			 */
			if ((lastcount - ptr16[0] > 1) && \
					(lastcount - ptr16[0] > 65535))
			{
				ms_reset_count++;
				printf("MS2 rtvars reset detected, lastcount %i, current %i\n",lastcount,ptr16[0]);
				gdk_beep();
			}
			else
				ms_goodread_count++;
			lastcount = ptr16[0];
			/* Feed raw buffer over to post_process()
			 * as a void * and pass it a pointer to the new
			 * area for the parsed data...
			 */
			process_rt_vars((void *)message->recv_buf);
			break;
		case MS2_BOOTLOADER:
			printf("MS2_BOOTLOADER not written yet\n");
			break;
		case MS1_GETERROR:
			forced_update = TRUE;
			force_page_change = TRUE;
			count = read_data(-1,&message->recv_buf);
			if (count <= 10)
			{
				thread_update_logbar("error_status_view",NULL,g_strdup("No ECU Errors were reported....\n"),FALSE,FALSE);
				break;
			}
			if (g_utf8_validate(((gchar *)message->recv_buf)+1,count-1,NULL))
			{
				thread_update_logbar("error_status_view",NULL,g_strndup(((gchar *)message->recv_buf+7)+1,count-8),FALSE,FALSE);
				if (dbg_lvl & (IO_PROCESS|SERIAL_RD))
				{
					tmpbuf = g_strndup(((gchar *)message->recv_buf)+1,count-1);
					dbg_func(g_strdup_printf(__FILE__"\tECU  ERROR string: \"%s\"\n",tmpbuf));
					g_free(tmpbuf);
				}

			}
			else
				thread_update_logbar("error_status_view",NULL,g_strdup("The data came back as gibberish, please try again...\n"),FALSE,FALSE);
			break;
		default:
			break;
	}
}

/*!
 \brief burn_ecu_flash() issues the commands to the ECU to burn the contents
 of RAM to flash.
 */
EXPORT void post_burn_cb()
{
	gint i = 0;
	extern Firmware_Details * firmware;

	if (dbg_lvl & SERIAL_WR)
		dbg_func(g_strdup(__FILE__": post_burn_cb()\n\tBurn to Flash Completed\n"));

	/* sync temp buffer with current burned settings */
	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;
		backup_current_data(firmware->canID,i);
	}

	return;
}
