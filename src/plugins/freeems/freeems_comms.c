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
#include <debugging.h>
#include <defines.h>
#include <freeems_comms.h>
#include <freeems_plugin.h>
#include <gtk/gtk.h>
#include <packet_handlers.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <poll.h>
#endif
#include <serialio.h>
#include <string.h>
#include <template.h>
#include <sys/select.h>
#include <unistd.h>


extern gconstpointer *global_data;

void read_error(gpointer);
void *win32_reader(gpointer);

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
	 * Thus we need to handle all possible conditions if possible
	 */
	static gboolean serial_is_open = FALSE; /* Assume never opened */
	static GAsyncQueue *io_repair_queue = NULL;
	gchar * potential_ports;
	gint len = 0;
	gboolean autodetect = FALSE;
	guchar buf [1024];
	gchar ** vector = NULL;
	guint i = 0;
	Serial_Params *serial_params = NULL;
	void (*unlock_serial_f)(void) = NULL;
	void (*close_serial_f)(void) = NULL;
	gboolean (*open_serial_f)(const gchar *,gboolean) = NULL;
	gboolean (*lock_serial_f)(const gchar *) = NULL;
	void (*setup_serial_params_f)(void) = NULL;

	serial_params = DATA_GET(global_data,"serial_params");

	get_symbol_f("setup_serial_params",(void *)&setup_serial_params_f);
	get_symbol_f("open_serial",(void *)&open_serial_f);
	get_symbol_f("close_serial",(void *)&close_serial_f);
	get_symbol_f("lock_serial",(void *)&lock_serial_f);
	get_symbol_f("unlock_serial",(void *)&unlock_serial_f);
	dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread created!\n"));

	if (DATA_GET(global_data,"offline"))
	{
		g_timeout_add(100,(GSourceFunc)queue_function_f,"kill_conn_warning");
		dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, offline mode!\n"));
		g_thread_exit(0);
	}
	if (!io_repair_queue)
		io_repair_queue = DATA_GET(global_data,"io_repair_queue");
	/* IF serial_is_open is true, then the port was ALREADY opened 
	 * previously but some error occurred that sent us down here. Thus
	 * first do a simple comms test, if that succeeds, then just cleanup 
	 * and return,  if not, close the port and essentially start over.
	 */
	if (serial_is_open == TRUE)
	{
		dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port considered open, but throwing errors\n"));
		freeems_serial_disable();
		close_serial_f();
		unlock_serial_f();
		serial_is_open = FALSE;
		/* Fall through */
	}
	while (!serial_is_open)
	{
		/* If "leaving" flag set, EXIT now */
		if (DATA_GET(global_data,"leaving"))
			g_thread_exit(0);
		dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port NOT considered open yet.\n"));
		autodetect = (GBOOLEAN) DATA_GET(global_data,"autodetect_port");
		if (!autodetect) /* User thinks he/she is S M A R T */
		{
			potential_ports = (gchar *)DATA_GET(global_data, "override_port");
			if (potential_ports == NULL)
				potential_ports = (gchar *)DATA_GET(global_data,"potential_ports");
		}
		else    /* Auto mode */
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
				g_timeout_add(300,(GSourceFunc)queue_function_f,"kill_conn_warning");
				dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, told to!\n"));
				g_thread_exit(0);
			}
			if (!g_file_test(vector[i],G_FILE_TEST_EXISTS))
			{
				dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port %s does NOT exist\n",vector[i]));

				/* Wait 200 ms to avoid deadlocking */
				g_usleep(200000);
				continue;
			}
			g_usleep(100000);
			dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Attempting to open port %s\n",vector[i]));
			thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Attempting to open port %s\n"),vector[i]),FALSE,FALSE);
			if (lock_serial_f(vector[i]))
			{
				if (open_serial_f(vector[i],TRUE))
				{
					if (autodetect)
						thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
					dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port %s opened\n",vector[i]));
					setup_serial_params_f();
					freeems_serial_enable();

					thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Searching for ECU\n")),FALSE,FALSE);
					dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Performing ECU comms test via port %s.\n",vector[i]));
					if (comms_test())
					{       /* We have a winner !!  Abort loop */
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Search successfull\n")),FALSE,FALSE);
						serial_is_open = TRUE;
						break;
					}
					else
					{
						dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t COMMS test failed, no ECU found, closing port %s.\n",vector[i]));
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("No ECU found...\n")),FALSE,FALSE);
						freeems_serial_disable();
						close_serial_f();
						unlock_serial_f();
						/*g_usleep(100000);*/
					}
				}
				g_usleep(100000);
			}
			else
			{
				dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port %s is open by another application\n",vector[i]));
				thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("Port %s is open by another application\n"),vector[i]),FALSE,FALSE);
			}
		}
		queue_function_f("conn_warning");
	}

	if (serial_is_open)
	{
		queue_function_f("kill_conn_warning");
		thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
	}
	if (vector)
		g_strfreev(vector);
	dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, device found!\n"));
	g_thread_exit(0);
	return NULL;
}
		

