/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <defines.h>
#include <protos.h>
#include <structures.h>
#include <globals.h>
#include <errno.h>

extern GtkWidget *ms_ecu_revision_entry;
gfloat ecu_version;


void interrogate_ecu()
{
	/* As of 10/26/2003 several MegaSquirt variants have arrived, the 
	 * major ones being the DualTable Code, MegaSquirtnSpark, and
	 * MegaSquirtnEDIS.  MegaTunix attempts to talk to all of them
	 * and it is this functions job to try and determine which unit 
	 * we are talking to.  The Std Version number query "Q" cannot be 
	 * relied upon as severl of the forks use the same friggin number.
	 * So We use the approach of querying which commands respond to 
	 * try and get as close as possible.
	 */
	struct pollfd ufds;
	gint size = 1024;
	char buf[size];
	const gchar *cmd_chars[] = {"A","C","Q","V","S","I"};
	gint tests_to_run = 0;
	gint res = 0;
	gint count = 0;
	gint tmp = 0;
	gint i = 0;

	printf("entered interrogate_ecu()\n");
	/*                 Readback initiator commands	
	 *                "A" "C" "Q" "S" "V" "I"
	 * B&G 2.x         22   1   1   0 125   0
	 * DualTable 090   22   1   1   0 128   0
	 * DualTable 099b  22   1   1   0 128   0
	 * DualTable 099b  22   1   1   0 128   0
	 * DualTable 100   22   1   1   0 128   0
	 * DualTable 101   22   1   1  18 128   0
	 * DualTable 102   22   1   1  19 128   0
	 * Sqrtnspark 2.02 22   1   1   0 125  83
	 * SqrtnSpark 3.0  22   1   1   0 125  95
	 * SqrtnEDIS 0.108 22   1   1   0 125  83
	 */
	ufds.fd = serial_params.fd;
	ufds.events = POLLIN;

	tmp = serial_params.newtio.c_cc[VMIN];
	serial_params.newtio.c_cc[VMIN]     = 1; /*wait for 1 char */
	tcflush(serial_params.fd, TCIFLUSH);
	tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);


	tests_to_run = sizeof(cmd_chars)/sizeof(gchar *);

	for (i=0;i<tests_to_run;i++)
	{
		count = 0;
		res = write(serial_params.fd,cmd_chars[i],1);
		res = poll (&ufds,1,serial_params.poll_timeout);
		if (res)
		{	
			while (poll(&ufds,1,serial_params.poll_timeout))
				count += read(serial_params.fd,&buf,1);
		}
		else
			printf("Failed to respond to \"%s\" command\n",cmd_chars[i]);

		printf("\"%s\" command returned %i bytes\n",cmd_chars[i],count);
	}

	/* flush serial port... */
	serial_params.newtio.c_cc[VMIN]     = tmp; /*restore original*/
	tcflush(serial_params.fd, TCIFLUSH);
	tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);


	return;
}


