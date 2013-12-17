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
  \file src/plugins/mscommon/mtxsocket.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS personality specific remote network mode functionality
  \author David Andruczyk
  */


/* Current GSocket Implementation */
#include <api-versions.h>
#include <args.h>
#include <config.h>
#include <configfile.h>
#include <datamgmt.h>
#include <defines.h>
#include <enums.h>
#include <errno.h>
#include <fcntl.h>
#include <firmware.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <init.h>
#include <mscommon_comms.h>
#include <mscommon_plugin.h>
#include <mtxsocket.h>
#include <rtv_map_loader.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __WIN32__
#include <poll.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif
#ifdef __WIN32__
#include <winsock2.h>
#endif
#include <unistd.h>

#define ERR_MSG "Bad Request: "

static GPtrArray *slave_list = NULL;
static const guint8 SLAVE_MEMORY_UPDATE=0xBE;
static const guint8 SLAVE_STATUS_UPDATE=0xBF;
GThread *ascii_socket_id = NULL;
GThread *binary_socket_id = NULL;
GThread *control_socket_id = NULL;
GThread *notify_slaves_id = NULL;
extern gconstpointer *global_data;

/*!
 *\brief open_tcpip_sockets opens up the TCP sockets to handle the connection
 of slave isntances, Opens 3 ports, one for ASCII, one for binary, and the
 third is the callback socket that clients connect to to get the changes
 fed back made by the master or other slave instances.
 */
G_MODULE_EXPORT void open_tcpip_sockets(void)
{
	MtxSocket *mtxsock = NULL;
	gboolean fail1,fail2,fail3;

	ENTER();
	/* Open The three sockets,  ASCII interface, binary interface
	 * and control socket for telling other instances to update 
	 * stuff..
	 */
	mtxsock = g_new0(MtxSocket,1);
	mtxsock->socket = setup_socket(MTX_SOCKET_ASCII_PORT);
	mtxsock->fd = g_socket_get_fd(mtxsock->socket);
	mtxsock->type = MTX_SOCKET_ASCII;
	if (mtxsock->fd)
	{
		ascii_socket_id = g_thread_new("ASCII TCP socketthread",socket_thread_manager,
				(gpointer)mtxsock); /* Thread args */
		DATA_SET(global_data,"ascii_socket",mtxsock);
		fail1 = FALSE;
	}
	else
	{
		fail1 = TRUE;
		g_free(mtxsock);
		MTXDBG(CRITICAL,_("ERROR setting up ASCII TCP socket\n"));
	}

	mtxsock = g_new0(MtxSocket,1);
	mtxsock->socket = setup_socket(MTX_SOCKET_BINARY_PORT);
	mtxsock->fd = g_socket_get_fd(mtxsock->socket);
	mtxsock->type = MTX_SOCKET_BINARY;
	if (mtxsock->fd)
	{
		binary_socket_id = g_thread_new("Binary TCP Socket Thread",
				socket_thread_manager,
				(gpointer)mtxsock); /* Thread args */
		DATA_SET(global_data,"binary_socket",mtxsock);
		fail2 = FALSE;
	}
	else
	{
		fail2 = TRUE;
		g_free(mtxsock);
		MTXDBG(CRITICAL,_("ERROR setting up BINARY TCP socket\n"));
	}

	mtxsock = g_new0(MtxSocket,1);
	mtxsock->socket = setup_socket(MTX_SOCKET_CONTROL_PORT);
	mtxsock->fd = g_socket_get_fd(mtxsock->socket);
	mtxsock->type = MTX_SOCKET_CONTROL;
	if (mtxsock->fd)
	{
		control_socket_id = g_thread_new("Binary TCP Control Socket Thread",
				socket_thread_manager,
				(gpointer)mtxsock); /* Thread args */
		DATA_SET(global_data,"control_socket",mtxsock);
		fail3 = FALSE;
	}
	else
	{
		fail3 = TRUE;
		g_free(mtxsock);
		MTXDBG(CRITICAL,_("ERROR setting up TCP control socket\n"));
	}

	if ((!fail1) && (!fail2) &&(!fail3))
	{
		notify_slaves_id = g_thread_new("Slave Notifier Thread",
				notify_slaves_thread,
				NULL); /* Thread args */
		DATA_SET(global_data,"notify_slaves_id",GINT_TO_POINTER(notify_slaves_id));
	}
	EXIT();
	return;
}


/*!
 \brief Sets up incoming sockets (master mode only)
 \param port if the port number to setup
 \returns a pointer to a GSocket structure
 */
G_MODULE_EXPORT GSocket *setup_socket(gint port)
{
	GSocket *sock = NULL;
	GError *error = NULL;
	GInetAddress *inetaddr = NULL;
	GSocketAddress *sockaddr = NULL;

	ENTER();
	sock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, &error);
	if (!sock)
	{
		MTXDBG(CRITICAL|SERIAL_RD|SERIAL_WR,_("Socket creation error: %s\n"),error->message);
		g_error_free(error);
		error = NULL;
		EXIT();
		return NULL;

	}
	g_socket_set_blocking(sock,TRUE);
	inetaddr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
	sockaddr = g_inet_socket_address_new(inetaddr,port);
	if(!g_socket_bind(sock,sockaddr,TRUE,&error))
	{
		MTXDBG(CRITICAL|SERIAL_RD|SERIAL_WR,_("Socket bind error: %s\n"),error->message);
		g_error_free(error);
		error = NULL;
		g_socket_close(sock,NULL);
		EXIT();
		return NULL;
	}
	g_object_unref(sockaddr);
	g_object_unref(inetaddr);

	/* Max 5 clients outstanding */
	g_socket_set_listen_backlog (sock,5);
	if (!g_socket_listen(sock,&error))
	{
		MTXDBG(CRITICAL|SERIAL_RD|SERIAL_WR,_("Socket listen error: %s\n"),error->message);
		g_error_free(error);
		error = NULL;
		g_socket_close(sock,NULL);
		EXIT();
		return NULL;
	}

	/*	printf("\nTCP/IP Socket ready: %s:%d\n\n",inet_ntoa(server_address.sin_addr),ntohs(server_address.sin_port));*/
	EXIT();
	return (sock);
}


/*!
 \brief socket_thread_manager()'s sole purpose in life is to wait for socket
 connections and spawn threads to handle their I/O.  These sockets are for
 remote megatunix management (logging, dashboards, and other cool things)
 \param data is a pointer to the socket descriptor for the open TCP socket.
 \see MtxSocket
 \see MtxSocketClient
 **/
G_MODULE_EXPORT void *socket_thread_manager(gpointer data)
{
	GtkWidget *widget = NULL;
	gchar * tmpbuf = NULL;
	fd_set rd;
	gint res = 0;
	gint i = 0;
	GSocket *socket = NULL;
	GError *error = NULL;
	MtxSocket *mtxsock = (MtxSocket *)data;
	MtxSocketClient * cli_data = NULL;
	GTimeVal cur;
	GSocketAddress *sockaddr = NULL;
	static MtxSocketClient *last_bin_client = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	MTXDBG(MTXSOCKET|THREADS|CRITICAL,_("Thread created!\n"));

	while (TRUE)
	{
		if (DATA_GET(global_data,"leaving")) /* MTX shutting down */
		{
			g_socket_close(mtxsock->socket,NULL);
			g_free(mtxsock);
			g_thread_exit(0);
		}
		if (!(GBOOLEAN)DATA_GET(global_data,"network_access"))
		{
			g_socket_close(mtxsock->socket,NULL);
			g_free(mtxsock);
			g_thread_exit(0);
		}

		cur.tv_sec = 1;
		cur.tv_usec = 0;
		FD_ZERO(&rd);
		FD_SET(mtxsock->fd,&rd);
		res = select(mtxsock->fd+1,&rd,NULL,NULL,(struct timeval *)&cur);
		if (res < 0) /* Error, FD closed, abort */
		{
			/*printf("poll error, closing socket!\n");*/
			g_socket_close(mtxsock->socket,NULL);
			g_free(mtxsock);
			g_thread_exit(0);
		}
		if (res == 0) /* Timeout, loop around */
		{
			/*printf("poll timeout, looping...\n");*/
			continue;
		}
		socket = g_socket_accept(mtxsock->socket,NULL,&error);
		if (((mtxsock->type == MTX_SOCKET_ASCII) || (mtxsock->type == MTX_SOCKET_BINARY)) && (firmware))
		{
			sockaddr = g_socket_get_remote_address(socket,NULL);

			cli_data = g_new0(MtxSocketClient, 1);
			cli_data->ip = g_inet_address_to_string(g_inet_socket_address_get_address((GInetSocketAddress *)sockaddr));
			cli_data->socket = socket;
			cli_data->fd = g_socket_get_fd(socket);
			cli_data->type = mtxsock->type;
			cli_data->ecu_data = g_new0(guint8 *, firmware->total_pages);
			g_object_unref(sockaddr);
			/*printf ("created slave %p, ecu_data %p\n",cli_data,cli_data->ecu_data);*/
			for (i=0;i<firmware->total_pages;i++)
			{
				cli_data->ecu_data[i] = g_new0(guint8, firmware->page_params[i]->length);
				/*printf ("created slave %p, ecu_data[%i] %p\n",cli_data,i,cli_data->ecu_data[i]);*/
				if (firmware->ecu_data[i])
					memcpy (cli_data->ecu_data[i],firmware->ecu_data[i],firmware->page_params[i]->length);

			}
		}

		if (mtxsock->type == MTX_SOCKET_ASCII)
		{
			g_thread_new("ASCII Socket Server",
					ascii_socket_server,
					cli_data); /* Thread args */
		}
		if (mtxsock->type == MTX_SOCKET_BINARY)
		{
			last_bin_client = cli_data;
			g_thread_new("BINARY Socket Server",
					binary_socket_server,
					cli_data); /* Thread args */
		}
		if (mtxsock->type == MTX_SOCKET_CONTROL)
		{
			/*			printf("Connected slave pointer is %p\n",last_bin_client);*/
			if (!slave_list)
				slave_list = g_ptr_array_new();
			last_bin_client->control_socket = socket;
			last_bin_client->container = (gpointer)slave_list;
			g_ptr_array_add(slave_list,last_bin_client);
			last_bin_client = NULL; /* to prevent adding it to multiple clients by mistake. The next binary client will regenerated it */
			widget = lookup_widget_f("connected_clients_entry");
			tmpbuf = g_strdup_printf("%i",slave_list->len);
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);

		}
	}
	EXIT();
	return NULL;
}


