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

#include <api-versions.h>
#include <args.h>
#include <config.h>
#include <configfile.h>
#include <comms.h>
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <fcntl.h>
#include <firmware.h>
#include <gui_handlers.h>
#include <glib.h>
#include <init.h>
#include <mtxsocket.h>
#ifndef __WIN32__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <threads.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <unistd.h>

#define ERR_MSG "Bad Request: "

static GPtrArray *slave_list = NULL;
static gint controlsocket = 0;
static const guint8 SLAVE_MEMORY_UPDATE=0xBE;


/*!
 * \brief Sets up incoming sockets (master mode only)
 */
gboolean setup_socket(gint port)
{
	int sock = 0;
	struct sockaddr_in server_address;
	int reuse_addr = 1;
	int res = 0;
#ifdef __WIN32__
	WSADATA wsaData;
	res = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (res != 0)
	{
		printf("WSAStartup failed: %d\n",res);
		return FALSE;
	}
#endif

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("Socket!");
#ifdef __WIN32__
		WSACleanup();
#endif
		return FALSE;

	}
	/* So that we can re-bind to it without TIME_WAIT problems */
#ifdef __WIN32__
	res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse_addr, sizeof(reuse_addr));
#else
	res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
#endif
	if (res == -1)
		perror("setsockopt(...,SO_REUSEADDR,...)");

	memset((char *) &server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *) &server_address,sizeof(server_address)) < 0 ) {
		perror("bind");
#ifdef __WIN32__
		closesocket(sock);
		WSACleanup();
#else
		close(sock);
#endif
		return FALSE;
	}

	/* Set up queue for incoming connections. */
	if((listen(sock,5))<0)
	{
		perror("listen");
#ifdef __WIN32__
		closesocket(sock);
		WSACleanup();
#else
		close(sock);
#endif
		return FALSE;
	}

	printf("\nTCP/IP Socket ready: %s:%d\n\n",inet_ntoa(server_address.sin_addr),ntohs(server_address.sin_port));
	return sock;
}

/*!
 \brief socket_thread_manager()'s sole purpose in life is to wait for socket
 connections and spawn threads to handle their I/O.  These sockets are for
 remote megatunix management (logging, dashboards, and other cool things)
 \param data (gpointer) socket descriptor for the open TCP socket.
 **/
void *socket_thread_manager(gpointer data)
{
	extern Firmware_Details *firmware;
	gint i = 0;
	MtxSocket *socket = (MtxSocket *)data;
	struct sockaddr_in client;
#ifdef __WIN32__
	int length = sizeof(client);
#else
	socklen_t length = sizeof(client);
#endif
	MtxSocketClient * cli_data = NULL;
	gint fd = 0;

	while (TRUE)
	{
		fd = accept(socket->fd,(struct sockaddr *)&client, &length);
		cli_data = g_new0(MtxSocketClient, 1);
		cli_data->ip = g_strdup(inet_ntoa(client.sin_addr));
		cli_data->port = ntohs(client.sin_port);
		cli_data->fd = fd;
		cli_data->type = socket->type;

		if (((socket->type == MTX_SOCKET_ASCII) || (socket->type == MTX_SOCKET_BINARY)) && (firmware))
		{
			cli_data->ecu_data = g_new0(guint8 *, firmware->total_pages);
			for (i=0;i<firmware->total_pages;i++)
			{
				cli_data->ecu_data[i] = g_new0(guint8, firmware->page_params[i]->length);
				if (firmware->ecu_data[i])
					memcpy (cli_data->ecu_data[i],firmware->ecu_data[i],firmware->page_params[i]->length);

			}
		}


		if (socket->type == MTX_SOCKET_ASCII)
		{
			g_thread_create(ascii_socket_client,
					cli_data, /* Thread args */
					TRUE,   /* Joinable */
					NULL);  /* GError pointer */
		}
		if (socket->type == MTX_SOCKET_BINARY)
		{
			g_thread_create(binary_socket_client,
					cli_data, /* Thread args */
					TRUE,   /* Joinable */
					NULL);  /* GError pointer */
		}
		if (socket->type == MTX_SOCKET_CONTROL)
		{
			printf("cli_data pointer is %p\n",cli_data);
			if (!slave_list)
				slave_list = g_ptr_array_new();
			cli_data->container = (gpointer)slave_list;
			g_ptr_array_add(slave_list,cli_data);


		}
	}
}


/*!
 \brief socket_client is a thread that is spawned for each remote connection
 to megatunix.  It's purpose in life it to answer remote requests.  If the
 remote side is closed down, it sees a zero byte read and exits, killing off
 the thread..
 \param data  gpointer representation of the socket filedescriptor
 */
void *ascii_socket_client(gpointer data)
{
	MtxSocketClient *client = (MtxSocketClient *) data;
	gint fd = client->fd;
	gchar buf[4096];
	gchar * cbuf = NULL;  /* Client buffer */
	gchar * tmpbuf = NULL;
	fd_set rd;
	FD_ZERO(&rd);
	FD_SET(fd,&rd);
	gint res = 0;


	tmpbuf = g_strdup_printf("Welcome to MegaTunix %s, ASCII mode enabled\n\rEnter 'help' for assistance\n\r",VERSION);
	send(fd,tmpbuf,strlen(tmpbuf),0);
	g_free(tmpbuf);

	while (TRUE)
	{
		if (!fd)
		{
			dealloc_client_data(client);
			return(0);
		}
		res = select(fd+1,&rd,NULL,NULL,NULL);
		if (res < 0) /* Error, socket closed, abort */
		{
#ifdef __WIN32__
			closesocket(fd);
#else
			close(fd);
#endif
			dealloc_client_data(client);
			g_thread_exit(0);
		}
		if (res > 0) /* Data Arrived */
		{
			res = recv(fd,(char *)&buf,4096,0);
			if (res <= 0)
			{
#ifdef __WIN32__
				closesocket(fd);
#else
				close(fd);
#endif
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
#ifdef __WIN32__
				closesocket(fd);
#else
				close(fd);
#endif
				dealloc_client_data(client);
				g_thread_exit(0);
			}
		}
	}
}


