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
#include <string.h>
#include <structures.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <unistd.h>

extern gboolean connected;
extern GtkWidget *ms_ecu_revision_entry;
extern GtkTextBuffer *textbuffer;
extern GtkWidget *interr_view;
gboolean interrogated = FALSE;

extern struct Serial_Params *serial_params;
gfloat ecu_version;
const gchar *cmd_chars[] = {"A","C","Q","V","S","I","?"};
gboolean dualtable;
struct Cmd_Results
{
	gchar *cmd_string;
	gint count;
} ;

/* The Various MegaSquirt variants that MegaTunix attempts to support
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
	char buf[size];
	gint res = 0;
	gint count = 0;
	gint i = 0;
	gint len = 0;
	gint v_bytes = 0;
	gint s_bytes = 0;
	gint i_bytes = 0;
	gint quest_bytes = 0;
	gchar *tmpbuf;
	gboolean con_status = FALSE;
	gint tests_to_run = sizeof(cmd_chars)/sizeof(gchar *);
	struct Cmd_Results cmd_res[tests_to_run]; 

	if (!connected)
	{
		con_status = check_ecu_comms(NULL,NULL);
		if (con_status == FALSE)
		{
			interrogated = FALSE;
			return;
		}
	}

	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;

	tcflush(serial_params->fd, TCIFLUSH);

	for (i=0;i<tests_to_run;i++)
	{
		count = 0;
		cmd_res[i].cmd_string = g_strdup(cmd_chars[i]);
		len = strlen(cmd_res[i].cmd_string);
		res = write(serial_params->fd,cmd_chars[i],len);
		res = poll (&ufds,1,serial_params->poll_timeout);
		if (res)
		{	
			while (poll(&ufds,1,serial_params->poll_timeout))
				count += read(serial_params->fd,&buf,64);
		}
		else
		{
			/* Poll timout, ECU not connected???  */
		}
		cmd_res[i].count = count;
	}
	for (i=0;i<tests_to_run;i++)
	{
		if (cmd_res[i].count > 0)
		{
			tmpbuf = g_strdup_printf(
					"Command %s, returned %i bytes\n",
					cmd_res[i].cmd_string, 
					cmd_res[i].count);
			/* Store counts for VE/realtime readback... */
			if (strstr(cmd_res[i].cmd_string,"V"))
				serial_params->table0_size = cmd_res[i].count;
			if (strstr(cmd_res[i].cmd_string,"A"))
				serial_params->rtvars_size = cmd_res[i].count;
				
			update_logbar(interr_view,NULL,tmpbuf,FALSE);
			g_free(tmpbuf);
		}
		else
		{
			tmpbuf = g_strdup_printf(
					"Command %s isn't supported...\n",
					cmd_res[i].cmd_string);
			update_logbar(interr_view,NULL,tmpbuf,FALSE);
			g_free(tmpbuf);
		}
	}

	tcflush(serial_params->fd, TCIFLUSH);

	for(i=0;i<tests_to_run;i++)
	{
		if (strcmp(cmd_res[i].cmd_string,"V")== 0)
			v_bytes = cmd_res[i].count;
		if (strcmp(cmd_res[i].cmd_string,"S")== 0)
			s_bytes = cmd_res[i].count;
		if (strcmp(cmd_res[i].cmd_string,"I")== 0)
			i_bytes = cmd_res[i].count;
		if (strcmp(cmd_res[i].cmd_string,"?")== 0)
			quest_bytes = cmd_res[i].count;
	}
	if (v_bytes > 125)
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

	interrogated = TRUE;
	return;
}
