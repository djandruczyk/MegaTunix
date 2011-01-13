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

#include <binlogger.h>
#include <config.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <glib.h>
#include <mem_mgmt.h>


extern gconstpointer *global_data;


G_MODULE_EXPORT void open_binary_logs(void)
{
	GIOChannel *ichan = NULL;
	GIOChannel *ochan = NULL;
	GError *err = NULL;
#ifdef __WIN32__
	gchar *ifilename = "C:\\program files\\megatunix\\inputlog.bin";
	gchar *ofilename = "C:\\program files\\megatunix\\outputlog.bin";
#else
	gchar *ifilename = "/tmp/inputlog.bin";
	gchar *ofilename = "/tmp/outputlog.bin";
#endif

	ichan = g_io_channel_new_file(ifilename,"w",&err);
	if (ichan)
	{
		g_io_channel_set_encoding(ichan,NULL,NULL);
		DATA_SET(global_data,"inbound_raw_logchan",ichan);
	}
	else
	{
		printf("unable to open input (reader) binary log, error %s\n",err->message);
		g_error_free(err);
	}
	ochan = g_io_channel_new_file(ofilename,"w",&err);
	if (ochan)
	{
		g_io_channel_set_encoding(ochan,NULL,NULL);
		DATA_SET(global_data,"outbound_raw_logchan",ochan);
	}
	else
	{
		printf("unable to open output (writer) binary log, error %s\n",err->message);
		g_error_free(err);
	}
}


G_MODULE_EXPORT void close_binary_logs(void)
{
	GIOChannel *ichan = NULL;
	GIOChannel *ochan = NULL;

	ichan = DATA_GET(global_data,"inbound_raw_logchan");
	if (ichan)
	{
		DATA_SET(global_data,"inbound_raw_logchan",NULL);
		g_io_channel_shutdown(ichan,TRUE,NULL);
		g_io_channel_unref(ichan);
	}
	ochan = DATA_GET(global_data,"outbound_raw_logchan");
	if (ochan)
	{
		DATA_SET(global_data,"outbound_raw_logchan",NULL);
		g_io_channel_shutdown(ochan,TRUE,NULL);
		g_io_channel_unref(ochan);
	}
}


G_MODULE_EXPORT gboolean flush_binary_logs(gpointer data)
{
	GIOChannel *ichan = NULL;
	GIOChannel *ochan = NULL;

	ichan = DATA_GET(global_data,"inbound_raw_logchan");
	ochan = DATA_GET(global_data,"outbound_raw_logchan");

	if (ichan)
		g_io_channel_flush(ichan,NULL);
	if (ochan)
		g_io_channel_flush(ochan,NULL);
	return TRUE;
}


G_MODULE_EXPORT void log_outbound_data(const void * buf, size_t count)
{
	static GIOChannel *ochan = NULL;

	if (!ochan)
		ochan = DATA_GET(global_data,"outbound_raw_logchan");
	if (ochan)
		g_io_channel_write_chars(ochan,buf,count,NULL,NULL);
}


G_MODULE_EXPORT void log_inbound_data(const void * buf, size_t count)
{
	static GIOChannel *ichan = NULL;

	if (!ichan)
		ichan = DATA_GET(global_data,"inbound_raw_logchan");
	if (ichan)
		g_io_channel_write_chars(ichan,buf,count,NULL,NULL);
}
