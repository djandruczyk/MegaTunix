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
#include <getfiles.h>
#include <init.h>
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
#include <threads.h>
#include <unistd.h>

extern gint ecu_caps;
extern gboolean connected;
extern GtkWidget *ms_ecu_revision_entry;
extern GtkTextBuffer *textbuffer;
extern GtkWidget *interr_view;
extern struct Serial_Params *serial_params;
struct Firmware_Details *firmware = NULL;
gboolean interrogated = FALSE;


/*!
 \brief interrogate_ecu() interrogates the target ECU to determine what
 firmware it is running.  It does this by reading a list of tests, sending
 those tests in turn, reading the responses and them comparing the group of
 responses against a list of interrogation profiles until it finds a match.
 */
void interrogate_ecu()
{
	struct Command *cmd = NULL;
	struct Canidate *canidate = NULL;
	extern GHashTable *dynamic_widgets;
	gint size = 4096;
	gint res = 0;
	gint count = 0;
	gint i = 0;
	gint j = 0;
	gint tests_to_run = 0;
	gint total_read = 0;
	gint total_wanted = 0;
	//gint read_amount = 0;
	gint zerocount = 0;
	gchar *string = NULL;
	GArray *cmd_array = NULL;
	guchar buf[size];
	guchar *ptr = NULL;
	guchar *p = NULL;
	GHashTable *cmd_details = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	/* prevent multiple runs of interrogator simultaneously */
	g_static_mutex_lock(&mutex);
	dbg_func(g_strdup("\n"__FILE__": interrogate_ecu() ENTERED\n\n"),INTERROGATOR);

	if (!connected)
	{
		dbg_func(g_strdup(__FILE__": interrogate_ecu()\n\tNOT connected to ECU!!!!\n"),CRITICAL);
		g_static_mutex_unlock(&mutex);
		return;
	}


	/* Allocate memory to store interrogation results */
	canidate = g_malloc0(sizeof(struct Canidate));
	canidate->bytecounts = g_hash_table_new(g_str_hash,g_str_equal);

	cmd_details = g_hash_table_new(g_str_hash,g_str_equal);

	/* Load tests from config files */
	cmd_array = validate_and_load_tests(cmd_details);
	if (!cmd_array)
	{
		dbg_func(g_strdup(__FILE__": interrogate_ecu()\n\t validate_and_load_tests() didn't return a valid list of commands\n\t MegaTunix was NOT installed correctly, Aborting Interrogation\n"),CRITICAL);
		g_static_mutex_unlock(&mutex);
		return;
	}
	/* how many tests.... */
	tests_to_run = cmd_array->len;

	for (i=0;i<tests_to_run;i++)
	{
		flush_serial(serial_params->fd,TCIOFLUSH);
		count = 0;
		cmd = g_array_index(cmd_array,struct Command *, i);

		/* flush buffer to known state.. */
		memset (buf,0,size);

		ptr = buf;

		string = g_strdup(cmd->string);
		res = write(serial_params->fd,string,cmd->len);
		if (res != cmd->len)
			dbg_func(g_strdup(__FILE__": interrogate_ecu()\n\tError writing data to the ECU\n"),CRITICAL);
		else
			dbg_func(g_strdup_printf("\tSent command \"%s\"\n",string),INTERROGATOR);
		if (cmd->multipart)
		{
			res = write(serial_params->fd,&cmd->cmd_int_arg,1);
			if (res != 1)
				dbg_func(g_strdup(__FILE__": interrogate_ecu()\n\tError writing data to the ECU\n"),CRITICAL);
			else
				dbg_func(g_strdup_printf("\tSent argument \"%i\"\n",cmd->cmd_int_arg),INTERROGATOR);
		}
		g_free(string);

		total_read = 0;
		total_wanted = size;
		zerocount = 0;
		while (total_read < total_wanted )
		{
			dbg_func(g_strdup_printf("\tInterrogation for command %s requesting %i bytes\n",cmd->string,total_wanted-total_read),INTERROGATOR);
			//			read_amount = (total_wanted-total_read) > 8? 8:total_wanted-total_read;
			total_read += res = read(serial_params->fd,
					ptr+total_read,
					//	read_amount);
				   total_wanted-total_read);

			dbg_func(g_strdup_printf("\tInterrogation for command %s read %i bytes, running total %i\n",cmd->string,res,total_read),INTERROGATOR);
			// If we get nothing back (i.e. timeout, assume done)
			if (res == 0)
				zerocount++;
			//				break;

			if (zerocount > 1)
				break;

		}


		if (total_read > 0)
		{
			thread_update_logbar("interr_view",NULL,g_strdup_printf("Command \"%s\" (%s), returned %i bytes\n",cmd->string, cmd->desc,total_read),FALSE,FALSE);

			dbg_func(g_strdup_printf(__FILE__": interrogate_ecu()\n\tRead the following from the %s command\n",cmd->string),SERIAL_RD);
			dbg_func(g_strdup("Data is in HEX!!\n"),SERIAL_RD);
			p = buf;
			for (j=0;j<total_read;j++)
			{
				dbg_func(g_strdup_printf("%.2x ", p[j]),SERIAL_RD);
				if (!((j+1)%8))
					dbg_func(g_strdup("\n"),SERIAL_RD);
			}
			dbg_func(g_strdup("\n\n"),SERIAL_RD);
		}

		dbg_func(g_strdup_printf("\tReceived %i bytes\n",total_read),INTERROGATOR);
		ptr = buf;

		/* copy data from tmp buffer to struct pointer */
		switch (cmd->store_type)
		{
			case SIG:
				if (total_read > 0)
					canidate->sig_str = g_strndup(buf,total_read);
				else
					canidate->sig_str = NULL;
				break;
			case EXTVER:
				if (total_read > 0)
					canidate->quest_str = g_strndup(buf,total_read);
				else
					canidate->quest_str = NULL;
				break;
			case VNUM:
				if (total_read > 0)
				{
					canidate->ver_num = buf[0];
				//	memcpy(&(canidate->ver_num),buf,total_read);
				}
				else 
					canidate->ver_num = 0;
				break;
			default:
				break;
		}

		/* store number of bytes received in counter variable */
		g_hash_table_insert(canidate->bytecounts,g_strdup(cmd->key),
				GINT_TO_POINTER(total_read));
	}

	interrogated = determine_ecu(canidate,cmd_array,cmd_details);	
	if (interrogated)
		set_widget_sensitive(g_hash_table_lookup(dynamic_widgets,"offline_button"),FALSE);

	if (canidate)
		g_free(canidate);

	free_test_commands(cmd_array);
	g_hash_table_destroy(cmd_details);

	g_static_mutex_unlock(&mutex);
	dbg_func(g_strdup("\n"__FILE__": interrogate_ecu() LEAVING\n\n"),INTERROGATOR);

	return;
}