G_MODULE_EXPORT void freeems_serial_disable(void)
{
	GIOChannel *channel = NULL;
	GAsyncQueue *queue = NULL;
	Serial_Params *serial_params = NULL;
	GTimeVal now;
	GCond *cond = NULL;
	GMutex *mutex = g_mutex_new();
	gboolean res = FALSE;
	gint tmpi = 0;

	/*printf("freeems serial DISable!\n");*/
#ifdef __WIN32__
	g_mutex_lock(mutex);
	g_get_current_time(&now);
        g_time_val_add(&now,250000);
        cond = DATA_GET(global_data,"serial_reader_cond");
        res = g_cond_timed_wait(cond,mutex,&now);
	if (res)
		printf("condition signaled\n");
	else
		printf("cond timeout\n");
	g_mutex_unlock(mutex);
	g_mutex_free(mutex);
#else
	serial_params = DATA_GET(global_data,"serial_params");
	DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
	channel = DATA_GET(global_data,"serial_channel");
	if (channel)
		g_io_channel_shutdown(channel,FALSE,NULL);
	DATA_SET(global_data,"serial_channel",NULL);
	DATA_SET(global_data,"read_watch",NULL);
#endif
}


G_MODULE_EXPORT void freeems_serial_enable(void)
{
	GIOChannel *channel = NULL;
	Serial_Params *serial_params = NULL;
	gint tmpi = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	if ((!serial_params->open) || (!serial_params->fd))
	{
		dbg_func_f(CRITICAL,g_strdup(_(__FILE__": freeems_serial_setup, serial port is NOT open, or filedescriptor is invalid!\n")));
		return;
	}

#ifdef __WIN32__
	g_thread_create(win32_reader,GINT_TO_POINTER(serial_params->fd),TRUE,NULL);
	return;
#endif
	channel = g_io_channel_unix_new(serial_params->fd);
	DATA_SET(global_data,"serial_channel",channel);
	/* Set to raw mode */
	g_io_channel_set_encoding(channel, NULL, NULL);
	/* Set to unbuffered mode */
	g_io_channel_set_buffered(channel, FALSE);
	/* Reader */
	tmpi = g_io_add_watch_full(channel,0,G_IO_IN,able_to_read,NULL, read_error);
	DATA_SET(global_data,"read_watch",GINT_TO_POINTER(tmpi));

/*	// Writer 
 *	tmpi = g_io_add_watch(channel,G_IO_OUT, able_to_write,NULL);
 *	DATA_SET(global_data,"write_watch",GINT_TO_POINTER(tmpi));
 *
 *	// Error Catcher 
 *	tmpi = g_io_add_watch(channel,G_IO_ERR|G_IO_HUP|G_IO_NVAL, serial_error,NULL);
 *	DATA_SET(global_data,"error_watch",GINT_TO_POINTER(tmpi));
 */

}


G_MODULE_EXPORT void read_error(gpointer data)
{
	printf("READ ERROR, disabling io_channel\n");
	freeems_serial_disable();
}


G_MODULE_EXPORT gboolean able_to_read(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	static gsize wanted = 2048;
	gboolean res = FALSE;
	gchar buf[2048];
	gchar *ptr = NULL;
	gsize requested = 2048;
	gsize received = 0;
	gsize read_pos = 0;
	GIOStatus status;
	GError *err = NULL;

	read_pos = requested-wanted;
	status = g_io_channel_read_chars(channel, &buf[read_pos], wanted, &received, &err);
	/*printf("Want %i, got %i,",wanted, received); */
	wanted -= received;
	/*	printf("Still need %i\n",wanted); */
	if (wanted <= 0)
		wanted = 2048;
	if (err)
	{
		printf("error reported: \"%s\"\n",err->message);
		g_error_free(err);
	}
	switch (status)
	{
		case G_IO_STATUS_ERROR:
			printf("IO ERROR!\n");
			res =  FALSE;
			break;
		case G_IO_STATUS_NORMAL:
			res = TRUE;
			break;
		case G_IO_STATUS_EOF:
			printf("EOF!\n");
			res = FALSE;
			break;
		case G_IO_STATUS_AGAIN:
			printf("TEMP UNAVAIL!\n");
			res =  TRUE;
			break;
	}
	printf("read %i bytes\n",received);
	if (res)
		handle_data((guchar *)buf+read_pos,received);
	/* Returning false will cause the channel to shutdown*/
	return res;
}



