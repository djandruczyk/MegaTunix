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


#include <args.h>
#include <config.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <glib.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


extern GObject *global_data;
/*!
 \brief handle_args() handles parsing of cmd line arguments to megatunix
 \param argc (gint) count of command line arguments
 \param argv (char **) array of command line args
 \returns void
 */
void handle_args(gint argc, gchar * argv[])
{
	GError *error = NULL;
	CmdLineArgs *args = NULL;
	GOptionContext *context = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;

	args = init_args();
	GOptionEntry entries[] =
	{
		{"debugargs",'d',0,G_OPTION_ARG_NONE,&args->debug,"Dump argument debugging info to console",NULL},
		{"version",'v',0,G_OPTION_ARG_NONE,&args->version,"Print MegaTunix's Version number",NULL},
		{"quiet",'q',0,G_OPTION_ARG_NONE,&args->be_quiet,"Suppress all GUI error notifications",NULL},
		{"no-rttext",'r',0,G_OPTION_ARG_NONE,&args->hide_rttext,"Hide RealTime Vars Text window",NULL},
		{"no-status",'s',0,G_OPTION_ARG_NONE,&args->hide_status,"Hide ECU Status window",NULL},
		{"no-maingui",'h',0,G_OPTION_ARG_NONE,&args->hide_maingui,"Hide Main Gui window (i.e, dash only)",NULL},
		{"autolog",'a',0,G_OPTION_ARG_NONE,&args->autolog_dump,"Automatically dump datalog to file every N minutes",NULL},
		{"minutes",'m',0,G_OPTION_ARG_INT,&args->autolog_minutes,"How many minutes of data logged per logfile (default 5 minutes)","N"},
		{"log_dir",'l',0,G_OPTION_ARG_FILENAME,&args->autolog_dump_dir,"Directory to put datalogs into",NULL},
		{"log_basename",'b',0,G_OPTION_ARG_FILENAME,&args->autolog_basename,"Base filename for logs.",NULL},
		{ NULL },
	};

	context = g_option_context_new ("- MegaTunix options");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_parse (context, &argc, &argv, &error);

	if (args->version)
	{
		printf("%i.%i.%i\n",_MAJOR_,_MINOR_,_MICRO_);
		exit(0);
	}
	if (args->autolog_dump)
	{

		if (args->autolog_minutes == 0)
			args->autolog_minutes = 5;
		if (!args->autolog_dump_dir)
			args->autolog_dump_dir = (gchar *)HOME();
		if (!args->autolog_basename)
		{
			t = g_malloc(sizeof(time_t));
			time(t);
			tm = localtime(t);
			g_free(t);
			args->autolog_basename = g_strdup_printf("datalog_%.2i-%.2i-%i",1+(tm->tm_mon),tm->tm_mday,1900+(tm->tm_year));
		}
		g_timeout_add(args->autolog_minutes*60000,(GtkFunction)autolog_dump,NULL);
	}
	if (args->debug)
	{
		printf("debug option \"%i\"\n",args->debug);
		printf("version option \"%i\"\n",args->version);
		printf("quiet option \"%i\"\n",args->be_quiet);
		printf("no rttext option \"%i\"\n",args->hide_rttext);
		printf("no status option \"%i\"\n",args->hide_status);
		printf("no maingui option \"%i\"\n",args->hide_maingui);
		printf("autolog_dump \"%i\"\n",args->autolog_dump);
		printf("autolog_minutes \"%i\"\n",args->autolog_minutes);
		printf("autolog_dump_dir \"%s\"\n",args->autolog_dump_dir);
		printf("autolog_basename \"%s\"\n",args->autolog_basename);
	}
	OBJ_SET(global_data,"args",args);
	g_option_context_free(context);
}


CmdLineArgs * init_args()
{
	CmdLineArgs *args;
	args = g_new0(CmdLineArgs, 1);
	args->debug = FALSE;
	args->be_quiet = FALSE;
	args->autolog_dump = FALSE;
	args->hide_rttext = FALSE;
	args->hide_status = FALSE;
	args->hide_maingui = FALSE;
	args->autolog_minutes = 5;
	args->autolog_dump_dir = NULL;
	args->autolog_basename = NULL;
	
	return (args);
}