/*!
 \brief ascii_socket_server, answers simple requests for data on the
 ASCII communications port 12764 (for telnet/other simple low speed apps)
 \param data gpointer representation of the socket filedescriptor
 */
G_MODULE_EXPORT void *ascii_socket_server(gpointer data)
{
	MtxSocketClient *client = (MtxSocketClient *) data;
	GTimeVal cur;
	gint fd = client->fd;
	gchar buf[4096];
	gchar * cbuf = NULL;  /* Client buffer */
	gchar * tmpbuf = NULL;
	fd_set rd;
	gint res = 0;
	GError *error = NULL;

	ENTER();
	MTXDBG(THREADS|CRITICAL,_("Thread created!\n"));

	tmpbuf = g_strdup_printf(_("Welcome to MegaTunix %s, ASCII mode enabled\nEnter 'help' for assistance\n"),VERSION);
	net_send(client->socket,(guint8 *)tmpbuf,strlen(tmpbuf));
	g_free(tmpbuf);

	while (TRUE)
	{
		if (DATA_GET(global_data,"leaving"))
			goto close_ascii;
		if (!(GBOOLEAN)DATA_GET(global_data,"network_access"))
			goto close_ascii;

		FD_ZERO(&rd);
		FD_SET(fd,&rd);
		cur.tv_sec = 1;
		cur.tv_usec = 0;
		res = select(fd+1,&rd,NULL,NULL,(struct timeval *)&cur);
		if (res < 0) /* Error, socket closed, abort */
		{
close_ascii:
			g_socket_close(client->socket,NULL);
			g_object_unref(client->socket);
			dealloc_client_data(client);
			g_thread_exit(0);
		}
		if (res > 0) /* Data Arrived */
		{
			res = g_socket_receive(client->socket,buf,4096,NULL,&error);
			if (res < 0)
			{

				MTXDBG(THREADS|CRITICAL,_("Receive error \"%s\"\n"),error->message);
				g_error_free(error);
				error = NULL;
				g_socket_close(client->socket,NULL);
				g_object_unref(client->socket);
				dealloc_client_data(client);
				g_thread_exit(0);
			}
			/* If command validator returns false, the connection
			 * was quit, thus close and exit nicely
			 */
			cbuf = g_new0(gchar, 4096);
			memcpy (cbuf,buf,res);
			if (client->type == MTX_SOCKET_ASCII)
				res = validate_remote_ascii_cmd(client,cbuf,res);
			g_free(cbuf);
			if (!res)
			{
				g_socket_close(client->socket,NULL);
				g_object_unref(client->socket);
				dealloc_client_data(client);
				g_thread_exit(0);
			}
		}
	}
	EXIT();
	return NULL;
}


/*!
  \brief This thread handles the binary socket port (port 12765), and 
  acts as a MegaSquirt state machine, thus a remote client uses the same
  API as if it was talking to a MS on a serial port.  It emulates most of the
  MS features, thus allowing non-megatunix clients, however MS doesn't have any
  sort of notification of changes, which is why another port (12766) is used
  to pass messages from the MASTER (the one connected directly to the MS via
  serial) to all connected slaves to keep them in sync.
  \param data is a pointer to the MtxSocketClient structure
  \see MtxSocketClient
  */