/*!
 \brief determine_ecu() trys to match determine the target firmware by 
 loading the interrogation profiles in turn and comparing the data from our
 test ECU adn a profile until a match is found, 
 \param canidate (struct Canidate *) pointer to the Canidate structure
 \param cmd_array (GArray *) pointer to the array of commands sent
 \param cmd_details (GHashTable) details on the interrogation process with
 the target ECU
 \returns TRUE on successfull interrogation, FALSE on no match
 */
gboolean determine_ecu(struct Canidate *canidate, GArray *cmd_array, GHashTable *cmd_details)
{
	struct Canidate *potential = NULL;
	struct Command *cmd = NULL;
	gint i = 0;
	gint num_tests = cmd_array->len;
	gboolean match = FALSE;
	gchar ** filenames = NULL;
	extern struct Io_Cmds *cmds;

	filenames = get_files(g_strconcat(INTERROGATOR_DIR,"/Profiles/",NULL));	
	if (!filenames)
	{
		dbg_func(g_strdup(__FILE__": determine_ecu()\n\t NO Interrogation profiles found,  was MegaTunix installed properly?\n\n"),CRITICAL);
		return FALSE;
	}

	while (filenames[i])
	{
		potential = load_potential_match(cmd_array,filenames[i]);
		if (check_for_match(cmd_array,potential,canidate))
		{
			match = TRUE;
			break;
		}
		i++;
	}
	g_strfreev(filenames);
	/* Update the screen with the data... */
	for (i=0;i<num_tests;i++)
	{
		cmd = g_array_index(cmd_array,struct Command *,i);
		dbg_func(g_strdup_printf("\tCommand \"%s\" (%s), returned %i bytes\n",
					cmd->string,
					cmd->desc,
					(gint) g_hash_table_lookup(canidate->bytecounts, cmd->key)),INTERROGATOR);
		if (cmd->store_type == VNUM)
		{
			if (canidate->ver_num == 0)
				thread_update_widget(g_strdup("ecu_revision_entry"),MTX_ENTRY,g_strdup(""));
			else
			thread_update_widget(g_strdup("ecu_revision_entry"),MTX_ENTRY,g_strdup_printf("%.1f",(float)canidate->ver_num/10.0));

		}
		if (cmd->store_type == SIG)
		{
			if (canidate->sig_str == NULL)
				thread_update_widget(g_strdup("ecu_signature_entry"),MTX_ENTRY,g_strdup(""));
			else
				thread_update_widget(g_strdup("ecu_signature_entry"),MTX_ENTRY,g_strndup(canidate->sig_str,(gint)g_hash_table_lookup(canidate->bytecounts, cmd->key)));

		}
		if (cmd->store_type == EXTVER)
		{
			if (canidate->quest_str == NULL)
				thread_update_widget(g_strdup("ext_revision_entry"),MTX_ENTRY,g_strdup(""));
			else
				thread_update_widget(g_strdup("ext_revision_entry"),MTX_ENTRY,g_strndup(canidate->quest_str,(gint)g_hash_table_lookup(canidate->bytecounts, cmd->key)));


		}

	}
	if (match == FALSE) // (we DID NOT find one)
	{
		dbg_func(g_strdup(__FILE__":\n\tdetermine_ecu()\n\tFirmware NOT DETECTED, send contents of the interrogation window\n\tand the firmware details to the MegaTunix author\n"),CRITICAL);
		thread_update_logbar("interr_view","warning",g_strdup("Firmware NOT DETECTED properly, Expect MegaTunix to NOT behave properly \nContact the author with the contents of this window\n"),FALSE,FALSE);
		goto freeup;
	}

	load_profile_details(potential);
	load_lookuptables(potential);
	/* Set flags */
	ecu_caps = potential->capabilities;

	/* Set expected sizes for commands */
	if (!firmware)
		firmware = g_new0(struct Firmware_Details,1);

	firmware->name = g_strdup(potential->name);
	firmware->tab_list = g_strsplit(potential->load_tabs,",",0);
	firmware->tab_confs = g_strsplit(potential->tab_confs,",",0);
	firmware->rtv_map_file = g_strdup(potential->rtv_map_file);
	firmware->sliders_map_file = g_strdup(potential->sliders_map_file);
	firmware->multi_page = potential->multi_page;
	firmware->require_page = potential->require_page;
	firmware->total_tables = potential->total_tables;
	firmware->total_pages = potential->total_pages;
	firmware->write_cmd = g_strdup(potential->write_cmd);
	firmware->burn_cmd = g_strdup(potential->burn_cmd);
	firmware->page_cmd = g_strdup(potential->page_cmd);

	/* Allocate RAM for the Req_Fuel_Params structures. */
	firmware->rf_params = g_new0(struct Req_Fuel_Params *,firmware->total_tables);

	/* Allocate RAM for the Table_Params structures and copy data in.. */
	firmware->table_params = g_new0(struct Table_Params *,firmware->total_tables);
	for (i=0;i<firmware->total_tables;i++)
	{
		firmware->rf_params[i] = g_new0(struct Req_Fuel_Params ,1);
		firmware->table_params[i] = initialize_table_params();
		memcpy(firmware->table_params[i],potential->table_params[i],sizeof(struct Table_Params));
	}

	/* Allocate RAM for the Page_Params structures and copy data in.. */
	firmware->page_params = g_new0(struct Page_Params *,firmware->total_pages);
	for (i=0;i<firmware->total_pages;i++)
	{
		firmware->page_params[i] = initialize_page_params();
		memcpy(firmware->page_params[i],potential->page_params[i],sizeof(struct Page_Params));
	}

	mem_alloc();

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
				potential->bytecounts,cmd->key);
	}
	else
		dbg_func(g_strdup(__FILE__": determine_ecu()\n\tRead cmd is NOT defined in interrogation profile,\n\tCRITICAL ERROR, expect imminent crash\n\n"),CRITICAL);
	/* VE/Constants */
	if (potential->ve_cmd_key != NULL)
	{
		cmd = (struct Command *)g_hash_table_lookup(cmd_details,potential->ve_cmd_key);
		cmds->veconst_cmd = g_strdup(cmd->string);
		cmds->ve_cmd_len = cmd->len;
	}
	else
		dbg_func(g_strdup(__FILE__": determine_ecu()\n\tVE/Constants cmd is NOT defined in interrogation profile,\n\tCRITICAL ERROR, expect imminent crash\n\n"),CRITICAL);
	/* Ignition vars */
	if (potential->ign_cmd_key != NULL)
	{
		cmd = (struct Command *)g_hash_table_lookup(cmd_details,potential->ign_cmd_key);
		cmds->ignition_cmd = g_strdup(cmd->string);
		cmds->ign_cmd_len = cmd->len;
	}

	/* Raw memory */
	if (potential->raw_mem_cmd_key != NULL)
	{
		cmd = (struct Command *)g_hash_table_lookup(cmd_details,potential->raw_mem_cmd_key);
		cmds->raw_mem_cmd = g_strdup(cmd->string);
		cmds->raw_mem_cmd_len = cmd->len;
		firmware->memblock_size = (gint)g_hash_table_lookup(
				potential->bytecounts,cmd->key);
	}


	/* Display firmware version in the window... */

	dbg_func(g_strdup_printf(__FILE__": determine_ecu()\n\tDetected Firmware: %s\n",potential->name),INTERROGATOR);
	thread_update_logbar("interr_view","warning",g_strdup_printf("Detected Firmware: %s\n",potential->name),FALSE,FALSE);
	goto freeup;