void *binary_socket_client(gpointer data)
{
	MtxSocketClient *client = (MtxSocketClient *) data;
	gint fd = client->fd;
	gchar buf;
	gint res = 0;
	gint canID = 0;
	gint tableID = 0;
	gint mtx_page = 0;
	gint offset = 0;
	gint offset_h = 0;
	gint offset_l = 0;
	gint count = 0;
	gint count_h = 0;
	gint count_l = 0;
	gint index = 0;
	guint8 *buffer = NULL;
	guint8 byte = 0;
	gfloat tmpf = 0.0;
	gint tmpi = 0;
	OutputData *output = NULL;
	State state = WAITING_FOR_CMD;;
	State next_state = WAITING_FOR_CMD;
	SubState substate = UNDEFINED_SUBSTATE;
	extern Firmware_Details *firmware;
	extern volatile gint last_page;

	while(TRUE)
	{
		res = recv(fd,&buf,1,0);
		if (res <= 0)
		{
#ifdef __WIN32__
			closesocket(fd);
#else
			close(fd);
#endif
			dealloc_client_data(client);
			g_thread_exit(0);
		}
		switch (state)
		{
			case WAITING_FOR_CMD:
				switch (buf)
				{
					case 'a':	/* MS2 table full table read */
						if (firmware->capabilities & MS2)
						{
//							printf("'a' received\n");
							state = GET_CAN_ID;
							next_state = WAITING_FOR_CMD;
							substate = SEND_FULL_TABLE;
						}
						continue;;
					case 'A':	/* MS1 RTvars */
//						printf("'A' received\n");
						if (firmware->capabilities & MSNS_E)
							res = send (fd,(char *)firmware->rt_data,22,0);
						else if (firmware->capabilities & MS1)
							res = send (fd,(char *)firmware->rt_data,firmware->rtvars_size,0);
//						printf("MS1 rtvars sent, %i bytes delivered\n",res);
						continue;
					case 'b':	/* MS2 burn */
						if (firmware->capabilities & MS2)
						{
//							printf("'b' received\n");
							state = GET_CAN_ID;
							next_state = WAITING_FOR_CMD;
							substate = BURN_MS2_FLASH;
						}
						continue;
					case 'B':	/* MS1 burn */
//						printf("'B' received\n");
						if (firmware->capabilities & MS1)
							io_cmd(firmware->burn_all_command,NULL);
						continue;
					case 'r':	/* MS2 partial table read */
						if (firmware->capabilities & MS2)
						{
//							printf("'r' received\n");
							state = GET_CAN_ID;
							next_state = GET_HIGH_OFFSET;
							substate = SEND_PARTIAL_TABLE;
						}
						continue;
					case 'w':	/* MS2 chunk write */
						if (firmware->capabilities & MS2)
						{
//							printf("'w' received\n");
							state = GET_CAN_ID;
							next_state = GET_HIGH_OFFSET;
							substate = GET_VAR_DATA;
						}
						continue;
					case 'c':	/* MS2 Clock read */
						if (firmware->capabilities & MS2)
						{
//							printf("'c' received\n");
							state = WAITING_FOR_CMD;
							lookup_current_value("raw_secl",&tmpf);
							tmpi = (guint16)tmpf;
							res = send(fd,(char *)&tmpi,2,0);
//							printf("MS2 clock sent, %i bytes delivered\n",res);
						}
						continue;
					case 'C': 	/* MS1 Clock read */
						if (firmware->capabilities & MS1)
						{
//							printf("'C' received\n");
							lookup_current_value("raw_secl",&tmpf);
							tmpi = (guint8)tmpf;
							res = send(fd,(char *)&tmpi,1,0);
//							printf("MS1 clock sent, %i bytes delivered\n",res);
						}
						continue;
					case 'P':	/* MS1 Page change */
//						printf ("'P' (MS1 Page change)\n");
						if (firmware->capabilities & MS1)
						{
							state = GET_MS1_PAGE;
							next_state = WAITING_FOR_CMD;
						}
						continue;
					case 'Q':	/* MS1 Numeric Revision read 
							 * MS2 Text revision, API clash!
							 */ 
						if (!firmware)
							continue;
						else
						{
//							printf ("'Q' (MS1 ecu revision, or ms2 text rev)\n");
							if (firmware->capabilities & MS1)
								res = send(fd,(const void *)&(firmware->ecu_revision),1,0);
							else
								if (firmware->text_revision)
									res = send(fd,firmware->text_revision,strlen(firmware->text_revision),0);
						}
//						printf("numeric/text revision sent, %i bytes delivered\n",res);
						continue;
					case 'R':	/* MSnS Extra (MS1) RTvars */
						if (firmware->capabilities & MSNS_E)
						{
//							printf ("'R' (MS1 extra RTvars)\n");
							res = send (fd,(char *)firmware->rt_data,firmware->rtvars_size,0);
//							printf("MSnS-E rtvars, %i bytes delivered\n",res);
						}
						continue;
					case 'T':	/* MS1 Text Revision */
						if (firmware->capabilities & MS1)
						{
//							printf ("'T' (MS1 text revision)\n");
							if (firmware->text_revision)
							{
								res = send(fd,firmware->text_revision,strlen(firmware->text_revision),0);
//								printf("MS1 textrev, %i bytes delivered\n",res);
							}
						}
						continue;
					case 'S':	/* MS1/2 Signature Read */
//						printf("'S' received\n");
						state = WAITING_FOR_CMD;
						if (firmware)
						{
							if (firmware->actual_signature)
								res = send(fd,firmware->actual_signature,strlen(firmware->actual_signature),0);
//							printf("MS signature, %i bytes delivered\n",res);
						}
						continue;
					case 'V':	/* MS1 VE/data read */
						if (firmware->capabilities & MS1)
						{
//							printf("'V' received (MS1 VEtable)\n");
							res = send (fd,(char *)firmware->ecu_data[last_page],firmware->page_params[last_page]->length,0);
//							printf("MS1 VEtable, %i bytes delivered\n",res);
						}
						continue;
					case 'W':	/* MS1 Simple write */
						if (firmware->capabilities & MS1)
						{
//							printf("'W' received (MS1 Write)\n");
							state = GET_MS1_OFFSET;
							next_state = GET_MS1_BYTE;
						}
						continue;
					case 'X':	/* MS1 Chunk write */
						if (firmware->capabilities & MS1)
						{
//							printf("'X' received (MS1 Chunk Write)\n");
							state = GET_MS1_OFFSET;
							next_state = GET_MS1_COUNT;
						}
						continue;

					default:
						continue;
				}
			case GET_CAN_ID:
//				printf("get_can_id block\n");
				canID = (guint8)buf;
//				printf("canID received is %i\n",canID);
				if ((canID < 0) || (canID > 8))
				{
//					printf( "canID is out of range!\n");
					state = WAITING_FOR_CMD;
				}
				else
					state = GET_TABLE_ID;
				continue;
			case GET_TABLE_ID:
//				printf("get_table_id block\n");
				tableID = (guint8)buf;
//				printf("tableID received is %i\n",tableID);
				if (tableID > firmware->total_tables)
				{
					state = WAITING_FOR_CMD;
					break;
				}
				state = next_state;
				if (substate == SEND_FULL_TABLE)
				{
					if (find_mtx_page(tableID,&mtx_page))
					{
						if (firmware->ecu_data[mtx_page])
						{
							res = send(fd,(char *)firmware->ecu_data[mtx_page],firmware->page_params[mtx_page]->length,0);
//							printf("Full table sent, %i bytes\n",res);
						}
					}
				}
				else if (substate == BURN_MS2_FLASH)
				{
					if (find_mtx_page(tableID,&mtx_page))
					{
//						printf("MS2 burn: Can ID is %i, tableID %i mtx_page %i\n",canID,tableID,mtx_page);
						output = initialize_outputdata();
						OBJ_SET(output->object,"page",GINT_TO_POINTER(mtx_page));
						OBJ_SET(output->object,"phys_ecu_page",GINT_TO_POINTER(tableID));
						OBJ_SET(output->object,"canID",GINT_TO_POINTER(canID));
						OBJ_SET(output->object,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
						io_cmd(firmware->burn_command,output);
					}

				}
				else 
					next_state = WAITING_FOR_CMD;
				continue;
			case GET_HIGH_OFFSET:
//				printf("get_high_offset block\n");
				offset_h = (guint8)buf;
//				printf("high offset received is %i\n",offset_h);
				state = GET_LOW_OFFSET;
				continue;
			case GET_LOW_OFFSET:
//				printf("get_low_offset block\n");
				offset_l = (guint8)buf;
//				printf("low offset received is %i\n",offset_l);
				offset = offset_l + (offset_h << 8);
				state = GET_HIGH_COUNT;
				continue;
			case GET_HIGH_COUNT:
//				printf("get_high_count block\n");
				count_h = (guint8)buf;
//				printf("high count received is %i\n",count_h);
				state = GET_LOW_COUNT;
				continue;
			case GET_LOW_COUNT:
//				printf("get_low_count block\n");
				count_l = (guint8)buf;
//				printf("low count received is %i\n",count_l);
				count = count_l + (count_h << 8);
				state = next_state;
				if (substate == GET_VAR_DATA)
				{
					state = GET_DATABYTE;
					buffer = g_new0(guint8, count);
					index = 0;
				}

				if (substate == SEND_PARTIAL_TABLE)
				{
					if (find_mtx_page(tableID,&mtx_page))
					{
						if (firmware->ecu_data[mtx_page])
							res = send(fd,(char *)firmware->ecu_data[mtx_page]+offset,count,0);
//						printf("MS2 partial table, %i bytes delivered\n",res);
					}
				}
				continue;
			case GET_DATABYTE:
//				printf("get_databyte\n");
				buffer[index] = (guint8)buf;
				index++;
//				printf ("Databyte index %i of %i\n",index,count);
				if (index >= count)
				{
					if (firmware->capabilities & MS2)
					{
						if (find_mtx_page(tableID,&mtx_page))
							chunk_write(canID,mtx_page,offset,count,buffer);
					}
					else
						chunk_write(0,last_page,offset,count,buffer);
					state = WAITING_FOR_CMD;
				}
				else
					state = GET_DATABYTE;
				continue;
			case GET_MS1_PAGE:
//				printf("get_ms1_page\n");
				tableID = (guint8)buf;
//				printf ("Passed page %i\n",tableID);
				handle_page_change(tableID,last_page);
				//				queue_ms1_page_change(tableID);
				state = WAITING_FOR_CMD;
				continue;
			case GET_MS1_OFFSET:
//				printf("get_ms1_offset\n");
				offset = (guint8)buf;
//				printf ("Passed offset %i\n",offset);
				state = next_state;
				continue;
			case GET_MS1_COUNT:
//				printf("get_ms1_count\n");
				count = (guint8)buf;
				index = 0;
				buffer = g_new0(guint8, count);
//				printf ("Passed count %i\n",count);
				state = GET_DATABYTE;
				continue;
			case GET_MS1_BYTE:
//				printf("get_ms1_byte\n");
				byte = (guint8)buf;
//				printf ("Passed byte %i\n",byte);
				send_to_ecu(0,last_page,offset,MTX_U08,byte,TRUE);
//				printf("Writing byte %i to ecu on page %i, offset %i\n",byte,last_page,offset);
				state = WAITING_FOR_CMD;
				continue;
			default:
				printf("case not handled in state machine, BUG!\n");
				continue;

		}
	}
}



/*!
 \brief This function validates incoming commands from the TCP socket 
 thread(s).  Commands need to be comma separated, ASCII text, minimum of two
 arguments (more are allowed)
 \param client, MtxSocketClient structure
 \param buf, input buffer
 \param len, length of input buffer
 */
gboolean validate_remote_ascii_cmd(MtxSocketClient *client, gchar * buf, gint len)
{
	gint fd = client->fd;
	extern Firmware_Details *firmware;
	extern gboolean connected;
	gchar ** vector = NULL;
	gchar * arg2 = NULL;
	gint args = 0;
	gsize res = 0;
	gint cmd = 0;
	gboolean retval = TRUE;
	gboolean send_rescode = TRUE;
	gchar *tmpbuf = g_strchomp(g_strdelimit(g_strndup(buf,len),"\n\r\t",' '));
	if (!tmpbuf)
		return TRUE;
	vector = g_strsplit(tmpbuf,",",2);
	g_free(tmpbuf);
	args = g_strv_length(vector);
	if (!vector[0])
	{
		g_strfreev(vector);
		return TRUE;
	}
	tmpbuf = g_ascii_strup(vector[0],-1);
	arg2 = g_strdup(vector[1]);
	g_strfreev(vector);
	cmd = translate_string(tmpbuf);
	g_free(tmpbuf);

	switch (cmd)
	{
		case GET_RT_VARS:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_rt_vars(fd, arg2);
			break;
		case GET_RTV_LIST:
			socket_get_rtv_list(fd);
			break;
		case GET_ECU_VARS:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_vars(client,arg2);
			break;
		case GET_ECU_VAR_U08:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(client,arg2,MTX_U08);
			break;
		case GET_ECU_VAR_S08:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(client,arg2,MTX_S08);
			break;
		case GET_ECU_VAR_U16:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(client,arg2,MTX_U16);
			break;
		case GET_ECU_VAR_S16:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(client,arg2,MTX_S16);
			break;
		case GET_ECU_VAR_U32:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(client,arg2,MTX_U32);
			break;
		case GET_ECU_VAR_S32:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(client,arg2,MTX_S32);
			break;
		case SET_ECU_VAR_U08:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(client,arg2,MTX_U08);
			break;
		case SET_ECU_VAR_S08:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(client,arg2,MTX_S08);
			break;
		case SET_ECU_VAR_U16:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(client,arg2,MTX_U16);
			break;
		case SET_ECU_VAR_S16:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(client,arg2,MTX_S16);
			break;
		case SET_ECU_VAR_U32:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(client,arg2,MTX_U32);
			break;
		case SET_ECU_VAR_S32:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(client,arg2,MTX_S32);
			break;
		case BURN_FLASH:
			io_cmd(firmware->burn_all_command,NULL);

			break;
		case GET_SIGNATURE:
			if (!firmware)
				send(fd,"Not Connected yet",strlen(" Not Connected yet"),0);
			else
			{
				if (firmware->actual_signature)
					send(fd,firmware->actual_signature,strlen(firmware->actual_signature),0);
				else
					send(fd,"Offline mode, no signature",strlen("Offline mode, no signature"),0);
			}
			res = send(fd,"\n\r",strlen("\n\r"),0);
			break;
		case GET_REVISION:
			if (!firmware)
				send(fd,"Not Connected yet",strlen(" Not Connected yet"),0);
			else
			{
				if (firmware->text_revision)
					send(fd,firmware->text_revision,strlen(firmware->text_revision),0);
				else
					send(fd,"Offline mode, no revision",strlen("Offline mode, no revision"),0);
			}
			res = send(fd,"\n\r",strlen("\n\r"),0);
			break;
		case HELP:
			tmpbuf = g_strdup("\
Supported Calls:\n\r\
help\n\r\
quit\n\r\
get_signature <-- Returns ECU Signature\n\r\
get_revision <-- Returns ECU Textual Revision\n\r\
get_rtv_list <-- returns runtime variable listing\n\r\
get_rt_vars,[*|<var1>,<var2>,...] <-- returns values of specified variables\n\r\tor all variables if '*' is specified\n\r\
get_ecu_var[u08|s08|u16|s16|u32|s32],<canID>,<page>,<offset> <-- returns the\n\r\tecu variable at the spcified location, if firmware\n\r\tis not CAN capable, use 0 for canID, likewise for non-paged\n\r\tfirmwares use 0 for page...\n\r\
set_ecu_var[u08|s08|u16|s16|u32|s32],<canID>,<page>,<offset>,<data> <-- Sets\n\r\tthe ecu variable at the spcified location, if firmware\n\r\tis not CAN capable, use 0 for canID, likewise for non-paged\n\r\tfirmwares use 0 for page...\n\r\
burn_flash <-- Burns contents of ecu ram for current page to flash\n\r\n\r");
			send(fd,tmpbuf,strlen(tmpbuf),0);
			g_free(tmpbuf);
			send_rescode = TRUE;
			break;
		case QUIT:
			tmpbuf = g_strdup("\rBuh Bye...\n\r");
			send(fd,tmpbuf,strlen(tmpbuf),0);
			g_free(tmpbuf);
			retval = FALSE;
			send_rescode = FALSE;
			break;
		default:
			return_socket_error(fd);
			break;
	}
	/* Send result code*/
	if (send_rescode)
	{
		if (!connected)
			send(fd,"NOT CONNECTED,",strlen("NOT CONNECTED,"),0);
		if (check_for_changes(client))
			send(fd,"ECU_DATA_CHANGED,",strlen("ECU_DATA_CHANGED,"),0);
		send(fd,"OK",strlen("OK"),0);

		send(fd,"\n\r",strlen("\n\r"),0);
	}
	g_free(arg2);
	return retval;
}


void return_socket_error(gint fd)
{
	send(fd,ERR_MSG,strlen(ERR_MSG),0);
	send(fd,"\n\r",strlen("\n\r"),0);
}


void socket_get_rt_vars(gint fd, gchar *arg2)
{
	gint res = 0;
	gchar **vars = NULL;
	extern Rtv_Map *rtv_map;
	gint i = 0;
	gint j = 0;
	GObject * object = NULL;
	gint tmpi = 0;
	gfloat tmpf = 0.0;
	GString *output;

	vars = g_strsplit(arg2,",",-1);
	output = g_string_sized_new(8);
	for (i=0;i<g_strv_length(vars);i++)
	{
		if (g_strcasecmp(vars[i],"*")==0)
		{
			for (j=0;j<rtv_map->rtv_list->len;j++)
			{
				object = g_array_index(rtv_map->rtv_list,GObject *, j);
				lookup_current_value((gchar *)OBJ_GET(object,"internal_names"),&tmpf);
				lookup_precision((gchar *)OBJ_GET(object,"internal_names"),&tmpi);
				if (j < (rtv_map->rtv_list->len-1))
					g_string_append_printf(output,"%1$.*2$f ",tmpf,tmpi);
				else
					g_string_append_printf(output,"%1$.*2$f\n\r",tmpf,tmpi);

			}
		}
		else
		{
			lookup_current_value(vars[i],&tmpf);
			lookup_precision(vars[i],&tmpi);
			if (i < (g_strv_length(vars)-1))
				g_string_append_printf(output,"%1$.*2$f ",tmpf,tmpi);
			else
				g_string_append_printf(output,"%1$.*2$f\n\r",tmpf,tmpi);
		}
	}
	res = send(fd,output->str,output->len,0);
	g_string_free(output,TRUE);
	g_strfreev(vars);
}


void socket_get_rtv_list(gint fd)
{
	extern Rtv_Map *rtv_map;
	gint i = 0;
	gint res = 0;
	gint len = 0;
	gchar * tmpbuf = NULL;
	GObject * object = NULL;

	for (i=0;i<rtv_map->rtv_list->len;i++)
	{
		object = g_array_index(rtv_map->rtv_list,GObject *, i);
		if (i < rtv_map->rtv_list->len-1)
			tmpbuf = g_strdup_printf("%s ",(gchar *)OBJ_GET(object,"internal_names"));
		else
			tmpbuf = g_strdup_printf("%s",(gchar *)OBJ_GET(object,"internal_names"));
		if (tmpbuf)
		{
			len = strlen(tmpbuf);
			res = send(fd,tmpbuf,len,0);
			if (res != len)
				printf("SHORT WRITE!\n");
			g_free(tmpbuf);
		}
	}
	tmpbuf = g_strdup("\r\n");
	len = strlen(tmpbuf);
	res = send(fd,tmpbuf,len,0);
	if (len != res)
		printf("SHORT WRITE!\n");
	g_free(tmpbuf);
}


void socket_get_ecu_var(MtxSocketClient *client, gchar *arg2, DataSize size)
{
	gint fd = client->fd;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint res = 0;
	gint tmpi = 0;
	gchar ** vars = NULL;
	gchar * tmpbuf = NULL;
	gint len = 0;

	/* We want canID, page, offset
	 * If firmware in use doesn't have canBUS capability
	 * just use zero for the option, likewise for page
	 */
	vars = g_strsplit(arg2,",",-1);
	if (g_strv_length(vars) != 3)
	{
		return_socket_error(fd);
		g_strfreev(vars); 
	}
	else
	{
		canID = atoi(vars[0]);
		page = atoi(vars[1]);
		offset = atoi(vars[2]);
		tmpi = get_ecu_data(canID,page,offset,size);
		_set_sized_data(client->ecu_data[page],offset,size,tmpi);
		tmpbuf = g_strdup_printf("%i\r\n",tmpi);
		len = strlen(tmpbuf);
		res = send(fd,tmpbuf,len,0);
		if (len != res)
			printf("SHORT WRITE!\n");
		g_free(tmpbuf);
		g_strfreev(vars); 
	}
}


void socket_get_ecu_vars(MtxSocketClient *client, gchar *arg2)
{
	extern Firmware_Details *firmware;
	gint fd = client->fd;
	gint canID = 0;
	gint page = 0;
	gint tmpi = 0;
	gchar ** vars = NULL;
	GString * output = NULL;
	gint i = 0;
	gint len = 0;

	/* We want canID, page
	 * If firmware in use doesn't have canBUS capability
	 * just use zero for the option, likewise for page
	 */
	vars = g_strsplit(arg2,",",-1);
	if (g_strv_length(vars) != 2)
	{
		return_socket_error(fd);
		g_strfreev(vars); 
	}
	else
	{
		canID = atoi(vars[0]);
		page = atoi(vars[1]);
		len = firmware->page_params[page]->length;
		output = g_string_sized_new(8);
		for (i=0;i<len;i++)
		{
			tmpi = get_ecu_data(canID,page,i,MTX_U08);
			_set_sized_data(client->ecu_data[page],i,MTX_U08,tmpi);
			if (i < (len -1))
				g_string_append_printf(output,"%i,",tmpi);
			else
				g_string_append_printf(output,"%i\n\r",tmpi);
		}
		g_strfreev(vars); 
		send(fd,output->str,output->len,0);
		g_string_free(output,TRUE);
	}
}


void socket_set_ecu_var(MtxSocketClient *client, gchar *arg2, DataSize size)
{
	int fd = client->fd;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint data = 0;
	gchar ** vars = NULL;

	/* We want canID, page, offset, data
	 * If firmware in use doesn't have canBUS capability
	 * just use zero for the option, likewise for page
	 */
	vars = g_strsplit(arg2,",",-1);
	if (g_strv_length(vars) != 4)
	{
		return_socket_error(fd);
		g_strfreev(vars); 
	}
	else
	{
		canID = atoi(vars[0]);
		page = atoi(vars[1]);
		offset = atoi(vars[2]);
		data = atoi(vars[3]);
		send_to_ecu(canID,page,offset,size,data,TRUE);
		_set_sized_data(client->ecu_data[page],offset,size,data);
		g_strfreev(vars); 
	}
}


gboolean check_for_changes(MtxSocketClient *client)
{
	gint i = 0;
	extern Firmware_Details *firmware;

	if (!firmware)
		return FALSE;
	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;

		if (!firmware->ecu_data[i])
			continue;
		if(memcmp(client->ecu_data[i],firmware->ecu_data[i],firmware->page_params[i]->length) != 0)
			return TRUE;
	}
	return FALSE;

}