G_MODULE_EXPORT void *binary_socket_server(gpointer data)
{
	GtkWidget *widget = NULL;
	MtxSocketClient *client = (MtxSocketClient *) data;
	gchar buf;
	gchar *tmpbuf = NULL;
	gint res = 0;
	gint canID = 0;
	gint tableID = 0;
	gint mtx_page = 0;
	gint offset = 0;
	guint8 offset_h = 0;
	guint8 offset_l = 0;
	gint count = 0;
	guint8 count_h = 0;
	guint8 count_l = 0;
	gint index = 0;
	guint8 *buffer = NULL;
	guint8 byte = 0;
	gfloat tmpf = 0.0;
	gint tmpi = 0;
	OutputData *output = NULL;
	State state;
	State next_state;
	State substate;
	GError *error = NULL;
	Firmware_Details *firmware;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	state = WAITING_FOR_CMD;
	next_state = WAITING_FOR_CMD;
	substate = UNDEFINED_SUBSTATE;

	MTXDBG(THREADS|CRITICAL,_("Thread created!\n"));
	while(TRUE)
	{
		/* Condition handling */
		if (DATA_GET(global_data,"leaving"))
			goto close_binary;
		if (!(GBOOLEAN)DATA_GET(global_data,"network_access"))
			goto close_binary;

		if (res < 0)
			goto close_binary2;
		res = g_socket_receive(client->socket,&buf,1,NULL,&error);
		if (res < 0)
		{
close_binary:
			if (error)
			{
				MTXDBG(THREADS|CRITICAL,_("Receive error \"%s\"\n"),error->message);
				g_error_free(error);
			}
			error = NULL;
close_binary2:
			g_socket_close(client->socket,NULL);
			g_object_unref(client->socket);

			if (slave_list)
			{
				g_ptr_array_remove(slave_list,client);
				dealloc_client_data(client);
				client = NULL;
				widget = lookup_widget_f("connected_clients_entry");
				tmpbuf = g_strdup_printf("%i",slave_list->len);
				gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
				g_free(tmpbuf);
			}
			g_thread_exit(0);
		}
		/* State machine... */
		switch (state)
		{
			case WAITING_FOR_CMD:
				switch (buf)
				{
					case '!': /* Potential reinit/reboot */
						if ((firmware->capabilities & MS2) || (firmware->capabilities & MS1_E))
							state = GET_REINIT_OR_REBOOT;
						continue;
					case 'a':/* MS2 table full table read */
						if (firmware->capabilities & MS2)
						{
							/*printf("'a' received\n");*/
							state = GET_CAN_ID;
							next_state = WAITING_FOR_CMD;
							substate = SEND_FULL_TABLE;
						}
						continue;;
					case 't':/* MS2 Lookuptable update */
						/*printf("'t' received\n");*/

						if (firmware->capabilities & MS2)
						{
							state = GET_TABLE_ID;
							next_state = GET_DATABLOCK;
							substate = RECV_LOOKUPTABLE;
						}
						continue;
					case 'A':/* MS1 RTvars */
						/*						printf("'A' received\n");*/
						if (firmware->capabilities & MS1_E)
							res = net_send (client->socket,(guint8 *)firmware->rt_data,22);
						else 
							res = net_send (client->socket,(guint8 *)firmware->rt_data,firmware->rtvars_size);
						/*						printf("MS1 rtvars sent, %i bytes delivered\n",res);*/
						continue;
					case 'b':/* MS2 burn */
						if (firmware->capabilities & MS2)
						{
							/*							printf("'b' received\n");*/
							state = GET_CAN_ID;
							next_state = WAITING_FOR_CMD;
							substate = BURN_MS2_FLASH;
						}
						continue;
					case 'B':/* MS1 burn */
						/*						printf("'B' received\n");*/
						if (firmware->capabilities & MS1)
							io_cmd_f(firmware->burn_all_command,NULL);
						continue;
					case 'r':/* MS2 partial table read */
						if (firmware->capabilities & MS2)
						{
							/*							printf("'r' received\n");*/
							state = GET_CAN_ID;
							next_state = GET_HIGH_OFFSET;
							substate = SEND_PARTIAL_TABLE;
						}
						continue;
					case 'w':/* MS2 chunk write */
						if (firmware->capabilities & MS2)
						{
							/*							printf("'w' received\n");*/
							state = GET_CAN_ID;
							next_state = GET_HIGH_OFFSET;
							substate = GET_VAR_DATA;
						}
						continue;
					case 'c':/* MS2 Clock read */
						if (firmware->capabilities & MS2)
						{
							/*							printf("'c' received\n");*/
							state = WAITING_FOR_CMD;
							lookup_current_value_f("raw_secl",&tmpf);
							tmpi = (guint16)tmpf;
							res = net_send(client->socket,(guint8 *)&tmpi,2);
							/*							printf("MS2 clock sent, %i bytes delivered\n",res);*/
						}
						continue;
					case 'C':/* MS1 Clock read */
						if (firmware->capabilities & MS1)
						{
							/*							printf("'C' received\n");*/
							lookup_current_value_f("raw_secl",&tmpf);
							tmpi = (guint8)tmpf;
							res = net_send(client->socket,(guint8 *)&tmpi,1);
							/*							printf("MS1 clock sent, %i bytes delivered\n",res);*/
						}
						continue;
					case 'P':/* MS1 Page change */
						/*						printf ("'P' (MS1 Page change)\n");*/
						if (firmware->capabilities & MS1)
						{
							state = GET_MS1_PAGE;
							next_state = WAITING_FOR_CMD;
						}
						continue;
					case 'Q':/* MS1 Numeric Revision read 
							  * MS2 Text revision, API clash!
							  */ 
						if (!firmware)
							continue;
						else
						{
							/*							printf ("'Q' (MS1 ecu revision, or ms2 text rev)\n");*/
							if ((firmware->capabilities & MS1) && (!(firmware->capabilities & JIMSTIM)))
								res = net_send(client->socket,(guint8 *)&(firmware->ecu_revision),1);
							else
								if (firmware->text_revision)
									res = net_send(client->socket,(guint8 *)firmware->text_revision,firmware->txt_rev_len);
								else
									printf(_("text_revision undefined!\n"));
						}
						/*						printf("numeric/text revision sent, %i bytes delivered\n",res);*/
						continue;
					case 'R':/* MSnS Extra (MS1) RTvars */
						if (firmware->capabilities & MS1_E)
						{
							/*							printf ("'R' (MS1 extra RTvars)\n");*/
							res = net_send (client->socket,(guint8 *)firmware->rt_data,firmware->rtvars_size);
							/*							printf("MSnS-E rtvars, %i bytes delivered\n",res);*/
						}
						continue;
					case 'T':/* MS1 Text Revision */
						if (firmware->capabilities & MS1)
						{
							/*							printf ("'T' (MS1 text revision)\n");*/
							if (firmware->text_revision)
							{
								res = net_send(client->socket,(guint8 *)firmware->text_revision,strlen(firmware->text_revision));
								/*								printf("MS1 textrev, %i bytes delivered\n",res);*/
							}
						}
						continue;
					case 'S':/* MS1/2 Signature Read */
						/*						printf("'S' received\n");*/
						state = WAITING_FOR_CMD;
						if (firmware)
						{
							if (firmware->actual_signature)
								res = net_send(client->socket,(guint8 *)firmware->actual_signature,firmware->signature_len);
							/*printf("MS signature, %i bytes delivered\n",res);*/
						}
						continue;
					case 'V':/* MS1 VE/data read */
						if (firmware->capabilities & MS1)
						{
							/*							printf("'V' received (MS1 VEtable)\n");*/
							if ((GINT)DATA_GET(global_data,"last_page") < 0)
								DATA_SET(global_data,"last_page",GINT_TO_POINTER(0));
							res = net_send (client->socket,(guint8 *)firmware->ecu_data[(GINT)DATA_GET(global_data,"last_page")],firmware->page_params[(GINT)DATA_GET(global_data,"last_page")]->length);
							/*							printf("MS1 VEtable, %i bytes delivered\n",res);*/
						}
						continue;
					case 'W':/* MS1 Simple write */
						if (firmware->capabilities & MS1)
						{
							/*							printf("'W' received (MS1 Write)\n");*/
							state = GET_MS1_OFFSET;
							next_state = GET_MS1_BYTE;
						}
						continue;
					case 'X':/* MS1 Chunk write */
						if (firmware->capabilities & MS1)
						{
							/*							printf("'X' received (MS1 Chunk Write)\n");*/
							state = GET_MS1_OFFSET;
							next_state = GET_MS1_COUNT;
						}
						continue;

					default:
						continue;
				}
			case GET_REINIT_OR_REBOOT:
				if ((buf == '!') && (firmware->capabilities & MS2))
					state = GET_MS2_REBOOT;
				else if ((buf == '!') && (firmware->capabilities & MS1_E))
					state = GET_MS1_EXTRA_REBOOT;
				if (buf == 'x')
				{
					io_cmd_f("ms2_reinit",NULL);
					state = WAITING_FOR_CMD;
				}
				continue;
			case GET_MS1_EXTRA_REBOOT:
				if (buf == 'X')
				{
					io_cmd_f("ms1_extra_reboot_get_error",NULL);
					state = WAITING_FOR_CMD;
				}
				continue;
			case GET_MS2_REBOOT:
				if (buf == 'x')
				{
					io_cmd_f("ms2_reboot",NULL);
					state = WAITING_FOR_CMD;
				}
				continue;
			case GET_CAN_ID:
				/*				printf("get_can_id block\n");*/
				canID = (guint8)buf;
				/*				printf("canID received is %i\n",canID);*/
				if ((canID < 0) || (canID > 8))
				{
					/*					printf( "canID is out of range!\n");*/
					state = WAITING_FOR_CMD;
				}
				else
					state = GET_TABLE_ID;
				continue;
			case GET_TABLE_ID:
				/*				printf("get_table_id block\n"); */
				tableID = (guint8)buf;
				/*				printf("tableID received is %i\n",tableID); */
				if (tableID > firmware->total_tables)
				{
					state = WAITING_FOR_CMD;
					break;
				}
				state = next_state;
				if (substate == SEND_FULL_TABLE)
				{
					if (ms_find_mtx_page(tableID,&mtx_page))
					{
						if (firmware->ecu_data[mtx_page])
						{
							res = net_send(client->socket,(guint8 *)firmware->ecu_data[mtx_page],firmware->page_params[mtx_page]->length);
							/*							printf("Full table sent, %i bytes\n",res);*/
						}
					}
				}
				else if (substate == RECV_LOOKUPTABLE)
				{
					/*					printf("lookuptable tableID\n");*/
					if ((tableID < 0) || (tableID > 3))	/* Limit check */
						break;
					if (tableID == 2) /* EGO is 1024 bytes, others are 2048 */
						count = 1024;
					else
						count = 2048;
					buffer = g_new0(guint8, count);
					index = 0;
					/*					printf("Count to be received is %i\n",count);*/
				}
				else if (substate == BURN_MS2_FLASH)
				{
					if (ms_find_mtx_page(tableID,&mtx_page))
					{
						/*						printf("MS2 burn: Can ID is %i, tableID %i mtx_page %i\n",canID,tableID,mtx_page);*/
						output = initialize_outputdata_f();
						DATA_SET(output->data,"page",GINT_TO_POINTER(mtx_page));
						DATA_SET(output->data,"phys_ecu_page",GINT_TO_POINTER(tableID));
						DATA_SET(output->data,"canID",GINT_TO_POINTER(canID));
						DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
						io_cmd_f(firmware->burn_command,output);
					}

				}
				else 
					next_state = WAITING_FOR_CMD;
				continue;

			case GET_HIGH_OFFSET:
				/*				printf("get_high_offset block\n");*/
				offset_h = (guint8)buf;
				/*				printf("high offset received is %i\n",offset_h);*/
				state = GET_LOW_OFFSET;
				continue;
			case GET_LOW_OFFSET:
				/*				printf("get_low_offset block\n");*/
				offset_l = (guint8)buf;
				/*				printf("low offset received is %i\n",offset_l);*/
				offset = offset_l + (offset_h << 8);
				state = GET_HIGH_COUNT;
				continue;
			case GET_HIGH_COUNT:
				/*				printf("get_high_count block\n");*/
				count_h = (guint8)buf;
				/*				printf("high count received is %i\n",count_h);*/
				state = GET_LOW_COUNT;
				continue;
			case GET_LOW_COUNT:
				/*				printf("get_low_count block\n");*/
				count_l = (guint8)buf;
				/*				printf("low count received is %i\n",count_l);*/
				count = count_l + (count_h << 8);
				state = next_state;
				if (substate == GET_VAR_DATA)
				{
					state = GET_DATABLOCK;
					buffer = g_new0(guint8, count);
					index = 0;
				}

				if (substate == SEND_PARTIAL_TABLE)
				{
					if (ms_find_mtx_page(tableID,&mtx_page))
					{
						if (firmware->ecu_data[mtx_page])
							res = net_send(client->socket,(guint8 *)firmware->ecu_data[mtx_page]+offset,count);
						/*						printf("MS2 partial table, %i bytes delivered\n",res);*/
					}
				}
				continue;
			case GET_DATABLOCK:
				/*				printf("get_datablock\n");*/
				buffer[index] = (guint8)buf;
				index++;
				/*				printf ("Datablock index %i of %i\n",index,count);*/
				if (index >= count)
				{
					if (firmware->capabilities & MS2)
					{
						if (substate == RECV_LOOKUPTABLE)
						{
							/*							printf("Received lookuptable from slave, sending to ECU\n");*/
							if (ms_find_mtx_page(tableID,&mtx_page))
								ms_table_write(mtx_page,count,(guint8 *) buffer);
						}
						else
						{
							if (ms_find_mtx_page(tableID,&mtx_page))
							{
								memcpy (client->ecu_data[mtx_page]+offset,buffer,count);
								ms_chunk_write(canID,mtx_page,offset,count,buffer);
							}
						}
					}
					else
					{
						/*printf("updating local ms1 chunk buffer\n");*/
						memcpy (client->ecu_data[(GINT)DATA_GET(global_data,"last_page")]+offset,buffer,count);
						ms_chunk_write(0,(GINT)DATA_GET(global_data,"last_page"),offset,count,buffer);
					}
					state = WAITING_FOR_CMD;
				}
				else
					state = GET_DATABLOCK;
				continue;
			case GET_MS1_PAGE:
				/*				printf("get_ms1_page\n");*/
				tableID = (guint8)buf;
				/*				printf ("Passed page %i\n",tableID);*/
				ms_handle_page_change(tableID,(GINT)DATA_GET(global_data,"last_page"));
				state = WAITING_FOR_CMD;
				continue;
			case GET_MS1_OFFSET:
				/*				printf("get_ms1_offset\n");*/
				offset = (guint8)buf;
				/*				printf ("Passed offset %i\n",offset);*/
				state = next_state;
				continue;
			case GET_MS1_COUNT:
				/*				printf("get_ms1_count\n");*/
				count = (guint8)buf;
				index = 0;
				buffer = g_new0(guint8, count);
				/*				printf ("Passed count %i\n",count);*/
				state = GET_DATABLOCK;
				continue;
			case GET_MS1_BYTE:
				/*				printf("get_ms1_byte\n");*/
				byte = (guint8)buf;
				/*				printf ("Passed byte %i\n",byte);*/
				_set_sized_data_f(client->ecu_data[(GINT)DATA_GET(global_data,"last_page")],offset,MTX_U08,byte,firmware->bigendian);
				ms_send_to_ecu(0,(GINT)DATA_GET(global_data,"last_page"),offset,MTX_U08,byte,TRUE);

				/*				printf("Writing byte %i to ecu on page %i, offset %i\n",byte,(GINT)DATA_GET(global_data,"last_page"),offset);*/
				state = WAITING_FOR_CMD;
				continue;
			default:
				printf("case not handled in state machine, BUG!\n");
				continue;
		}
	}
	EXIT();
	return NULL;
}



