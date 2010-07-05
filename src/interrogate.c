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
#include <crx.h>
#include <dataio.h>
#include <defines.h>
#include <debugging.h>
#include <dep_loader.h>
#include <enums.h>
#include <errno.h>
#include <getfiles.h>
#include <glib.h>
#include <init.h>
#include <interrogate.h>
#include <lookuptables.h>
#include <mode_select.h>
#include <mtxmatheval.h>
#include <multi_expr_loader.h>
#include <notifications.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>
#include <widgetmgmt.h>

extern gboolean connected;
extern GtkTextBuffer *textbuffer;
extern GtkWidget *interr_view;
extern gint dbg_lvl;
extern Serial_Params *serial_params;
extern GObject *global_data;
Firmware_Details *firmware = NULL;
gboolean interrogated = FALSE;


#define BUFSIZE 4096

/*!
 \brief interrogate_ecu() interrogates the target ECU to determine what
 firmware it is running.  It does this by reading a list of tests, sending
 those tests in turn, reading the responses and them comparing the group of
 responses against a list of interrogation profiles until it finds a match.
 */
EXPORT gboolean interrogate_ecu()
{
	GArray *tests = NULL;
	GHashTable *tests_hash = NULL;
	Detection_Test *test = NULL;
	guchar uint8 = 0;
	gchar sint8 = 0;
	guint16 uint16 = 0;
	gint16 sint16 = 0;
	gboolean res = 0;
	gint count = 0;
	gint i = 0;
	gint j = 0;
	gint len = 0;
	gint tests_to_run = 0;
	gint total_read = 0;
	gint total_wanted = 0;
	gint zerocount = 0;
	gchar *string = NULL;
	guchar buf[BUFSIZE];
	guchar *ptr = NULL;
	gchar * message = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	extern volatile gboolean offline;

	if (offline)
		return FALSE;
	/* prevent multiple runs of interrogator simultaneously */
	dbg_func(MUTEX,g_strdup_printf(__FILE__": interrogate_ecu() before lock reentrant mutex\n"));
	g_static_mutex_lock(&mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": interrogate_ecu() after lock reentrant mutex\n"));
	dbg_func(INTERROGATOR,g_strdup("\n"__FILE__": interrogate_ecu() ENTERED\n\n"));

	if (!connected)
	{
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": interrogate_ecu()\n\tNOT connected to ECU!!!!\n"));
		dbg_func(MUTEX,g_strdup_printf(__FILE__": interrogate_ecu() before UNlock reentrant mutex\n"));
		g_static_mutex_unlock(&mutex);
		dbg_func(MUTEX,g_strdup_printf(__FILE__": interrogate_ecu() after UNlock reentrant mutex\n"));
		return FALSE;
	}
	thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup("Interrogating ECU..."));

	/* Load tests from config files */
	tests = validate_and_load_tests(&tests_hash);

	if ((!tests) || (tests->len < 1))
	{
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": interrogate_ecu()\n\t validate_and_load_tests() didn't return a valid list of commands\n\t MegaTunix was NOT installed correctly, Aborting Interrogation\n"));
		dbg_func(MUTEX,g_strdup_printf(__FILE__": interrogate_ecu() before UNlock reentrant mutex\n"));
		g_static_mutex_unlock(&mutex);
		dbg_func(MUTEX,g_strdup_printf(__FILE__": interrogate_ecu() after UNlock reentrant mutex\n"));
		return FALSE;
	}
	thread_widget_set_sensitive("offline_button",FALSE);
	thread_widget_set_sensitive("interrogate_button",FALSE);
	/* how many tests.... */
	tests_to_run = tests->len;

	for (i=0;i<tests_to_run;i++)
	{
		flush_serial(serial_params->fd,BOTH);
		count = 0;
		test = g_array_index(tests,Detection_Test *, i);

		/* flush buffer to known state.. */
		memset (buf,0,BUFSIZE);

		ptr = buf;

		for (j=0;j<test->test_arg_count;j++)
		{
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_CHAR)
			{
				string = g_strdup(test->test_vector[j]);
				res = write_wrapper(serial_params->fd,string,1,&len);
				if (!res)
					interrogate_error("String Write error",j);
				dbg_func(INTERROGATOR,g_strdup_printf("\tSent command \"%s\"\n",string));
				g_free(string);
			}
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_U08)
			{
				uint8 = (guint8)atoi(test->test_vector[j]);
				res = write_wrapper(serial_params->fd,&uint8,1,&len);
				if (!res)
					interrogate_error("U08 Write error",j);
				dbg_func(INTERROGATOR,g_strdup_printf("\tSent command \"%i\"\n",uint8));
			}
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_U16)
			{
				uint16 = (guint16)atoi(test->test_vector[j]);
				res = write_wrapper(serial_params->fd,&uint16,2,&len);
				if (!res)
					interrogate_error("U16 Write error",j);
				dbg_func(INTERROGATOR,g_strdup_printf("\tSent command \"%i\"\n",uint8));
			}
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_S08)
			{
				sint8 = (gint8)atoi(test->test_vector[j]);
				res = write_wrapper(serial_params->fd,&sint8,1,&len);
				if (!res)
					interrogate_error("S08 Write error",j);
				dbg_func(INTERROGATOR,g_strdup_printf("\tSent command \"%i\"\n",sint8));
			}
			if (g_array_index(test->test_arg_types,DataSize,j) == MTX_S16)
			{
				sint16 = (gint16)atoi(test->test_vector[j]);
				res = write_wrapper(serial_params->fd,&sint16,2,&len);
				if (!res)
					interrogate_error("S16 Write error",j);
				dbg_func(INTERROGATOR,g_strdup_printf("\tSent command \"%i\"\n",sint8));
			}
		}

		total_read = 0;
		total_wanted = BUFSIZE;
		zerocount = 0;
		while ((total_read < total_wanted ) && (total_wanted-total_read) > 0 )
		{
			dbg_func(INTERROGATOR,g_strdup_printf("\tInterrogation for command %s requesting %i bytes\n",test->test_name,total_wanted-total_read));

			res = read_wrapper(serial_params->fd,
					ptr+total_read,
					total_wanted-total_read,&len);
			total_read += len;

			dbg_func(INTERROGATOR,g_strdup_printf("\tInterrogation for command %s read %i bytes, running total %i\n",test->test_name,len,total_read));
			/* If we get nothing back (i.e. timeout, inc counter)*/
			if ((!res) || (len == 0))
				zerocount++;

			if (zerocount > 2)
				break;
		}
		dbg_func(INTERROGATOR,g_strdup_printf("\tReceived %i bytes\n",total_read));
		ptr = buf;

		/* copy data from tmp buffer to struct pointer */
		test->num_bytes = total_read;
		if (total_read <= 0)
			test->result_str = g_strdup("");
		else
			test->result_str = g_strndup((gchar *)ptr, total_read);

		if (total_read > 0)
		{
			if (test->result_type == RESULT_TEXT)
				thread_update_logbar("interr_view",NULL,g_strdup_printf(_("Command \"%s\" (%s), returned %i bytes (%s)\n"),test->actual_test, test->test_desc,total_read,test->result_str),FALSE,FALSE);
			else if (test->result_type == RESULT_DATA)
				thread_update_logbar("interr_view",NULL,g_strdup_printf(_("Command \"%s\" (%s), returned %i bytes\n"),test->actual_test, test->test_desc,total_read),FALSE,FALSE);
			ptr = buf;
			if (dbg_lvl & (SERIAL_RD|INTERROGATOR))
			{
				dbg_func(SERIAL_RD|INTERROGATOR,g_strdup_printf(__FILE__": interrogate_ecu()\n\tRead the following from the %s command\n",test->test_name));
				message = g_strndup(((gchar *)buf),total_read);
				dbg_func(SERIAL_RD|INTERROGATOR,g_strdup_printf(__FILE__": interrogate.c()\n\tDumping Output string: \"%s\"\n",message));
				g_free(message);
				dbg_func(SERIAL_RD|INTERROGATOR,g_strdup("Data is in HEX!!\n"));
			}
			for (j=0;j<total_read;j++)
			{
				dbg_func(SERIAL_RD|INTERROGATOR,g_strdup_printf("%.2x ", ptr[j]));
				if (!((j+1)%8)) /* every 8 bytes give a CR */
				{
					dbg_func(SERIAL_RD|INTERROGATOR,g_strdup("\n"));
				}
			}
			dbg_func(SERIAL_RD|INTERROGATOR,g_strdup("\n\n"));
		}
	}

	interrogated = determine_ecu(tests,tests_hash);	
	if (interrogated)
	{
		thread_widget_set_sensitive("interrogate_button",FALSE);
		thread_widget_set_sensitive("offline_button",FALSE);
	}

	free_tests_array(tests);
	g_hash_table_destroy(tests_hash);

	if (!interrogated)
	{
		thread_widget_set_sensitive("interrogate_button",TRUE);
		thread_widget_set_sensitive("offline_button",TRUE);
	}

	dbg_func(MUTEX,g_strdup_printf(__FILE__": interrogate_ecu() before UNlock reentrant mutex\n"));
	g_static_mutex_unlock(&mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": interrogate_ecu() after UNlock reentrant mutex\n"));
	dbg_func(INTERROGATOR,g_strdup("\n"__FILE__": interrogate_ecu() LEAVING\n\n"));
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
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": determine_ecu()\n\t NO Interrogation profiles found,  was MegaTunix installed properly?\n\n"));
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
		if (test->result_type == RESULT_TEXT)
			dbg_func(INTERROGATOR,g_strdup_printf("\tCommand \"%s\" (%s), returned %i bytes (%s)\n",
						test->actual_test,
						test->test_desc,
						test->num_bytes,
						test->result_str));
		else if (test->result_type == RESULT_DATA)
			dbg_func(INTERROGATOR,g_strdup_printf("\tCommand \"%s\" (%s), returned %i bytes\n",
						test->actual_test,
						test->test_desc,
						test->num_bytes));
	}
	if (match == FALSE) /* (we DID NOT find one) */
	{
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__":\n\tdetermine_ecu()\n\tFirmware NOT DETECTED, Enable Interrogation debugging, retry interrogation,\nclose megatunix, and send ~/MTXlog.txt to the author for analysis with a note\ndescribing which firmware you are attempting to talk to.\n"));
		thread_update_logbar("interr_view","warning",g_strdup("Firmware NOT DETECTED, Enable Interrogation debugging, retry interrogation,\nclose megatunix, and send ~/MTXlog.txt to the author for analysis with a note\ndescribing which firmware you are attempting to talk to.\n"),FALSE,FALSE);
		return FALSE;
	}
	else
	{
		if (!firmware)
			firmware = g_new0(Firmware_Details,1);

		if (!load_firmware_details(firmware,filename))
			return FALSE;
		update_interrogation_gui(firmware);
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
		dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_firmware_details()\n\tFile \"%s\" NOT OPENED successfully\n",filename));
	get_file_api(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		thread_update_logbar("interr_view","warning",g_strdup_printf(_("Interrogation profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE);
		cfg_free(cfgfile);
		return FALSE;
	}

	firmware->profile_filename = g_strdup(filename);

	cfg_read_string(cfgfile,"interrogation_profile","name",&firmware->name);
	cfg_read_string(cfgfile,"parameters","TextVerVia",&firmware->TextVerVia);
	cfg_read_string(cfgfile,"parameters","NumVerVia",&firmware->NumVerVia);
	cfg_read_string(cfgfile,"parameters","SignatureVia",&firmware->SignatureVia);

	dbg_func(INTERROGATOR,g_strdup_printf(__FILE__": load_profile_details()\n\tfile:%s opened successfully\n",filename));
	if(!cfg_read_string(cfgfile,"parameters","Capabilities",
				&tmpbuf))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"Capabilities\" enumeration list not found in interrogation profile, ERROR\n"));
	else
	{
		firmware->capabilities = translate_capabilities(tmpbuf);
		g_free(tmpbuf);

		/*
		if (firmware->capabilities & MS1)
			printf("MS1\n");
		if (firmware->capabilities & MS1_STD)
			printf("MS1_STD\n");
		if (firmware->capabilities & MSNS_E)
			printf("MSNS_E\n");
		if (firmware->capabilities & MS1_DT)
			printf("MS1_DT\n");
		if (firmware->capabilities & MS2)
			printf("MS2\n");
		if (firmware->capabilities & MS2_STD)
			printf("MS2_STD\n");
		if (firmware->capabilities & MS2_E)
			printf("MS2_E\n");
		if (firmware->capabilities & MS2_E_COMPMON)
			printf("MS2_E_COMPMON\n");
			*/
	}
	if(!cfg_read_string(cfgfile,"parameters","RT_Command",
				&firmware->rt_command))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"RT_Command\" variable not found in interrogation profile, ERROR\n"));
	if (firmware->capabilities & PIS)
	{
		if(!cfg_read_int(cfgfile,"parameters","CLT_Table_Page",
					&firmware->clt_table_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"CLT_Table_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","MAT_Table_Page",
					&firmware->mat_table_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"MAT_Table_Page\" variable not found in interrogation profile, ERROR\n"));
	}
	if (firmware->capabilities & MS2)
	{
		if(!cfg_read_int(cfgfile,"parameters","MS2_RT_Page",
					&firmware->ms2_rt_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"MS2_RT_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","InterCharDelay",
					&firmware->interchardelay))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"InterCharDelay\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","CLT_Table_Page",
					&firmware->clt_table_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"CLT_Table_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","MAT_Table_Page",
					&firmware->mat_table_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"MAT_Table_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","EGO_Table_Page",
					&firmware->ego_table_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"EGO_Table_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","MAF_Table_Page",
					&firmware->maf_table_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"MAF_Table_Page\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_int(cfgfile,"parameters","RT_total_bytes",
				&firmware->rtvars_size))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"RT_total_bytes\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","Get_All_Command",
				&firmware->get_all_command))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"Get_All_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","VE_Command",
				&firmware->ve_command))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"VE_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","Write_Command",
				&firmware->write_command))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"Write_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","Burn_Command",
				&firmware->burn_command))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"Burn_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","Burn_All_Command",
				&firmware->burn_all_command))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"Burn_All_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_boolean(cfgfile,"parameters","MultiPage",
				&firmware->multi_page))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"MultiPage\" flag not found in parameters section in interrogation profile, ERROR\n"));
	if ((firmware->multi_page) && (!(firmware->capabilities & MS2)))
	{
		if(!cfg_read_string(cfgfile,"parameters","Page_Command",
					&firmware->page_command))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"Page_Command\" flag not found in parameters section in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_boolean(cfgfile,"parameters","ChunkWriteSupport",
				&firmware->chunk_support))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"ChunkWriteSupport\" flag not found in parameters section in interrogation profile, ERROR\n"));
	if (firmware->chunk_support)
	{
		if(!cfg_read_string(cfgfile,"parameters","Chunk_Write_Command",
					&firmware->chunk_write_command))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"Chunk_Write_Command\" flag not found in parameters section in interrogation profile, ERROR\n"));
	}
	if (firmware->capabilities & MS2)
	{
		if(!cfg_read_string(cfgfile,"parameters","Table_Write_Command",
					&firmware->table_write_command))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"Table_Write_Command\" flag not found in parameters section in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_int(cfgfile,"parameters","TotalPages",
				&firmware->total_pages))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"TotalPages\" value not found in interrogation profile, ERROR\n"));
	cfg_read_int(cfgfile,"parameters","ReadOnlyAbove",&firmware->ro_above);
	if(!cfg_read_int(cfgfile,"parameters","TotalTables",
				&firmware->total_tables))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"TotalTables\" value not found in interrogation profile, ERROR\n"));
	cfg_read_int(cfgfile,"parameters","TotalTETables",
					&firmware->total_te_tables);
	if ((firmware->capabilities & MS2_E) || (firmware->capabilities & MSNS_E))
	{
		if(!cfg_read_int(cfgfile,"parameters","TrigmonPage",&firmware->trigmon_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"TrigmonPage\" value not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","ToothmonPage",&firmware->toothmon_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"ToothmonPage\" value not found in interrogation profile, ERROR\n"));
		if (firmware->capabilities & MS2_E_COMPMON)
		{
			if(!cfg_read_int(cfgfile,"parameters","CompositemonPage",&firmware->compositemon_page))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"CompositemonPage\" value not found in interrogation profile, ERROR\n"));
		}
	}
	if(!cfg_read_string(cfgfile,"gui","LoadTabs",
				&tmpbuf))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"LoadTabs\" list not found in interrogation profile, ERROR\n"));
	else
	{
		firmware->tab_list = g_strsplit(tmpbuf,",",0);
		g_free(tmpbuf);
	}
	if(!cfg_read_string(cfgfile,"gui","TabConfs",
				&tmpbuf))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"TabConfs\" list not found in interrogation profile, ERROR\n"));
	else
	{
		firmware->tab_confs = g_strsplit(tmpbuf,",",0);
		g_free(tmpbuf);
	}
	if(!cfg_read_string(cfgfile,"gui","RealtimeMapFile",
				&firmware->rtv_map_file))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"RealtimeMapFile\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"gui","SliderMapFile",
				&firmware->sliders_map_file))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"SliderMapFile\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"gui","RuntimeTextMapFile",
				&firmware->rtt_map_file))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"RuntimeTextMapFile\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"gui","StatusMapFile",
				&firmware->status_map_file))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"StatusMapFile\" variable not found in interrogation profile, ERROR\n"));
	if (!cfg_read_string(cfgfile,"lookuptables","tables",
				&tmpbuf))
		dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"tables\" lookuptable name not found in interrogation profile, ERROR\n"));
	else
	{
		list = g_strsplit(tmpbuf,",",0);
		g_free(tmpbuf);
		i = 0;
		while (list[i] != NULL)
		{	
			if (!cfg_read_string(cfgfile,"lookuptables",list[i],&tmpbuf))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"%s\" key name not found in \"[lookuptables]\"\n\t section of interrogation profile, ERROR\n",list[i]));
			else
			{
				dbg_func(INTERROGATOR,g_strdup_printf(__FILE__": load_profile_details()\n\t \"[lookuptables]\"\n\t section loading table %s, file %s\n",list[i],tmpbuf));
				get_table(list[i],tmpbuf,NULL);
				g_free(tmpbuf);
			}
			i++;
		}
		g_strfreev(list);
	}

	/* Allocate space for Page Params structures.... */
	firmware->page_params = g_new0(Page_Params *,firmware->total_pages);
	for (i=0;i<firmware->total_pages;i++)
	{
		firmware->page_params[i] = initialize_page_params();
		section = g_strdup_printf("page_%i",i);

		if (firmware->multi_page)
			if(!cfg_read_int(cfgfile,section,"phys_ecu_page",&firmware->page_params[i]->phys_ecu_page))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"phys_ecu_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
		if(!cfg_read_boolean(cfgfile,section,"dl_by_default",&firmware->page_params[i]->dl_by_default))
		{
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"dl_by_default\" flag not found in \"%s\" section in interrogation profile, assuming TRUE\n",section));
			firmware->page_params[i]->dl_by_default = TRUE;
		}
		if(!cfg_read_int(cfgfile,section,"length",&firmware->page_params[i]->length))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"length\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
		g_free(section);
	}

	/* Allocate space for Table Offsets structures.... */
	firmware->table_params = g_new0(Table_Params *,firmware->total_tables);
	for (i=0;i<firmware->total_tables;i++)
	{
		firmware->table_params[i] = initialize_table_params();

		section = g_strdup_printf("table_%i",i);
		cfg_read_string(cfgfile,section,"bind_to_list",&firmware->table_params[i]->bind_to_list);
		if(cfg_read_string(cfgfile,section,"match_type",&tmpbuf))
		{
			firmware->table_params[i]->match_type = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		cfg_read_boolean(cfgfile,section,"is_spark",&firmware->table_params[i]->is_spark);
		cfg_read_boolean(cfgfile,section,"is_fuel",&firmware->table_params[i]->is_fuel);
		if ((firmware->table_params[i]->is_fuel) && !(firmware->capabilities & PIS))
		{
			if(!cfg_read_int(cfgfile,section,"divider_page",&firmware->table_params[i]->divider_page))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"divider_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"divider_offset",&firmware->table_params[i]->divider_offset))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"divider_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"reqfuel_page",&firmware->table_params[i]->reqfuel_page))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"reqfuel_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"reqfuel_offset",&firmware->table_params[i]->reqfuel_offset))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"reqfuel_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_string(cfgfile,section,"reqfuel_size",&tmpbuf))
				firmware->table_params[i]->reqfuel_size = MTX_U08;
			else
			{
				firmware->table_params[i]->reqfuel_size = translate_string(tmpbuf);
				g_free(tmpbuf);
			}
			if(!cfg_read_int(cfgfile,section,"stroke_page",&firmware->table_params[i]->stroke_page))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"stroke_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"stroke_offset",&firmware->table_params[i]->stroke_offset))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"stroke_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"stroke_mask",&firmware->table_params[i]->stroke_mask))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"stroke_mask\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"num_cyl_page",&firmware->table_params[i]->num_cyl_page))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"num_cyl_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"num_cyl_offset",&firmware->table_params[i]->num_cyl_offset))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"num_cyl_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"num_cyl_mask",&firmware->table_params[i]->num_cyl_mask))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"num_cyl_mask\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"num_inj_page",&firmware->table_params[i]->num_inj_page))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"num_inj_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"num_inj_offset",&firmware->table_params[i]->num_inj_offset))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"num_inj_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if(!cfg_read_int(cfgfile,section,"num_inj_mask",&firmware->table_params[i]->num_inj_mask))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"num_inj_mask\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			if (!(firmware->capabilities & MS2))
			{
				if(!cfg_read_int(cfgfile,section,"rpmk_page",&firmware->table_params[i]->rpmk_page))
					dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"rpmk_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
				if(!cfg_read_int(cfgfile,section,"rpmk_offset",&firmware->table_params[i]->rpmk_offset))
					dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"rpmk_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			}
			if (!(firmware->capabilities & MS1_DT))
			{
				if(!cfg_read_int(cfgfile,section,"alternate_page",&firmware->table_params[i]->alternate_page))
					dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"alternate_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
				if(!cfg_read_int(cfgfile,section,"alternate_offset",&firmware->table_params[i]->alternate_offset))
					dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"alternate_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			}
			if (firmware->capabilities & MSNS_E)
			{
				if(!cfg_read_int(cfgfile,section,"dtmode_offset",&firmware->table_params[i]->dtmode_offset))
					dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"dtmode_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
				if(!cfg_read_int(cfgfile,section,"dtmode_page",&firmware->table_params[i]->dtmode_page))
					dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"dtmode_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
				if(!cfg_read_int(cfgfile,section,"dtmode_mask",&firmware->table_params[i]->dtmode_mask))
					dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"dtmode_mask\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
			}
		}
		if(!cfg_read_int(cfgfile,section,"x_page",&firmware->table_params[i]->x_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"x_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
		if(!cfg_read_int(cfgfile,section,"y_page",&firmware->table_params[i]->y_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"y_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
		if(!cfg_read_int(cfgfile,section,"z_page",&firmware->table_params[i]->z_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"z_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
		if(!cfg_read_int(cfgfile,section,"x_base_offset",&firmware->table_params[i]->x_base))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"y_base_offset",&firmware->table_params[i]->y_base))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"z_base_offset",&firmware->table_params[i]->z_base))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"x_bincount",&firmware->table_params[i]->x_bincount))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_bincount\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_size",&tmpbuf))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->table_params[i]->x_size = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"y_size",&tmpbuf))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->table_params[i]->y_size = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"z_size",&tmpbuf))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->table_params[i]->z_size = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_int(cfgfile,section,"y_bincount",&firmware->table_params[i]->y_bincount))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_bincount\" variable not found in interrogation profile, ERROR\n"));
		cfg_read_boolean(cfgfile,section,"x_multi_source",&firmware->table_params[i]->x_multi_source);
		if (firmware->table_params[i]->x_multi_source)
		{
			/* READ multi-source stuff, but do NOT create
			 * evaluators,  we do that in the final copy
			 * over to the firmware struct
			 */
			if(!cfg_read_string(cfgfile,section,"x_multi_expr_keys",&firmware->table_params[i]->x_multi_expr_keys))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_multi_expr_keys\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_source_key",&firmware->table_params[i]->x_source_key))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_source_key\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_sources",&firmware->table_params[i]->x_sources))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_sources\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_suffixes",&firmware->table_params[i]->x_suffixes))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_suffixes\" variable not found in interrogation profile, ERROR\n"));
			cfg_read_string(cfgfile,section,"x_conv_expr",&firmware->table_params[i]->x_conv_expr);
			if(!cfg_read_string(cfgfile,section,"x_conv_exprs",&firmware->table_params[i]->x_conv_exprs))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_conv_exprs\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_precisions",&firmware->table_params[i]->x_precisions))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_precisions\" variable not found in interrogation profile, ERROR\n"));
		}
		else
		{
			if(!cfg_read_string(cfgfile,section,"x_source",&firmware->table_params[i]->x_source))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_source\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_suffix",&firmware->table_params[i]->x_suffix))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_suffix\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_conv_expr",&firmware->table_params[i]->x_conv_expr))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_conv_expr\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_int(cfgfile,section,"x_precision",&firmware->table_params[i]->x_precision))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"x_precision\" variable not found in interrogation profile for table %i, ERROR\n",i));
		}
		cfg_read_boolean(cfgfile,section,"y_multi_source",&firmware->table_params[i]->y_multi_source);
		if (firmware->table_params[i]->y_multi_source)
		{
			/* READ multi-source stuff, but do NOT create
			 * evaluators,  we do that in the final copy
			 * over to the firmware struct
			 */
			if(!cfg_read_string(cfgfile,section,"y_multi_expr_keys",&firmware->table_params[i]->y_multi_expr_keys))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_multi_expr_keys\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_source_key",&firmware->table_params[i]->y_source_key))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_source_key\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_sources",&firmware->table_params[i]->y_sources))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_sources\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_suffixes",&firmware->table_params[i]->y_suffixes))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_suffixes\" variable not found in interrogation profile, ERROR\n"));
			cfg_read_string(cfgfile,section,"y_conv_expr",&firmware->table_params[i]->y_conv_expr);
			if(!cfg_read_string(cfgfile,section,"y_conv_exprs",&firmware->table_params[i]->y_conv_exprs))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_conv_exprs\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_precisions",&firmware->table_params[i]->y_precisions))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_precisions\" variable not found in interrogation profile, ERROR\n"));

		}
		else
		{
			if(!cfg_read_string(cfgfile,section,"y_source",&firmware->table_params[i]->y_source))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_source\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_suffix",&firmware->table_params[i]->y_suffix))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_suffix\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_conv_expr",&firmware->table_params[i]->y_conv_expr))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_conv_expr\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_int(cfgfile,section,"y_precision",&firmware->table_params[i]->y_precision))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"y_precision\" variable not found in interrogation profile for table %i, ERROR\n",i));
		}
		cfg_read_boolean(cfgfile,section,"z_multi_source",&firmware->table_params[i]->z_multi_source);
		if(firmware->table_params[i]->z_multi_source)
		{
			/* READ multi-source stuff, but do NOT create
			 * evaluators,  we do that in the final copy
			 * over to the firmware struct
			 */
			if(!cfg_read_string(cfgfile,section,"z_multi_expr_keys",&firmware->table_params[i]->z_multi_expr_keys))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_multi_expr_keys\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_source_key",&firmware->table_params[i]->z_source_key))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_source_key\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_sources",&firmware->table_params[i]->z_sources))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_sources\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_suffixes",&firmware->table_params[i]->z_suffixes))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_suffixes\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_conv_exprs",&firmware->table_params[i]->z_conv_exprs))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_conv_exprs\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_precisions",&firmware->table_params[i]->z_precisions))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_precisions\" variable not found in interrogation profile, ERROR\n"));

		}
		else
		{
			if(!cfg_read_string(cfgfile,section,"z_source",&firmware->table_params[i]->z_source))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_source\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_suffix",&firmware->table_params[i]->z_suffix))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_suffix\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_conv_expr",&firmware->table_params[i]->z_conv_expr))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_conv_expr\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_int(cfgfile,section,"z_precision",&firmware->table_params[i]->z_precision))
				dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"z_precision\" variable not found in interrogation profile for table %i, ERROR\n",i));
			if(cfg_read_string(cfgfile,section,"z_depend_on",&firmware->table_params[i]->z_depend_on))
			{
				load_dependancies(firmware->table_params[i]->z_object,cfgfile,section,"z_depend_on");
				if(!cfg_read_string(cfgfile,section,"z_alt_lookuptable",&tmpbuf))
					dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_alt_lookuptable\" variable not found in interrogation profile, NOT NECESSARILY AN ERROR\n"));
				else
				{
					OBJ_SET(firmware->table_params[i]->z_object,"alt_lookuptable",g_strdup(tmpbuf));
					g_free(tmpbuf);
				}
				if(!cfg_read_string(cfgfile,section,"z_lookuptable",&tmpbuf))
					dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"z_lookuptable\" variable not found in interrogation profile, NOT NECESSARILY AN ERROR\n"));
				else
				{
					OBJ_SET(firmware->table_params[i]->z_object,"lookuptable",g_strdup(tmpbuf));
					g_free(tmpbuf);
				}
			}
		}
		if(!cfg_read_string(cfgfile,section,"table_name",&firmware->table_params[i]->table_name))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"table_name\" variable not found in interrogation profile, ERROR\n"));
		g_free(section);
	}

	/* Allocate space for Table Editor structures.... */
	firmware->te_params = g_new0(TE_Params *,firmware->total_te_tables);
	for (i=0;i<firmware->total_te_tables;i++)
	{
		firmware->te_params[i] = initialize_te_params();

		section = g_strdup_printf("te_table_%i",i);
		cfg_read_boolean(cfgfile,section,"x_lock",&firmware->te_params[i]->x_lock);
		cfg_read_boolean(cfgfile,section,"y_lock",&firmware->te_params[i]->y_lock);
		cfg_read_boolean(cfgfile,section,"reversed",&firmware->te_params[i]->reversed);
		cfg_read_string(cfgfile,section,"bind_to_list",&firmware->te_params[i]->bind_to_list);
		if(cfg_read_string(cfgfile,section,"match_type",&tmpbuf))
		{
			firmware->te_params[i]->match_type = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		cfg_read_boolean(cfgfile,section,"gauge_temp_dep",&firmware->te_params[i]->gauge_temp_dep);
		cfg_read_string(cfgfile,section,"gauge",&firmware->te_params[i]->gauge);
		cfg_read_string(cfgfile,section,"c_gauge",&firmware->te_params[i]->c_gauge);
		cfg_read_string(cfgfile,section,"f_gauge",&firmware->te_params[i]->f_gauge);
		if (firmware->te_params[i]->f_gauge || firmware->te_params[i]->c_gauge || firmware->te_params[i]->gauge)	
			cfg_read_string(cfgfile,section,"gauge_datasource",&firmware->te_params[i]->gauge_datasource);
		cfg_read_boolean(cfgfile,section,"x_use_color",&firmware->te_params[i]->x_use_color);
		cfg_read_boolean(cfgfile,section,"y_use_color",&firmware->te_params[i]->y_use_color);
		cfg_read_boolean(cfgfile,section,"x_temp_dep",&firmware->te_params[i]->x_temp_dep);
		cfg_read_boolean(cfgfile,section,"y_temp_dep",&firmware->te_params[i]->y_temp_dep);
		if(!cfg_read_string(cfgfile,section,"x_axis_label",&firmware->te_params[i]->x_axis_label))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"x_axis_label\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
		if(!cfg_read_string(cfgfile,section,"y_axis_label",&firmware->te_params[i]->y_axis_label))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"y_axis_label\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
		if(!cfg_read_int(cfgfile,section,"x_page",&firmware->te_params[i]->x_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"x_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
		if(!cfg_read_int(cfgfile,section,"y_page",&firmware->te_params[i]->y_page))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"y_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n",section));
		if(!cfg_read_int(cfgfile,section,"x_base_offset",&firmware->te_params[i]->x_base))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"y_base_offset",&firmware->te_params[i]->y_base))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"bincount",&firmware->te_params[i]->bincount))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_bincount\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_size",&tmpbuf))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->te_params[i]->x_size = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"y_size",&tmpbuf))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->te_params[i]->y_size = translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"x_source",&firmware->te_params[i]->x_source))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_source\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_units",&firmware->te_params[i]->x_units))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_units\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_name",&firmware->te_params[i]->x_name))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_name\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_dl_conv_expr",&firmware->te_params[i]->x_dl_conv_expr))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_dl_conv_expr\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_ul_conv_expr",&firmware->te_params[i]->x_ul_conv_expr))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"x_ul_conv_expr\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"x_raw_lower",&firmware->te_params[i]->x_raw_lower))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"x_raw_lower\" variable not found in interrogation profile for table %i, ERROR\n",i));
		if(!cfg_read_int(cfgfile,section,"x_raw_upper",&firmware->te_params[i]->x_raw_upper))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"x_raw_upper\" variable not found in interrogation profile for table %i, ERROR\n",i));
		if(!cfg_read_int(cfgfile,section,"x_precision",&firmware->te_params[i]->x_precision))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"x_precision\" variable not found in interrogation profile for table %i, ERROR\n",i));
		if(!cfg_read_string(cfgfile,section,"y_units",&firmware->te_params[i]->y_units))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_units\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"y_name",&firmware->te_params[i]->y_name))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_name\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"y_dl_conv_expr",&firmware->te_params[i]->y_dl_conv_expr))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_dl_conv_expr\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"y_ul_conv_expr",&firmware->te_params[i]->y_ul_conv_expr))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"y_ul_conv_expr\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"y_raw_lower",&firmware->te_params[i]->y_raw_lower))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"y_raw_lower\" variable not found in interrogation profile for table %i, ERROR\n",i));
		if(!cfg_read_int(cfgfile,section,"y_raw_upper",&firmware->te_params[i]->y_raw_upper))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"y_raw_upper\" variable not found in interrogation profile for table %i, ERROR\n",i));
		if(!cfg_read_int(cfgfile,section,"y_precision",&firmware->te_params[i]->y_precision))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": load_profile_details()\n\t\"y_precision\" variable not found in interrogation profile for table %i, ERROR\n",i));
		if(!cfg_read_string(cfgfile,section,"title",&firmware->te_params[i]->title))
			dbg_func(INTERROGATOR|CRITICAL,g_strdup(__FILE__": load_profile_details()\n\t\"title\" variable not found in interrogation profile, ERROR\n"));
		g_free(section);
	}

	cfg_free(cfgfile);
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
				printf(_("X multi_sources length mismatch!\n"));
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
				printf(_("Y multi_sources length mismatch!\n"));
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
				printf(_("Z multi_sources length mismatch!\n"));
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

	dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": determine_ecu()\n\tDetected Firmware: %s\n",firmware->name));
	thread_update_logbar("interr_view","warning",g_strdup_printf(_("Detected Firmware: %s\n"),firmware->name),FALSE,FALSE);
	thread_update_logbar("interr_view","info",g_strdup_printf(_("Loading Settings from: \"%s\"\n"),firmware->profile_filename),FALSE,FALSE);

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
		thread_update_logbar("interr_view","warning",g_strdup_printf(_("Interrogation profile tests API mismatch (%i.%i != %i.%i):\n\tFile %s.\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE);
		return NULL;
	}

	*tests_hash = g_hash_table_new(g_str_hash,g_str_equal);

	dbg_func(INTERROGATOR,g_strdup_printf(__FILE__": validate_and_load_tests()\n\tfile %s, opened successfully\n",filename));
	tests = g_array_new(FALSE,TRUE,sizeof(Detection_Test *));
	cfg_read_int(cfgfile,"interrogation_tests","total_tests",&total_tests);
	for (i=0;i<total_tests;i++)
	{
		test = g_new0(Detection_Test, 1);
		section = g_strdup_printf("test_%.2i",i);
		if (!cfg_read_string(cfgfile,section,"test_name",&test->test_name))
		{
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": validate_and_load_tests(),\n\ttest_name for %s is NULL\n",section));
			g_free(section);
			break;
		}
		if (!cfg_read_string(cfgfile,section,"test_result_type",&tmpbuf))
		{
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": validate_and_load_tests(),\n\ttest_result_type for %s is NULL\n",section));
			g_free(section);
			break;
		}
		else
		{
			test->result_type=translate_string(tmpbuf);
			g_free(tmpbuf);
		}
		if (!cfg_read_string(cfgfile,section,"actual_test",&test->actual_test))
		{
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": validate_and_load_tests(),\n\tactual_test for %s is NULL\n",section));
			g_free(section);
			break;
		}
		if (!cfg_read_string(cfgfile,section,"test_arg_types",&tmpbuf))
		{
			dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": validate_and_load_tests(),\n\ttest_arg_types for %s is NULL\n",section));
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
		dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": translate_capabilities()\n\tstring fed is NULLs\n"));
		return -1;
	}

	vector = g_strsplit(string,",",0);
	dbg_func(INTERROGATOR,g_strdup_printf(__FILE__": translate_capabilities()\n\tstring fed is %s\n",string));
	while (vector[i] != NULL)
	{
		dbg_func(INTERROGATOR,g_strdup_printf(__FILE__": translate_capabilities()\n\tTrying to translate %s\n",vector[i]));
		value += translate_string(vector[i]);
		dbg_func(INTERROGATOR,g_strdup_printf(__FILE__": translate_capabilities()\n\tTranslated value of %s is %i\n",vector[i],value));
		i++;
	}

	g_strfreev(vector);
	return value;	
}