freeup:
	if (canidate->sig_str)
		g_free(canidate->sig_str);
	if (canidate->quest_str)
		g_free(canidate->quest_str);
	if (match)
		return TRUE;
	else
		return FALSE;

}


/*!
 \brief validate_and_load_tests() loads the list of tests from the system
 checks them for validity, populates and array and returns it
 \param cmd_details (GHashTable *) a hashtable where details regarding each 
 command tested against the ECU arestored
 \returns a dynamic GArray for commands
 */
GArray * validate_and_load_tests(GHashTable *cmd_details)
{
	ConfigFile *cfgfile;
	GArray * cmd_array = NULL;
	gchar * filename = NULL;
	gchar *section = NULL;
	gchar * tmpbuf = NULL;
	gint total_tests = 0;
	gint i = 0;
	gboolean tmpi = FALSE;
	struct Command *cmd = NULL;

	filename = get_file(g_strconcat(INTERROGATOR_DIR,"/","tests",NULL));
	if (!filename)
		return NULL;

	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{	
		dbg_func(g_strdup_printf(__FILE__": validate_and_load_tests()\n\tfile %s, opened successfully\n",filename),INTERROGATOR);
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
			if (g_ascii_islower(*(cmd->string)))
			{
				cmd->key = g_strdup_printf("CMD_LOWER_%s",g_ascii_strup(cmd->string,cmd->len));
				g_hash_table_insert(cmd_details,g_strdup(cmd->key),cmd);
			}
			else
			{
				cmd->key = g_strdup_printf("CMD_UPPER_%s",cmd->string);
				g_hash_table_insert(cmd_details,g_strdup(cmd->key),cmd);
			}
			g_array_insert_val(cmd_array,i,cmd);
		}
		cfg_free(cfgfile);
	}

	g_free(filename);
	return cmd_array;
}


