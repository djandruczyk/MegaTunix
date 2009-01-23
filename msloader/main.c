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
#include <defines.h>
#include <fcntl.h>
#include <getfiles.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <ms1_loader.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Prototypes */
void usage_and_exit(gchar *);
void verify_args(gint , gchar **);
gint main(gint , gchar **);
gboolean message_handler(gpointer);
/* Prototypes */

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
	gboolean result = FALSE;
	
	verify_args(argc, argv);

	output(g_strdup_printf("MegaTunix msloader %s\n",VERSION),TRUE);

	/* If we got this far, all is good argument wise */
	port_fd = setup_port(argv[1], 9600);
	if (port_fd > 0)
		output("Port successfully opened\n",FALSE);
	else
	{
		output("Could NOT open Port check permissions\n",FALSE);
		exit(-1);
	}
#ifdef __WIN32__
	file_fd = open(argv[2], O_RDWR | O_BINARY );
#else
	file_fd = g_open(argv[2],O_RDONLY,S_IRUSR);
#endif
	if (file_fd > 0 )
		output("Firmware file successfully opened\n",FALSE);
	else
	{
		output("Could NOT open firmware file, check permissions/paths\n",FALSE);
		exit(-1);
	}
	ecu_state = detect_ecu(port_fd);
	switch (ecu_state)
	{
		case NOT_LISTENING:
			output("NO response to signature request\n",FALSE);
			break;
		case IN_BOOTLOADER:
			output("ECU is in bootloader mode, good!\n",FALSE);
			break;
		case LIVE_MODE:
			output("ECU detected in LIVE! mode, attempting to access bootloader\n",FALSE);
			result = jump_to_bootloader(port_fd);
			if (result)
			{
				ecu_state = detect_ecu(port_fd);
				if (ecu_state == IN_BOOTLOADER)
				{
					output("ECU is in bootloader mode, good!\n",FALSE);
					break;
				}
				else
					output("Could NOT attain bootloader mode\n",FALSE);
			}
			else
				output("Could NOT attain bootloader mode\n",FALSE);
			break;
	}
	if (ecu_state != IN_BOOTLOADER)
	{
		output("Please jump the boot jumper on the ECU and power cycle it\n\nPress any key to continue\n",FALSE);
		getc(stdin);
		ecu_state = detect_ecu(port_fd);
		if (ecu_state != IN_BOOTLOADER)
		{
			output("Unable to get to the bootloader, update FAILED!\n",FALSE);
			exit (-1);
		}
		else
			output("Got into the bootloader, good!\n",FALSE);

	}
	result = prepare_for_upload(port_fd);
	if (!result)
	{
		output("Failure getting ECU into a state to accept the new firmware\n",FALSE);
		exit (-1);
	}
	upload_firmware(port_fd,file_fd);
	output("Firmware upload completed...\n",FALSE);
	reboot_ecu(port_fd);
	output("ECU reboot complete\n",FALSE);
	close_port(port_fd);
	
	return (0) ;
}

void verify_args(gint argc, gchar **argv)
{
	/* Invalid arg count, abort */
	if (argc != 3)
		usage_and_exit(g_strdup("Invalid number of arguments!"));

	
	if (!g_file_test(argv[1], G_FILE_TEST_EXISTS))
		usage_and_exit(g_strdup_printf("Port \"%s\" does NOT exist...",argv[1]));

	if (!g_file_test(argv[2], G_FILE_TEST_IS_REGULAR))
		usage_and_exit(g_strdup_printf("Filename \"%s\" does NOT exist...",argv[2]));
}

void usage_and_exit(gchar * msg)
{
	printf("\nERROR!!!\n - %s\n",msg);
	g_free(msg);
	printf("\nINVALID USAGE\n - msloader /path/to/port /path/to/.s19\n\n");
	exit (-1);
}


void output(gchar *msg, gboolean free_it)
{
	printf("%s",msg);
	if (free_it)
		g_free(msg);
}
