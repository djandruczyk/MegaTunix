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
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <interrogate.h>
#include <lookuptables.h>
#include <mode_select.h>
#include <notifications.h>
#include <serialio.h>
#include <stdlib.h>
#include <structures.h>
#include <string.h>
#include <stringmatch.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <threads.h>
#include <unistd.h>

extern gint ecu_caps;
extern gboolean connected;
extern GtkWidget *ms_ecu_revision_entry;
extern GtkTextBuffer *textbuffer;
extern GtkWidget *interr_view;
extern struct Serial_Params *serial_params;
extern struct DynamicEntries entries;
struct Firmware_Details *firmware = NULL;
gboolean interrogated = FALSE;

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
	struct Command *cmd = NULL;
	struct Canidate *canidate = NULL;
	gint size = 1024;
	gint res = 0;
	gint count = 0;
	gint i = 0;
	gint total = 0;
	gint last_page = 0;
	gint tests_to_run = 0;
	gchar *string = NULL;
	gchar * tmpbuf = NULL;
	GArray *cmd_array = NULL;
	unsigned char buf[size];
	unsigned char *ptr = buf;
	GHashTable *cmd_details = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	/* prevent multiple runs of interrogator simultaneously */
	g_static_mutex_lock(&mutex);

	if (!connected)
	{
		gdk_threads_enter();
		no_ms_connection();
		gdk_threads_leave();
		g_static_mutex_unlock(&mutex);
		return;
	}
	
	/* Allocate memory to store interrogation results */
	canidate = g_malloc0(sizeof(struct Canidate));
	canidate->bytecounts = g_hash_table_new(g_str_hash,g_str_equal);

	cmd_details = g_hash_table_new(g_str_hash,g_str_equal);

	/* Build table of strings to enum values */
	build_string_2_enum_table();
	/* Load tests from config files */
	cmd_array = validate_and_load_tests(cmd_details);
	/* how many tests.... */
	tests_to_run = cmd_array->len;

	/* Configure port for polled IO and flush I/O buffer */
	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;
	tcflush(serial_params->fd, TCIOFLUSH);

	for (i=0;i<tests_to_run;i++)
	{
		count = 0;
		total = 0;
		cmd = g_array_index(cmd_array,struct Command *, i);

		/* flush buffer to known state.. */
		memset (buf,0,size);

		ptr = buf;
		/* set page */
		if (last_page != cmd->page)
			set_ms_page(cmd->page);
		last_page = cmd->page;

		string = g_strdup(cmd->string);
		res = write(serial_params->fd,string,cmd->len);
		if (res != cmd->len)
			dbg_func(__FILE__": Error writing data to the ECU\n",CRITICAL);
		dbg_func(g_strdup_printf(__FILE__": interrogate_ecu() sent command \"%s\"\n",string),INTERROGATOR);
		if (cmd->multipart)
		{
			res = write(serial_params->fd,&cmd->cmd_int_arg,1);
			if (res != 1)
				dbg_func(__FILE__": Error writing data to the ECU\n",CRITICAL);
			dbg_func(g_strdup_printf(__FILE__": interrogate_ecu() sent argument \"%i\"\n",cmd->cmd_int_arg),INTERROGATOR);
		}
		g_free(string);
		res = poll (&ufds,1,25);
		if (res)
		{	
			while (poll(&ufds,1,50))
			{
				total += count = read(serial_params->fd,ptr+total,64);
			}
			dbg_func(g_strdup_printf(__FILE__": interrogate_ecu() received %i bytes\n",total),INTERROGATOR);
			ptr = buf;

			/* copy data from tmp buffer to struct pointer */
			switch (cmd->store_type)
			{
				case SIG:
					canidate->sig_str = g_strndup(buf,total);
					break;
				case EXTVER:
					canidate->quest_str = g_strndup(buf,total);
					break;
				case VNUM:
					memcpy(&(canidate->ver_num),buf,total);
					break;
				default:
					break;
			}
		}

		/* store number of bytes received in counter variable */
		tmpbuf = g_strdup_printf("CMD_%s_%i",cmd->string,cmd->page);
		g_hash_table_insert(canidate->bytecounts,g_strdup(tmpbuf),
				GINT_TO_POINTER(total));
		g_free(tmpbuf);
	}
	/* Reset page to 0 just to be 100% sure... */
	set_ms_page(0);
	/* flush serial port */
	tcflush(serial_params->fd, TCIOFLUSH);

	interrogated = TRUE;

	determine_ecu(canidate,cmd_array,cmd_details);	

	if (canidate)
		g_free(canidate);

	free_test_commands(cmd_array);
	g_hash_table_destroy(cmd_details);

	g_static_mutex_unlock(&mutex);

	return;
}

