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

#ifndef CRTSCTS
#define CRTSCTS 0
#endif


#include <config.h>
#include <defines.h>
#include <enums.h>
#include <errno.h>
#include <fcntl.h>
#include <getfiles.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <getfiles.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winserialio.h>
#ifndef __WIN32__
 #include <termios.h>
#endif




typedef enum
{
	NOT_LISTENING=0xcba,
	IN_BOOTLOADER,
	LIVE_MODE
}EcuState;

/* Prototypes */
void usage_and_exit(gchar *);
void verify_args(gint , gchar **);
gint setup_port(gchar * );
void flush_serial(gint, FlushDirection);
EcuState detect_ecu(gint);
gboolean jump_to_bootloader(gint);
gboolean prepare_for_upload(gint);
void upload_firmware(gint, gint);
void reboot_ecu(gint);
void close_port(gint);

/* Globals */
#ifndef __WIN32__
struct termios oldtio;
struct termios newtio;
#endif

/*!
 \brief main() is the typical main function in a C program, it performs
 all core initialization, loading of all main parameters, initializing handlers
 and entering gtk_main to process events until program close
 \param argc (gint) count of command line arguments
 \param argv (char **) array of command line args
 \returns TRUE
 */
gint main(gint argc, gchar ** argv)
{
	gint port_fd = 0;
	gint file_fd = 0;
	EcuState ecu_state = NOT_LISTENING;
	gchar * tmpbuf = NULL;
	gboolean result = FALSE;
	
	verify_args(argc, argv);
	/* If we got this far, all is good argument wise */
	port_fd = setup_port(argv[1]);
	if (port_fd > 0)
		printf("Port successfully opened\n");
	else
	{
		printf("Couldn't open port, check permissions/paths\n");
		exit(-1);
	}
#ifdef __WIN32__
	file_fd = open(argv[2], O_RDWR | O_BINARY );
#else
	file_fd = g_open(argv[2],O_RDONLY,S_IRUSR);
#endif
	if (file_fd > 0 )
		printf("Firmware file successfully opened\n");
	else
	{
		printf("Couldn't open firmware file, check permissions/paths\n");
		exit(-1);
	}
	ecu_state = detect_ecu(port_fd);
	switch (ecu_state)
	{
		case NOT_LISTENING:
			printf("Couldn't detect firmware signature\n");
			break;
		case IN_BOOTLOADER:
			printf ("ECU is in bootloader mode, good!\n");
			break;
		case LIVE_MODE:
			printf("Ecu detected in live mode, trying to get to bootloader mode\n");
			result = jump_to_bootloader(port_fd);
			if (result)
			{
				ecu_state = detect_ecu(port_fd);
				if (ecu_state == IN_BOOTLOADER)
				{
					printf ("ECU is in bootloader mode, good!\n");
					break;
				}
				else
					printf("Couldn't attain bootloader mode\n");
			}
			break;
	}
	if (ecu_state != IN_BOOTLOADER)
	{
		printf("Please jump the boot jumper on the ECU and power cycle it\n");
		printf("Press any key to continue\n");
		getc(stdin);
		g_free(tmpbuf);
		ecu_state = detect_ecu(port_fd);
		if (ecu_state != IN_BOOTLOADER)
		{
			printf("unable to get to the bootloader, exiting!\n");
			exit (-1);
		}
		else
			printf("Got into the bootloader, good!\n");

	}
	result = prepare_for_upload(port_fd);
	if (!result)
	{
		printf("Failure getting ECU into a state to accept new firmware\n");
		exit (-1);
	}
	upload_firmware(port_fd,file_fd);
	printf("Firmware upload completed!!!\n");
	reboot_ecu(port_fd);
	printf("Ecu Reboot complete!!!\n");
	close_port(port_fd);
	
	return (0) ;
}

void verify_args(gint argc, gchar **argv)
{
	/* Invalid arg count, abort */
	if (argc != 3)
		usage_and_exit(g_strdup("Invalid number of arguments!!"));

	
	if (!g_file_test(argv[1], G_FILE_TEST_EXISTS))
		usage_and_exit(g_strdup_printf("Port \"%s\" does NOT exist...",argv[1]));

	if (!g_file_test(argv[2], G_FILE_TEST_IS_REGULAR))
		usage_and_exit(g_strdup_printf("Filename \"%s\" does NOT exist...",argv[2]));
}

void usage_and_exit(gchar * msg)
{
	printf("\n\nERROR!!!\n\t%s\n",msg);
	g_free(msg);
	printf("\n\nINVALID USAGE\n\n  msloader /path/to/port  /path/to/.s19\n\n");
	exit (-1);
}

