/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*! 
  \file src/args.c
  \ingroup CoreMtx 
  \brief Processes Command Line arguments for things like personality
  selection, Dashboard,  online/offline mode, background logging, etc
  \author David Andruczyk
  */

#include <args.h>
#include <dashboard.h>
#include <defines.h>
#include <datalogging_gui.h>
#include <debugging.h>
#include <errno.h>
#include <init.h>
#include <stdlib.h>
#include <time.h>

extern gconstpointer *global_data;

/*!
  \brief handle_args() handles parsing of cmd line arguments to megatunix
  \param argc is the count of command line arguments
  \param argv is the array of command line args
  */
G_MODULE_EXPORT void handle_args(gint argc, gchar * argv[])
{
	GError *error = NULL;
	CmdLineArgs *args = NULL;
	GOptionContext *context = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;
	gboolean result = FALSE;
	gchar ** vector = NULL;
	gchar * netinfo = NULL;
	gchar * dash = NULL;

	ENTER();

	args = init_args();
	GOptionEntry entries[] =
	{
		{"verbose",'v',0,G_OPTION_ARG_NONE,&args->verbose,"Make MegaTunix be more verbose (debug option)",NULL},
		{"debugargs",'d',0,G_OPTION_ARG_NONE,&args->debugargs,"Dump argument debugging info to console",NULL},
		{"DEBUG Log",'L',0,G_OPTION_ARG_FILENAME,&args->dbglog,"Debug logfile name (referenced from homedir)",NULL},
		{"Version",'V',0,G_OPTION_ARG_NONE,&args->version,"Print MegaTunix's Version number",NULL},
		{"quiet",'q',0,G_OPTION_ARG_NONE,&args->be_quiet,"Suppress all GUI error notifications",NULL},
		{"persona",'p',0,G_OPTION_ARG_STRING,&args->persona,"ECU Personality <MS1|MS2|MS3-1.0|MS3-1.1|LibreEMS|JimStim|PIS>",NULL},
		{"offline",'o',0,G_OPTION_ARG_NONE,&args->offline,"Offline mode",NULL},
		{"Port",'P',0,G_OPTION_ARG_STRING,&args->port,"Use this serial port ONLY",NULL},
		{"project",0,0,G_OPTION_ARG_STRING,&args->project_name,"Use this project name",NULL},
		/*{"Listen",'L',0,G_OPTION_ARG_NONE,&args->listen_mode,"Startup MegaTunix in Listen mode, awaiting external call-home connection.",NULL},*/
		{"inhibit-tabs",'i',0,G_OPTION_ARG_NONE,&args->inhibit_tabs,"Prevent loading of tabs (debug option)",NULL},
		{"network",'n',0,G_OPTION_ARG_STRING,&netinfo,"Network connect instead of serial","host[:port]"},
		{"no-rttext",'r',0,G_OPTION_ARG_NONE,&args->hide_rttext,"Hide RealTime Vars Text window",NULL},
		{"no-status",'s',0,G_OPTION_ARG_NONE,&args->hide_status,"Hide ECU Status window",NULL},
		{"no-maingui",'m',0,G_OPTION_ARG_NONE,&args->hide_maingui,"Hide Main Gui window (i.e, dash only)",NULL},
		{"Dashboard",'D',0,G_OPTION_ARG_STRING,&dash,"Dashboard to load, use \"list\" for a listing","Dash name"},
		{"FullscreenDash",'F',0,G_OPTION_ARG_NONE,&args->dash_fullscreen,"Dashboard  should go fullscreen",NULL},
		{"autolog",'a',0,G_OPTION_ARG_NONE,&args->autolog_dump,"Automatically dump datalog to file every N minutes",NULL},
		{"minutes",'t',0,G_OPTION_ARG_INT,&args->autolog_minutes,"Minutes of data logged per logfile (default 5 minutes)",NULL},
		{"log_dir",'l',0,G_OPTION_ARG_FILENAME,&args->autolog_dump_dir,"Directory to put datalogs into","/path/to/file"},
		{"log_basename",'b',0,G_OPTION_ARG_FILENAME,&args->autolog_basename,"Base filename for logs.",NULL},
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
	if (dash)
	{
		if (g_ascii_strcasecmp(dash,"list") == 0)
		{
			print_dash_choices((gchar *)DATA_GET(global_data,"project_name"));
			exit(1);
		}
		else
		{

			args->dashboard = validate_dash_choice(dash, &result);
			if (!result)
				printf("Could not locate dashboard \"%s\",\ncheck spelling or use \'-D list\' to see the choices\n",dash);
		}
		g_free(dash);
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
	if (args->project_name)
		DATA_SET(global_data,"project_name",g_strdup(args->project_name));
	else
		DATA_SET(global_data,"project_name",g_strdup("default"));

	if (args->port)
	{
		DATA_SET(global_data,"autodetect_port",GINT_TO_POINTER(FALSE));
		DATA_SET_FULL(global_data,"override_port",g_strdup(args->port),g_free);
	}
	if (args->persona)
		DATA_SET_FULL(global_data,"cli_persona",g_strdup(args->persona),g_free);
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
					MTXDBG(IO_PROCESS|CRITICAL,_("Autolog dump dir creation ERROR: \"%s\"\n"),g_strerror(errno));
			}
		}
		if (!args->autolog_basename)
		{
			t = (time_t *)g_malloc(sizeof(time_t));
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
		if (!(args->persona))
			printf("Suggest specifying a persona with the  \"-p\" option when using offline mode\n");
	}
	if (args->listen_mode)
	{
		printf(_("Should do listen mode\n"));
	}
	if (args->hide_rttext)
		DATA_SET(global_data,"rtt_visible",GINT_TO_POINTER(FALSE));
	else
		DATA_SET(global_data,"rtt_visible",GINT_TO_POINTER(TRUE));
	if (args->hide_status)
		DATA_SET(global_data,"status_visible",GINT_TO_POINTER(FALSE));
	else
		DATA_SET(global_data,"status_visible",GINT_TO_POINTER(TRUE));
	if (args->hide_maingui)
	{
		DATA_SET(global_data,"main_visible",GINT_TO_POINTER(FALSE));
		DATA_SET(global_data,"gui_visible",GINT_TO_POINTER(FALSE));
	}
	else
	{
		DATA_SET(global_data,"main_visible",GINT_TO_POINTER(TRUE));
		DATA_SET(global_data,"gui_visible",GINT_TO_POINTER(TRUE));
	}
	if (args->debugargs)
	{
		printf(_("verbose option \"%i\"\n"),args->verbose);
		printf(_("debug option \"%i\"\n"),args->debugargs);
		printf(_("Global debug filename\"%s\"\n"),args->dbglog);
		printf(_("Version option \"%i\"\n"),args->version);
		printf(_("Port option \"%s\"\n"),args->port);
		printf(_("Project option \"%s\"\n"),args->project_name);
		printf(_("Persona option \"%s\"\n"),args->persona);
		printf(_("quiet option \"%i\"\n"),args->be_quiet);
		printf(_("inhibit tabs \"%i\"\n"),args->inhibit_tabs);
		printf(_("Dashboard Selected \"%s\"\n"),args->dashboard);
		printf(_("Dashboard Fullscreen \"%i\"\n"),args->dash_fullscreen);
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
		printf("MegaTunix Version: %s\n",MTX_VER_STRING);
		exit(0);
	}
	DATA_SET_FULL(global_data,"args",args,args_free);
	g_option_context_free(context);
	EXIT();
	return;
}


/*!
  \brief Initializes and returns a pointer to a CmdLineArgs structure.
  \returns pointer to CmdLineArgs structure
  */
G_MODULE_EXPORT CmdLineArgs * init_args(void)
{
	CmdLineArgs *args;

	ENTER();
	args = g_new0(CmdLineArgs, 1);
	args->debugargs = FALSE;
	args->verbose = FALSE;
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
	
	EXIT();
	return (args);
}


/*!
  \brief frees a CmdLineArgs structure
  \param data is the pointer to CmdLineArgs structure to be freed
  */
G_MODULE_EXPORT void args_free(gpointer data)
{
	CmdLineArgs *args = (CmdLineArgs *)data;

	ENTER();

	g_return_if_fail(args);
	cleanup(args->dbglog);
	cleanup(args->autolog_dump_dir);
	cleanup(args->autolog_basename);
	cleanup(args->network_host);
	cleanup(args->port);
	cleanup(args->persona);
	cleanup(args);
	EXIT();
	return;
}