/*!
 \brief check_for_match() compares the resutls of the interrogation with the
 ECU to the canidates in turn. When a match occurs TRUE is returned
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
	guint i = 0;
	gint len = 0;
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
		thread_update_logbar("interr_view","warning",g_strdup_printf(_("Interrogation profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE);
		cfg_free(cfgfile);
		return FALSE;
	}

	if (cfg_read_string(cfgfile,"interrogation","match_on",&tmpbuf) == FALSE)
		printf(_("ERROR:!! \"match_on\" key missing from interrogation profile [interrogation] section\n"));
	match_on = g_strsplit(tmpbuf,",",-1);
	g_free(tmpbuf);

	for (i=0;i<g_strv_length(match_on);i++)
	{
		pass = FALSE;
		/*printf("checking for match on %s\n",match_on[i]);*/
		test = g_hash_table_lookup(tests_hash,match_on[i]);
		if (!test)
		{
			printf(_("ERROR test data not found for test \"%s\"\n"),match_on[i]);
			continue;
		}

		/* If the test_name is NOT IN the interrogation profile,  we 
		 * abort as it's NOT match
		 */
		if (!cfg_read_string(cfgfile,"interrogation",test->test_name,&tmpbuf))
		{
			dbg_func(INTERROGATOR,g_strdup_printf("\n"__FILE__": check_for_match()\n\tMISMATCH,\"%s\" is NOT a match...\n\n",filename));
			cfg_free(cfgfile);
			g_strfreev(match_on);
			return FALSE;
		}
		vector = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
		/* Possible choices are "Count", "submatch" and "fullmatch", so
		 * stringparse to get them into a consistent form
		 */
		if (g_strv_length(vector) != 2)
			printf(_("ERROR interrogation check_for match vector does NOT have two args it has %i\n"),g_strv_length(vector));
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
			case REGEX:
				if (regex(vector[1],test->result_str,&len))
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
			dbg_func(INTERROGATOR,g_strdup_printf("\n"__FILE__": check_for_match()\n\tMISMATCH,\"%s\" is NOT a match...\n\n",filename));
			g_strfreev(match_on);
			cfg_free(cfgfile);
			return FALSE;
		}

	}
	g_strfreev(match_on);
	dbg_func(INTERROGATOR,g_strdup_printf("\n"__FILE__": check_for_match()\n\t\"%s\" is a match for all conditions ...\n\n",filename));
	cfg_free(cfgfile);
	return TRUE;
}


/*! brief destroys Array holding Detection_Test structures
 */
void free_tests_array(GArray *tests)
{
	guint i = 0;
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
	dbg_func(INTERROGATOR|CRITICAL,g_strdup_printf("\tInterrogation error: \"%s\" data: \"%i\"n",text,num));
}


/* !brief updates the interrogation gui with the text revision, signature
 * and ecu numerical revision
 */
void update_interrogation_gui(Firmware_Details *firmware)
{
	if (firmware->TextVerVia)
		io_cmd(firmware->TextVerVia,NULL);
	if (firmware->NumVerVia)
		io_cmd(firmware->NumVerVia,NULL);
	if (firmware->SignatureVia)
		io_cmd(firmware->SignatureVia,NULL);
}