gint * convert_socket_data(gchar *buf, gint len)
{
	gint i = 0;
	gint *res = g_new0(gint,len);

	for (i=0;i<len;i++)
	{
		memcpy (&res[i],&buf[i],1);
//		printf("data[%i] is %i\n",i,res[i]);
	}
	return res;
}


void *network_repair_thread(gpointer data)
{
	/* - DEV code for setting up connection to a network socket
	 * in place of a serial port,  useful for chaining instances of
	 * megatunix to a master, allows "group mind" tuning, or a complete
	 * disaster...
	 */
	static gboolean network_is_open = FALSE; /* Assume never opened */
	extern volatile gboolean offline;
	extern GAsyncQueue *io_repair_queue;
	extern GObject *global_data;
	CmdLineArgs *args = NULL;
	args = OBJ_GET(global_data,"args");
	gint i = 0;

	if (offline)
	{
		g_timeout_add(100,(GtkFunction)queue_function,g_strdup("kill_conn_warning"));
		g_thread_exit(0);
	}

	if (!io_repair_queue)
		io_repair_queue = g_async_queue_new();
	/* IF network_is_open is true, then the port was ALREADY opened 
	 * previously but some error occurred that sent us down here. Thus
	 * first do a simple comms test, if that succeeds, then just cleanup 
	 * and return,  if not, close the port and essentially start over.
	 */
	if (network_is_open == TRUE)
	{
		dbg_func(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" network_repair_thread()\n\t Port considered open, but throwing errors\n"));
		i = 0;
		while (i <= 5)
		{
			dbg_func(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" network_repair_thread()\n\t Calling comms_test, attempt %i\n",i));
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
			printf ("exiting network repair thread immediately\n");
			g_timeout_add(100,(GtkFunction)queue_function,g_strdup("kill_conn_warning"));
			g_thread_exit(0);
		}
		if (open_network(args->network_host,args->network_port))
		{
			g_usleep(100000); /* Sleep 100ms */
			if (comms_test())
			{
				network_is_open = TRUE;
				open_control_socket(args->network_host,MTX_SOCKET_CONTROL_PORT);
				break;
			}
			else
			{
				close_network();
				continue;
			}

		}
	}
	if (network_is_open)
	{
		thread_update_widget(g_strdup("active_port_entry"),MTX_ENTRY,g_strdup_printf("%s:%i",args->network_host,args->network_port));
	}
	g_thread_exit(0);
	return NULL;
}


