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
  \file src/plugins/libreems/libreems_comms.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS Specific comms routines
  \author David Andruczyk
  */

#include <datamgmt.h>
#include <defines.h>
#include <firmware.h>
#include <libreems_comms.h>
#include <libreems_plugin.h>
#include <packet_handlers.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <poll.h>
#include <sys/select.h>
#endif
#include <serialio.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

extern gconstpointer *global_data;

/*!
  \brief This thread attempts to reconnect to a device that has vanished
  either due to Serial IO errors, or a loss of the physical device (USB unplug)
  This is a thread and will exit if specific flags are set or when it manages
  to re-connect to the device
  \param data is unused
  \returns NULL
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

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	get_symbol_f("setup_serial_params",(void **)&setup_serial_params_f);
	get_symbol_f("open_serial",(void **)&open_serial_f);
	get_symbol_f("close_serial",(void **)&close_serial_f);
	get_symbol_f("lock_serial",(void **)&lock_serial_f);
	get_symbol_f("unlock_serial",(void **)&unlock_serial_f);
	MTXDBG(THREADS|CRITICAL,_("LibreEMS serial_repair_thread() created!\n"));

	if (DATA_GET(global_data,"offline"))
	{
		g_timeout_add(100,(GSourceFunc)queue_function_f,(gpointer)"kill_conn_warning");
		MTXDBG(THREADS|CRITICAL,_("LibreEMS serial_repair_thread() exiting, offline mode!\n"));
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
		libreems_serial_disable();
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
		MTXDBG(SERIAL_RD|SERIAL_WR,_("Port NOT considered open yet.\n"));
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
		for (guint i=0;i<g_strv_length(vector);i++)
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
				MTXDBG(THREADS|CRITICAL,_("LibreEMS serial_repair_thread() exiting, told to!\n"));
				g_thread_exit(0);
			}
			if (!g_file_test(vector[i],G_FILE_TEST_EXISTS))
			{
				MTXDBG(SERIAL_RD|SERIAL_WR,_("Port %s does NOT exist\n"),vector[i]);

				/* Wait 100 ms to avoid deadlocking */
				g_usleep(100000);
				continue;
			}
			g_usleep(100000);
			MTXDBG(SERIAL_RD|SERIAL_WR,_("Attempting to open port %s\n"),vector[i]);
			thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Attempting to open port %s\n"),vector[i]),FALSE,FALSE);
			if (lock_serial_f(vector[i]))
			{
				if (open_serial_f(vector[i],TRUE))
				{
					if (autodetect)
						thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
					MTXDBG(SERIAL_RD|SERIAL_WR,_("Port %s opened\n"),vector[i]);
					setup_serial_params_f();
					libreems_serial_enable();

					thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Searching for ECU\n")),FALSE,FALSE);
					MTXDBG(SERIAL_RD|SERIAL_WR,_("Performing ECU comms test via port %s.\n"),vector[i]);
					if (comms_test())
					{       /* We have a winner !!  Abort loop */
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Search successfull\n")),FALSE,FALSE);
						serial_is_open = TRUE;
						break;
					}
					else
					{
						MTXDBG(SERIAL_RD|SERIAL_WR,_("COMMS test failed, no ECU found, closing port %s.\n"),vector[i]);
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("No ECU found...\n")),FALSE,FALSE);
						libreems_serial_disable();
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
	}

	if (serial_is_open)
	{
		queue_function_f("kill_conn_warning");
		thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
	}
	if (vector)
		g_strfreev(vector);
	MTXDBG(THREADS|CRITICAL,_("LibreEMS serial_repair_thread()  exiting, device found!\n"));
	g_thread_exit(0);
	EXIT();
	return NULL;
}
		

/*!
 \brief Disables the serial connection and shuts down the reader thread
  */
