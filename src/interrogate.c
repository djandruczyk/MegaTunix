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

#include <api-versions.h>
#include <apicheck.h>
#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <getfiles.h>
#include <glib.h>
#include <init.h>
#include <interrogate.h>
#include <lookuptables.h>
#include <mode_select.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <multi_expr_loader.h>
#include <notifications.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <sys/stat.h>
#include <threads.h>
#include <unistd.h>

extern gboolean connected;
extern GtkTextBuffer *textbuffer;
extern GtkWidget *interr_view;
extern gint dbg_lvl;
extern Serial_Params *serial_params;
extern GObject *global_data;
Firmware_Details *firmware = NULL;
gboolean interrogated = FALSE;


/*!
 \brief interrogate_ecu() interrogates the target ECU to determine what
 firmware it is running.  It does this by reading a list of tests, sending
 those tests in turn, reading the responses and them comparing the group of
 responses against a list of interrogation profiles until it finds a match.
 */
EXPORT gboolean interrogate_ecu()
{
	GArray *tests = NULL;
	extern GHashTable *dynamic_widgets;
	GHashTable *tests_hash = NULL;
	Detection_Test *test = NULL;
	guchar uint8 = 0;
	gchar sint8 = 0;
	guint16 uint16 = 0;
	gint16 sint16 = 0;
	gint size = 4096;
	gint res = 0;
	gint count = 0;
	gint i = 0;
	gint j = 0;
	gint tests_to_run = 0;
	gint total_read = 0;
	gint total_wanted = 0;
	gint zerocount = 0;
	gchar *string = NULL;
	guchar buf[size];
	guchar *ptr = NULL;
	gchar * message = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	/* prevent multiple runs of interrogator simultaneously */
	g_static_mutex_lock(&mutex);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"offline_button"),FALSE);
	if (dbg_lvl & INTERROGATOR)
		dbg_func(g_strdup("\n"__FILE__": interrogate_ecu() ENTERED\n\n"));

	if (!connected)
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": interrogate_ecu()\n\tNOT connected to ECU!!!!\n"));
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"offline_button"),TRUE);
		g_static_mutex_unlock(&mutex);
		return FALSE;
	}
	thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup("Interrogating ECU..."));

	/* Load tests from config files */
	tests = validate_and_load_tests(&tests_hash);

	if ((!tests) || (tests->len < 1))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": interrogate_ecu()\n\t validate_and_load_tests() didn't return a valid list of commands\n\t MegaTunix was NOT installed correctly, Aborting Interrogation\n"));
		g_static_mutex_unlock(&mutex);
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"offline_button"),TRUE);
		return FALSE;
	}
	/* how many tests.... */
	tests_to_run = tests->len;

	for (i=0;i<tests_to_run;i++)
	{
		flush_serial(serial_params->fd,TCIOFLUSH);
		count = 0;
		test = g_array_index(tests,Detection_Test *, i);

		/* flush buffer to known state.. */
		memset (buf,0,size);

		ptr = buf;

		for (j=0;j<test->test_arg_count;j++)
		{
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_CHAR)
			{
				string = g_strdup(test->test_vector[j]);
				res = write(serial_params->fd,string,1);
				if (res != 1)
					interrogate_error("String Write error",j);
				if (dbg_lvl & INTERROGATOR)
					dbg_func(g_strdup_printf("\tSent command \"%s\"\n",string));
				g_free(string);
			}
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_U08)
			{
				uint8 = (guint8)atoi(test->test_vector[j]);
				res = write(serial_params->fd,&uint8,1);
				if (res != 1)
					interrogate_error("U08 Write error",j);
				if (dbg_lvl & INTERROGATOR)
					dbg_func(g_strdup_printf("\tSent command \"%i\"\n",uint8));
			}
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_U16)
			{
				uint16 = (guint16)atoi(test->test_vector[j]);
				res = write(serial_params->fd,&uint16,2);
				if (res != 1)
					interrogate_error("U16 Write error",j);
				if (dbg_lvl & INTERROGATOR)
					dbg_func(g_strdup_printf("\tSent command \"%i\"\n",uint8));
			}
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_S08)
			{
				sint8 = (gint8)atoi(test->test_vector[j]);
				res = write(serial_params->fd,&sint8,1);
				if (res != 1)
					interrogate_error("S08 Write error",j);
				if (dbg_lvl & INTERROGATOR)
					dbg_func(g_strdup_printf("\tSent command \"%i\"\n",sint8));
			}
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_S16)
			{
				sint16 = (gint16)atoi(test->test_vector[j]);
				res = write(serial_params->fd,&sint16,2);
				if (res != 1)
					interrogate_error("S16 Write error",j);
				if (dbg_lvl & INTERROGATOR)
					dbg_func(g_strdup_printf("\tSent command \"%i\"\n",sint8));
			}
		}

		total_read = 0;
		total_wanted = size;
		zerocount = 0;
		while ((total_read < total_wanted ) && (total_wanted-total_read) > 0 )
		{
			if (dbg_lvl & INTERROGATOR)
				dbg_func(g_strdup_printf("\tInterrogation for command %s requesting %i bytes\n",test->test_name,total_wanted-total_read));
			total_read += res = read(serial_params->fd,
					ptr+total_read,
					total_wanted-total_read);

			if (dbg_lvl & INTERROGATOR)
				dbg_func(g_strdup_printf("\tInterrogation for command %s read %i bytes, running total %i\n",test->test_name,res,total_read));
			/* If we get nothing back (i.e. timeout, assume done)*/
			if (res <= 0)
				zerocount++;

			if (zerocount > 1)
				break;
		}


		if (total_read > 0)
		{
			thread_update_logbar("interr_view",NULL,g_strdup_printf("Command \"%s\" (%s), returned %i bytes\n",test->actual_test, test->test_desc,total_read),FALSE,FALSE);
			ptr = buf;
			if (dbg_lvl & (SERIAL_RD|INTERROGATOR))
			{
				dbg_func(g_strdup_printf(__FILE__": interrogate_ecu()\n\tRead the following from the %s command\n",test->test_name));
				message = g_strndup(((gchar *)buf),total_read);
				dbg_func(g_strdup_printf(__FILE__": interrogate.c()\n\tDumping Output string: \"%s\"\n",message));
				g_free(message);
				dbg_func(g_strdup("Data is in HEX!!\n"));
			}
			for (j=0;j<total_read;j++)
			{
				if (dbg_lvl & (SERIAL_RD|INTERROGATOR))
					dbg_func(g_strdup_printf("%.2x ", ptr[j]));
				if (!((j+1)%8)) /* every 8 bytes give a CR */
				{
					if (dbg_lvl & (SERIAL_RD|INTERROGATOR))
						dbg_func(g_strdup("\n"));
				}
			}
			if (dbg_lvl & (SERIAL_RD|INTERROGATOR))
				dbg_func(g_strdup("\n\n"));
		}

		if (dbg_lvl & INTERROGATOR)
			dbg_func(g_strdup_printf("\tReceived %i bytes\n",total_read));
		ptr = buf;

		/* copy data from tmp buffer to struct pointer */
		test->num_bytes = total_read;
		if (total_read == 0)
			test->result_str = g_strdup("");
		else
			test->result_str = g_strndup((gchar *)ptr, total_read);
	}

	interrogated = determine_ecu(tests,tests_hash);	
	if (interrogated)
	{
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"interrogate_button"),FALSE);
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"offline_button"),FALSE);
	}

	free_tests_array(tests);
	g_hash_table_destroy(tests_hash);

	if (!interrogated)
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"offline_button"),TRUE);

	g_static_mutex_unlock(&mutex);
	if (dbg_lvl & INTERROGATOR)
		dbg_func(g_strdup("\n"__FILE__": interrogate_ecu() LEAVING\n\n"));
	thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup("Interrogation Complete..."));
	return interrogated;
}