/*!
 \brief This function validates incoming commands from the TCP socket 
 thread(s).  Commands need to be comma separated, ASCII text, minimum of two
 arguments (more are allowed)
 \param client is a pointer to an active MtxSocketClient structure
 \param buf is the pointer to the input buffer
 \param len is the length of input buffer
 \returns TRUE on valid command, FALSE otherwise
 */
G_MODULE_EXPORT gboolean validate_remote_ascii_cmd(MtxSocketClient *client, gchar * buf, gint len)
{
	gchar ** vector = NULL;
	gchar * arg2 = NULL;
	gint args = 0;
	gsize res = 0;
	gint cmd = 0;
	gboolean retval = TRUE;
	gboolean send_rescode = TRUE;
	gchar *tmpbuf = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	tmpbuf = g_strchomp(g_strdelimit(g_strndup(buf,len),"\n\r\t",' '));
	if (!tmpbuf)
	{
		EXIT();
		return TRUE;
	}
	vector = g_strsplit(tmpbuf,",",2);
	g_free(tmpbuf);
	args = g_strv_length(vector);
	if (!vector[0])
	{
		g_strfreev(vector);
		EXIT();
		return TRUE;
	}
	tmpbuf = g_ascii_strup(vector[0],-1);
	arg2 = g_strdup(vector[1]);
	g_strfreev(vector);
	cmd = translate_string_f(tmpbuf);
	g_free(tmpbuf);

	switch (cmd)
	{
		case GET_RT_VARS:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_get_rt_vars(client->socket, arg2);
			break;
		case GET_RTV_LIST:
			socket_get_rtv_list(client->socket);
			break;
		case GET_ECU_PAGE:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_get_ecu_page(client,arg2);
			break;
		case GET_ECU_VAR_U08:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_get_ecu_var(client,arg2,MTX_U08);
			break;
		case GET_ECU_VAR_S08:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_get_ecu_var(client,arg2,MTX_S08);
			break;
		case GET_ECU_VAR_U16:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_get_ecu_var(client,arg2,MTX_U16);
			break;
		case GET_ECU_VAR_S16:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_get_ecu_var(client,arg2,MTX_S16);
			break;
		case GET_ECU_VAR_U32:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_get_ecu_var(client,arg2,MTX_U32);
			break;
		case GET_ECU_VAR_S32:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_get_ecu_var(client,arg2,MTX_S32);
			break;
		case SET_ECU_VAR_U08:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_set_ecu_var(client,arg2,MTX_U08);
			break;
		case SET_ECU_VAR_S08:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_set_ecu_var(client,arg2,MTX_S08);
			break;
		case SET_ECU_VAR_U16:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_set_ecu_var(client,arg2,MTX_U16);
			break;
		case SET_ECU_VAR_S16:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_set_ecu_var(client,arg2,MTX_S16);
			break;
		case SET_ECU_VAR_U32:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_set_ecu_var(client,arg2,MTX_U32);
			break;
		case SET_ECU_VAR_S32:
			if  (args != 2) 
				return_socket_error(client->socket);
			else
				socket_set_ecu_var(client,arg2,MTX_S32);
			break;
		case GO_BURN_FLASH:
			io_cmd_f(firmware->burn_all_command,NULL);

			break;
		case GET_SIGNATURE:
			if (!firmware)
				net_send(client->socket,(guint8 *)_("Not Connected yet"),strlen(_("Not Connected yet")));
			else
			{
				if (firmware->actual_signature)
					net_send(client->socket,(guint8 *)firmware->actual_signature,strlen(firmware->actual_signature));
				else
					net_send(client->socket,(guint8 *)_("Offline mode, no signature"),strlen(_("Offline mode, no signature")));
			}
			res = net_send(client->socket,(guint8 *)"\n\r",strlen("\n\r"));
			break;
		case GET_REVISION:
			if (!firmware)
				net_send(client->socket,(guint8 *)_("Not Connected yet"),strlen(_("Not Connected yet")));
			else
			{
				if (firmware->text_revision)
					net_send(client->socket,(guint8 *)firmware->text_revision,strlen(firmware->text_revision));
				else
					net_send(client->socket,(guint8 *)_("Offline mode, no revision"),strlen(_("Offline mode, no revision")));
			}
			res = net_send(client->socket,(guint8 *)"\n\r",strlen("\n\r"));
			break;
		case HELP:
			tmpbuf = g_strdup("\n\
Supported Calls:\n\r\
 help\n\r\
 quit\n\r\
 get_signature <-- Returns ECU Signature\n\r\
 get_revision <-- Returns ECU Textual Revision\n\r\
 get_rtv_list <-- returns runtime variable listing\n\r\
 get_rt_vars,[*|<var1>,<var2>,...] <-- returns values of specified variables\n\r\tor all variables if '*' is specified\n\r\
 get_ecu_var_[u08|s08|u16|s16|u32|s32],<canID>,<page>,<offset> <-- returns the\n\r\tecu variable at the spcified location, if firmware\n\r\tis not CAN capable, use 0 for canID, likewise for non-paged\n\r\tfirmwares use 0 for page...\n\r\
 set_ecu_var_[u08|s08|u16|s16|u32|s32],<canID>,<page>,<offset>,<data> <-- Sets\n\r\tthe ecu variable at the spcified location, if firmware\n\r\tis not CAN capable, use 0 for canID, likewise for non-paged\n\r\tfirmwares use 0 for page...\n\r\
get_ecu_page,<canID>,<page> <-- returns the whole ECU page at canID,page\n\r\
 burn_flash <-- Burns contents of ecu ram for current page to flash\n\r\n\r");
			net_send(client->socket,(guint8 *)tmpbuf,strlen(tmpbuf));
			g_free(tmpbuf);
			send_rescode = TRUE;
			break;
		case QUIT:
			tmpbuf = g_strdup("\rBuh Bye...\n\r");
			net_send(client->socket,(guint8 *)tmpbuf,strlen(tmpbuf));
			g_free(tmpbuf);
			retval = FALSE;
			send_rescode = FALSE;
			break;
		default:
			return_socket_error(client->socket);
			break;
	}
	/* Send result code*/
	if (send_rescode)
	{
		if (!DATA_GET(global_data,"connected"))
			net_send(client->socket,(guint8 *)"NOT CONNECTED,",strlen("NOT CONNECTED,"));
		if (check_for_changes(client))
			net_send(client->socket,(guint8 *)"ECU_DATA_CHANGED,",strlen("ECU_DATA_CHANGED,"));
		net_send(client->socket,(guint8 *)"OK",strlen("OK"));

		net_send(client->socket,(guint8 *)"\n\r",strlen("\n\r"));
	}
	g_free(arg2);
	EXIT();
	return retval;
}