G_MODULE_EXPORT void libreems_serial_disable(void)
{
	GIOChannel *channel = NULL;
	GAsyncQueue *queue = NULL;
	Serial_Params *serial_params = NULL;
	GMutex mutex;
	GCond *cond = NULL;
	GThread *thread = NULL;
	gboolean res = FALSE;
	gint tmpi = 0;

	ENTER();
	g_mutex_init(&mutex);
	DATA_SET(global_data,"serial_abort",GINT_TO_POINTER(TRUE));
	thread = (GThread *)DATA_GET(global_data,"serial_thread_id");
	g_mutex_lock(&mutex);
	/* Wait up to 0.25 seconds for thread to exit */
	cond = (GCond *)DATA_GET(global_data,"serial_reader_cond");
	g_return_if_fail(cond);
	if ((cond) && (thread))
	{
		res = g_cond_wait_until(cond,&mutex,g_get_monotonic_time() + 250 * G_TIME_SPAN_MILLISECOND);
		g_thread_join(thread);
	}
	DATA_SET(global_data,"serial_thread_id",NULL);
	/*
	   if (res)
	   printf("condition signaled\n");
	   else
	   printf("cond timeout\n");
	 */
	g_mutex_unlock(&mutex);
	g_mutex_clear(&mutex);
	EXIT();
	return;
}


/*!
 \brief Enables the serial connection and starts up  the reader thread
  */
G_MODULE_EXPORT void libreems_serial_enable(void)
{
	GIOChannel *channel = NULL;
	Serial_Params *serial_params = NULL;
	GThread *thread = NULL;
	gint tmpi = 0;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	if ((!serial_params->open) || (!serial_params->fd))
	{
		MTXDBG(CRITICAL,_("Serial port is NOT open, or filedescriptor is invalid!\n"));
		EXIT();
		return;
	}

	DATA_SET(global_data,"serial_abort",GINT_TO_POINTER(FALSE));
#ifdef __WIN32__
	thread = g_thread_new("Win32 Reader",win32_reader,GINT_TO_POINTER(serial_params->fd));
#else
	thread = g_thread_new("UNIX Reader",unix_reader,GINT_TO_POINTER(serial_params->fd));
#endif
	DATA_SET(global_data,"serial_thread_id",thread);
	EXIT();
	return;
}


/*!
  \brief Simple communication test, Assembles a packet, sends it, 
  subscribes to the response and waits for it, or a timeout.
  \returns TRUE on success, FALSE on failure
  */
G_MODULE_EXPORT gboolean comms_test(void)
{
	GAsyncQueue *queue = NULL;
	LibreEMS_Packet *packet = NULL;
	GCond *cond = NULL;
	gboolean res = FALSE;
	gint len = 0;
	/* Packet sends back Interface Version */
	/* START, Header, Payload ID H, PAyload ID L, CKsum, STOP */
	guint8 *buf = NULL;
	/* Raw packet */
	guint8 pkt[INTERFACE_VERSION_REQ_PKT_LEN];
	gint tmit_len = 0;

	ENTER();
	Serial_Params *serial_params = NULL;

	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	queue = (GAsyncQueue *)DATA_GET(global_data,"packet_queue");

	MTXDBG(SERIAL_RD,_("Entered...\n"));
	if (!serial_params)
	{
		EXIT();
		return FALSE;
	}
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_BASIC_DATALOG);
	packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,250000);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_BASIC_DATALOG);
	if (packet)
	{
		MTXDBG(SERIAL_RD,_("Found streaming ECU!!\n"));
		g_async_queue_unref(queue);
		libreems_packet_cleanup(packet);
		DATA_SET(global_data,"connected",GINT_TO_POINTER(TRUE));
		EXIT();
		return TRUE;
	}
	else
	{ /* Assume ECU is in non-streaming mode, try and probe it */
		gint sum = 0;
		MTXDBG(SERIAL_RD,_("Requesting LibreEMS Interface Version\n"));
		register_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
		pkt[HEADER_IDX] = 0;
		pkt[H_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0xff00 ) >> 8;
		pkt[L_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0x00ff );
		for (gint i=0;i<INTERFACE_VERSION_REQ_PKT_LEN-1;i++)
			sum += pkt[i];
		pkt[INTERFACE_VERSION_REQ_PKT_LEN-1] = sum;
		buf = finalize_packet((guint8 *)&pkt,INTERFACE_VERSION_REQ_PKT_LEN,&tmit_len);

		if (!write_wrapper_f(serial_params->fd, buf, tmit_len, &len))
		{
			g_free(buf);
			deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
			g_async_queue_unref(queue);
			EXIT();
			return FALSE;
		}
		g_free(buf);
		packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,250000);
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
		g_async_queue_unref(queue);
		if (packet)
		{
			MTXDBG(SERIAL_RD,_("Found via probing!!\n"));
			libreems_packet_cleanup(packet);
			DATA_SET(global_data,"connected",GINT_TO_POINTER(TRUE));
			EXIT();
			return TRUE; 
		}
	}
	DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
	MTXDBG(SERIAL_RD,_("No device found...\n"));
	EXIT();
	return FALSE;
}


