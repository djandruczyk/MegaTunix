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
#include <structures.h>
#include <globals.h>
#include <errno.h>
#include <interrogate.h>

extern GtkWidget *ms_ecu_revision_entry;
extern GtkTextBuffer *textbuffer;
gfloat ecu_version;
const gchar *cmd_chars[] = {"A","C","Q","V","S","I"};
struct Cmd_Results
{
	gchar *cmd_string;
	gint count;
};

/* The Various MegaSquirt variants that MegaTunix attempts to support
 * have one major problem.  Inconsistent version numbering.  Several
 * variants use the same number, We attempt to instead query the various
 * readback commands to determine how much data they return,  With this
 * we can make an educated guess and help guide the user to make the final
 * selection on the General Tab.
 * Actual results of querying the variosu MS codes are below.  The list will
 * be updated as the versions out there preogress.
 *
 * "A" = Relatime Variables return command
 * "C" = Echo back Secl (MS 1sec resolution clock)
 * "Q" = Echo back embedded version number
 * "V" = Return VEtable and Constants
 * "I" = Return Ignition table (Spark variants only)
 *
 *                 Readback initiator commands	
 *                "A" "C" "Q" "S" "V" "I"
 * B&G 2.x         22   1   1   0 125   0
 * DualTable 090   22   1   1   0 128   0
 * DualTable 099b  22   1   1   0 128   0
 * DualTable 100   22   1   1   0 128   0
 * DualTable 101   22   1   1  18 128   0
 * DualTable 102   22   1   1  19 128   0
 * Sqrtnspark 2.02 22   1   1   0 125  83
 * SqrtnSpark 3.0  22   1   1   0 125  95
 * SqrtnEDIS 0.108 22   1   1   0 125  83
 */

void interrogate_ecu()
{
	/* As of 10/26/2003 several MegaSquirt variants have arrived, the 
	 * major ones being the DualTable Code, MegaSquirtnSpark, and
	 * MegaSquirtnEDIS.  MegaTunix attempts to talk to all of them
	 * and it is this functions job to try and determine which unit 
	 * we are talking to.  The Std Version number query "Q" cannot be 
	 * relied upon as severl of the forks use the same damn number.
	 * So we use the approach of querying which commands respond to 
	 * try and get as close as possible.
	 */
	struct pollfd ufds;
	gint size = 1024;
	char buf[size];
	gint res = 0;
	gint count = 0;
	gint tmp = 0;
	gint i = 0;
	gint len = 0;
	gint v_bytes = 0;
	gint s_bytes = 0;
	gint i_bytes = 0;
	gchar *tmpbuf;
	GtkTextIter iter;
	gint tests_to_run = sizeof(cmd_chars)/sizeof(gchar *);
	struct Cmd_Results cmd_results[tests_to_run]; 

	ufds.fd = serial_params.fd;
	ufds.events = POLLIN;

	tmp = serial_params.newtio.c_cc[VMIN];
	serial_params.newtio.c_cc[VMIN]     = 1; /*wait for 1 char */
	tcflush(serial_params.fd, TCIFLUSH);
	tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);

	for (i=0;i<tests_to_run;i++)
	{
		count = 0;
		cmd_results[i].cmd_string = g_strdup(cmd_chars[i]);
		len = strlen(cmd_results[i].cmd_string);
		res = write(serial_params.fd,cmd_chars[i],len);
		res = poll (&ufds,1,serial_params.poll_timeout);
		if (res)
		{	
			while (poll(&ufds,1,serial_params.poll_timeout))
				count += read(serial_params.fd,&buf,64);
		}
		cmd_results[i].count = count;
	}

//	printf("\n");
	gtk_text_buffer_get_iter_at_offset (textbuffer, &iter, 0);
	gtk_text_buffer_create_tag(textbuffer,"red_foreground","foreground",
				"red", NULL);
	for (i=0;i<tests_to_run;i++)
	{
		if (cmd_results[i].count > 0)
		{
//			printf("Command %s, returned %i bytes\n",cmd_results[i].cmd_string, cmd_results[i].count);
			tmpbuf = g_strdup_printf("Command %s, returned %i bytes\n",cmd_results[i].cmd_string, cmd_results[i].count);
			gtk_text_buffer_insert(textbuffer,&iter,tmpbuf,-1);
			g_free(tmpbuf);
		}
		else
		{
//			printf("Command %s is not supported by this ECU\n",cmd_results[i].cmd_string);
			tmpbuf = g_strdup_printf("Command %s is not supported by this ECU\n",cmd_results[i].cmd_string);
			gtk_text_buffer_insert(textbuffer,&iter,tmpbuf,-1);
			g_free(tmpbuf);
		}
	}

	/* flush serial port... */
	serial_params.newtio.c_cc[VMIN]     = tmp; /*restore original*/
	tcflush(serial_params.fd, TCIFLUSH);
	tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);


	for(i=0;i<tests_to_run;i++)
	{
		if (strcmp(cmd_results[i].cmd_string,"V")== 0)
			v_bytes = cmd_results[i].count;
		if (strcmp(cmd_results[i].cmd_string,"S")== 0)
			s_bytes = cmd_results[i].count;
		if (strcmp(cmd_results[i].cmd_string,"I")== 0)
			i_bytes = cmd_results[i].count;
	}
	if (v_bytes > 125)
	{
//		printf("Code is dualtable version ");
		gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,"Code is DualTable version: ",-1,"red_foreground",NULL);
		if (s_bytes == 0)
		{
//			printf("0.90, 0.99b, or 1.00\n");
			gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,"0.90, 0.99b, or 1.00\n",-1,"red_foreground",NULL);
		}
		if (s_bytes == 18)
		{
//			printf("1.01\n");
			gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,"1.01\n",-1,"red_foreground",NULL);
		}
		if (s_bytes == 19)
		{
//			printf("1.02\n");
			gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,"1.02\n",-1,"red_foreground",NULL);
		}
	}
	else
	{
		switch (i_bytes)
		{
			case 0:
				gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,"Code is Standard B&G 2.x code\n", -1,"red_foreground",NULL);
//				printf("Code is Standard B&G 2.x code\n");
				break;
			case 83:
				gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,"Code is SquirtnSpark 2.02 or SquirtnEDIS 0.108\n", -1,"red_foreground",NULL);
//				printf("Code is SquirtnSpark 2.02 OR SquirtnEDIS 0.108\n");
				break;
			case 95:
				gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,"Code is SquirtnSpark 3.0\n", -1,"red_foreground",NULL);
//				printf("Code is SquirtnSpark 3.0\n");
				break;
			default:
				gtk_text_buffer_insert_with_tags_by_name(textbuffer,&iter,"Code is not recognized\n Contact author!!!\n", -1,"red_foreground",NULL);
//				printf("i_bytes is something new, contact author\n");
				break;
		}
	}
	
	return;
}