/*!
  \brief simple wrapper function to spit back a generic error to a client
  \param socket is a pointer to the network socket
  */
G_MODULE_EXPORT void return_socket_error(GSocket *socket)
{
	ENTER();
	net_send(socket,(guint8 *)ERR_MSG,strlen(ERR_MSG));
	net_send(socket,(guint8 *)"\n\r",strlen("\n\r"));
	EXIT();
	return;
}


/*!
  \brief This returns the RTV vars requested by the comma separated string,
  arg2
  \param socket is the pointer to the open GSocket structure
  \param arg2 is a command separated list of internal names of runtime 
  variables the remote end is interested in.
  */
G_MODULE_EXPORT void socket_get_rt_vars(GSocket *socket, gchar *arg2)
{
	gint res = 0;
	gchar **vars = NULL;
	guint i = 0;
	guint j = 0;
	gconstpointer * object = NULL;
	gint tmpi = 0;
	gfloat tmpf = 0.0;
	GString *output;
	Rtv_Map *rtv_map = NULL;

	ENTER();
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	vars = g_strsplit(arg2,",",-1);
	output = g_string_sized_new(8);
	for (i=0;i<g_strv_length(vars);i++)
	{
		if (g_ascii_strcasecmp(vars[i],"*")==0)
		{
			for (j=0;j<rtv_map->rtv_list->len;j++)
			{
				object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list,j);
				lookup_current_value_f((gchar *)DATA_GET(object,"internal_names"),&tmpf);
				lookup_precision_f((gchar *)DATA_GET(object,"internal_names"),&tmpi);
				if (j < (rtv_map->rtv_list->len-1))
					g_string_append_printf(output,"%1$.*2$f ",tmpf,tmpi);
				else
					g_string_append_printf(output,"%1$.*2$f\n\r",tmpf,tmpi);

			}
		}
		else
		{
			lookup_current_value_f(vars[i],&tmpf);
			lookup_precision_f(vars[i],&tmpi);
			if (i < (g_strv_length(vars)-1))
				g_string_append_printf(output,"%1$.*2$f ",tmpf,tmpi);
			else
				g_string_append_printf(output,"%1$.*2$f\n\r",tmpf,tmpi);
		}
	}
	res = net_send(socket,(guint8 *)output->str,output->len);
	g_string_free(output,TRUE);
	g_strfreev(vars);
	EXIT();
	return;
}


/*!
  \brief Echos back to the client a list of all avaialble runtime variables
  \param socket is a pointer to the active GSocket structure
  */
G_MODULE_EXPORT void socket_get_rtv_list(GSocket *socket)
{
	guint i = 0;
	gint res = 0;
	gint len = 0;
	gchar * tmpbuf = NULL;
	gconstpointer * object = NULL;
	Rtv_Map *rtv_map = NULL;

	ENTER();
	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");
	for (i=0;i<rtv_map->rtv_list->len;i++)
	{
		object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list,i);
		if (i < rtv_map->rtv_list->len-1)
			tmpbuf = g_strdup_printf("%s ",(gchar *)DATA_GET(object,"internal_names"));
		else
			tmpbuf = g_strdup_printf("%s",(gchar *)DATA_GET(object,"internal_names"));
		if (tmpbuf)
		{
			len = strlen(tmpbuf);
			res = net_send(socket,(guint8 *)tmpbuf,len);
			if (res != len)
				printf(_("SHORT WRITE!\n"));
			g_free(tmpbuf);
		}
	}
	tmpbuf = g_strdup("\r\n");
	len = strlen(tmpbuf);
	res = net_send(socket,(guint8 *)tmpbuf,len);
	if (len != res)
		printf(_("SHORT WRITE!\n"));
	g_free(tmpbuf);
	EXIT();
	return;
}


/*!
  \brief The client requested an ECU variable, validate its request and return
  the value back to the client
  \param client is a pointer to the active MtxSocketclient structure
  \param arg2 is a comma separated list of coordinates, i.e. canID,page, 
  and offset. All three are required, 
  \param size is the size enumeration of the data to retrieve
  */
G_MODULE_EXPORT void socket_get_ecu_var(MtxSocketClient *client, gchar *arg2, DataSize size)
{
	gchar ** vars = NULL;
	gchar * tmpbuf = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	/* We want canID, page, offset
	 * If firmware in use doesn't have canBUS capability
	 * just use zero for the option, likewise for page
	 */
	vars = g_strsplit(arg2,",",-1);
	if (g_strv_length(vars) != 3)
	{
		return_socket_error(client->socket);
		g_strfreev(vars); 
	}
	else
	{
		gint canID = atoi(vars[0]);
		gint page = atoi(vars[1]);
		gint offset = atoi(vars[2]);
		gint tmpi = ms_get_ecu_data(canID,page,offset,size);
		_set_sized_data_f(client->ecu_data[page],offset,size,tmpi,firmware->bigendian);
		tmpbuf = g_strdup_printf("%i\r\n",tmpi);
		gint len = strlen(tmpbuf);
		gint res = net_send(client->socket,(guint8 *)tmpbuf,len);
		if (len != res)
			printf(_("SHORT WRITE!\n"));
		g_free(tmpbuf);
		g_strfreev(vars); 
	}
	EXIT();
	return;
}


/*!
  \brief The client requested multiple ECU variables, validate its request 
  and return the values back to the client
  \param client is a pointer to the active MtxSocketclient structure
  \param arg2 is a 2 value comma separated list of coordinates, 
  i.e. canID and page, This fucntion returns the WHOLE PAGE
  */
G_MODULE_EXPORT void socket_get_ecu_page(MtxSocketClient *client, gchar *arg2)
{
	gchar ** vars = NULL;
	GString * output = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	/* We want canID, page
	 * If firmware in use doesn't have canBUS capability
	 * just use zero for the option, likewise for page
	 */
	vars = g_strsplit(arg2,",",-1);
	if (g_strv_length(vars) != 2)
	{
		return_socket_error(client->socket);
		g_strfreev(vars); 
	}
	else
	{
		gint canID = atoi(vars[0]);
		gint page = atoi(vars[1]);
		gint len = firmware->page_params[page]->length;
		gint tmpi = 0;
		output = g_string_sized_new(8);
		for (gint i=0;i<len;i++)
		{
			tmpi = ms_get_ecu_data(canID,page,i,MTX_U08);
			_set_sized_data_f(client->ecu_data[page],i,MTX_U08,tmpi,firmware->bigendian);
			if (i < (len -1))
				g_string_append_printf(output,"%i,",tmpi);
			else
				g_string_append_printf(output,"%i\n\r",tmpi);
		}
		g_strfreev(vars); 
		net_send(client->socket,(guint8 *)output->str,output->len);
		g_string_free(output,TRUE);
	}
	EXIT();
	return;
}


/*!
  \brief The slave is updating an ECU location in memory, validate it, update
  which will trigger the master Gui and all slave gui's to update accordingly
  \param client is a pointer to the MtxSocketClient structure
  \param args is the coordinates (4 values required), canID, page, offset,
  and the new data
  \param size is hte size to store. 
  \bug this function works properly only for 8 bit values....
  */
G_MODULE_EXPORT void socket_set_ecu_var(MtxSocketClient *client, gchar *arg2, DataSize size)
{
	gchar ** vars = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	/* We want canID, page, offset, data
	 * If firmware in use doesn't have canBUS capability
	 * just use zero for the option, likewise for page
	 */
	vars = g_strsplit(arg2,",",-1);
	if (g_strv_length(vars) != 4)
	{
		return_socket_error(client->socket);
		g_strfreev(vars); 
	}
	else
	{
		gint canID = atoi(vars[0]);
		gint page = atoi(vars[1]);
		gint offset = atoi(vars[2]);
		gint data = atoi(vars[3]);
		_set_sized_data_f(client->ecu_data[page],offset,size,data,firmware->bigendian);
		ms_send_to_ecu(canID,page,offset,size,data,TRUE);
		g_strfreev(vars); 
	}
	EXIT();
	return;
}