/*!
  \brief Windows reader thread.  Runs continuously once port is open to a 
  working device. This is a strict timeout blocking read due to windows's
  absolutelybraindead I/O model where select/poll isn't allowed on anything
  but network sockets. Overallpaed I/O is just too ugly...
  \param data is the encapsulation of the port filedescriptor
  \returns NULL on termination
  */
void *win32_reader(gpointer data)
{
	gint fd = (GINT)data;
	gint errcount = 0;
	static gsize wanted = 2048;
	gboolean res = FALSE;
	gchar buf[2048];
	gchar *ptr = NULL;
	gsize requested = 2048;
	gssize received = 0;
	gsize read_pos = 0;
	GIOStatus status;
	GError *err = NULL;
	GCond *cond = NULL;

	ENTER();
	cond = (GCond *)DATA_GET(global_data,"serial_reader_cond");
	while (TRUE)
	{
		if ((DATA_GET(global_data,"leaving")) || (DATA_GET(global_data,"serial_abort")))
		{
			if (cond)
				g_cond_signal(cond);
			g_thread_exit(0);
		}
		read_pos = requested-wanted;
		received = read(fd, &buf[read_pos], wanted);
		g_usleep(10000);
		/*printf("Want %i, got %i,",wanted, received); */
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
		/*printf("WIN32 read %i bytes\n",received);*/
		if (received > 0)
			handle_data((guchar *)buf+read_pos,received);
		g_cond_signal(cond);
	}
	EXIT();
	return NULL;
}


/*!
  \brief Unix serial read thread, initiated as soon as a connection is made.
  Uses select() and nonblocking I/O in order to be nice with the port
  \param data is the encapsulation of the serial/network port filedescriptor
  \returns NULL/0
  */
void *unix_reader(gpointer data)
{
	gint fd = (GINT)data;
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

	ENTER();
	cond = (GCond *)DATA_GET(global_data,"serial_reader_cond");
	FD_ZERO(&readfds);
	while (TRUE)
	{
		if ((DATA_GET(global_data,"leaving")) || (DATA_GET(global_data,"serial_abort")))
		{
			if (cond)
				g_cond_signal(cond);
			g_thread_exit(0);
		}
		t.tv_sec = 0;
		t.tv_usec = 100000;
		FD_SET(fd,&readfds);
		res = select(fd+1,&readfds, NULL, NULL, &t);
		if (res == -1)
		{
			DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
			g_cond_signal(cond);
			g_thread_exit(0);
		}
		if (res == 0) /* Timeout */
		{
			g_cond_signal(cond);
			continue;
		}
		if (FD_ISSET(fd,&readfds))
		{
			read_pos = requested-wanted;
			received = read(fd, &buf[read_pos], wanted);
			/*printf("Want %i, got %i,",wanted, received); */
			if (received <= 0)
			{
				DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
				g_cond_signal(cond);
				g_thread_exit(0);
			}

			wanted -= received;
			/*	printf("Still need %i\n",wanted); */
			if (wanted <= 0)
				wanted = 2048;
			/*printf("UNIX read %i bytes\n",received);*/
			if (received > 0)
				handle_data((guchar *)buf+read_pos,received);
			g_cond_signal(cond);
		}
	}
	EXIT();
	return NULL;
}