gboolean open_network(gchar * host, gint port)
{
	extern GObject * global_data;
	int clientsocket = 0;
	gint status = 0;
	struct hostent *hostptr = NULL;
	struct sockaddr_in servername = { 0 };
	extern Serial_Params *serial_params;
#ifdef __WIN32__
	struct WSAData wsadata;
	status = WSAStartup(MAKEWORD(2, 2),&wsadata);
	if (status != 0)
	{
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		dbg_func(CRITICAL|SERIAL_RD|SERIAL_WR,g_strdup_printf("WSAStartup failed with error: %d\n", status));
		return FALSE;
	}
#endif

	//	printf ("Trying to open network port!\n");
	clientsocket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (!clientsocket)
	{
		dbg_func(CRITICAL|SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__"open_network()\n\tSocket open error: %s\n",strerror(errno)));
#ifdef __WIN32__
		WSACleanup();
#endif
		return FALSE;
	}
	//	printf("Socket created!\n");
	hostptr = gethostbyname(host);
	if (hostptr == NULL)
	{
		hostptr = gethostbyaddr(host,strlen(host), AF_INET);
		if (hostptr == NULL)
		{
			dbg_func(CRITICAL|SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__"open_network()\n\tError resolving server address: \"%s\"\n",host));
#ifdef __WIN32__
			WSACleanup();
#endif
			return FALSE;
		}
	}
	//	printf("host resolved!\n");
	servername.sin_family = AF_INET;
	servername.sin_port = htons(port);
	memcpy(&servername.sin_addr,hostptr->h_addr,hostptr->h_length);
	status = connect(clientsocket,(struct sockaddr *) &servername, sizeof(servername));
	if (status == -1)
	{
		dbg_func(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__"open_network()\n\tSocket connect error: %s\n",strerror(errno)));