/*!
  \brief compares the content of the master's representation of ECU memory with
  the slave's,  if they match return FALSE, if not, return TRUE
  \param client is a pointer to the active socket client
  */
G_MODULE_EXPORT gboolean check_for_changes(MtxSocketClient *client)
{
	gint i = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!firmware)
	{
		EXIT();
		return FALSE;
	}
	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;

		if (!firmware->ecu_data[i])
			continue;
		if(memcmp(client->ecu_data[i],firmware->ecu_data[i],firmware->page_params[i]->length) != 0)
		{
			EXIT();
			return TRUE;
		}
	}
	EXIT();
	return FALSE;
}


/*!
  \brief this is analagous to the serial_repair_thread and goes through the
  motions of trying to reconnect to the target if the connection is lost
  \param data is unused
  \see serial_repair_thread
  */
G_MODULE_EXPORT void *network_repair_thread(gpointer data)
{
	/* - DEV code for setting up connection to a network socket
	 * in place of a serial port,  useful for chaining instances of
	 * megatunix to a master, allows "group mind" tuning, or a complete
	 * disaster...
	 */
	static gboolean network_is_open = FALSE; /* Assume never opened */
	static GAsyncQueue *io_repair_queue;
	void (*setup_serial_params_f)(void) = NULL;
	volatile gboolean autodetect = TRUE;
	gchar * host = NULL;
	gint port = 0;
	gchar ** vector = NULL;
	CmdLineArgs *args = NULL;

	ENTER();
	get_symbol_f("setup_serial_params",(void **)&setup_serial_params_f);
	args = (CmdLineArgs *)DATA_GET(global_data,"args");
	g_return_val_if_fail(args,NULL);
	g_return_val_if_fail(setup_serial_params_f,NULL);

	MTXDBG(THREADS|CRITICAL,_("Thread created!\n"));
	if (DATA_GET(global_data,"offline"))
	{
		g_timeout_add(100,(gboolean (*)(void*))queue_function_f,(gpointer)"kill_conn_warning");
		g_thread_exit(0);
	}
	if (!io_repair_queue)
		io_repair_queue = (GAsyncQueue *)DATA_GET(global_data,"io_repair_queue");
	/* IF network_is_open is true, then the port was ALREADY opened 
	 * previously but some error occurred that sent us down here. Thus
	 * first do a simple comms test, if that succeeds, then just cleanup 
	 * and return,  if not, close the port and essentially start over.
	 */
	if (network_is_open == TRUE)
	{
		MTXDBG(SERIAL_RD|SERIAL_WR,_("Port considered open, but throwing errors\n"));
		gint i = 0;
		while (i <= 5)
		{
			MTXDBG(SERIAL_RD|SERIAL_WR,_("Calling comms_test, attempt %i\n"),i);
			if (comms_test())
			{
				g_thread_exit(0);
			}
			i++;
		}
		close_network();
		close_control_socket();
		network_is_open = FALSE;
		/* Fall through */
	}
	/* App just started, no connection yet*/
	while (network_is_open == FALSE)              
	{
		/* Message queue used to exit immediately */
		if (g_async_queue_try_pop(io_repair_queue))
		{
			g_timeout_add(100,(gboolean (*)(void*))queue_function_f,(gpointer)"kill_conn_warning");
			g_thread_exit(0);
		}
		autodetect = (GBOOLEAN) DATA_GET(global_data,"autodetect_port");
		if (!autodetect)
		{
			vector = g_strsplit((gchar *)DATA_GET(global_data, "override_port"),":",2);
			if (g_strv_length(vector) == 2)
			{
				host = vector[0];
				port = strtol(vector[1],NULL,10);
			}
			else
			{
				host = args->network_host;
				port = args->network_port;
			}
		}
		else
		{
			host = args->network_host;
			port = args->network_port;
		}
		thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Attempting to open connection to %s:%i\n"),host,port),FALSE,FALSE);
		if (open_network(host,port))
		{
			setup_serial_params_f();

			thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Network Connection established to %s:%i\n"),host,port),FALSE,FALSE);
			if (comms_test())
			{
				network_is_open = TRUE;
				thread_update_logbar_f("comms_view","info",g_strdup_printf(_("Comms Test Success!, Opening Control Socket\n")),FALSE,FALSE);
				open_control_socket(host,MTX_SOCKET_CONTROL_PORT);
				g_free(vector);
				break;
			}
			else
			{
				thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("Comms Test failed, closing port\n")),FALSE,FALSE);
				close_network();
				close_control_socket();
				g_free(vector);
				g_usleep(200000); /* Sleep 200ms */
				continue;
			}
		}
		else
		{
			thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("Failed to open network connection to %s:%i, sleeping...\n"),host,port),FALSE,FALSE);
			g_usleep(500000); /* Sleep 500ms */
		}
	}
	if (network_is_open)
	{
		thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup_printf("%s:%i",host,port));
	}
	g_thread_exit(0);
	EXIT();
	return NULL;
}


/*! 
  \brief Opens a network connection to a remote machine, sets up the global
  Serial_Params structure on success
  \param host is the name of the remote host
  \param port is the port on the target to attempt to open
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean open_network(gchar * host, gint port)
{
	GSocket *clientsocket = NULL;
	gint status = 0;
	GSocketAddress *sockaddr = NULL;
	GError *error = NULL;
	GResolver *resolver = NULL;
	GList *list = NULL;
	Serial_Params *serial_params;
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	ENTER();

	clientsocket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, &error);
	if (!clientsocket)
	{
		MTXDBG(CRITICAL|SERIAL_RD|SERIAL_WR,_("Socket open error: %s\n"),error->message);
		g_error_free(error);
		error = NULL;
		EXIT();
		return FALSE;
	}

	resolver = g_resolver_get_default();
	list = g_resolver_lookup_by_name(resolver,host,NULL,NULL);
	sockaddr = g_inet_socket_address_new((GInetAddress *)g_list_nth_data(list,0),port);
	status = g_socket_connect(clientsocket,sockaddr,NULL,&error);
	if (!status)
	{
		MTXDBG(SERIAL_RD|SERIAL_WR,_("Socket connect error: %s\n"),error->message);
		g_error_free(error);
		error = NULL;
		EXIT();
		return FALSE;
	}
	g_resolver_free_addresses(list);
	g_object_unref(resolver);
	g_object_unref(sockaddr);

	/*	printf("connected!!\n");*/
	serial_params->fd = g_socket_get_fd(clientsocket);
	serial_params->socket = clientsocket;
	serial_params->net_mode = TRUE;
	serial_params->open = TRUE;

	EXIT();
	return TRUE;
}


/*!
  \brief closes the binary socket
  */
G_MODULE_EXPORT gboolean close_network(void)
{
	Serial_Params *serial_params;
	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	/*	printf("Closing network port!\n");*/
	g_socket_shutdown(serial_params->socket,TRUE,TRUE,NULL);
	g_socket_close(serial_params->socket,NULL);
	serial_params->open = FALSE;
	serial_params->fd = -1;
	DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
	EXIT();
	return TRUE;
}


/*!
  \brief closes the control socket
  */
G_MODULE_EXPORT gboolean close_control_socket(void)
{
	Serial_Params *serial_params;
	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	g_socket_shutdown(serial_params->ctrl_socket,TRUE,TRUE,NULL);
	g_socket_close(serial_params->ctrl_socket,NULL);
	serial_params->ctrl_socket = NULL;
	EXIT();
	return TRUE;
}


/*!
  \brief notify_slaves_thread()'s sole purpose in life is to listen for 
  messages on the async queue from the IO core for messages to send to slaves
  and dispatch them out.  It should check if a slave disconected and in that 
  case delete their entry from the slave list.
  \param data (gpointer) unused.
  */