/*!
  \brief Generic per-firmware function to setup the Runtime Vars Handling.
  This initiates a subscriber queue, anda thread to handle requests coming in
  the queue
  \returns TRUE
  */
G_MODULE_EXPORT void setup_rtv_pf(void)
{
	GAsyncQueue *queue = NULL;
	GThread *thread = NULL;

	ENTER();
	queue = g_async_queue_new();
	DATA_SET(global_data,"rtv_subscriber_queue", queue);
	thread = g_thread_new("RTV Subscriber thread", rtv_subscriber,queue);
	DATA_SET(global_data,"rtv_subscriber_thread", thread);
	/* This sends packets to the rtv_subscriber queue */
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_BASIC_DATALOG);
	/* Fake out so tickler doesn't try to start it */
	DATA_SET(global_data,"realtime_id",GINT_TO_POINTER(1));
	EXIT();
	return;
}


/*
 \brief Per plugin handler which shuts down the RTV stuff. Kills off the 
 rtv_subscriber thread, dismantles the queue in preparation for plugin shutdown
 \returns TRUE
 */
G_MODULE_EXPORT gboolean teardown_rtv(void)
{
	GAsyncQueue *queue = NULL;
	GThread *thread = NULL;
	GMutex *mutex = (GMutex *)DATA_GET(global_data,"rtv_subscriber_mutex");

	ENTER();
	/* This sends packets to the rtv_subscriber queue */
	thread = (GThread *)DATA_GET(global_data,"rtv_subscriber_thread");
	if (thread)
	{
		DATA_SET(global_data,"rtv_subscriber_thread_exit",GINT_TO_POINTER(1));
		g_thread_join(thread);
		DATA_SET(global_data,"rtv_subscriber_thread",NULL);
		DATA_SET(global_data,"rtv_subscriber_thread_exit",NULL);
	}
	g_mutex_lock(mutex);
	queue = (GAsyncQueue *)DATA_GET(global_data,"rtv_subscriber_queue");
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_BASIC_DATALOG);
	g_async_queue_unref(queue);
	DATA_SET(global_data,"rtv_subscriber_queue",NULL);
	g_mutex_unlock(mutex);
	DATA_SET(global_data,"realtime_id",NULL);
	EXIT();
	return TRUE;
}


/*! 
  \brief The thread which pops stuff off the RTV queue and processes them
  \param data is the pointer to the queue to pop entries off of
  \returns NULL
  */
G_MODULE_EXPORT void *rtv_subscriber(gpointer data)
{
	GAsyncQueue *queue = (GAsyncQueue *)data;
	static GMutex *mutex = NULL;
	LibreEMS_Packet *packet = NULL;

	ENTER();
	mutex = (GMutex *)DATA_GET(global_data,"rtv_subscriber_mutex");
	if (!mutex)
		g_thread_exit(0);

	while (!DATA_GET(global_data,"rtv_subscriber_thread_exit"))
	{
		/* Wait up to 0.25 seconds for thread to exit */
		packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,250000);
		g_mutex_unlock(mutex);
		if (packet)
		{
			DATA_SET(global_data,"rt_goodread_count",GINT_TO_POINTER((GINT)DATA_GET(global_data,"rt_goodread_count")+1));
			process_rt_vars_f(packet->data+packet->payload_base_offset,packet->payload_length);
			libreems_packet_cleanup(packet);
		}
	}
	g_thread_exit(0);
	EXIT();
	return NULL;
}


/*! 
  \brief Per firmware plugin function to signal a request for ECU Runtime vars
  */
