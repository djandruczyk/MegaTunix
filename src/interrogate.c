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
static struct Result
{
	gchar *cmd_desc;	/* Command Description */
	gchar *recvd;		/* What came back... */
	gint recvd_len;		/* how many bytes came back */
	gint cmd_num;		/* command number maps to the command string */
};

static struct Canidate
{
	gint a_bytes;		/* how many bytes for "A" command */
	gint c_bytes;		/* how many bytes for "C" command */
	gint q_bytes;		/* how many bytes for "Q" command */
	gint v0_bytes;		/* how many bytes for "P0V" command */
	gint v1_bytes;		/* how many bytes for "P1V" command */
	unsigned char *v0_data;	/* V0 data */
	unsigned char *v1_data;	/* V1 data */
	gint s_bytes;		/* how many bytes for "S" command */
	gint quest_bytes;	/* how many bytes for "?" command */
	gint i_bytes;		/* how many bytes for "I" command */
	gint f0_bytes;		/* how many bytes for "F0" command */
	gint f1_bytes;		/* how many bytes for "F1" command */
	gchar *sig_str;		/* Signature string to search for */
	gchar *quest_str;	/* Ext Version string to search for */
	gint ver_num;		/* Version number to search for */
	gchar *firmware_name;	/* Name of this firmware */
	gboolean dt;		/* Dualtable capable firmware */
	gboolean ignition;	/* Ignition variant */
	gboolean iac;		/* Extended IAC ability... */
} canidates[] = 
{
	{22,1,1,125,125,"","",0,0,0,0,0,"","",20,
			"Standard B&G (2.0-3.01)",FALSE,FALSE,FALSE},
	{22,1,1,128,128,"","",0,0,0,255,255,"","",1,
			"Dualtable 0.90-1.0",TRUE,FALSE,FALSE},
	{22,1,1,128,128,"","",18,0,0,255,255,"v.1.01","",1,
			"Dualtable 1.01",TRUE,FALSE,FALSE},
	{22,1,1,128,128,"","",19,0,0,255,255,"v.1.02","",1,
			"Dualtable 1.02",TRUE,FALSE,TRUE},
	{22,1,1,125,125,"","",31,0,0,0,0,"GM-IAC","",29,
			"MS-2.9 GM-IAC",FALSE,FALSE,TRUE},
	{22,1,1,125,125,"","",0,0,81,0,0,"","",20,
			"SquirtnSpark 2.02 or SquirtnEdis 0.108",
			FALSE,TRUE,FALSE},
	{22,1,1,125,125,"","",0,0,95,0,0,"","",30,
			"SquirtnSpark 3.0",FALSE,TRUE,FALSE},
	{22,1,1,125,125,"","",0,32,95,0,0,"","EDIS v3.005",30,
			"MegaSquirtnEDIS 3.05",FALSE,TRUE,FALSE}
};