G_MODULE_EXPORT void *notify_slaves_thread(gpointer data)
{
	static GAsyncQueue *slave_msg_queue = NULL;
	static Firmware_Details *firmware = NULL;
	GtkWidget *widget = NULL;
	gchar * tmpbuf = NULL;
	SlaveMessage *msg = NULL;
	MtxSocketClient * cli_data = NULL;
	fd_set wr;
	gboolean *to_be_closed = NULL;
	guint i = 0;
	gint fd = 0;
	gint res = 0;
	gint len = 0;
	guint8 *buffer = NULL;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!slave_msg_queue)
		slave_msg_queue = (GAsyncQueue *)DATA_GET(global_data,"slave_msg_queue");

	g_return_val_if_fail(firmware,NULL);
	g_return_val_if_fail(slave_msg_queue,NULL);
	MTXDBG(THREADS|CRITICAL,_("Thread created!\n"));
	while(TRUE) /* endless loop */
	{
		if ((DATA_GET(global_data,"leaving")) || (!(GBOOLEAN)DATA_GET(global_data,"network_access")))
		{
			/* drain queue and exit thread */
			while (g_async_queue_try_pop(slave_msg_queue) != NULL)
			{}
			/* Deallocate and exit! */
			if (slave_list)
			{
				for (gint i=0;i<slave_list->len;i++)
				{
					cli_data = (MtxSocketClient *)g_ptr_array_index(slave_list,i);
					g_socket_close(cli_data->control_socket,NULL);
					g_ptr_array_remove(slave_list,cli_data);
					dealloc_client_data(cli_data);
				}
			}
			g_thread_exit(0);
		}
		msg = (SlaveMessage *)g_async_queue_timeout_pop(slave_msg_queue,100000);

		if (!slave_list) /* List not created yet.. */
			continue;
		if (!msg) /* Null message)*/
			continue;

		/*printf("There are %i clients in the slave pointer array\n",slave_list->len);
		 */
		to_be_closed = g_new0(gboolean, slave_list->len);
		for (gint i=0;i<slave_list->len;i++)
		{
			cli_data = (MtxSocketClient *)g_ptr_array_index(slave_list,i);
			if ((!cli_data) || (!cli_data->ecu_data[0]))
			{
				to_be_closed[i] = TRUE;
				continue;
			}
			fd = g_socket_get_fd(cli_data->control_socket);

			FD_ZERO(&wr);
			FD_SET(fd,&wr);
			res = select(fd+1,NULL,&wr,NULL,NULL); 
			if (res <= 0)
			{
				/*				printf("Select error!, closing this socket\n");*/
				to_be_closed[i] = TRUE;
				continue;
			}
			/*
			   printf("sending chunk update\n");
			   printf("notify slaves, slave %p, ecu_data %p\n",cli_data,cli_data->ecu_data);
			 */
			/* We need to check if this slave sent the update,  
			 * if so, DO NOT send the same thing back to that 
			 * slave as he already knows....
			 */
			if (msg->type == MTX_DATA_CHANGED)
			{
				if (msg->mode == MTX_SIMPLE_WRITE)
				{
					if (_get_sized_data_f(cli_data->ecu_data[msg->page],msg->offset,MTX_U08,firmware->bigendian) == ms_get_ecu_data(0,msg->page,msg->offset,MTX_U08))
						continue;
				}
				if (msg->mode == MTX_CHUNK_WRITE)
				{
					if (memcmp (cli_data->ecu_data[msg->page]+msg->offset,firmware->ecu_data[msg->page]+msg->offset,msg->length) == 0)
						continue;
				}

				buffer = build_netmsg(SLAVE_MEMORY_UPDATE,msg,&len);
				res = net_send(cli_data->control_socket,(guint8 *)buffer,len);
				if (res == len)
				{
					if (msg->mode == MTX_SIMPLE_WRITE)
						_set_sized_data_f(cli_data->ecu_data[msg->page],msg->offset,msg->size,msg->value,firmware->bigendian);
					else if (msg->mode == MTX_CHUNK_WRITE)
						memcpy (cli_data->ecu_data[msg->page],&msg->value,msg->length);
				}
				else
					printf(_("Peer update WRITE ERROR!\n"));
				g_free(buffer);
			}
			if (msg->type == MTX_STATUS_CHANGED)
			{
				buffer = build_status_update(SLAVE_STATUS_UPDATE,msg,&len);
				res = net_send(cli_data->control_socket,(guint8 *)buffer,len);
				if (res != len)
					printf(_("Peer update WRITE ERROR!\n"));
				g_free(buffer);
			}

			if (res == -1)
				to_be_closed[i] = TRUE;
		}
		for (i=0;i<slave_list->len;i++)
		{
			if (to_be_closed[i])
			{
				cli_data = (MtxSocketClient *)g_ptr_array_index(slave_list,i);

				g_socket_close(cli_data->control_socket,NULL);
				if (slave_list)
				{
					g_ptr_array_remove(slave_list,cli_data);
					dealloc_client_data(cli_data);
					cli_data = NULL;
					res = 0;
					widget = lookup_widget_f("connected_clients_entry");
					tmpbuf = g_strdup_printf("%i",slave_list->len);
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					g_free(tmpbuf);
				}
			}
		}
		g_free(to_be_closed);
		to_be_closed = NULL;
		if (msg->data)
			g_free(msg->data);
		g_free(msg);
		msg = NULL;
	}
	EXIT();
	return NULL;
}


/*!
  \brief handles notofications from the master to the slaves.  The slave
  will connect and basically sit and wait for data to arrive. when changes
  are made on the master or another slave, the master will send out messages
  to the connected slaves letting them know about the change
  \param data is a pointer to the MtxSocketClient structure
  */
G_MODULE_EXPORT void *control_socket_client(gpointer data)
{
	MtxSocketClient *client = (MtxSocketClient *) data;
	guint8 buf;
	gint i = 0;
	gint res = 0;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint id = 0;
	guint8 offset_h = 0;
	guint8 offset_l = 0;
	gint count = 0;
	guint8 count_h = 0;
	guint8 count_l = 0;
	gint index = 0;
	RemoteAction action;
	GuiColor color = BLACK;
	gchar *tmpbuf = NULL;
	gchar *string = NULL;
	guint8 *buffer = NULL;
	State state;
	State substate;
	GError *error = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	state = WAITING_FOR_CMD;
	substate = UNDEFINED_SUBSTATE;
	MTXDBG(MTXSOCKET|THREADS,_("Thread created!\n"));
	while(TRUE)
	{
		if (DATA_GET(global_data,"leaving"))
			goto close_control;
		res = g_socket_receive(client->socket,(gchar *)&buf,1,NULL,&error);
		if (res <= 0)
		{
close_control:
			MTXDBG(MTXSOCKET|THREADS|CRITICAL,_("Receive error: \"%s\"\n"),error->message);
			g_error_free(error);
			error = NULL;

			close_control_socket();
			dealloc_client_data(client);
			g_thread_exit(0);
		}
		MTXDBG(MTXSOCKET,_("controlsocket Data arrived!\n"));
		MTXDBG(MTXSOCKET,_("Integer \"%i\", Hex \"%.2X\"\n"),(GINT)buf,buf); 
		switch (state)
		{
			case WAITING_FOR_CMD:
				if (buf == SLAVE_MEMORY_UPDATE)
				{
					MTXDBG(MTXSOCKET,_("Slave chunk update received\n"));
					state = GET_CAN_ID;
					substate = GET_VAR_DATA;
				}
				else if (buf == SLAVE_STATUS_UPDATE)
				{
					MTXDBG(MTXSOCKET,_("slave status update!\n"));
					state = GET_ACTION;
					/* Put in handlers here to pickup
					   status messages and other stuff
					   from master, i.e. burn notify, 
					   closing, chat, etc
					   */
				}
				continue;
			case GET_ACTION:
				action = (RemoteAction)(guint8)buf;
				MTXDBG(MTXSOCKET,_("Got action message!\n"));
				if (action == GROUP_SET_COLOR)
					state = GET_COLOR;
				else
					state = WAITING_FOR_CMD;
				continue;
			case GET_COLOR:
				MTXDBG(MTXSOCKET,_("got color message\n"));
				color = (GuiColor)buf;
				state = GET_HIGH_COUNT;
				substate = GET_STRING;
				continue;
			case GET_CAN_ID:
				MTXDBG(MTXSOCKET,_("get_canid block\n"));
				canID = (guint8)buf;
				MTXDBG(MTXSOCKET,_("canID received is %i\n"),canID);
				state = GET_MTX_PAGE;
				continue;
			case GET_MTX_PAGE:
				MTXDBG(MTXSOCKET,_("get_mtx_page block\n"));
				page = (guint8)buf;
				MTXDBG(MTXSOCKET,_("page received is %i\n"),page);
				state = GET_HIGH_OFFSET;
				continue;
			case GET_HIGH_OFFSET:
				MTXDBG(MTXSOCKET,_("get_high_offset block\n"));
				offset_h = (guint8)buf;
				MTXDBG(MTXSOCKET,_("high offset received is %i\n"),offset_h);
				state = GET_LOW_OFFSET;
				continue;
			case GET_LOW_OFFSET:
				MTXDBG(MTXSOCKET,_("get_low_offset block\n"));
				offset_l = (guint8)buf;
				MTXDBG(MTXSOCKET,_("low offset received is %i\n"),offset_l);
				offset = offset_l + (offset_h << 8);
				state = GET_HIGH_COUNT;
				continue;
			case GET_HIGH_COUNT:
				MTXDBG(MTXSOCKET,_("get_high_count block\n"));
				count_h = (guint8)buf;
				MTXDBG(MTXSOCKET,_("high count received is %i\n"),count_h);
				state = GET_LOW_COUNT;
				continue;
			case GET_LOW_COUNT:
				MTXDBG(MTXSOCKET,_("get_low_count block\n"));
				count_l = (guint8)buf;
				MTXDBG(MTXSOCKET,_("low count received is %i\n"),count_l);
				count = count_l + (count_h << 8);
				if (substate == GET_VAR_DATA)
				{
					state = GET_DATABLOCK;
					buffer = g_new0(guint8, count);
					index = 0;
				}
				if (substate == GET_STRING)
				{
					state = GET_STRING;
					string = g_new0(gchar, count);
					index = 0;
					substate = SET_COLOR;
				}
				continue;
			case GET_STRING:
				MTXDBG(MTXSOCKET,_("get_string\n"));
				string[index] = (gchar)buf;
				MTXDBG(MTXSOCKET,_("Charactor \"%c\"\n"),(gchar)buf);
				index++;
				if (index >= count)
				{
					state = WAITING_FOR_CMD;
					if (substate == SET_COLOR)
					{
						MTXDBG(MTXSOCKET,_("setting group \"%s\" color \n"),string);
						thread_set_group_color_f(color,string);
					}
					g_free(string);
					index = 0;
				}
				else
					state = GET_STRING;
				continue;
			case GET_DATABLOCK:
				MTXDBG(MTXSOCKET,_("get_datablock\n"));
				buffer[index] = (guint8)buf;
				index++;
				MTXDBG(MTXSOCKET,_("Datablock index %i of %i\n"),index,count);
				if (index >= count)
				{
					MTXDBG(MTXSOCKET,_("Got all needed data, updating gui\n"));
					state = WAITING_FOR_CMD;
					ms_store_new_block(canID,page,offset,buffer,count);
					/* Update gui with changes */
					if ((GINT)DATA_GET(global_data,"mtx_color_scale") == AUTO_COLOR_SCALE)
					{                       
						if (!firmware)
						{
							firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
							g_return_val_if_fail(firmware,NULL);
						}
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
										id = g_timeout_add(200,(GSourceFunc)table_color_refresh_wrapper_f,GINT_TO_POINTER(i));
										DATA_SET(global_data,tmpbuf,GINT_TO_POINTER(id));
									}               
									g_free(tmpbuf); 
								}               
							}               
						}               
					}
					thread_refresh_widget_range_f(page,offset,count);
					g_free(buffer);
					index = 0;
					MTXDBG(MTXSOCKET,_("Gui Update complete\n"));
				}
				else
					state = GET_DATABLOCK;
				continue;
			default:
				MTXDBG(MTXSOCKET|CRITICAL,_("Case not handled, bug in state machine!\n"));
				continue;

		}
	}
	EXIT();
	return NULL;
}


