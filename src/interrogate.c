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
#include <gui_handlers.h>
#include <interrogate.h>
#include <notifications.h>
#include <serialio.h>
#include <stdio.h>
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
extern struct DynamicEntries entries;
gboolean interrogated = FALSE;

static struct Canidate
{
	gint bytes[10];		/* byte count for each of the 9 test cmds */
	unsigned char *v0_data;	/* V0 data */
	unsigned char *v1_data;	/* V1 data */
	gchar *sig_str;		/* Signature string to search for */
	gchar *quest_str;	/* Ext Version string to search for */
	gint ver_num;		/* Version number to search for */
	gchar *firmware_name;	/* Name of this firmware */
	gboolean dt_cap;	/* Dualtable capable firmware */
	gboolean ign_cap;	/* Ignition variant */
	gboolean iac_cap;		/* Extended IAC ability... */
} canidates[] = 
{
	{ {22,1,1,125,125,0,0,0,0,0},NULL,NULL,NULL,NULL,20,
			"Standard B&G (2.0-3.01)",FALSE,FALSE,FALSE},
	{ {22,1,1,128,128,0,0,0,255,255}, NULL,NULL,NULL,NULL,1,
			"Dualtable 0.90-1.0",TRUE,FALSE,FALSE},
	{ {22,1,1,128,128,18,0,0,255,255},NULL,NULL,"v.1.01",NULL,1,
			"Dualtable 1.01",TRUE,FALSE,FALSE},
	{ {22,1,1,128,128,19,0,0,255,255},NULL,NULL,"v.1.02",NULL,1,
			"Dualtable 1.02",TRUE,FALSE,TRUE},
	{ {22,1,1,125,125,31,0,0,0,0},NULL,NULL,"GM-IAC",NULL,29,
			"MS-2.9 GM-IAC",FALSE,FALSE,TRUE},
	{ {22,1,1,125,125,0,0,81,0,0},NULL,NULL,NULL,NULL,20,
			"SquirtnSpark 2.02 or SquirtnEdis 0.108",
			FALSE,TRUE,FALSE},
	{ {22,1,1,125,125,0,0,95,0,0},NULL,NULL,NULL,NULL,30,
			"SquirtnSpark 3.0",FALSE,TRUE,FALSE},
	{ {22,1,1,125,125,0,32,95,0,0},NULL,NULL,NULL,"EDIS v3.005",30,
			"MegaSquirtnEDIS 3.05",FALSE,TRUE,FALSE}
};

static struct 
{
	gint page;		/* ms page in memory where it resides */
	gchar *cmd_string;	/* command to get the data */
	gchar *cmd_desc;	/* command description */
	gint cmd_len;		/* Command length in chars to send */
	gboolean combine;	/* combine page with cmd as cmd_name */
} cmds[] = {
	{ 0,"A", "Runtime Vars", 1, FALSE },
	{ 0,"C", "MS Clock", 1, FALSE },
	{ 0,"Q", "MS Revision", 1, FALSE },
	{ 0,"V", "Ve/Constants page0", 1, TRUE },
	{ 1,"V", "Ve/Constants page1", 1, TRUE },
	{ 0,"S", "Signature Echo", 1, FALSE },
	{ 0,"?", "Extended Version", 1, FALSE },
	{ 0,"I", "Ignition Vars", 1, FALSE },
	{ 0,"F0", "Memory readback 1st 256 bytes", 2, FALSE },
	{ 0,"F1", "Memory readback 2nd 256 bytes", 2, FALSE }
};