static struct 
{
	gint page;		/* ms page in memory where it resides */
	gchar *cmd_string;	/* command to get the data */
	gchar *cmd_desc;	/* command description */
	gint cmd_len;		/* Command length in chars to send */
	gboolean combine;	/* combine page with cmd as cmd_name */
} commands[] = {
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
	gchar * key;
	extern gboolean raw_reader_running;
	gboolean restart_reader = FALSE;
	gchar *string;
	gboolean con_status = FALSE;
	GHashTable *command_results = NULL;
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

	/* Allocate hash table to store the results for each test... */
	command_results = g_hash_table_new(NULL,NULL);	

	/* Configure port for polled IO and flush I/O buffer */
	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;
	tcflush(serial_params->fd, TCIOFLUSH);

	for (i=0;i<tests_to_run;i++)
	{
		count = 0;
		total = 0;
		struct Result *result;
		/* flush buffer to known state.. */
		memset (buf,0,size);
		result = g_malloc0(sizeof(struct Result));
		result->cmd_desc = g_strdup(commands[i].cmd_desc);
		result->cmd_num = i;	/* command number */

		ptr = buf;
		len = commands[i].cmd_len;
		/* set page */
		set_ms_page(commands[i].page);
		string = g_strdup(commands[i].cmd_string);
		if (commands[i].combine)
			key = g_strdup_printf("%s%i",string,commands[i].page);
		else
			key = g_strdup(string);
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
			/* copy data from tmp buffer to struct pointer */
			result->recvd = g_memdup(buf,total);
	
		}
		g_free(string);
		result->recvd_len = total;
		g_hash_table_insert(command_results,g_strdup(key),
				(gpointer)result);
		g_free(key);
	}
	/* Reset page to 0 just to be 100% sure... */
	set_ms_page(0);
	/* flush serial port */
	tcflush(serial_params->fd, TCIOFLUSH);

	printf("all tests complete, calling determinator\n");
	interrogated = TRUE;

	g_static_mutex_unlock(&mutex);

	printf("mutex unlocked\n");
	if (restart_reader)
		start_serial_thread();
	determine_ecu(command_results);	
	return;
}
void determine_ecu(void *ptr)
{
	struct Canidate *canidate = NULL;
	canidate = g_malloc(sizeof(struct Canidate));
	gint i = 0;
	gint num_choices = sizeof(canidates)/sizeof(canidates[0]);
	gint matching_index = -1;

	GHashTable *command_results = (GHashTable *)ptr;
	/* Extracts all the data into a "Canidate" structure" for comparison
	* against the choices[] array
	*/
	g_hash_table_foreach(command_results,extract_data,canidate);

	for (i=0;i<num_choices;i++)
	{
		printf("checking canidate %i\n",i);
		if (canidate->a_bytes != canidates[i].a_bytes)
			continue;
		if (canidate->c_bytes != canidates[i].c_bytes)
			continue;
		if (canidate->q_bytes != canidates[i].q_bytes)
			continue;
		if (canidate->s_bytes != canidates[i].s_bytes)
			continue;
/*		if (strstr(canidate->sig_str,canidates[i].sig_str)==NULL)
			continue;
		if (strstr(canidate->quest_str,canidates[i].quest_str)==NULL)
			continue;
*/
		if (canidate->quest_bytes != canidates[i].quest_bytes)
			continue;
		if (canidate->v0_bytes != canidates[i].v0_bytes)
			continue;
		if (canidate->v1_bytes != canidates[i].v1_bytes)
			continue;
		if (canidate->i_bytes != canidates[i].i_bytes)
			continue;
		if (canidate->f0_bytes != canidates[i].f0_bytes)
			continue;
		if (canidate->f1_bytes != canidates[i].f1_bytes)
			continue;
		else
			matching_index = i;
		break;
	}
	
}

void extract_data (gpointer key_ptr, gpointer value, gpointer data)
{
	struct Result *result = (struct Result *)value;
	struct Canidate *canidate = (struct Canidate *)data;

	switch (result->cmd_num)
	{
		case 0:
			canidate->a_bytes = result->recvd_len;
			break;
		case 1:
			canidate->c_bytes = result->recvd_len;
			break;
		case 2:
			canidate->q_bytes = result->recvd_len;
			break;
		case 3:
			canidate->v0_bytes = result->recvd_len;
			canidate->v0_data = g_memdup(result->recvd,
					result->recvd_len);
			break;
		case 4:
			canidate->v1_bytes = result->recvd_len;
			canidate->v1_data = g_memdup(result->recvd,
					result->recvd_len);
			break;
		case 5:
			canidate->s_bytes = result->recvd_len;
			canidate->sig_str = g_memdup(result->recvd,
					result->recvd_len);
			break;
		case 6:
			canidate->quest_bytes = result->recvd_len;
			canidate->quest_str = g_memdup(result->recvd,
					result->recvd_len);
			break;
		case 7:
			canidate->i_bytes = result->recvd_len;
			break;
		case 8:
			canidate->f0_bytes = result->recvd_len;
			break;
		case 9:
			canidate->f1_bytes = result->recvd_len;
			break;

	}
			
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
