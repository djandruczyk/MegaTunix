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
#include <defines.h>
#include <errno.h>
#include <globals.h>
#include <interrogate.h>
#include <notifications.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <structures.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <threads.h>
#include <unistd.h>

extern gboolean connected;
extern GtkWidget *ms_ecu_revision_entry;
extern GtkTextBuffer *textbuffer;
extern GtkWidget *interr_view;
extern struct Serial_Params *serial_params;
gboolean interrogated = FALSE;
gfloat ecu_version;
gboolean dualtable;

static struct 
{
	gchar *cmd_string;
	gchar *cmd_desc;
	gint cmd_len;
	gint count;
	unsigned char *buffer;
} commands[] = {
	{ "A", "Runtime Vars", 1, 0,NULL },
	{ "C", "MS Clock", 1, 0,NULL },
	{ "Q", "MS Revision", 1, 0,NULL },
	{ "V", "Ve/Constants", 1, 0,NULL },
	{ "F0", "Memory readback first 256 bytes ", 2, 0,NULL },
	{ "P1V", "Ve/Constants (page1)", 3, 0,NULL },
	{ "F1", "Memory readback second 256 bytes ", 2, 0,NULL },
	{ "S", "Signature Echo", 1, 0,NULL },
	{ "I", "Ignition Vars", 1, 0,NULL },
	{ "?", "Extended Version", 1, 0,NULL }
};
/*
 * The Various MegaSquirt variants that MegaTunix attempts to support
 * have one major problem.  Inconsistent version numbering.  Several
 * variants use the same number, We attempt to instead query the various
 * readback commands to determine how much data they return,  With this
 * we can make an educated guess and help guide the user to make the final
 * selection on the General Tab.
 * Actual results of querying the variosu MS codes are below.  The list will
 * be updated as the versions out there progress.
 *
 * "A" = Realtime Variables return command
 * "C" = Echo back Secl (MS 1sec resolution clock)
 * "Q" = Echo back embedded version number
 * "V" = Return VEtable and Constants
 * "S" = Return something else???
 * "I" = Return Ignition table (Spark variants only)
 * "?" = Return textual identification (new as of MSnEDIS 3.0.5)
 *
 *                 Readback initiator commands	
 *                "A" "C" "Q" "S" "V" "I" "?"
 * B&G 2.x         22   1   1   0 125   0   0
 * DualTable 090   22   1   1   0 128   0   0
 * DualTable 099b  22   1   1   0 128   0   0
 * DualTable 100   22   1   1   0 128   0   0
 * DualTable 101   22   1   1  18 128   0   0
 * DualTable 102   22   1   1  19 128   0   0
 * Sqrtnspark 2.02 22   1   1   0 125  83   0
 * SqrtnSpark 3.0  22   1   1   0 125  95   0
 * SqrtnEDIS 0.108 22   1   1   0 125  83   0
 * SqrtnEDIS 3.0.5 22   1   1   0 125  83  32
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
	unsigned char buf[size];
	unsigned char *ptr = buf;
	gint res = 0;
	gint count = 0;
	gint i = 0;
	gint j = 0;
	gint len = 0;
	gint total = 0;
	gint table0_index = 0;
	gint table1_index = 0;
	gint v0_bytes = 0;
	gint v1_bytes = 0;
	gint s_bytes = 0;
	gint i_bytes = 0;
	gint quest_bytes = 0;
	gchar *tmpbuf;
	extern gboolean raw_reader_running;
	gboolean restart_reader = FALSE;
	gchar *string;
	gboolean con_status = FALSE;
	gint tests_to_run = sizeof(commands)/sizeof(commands[0]);

	if (!connected)
	{
		con_status = check_ecu_comms(NULL,NULL);
		if (con_status == FALSE)
		{
			interrogated = FALSE;
			no_ms_connection();
			return;
		}
	}
	if (raw_reader_running)
	{
		restart_reader = TRUE;
		stop_serial_thread();
	}


	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;

	tcflush(serial_params->fd, TCIFLUSH);

	for (i=0;i<tests_to_run;i++)
	{
		count = 0;
		total = 0;
		/* flush buffer to known state.. */
		memset (buf,0,size);

		ptr = buf;
		string = g_strdup(commands[i].cmd_string);
		len = commands[i].cmd_len;
		res = write(serial_params->fd,string,len);
		if (res != len)
			fprintf(stderr,__FILE__": Error writing data to the ECU\n");
		res = poll (&ufds,1,10);
		if (res)
		{	
			printf("command %s returned ",string);
			while (poll(&ufds,1,10))
			{
				total += count = read(serial_params->fd,ptr+total,64);
				//printf("count %i, total %i\n",count,total);
			}
			printf("%i bytes\n",total);
			
			for (j=0;j<total;j++)
				printf("buffer [%i]= %i\n",j,buf[j]);
			
			ptr = buf;
			/* copy data from tmp buffer to struct pointer */
			commands[i].buffer = g_memdup(buf,total);
		}
		g_free(string);
		commands[i].count = total;
	}