G_MODULE_EXPORT gboolean able_to_write(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	g_usleep(100000);
	printf("Able to write, sleeping 100 ms\n");
	return TRUE;
}


G_MODULE_EXPORT gboolean serial_error(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	switch (cond)
	{
		case G_IO_IN: 
			printf("data waiting to be read!\n");
			break;
		case G_IO_OUT: 
			printf("channel ready to be written to!\n");
			break;
		case G_IO_PRI: 
			printf("Priority data waiting to be read!\n");
			break;
		case G_IO_ERR: 
			printf("ERROR condition!\n");
			break;
		case G_IO_HUP: 
			printf("Hungup/connection brokenm resetting!\n");
			break;
		case G_IO_NVAL: 
			printf("Invalid req, descriptor not openm resetting!\n");
			break;
	}
	return TRUE;
}


G_MODULE_EXPORT gboolean comms_test(void)
{
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	gint len = 0;
	/* Packet sends back Interface Version */
	/* START, Header, Payload ID H, PAyload ID L, CKsum, STOP */
	unsigned char pkt[6] = {0xAA,0x00,0x00,0x00,0x00,0xCC};
	Serial_Params *serial_params = NULL;

	serial_params = DATA_GET(global_data,"serial_params");
	queue = DATA_GET(global_data,"packet_queue");

	dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\t Entered...\n"));
	if (!serial_params)
		return FALSE;
	g_get_current_time(&tval);
	g_time_val_add(&tval,100000);
	g_async_queue_ref(queue);
	packet = g_async_queue_timed_pop(queue,&tval);
	g_async_queue_unref(queue);
	if (packet)
	{
		dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\t Packet Arrived!!\n"));
		DATA_SET(global_data,"connected",GINT_TO_POINTER(TRUE));
		g_free(packet->data);
		g_free(packet);
		return TRUE; 
	}
	else
	{ /* Assume ECU is in non-streaming mode, try and probe it */
		dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tRequesting FreeEMS Interface Version\n"));
		if (!write_wrapper_f(serial_params->fd,&pkt, 6, &len))
			return FALSE;
		g_get_current_time(&tval);
		g_time_val_add(&tval,100000);
		g_async_queue_ref(queue);
		packet = g_async_queue_timed_pop(queue,&tval);
		g_async_queue_unref(queue);
		if (packet)
		{
			dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tFound via probing!!\n"));
			DATA_SET(global_data,"connected",GINT_TO_POINTER(TRUE));
			g_free(packet->data);
			g_free(packet);
			return TRUE; 
		}
	}
	DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
	dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tNo device found...\n"));
	return FALSE;
}


void *win32_reader(gpointer data)
{
	gint fd = (gint)data;
	gint errcount = 0;
	static gsize wanted = 2048;
	gboolean res = FALSE;
	gchar buf[2048];
	gchar *ptr = NULL;
	gsize requested = 2048;
	gsize received = 0;
	gsize read_pos = 0;
	GIOStatus status;
	GError *err = NULL;
	GCond *cond = NULL;

	cond = DATA_GET(global_data,"serial_reader_cond");
	while (TRUE)
	{
		read_pos = requested-wanted;
		received = read(fd, &buf[read_pos], wanted);
		g_usleep(10000);
		//printf("Want %i, got %i,",wanted, received); 
		if (received == -1)
		{
			DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
			g_cond_signal(cond);
			g_thread_exit(0);
		}

		wanted -= received;
		/*	printf("Still need %i\n",wanted); */
		if (wanted <= 0)
			wanted = 2048;
		//printf("WIN32 read %i bytes\n",received);
		if (received > 0)
			handle_data((guchar *)buf+read_pos,received);
		g_cond_signal(cond);
	}
}


