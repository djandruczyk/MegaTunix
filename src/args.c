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
#include <debugging.h>
#include <defines.h>
#include <errno.h>
#include <glib.h>
#include <mtxsocket.h>
#include <offline.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


extern gconstpointer *global_data;
/*!
 \brief handle_args() handles parsing of cmd line arguments to megatunix
 \param argc (gint) count of command line arguments
 \param argv (char **) array of command line args
 \returns void
 */
G_MODULE_EXPORT void handle_args(gint argc, gchar * argv[])
{
	GError *error = NULL;
	CmdLineArgs *args = NULL;
	GOptionContext *context = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;
	gint result = 0;
	gchar ** vector = NULL;
	gchar * netinfo = NULL;

	args = init_args();
	GOptionEntry entries[] =
	{
		{"debugargs",'d',0,G_OPTION_ARG_NONE,&args->debug,"Dump argument debugging info to console",NULL},
		{"DEBUG Log",'D',0,G_OPTION_ARG_FILENAME,&args->dbglog,"Debug logfile name (referenced from homedir)",NULL},
		{"version",'v',0,G_OPTION_ARG_NONE,&args->version,"Print MegaTunix's Version number",NULL},
		{"quiet",'q',0,G_OPTION_ARG_NONE,&args->be_quiet,"Suppress all GUI error notifications",NULL},
		{"offline",'o',0,G_OPTION_ARG_NONE,&args->offline,"Offline mode",NULL},
		{"Port",'P',0,G_OPTION_ARG_STRING,&args->port,"Use this serial port ONLY",NULL},
		{"Listen",'L',0,G_OPTION_ARG_NONE,&args->listen_mode,"Startup MegaTunix in Listen mode, awaiting external call-home connection.",NULL},
		{"network",'n',0,G_OPTION_ARG_STRING,&netinfo,"Connect to Network socket instead of serial","host[:port]"},
		{"no-rttext",'r',0,G_OPTION_ARG_NONE,&args->hide_rttext,"Hide RealTime Vars Text window",NULL},
		{"no-status",'s',0,G_OPTION_ARG_NONE,&args->hide_status,"Hide ECU Status window",NULL},
		{"no-maingui",'m',0,G_OPTION_ARG_NONE,&args->hide_maingui,"Hide Main Gui window (i.e, dash only)",NULL},
		{"autolog",'a',0,G_OPTION_ARG_NONE,&args->autolog_dump,"Automatically dump datalog to file every N minutes",NULL},
		{"minutes",'t',0,G_OPTION_ARG_INT,&args->autolog_minutes,"How many minutes of data logged per logfile (default 5 minutes)","Minutes per log (integer)"},
		{"log_dir",'l',0,G_OPTION_ARG_FILENAME,&args->autolog_dump_dir,"Directory to put datalogs into","Directory to place logs in"},
		{"log_basename",'b',0,G_OPTION_ARG_FILENAME,&args->autolog_basename,"Base filename for logs.","Root of logfile name"},
		{ NULL },
	};

	context = g_option_context_new ("- MegaTunix Options");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_parse (context, &argc, &argv, &error);

	if (error)
	{
		printf(_("ERROR OCCURRED\n"));
		switch (error->code) 
		{
			case G_OPTION_ERROR_UNKNOWN_OPTION:
				printf("G_OPTION_ERROR_UNKNOWN_OPTION\n");
				break;
			case G_OPTION_ERROR_BAD_VALUE:
				printf("G_OPTION_ERROR_BAD_VALUE\n");
				break;
			case G_OPTION_ERROR_FAILED:
				printf("G_OPTION_ERROR_FAILED\n");
				break;
			default:
				printf(_("not sure what the error is\n"));
				break;
		}
	}
	if (netinfo)
	{
		vector = g_strsplit(netinfo,":",2);
		g_free(netinfo);
		if (g_strv_length(vector) > 2)
		{
			printf(_("Network info provided is invalid!\n"));
			args->network_mode = FALSE;
		}
		else
		{
			args->network_host = g_strdup(vector[0]);
			if (g_strv_length(vector) == 1)
				args->network_port = MTX_SOCKET_BINARY_PORT;
			else
				args->network_port = atoi(vector[1]);
			args->network_mode = TRUE;
			args->listen_mode = FALSE;
		}
		g_strfreev(vector);
	}
	if (args->port)
	{
		DATA_SET(global_data,"autodetect_port",GINT_TO_POINTER(FALSE));
		DATA_SET_FULL(global_data,"override_port",g_strdup(args->port),g_free);
	}
	if (args->autolog_dump)
	{

		if (args->autolog_minutes == 0)
			args->autolog_minutes = 5;
		if (!args->autolog_dump_dir)
			args->autolog_dump_dir = (gchar *)HOME();
		else
		{
			if (!(g_file_test(args->autolog_dump_dir, G_FILE_TEST_IS_DIR)))
			{
				result = g_mkdir_with_parents(args->autolog_dump_dir, 755);
				if (!result)
					dbg_func(IO_PROCESS|CRITICAL,g_strdup_printf(__FILE__"\tAutolog dump dir creation ERROR: \"%s\"\n",g_strerror(errno)));
			}
		}
		if (!args->autolog_basename)
		{
			t = g_malloc(sizeof(time_t));
			time(t);
			tm = localtime(t);
			g_free(t);
			args->autolog_basename = g_strdup_printf("datalog_%.2i-%.2i-%i",1+(tm->tm_mon),tm->tm_mday,1900+(tm->tm_year));
		}
		g_timeout_add(args->autolog_minutes*60000,(GSourceFunc)autolog_dump,NULL);
	}
	if (args->offline)
	{
		DATA_SET(global_data,"offline",GINT_TO_POINTER(TRUE));
		g_timeout_add(1000,(GSourceFunc)set_offline_mode,NULL);
	}
	if (args->listen_mode)
	{
		printf(_("Should do listen mode\n"));
	}
	if (args->hide_rttext)
		DATA_SET(global_data,"rtt_visible",GINT_TO_POINTER(FALSE));
	if (args->hide_status)
		DATA_SET(global_data,"status_visible",GINT_TO_POINTER(FALSE));
	if (args->hide_maingui)
		DATA_SET(global_data,"main_visible",GINT_TO_POINTER(FALSE));
	if (args->debug)
	{
		printf(_("debug option \"%i\"\n"),args->debug);
		printf(_("Global debug filename\"%s\"\n"),args->dbglog);
		printf(_("version option \"%i\"\n"),args->version);
		printf(_("Port option \"%s\"\n"),args->port);
		printf(_("quiet option \"%i\"\n"),args->be_quiet);
		printf(_("no rttext option \"%i\"\n"),args->hide_rttext);
		printf(_("no status option \"%i\"\n"),args->hide_status);
		printf(_("no maingui option \"%i\"\n"),args->hide_maingui);
		printf(_("autolog_dump \"%i\"\n"),args->autolog_dump);
		printf(_("autolog_minutes \"%i\"\n"),args->autolog_minutes);
		printf(_("autolog_dump_dir \"%s\"\n"),args->autolog_dump_dir);
		printf(_("autolog_basename \"%s\"\n"),args->autolog_basename);
		printf(_("listen mode \"%i\"\n"),args->listen_mode);
		printf(_("network mode \"%i\"\n"),args->network_mode);
		printf(_("network host \"%s\"\n"),args->network_host);
		printf(_("network port \"%i\"\n"),args->network_port);
	}
	if (args->version)
	{
		if (g_strcasecmp(_VER_SUFFIX_,"") == 0)
			printf("%i.%i.%i\n",_MAJOR_,_MINOR_,_MICRO_);
		else
			printf("%i.%i.%i-%s\n",_MAJOR_,_MINOR_,_MICRO_,_VER_SUFFIX_);
		exit(0);
	}
	DATA_SET_FULL(global_data,"args",args,g_free);
	g_option_context_free(context);
}


G_MODULE_EXPORT CmdLineArgs * init_args(void)
{
	CmdLineArgs *args;
	args = g_new0(CmdLineArgs, 1);
	args->debug = FALSE;
	args->dbglog = NULL;
	args->be_quiet = FALSE;
	args->autolog_dump = FALSE;
	args->hide_rttext = FALSE;
	args->hide_status = FALSE;
	args->hide_maingui = FALSE;
	args->autolog_minutes = 5;
	args->autolog_dump_dir = NULL;
	args->autolog_basename = NULL;
	args->listen_mode = FALSE;
	args->network_mode = FALSE;
	args->network_host = NULL;
	args->network_port = 0;
	
	return (args);
}