#ifdef __WIN32__
		WSACleanup();
#endif
		return FALSE;
	}
	//	printf("connected!!\n");
	serial_params->fd = clientsocket;
	serial_params->net_mode = TRUE;
	serial_params->open = TRUE;

	return TRUE;
}


gboolean open_notification_link(gchar * host, gint port)
{
	extern GObject * global_data;
	int clientsocket = 0;
	gint status = 0;
	struct hostent *hostptr = NULL;
	struct sockaddr_in servername = { 0 };
	extern Serial_Params *serial_params;
#ifdef __WIN32__
	struct WSAData wsadata;
	status = WSAStartup(MAKEWORD(2, 2),&wsadata);
	if (status != 0)
	{
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		dbg_func(CRITICAL|SERIAL_RD|SERIAL_WR,g_strdup_printf("WSAStartup failed with error: %d\n", status));
		return FALSE;
	}
#endif

	//	printf ("Trying to open network port!\n");
	clientsocket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (!clientsocket)
	{
		dbg_func(CRITICAL|SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__"open_network()\n\tSocket open error: %s\n",strerror(errno)));
#ifdef __WIN32__
		WSACleanup();
#endif
		return FALSE;
	}
	//	printf("Socket created!\n");
	hostptr = gethostbyname(host);
	if (hostptr == NULL)
	{
		hostptr = gethostbyaddr(host,strlen(host), AF_INET);
		if (hostptr == NULL)
		{
			dbg_func(CRITICAL|SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__"open_network()\n\tError resolving server address: \"%s\"\n",host));
#ifdef __WIN32__
			WSACleanup();
#endif
			return FALSE;
		}
	}
	//	printf("host resolved!\n");
	servername.sin_family = AF_INET;
	servername.sin_port = htons(port);
	memcpy(&servername.sin_addr,hostptr->h_addr,hostptr->h_length);
	status = connect(clientsocket,(struct sockaddr *) &servername, sizeof(servername));
	if (status == -1)
	{
		dbg_func(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__"open_network()\n\tSocket connect error: %s\n",strerror(errno)));