void *unix_reader(gpointer data)
{
	gint fd = (gint)data;
	gint errcount = 0;
	static gsize wanted = 2048;
	gboolean res = FALSE;
	gchar buf[2048];
	gchar *ptr = NULL;
	gsize requested = 2048;
	gsize received = 0;
	gsize read_pos = 0;
	GIOStatus status;
	GError *err = NULL;
	GCond *cond = NULL;
	fd_set readfds;
	struct timeval t;


	cond = DATA_GET(global_data,"serial_reader_cond");
	FD_SET(fd,&readfds);

	while (TRUE)
	{
		t.tv_sec = 1;
		t.tv_usec = 0;
		res = select(fd+1,&readfds, NULL, NULL, &t);
		if (res == 0)
		{
			printf("select timeout!\n");
			g_cond_signal(cond);
			continue;
		}
		if (FD_ISSET(fd,&readfds))
		{
			read_pos = requested-wanted;
			received = read(fd, &buf[read_pos], wanted);
			printf("Want %i, got %i,",wanted, received); 
			if (received == -1)
			{
				DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
				g_cond_signal(cond);
				g_thread_exit(0);
			}

			wanted -= received;
			/*	printf("Still need %i\n",wanted); */
			if (wanted <= 0)
				wanted = 2048;
			printf("UNIX read %i bytes\n",received);
			if (received > 0)
				handle_data((guchar *)buf+read_pos,received);
			g_cond_signal(cond);
		}
	}
}


/*! 
 \brief build_output_message() is called when doing output to the ECU, to 
 append the needed data together into one nice blob for sending
 */
G_MODULE_EXPORT void build_output_message(Io_Message *message, Command *command, gpointer data)
{
	gboolean have_sequence = FALSE;
	gboolean have_payload_id = FALSE;
	gboolean have_location_id = FALSE;
	gboolean have_offset = FALSE;
	gboolean have_length = FALSE;
	gboolean have_datablock = FALSE;
	gboolean have_databyte = FALSE;
	guint8 *payload_data = NULL;
	gint payload_data_length = 0;
	gint byte = 0;
	gint seq_num = -1;
	gint payload_id = -1;
	gint location_id = -1;
	gint offset = -1;
	gint length = -1;
	gint packet_length = 2; /* Header + cksum, rest come in below */
	gint payload_length = 0;
	guint i = 0;
	gint pos = 0;
	guint8 sum = 0;
	OutputData *output = NULL;
	PotentialArg * arg = NULL;
	guint8 *buf = NULL; /* Raw packet before escapes/start/stop */
	DBlock *block = NULL;

	if (data)
		output = (OutputData *)data;

	message->sequence = g_array_new(FALSE,TRUE,sizeof(DBlock *));

	payload_length = 0;
	/* Arguments */
	for (i=0;i<command->args->len;i++)
	{
		arg = g_array_index(command->args,PotentialArg *, i);
		switch (arg->type)
		{
			case ACTION:
				/*printf("build_output_message(): ACTION being created!\n");*/
				block = g_new0(DBlock, 1);
				block->type = ACTION;
				block->action = arg->action;
				block->arg = arg->action_arg;
				g_array_append_val(message->sequence,block);
				break;
			case SEQUENCE_NUM:
				have_sequence = TRUE;
				seq_num = (GINT)DATA_GET(output->data,arg->internal_name);
				printf("Sequence number present %i\n",seq_num);
				packet_length += 1;
				break;
			case PAYLOAD_ID:
				have_payload_id = TRUE;
				payload_id = (GINT)DATA_GET(output->data,arg->internal_name);
				printf("Payload ID number present %i\n",payload_id);
				packet_length += 2;
				break;
				/* Payload specific stuff */
			case LOCATION_ID:
				have_location_id = TRUE;
				location_id = (GINT)DATA_GET(output->data,arg->internal_name);
				printf("Location ID number present %i\n",location_id);
				payload_length += 2;
				break;
			case OFFSET:
				have_offset = TRUE;
				offset = (GINT)DATA_GET(output->data,arg->internal_name);
				printf("Location ID number present %i\n",location_id);
				payload_length += 2;
				break;
			case LENGTH:
				have_length = TRUE;
				length = (GINT)DATA_GET(output->data,arg->internal_name);
				printf("Payload length present %i\n",length);
				payload_length += 2;
				break;
			case DATABYTE:
				have_databyte = TRUE;
				byte = (GINT)DATA_GET(output->data,arg->internal_name);
				printf("DataByte present %i\n",byte);
				packet_length += 1;
				break;
			case DATA:
				have_datablock = TRUE;
				payload_data = (guint8 *)DATA_GET(output->data,arg->internal_name);
				payload_data_length = (GINT)DATA_GET(output->data,"num_bytes");
				payload_length += payload_data_length;
				break;
			default:
				printf("FreeEMS doesn't handle this type %s\n",arg->name);
				break;
		}
	}

	pos = 1;
	packet_length += payload_length;
	printf("total raw packet length (-start/end/cksum) %i\n",packet_length);
	/* Raw Packet */
	buf = g_new0(guint8, packet_length);

	/* Payload ID */
	buf[H_PAYLOAD_IDX] = (guint8)((payload_id & 0xff00) >> 8);
	buf[L_PAYLOAD_IDX] = (guint8)(payload_id & 0x00ff);
	pos += 2;

	/* Sequence number if present */
	if (have_sequence > 0)
	{
		buf[HEADER_IDX] |= HAS_SEQUENCE_MASK;
		buf[SEQ_IDX] = (guint8)seq_num;
		pos += 1;
	}

	/* Payload Length if present */
	if (payload_length > 0)
	{
		printf("payload length is %i\n",payload_length);
		buf[HEADER_IDX] |= HAS_LENGTH_MASK;
		if (have_sequence > 0)
		{
			buf[H_LEN_IDX] = (guint8)((payload_length & 0xff00) >> 8); 
			buf[L_LEN_IDX] = (guint8)(payload_length & 0x00ff); 
		}
		else
		{
			buf[H_LEN_IDX - 1] = (guint8)((payload_length & 0xff00) >> 8);
			buf[L_LEN_IDX - 1] = (guint8)(payload_length & 0x00ff); 
		}
		pos += 2;

		/* Location ID */
		buf[pos++] = (guint8)((location_id & 0xff00) >> 8); 
		buf[pos++] = (guint8)(location_id & 0x00ff); 
		/* Offset */
		buf[pos++] = (guint8)((offset & 0xff00) >> 8); 
		buf[pos++] = (guint8)(offset & 0x00ff); 
		/* Sub Length */
		buf[pos++] = (guint8)((length & 0xff00) >> 8); 
		buf[pos++] = (guint8)(length & 0x00ff); 
		
		g_memmove(buf+pos,payload_data,payload_data_length);
		pos += payload_data_length;
	}
	else if (have_databyte) /* For odd cmds that don't have a full payload */
		buf[pos++] = (guint8)(byte & 0x00ff); 

	/* Checksum it */
	for (i=0;i<packet_length;i++)
		sum += buf[i];
	buf[pos] = sum;
	pos++;

	/* Escape + start/stop it */
	block = g_new0(DBlock, 1);
	block->type = DATA;
	block->data = finalize_packet(buf,packet_length,&block->len);
	g_free(buf);
	g_array_append_val(message->sequence,block);
}