G_MODULE_EXPORT void signal_read_rtvars(void)
{
	OutputData *output = NULL;	
	Firmware_Details *firmware = NULL;

	ENTER();
        firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);

	output = initialize_outputdata_f();
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(1));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_BASIC_DATALOG));
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
	io_cmd_f(firmware->rt_command,output);
	EXIT();
	return;
}


/*!
 \brief Generic that gets called to send a value to the ECU.  This function
 is has an ECU agnostic interface and is for sending single 8-32 bit bits of 
 data to the ECU. This one extracts the important things from the passed ptr
 and sends to the real function which is ecu specific
 \param data is the pointer to the container of the important bits like
 LocationID or Page, canID, offset and size
 \param value is the new value to send to the ECU
 \param queue_update is a flag that triggers the ECU to update
 */
G_MODULE_EXPORT void send_to_ecu(gpointer data, gint value, gboolean queue_update)
{
	static Firmware_Details *firmware = NULL;
	gint page = 0;
	gint canID = 0;
	gint locID = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	GtkWidget *widget = (GtkWidget *)data;
	gconstpointer *gptr = (gconstpointer *)data;

	ENTER();
	if (!firmware)
        	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);

	if (GTK_IS_WIDGET(widget))
	{
		if (!OBJ_GET(widget,"location_id"))
		{
			page = (GINT)OBJ_GET(widget,"page");
			locID = firmware->page_params[page]->phys_ecu_page;
		}
		else
			locID = (GINT)OBJ_GET(widget,"location_id");
		canID = (GINT)OBJ_GET(widget,"canID");
		offset = (GINT)OBJ_GET(widget,"offset");
		size = (DataSize)(GINT)OBJ_GET(widget,"size");
	}
	else
	{
		if (!DATA_GET(gptr,"location_id"))
		{
			page = (GINT)DATA_GET(gptr,"page");
			locID = firmware->page_params[page]->phys_ecu_page;
		}
		else
			locID = (GINT)DATA_GET(gptr,"location_id");
		canID = (GINT)DATA_GET(gptr,"canID");
		offset = (GINT)DATA_GET(gptr,"offset");
		size = (DataSize)(GINT)DATA_GET(gptr,"size");
	}
	/*printf("locID %i, offset, %i, value %i\n",locID,offset,value);*/
	libreems_send_to_ecu(canID,locID,offset,size,value,queue_update);
	EXIT();
	return;
}


/*!
 \brief ECU specifc that gets called to send a value to the ECU. 
 \param canID is the can Identifier
 \param locID is the Location ID to where this value belongs
 \param offset is the offset from the beginning of the page that this data
 refers to.
 \param size is an enumeration corresponding to how big this variable is
 \param value is the value that should be sent to the ECU At page/offset
 \param queue_update if true queues a gui update, used to prevent
 a horrible stall when doing an ECU restore or batch load...
 */