/*!
 \brief free_test_commands() deallocates the data in the cmd_array array
 */
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
		if (cmd->key)
			g_free(cmd->key);
		g_free(cmd);
	}
	g_array_free(cmd_array,TRUE);
}

/*!
 \brief close_profile() closes the interrogation profile and deallocates and
 memory associated with it..
 \param canidate (struct Canidate *) pointer to the canidate structure
 */
void close_profile(struct Canidate *canidate)
{
	gint i = 0;

	dbg_func(g_strdup(__FILE__": close_profile(),\n\tDeallocating memory for potential canidate match\n"),INTERROGATOR);
	for (i=0;i<(canidate->total_pages);i++)
		if (canidate->page_params[i])
			g_free(canidate->page_params[i]);
	for (i=0;i<(canidate->total_tables);i++)
		if (canidate->table_params[i])
			dealloc_table_params(canidate->table_params[i]);
	if (canidate->sig_str)
		g_free(canidate->sig_str);
	if (canidate->quest_str)
		g_free(canidate->quest_str);
	if (canidate->name)
		g_free(canidate->name);
	if (canidate->bytecounts)
		g_hash_table_destroy(canidate->bytecounts);
	g_free(canidate);
	dbg_func(g_strdup(__FILE__": close_profile(),\n\tDeallocation of memory for potential canidate complete\n"),INTERROGATOR);

}


