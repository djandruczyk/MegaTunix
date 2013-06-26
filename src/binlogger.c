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
  \file src/binlogger.c
  \ingroup CoreMtx
  \brief Handles all aspects of binary logging of the serial I/O used for
  debugging purposes.
  \author David Andruczyk
  */

#include <binlogger.h>
#include <debugging.h>
#include <defines.h>

extern gconstpointer *global_data;
static GMutex imutex;
static GMutex omutex;

/*!
  \brief Opens the binary logs that record the raw serial IO activity for
  debugging purposes
  */
G_MODULE_EXPORT void open_binary_logs(void)
{
	GIOChannel *ichan = NULL;
	GIOChannel *ochan = NULL;
	GError *err = NULL;
	time_t *t = NULL;
	struct tm *tm = NULL;
	gchar *tmpbuf = NULL;

	ENTER();

	g_mutex_lock(&imutex);
	g_mutex_lock(&omutex);
	t = (time_t *)g_malloc(sizeof(time_t));
	time(t);
	tm = localtime(t);
	g_free(t);
	tmpbuf = g_strdup_printf("%i-%.2i-%.2i_%.2i%.2i",1900+(tm->tm_year),1+(tm->tm_mon),tm->tm_mday,tm->tm_hour,tm->tm_min);

#ifdef __WIN32__
	gchar *ifilename = g_strdup_printf("C:\\program files\\megatunix\\inputlog-%s.bin",tmpbuf);
	gchar *ofilename = g_strdup_printf("C:\\program files\\megatunix\\outputlog-%s.bin",tmpbuf);
#else
	gchar *ifilename = g_strdup_printf("/tmp/inputlog-%s.bin",tmpbuf);
	gchar *ofilename = g_strdup_printf("/tmp/outputlog-%s.bin",tmpbuf);
#endif
	g_free(tmpbuf);

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
	g_mutex_unlock(&imutex);
	g_mutex_unlock(&omutex);
	EXIT();
	return;
}


/*!
  \brief closes the binary logfiles
  */
G_MODULE_EXPORT void close_binary_logs(void)
{
	GIOChannel *ichan = NULL;
	GIOChannel *ochan = NULL;

	ENTER();

	g_mutex_lock(&imutex);
	ichan = (GIOChannel *)DATA_GET(global_data,"inbound_raw_logchan");
	if (ichan)
	{
		DATA_SET(global_data,"inbound_raw_logchan",NULL);
		g_io_channel_shutdown(ichan,TRUE,NULL);
		g_io_channel_unref(ichan);
	}
	g_mutex_unlock(&imutex);
	g_mutex_lock(&omutex);
	ochan = (GIOChannel *)DATA_GET(global_data,"outbound_raw_logchan");
	if (ochan)
	{
		DATA_SET(global_data,"outbound_raw_logchan",NULL);
		g_io_channel_shutdown(ochan,TRUE,NULL);
		g_io_channel_unref(ochan);
	}
	g_mutex_unlock(&omutex);
	EXIT();
	return;
}


/*!
  \brief flushes the binary log contents to disk
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean flush_binary_logs(gpointer data)
{
	GIOChannel *ichan = NULL;
	GIOChannel *ochan = NULL;

	ENTER();

	g_mutex_lock(&imutex);
	ichan = (GIOChannel *)DATA_GET(global_data,"inbound_raw_logchan");
	if (ichan)
		g_io_channel_flush(ichan,NULL);
	g_mutex_unlock(&imutex);

	g_mutex_lock(&omutex);
	ochan = (GIOChannel *)DATA_GET(global_data,"outbound_raw_logchan");
	if (ochan)
		g_io_channel_flush(ochan,NULL);
	g_mutex_unlock(&omutex);
	EXIT();
	return TRUE;
}


/*!
  \brief logs outbound (to ECU) data to the logfile
  \param buf is the pointer to the buffer to write
  \param count is how many bytes to write
  */
G_MODULE_EXPORT void log_outbound_data(const void * buf, size_t count)
{
	GIOChannel *ochan = NULL;

	ENTER();

	g_mutex_lock(&omutex);
	ochan = (GIOChannel *)DATA_GET(global_data,"outbound_raw_logchan");
	if (ochan)
		g_io_channel_write_chars(ochan,(const gchar *)buf,(gssize)count,NULL,NULL);
	g_mutex_unlock(&omutex);
	EXIT();
	return;
}


/*!
  \brief logs inbound (to ECU) data to the logfile
  \param buf is the pointer to the buffer to write
  \param count is how many bytes to write
  */
G_MODULE_EXPORT void log_inbound_data(const void * buf, size_t count)
{
	GIOChannel *ichan = NULL;

	ENTER();

	g_mutex_lock(&imutex);
	ichan = (GIOChannel *)DATA_GET(global_data,"inbound_raw_logchan");
	if (ichan)
		g_io_channel_write_chars(ichan,(const gchar *)buf,(gssize)count,NULL,NULL);
	g_mutex_unlock(&imutex);
	EXIT();
	return;
}