/*!
 \brief determine_ecu() trys to match determine the target firmware by 
 loading the interrogation profiles in turn and comparing the data from our
 test ECU adn a profile until a match is found, 
 \param canidate (Canidate *) pointer to the Canidate structure
 \param cmd_array (GArray *) pointer to the array of commands sent
 \param cmd_details (GHashTable) details on the interrogation process with
 the target ECU
 \returns TRUE on successfull interrogation, FALSE on no match
 */

gboolean determine_ecu(GArray *tests,GHashTable *tests_hash)
{

	gint i = 0;
	Detection_Test *test = NULL;
	gint num_tests = tests->len;
	gboolean match = FALSE;
	gchar * filename = NULL;
	gchar ** filenames = NULL;
	GArray *classes = NULL;

	filenames = get_files(g_strconcat(INTERROGATOR_DATA_DIR,PSEP,"Profiles",PSEP,NULL),g_strdup("prof"),&classes);	
	if (!filenames)
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": determine_ecu()\n\t NO Interrogation profiles found,  was MegaTunix installed properly?\n\n"));
		return FALSE;
	}

	i = 0;
	while (filenames[i])
	{
		if (check_for_match(tests_hash,filenames[i]))
		{
			match = TRUE;
			filename = g_strdup(filenames[i]);
			break;
		}
		i++;
	}
	g_strfreev(filenames);
	g_array_free(classes,TRUE);
	/* Update the screen with the data... */
	for (i=0;i<num_tests;i++)
	{
		test = g_array_index(tests,Detection_Test *,i);
		if (dbg_lvl & INTERROGATOR)
			dbg_func(g_strdup_printf("\tCommand \"%s\" (%s), returned %i bytes\n",
						test->actual_test,
						test->test_desc,
						test->num_bytes));

	}
	if (match == FALSE) /* (we DID NOT find one) */
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__":\n\tdetermine_ecu()\n\tFirmware NOT DETECTED, Enable Interrogation debugging, retry interrogation,\nclose megatunix, and send ~/MTXlog.txt to the author for analysis with a note\ndescribing which firmware you are attempting to talk to.\n"));
			thread_update_logbar("interr_view","warning",g_strdup("Firmware NOT DETECTED, Enable Interrogation debugging, retry interrogation,\nclose megatunix, and send ~/MTXlog.txt to the author for analysis with a note\ndescribing which firmware you are attempting to talk to.\n"),FALSE,FALSE);
		return FALSE;
	}
	else
	{

		if (!firmware)
			firmware = g_new0(Firmware_Details,1);

		if (!load_firmware_details(firmware,filename))
			return FALSE;
		update_interrogation_gui(firmware,tests_hash);

		return TRUE;
	
	}
}


/* !brief loads up all firmware details from the passed filename (interrogation
 * profile), and configures megatunix for use.
 */