#ifdef __WIN32__
		WSACleanup();
#endif
		return FALSE;
	}
	//	printf("connected!!\n");
	/* Should startup thread now to listen for notification messages
	 */

	return TRUE;
}
		

gboolean close_network(void)
{
	extern Serial_Params *serial_params;
	extern gboolean connected;
	//	printf("Closing network port!\n");
	close(serial_params->fd);
	serial_params->open = FALSE;
	serial_params->fd = -1;
	connected = FALSE;

#ifdef __WIN32__
	WSACleanup();
#endif
	return TRUE;
}


gboolean close_control_socket(void)
{
	extern gboolean connected;
	printf("Closing control socket (there is a bug here!) port!\n");
	close(controlsocket);

#ifdef __WIN32__
	WSACleanup();
#endif
	return TRUE;
}


/*!
 \brief notify_slaves_thread()'s sole purpose in life is to listen for 
 messages on the async queue from the IO core for messages to send to slaves
 and dispatch them out.  It should check if a slave disconected and in that 
 case delete their entry from the slave list.
 \param data (gpointer) unused.
 **/
void *notify_slaves_thread(gpointer data)
{
	GTimeVal cur;
	SlaveMessage *msg = NULL;
	MtxSocketClient * cli_data = NULL;
	guint8 byte = 0;
	fd_set wr;
	FD_ZERO(&wr);
	gboolean *to_be_closed = NULL;
	gint i = 0;
	gint len = 0;
	gint fd = 0;
	gint res = 0;
	extern GAsyncQueue *slave_msg_queue;
	extern volatile gboolean leaving;

	while(TRUE) /* endless loop */
	{
		g_get_current_time(&cur);
		g_time_val_add(&cur,1000000); /* 1000 ms timeout */
		msg = g_async_queue_timed_pop(slave_msg_queue,&cur);

		if (!slave_list) /* List not created yet.. */
			continue;

		if (leaving)
		{
			/* drain queue and exit thread */
			while (g_async_queue_try_pop(slave_msg_queue) != NULL)
			{}
			g_thread_exit(0);
		}
		if (!msg) /* Null message)*/
			continue;

		printf("There are %i clients in the slave pointer array\n",slave_list->len);
		to_be_closed = g_new0(gboolean, slave_list->len);
		for (i=0;i<slave_list->len;i++)
		{
			cli_data = g_ptr_array_index(slave_list,i);
			fd = cli_data->fd;
			FD_SET(fd,&wr);
			res = select(fd+1,NULL,&wr,NULL,NULL); 
			if (res <= 0)
			{
				printf("Select error!, closing this socket\n");
				to_be_closed[i] = TRUE;
				continue;
			}
			printf("sending chunk update\n");
			byte = SLAVE_MEMORY_UPDATE;

			/* Message type */
			res = send(fd,(char *)&byte,1,0);
			/* CanID */
			res = send(fd,(char *)&(msg->canID),1,0);
			/* Page (MTX internal page) */
			res = send(fd,(char *)&(msg->page),1,0);
			/* highbyte of offset */
			//printf("Offset is %i\n",msg->offset);
			byte = (msg->offset >> 8) & 0xff;
			//printf("high byte of offset is %i\n",byte);
			res = send(fd,(char *)&(byte),1,0);
			/* lowbyte of offset */
			byte = (msg->offset & 0xff);
			//printf("low byte of offset is %i\n",byte);
			res = send(fd,(char *)&(byte),1,0);
			/* highbyte of length */
			byte = (msg->length >> 8) & 0xff;
			//printf("high byte of offset is %i\n",byte);
			res = send(fd,(char *)&(byte),1,0);
			/* lowbyte of length */
			byte = (msg->length & 0xff);
			res = send(fd,(char *)&(byte),1,0);
			if (msg->mode == MTX_SIMPLE_WRITE)
				res = send(fd,(char *)&(msg->value),msg->length,0);
			else if (msg->mode == MTX_CHUNK_WRITE)
				res = send(fd,(char *)msg->data,msg->length,0);

			if (res == -1)
				to_be_closed[i] = TRUE;
		}
		len = slave_list->len;
		for (i=0;i<len;i++)
		{
			if (to_be_closed[i])
			{
				cli_data = g_ptr_array_index(slave_list,i);
				
				printf("socket dropped, closing client pointer %p\n",cli_data);
#ifdef __WIN32__
				closesocket(cli_data->fd);
#else
				close(cli_data->fd);
#endif
				printf("REMOVING ENTRY from ptr array\n");
				g_ptr_array_remove(slave_list,cli_data);
				dealloc_client_data(cli_data);
				res = 0;
			}
		}
		g_free(to_be_closed);
		to_be_closed = NULL;
		if (msg->data)
			g_free(msg->data);
		g_free(msg);
		msg = NULL;
	}
	return NULL;
}