/*! 
 \brief load_potential_match() loads an interrogation profile and loads only
 the essentials on if needed to perform an interrogation against it.
 \param cmd_array (GArray *) array of commands to load bytecounts for from
 the interrogation profile
 \param filename (gchr *) the name of the interrogation profile filename
 \returns a pointer to a Canidate structure
 */
struct Canidate * load_potential_match(GArray * cmd_array, gchar * filename)
{
	ConfigFile *cfgfile;
	struct Canidate *canidate = NULL;

	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{	
		canidate = initialize_canidate();
		canidate->filename = g_strdup(filename);
		dbg_func(g_strdup_printf(__FILE__": load_potential_match()\n\tfile:%s opened successfully\n",filename),INTERROGATOR);
		canidate->bytecounts = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		cfg_read_string(cfgfile,"interrogation_profile","name",&canidate->name);
		load_bytecounts(cmd_array, canidate->bytecounts, cfgfile);
		if (!cfg_read_string(cfgfile,"parameters","SignatureQueryString",&canidate->sig_str))
			dbg_func(g_strdup(__FILE__": load_potential_match()\n\t\"SignatureQueryString\" NOT found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_string(cfgfile,"parameters","ExtVerQueryString",&canidate->quest_str))
			dbg_func(g_strdup(__FILE__": load_potential_match()\n\t\"ExtVerQueryString\" NOT found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_int(cfgfile,"parameters","VerNumber",&canidate->ver_num))
			dbg_func(g_strdup(__FILE__": load_potential_match()\n\t\"VerNumber\" NOT found in interrogation profile, ERROR\n"),CRITICAL);

		cfg_free(cfgfile);

	}
	else
	{
		dbg_func(g_strdup_printf(__FILE__": load_potential_match()\n\tfailure opening file:\n\t%s\n",filename),CRITICAL);
		g_free(filename);
	}
	return canidate;
}

		
/*!
 \brief load_profile_details() loads the profile details for the interrogation
 profile.  This is only done on a match for final prep of megatunix data-
 structures for use.
 \param canidate (struct Canidate *) pointer to the canidate to store 
 the rest of the data into
 */
void load_profile_details(struct Canidate *canidate)
{
	ConfigFile *cfgfile;
	gchar * tmpbuf = NULL;
	gchar * filename = NULL;
	gchar * section = NULL;
	gchar ** list = NULL;
	gint i = 0;

	filename = g_strdup(canidate->filename);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{	
		dbg_func(g_strdup_printf(__FILE__": load_profile_details()\n\tfile:%s opened successfully\n",filename),INTERROGATOR);
		if(!cfg_read_string(cfgfile,"parameters","Rt_Cmd_Key",
					&canidate->rt_cmd_key))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Rt_Cmd_Key\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_string(cfgfile,"parameters","VE_Cmd_Key",
					&canidate->ve_cmd_key))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"VE_Cmd_Key\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
		cfg_read_string(cfgfile,"parameters","Ign_Cmd_Key",
					&canidate->ign_cmd_key);
		cfg_read_string(cfgfile,"parameters","Raw_Mem_Cmd_Key",
					&canidate->raw_mem_cmd_key);
		if(!cfg_read_string(cfgfile,"parameters","Write_Cmd",
					&canidate->write_cmd))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Write_Cmd\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_string(cfgfile,"parameters","Burn_Cmd",
					&canidate->burn_cmd))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Burn_Cmd\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_boolean(cfgfile,"parameters","MultiPage",
					&canidate->multi_page))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"MultiPage\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_boolean(cfgfile,"parameters","RequirePageChange",
					&canidate->require_page))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"MultiPage\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
		if ((canidate->multi_page) && (canidate->require_page))
		{
			if(!cfg_read_string(cfgfile,"parameters","Page_Cmd",
						&canidate->page_cmd))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Page_Cmd\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
		}
		if(!cfg_read_int(cfgfile,"parameters","TotalPages",
					&canidate->total_pages))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"TotalPages\" value not found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_int(cfgfile,"parameters","TotalTables",
					&canidate->total_tables))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"TotalTables\" value not found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_string(cfgfile,"parameters","Capabilities",
					&tmpbuf))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Capabilities\" enumeration list not found in interrogation profile, ERROR\n"),CRITICAL);
		else
		{
			canidate->capabilities = translate_capabilities(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,"gui","LoadTabs",
					&canidate->load_tabs))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"LoadTabs\" list not found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_string(cfgfile,"gui","TabConfs",
					&canidate->tab_confs))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"TabConfs\" list not found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_string(cfgfile,"gui","RealtimeMapFile",
					&canidate->rtv_map_file))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"RealtimeMapFile\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
		if(!cfg_read_string(cfgfile,"gui","SliderMapFile",
					&canidate->sliders_map_file))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"SliderMapFile\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
		if (!cfg_read_string(cfgfile,"lookuptables","tables",
					&tmpbuf))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"tables\" lookuptable name not found in interrogation profile, ERROR\n"),CRITICAL);
		else
		{
			list = g_strsplit(tmpbuf,",",0);
			g_free(tmpbuf);
			if (!canidate->lookuptables)
				canidate->lookuptables = g_hash_table_new(g_str_hash,g_str_equal);
			i = 0;
			while (list[i] != NULL)
			{	

				if (!cfg_read_string(cfgfile,"lookuptables",list[i],&tmpbuf))
					dbg_func(g_strdup_printf(__FILE__": load_profile_details()\n\t\"%s\" key name not found in \"[lookuptables]\"\n\t section of interrogation profile, ERROR\n",list[i]),CRITICAL);
				else
				{
					g_hash_table_insert(canidate->lookuptables,list[i],g_strdup(tmpbuf));
					g_free(tmpbuf);
				}
				i++;
			}
		}

		/* Allocate space for Table Offsets structures.... */
		canidate->table_params = g_new0(struct Table_Params *,canidate->total_tables);
		for (i=0;i<canidate->total_tables;i++)
		{
			canidate->table_params[i] = initialize_table_params();

			section = g_strdup_printf("table_%i",i);
			if(!cfg_read_int(cfgfile,section,"x_page",&canidate->table_params[i]->x_page))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_page\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"y_page",&canidate->table_params[i]->y_page))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_page\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"z_page",&canidate->table_params[i]->z_page))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_page\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"x_base_offset",&canidate->table_params[i]->x_base))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_base_offset\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"y_base_offset",&canidate->table_params[i]->y_base))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_base_offset\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"z_base_offset",&canidate->table_params[i]->z_base))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_base_offset\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"x_bincount",&canidate->table_params[i]->x_bincount))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_bincount\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"y_bincount",&canidate->table_params[i]->y_bincount))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_bincount\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_string(cfgfile,section,"x_suffix",&canidate->table_params[i]->x_suffix))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_suffix\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_string(cfgfile,section,"y_suffix",&canidate->table_params[i]->y_suffix))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_suffix\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_string(cfgfile,section,"z_suffix",&canidate->table_params[i]->z_suffix))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_suffix\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_string(cfgfile,section,"x_conv_expr",&canidate->table_params[i]->x_conv_expr))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_conv_expr\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_string(cfgfile,section,"y_conv_expr",&canidate->table_params[i]->y_conv_expr))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_conv_expr\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_string(cfgfile,section,"z_conv_expr",&canidate->table_params[i]->z_conv_expr))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_conv_expr\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			if(cfg_read_boolean(cfgfile,section,"x_disp_float",&canidate->table_params[i]->x_disp_float))
			{
				if (canidate->table_params[i]->x_disp_float)	
					if(!cfg_read_int(cfgfile,section,"x_disp_precision",&canidate->table_params[i]->x_disp_precision))
						dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_disp_precision\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			}
			if(cfg_read_boolean(cfgfile,section,"y_disp_float",&canidate->table_params[i]->y_disp_float))
			{
				if (canidate->table_params[i]->y_disp_float)	
					if(!cfg_read_int(cfgfile,section,"y_disp_precision",&canidate->table_params[i]->y_disp_precision))
						dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_disp_precision\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			}
			if(cfg_read_boolean(cfgfile,section,"z_disp_float",&canidate->table_params[i]->z_disp_float))
			{
				if (canidate->table_params[i]->z_disp_float)	
					if(!cfg_read_int(cfgfile,section,"z_disp_precision",&canidate->table_params[i]->z_disp_precision))
						dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_disp_precision\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			}
			if(!cfg_read_string(cfgfile,section,"table_name",&canidate->table_params[i]->table_name))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"table_name\" variable not found in interrogation profile, ERROR\n"),CRITICAL);
			g_free(section);
		}

		canidate->page_params = g_new0(struct Page_Params *,canidate->total_pages);
		for (i=0;i<canidate->total_pages;i++)
		{
			canidate->page_params[i] = initialize_page_params();
			section = g_strdup_printf("page_%i",i);
			if(!cfg_read_int(cfgfile,section,"length",&canidate->page_params[i]->length))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"length\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
			if(!cfg_read_boolean(cfgfile,section,"is_spark",&canidate->page_params[i]->is_spark))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"is_spark\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
			if (!(canidate->page_params[i]->is_spark))
			{
				if(!cfg_read_int(cfgfile,section,"divider_offset",&canidate->page_params[i]->divider_offset))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"divider_offset\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
				if(!cfg_read_int(cfgfile,section,"reqfuel_offset",&canidate->page_params[i]->reqfuel_offset))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"reqfuel_offset\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
				if(!cfg_read_int(cfgfile,section,"cfg11_offset",&canidate->page_params[i]->cfg11_offset))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"cfg11_offset\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
				if(!cfg_read_int(cfgfile,section,"cfg12_offset",&canidate->page_params[i]->cfg12_offset))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"cfg12_offset\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
				if(!cfg_read_int(cfgfile,section,"cfg13_offset",&canidate->page_params[i]->cfg13_offset))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"cfg13_offset\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
				if(!cfg_read_int(cfgfile,section,"rpmk_offset",&canidate->page_params[i]->rpmk_offset))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"rpmk_offset\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
				if (!(canidate->capabilities & DUALTABLE))
				{
					if(!cfg_read_int(cfgfile,section,"alternate_offset",&canidate->page_params[i]->alternate_offset))
						dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"alternate_offset\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
				}
			}
			else
			{
				if(!cfg_read_int(cfgfile,section,"spconfig_offset",&canidate->page_params[i]->spconfig_offset))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"spconfig_offset\" flag not found in interrogation profile, ERROR\n"),CRITICAL);
			}
		}

		cfg_free(cfgfile);
		g_free(filename);

	}
	else
	{
		dbg_func(g_strdup_printf(__FILE__": load_profile_details()\n\tfailure opening file:\n\t%s\n",filename),CRITICAL);
		g_free(filename);
	}
}