void determine_ecu(void *ptr, GArray *cmd_array, GHashTable *cmd_details)
{
	struct Canidate *canidate = (struct Canidate *)ptr;
	struct Canidate *potential = NULL;
	struct Command *cmd = NULL;
	gint i = 0;
	gint num_tests = cmd_array->len;
	gboolean match = FALSE;
	gchar * tmpbuf = NULL;
	GDir *dir = NULL;
	GError *error = NULL;
	gchar * path = NULL;
	gchar * filename = NULL;
	gchar * full_pathname = NULL;
	extern struct IoCmds *cmds;

	
	/* Search homedire for potential interrogation profiles FIRST... */
	path = g_strconcat(g_get_home_dir(),"/.MegaTunix/",INTERROGATOR_DIR,"/Profiles/",NULL);
	dir = g_dir_open(path,0,&error);	
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		full_pathname= g_strconcat(path,filename,NULL);
		potential = load_potential_match(cmd_array,full_pathname);
		if (check_for_match(cmd_array,potential,canidate))
		{
			match = TRUE;
			break;
		}

		filename = (gchar *) g_dir_read_name(dir);
	}
	/* IF search in homedir failes search system wide ones */
	if (match == FALSE)
	{
		path = g_strconcat(DATA_DIR,"/",INTERROGATOR_DIR,"/Profiles/",NULL);
		dir = g_dir_open(path,0,&error);	
		filename = (gchar *)g_dir_read_name(dir);
		while (filename != NULL)
		{
			full_pathname= g_strconcat(path,filename,NULL);
			potential = load_potential_match(cmd_array,full_pathname);
			if (check_for_match(cmd_array,potential,canidate))
			{
				match = TRUE;
				break;
			}
			filename = (gchar *) g_dir_read_name(dir);
		}
	}
	if (dir)
		g_dir_close(dir);
	/* Update the screen with the data... */
	for (i=0;i<num_tests;i++)
	{
		cmd = g_array_index(cmd_array,struct Command *,i);
		tmpbuf = g_strdup_printf("Command \"%s\" (%s), returned %i bytes\n",
				cmd->string, 
				cmd->desc, 
				(gint) g_hash_table_lookup(canidate->bytecounts, g_strdup_printf("CMD_%s_%i",cmd->string,cmd->page)));

		// Store counts for VE/realtime readback... 
		dbg_func(tmpbuf,INTERROGATOR);
		gdk_threads_enter();
		update_logbar(interr_view,NULL,tmpbuf,FALSE,FALSE);
		gdk_threads_leave();
		g_free(tmpbuf);
		if (cmd->store_type == VNUM)
		{
			if (canidate->ver_num == 0)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strdup_printf("%.1f",
						((float)canidate->ver_num/10.0));
			gdk_threads_enter();
			gtk_entry_set_text(GTK_ENTRY(entries.ecu_revision_entry)
					,tmpbuf);
			gdk_threads_leave();
			g_free(tmpbuf);
		}
		if (cmd->store_type == SIG)
		{
			if (canidate->sig_str == NULL)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strndup(
						canidate->sig_str,
						(gint)g_hash_table_lookup(canidate->bytecounts, g_strdup_printf("CMD_%s_%i",cmd->string,cmd->page)));
			gdk_threads_enter();
			gtk_entry_set_text(GTK_ENTRY(entries.ecu_signature_entry),tmpbuf);
			gdk_threads_leave();
			g_free(tmpbuf);
		}
		if (cmd->store_type == EXTVER)
		{
			if (canidate->quest_str == NULL)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strndup(
						canidate->quest_str,
						(gint)g_hash_table_lookup(canidate->bytecounts, g_strdup_printf("CMD_%s_%i",cmd->string,cmd->page)));
			gdk_threads_enter();
			gtk_entry_set_text(GTK_ENTRY(entries.extended_revision_entry),tmpbuf);
			gdk_threads_leave();
			g_free(tmpbuf);
		}

	}
	if (match == FALSE) // (we DID NOT find one)
	{
		tmpbuf = g_strdup_printf("Firmware NOT DETECTED properly, Expect MegaTunix to NOT behave properly \nContact the author with the contents of this window\n");
		dbg_func(g_strdup_printf(__FILE__":\n\tdetermine_ecu() Firmware NOT DETECTED, send contents of the\ninterrogation window and the firmware details to the MegaTunix author\n"),CRITICAL);
		gdk_threads_enter();
		update_logbar(interr_view,"warning",tmpbuf,FALSE,FALSE);
		gdk_threads_leave();
		g_free(tmpbuf);
		goto cleanup;
	}

	load_profile_details(potential);

	load_lookuptables(potential);
	/* Set flags */
	ecu_caps = potential->capabilities;
	/* Enable/Disable Controls */
	parse_ecu_capabilities(ecu_caps);

	/* Set expected sizes for commands */
	if (!firmware)
		firmware = g_new0(struct Firmware_Details,1);
	firmware->name = g_strdup(potential->name);
	firmware->tab_list = g_strsplit(potential->load_tabs,",",0);
	firmware->multi_page = potential->multi_page;
	firmware->total_pages = potential->total_pages;

	for (i=0;i<firmware->total_pages;i++)
	{
		firmware->page_params[i] = g_new0(struct Page_Params, 1);
				
		if (potential->page_params[i]->is_spark)
		{
			firmware->page_params[i]->size = (gint)g_hash_table_lookup(
					potential->bytecounts,
					g_strdup("CMD_I_0"));
		}
		else
		{
			firmware->page_params[i]->size = (gint)g_hash_table_lookup(
					potential->bytecounts,
					g_strdup_printf("CMD_V_%i",i));
		}
		firmware->page_params[i]->ve_base = potential->page_params[i]->ve_base;
		firmware->page_params[i]->rpm_base = potential->page_params[i]->rpm_base;
		firmware->page_params[i]->load_base = potential->page_params[i]->load_base;
		firmware->page_params[i]->rpm_bincount = potential->page_params[i]->rpm_bincount;
		firmware->page_params[i]->load_bincount = potential->page_params[i]->load_bincount;
		firmware->page_params[i]->is_spark = potential->page_params[i]->is_spark;
	}

	/* use commands defined in the interogation profile to map the proper
	 * command to the I/O routines...  Looks ugly but should (hopefully)
	 * be more flexible in the long run....
	 */

	if (potential->rt_cmd_key != NULL)
	{
		cmd = (struct Command *)g_hash_table_lookup(cmd_details,potential->rt_cmd_key);
		cmds->realtime_cmd = g_strdup(cmd->string);
		cmds->rt_cmd_len = cmd->len;
		firmware->rtvars_size = (gint)g_hash_table_lookup(
				potential->bytecounts,g_strdup_printf("CMD_%s_0",cmds->realtime_cmd));
	}
	else
	{
		dbg_func("Realtime Read cmd is NOT defined in interrogation profile, using hardcoded default\n",CRITICAL);
		cmds->realtime_cmd = g_strdup("A");
		cmds->rt_cmd_len = 1;
		firmware->rtvars_size = 22;
	}
	/* VE/Constants */
	if (potential->ve_cmd_key != NULL)
	{
		cmd = (struct Command *)g_hash_table_lookup(cmd_details,potential->ve_cmd_key);
		cmds->veconst_cmd = g_strdup(cmd->string);
		cmds->ve_cmd_len = cmd->len;
	}
	else
	{
		dbg_func("VE/Constants Read cmd is NOT defined in interrogation profile, using hardcoded default\n",CRITICAL);
		cmds->veconst_cmd = g_strdup("V");
		cmds->ve_cmd_len = 1;
	}
	/* Ignition vars */
	if (potential->ign_cmd_key != NULL)
	{
		cmd = (struct Command *)g_hash_table_lookup(cmd_details,potential->ign_cmd_key);
		cmds->ignition_cmd = g_strdup(cmd->string);
		cmds->ign_cmd_len = cmd->len;
	}
	else
	{
		dbg_func("Ignition Read cmd is NOT defined in interrogation profile, using hardcoded default\n",CRITICAL);
		cmds->ignition_cmd = g_strdup("I");
		cmds->ign_cmd_len = 1;
	}

	/* Raw memory */
	if (potential->raw_mem_cmd_key != NULL)
	{
		cmd = (struct Command *)g_hash_table_lookup(cmd_details,potential->raw_mem_cmd_key);
		cmds->raw_mem_cmd = g_strdup(cmd->string);
		cmds->raw_mem_cmd_len = cmd->len;
		firmware->memblock_size = (gint)g_hash_table_lookup(
				potential->bytecounts,g_strdup_printf("CMD_%s_0",cmds->raw_mem_cmd));
	}
	else
	{
		dbg_func("Raw Memory Read cmd is NOT defined in interrogation profile, using hardcoded default\n",CRITICAL);
		cmds->raw_mem_cmd = g_strdup("F");
		cmds->raw_mem_cmd_len = 1;
		firmware->memblock_size = 256;
	}


	/* Display firmware version in the window... */
	tmpbuf = g_strdup_printf("Detected Firmware: %s\n",potential->name);

	dbg_func(g_strdup_printf(__FILE__": determine_ecu() Detected Firmware: %s\n",potential->name),INTERROGATOR);
	gdk_threads_enter();
	update_logbar(interr_view,"warning",tmpbuf,FALSE,FALSE);
	gdk_threads_leave();
	g_free(tmpbuf);
	goto freeup;