//	tcflush(serial_params->fd, TCIFLUSH);
//	tcflush(serial_params->fd, TCIFLUSH);

	res = memcmp(commands[3].buffer, commands[5].buffer,128);
	printf("result of comparing 3 to 5 is %i\n",res);
	for (i=0;i<128;i++)
	{
		printf("buffer 3[%i]= %i, 5[%i]= %i\n",i,commands[3].buffer[i],i,commands[5].buffer[i]);
		
	}

	for (i=0;i<tests_to_run;i++)
	{
		/* Per command section */
		if (strstr(commands[i].cmd_string,"P1V"))
		{
			serial_params->table1_size = commands[i].count;
			v1_bytes = commands[i].count;
			table1_index = i;
		}
		else if (strstr(commands[i].cmd_string,"V"))
		{
			serial_params->table0_size = commands[i].count;
			v0_bytes = commands[i].count;
			table0_index = i;
		}
		else if (strstr(commands[i].cmd_string,"A"))
			serial_params->rtvars_size = commands[i].count;
		else if (strstr(commands[i].cmd_string,"S"))
			s_bytes = commands[i].count;
		else if (strstr(commands[i].cmd_string,"I"))
			i_bytes = commands[i].count;
		else if (strstr(commands[i].cmd_string,"?"))
			quest_bytes = commands[i].count;

		if ((v1_bytes == v0_bytes ) && (v0_bytes == 125))
			commands[table1_index].count = 0;

		if (commands[i].count > 0)
		{
			tmpbuf = g_strdup_printf(
					"Command %s, returned %i bytes\n",
					commands[i].cmd_string, 
					commands[i].count);
			/* Store counts for VE/realtime readback... */
				
			update_logbar(interr_view,NULL,tmpbuf,FALSE);
			g_free(tmpbuf);
		}
		else
		{
			tmpbuf = g_strdup_printf(
					"Command %s isn't supported...\n",
					commands[i].cmd_string);
			update_logbar(interr_view,NULL,tmpbuf,FALSE);
			g_free(tmpbuf);
		}

	}
	if (v0_bytes > 125)
	{
		update_logbar(interr_view,"warning","Code is DualTable version: ",FALSE);
		if (s_bytes == 0)
			update_logbar(interr_view,"warning","0.90, 0.99b, or 1.00\n",FALSE);
		if (s_bytes == 18)
			update_logbar(interr_view,"warning","1.01\n",FALSE);
		if (s_bytes == 19)
			update_logbar(interr_view,"warning","1.02\n",FALSE);
	}
	else
	{
		if (quest_bytes > 0)
			update_logbar(interr_view,"warning","Code is MegaSquirtnEDIS v3.05 code\n",FALSE);

					
		else
		{
			switch (i_bytes)
			{
				case 0:
					update_logbar(interr_view,"warning","Code is Standard B&G 2.x code\n",FALSE);
					break;
				case 83:
					update_logbar(interr_view,"warning","Code is SquirtnSpark 2.02 or SquirtnEDIS 0.108\n",FALSE);
					break;
				case 95:
					update_logbar(interr_view,"warning","Code is SquirtnSpark 3.0\n",FALSE);
					break;
				default:
					update_logbar(interr_view,"warning","Code is not recognized\n Contact author!!!\n",FALSE);
					break;
			}
		}
	}

	for (i=0;i<tests_to_run;i++)
	{
		if (commands[i].buffer != NULL)
		{
			g_free(commands[i].buffer);
			commands[i].buffer = NULL;
		}
	}

	interrogated = TRUE;
	if (restart_reader)
		start_serial_thread();
	return;
}