/*!
 \brief load_bytecounts() loads the counts of bytes per command from the 
 interrogation profile
 \param cmd_array (GArray *) array of commands 
 \param hash (GHashTable *) placeto put the data as we read it
 \param cfgfile (ConfigFile *) configfileto get data from
 */
void load_bytecounts(GArray *cmd_array, GHashTable *hash, ConfigFile *cfgfile)
{
	struct Command *cmd = NULL;
	gint i = 0;
	gint bytecount = -2;

	for (i=0;i<cmd_array->len;i++)
	{
		bytecount = -1;
		cmd = g_array_index(cmd_array,struct Command *, i);

		if(!cfg_read_int(cfgfile,"bytecounts",cmd->key,&bytecount))
			bytecount = 0;
		g_hash_table_insert(hash, g_strdup(cmd->key),GINT_TO_POINTER(bytecount));
		dbg_func(g_strdup_printf(__FILE__": load_bytecounts()\n\tInserting key %s, val %i\n",cmd->key,bytecount),INTERROGATOR);

	}	

}


/*!
 \brief translate_capabilities() converts a stringlist into a mask of 
 enumerations and returns it
 \param string (gchar *) listing of capabilities in textual format
 \returns an integer mask of the capabilites
 */
gint translate_capabilities(gchar *string)
{
	gchar **vector = NULL;
	gint i = 0;
	gint value = 0;


	if (!string)
	{
		dbg_func(g_strdup_printf(__FILE__": translate_capabilities()\n\tstring fed is NULLs\n"),CRITICAL);
		return -1;
	}

	vector = g_strsplit(string,",",0);
	dbg_func(g_strdup_printf(__FILE__": translate_capabilities()\n\tstring fed is %s\n",string),INTERROGATOR);
	while (vector[i] != NULL)
	{
		dbg_func(g_strdup_printf(__FILE__": translate_capabilities()\n\tTrying to translate %s\n",vector[i]),INTERROGATOR);
		value += translate_string(vector[i]);
		dbg_func(g_strdup_printf(__FILE__": translate_capabilities()\n\tTranslated value of %s is %i\n",vector[i],value),INTERROGATOR);
		i++;
	}

	g_strfreev(vector);
	return value;	
}