cleanup:
	/* Set flags */
	ecu_caps = 0;				// no flags 

	/* Enable/Disable Controls */
	parse_ecu_capabilities(ecu_caps);

	if (!firmware)
	{
		firmware = g_malloc0(sizeof(struct Firmware_Details));
		firmware->page_params[0] = g_new(struct Page_Params,1);
	}
	
	firmware->tab_list = g_strsplit("",",",0);
	firmware->page_params[0]->size = 125;	/* assumptions!!!! */
	firmware->rtvars_size = 22;	/* assumptions!!!! */

freeup:
	if (canidate->sig_str)
		g_free(canidate->sig_str);
	if (canidate->quest_str)
		g_free(canidate->quest_str);
	return;

}

GArray * validate_and_load_tests(GHashTable *cmd_details)
{
	ConfigFile *cfgfile;
	GArray * cmd_array = NULL;
	gchar * filename;
	gchar *section = NULL;
	gchar * tmpbuf;
	gint total_tests = 0;
	gint i = 0;
	gboolean tmpi = FALSE;
	struct Command *cmd = NULL;

	filename = g_strconcat(DATA_DIR,"/",INTERROGATOR_DIR,"/","tests",NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{	
		dbg_func(g_strdup_printf(__FILE__":\n\tfile %s,\n\topened successfully\n",filename),INTERROGATOR);
		cmd_array = g_array_new(FALSE,FALSE,sizeof(struct Command *));
		cfg_read_int(cfgfile,"interrogation_tests","total_tests",&total_tests);
		for (i=0;i<total_tests;i++)
		{
			cmd = g_new0(struct Command, 1);
			cmd->multipart=FALSE;
			section = g_strdup_printf("test_%.2i",i);
			cfg_read_string(cfgfile,section,"raw_cmd",
					&cmd->string);
			cfg_read_int(cfgfile,section,"cmd_length",
					&cmd->len);
			cfg_read_string(cfgfile,section,"cmd_desc",
					&cmd->desc);
			cfg_read_boolean(cfgfile,section,"multipart",&cmd->multipart);
			if (cmd->multipart)
				cfg_read_int(cfgfile,section,"cmd_int_arg",
						&cmd->cmd_int_arg);
					
			cfg_read_int(cfgfile,section,"page",
					&cmd->page);
			cfg_read_boolean(cfgfile,section,"store_data",
					&tmpi);
			if (tmpi)
			{
				cfg_read_string(cfgfile,section,"store_type",
						&tmpbuf);
				cmd->store_type = translate_string(tmpbuf);
				g_free(tmpbuf);
			}

			g_free(section);
			g_array_insert_val(cmd_array,i,cmd);
			g_hash_table_insert(cmd_details,g_strdup_printf("CMD_%s_%i",cmd->string,cmd->page),cmd);
		}
		cfg_free(cfgfile);
		
		
	}
	else
	{
		dbg_func(g_strdup_printf(__FILE__":\n\tfailure opening \"%s\" file\nMegaTunix was NOT installed!!!! \nPlease run \"make install\" from the top level MegaTunix dir\n",filename),CRITICAL);
		exit(-1);
	}

	g_free(filename);
	return cmd_array;
}

void free_test_commands(GArray * cmd_array)
{
	struct Command *cmd = NULL;
	gint i = 0;
	for (i=0;i<cmd_array->len;i++)
	{
		cmd = g_array_index(cmd_array,struct Command *,i);
		if (cmd->string)
			g_free(cmd->string);
		if (cmd->desc)
			g_free(cmd->desc);
		g_free(cmd);
	}
	g_array_free(cmd_array,TRUE);
}

void close_profile(void *ptr)
{
	struct Canidate *canidate = (struct Canidate *) ptr;
	gint i = 0;
	
	dbg_func(__FILE__": close_profile(),\n\tdeallocating memory for potential canidate match\n",INTERROGATOR);
	for (i=0;i<(canidate->total_pages);i++)
		if (canidate->page_params[i])
			g_free(canidate->page_params[i]);
	if (canidate->sig_str)
		g_free(canidate->sig_str);
	if (canidate->quest_str)
		g_free(canidate->quest_str);
	if (canidate->name)
		g_free(canidate->name);
	if (canidate->bytecounts)
		g_hash_table_destroy(canidate->bytecounts);
	g_free(canidate);
	dbg_func(__FILE__": close_profile(),\n\tdeallocation of memory for potential canidate complete\n",INTERROGATOR);
	
}

		
void * load_potential_match(GArray * cmd_array, gchar * filename)
{
	ConfigFile *cfgfile;
	struct Canidate *canidate = NULL;

	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{	
		canidate = g_malloc0(sizeof(struct Canidate));
		canidate->filename = g_strdup(filename);
		dbg_func(g_strdup_printf(__FILE__": load_potential_match() file:\n\t%s\n\topened successfully\n",filename),INTERROGATOR);
		canidate->bytecounts = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		cfg_read_string(cfgfile,"interrogation_profile","name",&canidate->name);
		load_bytecounts(cmd_array, canidate->bytecounts, (void *)cfgfile);
		cfg_read_string(cfgfile,"parameters","SignatureQueryString",
				&canidate->sig_str);
		cfg_read_string(cfgfile,"parameters","ExtVerQueryString",
				&canidate->quest_str);
		cfg_read_int(cfgfile,"parameters","VerNumber",
				&canidate->ver_num);

		cfg_free(cfgfile);
		g_free(filename);

	}
	else
	{
		dbg_func(g_strdup_printf(__FILE__": load_mini_profile() failure opening file:\n\t%s\n",filename),CRITICAL);
		g_free(filename);
	}
	return canidate;
}

		
void load_profile_details(void *ptr)
{
	ConfigFile *cfgfile;
	gchar * tmpbuf;
	gchar * filename;
	gchar * section;
	gint i = 0;
	struct Canidate *canidate = ptr;

	filename = g_strdup(canidate->filename);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{	
		dbg_func(g_strdup_printf(__FILE__": load_profile_details() file:\n\t%s\n\topened successfully\n",filename),INTERROGATOR);
		if(!cfg_read_string(cfgfile,"parameters","Rt_Cmd_Key",
				&canidate->rt_cmd_key))
			dbg_func(__FILE__": load_profile_details(), \"Rt_Cmd_Key\" variable not found in interrogation profile, ERROR\n",CRITICAL);
		if(!cfg_read_string(cfgfile,"parameters","VE_Cmd_Key",
				&canidate->ve_cmd_key))
			dbg_func(__FILE__": load_profile_details(), \"VE_Cmd_Key\" variable not found in interrogation profile, ERROR\n",CRITICAL);
		if(!cfg_read_string(cfgfile,"parameters","Ign_Cmd_Key",
				&canidate->ign_cmd_key))
			dbg_func(__FILE__": load_profile_details(), \"Ign_Cmd_Key\" variable not found in interrogation profile, ERROR\n",CRITICAL);
		if(!cfg_read_string(cfgfile,"parameters","Raw_Mem_Cmd_Key",
				&canidate->raw_mem_cmd_key))
			dbg_func(__FILE__": load_profile_details(), \"Raw_Mem_Cmd_Key\" variable not found in interrogation profile, ERROR\n",CRITICAL);

		if(!cfg_read_boolean(cfgfile,"parameters","MultiPage",
				&canidate->multi_page))
			dbg_func(__FILE__": load_profile_details(), \"MultiPage\" flag not found in interrogation profile, ERROR\n",CRITICAL);
		if(!cfg_read_int(cfgfile,"parameters","TotalPages",
				&canidate->total_pages))
			dbg_func(__FILE__": load_profile_details(), \"TotalPages\" value not found in interrogation profile, ERROR\n",CRITICAL);
		if(!cfg_read_string(cfgfile,"parameters","Capabilities",
				&tmpbuf))
			dbg_func(__FILE__": load_profile_details(), \"Capabilities\" enumeration list not found in interrogation profile, ERROR\n",CRITICAL);
		else
		{
			canidate->capabilities = translate_capabilities(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,"gui","LoadTabs",
				&canidate->load_tabs))
			dbg_func(__FILE__": load_profile_details(), \"LoadTabs\" list not found in interrogation profile, ERROR\n",CRITICAL);
		if(!cfg_read_string(cfgfile,"gui","RealtimeMapFile",
				&canidate->rtv_map_file))
//			dbg_func(__FILE__": load_profile_details(), \"RealtimeMapFile\" variable not found in interrogation profile, ERROR\n",CRITICAL);
		if (!cfg_read_string(cfgfile,"lookuptables","mat",
				&canidate->mat_tbl_name))
			dbg_func(__FILE__": load_profile_details(), \"MAT\" lookuptable name not found in interrogation profile, ERROR\n",CRITICAL);
		if(!cfg_read_string(cfgfile,"lookuptables","clt",
				&canidate->clt_tbl_name))
			dbg_func(__FILE__": load_profile_details(), \"CLT\" lookuptable name not found in interrogation profile, ERROR\n",CRITICAL);


		/* Allocate space Table Offsets structures.... */
		for (i=0;i<canidate->total_pages;i++)
		{
			canidate->page_params[i] = g_new0(struct Page_Params,1);
			section = g_strdup_printf("page_%i",i);
			if(!cfg_read_int(cfgfile,section,"ve_base_offset",
					&canidate->page_params[i]->ve_base))
				dbg_func(__FILE__": load_profile_details(), \"ve_base_offset\" variable not found in interrogation profile, ERROR\n",CRITICAL);
			if(!cfg_read_int(cfgfile,section,"rpm_base_offset",
					&canidate->page_params[i]->rpm_base))
				dbg_func(__FILE__": load_profile_details(), \"ve_base_offset\" variable not found in interrogation profile, ERROR\n",CRITICAL);
			if(!cfg_read_int(cfgfile,section,"load_base_offset",
					&canidate->page_params[i]->load_base))
				dbg_func(__FILE__": load_profile_details(), \"load_base_offset\" variable not found in interrogation profile, ERROR\n",CRITICAL);
			if(!cfg_read_int(cfgfile,section,"rpm_bincount",
					&canidate->page_params[i]->rpm_bincount))
				dbg_func(__FILE__": load_profile_details(), \"rpm_bincount\" variable not found in interrogation profile, ERROR\n",CRITICAL);
			if(!cfg_read_int(cfgfile,section,"load_bincount",
					&canidate->page_params[i]->load_bincount))
				dbg_func(__FILE__": load_profile_details(), \"load_bincount\" variable not found in interrogation profile, ERROR\n",CRITICAL);
			if(!cfg_read_boolean(cfgfile,section,"is_spark",
					&canidate->page_params[i]->is_spark))
				dbg_func(__FILE__": load_profile_details(), \"is_spark\" flag not found in interrogation profile, ERROR\n",CRITICAL);
			g_free(section);
		}

		cfg_free(cfgfile);
		g_free(filename);

	}
	else
	{
		dbg_func(g_strdup_printf(__FILE__": load_profile_details() failure opening file:\n\t%s\n",filename),CRITICAL);
		g_free(filename);
	}
}

void load_bytecounts(GArray *cmd_array, GHashTable *hash, void * input)
{
	struct Command *cmd = NULL;
	gint i = 0;
	gint bytecount = -2;
	gchar * tmpbuf = NULL;
	ConfigFile *cfgfile = (ConfigFile *) input;

	for (i=0;i<cmd_array->len;i++)
	{
		bytecount = -1;
		cmd = g_array_index(cmd_array,struct Command *, i);
		tmpbuf = g_strdup_printf("CMD_%s_%i",cmd->string,cmd->page);
		if(!cfg_read_int(cfgfile,"bytecounts",tmpbuf,
				&bytecount))
			dbg_func(g_strdup_printf(__FILE__": load_bytecounts(), \"%s\" key not found in interrogation profile, ERROR\n",tmpbuf),CRITICAL);
		else
		{
			g_hash_table_insert(hash, g_strdup(tmpbuf),GINT_TO_POINTER(bytecount));
			dbg_func(g_strdup_printf(__FILE__": load_bytecounts() inserting key %s, val %i\n",tmpbuf,bytecount),INTERROGATOR);
			g_free(tmpbuf);
		}
	}	

}

gint translate_capabilities(gchar *string)
{
	gchar **vector;
	gint i = 0;
	gint value = 0;
	

	vector = g_strsplit(string,",",0);
	dbg_func(g_strdup_printf(__FILE__": translate_capabilities() string fed is %s\n",string),INTERROGATOR);
	while (vector[i] != NULL)
	{
		dbg_func(g_strdup_printf(__FILE__": translate_capabilities() trying to translate %s\n",vector[i]),INTERROGATOR);
		value += translate_string(vector[i]);
		dbg_func(g_strdup_printf(__FILE__": translate_capabilities() translated value of %s is %i\n",vector[i],value),INTERROGATOR);
		i++;
	}

	g_strfreev(vector);
	return value;	
}

gboolean check_for_match(GArray *cmd_array, void *pot_ptr, void *can_ptr)
{
	struct Canidate *potential = (struct Canidate *)pot_ptr;
	struct Canidate *canidate = (struct Canidate *)can_ptr;
	gint num_tests = cmd_array->len;
	gint cbytes = 0;
	gint pbytes = 0;
	struct Command *cmd = NULL;
	
	gint j = 0;

	for (j=0;j<num_tests;j++)
	{
		cmd = g_array_index(cmd_array,struct Command *, j);
		cbytes = (gint)g_hash_table_lookup(
				canidate->bytecounts,
				g_strdup_printf("CMD_%s_%i",
					cmd->string,cmd->page));
		pbytes = (gint)g_hash_table_lookup(
				potential->bytecounts,
				g_strdup_printf("CMD_%s_%i",
					cmd->string,cmd->page));
		dbg_func(g_strdup_printf(__FILE__": determine_ecu() Test %s, canidate bytes %i, potential %i\n",cmd->string,cbytes,pbytes),INTERROGATOR);
		if (cbytes != pbytes)
		{
			// Mismatch, abort test and move on to the 
			// next one...
			dbg_func(g_strdup_printf(__FILE__": determine_ecu(), counts don't match, loading next profile\n"),INTERROGATOR);
			close_profile(potential);
			return FALSE;
		}
	}
	dbg_func(g_strdup_printf(__FILE__": determine_ecu() all bytecount tests passed for firmware %s\n",potential->name),INTERROGATOR);
	/* If all test pass, now check the Extended version
	 * If it matches,  jump out...
	 */
	if ((potential->quest_str != NULL) && (canidate->quest_str != NULL))
	{
		dbg_func(g_strdup_printf(__FILE__": determine_ecu() testing ext version, canidate %s, potential %s\n",canidate->quest_str,potential->quest_str),INTERROGATOR);
		if (strstr(canidate->quest_str,potential->quest_str) != NULL)
		{
			dbg_func(__FILE__": determine_ecu() found match on ext version\n",INTERROGATOR);
			return TRUE;
		}
	}
	else if ((potential->sig_str != NULL) && (canidate->sig_str != NULL))
	{
		dbg_func(g_strdup_printf(__FILE__": determine_ecu() testing signature, canidate %s, potential %s\n",canidate->sig_str,potential->sig_str),INTERROGATOR);
		if (strstr(canidate->sig_str,potential->sig_str) != NULL)
		{
			dbg_func(__FILE__": determine_ecu() found match on signature\n",INTERROGATOR);
			return TRUE;
		}
	}
	else
	{
		dbg_func(g_strdup_printf(__FILE__": determine_ecu() found match on bytecounts alone...\n"),INTERROGATOR);
		return TRUE;
	}
	return TRUE;
}