void *control_socket_client(gpointer data)
{
	MtxSocketClient *client = (MtxSocketClient *) data;
	gint fd = client->fd;
	guint8 buf;
	gint i = 0;
	gint res = 0;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint offset_h = 0;
	gint offset_l = 0;
	gint count = 0;
	gint count_h = 0;
	gint count_l = 0;
	gint index = 0;
	guint8 *buffer = NULL;
	State state = WAITING_FOR_CMD;;
	SubState substate = UNDEFINED_SUBSTATE;
	extern Firmware_Details *firmware;
	extern volatile gint last_page;

	while(TRUE)
	{
		res = recv(fd,(char *)&buf,1,0);
		if (res <= 0)
		{
#ifdef __WIN32__
			printf("READ ERROR, controlsocket\n");
			closesocket(fd);
#else
			close(fd);
#endif
			dealloc_client_data(client);
			g_thread_exit(0);
		}
		printf("controlsocket Data arrived!\n");
		printf("data %i, %c\n",(gint)buf,(gchar)buf);
		switch (state)
		{
			case WAITING_FOR_CMD:
				if (buf == SLAVE_MEMORY_UPDATE)
				{
					printf("Slave chunk update received\n");
					state = GET_CAN_ID;
					substate = GET_VAR_DATA;
					continue;
				}
				else
					continue;
			case GET_CAN_ID:
				printf("get_canid block\n");
				canID = (guint8)buf;
				printf("canID received is %i\n",canID);
				state = GET_MTX_PAGE;
				continue;
			case GET_MTX_PAGE:
				printf("get_mtx_page block\n");
				page = (guint8)buf;
				printf("page received is %i\n",page);
				state = GET_HIGH_OFFSET;
				continue;
			case GET_HIGH_OFFSET:
				printf("get_high_offset block\n");
				offset_h = (guint8)buf;
				printf("high offset received is %i\n",offset_h);
				state = GET_LOW_OFFSET;
				continue;
			case GET_LOW_OFFSET:
				printf("get_low_offset block\n");
				offset_l = (guint8)buf;
				printf("low offset received is %i\n",offset_l);
				offset = offset_l + (offset_h << 8);
				state = GET_HIGH_COUNT;
				continue;
			case GET_HIGH_COUNT:
				printf("get_high_count block\n");
				count_h = (guint8)buf;
				printf("high count received is %i\n",count_h);
				state = GET_LOW_COUNT;
				continue;
			case GET_LOW_COUNT:
				printf("get_low_count block\n");
				count_l = (guint8)buf;
				printf("low count received is %i\n",count_l);
				count = count_l + (count_h << 8);
				if (substate == GET_VAR_DATA)
				{
					state = GET_DATABYTE;
					buffer = g_new0(guint8, count);
					index = 0;
				}
				continue;
			case GET_DATABYTE:
				printf("get_databyte\n");
				buffer[index] = (guint8)buf;
				index++;
				printf ("Databyte index %i of %i\n",index,count);
				if (index >= count)
				{
					printf("Got all needed data, updating gui\n");
					state = WAITING_FOR_CMD;
					store_new_block(canID,page,offset,buffer,count);
					/* Update gui with changes */
					for (i=offset;i<(offset+count);i++)
						refresh_widgets_at_offset(page,i);
					g_free(buffer);
				}
				else
					state = GET_DATABYTE;
				continue;
			default:
				printf("Case not handled, bug in state machine!\n");
				continue;

		}
	}
}


