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
#include <config.h>
#include <configfile.h>
#include <comms.h>
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <fcntl.h>
#include <firmware.h>
#include <glib.h>
#include <init.h>
#include <mtxsocket.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
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


int setup_socket(void)
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
		return (-1);
	}
#endif

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("Socket!");
#ifdef __WIN32__
		WSACleanup();
#endif
		return(-1);

	}
	/* So that we can re-bind to it without TIME_WAIT problems */
#ifdef __WIN32__
	res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse_addr, sizeof(reuse_addr));
#else
	res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
#endif
	if (res == -1)
		perror("setsockopt(...,SO_REUSEADDR,...)");

	/* Set socket to non-blocking with our setnonblocking routine */
//	setnonblocking(sock);

	memset((char *) &server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(MTX_PORT);
	if (bind(sock, (struct sockaddr *) &server_address,sizeof(server_address)) < 0 ) {
		//writelogfile(logfd,LOG_EMERG,"error al hacer bind: %s\n",strerror(errno));
		perror("bind");
#ifdef __WIN32__
		closesocket(sock);
		WSACleanup();
#else
		close(sock);
#endif
		return(-1);
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
		return(-1);
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
	gint socket = (int) data;
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
		fd = accept(socket,(struct sockaddr *)&client, &length);
		cli_data = g_new0(MtxSocketClient, 1);
		cli_data->ip = g_strdup(inet_ntoa(client.sin_addr));
		cli_data->port = ntohs(client.sin_port);
		cli_data->fd = fd;
		if (firmware)
		{
			cli_data->ecu_data = g_new0(guint8 *, firmware->total_pages);
			for (i=0;i<firmware->total_pages;i++)
			{
				cli_data->ecu_data[i] = g_new0(guint8, firmware->page_params[i]->length);
				if (firmware->ecu_data[i])
					memcpy (cli_data->ecu_data[i],firmware->ecu_data[i],firmware->page_params[i]->length);

			}
		}

		g_thread_create(socket_client,
				cli_data, /* Thread args */
				TRUE,   /* Joinable */
				NULL);  /* GError pointer */
	}
}


/*!
 \brief socket_client is a thread that is spawned for each remote connection
 to megatunix.  It's purpose in life it to answer remote requests.  If the
 remote side is closed down, it sees a zero byte read and exits, killing off
 the thread..
 \param data  gpointer representation of the socket filedescriptor
 */
void *socket_client(gpointer data)
{
	MtxSocketClient *client = (MtxSocketClient *) data;
	gint fd = client->fd;
	char buf[1024];
	gchar * tmpbuf = NULL;
	fd_set rd;
	FD_ZERO(&rd);
	FD_SET(fd,&rd);
	gint res = 0;

/* Wait for API choice (i.e. if only a CR is received, set mode
 * to ASCII, otherwise check passed data for valid API, if valid set mode to
 * binary, otherwise drop the connection
 */
	tmpbuf = g_strdup_printf("Welcome to MegaTunix %s, hit enter for ASCII mode\n",VERSION);
	send(fd,tmpbuf,strlen(tmpbuf),0);
	g_free(tmpbuf);
	res = recv(fd,&buf,1024,0);
	printf("received\"%s\"\n",g_strescape(g_strndup(buf,res),NULL));
	/* A simple CR/LF is enough to trigger ASCII mode*/
	if (g_strncasecmp(buf,"\r\n",res) == 0)
	{
		send(fd,"ASCII mode enabled, enter \'help\' for assistance\n\r",strlen("ASCII mode enabled, enter \'help\' for assistance\n\r"),0);
		client->mode = MTX_ASCII;
	}
	else
	{
		client->mode = MTX_BINARY;
		res = validate_remote_binary_cmd(client,buf,res);
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
	}
	
	
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
			res = recv(fd,&buf,1024,0);
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
			if (client->mode == MTX_ASCII)
				res = validate_remote_ascii_cmd(client,buf,res);
			else if (client->mode == MTX_BINARY)
				res = validate_remote_binary_cmd(client,buf,res);
			else	
				printf("MTXsocket bug!, client->mode undefined!\n");
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


gboolean validate_remote_binary_cmd(MtxSocketClient *client, gchar * buf, gint len)
{
	gint fd = client->fd;
	extern Firmware_Details *firmware;
	extern gboolean connected;
	gchar ** vector = NULL;
	guint16 tmpi = 0;
	gfloat tmpf = 0.0;
	gchar * arg2 = NULL;
	gint canID = 0;
	gint tableID = 0;
	gchar tmpc;
	gchar basecmd;
	gboolean retval = TRUE;
	gboolean send_rescode = TRUE;
	gchar *tmpbuf = g_strchomp(g_strdelimit(g_strndup(buf,len),"\n\r\t",' '));
	gint length = strlen(tmpbuf);
	/* If nothing passed, return */
	if (!tmpbuf)
		return TRUE;

	printf("command send is \"%s\"\n",tmpbuf);
	basecmd = tmpbuf[0];
	printf("Basecmd is %c, cmd length is %i\n",basecmd,length);

	switch (basecmd)
	{
		case 'B':
			io_cmd(firmware->burn_all_command,NULL);
			break;
		case 'S':
			if (!firmware)
				send(fd,"Not Connected yet",strlen(" Not Connected yet"),0);
			else
			{
				if (firmware->actual_signature)
					send(fd,firmware->actual_signature,strlen(firmware->actual_signature),0);
				else
					send(fd,"Offline mode, no signature",strlen("Offline mode, no signature"),0);
			}
			break;
		case 'Q':
			if (!firmware)
				send(fd,"Not Connected yet",strlen(" Not Connected yet"),0);
			else
			{
				if (firmware->text_revision)
					send(fd,firmware->text_revision,strlen(firmware->text_revision),0);
				else
					send(fd,"Offline mode, no signature",strlen("Offline mode, no signature"),0);
			}
			break;
		case 'c': /* MS2 Clock */
			lookup_current_value("raw_secl",&tmpf);
			tmpi = (guint16)tmpf;
			send(fd,&tmpi,2,0);
			break;
		case 'a':
			tmpc = tmpbuf[1];
			canID = (gint)g_ascii_strtod(&tmpc,NULL);
			tmpc = tmpbuf[2];
			tableID = (gint)g_ascii_strtod(&tmpc,NULL);
			if (length != 3)
				printf("'a' param requires canID and table\n");
			else
				printf("Can ID is %i, table %i\n",canID,tableID);
			if ((canID == 0) && (tableID == 6))
			{
				if (firmware->rt_data)
					send(fd,firmware->rt_data,firmware->rtvars_size,0);
			}
			break;

	}
	g_free(tmpbuf);
	return TRUE;
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
