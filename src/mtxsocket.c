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
#include <configfile.h>
#include <comms.h>
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <fcntl.h>
#include <firmware.h>
#include <glib.h>
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
	gint socket = (int) data;
	struct sockaddr_in client;
#ifdef __WIN32__
	int length = sizeof(client);
#else
	socklen_t length = sizeof(client);
#endif
	gint fd = 0;

	while (TRUE)
	{
		fd = accept(socket,(struct sockaddr *)&client, &length);
		g_thread_create(socket_client,
				GINT_TO_POINTER(fd), /* Thread args */
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
	gint fd = (gint)data;
	char buf[1024];
	fd_set rd;
	FD_ZERO(&rd);
	FD_SET(fd,&rd);
	gint res = 0;

	while (TRUE)
	{
		if (!fd)
			return(0);
		res = select(fd+1,&rd,NULL,NULL,NULL);
		if (res < 0) /* Error, cocket closed, abort */
		{
#ifdef __WIN32__
			closesocket(fd);
#else
			close(fd);
#endif
		//	g_thread_exit(0);
			return 0;
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
				g_thread_exit(0);
			}
			if (!validate_remote_cmd(fd,buf,res))
				g_thread_exit(0);
		}
	}
}


/*!
 \brief This function validates incoming commands from the TCP socket 
 thread(s).  Commands need to be comma separated, ASCII text, minimum of two
 arguments (more are allowed)
 \param fd, client filedescriptor
 \param buf, input buffer
 \param len, length of input buffer
 */
gboolean validate_remote_cmd(gint fd, gchar * buf, gint len)
{
	extern Firmware_Details *firmware;
	gchar ** vector = NULL;
	gchar * arg2 = NULL;
	gint args = 0;
	gsize res = 0;
	gint cmd = 0;
	gboolean retval = TRUE;
	gchar *tmpbuf = g_strchomp(g_strdelimit(g_strndup(buf,len),"\n\r\t",' '));
	vector = g_strsplit(tmpbuf,",",2);
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
		case GET_ECU_VAR_U08:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(fd,arg2,MTX_U08);
			break;
		case GET_ECU_VAR_S08:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(fd,arg2,MTX_S08);
			break;
		case GET_ECU_VAR_U16:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(fd,arg2,MTX_U16);
			break;
		case GET_ECU_VAR_S16:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(fd,arg2,MTX_S16);
			break;
		case GET_ECU_VAR_U32:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(fd,arg2,MTX_U32);
			break;
		case GET_ECU_VAR_S32:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_get_ecu_var(fd,arg2,MTX_S32);
			break;
		case SET_ECU_VAR_U08:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(fd,arg2,MTX_U08);
			break;
		case SET_ECU_VAR_S08:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(fd,arg2,MTX_S08);
			break;
		case SET_ECU_VAR_U16:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(fd,arg2,MTX_U16);
			break;
		case SET_ECU_VAR_S16:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(fd,arg2,MTX_S16);
			break;
		case SET_ECU_VAR_U32:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(fd,arg2,MTX_U32);
			break;
		case SET_ECU_VAR_S32:
			if  (args != 2) 
				return_socket_error(fd);
			else
				socket_set_ecu_var(fd,arg2,MTX_S32);
			break;
		case BURN_FLASH:
			io_cmd(firmware->burn_all_command,NULL);

			break;
			
		case GET_SIGNATURE:
			send(fd,firmware->actual_signature,strlen(firmware->actual_signature),0);
			res = send(fd,"\n\r",strlen("\n\r"),0);
			break;
		case HELP:
			tmpbuf = g_strdup("\r\nSupported Calls:\n\rhelp\n\rquit\n\rget_signature <-- Returns ECU Signature\n\rget_rtv_list <-- returns runtime variable listing\n\rget_rt_vars,<var1>,<var2>,... <-- returns values of specified variables\n\rget_ecu_var[u08|s08|u16|s16|u32|s32],<canID>,<page>,<offset>\n\r\tReturns the ecu variable at the spcified location, if firmware\n\r\tis not CAN capable, use 0 for canID, likewise for non-paged\n\r\tfirmwares use 0 for page...\n\rset_ecu_var[u08|s08|u16|s16|u32|s32],<canID>,<page>,<offset>,<data>\n\r\tSets teh ecu variable at the spcified location, if firmware\n\r\tis not CAN capable, use 0 for canID, likewise for non-paged\n\r\tfirmwares use 0 for page...\n\rburn_flash <-- Burns contents of ecu ram for current page to flash\n\r");
//			tmpbuf = g_strdup("\rSee MegaTunix Documentation.\n\r");
			send(fd,tmpbuf,strlen(tmpbuf),0);
			g_free(tmpbuf);
			break;
		case QUIT:
			tmpbuf = g_strdup("\rBuh Bye...\n\r");
			send(fd,tmpbuf,strlen(tmpbuf),0);
			g_free(tmpbuf);
#ifdef __WIN32__
			closesocket(fd);
#else
			close(fd);
#endif
			retval = FALSE;
			break;
		default:
			return_socket_error(fd);
			break;
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
	gint i = 0;
	gint tmpi = 0;
	gfloat tmpf = 0.0;
	GString *output;

	vars = g_strsplit(arg2,",",-1);
	output = g_string_sized_new(8);
	for (i=0;i<g_strv_length(vars);i++)
	{
		lookup_current_value(vars[i],&tmpf);
		lookup_precision(vars[i],&tmpi);
		if (i < (g_strv_length(vars)-1))
			g_string_append_printf(output,"%1$.*2$f ",tmpf,tmpi);
		else
			g_string_append_printf(output,"%1$.*2$f\n\r",tmpf,tmpi);
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
		tmpbuf = g_strdup_printf("%s\r\n",(gchar *)OBJ_GET(object,"internal_names"));
		if (tmpbuf)
		{
			len = strlen(tmpbuf);
			res = send(fd,tmpbuf,len,0);
			if (res != len)
				printf("SHORT WRITE!\n");
			g_free(tmpbuf);
		}
	}
}


void socket_get_ecu_var(gint fd, gchar *arg2, DataSize size)
{
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
		tmpbuf = g_strdup_printf("%i\r\n",tmpi);
		len = strlen(tmpbuf);
		res = send(fd,tmpbuf,len,0);
		if (len != res)
			printf("SHORT WRITE!\n");
		g_free(tmpbuf);
		g_strfreev(vars); 
	}
}


void socket_set_ecu_var(gint fd, gchar *arg2, DataSize size)
{
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
		g_strfreev(vars); 
	}
}
