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
#include <glib/gprintf.h>
#include <interrogate.h>
#include <mode_select.h>
#include <notifications.h>
#include <serialio.h>
#include <structures.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <threads.h>
#include <unistd.h>

extern unsigned int ecu_caps;
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
	gchar sig_str[64];	/* Signature string to search for */
	gchar quest_str[64];	/* Ext Version string to search for */
	gint ver_num;		/* Version number to search for */
	gchar firmware_name[64];/* Name of this firmware */
	gint capabilities;	/* Bitmask of capabilities.... */
} canidates[] = 
{
	{ {22,0,0,125,125,0,0,0,0,0},NULL,NULL,{},{},0,
			{"Old Bowling & Grippo 1.0\0"},0},
	{ {22,1,1,125,125,0,0,0,0,0},NULL,NULL,{},{},20,
			{"Standard Bowling & Grippo (2.0-3.01)\0"},
			0},
	{ {22,1,1,128,128,0,0,0,255,255}, NULL,NULL,{},{},1,
			{"Dualtable 0.90-1.0\0"},DUALTABLE},
	{ {22,1,1,128,128,18,0,0,255,255},NULL,NULL,{"v.1.01\0"},{},1,
			{"Dualtable 1.01\0"},DUALTABLE},
	{ {22,1,1,128,128,19,0,0,255,255},NULL,NULL,{"v.1.02\0"},{},1,
			{"Dualtable 1.02\0"},DUALTABLE|IAC_PWM},
	{ {22,1,1,128,128,17,0,0,0,0},NULL,NULL,"Rover IAC\0",{},30,
			{"MS-3.0 Rover IAC (3.0.4)\0"},IAC_STEPPER},
	{ {22,1,1,128,128,16,0,0,0,0},NULL,NULL,"Rover IAC\0",{},30,
			{"MS-3.0 Rover IAC (3.0.5)\0"},IAC_STEPPER},
	{ {22,1,1,125,125,0,0,83,0,0},NULL,NULL,{},{},20,
			{"MegaSquirtnEDIS v0.108 OR SquirtnSpark 2.02\0"},
			S_N_EDIS},
	{ {22,1,1,125,125,0,0,95,0,0},NULL,NULL,{},{},30,
			{"SquirtnSpark 3.0\0"},S_N_SPARK},
	{ {22,1,1,125,125,0,32,95,0,0},NULL,NULL,{},{"EDIS v3.005\0"},30,
			{"MegaSquirtnEDIS 3.005\0"},S_N_EDIS},
	{ {22,1,1,125,125,0,32,95,0,0},NULL,NULL,{},{"EDIS v3.007\0"},30,
			{"MegaSquirtnEDIS 3.007\0"},S_N_EDIS},
	{ {22,1,1,125,125,0,0,99,255,255},NULL,NULL,{},{},13,
			{"MegaSquirt'N'Spark Extended 3.0.1\0"},
			S_N_SPARK|LAUNCH_CTRL}
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
	{ 0,"V", "VE/Constants page0", 1, TRUE },
	{ 1,"V", "VE/Constants page1", 1, TRUE },
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
			/* Set caps to std, no flags, disable all extra
			 * controls 
			 */
			parse_ecu_capabilities(0);
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
		g_free(string);
		if (res != len)
			g_fprintf(stderr,__FILE__": Error writing data to the ECU\n");
		res = poll (&ufds,1,25);
		if (res)
		{	
			while (poll(&ufds,1,25))
			{
				total += count = read(serial_params->fd,ptr+total,64);
				//g_printf("count %i, total %i\n",count,total);
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
					strncpy(canidate->sig_str,buf,total);
					break;
				case CMD_QUEST:
					strncpy(canidate->quest_str,buf,total);
					break;
				case CMD_Q:
					memcpy(&(canidate->ver_num),buf,total);
					break;
				default:
					break;
			}
			/* copy data from tmp buffer to struct pointer */
	
		}

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
	gchar * tmpbuf = NULL;

	/* compare the canidate to all the choices.  As OF now we are ONLY
	 * comparing byte counts as that is enough to guarantee unique-ness
	 * future firmwares may use the same bytecount but MUST use either
	 * unique version numbers AND/OR unique extended signatures to be
	 * properly detected and handled...
	 */
	for (i=0;i<num_choices;i++)
	{
		passcount = 0;
		for (j=0;j<num_tests;j++)
		{
			if (canidate->bytes[j] != canidates[i].bytes[j])
				goto retest; /*THIS IS UGLY <--- */
		}
		/* If all test pass, now check the Extended version
		 * If it matches,  jump out...
		 */
		if ((canidates[i].quest_str != NULL) && (canidate->quest_str != NULL))
		{
			if (strstr(canidate->quest_str,canidates[i].quest_str) != NULL)
			{
				match = i;
				break;
			}
		}
		else if ((canidates[i].sig_str != NULL) && (canidate->sig_str != NULL))
		{
			if (strstr(canidate->sig_str,canidates[i].sig_str) != NULL)
			{
				match = i;
				break;
			}
		}
		else
		{
			match = i;
			break;
		}
		retest:
		if (0)	/* THIS IS an UGLY HACK!!!! */
			;; /* Only here to suppress GCC warning!!!! */
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
				tmpbuf = g_strndup(canidate->sig_str,canidate->bytes[i]);
			gtk_entry_set_text(GTK_ENTRY(entries.ecu_signature_entry),tmpbuf);
			g_free(tmpbuf);
		}
		if (i == CMD_QUEST)
		{
			if (canidate->bytes[i] == 0)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strndup(canidate->quest_str,canidate->bytes[i]);
			gtk_entry_set_text(GTK_ENTRY(entries.extended_revision_entry),tmpbuf);
			g_free(tmpbuf);
		}

	}
	if (match == -1) // (we DID NOT find one)
	{
		tmpbuf = g_strdup_printf("Firmware NOT DETECTED properly, contact author with the contents of this window\n");
		// Store counts for VE/realtime readback... 

		update_logbar(interr_view,"warning",tmpbuf,FALSE,FALSE);
		g_free(tmpbuf);
		goto cleanup;
	}

	/* Set flags */
	ecu_caps = canidates[match].capabilities;
	/* Enable/Disable Controls */
	parse_ecu_capabilities(ecu_caps);

	/* Set expected sizes for commands */
	serial_params->table0_size = canidates[match].bytes[CMD_V0];
	serial_params->table1_size = canidates[match].bytes[CMD_V1];
	serial_params->rtvars_size = canidates[match].bytes[CMD_A];
	serial_params->ignvars_size = canidates[match].bytes[CMD_I];
	/* Enable Access to enhanced idle controls */
//	if ((iac_variant) || (dualtable))
//		gtk_widget_set_sensitive(GTK_WIDGET(buttons.pwm_idle_but),TRUE);
//	else
//
//		gtk_widget_set_sensitive(GTK_WIDGET(buttons.pwm_idle_but),FALSE);
	/* Display firmware version in the window... */
	tmpbuf = g_strdup_printf("Detected Firmware: %s\n",canidates[match].firmware_name);

	update_logbar(interr_view,"warning",tmpbuf,FALSE,FALSE);
	g_free(tmpbuf);

cleanup:
	g_free(canidate->v0_data);
	g_free(canidate->v1_data);
	g_free(canidate);
	return;

}