gboolean load_firmware_details(Firmware_Details *firmware, gchar * filename)
{
	ConfigFile *cfgfile;
	gchar * tmpbuf = NULL;
	gchar * section = NULL;
	gchar ** list = NULL;
	gint i = 0;
	MultiSource * multi = NULL;
	gchar **sources = NULL;
	gchar **suffixes = NULL;
	gchar **conv_exprs = NULL;
	gchar **precisions = NULL;
	gchar **expr_keys = NULL;
	gint major = 0;
	gint minor = 0;
	gint len1 = 0;
	gint len2 = 0;
	gint len3 = 0;
	gint len4 = 0;
	gint len5 = 0;
	gint j = 0;

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		if (dbg_lvl & (CRITICAL|INTERROGATOR))
			dbg_func(g_strdup_printf(__FILE__": load_firmware_details()\n\tFile \"%s\" NOT OPENED successfully\n",filename));
	get_file_api(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		thread_update_logbar("interr_view","warning",g_strdup_printf("Interrogation profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n",major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE);
		return FALSE;
	}

	firmware->profile_filename = g_strdup(filename);

	cfg_read_string(cfgfile,"interrogation_profile","name",&firmware->name);
	cfg_read_string(cfgfile,"parameters","TextVerVia",&firmware->TextVerVia);
	cfg_read_string(cfgfile,"parameters","NumVerVia",&firmware->NumVerVia);
	cfg_read_string(cfgfile,"parameters","SignatureVia",&firmware->SignatureVia);

	if (dbg_lvl & INTERROGATOR)
		dbg_func(g_strdup_printf(__FILE__": load_profile_details()\n\tfile:%s opened successfully\n",filename));
	if(!cfg_read_string(cfgfile,"parameters","Capabilities",
				&tmpbuf))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Capabilities\" enumeration list not found in interrogation profile, ERROR\n"));
	}
	else
	{
		firmware->capabilities = translate_capabilities(tmpbuf);
		g_free(tmpbuf);
	}
	if(!cfg_read_string(cfgfile,"parameters","RT_Command",
				&firmware->rt_command))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"RT_Command\" variable not found in interrogation profile, ERROR\n"));
	}
	if (firmware->capabilities & MS2)
	{
		if(!cfg_read_int(cfgfile,"parameters","MS2_RT_Page",
					&firmware->ms2_rt_page))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"MS2_RT_Page\" variable not found in interrogation profile, ERROR\n"));
		}
		if(!cfg_read_int(cfgfile,"parameters","InterCharDelay",
					&firmware->interchardelay))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"InterCharDelay\" variable not found in interrogation profile, ERROR\n"));
		}
	}
	if(!cfg_read_int(cfgfile,"parameters","RT_total_bytes",
				&firmware->rtvars_size))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"RT_total_bytes\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_string(cfgfile,"parameters","Get_All_Command",
				&firmware->get_all_command))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Get_All_Command\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_string(cfgfile,"parameters","VE_Command",
				&firmware->ve_command))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"VE_Command\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_string(cfgfile,"parameters","Write_Command",
				&firmware->write_command))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Write_Command\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_string(cfgfile,"parameters","Burn_Command",
				&firmware->burn_command))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Burn_Command\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_string(cfgfile,"parameters","Burn_All_Command",
					&firmware->burn_all_command))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Burn_All_Command\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_boolean(cfgfile,"parameters","MultiPage",
				&firmware->multi_page))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"MultiPage\" flag not found in interrogation profile, ERROR\n"));
	}
	if ((firmware->multi_page) && (!(firmware->capabilities & MS2)))
	{
		if(!cfg_read_string(cfgfile,"parameters","Page_Cmd",
					&firmware->page_cmd))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Page_Cmd\" flag not found in interrogation profile, ERROR\n"));
		}
	}
	if(!cfg_read_boolean(cfgfile,"parameters","ChunkWriteSupport",
				&firmware->chunk_support))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"ChunkWriteSupport\" flag not found in interrogation profile, ERROR\n"));
	}
	if (firmware->chunk_support)
	{
		if(!cfg_read_string(cfgfile,"parameters","Chunk_Write_Command",
					&firmware->chunk_write_command))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"Chunk_Write_Command\" flag not found in interrogation profile, ERROR\n"));
		}
	}
	if(!cfg_read_int(cfgfile,"parameters","TotalPages",
				&firmware->total_pages))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"TotalPages\" value not found in interrogation profile, ERROR\n"));
	}
	cfg_read_int(cfgfile,"parameters","ReadOnlyAbove",&firmware->ro_above);
	if(!cfg_read_int(cfgfile,"parameters","TotalTables",
				&firmware->total_tables))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"TotalTables\" value not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_string(cfgfile,"gui","LoadTabs",
				&tmpbuf))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"LoadTabs\" list not found in interrogation profile, ERROR\n"));
	}
	else
	{
		firmware->tab_list = g_strsplit(tmpbuf,",",0);
		g_free(tmpbuf);
	}
	if(!cfg_read_string(cfgfile,"gui","TabConfs",
				&tmpbuf))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"TabConfs\" list not found in interrogation profile, ERROR\n"));
	}
	else
	{
		firmware->tab_confs = g_strsplit(tmpbuf,",",0);
		g_free(tmpbuf);
	}
	if(!cfg_read_string(cfgfile,"gui","RealtimeMapFile",
				&firmware->rtv_map_file))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"RealtimeMapFile\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_string(cfgfile,"gui","SliderMapFile",
				&firmware->sliders_map_file))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"SliderMapFile\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_string(cfgfile,"gui","RuntimeTextMapFile",
				&firmware->rtt_map_file))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"RuntimeTextMapFile\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_string(cfgfile,"gui","StatusMapFile",
				&firmware->status_map_file))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"StatusMapFile\" variable not found in interrogation profile, ERROR\n"));
	}
	if (!cfg_read_string(cfgfile,"lookuptables","tables",
				&tmpbuf))
	{
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"tables\" lookuptable name not found in interrogation profile, ERROR\n"));
	}
	else
	{
		list = g_strsplit(tmpbuf,",",0);
		g_free(tmpbuf);
		i = 0;
		while (list[i] != NULL)
		{	
			if (!cfg_read_string(cfgfile,"lookuptables",list[i],&tmpbuf))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup_printf(__FILE__": load_profile_details()\n\t\"%s\" key name not found in \"[lookuptables]\"\n\t section of interrogation profile, ERROR\n",list[i]));
			}
			else
			{
				if (dbg_lvl & (INTERROGATOR))
					dbg_func(g_strdup_printf(__FILE__": load_profile_details()\n\t \"[lookuptables]\"\n\t section loading table %s, file %s\n",list[i],tmpbuf));
				get_table(list[i],tmpbuf,NULL);
				g_free(tmpbuf);
			}
			i++;
		}
		g_strfreev(list);
	}

	/* Allocate space for Table Offsets structures.... */
	firmware->table_params = g_new0(Table_Params *,firmware->total_tables);
	for (i=0;i<firmware->total_tables;i++)
	{
		firmware->table_params[i] = initialize_table_params();

		section = g_strdup_printf("table_%i",i);
		if(cfg_read_boolean(cfgfile,section,"is_fuel",&firmware->table_params[i]->is_fuel))
		{
			if(!cfg_read_int(cfgfile,section,"divider_offset",&firmware->table_params[i]->divider_offset))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"divider_offset\" flag not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_int(cfgfile,section,"reqfuel_offset",&firmware->table_params[i]->reqfuel_offset))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"reqfuel_offset\" flag not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_int(cfgfile,section,"cfg11_offset",&firmware->table_params[i]->cfg11_offset))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"cfg11_offset\" flag not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_int(cfgfile,section,"cfg12_offset",&firmware->table_params[i]->cfg12_offset))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"cfg12_offset\" flag not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_int(cfgfile,section,"cfg13_offset",&firmware->table_params[i]->cfg13_offset))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"cfg13_offset\" flag not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_int(cfgfile,section,"rpmk_offset",&firmware->table_params[i]->rpmk_offset))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"rpmk_offset\" flag not found in interrogation profile, ERROR\n"));
			}
			if (!(firmware->capabilities & DUALTABLE))
			{
				if(!cfg_read_int(cfgfile,section,"alternate_offset",&firmware->table_params[i]->alternate_offset))
				{
					if (dbg_lvl & (INTERROGATOR|CRITICAL))
						dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"alternate_offset\" flag not found in interrogation profile, ERROR\n"));
				}
			}
			if (firmware->capabilities & MSNS_E)
			{
				if(!cfg_read_int(cfgfile,section,"dtmode_offset",&firmware->table_params[i]->dtmode_offset))
				{
					if (dbg_lvl & (INTERROGATOR|CRITICAL))
						dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"dtmode_offset\" flag not found in interrogation profile, ERROR\n"));
				}
				if(!cfg_read_int(cfgfile,section,"dtmode_page",&firmware->table_params[i]->dtmode_page))
				{
					if (dbg_lvl & (INTERROGATOR|CRITICAL))
						dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"dtmode_page\" flag not found in interrogation profile, ERROR\n"));
				}
				if(!cfg_read_int(cfgfile,"parameters","TrigmonPage",&firmware->trigmon_page))
				{
					if (dbg_lvl & (INTERROGATOR|CRITICAL))
						dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"TrigmonPage\" value not found in interrogation profile, ERROR\n"));
				}
				if(!cfg_read_int(cfgfile,"parameters","ToothmonPage",&firmware->toothmon_page))
				{
					if (dbg_lvl & (INTERROGATOR|CRITICAL))
						dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"ToothmonPage\" value not found in interrogation profile, ERROR\n"));
				}
			}
		}
		if(!cfg_read_int(cfgfile,section,"x_page",&firmware->table_params[i]->x_page))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_page\" flag not found in interrogation profile, ERROR\n"));
		}
		if(!cfg_read_int(cfgfile,section,"y_page",&firmware->table_params[i]->y_page))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_page\" flag not found in interrogation profile, ERROR\n"));
		}
		if(!cfg_read_int(cfgfile,section,"z_page",&firmware->table_params[i]->z_page))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_page\" flag not found in interrogation profile, ERROR\n"));
		}
		if(!cfg_read_int(cfgfile,section,"x_base_offset",&firmware->table_params[i]->x_base))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_base_offset\" variable not found in interrogation profile, ERROR\n"));
		}
		if(!cfg_read_int(cfgfile,section,"y_base_offset",&firmware->table_params[i]->y_base))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_base_offset\" variable not found in interrogation profile, ERROR\n"));
		}
		if(!cfg_read_int(cfgfile,section,"z_base_offset",&firmware->table_params[i]->z_base))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_base_offset\" variable not found in interrogation profile, ERROR\n"));
		}
		if(!cfg_read_int(cfgfile,section,"x_bincount",&firmware->table_params[i]->x_bincount))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_bincount\" variable not found in interrogation profile, ERROR\n"));
		}
		if(!cfg_read_string(cfgfile,section,"x_size",&tmpbuf))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_size\" enumeration not found in interrogation profile, ERROR\n"));
		}
		else
		{
			firmware->table_params[i]->x_size = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"y_size",&tmpbuf))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_size\" enumeration not found in interrogation profile, ERROR\n"));
		}
		else
		{
			firmware->table_params[i]->y_size = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"z_size",&tmpbuf))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_size\" enumeration not found in interrogation profile, ERROR\n"));
		}
		else
		{
			firmware->table_params[i]->z_size = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_int(cfgfile,section,"y_bincount",&firmware->table_params[i]->y_bincount))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_bincount\" variable not found in interrogation profile, ERROR\n"));
		}
		cfg_read_boolean(cfgfile,section,"x_multi_source",&firmware->table_params[i]->x_multi_source);
		if (firmware->table_params[i]->x_multi_source)
		{
			/* READ multi-source stuff, but do NOT create
			 * evaluators,  we do that in the final copy
			 * over to the firmware struct
			 */
			if(!cfg_read_string(cfgfile,section,"x_multi_expr_keys",&firmware->table_params[i]->x_multi_expr_keys))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_multi_expr_keys\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"x_source_key",&firmware->table_params[i]->x_source_key))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_source_key\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"x_sources",&firmware->table_params[i]->x_sources))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_sources\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"x_suffixes",&firmware->table_params[i]->x_suffixes))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_suffixes\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"x_conv_exprs",&firmware->table_params[i]->x_conv_exprs))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_conv_exprs\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"x_precisions",&firmware->table_params[i]->x_precisions))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_precisions\" variable not found in interrogation profile, ERROR\n"));
			}
		}
		else
		{
			if(!cfg_read_string(cfgfile,section,"x_source",&firmware->table_params[i]->x_source))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_source\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"x_suffix",&firmware->table_params[i]->x_suffix))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_suffix\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"x_conv_expr",&firmware->table_params[i]->x_conv_expr))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"x_conv_expr\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_int(cfgfile,section,"x_precision",&firmware->table_params[i]->x_precision))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup_printf(__FILE__": load_profile_details()\n\t\"x_precision\" variable not found in interrogation profile for table %i, ERROR\n",i));
			}
		}
		cfg_read_boolean(cfgfile,section,"y_multi_source",&firmware->table_params[i]->y_multi_source);
		if (firmware->table_params[i]->y_multi_source)
		{
			/* READ multi-source stuff, but do NOT create
			 * evaluators,  we do that in the final copy
			 * over to the firmware struct
			 */
			if(!cfg_read_string(cfgfile,section,"y_multi_expr_keys",&firmware->table_params[i]->y_multi_expr_keys))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_multi_expr_keys\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"y_source_key",&firmware->table_params[i]->y_source_key))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_source_key\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"y_sources",&firmware->table_params[i]->y_sources))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_sources\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"y_suffixes",&firmware->table_params[i]->y_suffixes))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_suffixes\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"y_conv_exprs",&firmware->table_params[i]->y_conv_exprs))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_conv_exprs\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"y_precisions",&firmware->table_params[i]->y_precisions))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_precisions\" variable not found in interrogation profile, ERROR\n"));
			}

		}
		else
		{
			if(!cfg_read_string(cfgfile,section,"y_source",&firmware->table_params[i]->y_source))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_source\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"y_suffix",&firmware->table_params[i]->y_suffix))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_suffix\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"y_conv_expr",&firmware->table_params[i]->y_conv_expr))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"y_conv_expr\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_int(cfgfile,section,"y_precision",&firmware->table_params[i]->y_precision))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup_printf(__FILE__": load_profile_details()\n\t\"y_precision\" variable not found in interrogation profile for table %i, ERROR\n",i));
			}
		}
		cfg_read_boolean(cfgfile,section,"z_multi_source",&firmware->table_params[i]->z_multi_source);
		if(firmware->table_params[i]->z_multi_source)
		{
			/* READ multi-source stuff, but do NOT create
			 * evaluators,  we do that in the final copy
			 * over to the firmware struct
			 */
			if(!cfg_read_string(cfgfile,section,"z_multi_expr_keys",&firmware->table_params[i]->z_multi_expr_keys))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_multi_expr_keys\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"z_source_key",&firmware->table_params[i]->z_source_key))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_source_key\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"z_sources",&firmware->table_params[i]->z_sources))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_sources\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"z_suffixes",&firmware->table_params[i]->z_suffixes))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_suffixes\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"z_conv_exprs",&firmware->table_params[i]->z_conv_exprs))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_conv_exprs\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"z_precisions",&firmware->table_params[i]->z_precisions))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_precisions\" variable not found in interrogation profile, ERROR\n"));
			}

		}
		else
		{
			if(!cfg_read_string(cfgfile,section,"z_source",&firmware->table_params[i]->z_source))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_source\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"z_suffix",&firmware->table_params[i]->z_suffix))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_suffix\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_string(cfgfile,section,"z_conv_expr",&firmware->table_params[i]->z_conv_expr))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"z_conv_expr\" variable not found in interrogation profile, ERROR\n"));
			}
			if(!cfg_read_int(cfgfile,section,"z_precision",&firmware->table_params[i]->z_precision))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup_printf(__FILE__": load_profile_details()\n\t\"z_precision\" variable not found in interrogation profile for table %i, ERROR\n",i));
			}
		}
		if(!cfg_read_string(cfgfile,section,"table_name",&firmware->table_params[i]->table_name))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"table_name\" variable not found in interrogation profile, ERROR\n"));
		}
		g_free(section);
	}

	firmware->page_params = g_new0(Page_Params *,firmware->total_pages);
	for (i=0;i<firmware->total_pages;i++)
	{
		firmware->page_params[i] = initialize_page_params();
		section = g_strdup_printf("page_%i",i);

		if (firmware->multi_page)
			if(!cfg_read_int(cfgfile,section,"truepgnum",&firmware->page_params[i]->truepgnum))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"truepgnum\" flag not found in interrogation profile, ERROR\n"));
			}
		if(!cfg_read_boolean(cfgfile,section,"dl_by_default",&firmware->page_params[i]->dl_by_default))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"dl_by_default\" flag not found in interrogation profile, assuming TRUE\n"));
			firmware->page_params[i]->dl_by_default = TRUE;
		}
		if(!cfg_read_int(cfgfile,section,"length",&firmware->page_params[i]->length))
		{
			if (dbg_lvl & (INTERROGATOR|CRITICAL))
				dbg_func(g_strdup(__FILE__": load_profile_details()\n\t\"length\" flag not found in interrogation profile, ERROR\n"));
		}
		g_free(section);
	}

	cfg_free(cfgfile);
	g_free(cfgfile);
	g_free(filename);


	/* Allocate RAM for the Req_Fuel_Params structures.*/
	firmware->rf_params = g_new0(Req_Fuel_Params *,firmware->total_tables);

	/* Allocate RAM for the Table_Params structures and copy data in..*/
	for (i=0;i<firmware->total_tables;i++)
	{
		firmware->rf_params[i] = g_new0(Req_Fuel_Params ,1);
		/* Check for multi source table handling */
		if (firmware->table_params[i]->x_multi_source)
		{
			firmware->table_params[i]->x_multi_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,free_multi_source);
			expr_keys = g_strsplit(firmware->table_params[i]->x_multi_expr_keys,",",-1);
			sources = g_strsplit(firmware->table_params[i]->x_sources,",",-1);
			suffixes = g_strsplit(firmware->table_params[i]->x_suffixes,",",-1);
			conv_exprs = g_strsplit(firmware->table_params[i]->x_conv_exprs,",",-1);
			precisions = g_strsplit(firmware->table_params[i]->x_precisions,",",-1);
			len1 = g_strv_length(expr_keys);
			len2 = g_strv_length(sources);
			len3 = g_strv_length(suffixes);
			len4 = g_strv_length(conv_exprs);
			len5 = g_strv_length(precisions);
			if ((len1 != len2) || (len1 != len3) || (len1 != len4) || (len1 != len5))
				printf("X multi_sources length mismatch!\n");
			for (j=0;j<len1;j++)
			{
				multi = g_new0(MultiSource,1);
				multi->source = g_strdup(sources[j]);
				multi->conv_expr = g_strdup(conv_exprs[j]);
				multi->evaluator = evaluator_create(multi->conv_expr);
				multi->suffix = g_strdup(suffixes[j]);
				multi->precision = (gint)strtol(precisions[j],NULL,10);
				g_hash_table_insert(firmware->table_params[i]->x_multi_hash,g_strdup(expr_keys[j]),(gpointer)multi);
			}
			g_strfreev(expr_keys);
			g_strfreev(sources);
			g_strfreev(suffixes);
			g_strfreev(conv_exprs);
			g_strfreev(precisions);
		}
		else
			firmware->table_params[i]->x_eval = evaluator_create(firmware->table_params[i]->x_conv_expr);
		/* Check for multi source table handling */
		if (firmware->table_params[i]->y_multi_source)
		{
			firmware->table_params[i]->y_multi_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,free_multi_source);
			expr_keys = g_strsplit(firmware->table_params[i]->y_multi_expr_keys,",",-1);
			sources = g_strsplit(firmware->table_params[i]->y_sources,",",-1);
			suffixes = g_strsplit(firmware->table_params[i]->y_suffixes,",",-1);
			conv_exprs = g_strsplit(firmware->table_params[i]->y_conv_exprs,",",-1);
			precisions = g_strsplit(firmware->table_params[i]->y_precisions,",",-1);
			len1 = g_strv_length(expr_keys);
			len2 = g_strv_length(sources);
			len3 = g_strv_length(suffixes);
			len4 = g_strv_length(conv_exprs);
			len5 = g_strv_length(precisions);
			if ((len1 != len2) || (len1 != len3) || (len1 != len4) || (len1 != len5))
				printf("Y multi_sources length mismatch!\n");
			for (j=0;j<len1;j++)
			{
				multi = g_new0(MultiSource,1);
				multi->source = g_strdup(sources[j]);
				multi->conv_expr = g_strdup(conv_exprs[j]);
				multi->evaluator = evaluator_create(multi->conv_expr);
				multi->suffix = g_strdup(suffixes[j]);
				multi->precision = (gint)strtol(precisions[j],NULL,10);
				g_hash_table_insert(firmware->table_params[i]->y_multi_hash,g_strdup(expr_keys[j]),(gpointer)multi);
			}
			g_strfreev(expr_keys);
			g_strfreev(sources);
			g_strfreev(suffixes);
			g_strfreev(conv_exprs);
			g_strfreev(precisions);
		}
		else
			firmware->table_params[i]->y_eval = evaluator_create(firmware->table_params[i]->y_conv_expr);

		/* Check for multi source table handling */
		if (firmware->table_params[i]->z_multi_source)
		{
			firmware->table_params[i]->z_multi_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,free_multi_source);
			expr_keys = g_strsplit(firmware->table_params[i]->z_multi_expr_keys,",",-1);
			sources = g_strsplit(firmware->table_params[i]->z_sources,",",-1);
			suffixes = g_strsplit(firmware->table_params[i]->z_suffixes,",",-1);
			conv_exprs = g_strsplit(firmware->table_params[i]->z_conv_exprs,",",-1);
			precisions = g_strsplit(firmware->table_params[i]->z_precisions,",",-1);
			len1 = g_strv_length(expr_keys);
			len2 = g_strv_length(sources);
			len3 = g_strv_length(suffixes);
			len4 = g_strv_length(conv_exprs);
			len5 = g_strv_length(precisions);
			if ((len1 != len2) || (len1 != len3) || (len1 != len4) || (len1 != len5))
				printf("Z multi_sources length mismatch!\n");
			for (j=0;j<len1;j++)
			{
				multi = g_new0(MultiSource,1);
				multi->source = g_strdup(sources[j]);
				multi->conv_expr = g_strdup(conv_exprs[j]);
				multi->evaluator = evaluator_create(multi->conv_expr);
				multi->suffix = g_strdup(suffixes[j]);
				multi->precision = (gint)strtol(precisions[j],NULL,10);
				g_hash_table_insert(firmware->table_params[i]->z_multi_hash,g_strdup(expr_keys[j]),(gpointer)multi);
			}
			g_strfreev(expr_keys);
			g_strfreev(sources);
			g_strfreev(suffixes);
			g_strfreev(conv_exprs);
			g_strfreev(precisions);
		}
		else
			firmware->table_params[i]->z_eval = evaluator_create(firmware->table_params[i]->z_conv_expr);

	}

	mem_alloc();

	/* Display firmware version in the window... */

	if (dbg_lvl & INTERROGATOR)
		dbg_func(g_strdup_printf(__FILE__": determine_ecu()\n\tDetected Firmware: %s\n",firmware->name));
	thread_update_logbar("interr_view","warning",g_strdup_printf("Detected Firmware: %s\n",firmware->name),FALSE,FALSE);
	thread_update_logbar("interr_view","info",g_strdup_printf("Loading Settings from: \"%s\"\n",firmware->profile_filename),FALSE,FALSE);

	return TRUE;

}