gint setup_port(gchar * port_name)
{
	gint fd = 0;
#ifdef __WIN32__
	fd = open(port_name, O_RDWR | O_BINARY );
#else
	fd = open(port_name, O_RDWR | O_NOCTTY);
#endif
	if (fd < 0)
		return -1;	/* ERROR, port did NOT open */

#ifdef __WIN32__
	win32_setup_serial_params(fd, 9600);
#else

	/* Save serial port status */
	tcgetattr(fd,&oldtio);
	flush_serial(fd, TCIOFLUSH);

	memset(&newtio, 0, sizeof(newtio));
	/* 
	 * BAUDRATE: Set bps rate. You could also use cfsetispeed and 
	 * cfsetospeed
	 * CRTSCTS : output hardware flow control (only used if the cable has
	 * all necessary lines. See sect. 7 of Serial-HOWTO)
	 * CS8     : 8n1 (8bit,no parity,1 stopbit)
	 * CLOCAL  : local connection, no modem contol
	 * CREAD   : enable receiving characters
	 */

	/* Set baud (posix way) */

	cfsetispeed(&newtio, B9600);
	cfsetospeed(&newtio, B9600);

	/* Mask and set to 8N1 mode... */
	newtio.c_cflag &= ~(CRTSCTS | PARENB | CSTOPB | CSIZE);
	/* Set additional flags, note |= syntax.. */
	/* Enable receiver, ignore modem ctrls lines, use 8 bits */
	newtio.c_cflag |= CLOCAL | CREAD | CS8;

	/* RAW Input */
	/* Ignore signals, enable canonical, etc */
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/* Disable software flow control */
	newtio.c_iflag &= ~(IXON | IXOFF );

	/* Set raw output */
	newtio.c_oflag &= ~OPOST;
	/* 
	 *            initialize all control characters 
	 *                       default values can be found in /usr/include/termios.h, and are given
	 *                                  in the comments, but we don't need them here
	 *                                           */
	newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	newtio.c_cc[VERASE]   = 0;     /* del */
	newtio.c_cc[VKILL]    = 0;     /* @ */
	newtio.c_cc[VEOF]     = 0;     /* Ctrl-d */
	newtio.c_cc[VEOL]     = 0;     /* '\0' */
	newtio.c_cc[VMIN]     = 0;
	newtio.c_cc[VTIME]    = 1;     /* 100ms timeout */

	tcsetattr(fd,TCSAFLUSH,&newtio);

#endif
	return fd;

}


void close_port(gint fd)
{
#ifndef __WIN32__
	tcsetattr(fd,TCSAFLUSH,&oldtio);
#endif
	close(fd);
	return;
}

void flush_serial(gint fd, FlushDirection type)
{
#ifdef __WIN32__
	if (fd)
		win32_flush_serial(fd, type); 
#else
	if (fd)
	{
		switch (type)
		{
			case INBOUND:
				tcflush(fd, TCIFLUSH);
				break;
			case OUTBOUND:
				tcflush(fd, TCOFLUSH);
				break;
			case BOTH:
				tcflush(fd, TCIOFLUSH);
				break;
		}
	}
#endif
}

EcuState detect_ecu(gint fd)
{
	gint res = 0;
	gint size = 1024;
	guchar buf[1024];
	guchar *ptr = buf;
	gint total_read = 0;
	gint total_wanted = 0;
	gint zerocount = 0;
	gchar  *message = NULL;

	/* Probe for response 
	 * First check for signature (running state)
	 * If that fails, see if we are in bootloader mode already
	 */

	flush_serial(fd,BOTH);
	res = write (fd,"S",1);
	if (res != 1)
		printf("Failure sending signature request!\n");
	total_read = 0;
	total_wanted = size;
	zerocount = 0;
	while ((total_read < total_wanted ) && (total_wanted-total_read) > 0 )
	{
		total_read += res = read(fd,
				ptr+total_read,
				total_wanted-total_read);

		/* If we get nothing back (i.e. timeout, assume done)*/
		if (res <= 0)
			zerocount++;

		if (zerocount > 1)
			break;

	}
	if (total_read > 0)
	{
		message = g_strndup(((gchar *)buf),total_read);
		/* Check for "what" or "Boot" */
		if (g_strrstr_len(message,total_read, "what"))
		{
			g_free(message);
			return IN_BOOTLOADER;
		}
		else if (g_strrstr_len(message,total_read, "Boot"))
		{
			g_free(message);
			return IN_BOOTLOADER;
		}
		else	
		{
			printf("Detected signature: \"%s\"\n",message);
			g_free(message);
			return  LIVE_MODE;
		}
	}

	return NOT_LISTENING;
}

gboolean jump_to_bootloader(gint fd)
{
	gint res = 0;

	res = write (fd,"!!",2);
	if (res != 2)
		printf("Error trying to get \"Boot>\" Prompt\n");

	return TRUE;
}


gboolean prepare_for_upload(gint fd)
{
	gint res = 0;
	gchar buf[1024];
	gchar * message = NULL;

	res = write(fd,"W",1);
	if (res != 1)
	{
		printf("Error trying to initiate ECU wipe\n");
		return FALSE;
	}
	flush_serial(fd,OUTBOUND);
	g_usleep(1000000); /* 1000ms timeout for flash to erase */
	res = read(fd,&buf,1024);
	message = g_strndup(((gchar *)buf),res);
	if (g_strrstr_len(buf,res,"Complete"))
	{
		g_free(message);
		printf("Wipe Completed\n");
		res = write(fd,"U",1);
		if (res != 1)
		{
			printf("Error trying to initiate ECU upgrade\n");
			return FALSE;
		}
		flush_serial(fd,OUTBOUND);
		g_usleep(1000000); /* 1000ms timeout for flash to erase */
		res = read(fd,&buf,1024);
		if (g_strrstr_len(buf,res,"waiting"))
		{
			printf("Ready to update firmware\n");
			return TRUE;
		}
		else
		{
			printf("Error getting \"ready to update\" message\n");
			return FALSE;
		}
	}
	else
	{
		printf("Error wiping ECU, result \"%s\"\n",message);
		g_free(message);
		return FALSE;
	}
}


void upload_firmware(gint fd, gint file_fd)
{
	gint res = 0;
	gchar buf[64];
	gint i = 0;

	res = read(file_fd,buf,64);
	while (res > 0)
	{
		i+=res;
		write (fd,buf,64);
		printf("%6i bytes written.\n",i);
		res = read(file_fd,buf,64);
	}
	flush_serial(fd,BOTH);
	return;
}


void reboot_ecu(gint fd)
{
	gint res = 0;

	printf("sleeping 5 seconds\n");
	g_usleep(5000000);
	res = write (fd,"X",1);
	flush_serial(fd,OUTBOUND);
	if (res != 1)
		printf("Error trying to reboot ECU\n");

	return ;
}