G_MODULE_EXPORT void libreems_send_to_ecu(gint canID, gint locID, gint offset, DataSize size, gint value, gboolean queue_update)
{
	static Firmware_Details *firmware = NULL;
	OutputData *output = NULL;
	guint8 *data = NULL;
	guint16 u16 = 0;
	gint16 s16 = 0;
	guint32 u32 = 0;
	gint32 s32 = 0;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(firmware);
	g_return_if_fail(offset >= 0);

	MTXDBG(SERIAL_WR,_("Sending locID %i, offset %i, value %i \n"),locID,offset,value);

	switch (size)
	{
		case MTX_CHAR:
		case MTX_S08:
		case MTX_U08:
			/*printf("8 bit var %i at offset %i\n",value,offset);*/
			break;
		case MTX_S16:
		case MTX_U16:
			/*printf("16 bit var %i at offset %i\n",value,offset);*/
			break;
		case MTX_S32:
		case MTX_U32:
			/*printf("32 bit var %i at offset %i\n",value,offset);*/
			break;
		default:
			printf(_("libreems_send_to_ecu() ERROR!!! Size undefined for variable at canID %i, offset %i\n"),locID,offset);
	}
	output = initialize_outputdata_f();
	DATA_SET(output->data,"location_id", GINT_TO_POINTER(locID));
	DATA_SET(output->data,"payload_id", GINT_TO_POINTER(REQUEST_UPDATE_BLOCK_IN_RAM));
	DATA_SET(output->data,"offset", GINT_TO_POINTER(offset));
	DATA_SET(output->data,"size", GINT_TO_POINTER(size));
	DATA_SET(output->data,"value", GINT_TO_POINTER(value));
	DATA_SET(output->data,"length", GINT_TO_POINTER(get_multiplier_f(size)));
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_SIMPLE_WRITE));
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
	DATA_SET_FULL(output->data,"data",(gpointer)data, g_free);
	/* Set it here otherwise there's a risk of a missed burn due to 
	 * a potential race condition in the burn checker
	 */
	libreems_set_ecu_data(canID,locID,offset,size,value);
	/* IF the packet fails, update_write_status will rollback properly */

	output->queue_update = queue_update;
	io_cmd_f(firmware->write_command,output);
	EXIT();
	return;
}


/*!
 \brief Generic chunk write function that takes the internal mtx_page 
 and translates it as needed to the ECU specific page

 \param canID is the can identifier (0-14)
 \param page is the mtxpage where this data should reside
 \param offset is the offset from the beginning of the page that this data
 refers to.
 \param num_bytes is the length of block to sent
 \param block is the block of data to be sent which better damn well be
 int ECU byte order if there is an endianness thing..
 */
G_MODULE_EXPORT void ecu_chunk_write(gint canID, gint page, gint offset, gint num_bytes,guint8 *block)
{
	gint locID = 0;
	Firmware_Details *firmware = NULL;
	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);

	locID = firmware->page_params[page]->phys_ecu_page;

	libreems_chunk_write(canID,locID,offset,num_bytes,block);
	EXIT();
	return;
}


/*!
 \brief ECU Specif gets called to send a block of values to the ECU.
 \param canID is the can identifier (0-14)
 \param locID is the locationID in which the value refers to.
 \param offset is the offset from the beginning of the page that this data
 refers to.
 \param num_bytes is the length of block to sent
 \param block is the block of data to be sent which better damn well be
 int ECU byte order if there is an endianness thing..
 */
G_MODULE_EXPORT void libreems_chunk_write(gint canID, gint locID, gint offset, gint num_bytes, guint8 * block)
{
	OutputData *output = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	MTXDBG(SERIAL_WR,_("Sending canID %i, locID %i, offset %i, num_bytes %i, data %p\n"),canID,locID,offset,num_bytes,block);
	output = initialize_outputdata_f();
	DATA_SET(output->data,"canID", GINT_TO_POINTER(canID));
	DATA_SET(output->data,"location_id", GINT_TO_POINTER(locID));
	DATA_SET(output->data,"payload_id", GINT_TO_POINTER(REQUEST_UPDATE_BLOCK_IN_RAM));
	DATA_SET(output->data,"offset", GINT_TO_POINTER(offset));
	DATA_SET(output->data,"length", GINT_TO_POINTER(num_bytes));
	DATA_SET_FULL(output->data,"data", (gpointer)block, g_free);
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CHUNK_WRITE));

	libreems_store_new_block(canID,locID,offset,block,num_bytes);
	output->queue_update = TRUE;
	io_cmd_f(firmware->write_command,output);
	/*
	DATA_SET(global_data,"last_page",GINT_TO_POINTER(page));
	*/
	EXIT();
	return;
}


