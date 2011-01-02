/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <freeems_helpers.h>
#include <freeems_plugin.h>
#include <packet_handlers.h>
#include <threads.h>



G_MODULE_EXPORT void stop_streaming(void)
{
	OutputData *output = NULL;
	GCond *cond = NULL;
	GMutex *mutex = g_mutex_new();
	GTimeVal tval;
	gint res = 0;

	cond = g_cond_new();
	printf("Stop streaming!\n");
	output = initialize_outputdata_f();
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_ASYNC_DATALOG_TYPE));
	DATA_SET(output->data,"databyte",GINT_TO_POINTER(0));
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(6));
	register_packet_condition(SEQUENCE_NUM,cond,6);
	io_cmd("datalog_mgmt",output);
	g_get_current_time(&tval);
	g_time_val_add(&tval,5000000);
	g_mutex_lock(mutex);
	printf("going to wait on cond %p\n",cond);
	res = g_cond_timed_wait(cond,mutex,&tval);
	g_mutex_unlock(mutex);
	if (res)
		printf("COND ARRIVED!\n");
	else
		printf("TIMEOUT\n");
	return;
}