/*!
 \brief check_for_match() compares the resutls of the interrogation with the
 ECU to the canidates in turn.  when a match occurs TRUE is returns
 otherwise it returns FALSE
 \param cmd_array (GArray *) array of commands
 \param potential (struct Canidate *) potential 
 \param canidate (struct Canidate *) Canidate
 \returns TRUE on match, FALSE on failure
 */
gboolean check_for_match(GArray *cmd_array, struct Canidate *potential, struct Canidate *canidate)
{
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
				cmd->key);
		pbytes = (gint)g_hash_table_lookup(
				potential->bytecounts,
				cmd->key);
		dbg_func(g_strdup_printf(__FILE__": check_for_match()\n\tTest(%s) %s, canidate bytes %i, potential %i\n",cmd->key,cmd->string,cbytes,pbytes),INTERROGATOR);
		if (cbytes != pbytes)
		{
			// Mismatch, abort test and move on to the 
			// next one...
			dbg_func(g_strdup_printf(__FILE__": check_for_match()\n\tCounts don't match, loading next profile\n"),INTERROGATOR);
			close_profile(potential);
			return FALSE;
		}
	}
	dbg_func(g_strdup_printf(__FILE__": check_for_match()\n\tAll bytecount tests passed for firmware %s\n",potential->name),INTERROGATOR);
	/* If all test pass, now check the Extended version
	 * If it matches,  jump out...
	 */
	if (potential->ver_num != canidate->ver_num)
	{
		dbg_func(g_strdup_printf(__FILE__": check_for_match()\n\tCanidate version number \"%i\" does NOT match potential \"%i\", loading next profile\n",canidate->ver_num,potential->ver_num),INTERROGATOR);
		return FALSE;
	}
	else
		dbg_func(g_strdup_printf(__FILE__": check_for_match()\n\tCanidate version number \"%i\" Matches potential \"%i\", continuing tests...\n",canidate->ver_num,potential->ver_num),INTERROGATOR);

	
	if ((potential->quest_str != NULL) && (canidate->quest_str != NULL))
	{
		dbg_func(g_strdup_printf(__FILE__": check_for_match()\n\tTesting ext version, canidate \"%s\", potential \"%s\"\n",canidate->quest_str,potential->quest_str),INTERROGATOR);
		if (strstr(canidate->quest_str,potential->quest_str) == NULL)
		{
			dbg_func(g_strdup(__FILE__": check_for_match()\n\tDID NOT find match on extended version\n"),INTERROGATOR);
			return FALSE;
		}
		else
			dbg_func(g_strdup(__FILE__": check_for_match()\n\tFound match on extended version\n"),INTERROGATOR);
	}
	else if ((potential->sig_str != NULL) && (canidate->sig_str != NULL))
	{
		dbg_func(g_strdup_printf(__FILE__": check_for_match()\n\tTesting signature, canidate \"%s\", potential \"%s\"\n",canidate->sig_str,potential->sig_str),INTERROGATOR);
		if (strstr(canidate->sig_str,potential->sig_str) == NULL)
		{
			dbg_func(g_strdup(__FILE__": check_for_match()\n\tDID NOT find match on signature\n"),INTERROGATOR);
			return FALSE;
		}
		else
			dbg_func(g_strdup(__FILE__": check_for_match()\n\tFound match on signature\n"),INTERROGATOR);
	}
	else
	{
		dbg_func(g_strdup_printf(__FILE__": check_for_match()\n\tFound match on bytecounts and version number alone...\n\n"),INTERROGATOR);
		return TRUE;
	}

	dbg_func(g_strdup_printf("\n"__FILE__": check_for_match()\n\tFound match on bytecounts,version number and signature/extended version (Best match) ...\n\n"),INTERROGATOR);
	return TRUE;
}
