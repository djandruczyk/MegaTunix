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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

extern GObject *global_data;


void setnonblocking(int sock)
{
	int opts;

	opts = fcntl(sock,F_GETFL);
	if (opts < 0) {
		perror("fcntl(F_GETFL)");
		exit(-1);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock,F_SETFL,opts) < 0) {
		perror("fcntl(F_SETFL)");
		exit(-1);
	}
	return;
}

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
	gchar * ptr = buf;
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
			res = send(fd,buf,res,0);
		}
	}
}