guint8 *finalize_packet(guint8 *raw, gint raw_len, gint *final_len )
{
	gint i = 0;
	gint num_2_escape = 0;
	gint markers = 2;
	gint len = 0;
	gint pos = 0;
	guint8 *buf = NULL;
	/* This should allocate a buffer,
	   Escape any special bytes in the packet
	   Checksum it
	   Add start/end flags to it
	 */
	printf("finalize, raw input length is %i\n",raw_len);
	for (i=0;i<raw_len;i++)
	{
		printf("raw[%i] is %i\n",i,raw[i]);
		if ((raw[i] == START_BYTE) || (raw[i] == STOP_BYTE) || (raw[i] == ESCAPE_BYTE))
			num_2_escape++;
	}
	len = raw_len + num_2_escape + markers;
	printf("length of final pkt is %i\n",len);
	buf = g_new0(guint8,len);
	buf[0] = START_BYTE;
	pos = 1;
	for (i=0;i<raw_len;i++)
	{
		if ((raw[i] == START_BYTE) \
				|| (raw[i] == STOP_BYTE) \
				|| (raw[i] == ESCAPE_BYTE))
		{
			buf[pos] = ESCAPE_BYTE;
			pos++;
			buf[pos] = raw[i] ^ 0xFF;
			pos++;
		}
		else
		{
			buf[pos] = raw[i];
			pos++;
		}
	}
	buf[pos] = STOP_BYTE;
	printf("last byte at index %i, Stop is %i, buf %i\n",pos,STOP_BYTE,buf[pos]);
	printf("final length is %i\n",len);
	for (i=0;i<len;i++)
		printf("Packet index %i, value 0x%0.2X\n",i,buf[i]);
	if (len -1 != pos)
		printf("packet finalize problem, length mismatch\n");
	*final_len = len;
	return buf;
}