gboolean open_control_socket(gchar * host, gint port)
{
	extern GObject * global_data;
	int clientsocket = 0;
	gint status = 0;
	struct hostent *hostptr = NULL;
	struct sockaddr_in servername = { 0 };
	printf("Open control socket!\n");
#ifdef __WIN32__
	struct WSAData wsadata;
	status = WSAStartup(MAKEWORD(2, 2),&wsadata);
	if (status != 0)
	{
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		dbg_func(CRITICAL|SERIAL_RD|SERIAL_WR,g_strdup_printf("WSAStartup failed with error: %d\n", status));
		return FALSE;
	}
#endif

	//	printf ("Trying to open network port!\n");
	clientsocket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (!clientsocket)
	{
		dbg_func(CRITICAL|SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__"open_network()\n\tSocket open error: %s\n",strerror(errno)));
#ifdef __WIN32__
		WSACleanup();
#endif
		return FALSE;
	}
	//	printf("Socket created!\n");
	hostptr = gethostbyname(host);
	if (hostptr == NULL)
	{
		hostptr = gethostbyaddr(host,strlen(host), AF_INET);
		if (hostptr == NULL)
		{
			dbg_func(CRITICAL|SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__"open_network()\n\tError resolving server address: \"%s\"\n",host));
#ifdef __WIN32__
			WSACleanup();
#endif
			return FALSE;
		}
	}
	//	printf("host resolved!\n");
	servername.sin_family = AF_INET;
	servername.sin_port = htons(port);
	memcpy(&servername.sin_addr,hostptr->h_addr,hostptr->h_length);
	status = connect(clientsocket,(struct sockaddr *) &servername, sizeof(servername));
	if (status == -1)
	{
		dbg_func(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__"open_network()\n\tSocket connect error: %s\n",strerror(errno)));
#ifdef __WIN32__
		WSACleanup();
#endif
		return FALSE;
	}
	printf("connected!!\n");
	MtxSocketClient * cli_data = NULL;
	controlsocket = clientsocket;
	cli_data = g_new0(MtxSocketClient, 1);
	cli_data->ip = g_strdup(inet_ntoa(servername.sin_addr));
	cli_data->port = ntohs(servername.sin_port);
	cli_data->fd = clientsocket;
	cli_data->type = MTX_SOCKET_CONTROL;
	g_thread_create(control_socket_client,
			cli_data, /* Thread args */
			TRUE,   /* Joinable */
			NULL);  /* GError pointer */

	return TRUE;
}