typedef enum
{
	CMD_A,
	CMD_C,
	CMD_Q,
	CMD_V0,
	CMD_V1,
	CMD_S,
	CMD_QUEST,
	CMD_I,
	CMD_F0,
	CMD_F1
}TestCmds;

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
	gint len = 0;
	gint total = 0;
	extern gboolean raw_reader_running;
	gboolean restart_reader = FALSE;
	gchar *string;
	gboolean con_status = FALSE;
	gint tests_to_run = sizeof(cmds)/sizeof(cmds[0]);
	struct Canidate *canidate;
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

	/* Allocate hash table to store the results for each test... */
	canidate = g_malloc0(sizeof(struct Canidate));
	

	/* Configure port for polled IO and flush I/O buffer */
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
		len = cmds[i].cmd_len;
		/* set page */
		set_ms_page(cmds[i].page);
		string = g_strdup(cmds[i].cmd_string);
		res = write(serial_params->fd,string,len);
		if (res != len)
			fprintf(stderr,__FILE__": Error writing data to the ECU\n");
		res = poll (&ufds,1,25);
		if (res)
		{	
			while (poll(&ufds,1,25))
			{
				total += count = read(serial_params->fd,ptr+total,64);
				//printf("count %i, total %i\n",count,total);
			}

			ptr = buf;
			switch (i)
			{
				case CMD_V0:
					canidate->v0_data = g_memdup(buf,total);
					break;
				case CMD_V1:
					canidate->v1_data = g_memdup(buf,total);
					break;
				case CMD_S:
					canidate->sig_str = g_memdup(buf,total);
					break;
				case CMD_QUEST:
					canidate->quest_str = g_memdup(buf,total);
					break;
				case CMD_Q:
					memcpy(&(canidate->ver_num),buf,total);
					break;
				default:
					break;
			}
			/* copy data from tmp buffer to struct pointer */
	
		}
		g_free(string);
		canidate->bytes[i] = total;
	}
	/* Reset page to 0 just to be 100% sure... */
	set_ms_page(0);
	/* flush serial port */
	tcflush(serial_params->fd, TCIOFLUSH);

	interrogated = TRUE;


	if (restart_reader)
		start_serial_thread();
	determine_ecu(canidate);	

	g_static_mutex_unlock(&mutex);
	return;
}
void determine_ecu(void *ptr)
{
	struct Canidate *canidate = (struct Canidate *)ptr;
	gint i = 0;
	gint j = 0;
	gint num_tests =  sizeof(cmds)/sizeof(cmds[0]);
	gint num_choices = sizeof(canidates)/sizeof(canidates[0]);
	gint passcount = 0;
	gint match = -1;
	gchar * tmpbuf;

	/* compare the canidate to all the choices.  As OF now we are ONLY
	 * comparing byte counts as that is enough to guarantee unique-ness
	 * future firmwares may use the same bytecount but MUST use either
	 * unique version numbers AND/OR unique extended signatures to be
	 * properly detected and handled...
	 */
	for (i=0;i<num_choices;i++)
	{
		passcount = 0;
		//printf("checking canidate %i\n",i);
		for (j=0;j<num_tests;j++)
		{
			//printf("checking test %i\n",j);
			if (canidate->bytes[j] != canidates[i].bytes[j])
			{
				//printf("fail\n");
				continue;
			}
			else
				passcount++;
				//printf("pass\n");
		}
		if (passcount == num_tests) /* All tests pass */
		{
			match = i;
		//	printf("found match to canidate %i\n",i);
			break;
		}
	}
	if (match != -1) // (we found one)
	{
		set_dualtable_mode(canidates[match].dt_cap);	
		set_ignition_mode(canidates[match].ign_cap);	
		set_iac_mode(canidates[match].iac_cap);	
		serial_params->table0_size = canidates[match].bytes[CMD_V0];
		serial_params->table1_size = canidates[match].bytes[CMD_V1];
		serial_params->rtvars_size = canidates[match].bytes[CMD_A];
		serial_params->ignvars_size = canidates[match].bytes[CMD_I];
	}
	/* Update the screen with the data... */
	for (i=0;i<num_tests;i++)
	{
		tmpbuf = g_strdup_printf("Command \"%s\" (%s), returned %i bytes\n",
				cmds[i].cmd_string, 
				cmds[i].cmd_desc, 
				canidate->bytes[i]);
		// Store counts for VE/realtime readback... 

		update_logbar(interr_view,NULL,tmpbuf,FALSE,FALSE);
		g_free(tmpbuf);
		if (i == CMD_Q)
		{
			if (canidate->bytes[i] == 0)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strdup_printf("%.1f",
						((float)canidate->ver_num/10.0));
			gtk_entry_set_text(GTK_ENTRY(entries.ecu_revision_entry)
					,tmpbuf);
			g_free(tmpbuf);
		}
		if (i == CMD_S)
		{
			if (canidate->bytes[i] == 0)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strdup(canidate->sig_str);
			gtk_entry_set_text(GTK_ENTRY(entries.ecu_signature_entry),tmpbuf);
			g_free(tmpbuf);
		}
		if (i == CMD_QUEST)
		{
			if (canidate->bytes[i] == 0)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strdup(canidate->quest_str);
			gtk_entry_set_text(GTK_ENTRY(entries.extended_revision_entry),tmpbuf);
			g_free(tmpbuf);
		}

	}
	tmpbuf = g_strdup_printf("Detected Firmware: %s\n",
			canidates[match].firmware_name);
	update_logbar(interr_view,"warning",tmpbuf,FALSE,FALSE);
	g_free(tmpbuf);
	
}