/*!
  \brief Opens the control socket connection to the master instance
  \param host is the hostname to connect to
  \param port is the port number to connect to
  */
G_MODULE_EXPORT gboolean open_control_socket(gchar * host, gint port)
{
	GSocket *clientsocket = NULL;
	gint status = 0;
	GResolver *resolver = NULL;
	GList *list = NULL;
	GSocketAddress *sockaddr = NULL;
	MtxSocketClient * cli_data = NULL;
	GError *error = NULL;
	Serial_Params *serial_params;
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	ENTER();
	/*	printf ("Trying to open network port!\n");*/
	clientsocket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, &error);
	if (!clientsocket)
	{
		MTXDBG(MTXSOCKET|CRITICAL|SERIAL_RD|SERIAL_WR,_("Socket open error: %s\n"),error->message);
		g_error_free(error);
		error = NULL;
		EXIT();
		return FALSE;
	}

	resolver = g_resolver_get_default();
	list = g_resolver_lookup_by_name(resolver,host,NULL,NULL);
	sockaddr = g_inet_socket_address_new((GInetAddress *)g_list_nth_data(list,0),port);
	status = g_socket_connect(clientsocket,sockaddr,NULL,&error);
	if (status == -1)
	{
		MTXDBG(MTXSOCKET|CRITICAL|SERIAL_RD|SERIAL_WR,_("Socket connect error: %s\n"),error->message);
		g_error_free(error);
		error = NULL;
		EXIT();
		return FALSE;
	}
	/*	printf("connected!!\n");*/
	/* Should startup thread now to listen for notification messages
	 */

	serial_params->ctrl_socket = clientsocket;
	cli_data = g_new0(MtxSocketClient, 1);
	cli_data->ip = g_inet_address_to_string(g_inet_socket_address_get_address((GInetSocketAddress *)sockaddr));
	cli_data->socket = clientsocket;
	cli_data->fd = g_socket_get_fd(clientsocket);
	cli_data->type = MTX_SOCKET_CONTROL;

	g_resolver_free_addresses(list);
	g_object_unref(resolver);
	g_object_unref(sockaddr);

	g_thread_new("CONTROL Socket Client",
			control_socket_client,
			cli_data); /* Thread args */
	EXIT();
	return TRUE;
}


/*!
  \brief Wrapper function to deal with sending data to a remote connection
  \param socket is the pointer to the active GSocket
  \param buf is a pointer to the buffer to read from when sending
  \param len is the amount of data to send in bytes
  \returns the numbe of bytes sent
  */
G_MODULE_EXPORT gint net_send(GSocket *socket, guint8 *buf, gint len)
{
	gint total = 0;        /* how many bytes we've sent*/
	gint bytesleft = len; /* how many we have left to senda*/
	gint n = 0;
	GError *error = NULL;

	ENTER();
	if (!buf)
	{
		EXIT();
		return -1;
	}

	while (total < len) 
	{
		n = g_socket_send(socket,(gchar *)buf+total,(gsize)bytesleft,NULL,&error);
		if (n == -1) 
		{ 
			MTXDBG(MTXSOCKET|CRITICAL|SERIAL_WR,_("Socket send error: \"%s\"\n"),error->message);
			g_error_free(error);
			error = NULL;
			EXIT();
			return -1; 
		}
		total += n;
		bytesleft -= n;
	}

	EXIT();
	return total; 
}


/*!
  \brief Assembles a packet destined to the slaves for data changes
  \param update_type is the type of update to the slave
  \param msg is a pointer to a SlaveMessage structure
  \param msg_len is the pointer to a place to store the packet length when
  its done being assembled
  \returns a pointer to the buffer containing this packet
  */
G_MODULE_EXPORT guint8 * build_netmsg(guint8 update_type,SlaveMessage *msg,gint *msg_len)
{
	guint8 *buffer = NULL;
	gint buflen = 0;
	const gint headerlen = 7;

	ENTER();
	buflen = headerlen + msg->length;

	buffer = g_new0(guint8,buflen);	/* 7 byte msg header */
	buffer[0] = update_type;
	buffer[1] = msg->canID;
	buffer[2] = msg->page;
	buffer[3] = (msg->offset >> 8) & 0xff; /* Highbyte of offset */
	buffer[4] = msg->offset & 0xff; /* Highbyte of offset */
	buffer[5] = (msg->length >> 8) & 0xff; /* Highbyte of length */
	buffer[6] = msg->length & 0xff; /* Highbyte of length */
	if (msg->mode == MTX_SIMPLE_WRITE)
		g_memmove(buffer+headerlen,&msg->value,msg->length);
	if (msg->mode == MTX_CHUNK_WRITE)
		g_memmove(buffer+headerlen,msg->data,msg->length);

	*msg_len = buflen;
	EXIT();
	return buffer;
}


/*!
  \brief Assembles a packet destined to the slaves for status updates
  \param update_type is the type of update to the slave
  \param msg is a pointer to a SlaveMessage structure
  \param msg_len is the pointer to a place to store the packet length when
  its done being assembled
  \returns a pointer to the buffer containing this packet
  */
G_MODULE_EXPORT guint8 * build_status_update(guint8 update_type,SlaveMessage *msg,gint *msg_len)
{
	guint8 *buffer = NULL;
	gint buflen = 0;
	const gint headerlen = 5;

	ENTER();
	buflen = headerlen + msg->length;

	buffer = g_new0(guint8,buflen);	
	buffer[0] = update_type;
	buffer[1] = msg->action;
	buffer[2] = (guint8)msg->value;
	buffer[3] = (msg->length >> 8) & 0xff; /* Highbyte of length */
	buffer[4] = msg->length & 0xff; /* Highbyte of length */
	g_memmove(buffer+headerlen, msg->data,msg->length);

	*msg_len = buflen;
	EXIT();
	return buffer;
}


/*!
   \brief dealloc_client_data() deallocates the structure used for MTX TCP/IP
   sockets
   \param client is a pointer to the MtxSocketClient structure to deallocate
   */
G_MODULE_EXPORT void dealloc_client_data(MtxSocketClient *client)
{
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	/*printf("dealloc_client_data\n");*/
	if (client)
	{
		g_free(client->ip);

		if (client->ecu_data)
		{
			for (gint i=0;i<firmware->total_pages;i++)
				g_free (client->ecu_data[i]);
			g_free(client->ecu_data);
		}
		g_free(client);
	}
	EXIT();
	return;
}