/*!
 \brief validate_and_load_tests() loads the list of tests from the system
 checks them for validity, populates and array and returns it
 command tested against the ECU arestored
 \returns a dynamic GArray for commands
 */
GArray * validate_and_load_tests(GHashTable **tests_hash)
{
	ConfigFile *cfgfile;
	GArray * tests = NULL;
	Detection_Test *test = NULL;
	gchar * filename = NULL;
	gchar *section = NULL;
	gchar * tmpbuf = NULL;
	gchar ** vector = NULL;
	gint total_tests = 0;
	gint result = 0;
	gint major = 0;
	gint minor = 0;
	gint i = 0;
	gint j = 0;

	filename = get_file(g_build_filename(INTERROGATOR_DATA_DIR,"tests",NULL),NULL);
	if (!filename)
		return NULL;

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		return NULL;
	get_file_api(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		thread_update_logbar("interr_view","warning",g_strdup_printf("Interrogation profile tests API mismatch (%i.%i != %i.%i):\n\tFile %s.\n",major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE);
		return NULL;
	}

	*tests_hash = g_hash_table_new(g_str_hash,g_str_equal);

		if (dbg_lvl & INTERROGATOR)
			dbg_func(g_strdup_printf(__FILE__": validate_and_load_tests()\n\tfile %s, opened successfully\n",filename));
		tests = g_array_new(FALSE,TRUE,sizeof(Detection_Test *));
		cfg_read_int(cfgfile,"interrogation_tests","total_tests",&total_tests);
		for (i=0;i<total_tests;i++)
		{
			test = g_new0(Detection_Test, 1);
			section = g_strdup_printf("test_%.2i",i);
			if (!cfg_read_string(cfgfile,section,"test_name",&test->test_name))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup_printf(__FILE__": validate_and_load_tests(),\n\ttest_name for %s is NULL\n",section));
				g_free(section);
				break;
			}
			if (!cfg_read_string(cfgfile,section,"actual_test",&test->actual_test))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup_printf(__FILE__": validate_and_load_tests(),\n\tactual_test for %s is NULL\n",section));
				g_free(section);
				break;
			}
			if (!cfg_read_string(cfgfile,section,"test_arg_types",&tmpbuf))
			{
				if (dbg_lvl & (INTERROGATOR|CRITICAL))
					dbg_func(g_strdup_printf(__FILE__": validate_and_load_tests(),\n\ttest_arg_types for %s is NULL\n",section));
				g_free(section);
				break;
			}

			cfg_read_string(cfgfile,section,"test_desc",
					&test->test_desc);

			test->test_vector = g_strsplit(test->actual_test,",",-1);
			test->test_arg_count = g_strv_length(test->test_vector);
			test->test_arg_types = g_array_new(FALSE,TRUE,sizeof(DataSize));
			vector = g_strsplit(tmpbuf,",",-1);
			g_free(tmpbuf);
			for (j=0;j<test->test_arg_count;j++)
			{
				result = translate_string(vector[j]);
				g_array_insert_val(test->test_arg_types,j,result);
			}
			g_strfreev(vector);

			g_free(section);
			g_array_append_val(tests,test);
			g_hash_table_insert(*tests_hash,test->test_name,test);
		}
		cfg_free(cfgfile);
		g_free(cfgfile);

	g_free(filename);
	return tests;
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
		if (dbg_lvl & (INTERROGATOR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": translate_capabilities()\n\tstring fed is NULLs\n"));
		return -1;
	}

	vector = g_strsplit(string,",",0);
	if (dbg_lvl & INTERROGATOR)
		dbg_func(g_strdup_printf(__FILE__": translate_capabilities()\n\tstring fed is %s\n",string));
	while (vector[i] != NULL)
	{
		if (dbg_lvl & INTERROGATOR)
			dbg_func(g_strdup_printf(__FILE__": translate_capabilities()\n\tTrying to translate %s\n",vector[i]));
		value += translate_string(vector[i]);
		if (dbg_lvl & INTERROGATOR)
			dbg_func(g_strdup_printf(__FILE__": translate_capabilities()\n\tTranslated value of %s is %i\n",vector[i],value));
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
 \param potential (Canidate *) potential 
 \param canidate (Canidate *) Canidate
 \returns TRUE on match, FALSE on failure
 */
gboolean check_for_match(GHashTable *tests_hash, gchar *filename)
{

	ConfigFile *cfgfile = NULL;
	Detection_Test *test = NULL;
	gint i = 0;
	gboolean pass = FALSE;
	gchar * tmpbuf = NULL;
	gchar ** vector = NULL;
	gchar ** match_on = NULL;
	gint major = 0;
	gint minor = 0;
	MatchClass class = 0;

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		return FALSE;

	get_file_api(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		thread_update_logbar("interr_view","warning",g_strdup_printf("Interrogation profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n",major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE);
		cfg_free(cfgfile);
		g_free(cfgfile);
		return FALSE;
	}

	if (cfg_read_string(cfgfile,"interrogation","match_on",&tmpbuf) == FALSE)
		printf("ERROR:!! match_on key missing from interrogation profile\n");
	match_on = g_strsplit(tmpbuf,",",-1);
	g_free(tmpbuf);

	for (i=0;i<g_strv_length(match_on);i++)
	{
		pass = FALSE;
		/*printf("checking for match on %s\n",match_on[i]);*/
		test = g_hash_table_lookup(tests_hash,match_on[i]);
		if (!test)
		{
			printf("ERROR test data not found for test %s\n",match_on[i]);
			continue;
		}

		/* If the test_name is NOT IN the interrogation profile,  we 
		 * abort as it's NOT match
		 */
		if (!cfg_read_string(cfgfile,"interrogation",test->test_name,&tmpbuf))
		{
			if (dbg_lvl & INTERROGATOR)
				dbg_func(g_strdup_printf("\n"__FILE__": check_for_match()\n\tMISMATCH,\"%s\" is NOT a match...\n\n",filename));
			cfg_free(cfgfile);
			g_free(cfgfile);
			g_strfreev(match_on);
			return FALSE;
		}
		vector = g_strsplit(tmpbuf,",",-1);
		/* Possible choices are "Count", "submatch" and "fullmatch", so
		 * stringparse to get them into a consistent form
		 */
		if (g_strv_length(vector) != 2)
			printf("ERROR interrogation check_for match vector does NOT have two args it has %i\n",g_strv_length(vector));
		class = translate_string(vector[0]);
		/*printf("potential data is %s\n",vector[1]);*/
		switch (class)
		{
			case COUNT:
				if (test->num_bytes == atoi(vector[1]))
					pass=TRUE;
				break;
			case NUMMATCH:
				if ((gint)(test->result_str[0]) == atoi(vector[1]))
					pass=TRUE;
				break;
			case SUBMATCH:
				if (strstr(test->result_str,vector[1]) != NULL)
					pass=TRUE;
				break;
			case FULLMATCH:
				if (g_ascii_strcasecmp(test->result_str,vector[1]) == 0)
					pass=TRUE;
				break;
			default:
				pass=FALSE;
		}
		g_strfreev(vector);
		if (pass == TRUE)
			continue;
		else
		{
			if (dbg_lvl & INTERROGATOR)
				dbg_func(g_strdup_printf("\n"__FILE__": check_for_match()\n\tMISMATCH,\"%s\" is NOT a match...\n\n",filename));
			cfg_free(cfgfile);
			g_free(cfgfile);
			g_strfreev(match_on);
			return FALSE;
		}

	}
	g_strfreev(match_on);
	if (dbg_lvl & INTERROGATOR)
		dbg_func(g_strdup_printf("\n"__FILE__": check_for_match()\n\t\"%s\" is a match for all conditions ...\n\n",filename));
	cfg_free(cfgfile);
	g_free(cfgfile);
	return TRUE;
}


/*! brief destroys Array holding Detection_Test structures
 */
void free_tests_array(GArray *tests)
{
	gint i = 0;
	Detection_Test *test = NULL;

	for (i=0;i<tests->len;i++)
	{
		test = g_array_index(tests,Detection_Test *,i);
		if (test->test_name)
			g_free(test->test_name);
		if (test->test_desc)
			g_free(test->test_desc);
		if (test->actual_test)
			g_free(test->actual_test);
		if (test->result_str)
			g_free(test->result_str);
		if (test->test_vector)
			g_strfreev(test->test_vector);
		if (test->test_arg_types)
			g_array_free(test->test_arg_types,TRUE);
		g_free(test);
		test = NULL;
	}

	g_array_free(tests,TRUE);
}


/*! brief interrogate_error,  dumps an error out to the error handling
 * based on passed string and numeric pararms
 */
void interrogate_error(gchar *text, gint num)
{
	extern gint dbg_lvl;
	if (dbg_lvl & (CRITICAL|INTERROGATOR))
		dbg_func(g_strdup_printf("\tInterrogation error: \"%s\" data: \"%i\"n",text,num));
}


/* !brief updates the interrogation gui with the text revision, signature
 * and ecu numerical revision
 */
void update_interrogation_gui(Firmware_Details *firmware,GHashTable *tests_hash)
{
	Detection_Test *test = NULL;
	if (firmware->TextVerVia == NULL)
		thread_update_widget(g_strdup("text_version_entry"),MTX_ENTRY,g_strdup(""));
	else
	{
		test = g_hash_table_lookup(tests_hash,firmware->TextVerVia);
		if (test)
		{
			if (test->num_bytes > 0)
				thread_update_widget(g_strdup("text_version_entry"),MTX_ENTRY,g_strndup(test->result_str,test->num_bytes));
		}
		else
			printf("couldn't find test results for the %s test\n",firmware->TextVerVia);
	}
	test = NULL;
	if (firmware->NumVerVia == NULL)
		thread_update_widget(g_strdup("ecu_revision_entry"),MTX_ENTRY,g_strdup(""));
	else
	{
		test = g_hash_table_lookup(tests_hash,firmware->NumVerVia);
		if (test)
		{
			if (test->num_bytes > 0)
				thread_update_widget(g_strdup("ecu_revision_entry"),MTX_ENTRY,g_strdup_printf("%.1f",((gint)(test->result_str[0]))/10.0));
		}
		else
			printf("couldn't find test results for the %s test\n",firmware->NumVerVia);
	}
	if (firmware->SignatureVia == NULL)
		thread_update_widget(g_strdup("ecu_signature_entry"),MTX_ENTRY,g_strdup(""));
	else
	{
		test = g_hash_table_lookup(tests_hash,firmware->SignatureVia);
		if (test)
		{
			if (test->num_bytes > 0)
			{
				thread_update_widget(g_strdup("ecu_signature_entry"),MTX_ENTRY,g_strndup(test->result_str,test->num_bytes));
				firmware->actual_signature = g_strndup(test->result_str,test->num_bytes);
			}
		}
		else
			printf("couldn't find test results for the %s test\n",firmware->SignatureVia);
	}

}