/*!
 \brief update_write_status() checks the differences between the current ECU
 data snapshot and the last one, if there are any differences (things need to
 be burnt) then it turns all the widgets in the "burners" group to RED
 \param data is a pointer to message sent to ECU used to
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
	guint8 *block = NULL;
	WriteMode mode = MTX_CMD_WRITE;
	gint z = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;
	ecu_data_last = firmware->ecu_data_last;

//	printf("update write status\n");
	if (!output)
		goto red_or_black;
	else
	{
		gint locID = 0;
		canID = (GINT)DATA_GET(output->data,"canID");
		locID = (GINT)DATA_GET(output->data,"location_id");
		offset = (GINT)DATA_GET(output->data,"offset");
		length = (GINT)DATA_GET(output->data,"length");
		block = (guint8 *)DATA_GET(output->data,"data");
		mode = (WriteMode)(GINT)DATA_GET(output->data,"mode");
		libreems_find_mtx_page(locID,&page);

//		printf("locID %i, page %i\n",locID,page);
		if (!message->status) /* Bad write! */
		{
			MTXDBG(CRITICAL|SERIAL_WR,_("WRITE failed, rolling back!\n"));
			memcpy(ecu_data[page]+offset, ecu_data_last[page]+offset,length);
		}
	}

	if (output->queue_update)
	{
//		printf("queu update\n");
		if ((GINT)DATA_GET(global_data,"mtx_color_scale") == AUTO_COLOR_SCALE)
		{	
//			printf("auto-scale\n");
			for (gint i=0;i<firmware->total_tables;i++)
			{
//				printf("checking table %i\n",i);
				if (firmware->table_params[i]->z_page == page)
				{
//					printf("Found matching table %i at page %i\n",i,page);
					recalc_table_limits_f(canID,i);
					if ((firmware->table_params[i]->last_z_maxval != firmware->table_params[i]->z_maxval) || (firmware->table_params[i]->last_z_minval != firmware->table_params[i]->z_minval))
					{
//						printf("Table limits for table %i have changed\n",i);
						tmpbuf = g_strdup_printf("table%i_color_id",i);
						if (!DATA_GET(global_data,tmpbuf))
						{
//							printf("Creating deferred function\n");

							guint id = g_timeout_add(2000,(GSourceFunc)table_color_refresh_wrapper_f,GINT_TO_POINTER(i));
							DATA_SET(global_data,tmpbuf,GINT_TO_POINTER(id));
						}
						g_free(tmpbuf);
					}
				}
			}
		}
//		printf("Refreshing widgets at page %i, offset %i, length %i\n",page,offset,length);
		thread_refresh_widget_range_f(page,offset,length);

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
	for (gint i=0;i<firmware->total_pages;i++)
	{
		if (firmware->page_params[i]->read_only)
			continue;

		if(memcmp(ecu_data_last[i],ecu_data[i],firmware->page_params[i]->length) != 0)
		{
			firmware->page_params[i]->needs_burn = TRUE;
			thread_set_group_color_f(RED,"burners");
/*			thread_slaves_set_color(RED,"burners");*/
			EXIT();
			return;
		}
		else
			firmware->page_params[i]->needs_burn = FALSE;
	}
	thread_set_group_color_f(BLACK,"burners");
/*	thread_slaves_set_color(BLACK,"burners");*/
	EXIT();
	return;
}


/*!
  \brief A per ECU family function to be run after a single page burn.  It's
  purpose being to backup the in memory representation of the ECU
  \param data is the pointer to the Io_Message structure used for the 
  burn request
  */
G_MODULE_EXPORT void post_single_burn_pf(void *data)
{
	Io_Message *message = (Io_Message *)data;
	OutputData *output = (OutputData *)message->payload;
	Firmware_Details *firmware = NULL;
	gint locID = 0;
	gint page = 0;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	locID = (GINT)DATA_GET(output->data,"location_id");
	page = (GINT)DATA_GET(output->data,"page");

	/* sync temp buffer with current burned settings */
	if (firmware->page_params[page]->read_only)
	{
		EXIT();
		return;
	}
	libreems_backup_current_data(firmware->canID,locID);

	MTXDBG(SERIAL_WR,_("Burn to Flash Completed\n"));

	EXIT();
	return;
}
