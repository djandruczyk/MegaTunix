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

#include <arpa/inet.h>
#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <fcntl.h>
#include <glib.h>
#include <mtxsocket.h>
#include <poll.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define ERR_MSG "Bad Request\n"

extern GObject *global_data;

int setup_socket(void)
{
	int sock = 0;
	struct sockaddr_in server_address;
	int reuse_addr = 1;
	int res = 0;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("Socket!");
		exit(-1);
	}
	/* So that we can re-bind to it without TIME_WAIT problems */
	res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
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
		close(sock);
		exit(-1);
	}

	/* Set up queue for incoming connections. */
	if((listen(sock,5))<0)
	{
		perror("listen");
		close(sock);
		exit(-1);
	}

	printf("\nSocket TCP/IP ready: %s:%d\n\n",inet_ntoa(server_address.sin_addr),ntohs(server_address.sin_port));
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
	socklen_t length = sizeof(client);
	gint fd = 0;

	while (1)
	{
		fd = accept(socket,(struct sockaddr *)&client, &length);
//		printf("Connection accepted from %s, creating thread for it\n",inet_ntoa(client.sin_addr));
		g_thread_create(socket_client,
				GINT_TO_POINTER(fd), /* Thread args */
				TRUE,	/* Joinable */
				NULL);	/* GError pointer */
		/* Keep track of thread ID's */
	}
}


/*!
 * \brief socket_client is a thread that is spawned for each remote connection
 * to megatunix.  It's purpose in life it to answer remote requests.  If the
 * remote side is closed down, it sees a zero byte read and exits, killing off
 * the thread..
 *\param data  gpointer representation of the socket filedescriptor
 */
void *socket_client(gpointer data)
{
	gint fd = (gint)data;
	gchar buf[1024];
	struct pollfd ufds;

	ufds.fd = fd;
	ufds.events = POLLIN;
	gint timeo = 500; /* 500 ms timeout */
	gint res = 0;

	while(TRUE)
	{
		res = poll(&ufds,1,timeo);
		if (res) /* Data Arrived */
		{
			res = recv(fd,&buf,1024,0);
			if (res == 0) /* conn died? */
			{
				close(fd);
				g_thread_exit(0);
			}
			if(!validate_remote_cmd(fd, buf,res))
				g_thread_exit(0);
		}
	}
}


/*!
 \brief This function validates incoming commands from the TCP socket 
 thread(s).  Commands need to be comma separated, ASCII text, minimum of two
 arguments (more are allowed)
 Currently implemented args:
 GET_RT_VAR,<rt_varname> (second are is text name of variable)
 GET_RTV_LIST
 GET_RAW_ECU,<canID>,<page>,<offset>,<bytes>
 SET_RAW_ECU,<canID>,<page>,<offset>,<bytes>,<data>
 \param fd, client filedescriptor
 \param buf, input buffer
 \param len, length of input buffer
 */
gboolean validate_remote_cmd(gint fd, gchar * buf, gint len)
{
	gchar ** vector = NULL;
	gint args = 0;
	gint res = 0;
	gint tmpi = 0;
	gint cmd = 0;
	gfloat tmpf = 0.0;
	gchar *tmpbuf = g_strchomp(g_strdelimit(g_strndup(buf,len),"\n\r\t",' '));
	vector = g_strsplit(tmpbuf,",",-1);
	args = g_strv_length(vector);
	tmpbuf = g_ascii_strup(vector[0],-1);
	cmd = translate_string(tmpbuf);
	g_free(tmpbuf);
	
	switch (cmd)
	{
		case GET_RT_VAR:
			if  (args != 2) 
				res = send(fd,ERR_MSG,strlen(ERR_MSG),0);
			else if (lookup_current_value(vector[1],&tmpf))
			{
				lookup_precision(vector[1],&tmpi);
				tmpbuf = g_strdup_printf("%1$.*2$f\n",tmpf,tmpi);
				res = send(fd,tmpbuf,strlen(tmpbuf),0);
				g_free(tmpbuf);
			}
			else
			{
				tmpbuf = g_strdup("Variable Not Found\n");
				res = send(fd,tmpbuf,strlen(tmpbuf),0);
				g_free(tmpbuf);
			}
			break;
		case HELP:
			tmpbuf = g_strdup("See MegaTunix Documentation.\n");
			res = send(fd,tmpbuf,strlen(tmpbuf),0);
			g_free(tmpbuf);
			break;
		case QUIT:
			tmpbuf = g_strdup("Buh Bye...\n");
			res = send(fd,tmpbuf,strlen(tmpbuf),0);
			g_free(tmpbuf);
			close(fd);
			return FALSE;
			break;
		default:
			printf( "default, unhandled currently\n");
			res = send(fd,ERR_MSG,strlen(ERR_MSG),0);
			return TRUE;
			break;
	}
	return TRUE;
}
