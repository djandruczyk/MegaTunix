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
#include <gui_handlers.h>
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

extern gboolean dualtable;
extern gboolean connected;
extern GtkWidget *ms_ecu_revision_entry;
extern GtkTextBuffer *textbuffer;
extern GtkWidget *interr_view;
extern struct Serial_Params *serial_params;
extern struct DynamicEntries entries;
gboolean interrogated = FALSE;
gfloat ecu_version;
static struct 
{
	gint page;		/* ms page in memory where it resides */
	gchar *cmd_string;	/* command to get the data */
	gchar *cmd_desc;	/* command description */
	gint cmd_len;		/* Command length in chars to send */
	gint count;		/* number of bytes returned */
	char *buffer;		/* buffer to store returned data... */
} commands[] = {
	{ 0,"A", "Runtime Vars", 1, 0,NULL },
	{ 0,"C", "MS Clock", 1, 0,NULL },
	{ 0,"Q", "MS Revision", 1, 0,NULL },
	{ 0,"V", "Ve/Constants page0", 1, 0,NULL },
	{ 1,"V", "Ve/Constants page1", 1, 0,NULL },
	{ 0,"S", "Signature Echo", 1, 0,NULL },
	{ 0,"I", "Ignition Vars", 1, 0,NULL },
	{ 0,"?", "Extended Version", 1, 0,NULL },
	{ 0,"F0", "Memory readback 1st 256 bytes", 2, 0,NULL },
	{ 0,"F1", "Memory readback 2nd 256 bytes", 2, 0,NULL }
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
	gint tmp = 0;
	gint len = 0;
	gint total = 0;
	gint q_index = 0;
	gint s_index = 0;
	gint quest_index = 0;
	gint table0_index = 0;
	gint table1_index = 0;
	gint v0_bytes = 0;
	gint v1_bytes = 0;
	gint s_bytes = 0;
	gint i_bytes = 0;
	gint q_bytes = 0;
	gint quest_bytes = 0;
	gchar *tmpbuf;
	extern gboolean raw_reader_running;
	gboolean restart_reader = FALSE;
	gchar *string;
	gboolean con_status = FALSE;
	gint tests_to_run = sizeof(commands)/sizeof(commands[0]);
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	if (!connected)
	{
		con_status = check_ecu_comms(NULL,NULL);
		if (con_status == FALSE)
		{
			interrogated = FALSE;
			no_ms_connection();
			g_static_mutex_unlock(&mutex);
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

	tcflush(serial_params->fd, TCIOFLUSH);

	for (i=0;i<tests_to_run;i++)
	{
		count = 0;
		total = 0;
		/* flush buffer to known state.. */
		memset (buf,0,size);

		ptr = buf;
		len = commands[i].cmd_len;
		/* set page */
		set_ms_page(commands[i].page);
		string = g_strdup(commands[i].cmd_string);
		res = write(serial_params->fd,string,len);
		if (res != len)
			fprintf(stderr,__FILE__": Error writing data to the ECU\n");
		res = poll (&ufds,1,25);
		if (res)
		{	
			//printf("command %s returned ",string);
			while (poll(&ufds,1,25))
			{
				total += count = read(serial_params->fd,ptr+total,64);
				//printf("count %i, total %i\n",count,total);
			}
			//printf("%i bytes\n",total);

			ptr = buf;
			/* copy data from tmp buffer to struct pointer */
			commands[i].buffer = g_memdup(buf,total);
		}
		g_free(string);
		commands[i].count = total;
	}
	/* Reset page to 0 just to be 100% sure... */
	set_ms_page(0);
	/* flush serial port */
	tcflush(serial_params->fd, TCIOFLUSH);

	for (i=0;i<tests_to_run;i++)
	{
		/* Per command section */
		if ((strstr(commands[i].cmd_string,"V") && (commands[i].page == 0)))
		{
			serial_params->table0_size = commands[i].count;
			v0_bytes = commands[i].count;
			table0_index = i;
		}
		else if ((strstr(commands[i].cmd_string,"V") && (commands[i].page == 1)))
		{
			serial_params->table1_size = commands[i].count;
			v1_bytes = commands[i].count;
			table1_index = i;
		}
		else if (strstr(commands[i].cmd_string,"S"))
		{
			s_bytes = commands[i].count;
			s_index = i;
		}
		else if (strstr(commands[i].cmd_string,"?"))
		{
			quest_bytes = commands[i].count;
			quest_index = i;
		}
		else if (strstr(commands[i].cmd_string,"Q"))
		{
			q_bytes = commands[i].count;
			q_index = i;
		}
		else if (strstr(commands[i].cmd_string,"A"))
		{
			serial_params->rtvars_size = commands[i].count;
			/* if A command doesn't come back with 22 something
			 * went wrong...  re-interrogate..
			 */
			if (commands[i].count != 22)
				fprintf(stderr,__FILE__": Interrogate returned an invalid response to the \"A\" Command (runtime variables), which should always return 22 bytes.  We got %i bytes instead.  Seems like the MS is in an undefined state, powercycle the ECU and re-interrogate.\n\n",commands[i].count);
		}	
		else if (strstr(commands[i].cmd_string,"I"))
			i_bytes = commands[i].count;
	}
	if (v0_bytes == 128) /* dualtable potential */
	{
		res = memcmp(	commands[table0_index].buffer, 
				commands[table1_index].buffer,128);
		if (res != 0)
			set_dualtable_mode(TRUE);
	}
	else
		commands[table1_index].count = 0;
		

	for (i=0;i<tests_to_run;i++)
	{
		if (commands[i].count > 0)
		{
			tmpbuf = g_strdup_printf("Command \"%s\" (%s), returned %i bytes\n",
					commands[i].cmd_string, 
					commands[i].cmd_desc, 
					commands[i].count);
			/* Store counts for VE/realtime readback... */

			update_logbar(interr_view,NULL,tmpbuf,FALSE);
			g_free(tmpbuf);
		}
		else
		{
			tmpbuf = g_strdup_printf("Command \"%s\" (%s), isn't supported...\n",
					commands[i].cmd_string,
					commands[i].cmd_desc);
			update_logbar(interr_view,NULL,tmpbuf,FALSE);
			g_free(tmpbuf);
		}
	}
	
	if (q_bytes > 0) /* ECU reponded to basic version query */
	{
		memcpy(&tmp,commands[q_index].buffer,commands[q_index].count);
		tmpbuf = g_strdup_printf("%.1f",((float)tmp/10.0));
		gtk_entry_set_text(GTK_ENTRY(entries.ecu_revision_entry),tmpbuf);
		g_free(tmpbuf);
	}
	else
	{
		tmpbuf = g_strdup("");
		gtk_entry_set_text(GTK_ENTRY(entries.ecu_revision_entry),tmpbuf);
		g_free(tmpbuf);
	}

	if (s_bytes > 0) /* ECU reponded to basic version query */
	{
		tmpbuf = g_strdup(commands[s_index].buffer);
		gtk_entry_set_text(GTK_ENTRY(entries.ecu_signature_entry),tmpbuf);
		g_free(tmpbuf);
	}
	else
	{
		tmpbuf = g_strdup("");
		gtk_entry_set_text(GTK_ENTRY(entries.ecu_signature_entry),tmpbuf);
		g_free(tmpbuf);
	}
	if (quest_bytes > 0) /* ECU reponded to basic version query */
	{
		tmpbuf = g_strdup(commands[quest_index].buffer);
		gtk_entry_set_text(GTK_ENTRY(entries.extended_revision_entry),tmpbuf);
		g_free(tmpbuf);
	}
	else
	{
		tmpbuf = g_strdup("");
		gtk_entry_set_text(GTK_ENTRY(entries.extended_revision_entry),tmpbuf);
		g_free(tmpbuf);
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

	g_static_mutex_unlock(&mutex);
	return;
}