/*
{

	for (i=0;i<tests_to_run;i++)
	{
		// Per command section 
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
			// if A command doesn't come back with 22 something
			// went wrong...  re-interrogate..
			//
			if (commands[i].count != 22)
				fprintf(stderr,__FILE__": Interrogate returned an invalid response to the \"A\" Command (runtime variables), which should always return 22 bytes.  We got %i bytes instead.  Seems like the MS is in an undefined state, powercycle the ECU and re-interrogate.\n\n",commands[i].count);
		}	
		else if (strstr(commands[i].cmd_string,"I"))
			i_bytes = commands[i].count;
	}
	if (v0_bytes == 128) // dualtable potential 
	{
		res = memcmp(	commands[table0_index].buffer, 
				commands[table1_index].buffer,128);
		if (res != 0)
			set_dualtable_mode(TRUE);
		else
			set_dualtable_mode(FALSE);
	}
	else
	{
		commands[table1_index].count = 0;
		set_dualtable_mode(FALSE);
	}
		

	for (i=0;i<tests_to_run;i++)
	{
		if (commands[i].count > 0)
		{
			tmpbuf = g_strdup_printf("Command \"%s\" (%s), returned %i bytes\n",
					commands[i].cmd_string, 
					commands[i].cmd_desc, 
					commands[i].count);
			// Store counts for VE/realtime readback... 

			update_logbar(interr_view,NULL,tmpbuf,FALSE,FALSE);
			g_free(tmpbuf);
		}
		else
		{
			tmpbuf = g_strdup_printf("Command \"%s\" (%s), isn't supported...\n",
					commands[i].cmd_string,
					commands[i].cmd_desc);
			update_logbar(interr_view,NULL,tmpbuf,FALSE,FALSE);
			g_free(tmpbuf);
		}
	}
	
	if (q_bytes > 0) // ECU reponded to basic version query 
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

	if (s_bytes > 0) // ECU reponded to basic version query 
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
	if (quest_bytes > 0) // ECU reponded to basic version query 
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
		update_logbar(interr_view,"warning","Code is DualTable version: ",FALSE,FALSE);
		if (s_bytes == 0)
			update_logbar(interr_view,"warning","0.90, 0.99b, or 1.00\n",FALSE,FALSE);
		if (s_bytes == 18)
			update_logbar(interr_view,"warning","1.01\n",FALSE,FALSE);
		if (s_bytes == 19)
			update_logbar(interr_view,"warning","1.02\n",FALSE,FALSE);
	}
	else
	{
		if (quest_bytes > 0)
			update_logbar(interr_view,"warning","Code is MegaSquirtnEDIS v3.05 code\n",FALSE,FALSE);


		else
		{
			switch (i_bytes)
			{
				case 0:
					update_logbar(interr_view,"warning","Code is Standard B&G 2.x code\n",FALSE,FALSE);
					break;
				case 83:
					update_logbar(interr_view,"warning","Code is SquirtnSpark 2.02 or SquirtnEDIS 0.108\n",FALSE,FALSE);
					break;
				case 95:
					update_logbar(interr_view,"warning","Code is SquirtnSpark 3.0\n",FALSE,FALSE);
					break;
				default:
					update_logbar(interr_view,"warning","Code is not recognized\n Contact author!!!\n",FALSE,FALSE);
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
*/
