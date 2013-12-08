/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/mscommon/interrogate.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS Specific device interrogation/detection routines
  \author David Andruczyk
  */

#include <api-versions.h>
#include <dep_loader.h>
#include <getfiles.h>
#include <interrogate.h>
#include <libgen.h>
#include <mscommon_comms.h>
#include <mscommon_plugin.h>
#include <multi_expr_loader.h>
#include <serialio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern GtkWidget *interr_view;
extern gconstpointer *global_data;

#define BUFSIZE 4096

/*!
 \brief Iinterrogates the target ECU to determine what
 firmware it is running.  It does this by reading a list of tests, sending
 those tests in turn, reading the responses and them comparing the group of
 responses against a list of interrogation profiles until it finds a match.
 \returns TRUE on a success, FALSE otherwise
 */
G_MODULE_EXPORT gboolean interrogate_ecu(void)
{
	static GMutex mutex;
	gboolean interrogated = FALSE;
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
	gint adder = 0;
	gint base_offset = 0;
	unsigned long crc32 = 0;
	gint crc_pass = 0;
	gint crc_fail = 0;
	gchar *string = NULL;
	guchar buf[BUFSIZE];
	guchar *ptr = NULL;
	gchar * message = NULL;
	Serial_Params *serial_params = NULL;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	if (DATA_GET(global_data,"offline"))
	{
		EXIT();
		return FALSE;
	}
	/* prevent multiple runs of interrogator simultaneously */
	g_mutex_lock(&mutex);
	MTXDBG(INTERROGATOR,_("Entered\n"));

	if (!DATA_GET(global_data,"connected"))
	{
		MTXDBG(INTERROGATOR,_("NOT connected to ECU!!!!\n"));
		g_mutex_unlock(&mutex);
		EXIT();
		return FALSE;
	}
	thread_update_widget_f("titlebar",MTX_TITLE,g_strdup(_("Interrogating ECU...")));

	/* Load tests from config files */
	tests = validate_and_load_tests(&tests_hash);


	if ((!tests) || (tests->len < 1))
	{
		MTXDBG(INTERROGATOR|CRITICAL,_("validate_and_load_tests() didn't return a valid list of commands\n\t MegaTunix was NOT installed correctly, Aborting Interrogation\n"));
		update_logbar_f("interr_view",NULL,g_strdup(__FILE__": interrogate_ecu()\n\t validate_and_load_tests() didn't return a valid list of commands\n\t MegaTunix was NOT installed correctly, Aborting Interrogation\n"),FALSE,FALSE,TRUE);
		g_mutex_unlock(&mutex);
		EXIT();
		return FALSE;
	}
	thread_widget_set_sensitive_f("offline_button",FALSE);
	thread_widget_set_sensitive_f("interrogate_button",FALSE);
	/* how many tests.... */
	tests_to_run = tests->len;

	for (i=0;i<tests_to_run;i++)
	{
		flush_serial_f(serial_params->fd,BOTH);
		count = 0;
		test = g_array_index(tests,Detection_Test *, i);

		/* flush buffer to known state.. */
		memset (buf,0,BUFSIZE);

		ptr = buf;

		string = g_strdup(test->actual_test);
		res = write_wrapper_f(serial_params->fd,test->send_buf,test->send_len,&len);
		if (!res)
			MTXDBG(INTERROGATOR|CRITICAL,_("Interrogation error: Unable to send data: \"%s\"n"),(gchar *)g_strndup((const gchar *)test->send_buf,test->send_len));
		MTXDBG(INTERROGATOR,_("Sent command \"%s\"\n"),string);
		g_free(string);

		total_read = 0;
		total_wanted = BUFSIZE;
		zerocount = 0;
		while ((total_read < total_wanted ) && (total_wanted-total_read) > 0 )
		{
			MTXDBG(INTERROGATOR,_("Interrogation for command %s requesting %i bytes\n"),test->test_name,total_wanted-total_read);

			res = read_wrapper_f(serial_params->fd,
					ptr+total_read,
					total_wanted-total_read,&len);
			total_read += len;

			MTXDBG(INTERROGATOR,_("Interrogation for command %s read %i bytes, running total %i\n"),test->test_name,len,total_read);
			/* If we get nothing back (i.e. timeout, inc counter)*/
			if ((!res) || (len == 0))
				zerocount++;

			if (zerocount > 1)
				break;
		}
		MTXDBG(INTERROGATOR,_("Received %i bytes\n"),total_read);
		ptr = buf;
		test->recv_len = total_read;
		/* copy data from tmp buffer to struct pointer */
		if (test->ms3_crc32)
		{
			adder = 7;
			base_offset = 3;
			len = buf[0]*256 + buf[1];
			if ((len == 0 ) || (len > 2054))
				printf("packet length INVALID\n");
			if ((len + 6) != test->recv_len)
				printf("data length received DOES NOT MATCH packet length header\n");
			crc32 = crc32_computebuf(0,&buf[2],len);
			if ( (((crc32 >> 24) & 0xff) != buf[2 + len])
					|| (((crc32 >> 16) & 0xff) != buf[3 + len])
					|| (((crc32 >> 8) & 0xff) != buf[4 + len])
					|| ((crc32 & 0xff) != buf[5 + len])) {
				printf("CRC32 validation FAILED\n");
				crc_fail++;
			}
			else
			{
				printf("CRC32 validation SUCCEEDED\n");
				crc_pass++;
			}
			if (buf[2] & 0x80)
				printf("Packet contains error code 0x%x\n",buf[2] & 0xff);
			else
				printf("Packet contains status code 0x%x\n",buf[2] & 0xff);
		}
		if (total_read <= (0 + adder))
			test->result_str = g_strdup("");
		else
			test->result_str = g_strndup((gchar *)ptr+base_offset, total_read-adder);

		if (total_read > (0 + adder))
		{
			if (test->result_type == RESULT_TEXT)
				update_logbar_f("interr_view",NULL,g_strdup_printf(_("Command \"%s\" (%s), returned %i bytes \"%s\"\n"),test->actual_test, test->test_desc,total_read,test->result_str),FALSE,FALSE,TRUE);
			else if (test->result_type == RESULT_DATA)
				update_logbar_f("interr_view",NULL,g_strdup_printf(_("Command \"%s\" (%s), returned %i bytes\n"),test->actual_test, test->test_desc,total_read),FALSE,FALSE,TRUE);
			ptr = buf;
			MTXDBG(SERIAL_RD|INTERROGATOR,_("Read the following from the %s command\n"),test->test_name);
			message = g_strndup(((gchar *)buf),total_read);
			MTXDBG(SERIAL_RD|INTERROGATOR,_("Dumping Output string: \"%s\"\n"),message);
			g_free(message);
			QUIET_MTXDBG(SERIAL_RD|INTERROGATOR,_("Data is in HEX!!\n"));
			for (j=0;j<total_read;j++)
			{
				QUIET_MTXDBG(SERIAL_RD|INTERROGATOR,"%.2x ", ptr[j]);
				if (!((j+1)%8)) /* every 8 bytes give a CR */
					QUIET_MTXDBG(SERIAL_RD|INTERROGATOR,"\n");
			}
			QUIET_MTXDBG(SERIAL_RD|INTERROGATOR,"\n\n");
		}
	}
	interrogated = determine_ecu(tests,tests_hash);	
	DATA_SET(global_data,"interrogated",GINT_TO_POINTER(interrogated));
	if (interrogated)
	{
		thread_widget_set_sensitive_f("interrogate_button",FALSE);
		thread_widget_set_sensitive_f("offline_button",FALSE);
	}

	free_tests_array(tests);
	g_hash_table_destroy(tests_hash);

	if (!interrogated)
	{
		thread_widget_set_sensitive_f("interrogate_button",TRUE);
		thread_widget_set_sensitive_f("offline_button",TRUE);
	}

	g_mutex_unlock(&mutex);
	MTXDBG(INTERROGATOR,_("Leaving\n"));
	thread_update_widget_f("titlebar",MTX_TITLE,g_strdup("Interrogation Complete..."));
	EXIT();
	return interrogated;
}


/*!
 \brief Tries to match to determine the target firmware by 
 loading the interrogation profiles in turn and comparing the data from our
 test ECU and a profile until a match is found, 
 \param tests is a pointer to the Array of tests
 \param tests_hash is a pointer to the hashtable of tests
 \returns TRUE on successfull match, FALSE on no match
 */
G_MODULE_EXPORT gboolean determine_ecu(GArray *tests,GHashTable *tests_hash)
{
	gboolean retval = TRUE;
	gint i = 0;
	Detection_Test *test = NULL;
	gint num_tests = tests->len;
	gboolean match = FALSE;
	gchar * filename = NULL;
	gchar ** filenames = NULL;
	GArray *classes = NULL;
	Firmware_Details *firmware = NULL;
	gchar *pathstub = NULL;

	ENTER();
	pathstub = g_build_filename(INTERROGATOR_DATA_DIR,"Profiles",DATA_GET(global_data,"ecu_family"),NULL);
	filenames = get_files((const gchar *)DATA_GET(global_data,"project_name"),pathstub,"prof",&classes);
	g_free(pathstub);
	if (!filenames)
	{
		MTXDBG(INTERROGATOR|CRITICAL,_("NO Interrogation profiles found,  was MegaTunix installed properly?\n"));
		EXIT();
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
			MTXDBG(INTERROGATOR,_("Command \"%s\" (%s), returned %i bytes \"%s\"\n"),
						test->actual_test,
						test->test_desc,
						test->recv_len,
						test->result_str);
		else if (test->result_type == RESULT_DATA)
			MTXDBG(INTERROGATOR,_("Command \"%s\" (%s), returned %i bytes\n"),
						test->actual_test,
						test->test_desc,
						test->recv_len);
	}
	if (match == FALSE) /* (we DID NOT find one) */
	{
		MTXDBG(INTERROGATOR,_("Firmware NOT DETECTED, Enable Interrogation debugging, retry interrogation,\nclose megatunix, and send ~/mtx/%s/debug.log to the author for analysis with a note\ndescribing which firmware you are attempting to talk to.\n"),(gchar *)DATA_GET(global_data,"project_name"));
		update_logbar_f("interr_view","warning",g_strdup_printf("Firmware NOT DETECTED, Enable Interrogation debugging, retry interrogation,\nclose megatunix, and send ~/mtx/%s/debug.log to the author for analysis with a note\ndescribing which firmware you are attempting to talk to.\n",(gchar *)DATA_GET(global_data,"project_name")),FALSE,FALSE,TRUE);
		retval = FALSE;
	}
	else
	{
		if (!firmware)
		{
			firmware = g_new0(Firmware_Details,1);
			DATA_SET(global_data,"firmware",firmware);
		}

		if (!load_firmware_details(firmware,filename))
			retval = FALSE;
	}
	g_free(filename);
	EXIT();
	return(retval);
}


/*! 
 \brief loads up all firmware details allocating the required resrouces
 \param firmware is the pointer to the firmware datastructure
 \param filename is the pointer to the filename to parse
 \returns TRUE on success, FALSE otherwise
 */
G_MODULE_EXPORT gboolean load_firmware_details(Firmware_Details *firmware, const gchar * filename)
{
	ConfigFile *cfgfile;
	gchar * tmpbuf = NULL;
	gchar * section = NULL;
	gchar ** list = NULL;
	gint i = 0;
	MultiSource * multi = NULL;
	gchar **sources = NULL;
	gchar **suffixes = NULL;
	gchar **fromecu_mults = NULL;
	gchar **fromecu_adds = NULL;
	gchar **precisions = NULL;
	gchar **expr_keys = NULL;
	gchar **tables = NULL;
	gfloat tmpf = 0.0;
	gint major = 0;
	gint minor = 0;
	gint len1 = 0;
	gint len2 = 0;
	gint len3 = 0;
	gint len4 = 0;
	gint len5 = 0;
	gint len6 = 0;
	gint j = 0;

	ENTER();
	g_return_val_if_fail(firmware,FALSE);
	g_return_val_if_fail(filename,FALSE);

	cfgfile = cfg_open_file((gchar *)filename);
	if (!cfgfile)
	{
		MTXDBG(INTERROGATOR|CRITICAL,_("File \"%s\" NOT OPENED successfully\n"),filename);
		EXIT();
		return FALSE;
	}
	get_file_api_f(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		thread_update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE);
		cfg_free(cfgfile);
		EXIT();
		return FALSE;
	}

	firmware->profile_filename = g_strdup(filename);

	cfg_read_string(cfgfile,"interrogation_profile","name",&firmware->name);
	cfg_read_string(cfgfile,"parameters","TextVerVia",&firmware->TextVerVia);
	cfg_read_string(cfgfile,"parameters","NumVerVia",&firmware->NumVerVia);
	cfg_read_string(cfgfile,"parameters","SignatureVia",&firmware->SignatureVia);
	if(cfg_read_string(cfgfile,"parameters","EcuTempUnits",&tmpbuf))
	{
		firmware->ecu_temp_units = (TempUnits)translate_string_f(tmpbuf);
		g_free(tmpbuf);
	}
	else
		MTXDBG(INTERROGATOR,_("Failed to find EcuTempUnits key in interrogation profile\n"));

	MTXDBG(INTERROGATOR,_("File:%s opened successfully\n"),filename);
	if(!cfg_read_boolean(cfgfile,"parameters","BigEndian",&firmware->bigendian))
	{
		MTXDBG(INTERROGATOR|CRITICAL,_("\"BigEndian\" key not found in interrogation profile, assuming ECU firmware byte order is big endian, ERROR in interrogation profile\n"));
		firmware->bigendian = TRUE;
	}
	if(!cfg_read_string(cfgfile,"parameters","Capabilities",
				&tmpbuf))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"Capabilities\" enumeration list not found in interrogation profile, ERROR\n"));
	else
	{
		/*printf("Capabilities %s\n",tmpbuf);*/
		firmware->capabilities = translate_capabilities(tmpbuf);
		g_free(tmpbuf);
		/*
		   printf("CAP #'s MS1 %i MS1_STD %i MS1_E %i MS1_DT %i MS2 %i MS2_STD %i, MS2_E %i, MS2_E_COMPMON %i, PIS %i, JIMSTIM %i\n",MS1,MS1_STD,MS1_E,MS1_DT,MS2,MS2_STD,MS2_E,MS2_E_COMPMON,PIS,JIMSTIM);

		   if (firmware->capabilities & MS1)
		   printf("MS1\n");
		   if (firmware->capabilities & MS1_STD)
		   printf("MS1_STD\n");
		   if (firmware->capabilities & MS1_E)
		   printf("MS1_E\n");
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
		   if (firmware->capabilities & PIS)
		   printf("PIS\n");
		   if (firmware->capabilities & JIMSTIM)
		   printf("JIMSTIM\n");
		 */
	}
	if(!cfg_read_string(cfgfile,"parameters","RT_Command",
				&firmware->rt_command))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"RT_Command\" variable not found in interrogation profile, ERROR\n"));
	if (firmware->capabilities & PIS)
	{
		if(!cfg_read_int(cfgfile,"parameters","CLT_Table_Page",
					&firmware->clt_table_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"CLT_Table_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","MAT_Table_Page",
					&firmware->mat_table_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"MAT_Table_Page\" variable not found in interrogation profile, ERROR\n"));
	}
	if (firmware->capabilities & MS2)
	{
		if(!cfg_read_int(cfgfile,"parameters","MS2_RT_Page",
					&firmware->ms2_rt_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"MS2_RT_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","InterCharDelay",
					&firmware->interchardelay))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"InterCharDelay\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","CLT_Table_Page",
					&firmware->clt_table_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"CLT_Table_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","MAT_Table_Page",
					&firmware->mat_table_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"MAT_Table_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","EGO_Table_Page",
					&firmware->ego_table_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"EGO_Table_Page\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","MAF_Table_Page",
					&firmware->maf_table_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"MAF_Table_Page\" variable not found in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_int(cfgfile,"parameters","RT_total_bytes",
				&firmware->rtvars_size))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"RT_total_bytes\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","Get_All_Command",
				&firmware->get_all_command))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"Get_All_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","Read_Command",
				&firmware->read_command))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"Read_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","Write_Command",
				&firmware->write_command))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"Write_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","Burn_Command",
				&firmware->burn_command))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"Burn_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"parameters","Burn_All_Command",
				&firmware->burn_all_command))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"Burn_All_Command\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_boolean(cfgfile,"parameters","MultiPage",
				&firmware->multi_page))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"MultiPage\" flag not found in parameters section in interrogation profile, ERROR\n"));
	if ((firmware->multi_page) && (!(firmware->capabilities & MS2)))
	{
		if(!cfg_read_string(cfgfile,"parameters","Page_Command",
					&firmware->page_command))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"Page_Command\" flag not found in parameters section in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_boolean(cfgfile,"parameters","ChunkWriteSupport",
				&firmware->chunk_support))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"ChunkWriteSupport\" flag not found in parameters section in interrogation profile, ERROR\n"));
	if (firmware->chunk_support)
	{
		if(!cfg_read_string(cfgfile,"parameters","Chunk_Write_Command",
					&firmware->chunk_write_command))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"Chunk_Write_Command\" flag not found in parameters section in interrogation profile, ERROR\n"));
	}
	if (firmware->capabilities & MS2)
	{
		if(!cfg_read_string(cfgfile,"parameters","Table_Write_Command",
					&firmware->table_write_command))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"Table_Write_Command\" flag not found in parameters section in interrogation profile, ERROR\n"));
	}
	if(!cfg_read_int(cfgfile,"parameters","TotalPages",
				&firmware->total_pages))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"TotalPages\" value not found in interrogation profile, ERROR\n"));
	cfg_read_int(cfgfile,"parameters","ReadOnlyAbove",&firmware->ro_above);
	if(!cfg_read_int(cfgfile,"parameters","TotalTables",
				&firmware->total_tables))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"TotalTables\" value not found in interrogation profile, ERROR\n"));
	cfg_read_int(cfgfile,"parameters","TotalTETables",
			&firmware->total_te_tables);
	if ((firmware->capabilities & MS2_E) || (firmware->capabilities & MS1_E))
	{
		if(!cfg_read_int(cfgfile,"parameters","TrigmonPage",&firmware->trigmon_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"TrigmonPage\" value not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,"parameters","ToothmonPage",&firmware->toothmon_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"ToothmonPage\" value not found in interrogation profile, ERROR\n"));
		if (firmware->capabilities & MS2_E_COMPMON)
		{
			if(!cfg_read_int(cfgfile,"parameters","CompositemonPage",&firmware->compositemon_page))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"CompositemonPage\" value not found in interrogation profile, ERROR\n"));
		}
	}
	if(!cfg_read_string(cfgfile,"gui","LoadTabs",
				&tmpbuf))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"LoadTabs\" list not found in interrogation profile, ERROR\n"));
	else
	{
		firmware->tab_list = g_strsplit(tmpbuf,",",0);
		g_free(tmpbuf);
	}
	if(!cfg_read_string(cfgfile,"gui","TabConfs",
				&tmpbuf))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"TabConfs\" list not found in interrogation profile, ERROR\n"));
	else
	{
		firmware->tab_confs = g_strsplit(tmpbuf,",",0);
		g_free(tmpbuf);
	}
	if(!cfg_read_string(cfgfile,"gui","RealtimeMapFile",
				&firmware->rtv_map_file))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"RealtimeMapFile\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"gui","SliderMapFile",
				&firmware->sliders_map_file))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"SliderMapFile\" variable not found in interrogation profile, ERROR\n"));
	cfg_read_string(cfgfile,"gui","RuntimeTextMapFile",
			&firmware->rtt_map_file);
	/*		MTXDBG(INTERROGATOR|CRITICAL,_("\"RuntimeTextMapFile\" variable not found in interrogation profile, ERROR\n"));*/
	cfg_read_string(cfgfile,"gui","StatusMapFile",
			&firmware->status_map_file);
	/*		MTXDBG(INTERROGATOR|CRITICAL,_("\"StatusMapFile\" variable not found in interrogation profile, ERROR\n"));*/
	if (!cfg_read_string(cfgfile,"lookuptables","tables",
				&tmpbuf))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"tables\" lookuptable name not found in interrogation profile, ERROR\n"));
	else
	{
		list = g_strsplit(tmpbuf,",",0);
		g_free(tmpbuf);
		i = 0;
		while (list[i] != NULL)
		{	
			if (!cfg_read_string(cfgfile,"lookuptables",list[i],&tmpbuf))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"%s\" key name not found in \"[lookuptables]\"\n\t section of interrogation profile, ERROR\n"),list[i]);
			else
			{
				MTXDBG(INTERROGATOR,_("\"[lookuptables]\"\n\t section loading table %s, file %s\n"),list[i],tmpbuf);
				get_table_f(list[i],tmpbuf,NULL);
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
				MTXDBG(INTERROGATOR|CRITICAL,_("\"phys_ecu_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
		if(!cfg_read_boolean(cfgfile,section,"dl_by_default",&firmware->page_params[i]->dl_by_default))
		{
			MTXDBG(INTERROGATOR|CRITICAL,_("\"dl_by_default\" flag not found in \"%s\" section in interrogation profile, assuming TRUE\n"),section);
			firmware->page_params[i]->dl_by_default = TRUE;
		}
		if(!cfg_read_int(cfgfile,section,"length",&firmware->page_params[i]->length))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"length\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
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
			firmware->table_params[i]->match_type = (MatchType)translate_string_f(tmpbuf);
			g_free(tmpbuf);
		}
		cfg_read_boolean(cfgfile,section,"is_spark",&firmware->table_params[i]->is_spark);
		cfg_read_boolean(cfgfile,section,"is_fuel",&firmware->table_params[i]->is_fuel);
		if ((firmware->table_params[i]->is_fuel) && !(firmware->capabilities & PIS))
		{
			if(!cfg_read_int(cfgfile,section,"divider_page",&firmware->table_params[i]->divider_page))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"divider_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"divider_offset",&firmware->table_params[i]->divider_offset))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"divider_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"reqfuel_page",&firmware->table_params[i]->reqfuel_page))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"reqfuel_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"reqfuel_offset",&firmware->table_params[i]->reqfuel_offset))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"reqfuel_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_string(cfgfile,section,"reqfuel_size",&tmpbuf))
				firmware->table_params[i]->reqfuel_size = MTX_U08;
			else
			{
				firmware->table_params[i]->reqfuel_size = (DataSize)translate_string_f(tmpbuf);
				g_free(tmpbuf);
			}
			if(!cfg_read_int(cfgfile,section,"stroke_page",&firmware->table_params[i]->stroke_page))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"stroke_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"stroke_offset",&firmware->table_params[i]->stroke_offset))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"stroke_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"stroke_mask",&firmware->table_params[i]->stroke_mask))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"stroke_mask\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"num_cyl_page",&firmware->table_params[i]->num_cyl_page))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"num_cyl_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"num_cyl_offset",&firmware->table_params[i]->num_cyl_offset))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"num_cyl_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"num_cyl_mask",&firmware->table_params[i]->num_cyl_mask))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"num_cyl_mask\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"num_inj_page",&firmware->table_params[i]->num_inj_page))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"num_inj_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"num_inj_offset",&firmware->table_params[i]->num_inj_offset))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"num_inj_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if(!cfg_read_int(cfgfile,section,"num_inj_mask",&firmware->table_params[i]->num_inj_mask))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"num_inj_mask\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			if (!(firmware->capabilities & MS2))
			{
				if(!cfg_read_int(cfgfile,section,"rpmk_page",&firmware->table_params[i]->rpmk_page))
					MTXDBG(INTERROGATOR|CRITICAL,_("\"rpmk_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
				if(!cfg_read_int(cfgfile,section,"rpmk_offset",&firmware->table_params[i]->rpmk_offset))
					MTXDBG(INTERROGATOR|CRITICAL,_("\"rpmk_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			}
			if (!(firmware->capabilities & MS1_DT))
			{
				if(!cfg_read_int(cfgfile,section,"alternate_page",&firmware->table_params[i]->alternate_page))
					MTXDBG(INTERROGATOR|CRITICAL,_("\"alternate_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
				if(!cfg_read_int(cfgfile,section,"alternate_offset",&firmware->table_params[i]->alternate_offset))
					MTXDBG(INTERROGATOR|CRITICAL,_("\"alternate_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			}
			if (firmware->capabilities & MS1_E)
			{
				if(!cfg_read_int(cfgfile,section,"dtmode_offset",&firmware->table_params[i]->dtmode_offset))
					MTXDBG(INTERROGATOR|CRITICAL,_("\"dtmode_offset\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
				if(!cfg_read_int(cfgfile,section,"dtmode_page",&firmware->table_params[i]->dtmode_page))
					MTXDBG(INTERROGATOR|CRITICAL,_("\"dtmode_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
				if(!cfg_read_int(cfgfile,section,"dtmode_mask",&firmware->table_params[i]->dtmode_mask))
					MTXDBG(INTERROGATOR|CRITICAL,_("\"dtmode_mask\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
			}
		}
		if(!cfg_read_int(cfgfile,section,"x_page",&firmware->table_params[i]->x_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
		if(!cfg_read_int(cfgfile,section,"y_page",&firmware->table_params[i]->y_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
		if(!cfg_read_int(cfgfile,section,"z_page",&firmware->table_params[i]->z_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"z_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
		if(!cfg_read_int(cfgfile,section,"x_base_offset",&firmware->table_params[i]->x_base))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"y_base_offset",&firmware->table_params[i]->y_base))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"z_base_offset",&firmware->table_params[i]->z_base))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"z_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"x_bincount",&firmware->table_params[i]->x_bincount))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_bincount\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"y_bincount",&firmware->table_params[i]->y_bincount))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_bincount\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_size",&tmpbuf))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->table_params[i]->x_size = (DataSize)translate_string_f(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"y_size",&tmpbuf))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->table_params[i]->y_size = (DataSize)translate_string_f(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"z_size",&tmpbuf))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"z_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->table_params[i]->z_size = (DataSize)translate_string_f(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_int(cfgfile,section,"z_raw_lower",&firmware->table_params[i]->z_raw_lower))
		{
			firmware->table_params[i]->z_raw_lower = get_extreme_from_size_f(firmware->table_params[i]->z_size,LOWER);
		}
		if(!cfg_read_int(cfgfile,section,"z_raw_upper",&firmware->table_params[i]->z_raw_upper))
		{
			firmware->table_params[i]->z_raw_upper = get_extreme_from_size_f(firmware->table_params[i]->z_size,UPPER);
		}
		if(cfg_read_boolean(cfgfile,section,"x_multi_source",&firmware->table_params[i]->x_multi_source))
		{
			/* READ multi-source stuff, but do NOT create
			 * evaluators,  we do that in the final copy
			 * over to the firmware struct
			 */
			if(!cfg_read_string(cfgfile,section,"x_multi_expr_keys",&firmware->table_params[i]->x_multi_expr_keys))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_multi_expr_keys\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_source_key",&firmware->table_params[i]->x_source_key))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_source_key\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_sources",&firmware->table_params[i]->x_sources))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_sources\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_suffixes",&firmware->table_params[i]->x_suffixes))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_suffixes\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_fromecu_mults",&firmware->table_params[i]->x_fromecu_mults))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_fromecu_mults\" variable not found in interrogation profile, table %i, ERROR\n"),i);
			if(!cfg_read_string(cfgfile,section,"x_fromecu_adds",&firmware->table_params[i]->x_fromecu_adds))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_fromecu_adds\" variable not found in interrogation profile, table %i, ERROR\n"),i);
			if(!cfg_read_string(cfgfile,section,"x_precisions",&firmware->table_params[i]->x_precisions))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_precisions\" variable not found in interrogation profile, ERROR\n"));
			cfg_read_string(cfgfile,section,"x_lookuptables",&firmware->table_params[i]->x_lookuptables);
			cfg_read_boolean(cfgfile,section,"x_ignore_algorithm",&firmware->table_params[i]->x_ignore_algorithm);
		}
		else
		{
			if(!cfg_read_string(cfgfile,section,"x_source",&firmware->table_params[i]->x_source))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_source\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"x_suffix",&firmware->table_params[i]->x_suffix))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_suffix\" variable not found in interrogation profile, ERROR\n"));
			if(cfg_read_boolean(cfgfile,section,"x_complex",&firmware->table_params[i]->x_complex))
			{
				cfg_read_string(cfgfile,section,"x_fromecu_conv_expr",&firmware->table_params[i]->x_fromecu_conv_expr);
				cfg_read_string(cfgfile,section,"x_toecu_conv_expr",&firmware->table_params[i]->x_toecu_conv_expr);
			}
			else
			{
				if(cfg_read_float(cfgfile,section,"x_fromecu_mult",&tmpf))
				{
					firmware->table_params[i]->x_fromecu_mult = g_new0(gfloat, 1);
					*(firmware->table_params[i]->x_fromecu_mult) = tmpf;
				}
				if(cfg_read_float(cfgfile,section,"x_fromecu_add",&tmpf))
				{
					firmware->table_params[i]->x_fromecu_add = g_new0(gfloat, 1);
					*(firmware->table_params[i]->x_fromecu_add) = tmpf;
				}
			}
			if(!cfg_read_int(cfgfile,section,"x_precision",&firmware->table_params[i]->x_precision))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"x_precision\" variable not found in interrogation profile for table %i, ERROR\n"),i);
		}
		if(cfg_read_boolean(cfgfile,section,"y_multi_source",&firmware->table_params[i]->y_multi_source))
		{
			/* READ multi-source stuff, but do NOT create
			 * evaluators,  we do that in the final copy
			 * over to the firmware struct
			 */
			if(!cfg_read_string(cfgfile,section,"y_multi_expr_keys",&firmware->table_params[i]->y_multi_expr_keys))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_multi_expr_keys\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_source_key",&firmware->table_params[i]->y_source_key))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_source_key\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_sources",&firmware->table_params[i]->y_sources))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_sources\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_suffixes",&firmware->table_params[i]->y_suffixes))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_suffixes\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_fromecu_mults",&firmware->table_params[i]->y_fromecu_mults))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_fromecu_mults\" variable not found in interrogation profile, table %i, ERROR\n"),i);
			if(!cfg_read_string(cfgfile,section,"y_fromecu_adds",&firmware->table_params[i]->y_fromecu_adds))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_fromecu_adds\" variable not found in interrogation profile, table %i, ERROR\n"),i);
			if(!cfg_read_string(cfgfile,section,"y_precisions",&firmware->table_params[i]->y_precisions))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_precisions\" variable not found in interrogation profile, ERROR\n"));
			cfg_read_string(cfgfile,section,"y_lookuptables",&firmware->table_params[i]->y_lookuptables);
			cfg_read_boolean(cfgfile,section,"y_ignore_algorithm",&firmware->table_params[i]->y_ignore_algorithm);

		}
		else
		{
			if(!cfg_read_string(cfgfile,section,"y_source",&firmware->table_params[i]->y_source))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_source\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"y_suffix",&firmware->table_params[i]->y_suffix))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_suffix\" variable not found in interrogation profile, ERROR\n"));
			if(cfg_read_boolean(cfgfile,section,"y_complex",&firmware->table_params[i]->y_complex))
			{
				cfg_read_string(cfgfile,section,"y_fromecu_conv_expr",&firmware->table_params[i]->y_fromecu_conv_expr);
				cfg_read_string(cfgfile,section,"y_toecu_conv_expr",&firmware->table_params[i]->y_toecu_conv_expr);
			}
			else
			{
				if(cfg_read_float(cfgfile,section,"y_fromecu_mult",&tmpf))
				{
					firmware->table_params[i]->y_fromecu_mult = g_new0(gfloat, 1);
					*(firmware->table_params[i]->y_fromecu_mult) = tmpf;
				}
				if(cfg_read_float(cfgfile,section,"y_fromecu_add",&tmpf))
				{
					firmware->table_params[i]->y_fromecu_add = g_new0(gfloat, 1);
					*(firmware->table_params[i]->y_fromecu_add) = tmpf;
				}
			}
			if(!cfg_read_int(cfgfile,section,"y_precision",&firmware->table_params[i]->y_precision))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"y_precision\" variable not found in interrogation profile for table %i, ERROR\n"),i);
		}
		if(cfg_read_boolean(cfgfile,section,"z_multi_source",&firmware->table_params[i]->z_multi_source))
		{
			/* READ multi-source stuff, but do NOT create
			 * evaluators,  we do that in the final copy
			 * over to the firmware struct
			 */
			if(!cfg_read_string(cfgfile,section,"z_multi_expr_keys",&firmware->table_params[i]->z_multi_expr_keys))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_multi_expr_keys\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_source_key",&firmware->table_params[i]->z_source_key))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_source_key\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_sources",&firmware->table_params[i]->z_sources))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_sources\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_suffixes",&firmware->table_params[i]->z_suffixes))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_suffixes\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_fromecu_mults",&firmware->table_params[i]->z_fromecu_mults))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_fromecu_mults\" variable not found in interrogation profile, table %i, ERROR\n"),i);
			if(!cfg_read_string(cfgfile,section,"z_fromecu_adds",&firmware->table_params[i]->z_fromecu_adds))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_fromecu_adds\" variable not found in interrogation profile, table %i, ERROR\n"),i);
			if(!cfg_read_string(cfgfile,section,"z_precisions",&firmware->table_params[i]->z_precisions))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_precisions\" variable not found in interrogation profile, ERROR\n"));
			cfg_read_string(cfgfile,section,"z_lookuptables",&firmware->table_params[i]->z_lookuptables);
			cfg_read_boolean(cfgfile,section,"z_ignore_algorithm",&firmware->table_params[i]->z_ignore_algorithm);

		}
		else
		{
			if(!cfg_read_string(cfgfile,section,"z_source",&firmware->table_params[i]->z_source))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_source\" variable not found in interrogation profile, ERROR\n"));
			if(!cfg_read_string(cfgfile,section,"z_suffix",&firmware->table_params[i]->z_suffix))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_suffix\" variable not found in interrogation profile, ERROR\n"));
			if(cfg_read_boolean(cfgfile,section,"z_complex",&firmware->table_params[i]->z_complex))
			{
				cfg_read_string(cfgfile,section,"z_fromecu_conv_expr",&firmware->table_params[i]->z_fromecu_conv_expr);
				cfg_read_string(cfgfile,section,"z_toecu_conv_expr",&firmware->table_params[i]->z_toecu_conv_expr);
			}
			else
			{
				if(cfg_read_float(cfgfile,section,"z_fromecu_mult",&tmpf))
				{
					firmware->table_params[i]->z_fromecu_mult = g_new0(gfloat, 1);
					*(firmware->table_params[i]->z_fromecu_mult) = tmpf;
				}
				if(cfg_read_float(cfgfile,section,"z_fromecu_add",&tmpf))
				{
					firmware->table_params[i]->z_fromecu_add = g_new0(gfloat, 1);
					*(firmware->table_params[i]->z_fromecu_add) = tmpf;
				}
			}
			if(!cfg_read_int(cfgfile,section,"z_precision",&firmware->table_params[i]->z_precision))
				MTXDBG(INTERROGATOR|CRITICAL,_("\"z_precision\" variable not found in interrogation profile for table %i, ERROR\n"),i);
			if(cfg_read_string(cfgfile,section,"z_depend_on",&firmware->table_params[i]->z_depend_on))
			{
				firmware->table_params[i]->z_object = (GObject *)g_object_new(GTK_TYPE_INVISIBLE,NULL);
				g_object_ref_sink(GTK_OBJECT(firmware->table_params[i]->z_object));
				load_dependencies_obj(firmware->table_params[i]->z_object,cfgfile,section,"z_depend_on");
				if(!cfg_read_string(cfgfile,section,"z_alt_lookuptable",&tmpbuf))
					MTXDBG(INTERROGATOR|CRITICAL,_("\"z_alt_lookuptable\" variable not found in interrogation profile, NOT NECESSARILY AN ERROR\n"));
				else
				{
					OBJ_SET_FULL(firmware->table_params[i]->z_object,"alt_lookuptable",g_strdup(tmpbuf),g_free);
					g_free(tmpbuf);
				}
				if(!cfg_read_string(cfgfile,section,"z_lookuptable",&tmpbuf))
					MTXDBG(INTERROGATOR|CRITICAL,_("\"z_lookuptable\" variable not found in interrogation profile, NOT NECESSARILY AN ERROR\n"));
				else
				{
					OBJ_SET_FULL(firmware->table_params[i]->z_object,"lookuptable",g_strdup(tmpbuf),g_free);
					g_free(tmpbuf);
				}
			}
		}
		cfg_read_boolean(cfgfile,section,"x_use_color",&firmware->table_params[i]->x_use_color);
		cfg_read_boolean(cfgfile,section,"y_use_color",&firmware->table_params[i]->y_use_color);
		cfg_read_boolean(cfgfile,section,"z_use_color",&firmware->table_params[i]->z_use_color);
		if(!cfg_read_string(cfgfile,section,"table_name",&firmware->table_params[i]->table_name))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"table_name\" variable not found in interrogation profile, ERROR\n"));
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
			firmware->te_params[i]->match_type = (MatchType)translate_string_f(tmpbuf);
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
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_axis_label\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
		if(!cfg_read_string(cfgfile,section,"y_axis_label",&firmware->te_params[i]->y_axis_label))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_axis_label\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
		if(!cfg_read_int(cfgfile,section,"x_page",&firmware->te_params[i]->x_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
		if(!cfg_read_int(cfgfile,section,"y_page",&firmware->te_params[i]->y_page))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_page\" flag not found in \"%s\" section in interrogation profile, ERROR\n"),section);
		if(!cfg_read_int(cfgfile,section,"x_base_offset",&firmware->te_params[i]->x_base))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"y_base_offset",&firmware->te_params[i]->y_base))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_base_offset\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_int(cfgfile,section,"bincount",&firmware->te_params[i]->bincount))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_bincount\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_size",&tmpbuf))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->te_params[i]->x_size = (DataSize)translate_string_f(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"y_size",&tmpbuf))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_size\" enumeration not found in interrogation profile, ERROR\n"));
		else
		{
			firmware->te_params[i]->y_size = (DataSize)translate_string_f(tmpbuf);
			g_free(tmpbuf);
		}
		if(!cfg_read_string(cfgfile,section,"x_source",&firmware->te_params[i]->x_source))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_source\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_units",&firmware->te_params[i]->x_units))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_units\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"x_name",&firmware->te_params[i]->x_name))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_name\" variable not found in interrogation profile, ERROR\n"));
		if(cfg_read_float(cfgfile,section,"x_fromecu_mult",&tmpf))
		{
			firmware->te_params[i]->x_fromecu_mult = g_new0(gfloat,1 );
			*(firmware->te_params[i]->x_fromecu_mult) = tmpf;
		}
		if(cfg_read_float(cfgfile,section,"x_fromecu_add",&tmpf))
		{
			firmware->te_params[i]->x_fromecu_add = g_new0(gfloat,1 );
			*(firmware->te_params[i]->x_fromecu_add) = tmpf;
		}
		if(!cfg_read_int(cfgfile,section,"x_raw_lower",&firmware->te_params[i]->x_raw_lower))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_raw_lower\" variable not found in interrogation profile for table %i, ERROR\n"),i);
		if(!cfg_read_int(cfgfile,section,"x_raw_upper",&firmware->te_params[i]->x_raw_upper))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_raw_upper\" variable not found in interrogation profile for table %i, ERROR\n"),i);
		if(!cfg_read_int(cfgfile,section,"x_precision",&firmware->te_params[i]->x_precision))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"x_precision\" variable not found in interrogation profile for table %i, ERROR\n"),i);
		if(!cfg_read_string(cfgfile,section,"y_units",&firmware->te_params[i]->y_units))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_units\" variable not found in interrogation profile, ERROR\n"));
		if(!cfg_read_string(cfgfile,section,"y_name",&firmware->te_params[i]->y_name))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_name\" variable not found in interrogation profile, ERROR\n"));
		if(cfg_read_float(cfgfile,section,"y_fromecu_mult",&tmpf))
		{
			firmware->te_params[i]->y_fromecu_mult = g_new0(gfloat,1 );
			*(firmware->te_params[i]->y_fromecu_mult) = tmpf;
		}
		if(cfg_read_float(cfgfile,section,"y_fromecu_add",&tmpf))
		{
			firmware->te_params[i]->y_fromecu_add = g_new0(gfloat,1 );
			*(firmware->te_params[i]->y_fromecu_add) = tmpf;
		}
		if(!cfg_read_int(cfgfile,section,"y_raw_lower",&firmware->te_params[i]->y_raw_lower))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_raw_lower\" variable not found in interrogation profile for table %i, ERROR\n"),i);
		if(!cfg_read_int(cfgfile,section,"y_raw_upper",&firmware->te_params[i]->y_raw_upper))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_raw_upper\" variable not found in interrogation profile for table %i, ERROR\n"),i);
		if(!cfg_read_int(cfgfile,section,"y_precision",&firmware->te_params[i]->y_precision))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"y_precision\" variable not found in interrogation profile for table %i, ERROR\n"),i);
		if(!cfg_read_string(cfgfile,section,"title",&firmware->te_params[i]->title))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"title\" variable not found in interrogation profile, ERROR\n"));
		g_free(section);
	}

	cfg_free(cfgfile);

	/* Allocate RAM for the Req_Fuel_Params structures.*/
	firmware->rf_params = g_new0(Req_Fuel_Params *,firmware->total_tables);

	/* Allocate RAM for the Table_Params structures and copy data in..*/
	for (i=0;i<firmware->total_tables;i++)
	{
		firmware->rf_params[i] = g_new0(Req_Fuel_Params ,1);
		/* Check for multi source table handling */
		if (firmware->table_params[i]->x_multi_source)
		{
			firmware->table_params[i]->x_multi_hash = g_hash_table_new_full(g_str_hash,g_str_equal,NULL,NULL);
			expr_keys = g_strsplit(firmware->table_params[i]->x_multi_expr_keys,",",-1);
			sources = g_strsplit(firmware->table_params[i]->x_sources,",",-1);
			suffixes = g_strsplit(firmware->table_params[i]->x_suffixes,",",-1);
			fromecu_mults = g_strsplit(firmware->table_params[i]->x_fromecu_mults,",",-1);
			fromecu_adds = g_strsplit(firmware->table_params[i]->x_fromecu_adds,",",-1);
			precisions = g_strsplit(firmware->table_params[i]->x_precisions,",",-1);
			if (firmware->table_params[i]->x_lookuptables)
				tables = g_strsplit(firmware->table_params[i]->x_lookuptables,",",-1);
			else
				tables = NULL;
			len1 = g_strv_length(expr_keys);
			len2 = g_strv_length(sources);
			len3 = g_strv_length(suffixes);
			len4 = g_strv_length(fromecu_mults);
			len5 = g_strv_length(fromecu_adds);
			len6 = g_strv_length(precisions);
			if ((len1 != len2) || (len1 != len3) || (len1 != len4) || (len1 != len5) || (len1 != len6))
				printf(_("X multi_sources length mismatch!\n"));
			for (j=0;j<len1;j++)
			{
				multi = g_new0(MultiSource,1);
				multi->table_num = i;
				multi->source = g_strdup(sources[j]);
				multi->multiplier = g_new0(gfloat, 1);
				multi->adder = g_new0(gfloat, 1);
				*multi->multiplier = (gfloat)g_strtod(fromecu_mults[j],NULL);
				*multi->adder = (gfloat)g_strtod(fromecu_adds[j],NULL);
				multi->suffix = g_strdup(suffixes[j]);
				multi->precision = (gint)strtol(precisions[j],NULL,10);
				if (tables && tables[j])
					multi->lookuptable = g_strdup(tables[j]);
				else
					multi->lookuptable = NULL;
				g_hash_table_insert(firmware->table_params[i]->x_multi_hash,g_strdup(expr_keys[j]),(gpointer)multi);
			}
			g_strfreev(expr_keys);
			g_strfreev(sources);
			g_strfreev(suffixes);
			g_strfreev(fromecu_mults);
			g_strfreev(fromecu_adds);
			g_strfreev(precisions);
			if (tables)
				g_strfreev(tables);
		}
		else
		{
			if (firmware->table_params[i]->x_complex)
			{
				firmware->table_params[i]->x_ul_eval = evaluator_create_f(firmware->table_params[i]->x_fromecu_conv_expr);
				firmware->table_params[i]->x_dl_eval = evaluator_create_f(firmware->table_params[i]->x_toecu_conv_expr);
			}
		}
		/* Check for multi source table handling */
		if (firmware->table_params[i]->y_multi_source)
		{
			firmware->table_params[i]->y_multi_hash = g_hash_table_new_full(g_str_hash,g_str_equal,NULL,NULL);
			expr_keys = g_strsplit(firmware->table_params[i]->y_multi_expr_keys,",",-1);
			sources = g_strsplit(firmware->table_params[i]->y_sources,",",-1);
			suffixes = g_strsplit(firmware->table_params[i]->y_suffixes,",",-1);
			fromecu_mults = g_strsplit(firmware->table_params[i]->y_fromecu_mults,",",-1);
			fromecu_adds = g_strsplit(firmware->table_params[i]->y_fromecu_adds,",",-1);
			precisions = g_strsplit(firmware->table_params[i]->y_precisions,",",-1);
			if (firmware->table_params[i]->y_lookuptables)
				tables = g_strsplit(firmware->table_params[i]->y_lookuptables,",",-1);
			else
				tables = NULL;
			len1 = g_strv_length(expr_keys);
			len2 = g_strv_length(sources);
			len3 = g_strv_length(suffixes);
			len4 = g_strv_length(fromecu_mults);
			len5 = g_strv_length(fromecu_adds);
			len6 = g_strv_length(precisions);
			if ((len1 != len2) || (len1 != len3) || (len1 != len4) || (len1 != len5) || (len1 != len6))
				printf(_("Y multi_sources length mismatch!\n"));
			for (j=0;j<len1;j++)
			{
				multi = g_new0(MultiSource,1);
				multi->table_num = i;
				multi->source = g_strdup(sources[j]);
				multi->multiplier = g_new0(gfloat, 1);
				multi->adder = g_new0(gfloat, 1);
				*multi->multiplier = (gfloat)g_strtod(fromecu_mults[j],NULL);
				*multi->adder = (gfloat)g_strtod(fromecu_adds[j],NULL);
				multi->suffix = g_strdup(suffixes[j]);
				multi->precision = (gint)strtol(precisions[j],NULL,10);
				if (tables && tables[j])
					multi->lookuptable = g_strdup(tables[j]);
				else
					multi->lookuptable = NULL;
				g_hash_table_insert(firmware->table_params[i]->y_multi_hash,g_strdup(expr_keys[j]),(gpointer)multi);
			}
			g_strfreev(expr_keys);
			g_strfreev(sources);
			g_strfreev(suffixes);
			g_strfreev(fromecu_mults);
			g_strfreev(fromecu_adds);
			g_strfreev(precisions);
			if (tables)
				g_strfreev(tables);
		}
		else
		{
			if (firmware->table_params[i]->y_complex)
			{
				firmware->table_params[i]->y_ul_eval = evaluator_create_f(firmware->table_params[i]->y_fromecu_conv_expr);
				firmware->table_params[i]->y_dl_eval = evaluator_create_f(firmware->table_params[i]->y_toecu_conv_expr);
			}
		}

		/* Check for multi source table handling */
		if (firmware->table_params[i]->z_multi_source)
		{
			firmware->table_params[i]->z_multi_hash = g_hash_table_new_full(g_str_hash,g_str_equal,NULL,NULL);
			expr_keys = g_strsplit(firmware->table_params[i]->z_multi_expr_keys,",",-1);
			sources = g_strsplit(firmware->table_params[i]->z_sources,",",-1);
			suffixes = g_strsplit(firmware->table_params[i]->z_suffixes,",",-1);
			fromecu_mults = g_strsplit(firmware->table_params[i]->z_fromecu_mults,",",-1);
			fromecu_adds = g_strsplit(firmware->table_params[i]->z_fromecu_adds,",",-1);
			precisions = g_strsplit(firmware->table_params[i]->z_precisions,",",-1);
			if (firmware->table_params[i]->z_lookuptables)
				tables = g_strsplit(firmware->table_params[i]->z_lookuptables,",",-1);
			else
				tables = NULL;
			len1 = g_strv_length(expr_keys);
			len2 = g_strv_length(sources);
			len3 = g_strv_length(suffixes);
			len4 = g_strv_length(fromecu_mults);
			len5 = g_strv_length(fromecu_adds);
			len6 = g_strv_length(precisions);
			if ((len1 != len2) || (len1 != len3) || (len1 != len4) || (len1 != len5) || (len1 != len6))
				printf(_("Z multi_sources length mismatch!\n"));
			for (j=0;j<len1;j++)
			{
				multi = g_new0(MultiSource,1);
				multi->table_num = i;
				multi->source = g_strdup(sources[j]);
				multi->multiplier = g_new0(gfloat, 1);
				multi->adder = g_new0(gfloat, 1);
				*multi->multiplier = (gfloat)g_strtod(fromecu_mults[j],NULL);
				*multi->adder = (gfloat)g_strtod(fromecu_adds[j],NULL);
				multi->suffix = g_strdup(suffixes[j]);
				multi->precision = (gint)strtol(precisions[j],NULL,10);
				if (tables && tables[j])
					multi->lookuptable = g_strdup(tables[j]);
				else
					multi->lookuptable = NULL;
				g_hash_table_insert(firmware->table_params[i]->z_multi_hash,g_strdup(expr_keys[j]),(gpointer)multi);
			}
			g_strfreev(expr_keys);
			g_strfreev(sources);
			g_strfreev(suffixes);
			g_strfreev(fromecu_mults);
			g_strfreev(fromecu_adds);
			g_strfreev(precisions);
			if (tables)
				g_strfreev(tables);
		}
		else
		{
			if (firmware->table_params[i]->z_complex)
			{
				firmware->table_params[i]->z_ul_eval = evaluator_create_f(firmware->table_params[i]->z_fromecu_conv_expr);
				firmware->table_params[i]->z_dl_eval = evaluator_create_f(firmware->table_params[i]->z_toecu_conv_expr);
			}
		}
	}
	mem_alloc_f();

	/* Display firmware version in the window... */

	MTXDBG(INTERROGATOR,_("Detected Firmware: %s\n"),firmware->name);
	update_logbar_f("interr_view","warning",g_strdup_printf(_("Detected Firmware: %s\n"),firmware->name),FALSE,FALSE,TRUE);
	update_logbar_f("interr_view","info",g_strdup_printf(_("Loading Settings from: \"%s\"\n"),firmware->profile_filename),FALSE,FALSE,TRUE);

	EXIT();
	return TRUE;

}


/*!
 \brief validate_and_load_tests() loads the list of tests from the system
 checks them for validity, populates an array and returns it
 \param tests_hash is a double pointer to a hashtable to also populate
 \returns a pointer to a GArray of tests
 */
G_MODULE_EXPORT GArray * validate_and_load_tests(GHashTable **tests_hash)
{
	ConfigFile *cfgfile;
	GArray * tests = NULL;
	Detection_Test *test = NULL;
	gchar * filename = NULL;
	gchar *section = NULL;
	gchar * tmpbuf = NULL;
	gint total_tests = 0;
	gint len = 0;
	unsigned long crc32 = 0;
	gint result = 0;
	gint major = 0;
	gint minor = 0;
	gint tmpi = 0;
	gint i = 0;
	gint j = 0;
	gchar *pathstub = NULL;

	ENTER();
	pathstub = g_build_filename(INTERROGATOR_DATA_DIR,"Profiles",DATA_GET(global_data,"ecu_family"),"tests.cfg",NULL);
	filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,NULL);
	g_free(pathstub);
	if (!filename)
	{
		update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile tests file %s not found!\n"),filename),FALSE,FALSE,TRUE);
		EXIT();
		return NULL;
	}

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
	{
		update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile tests file %s unable to be opened!\n"),filename),FALSE,FALSE,TRUE);
		EXIT();
		return NULL;
	}
	get_file_api_f(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile tests API mismatch (%i.%i != %i.%i):\n\tFile %s.\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE,TRUE);
		EXIT();
		return NULL;
	}

	*tests_hash = g_hash_table_new(g_str_hash,g_str_equal);

	MTXDBG(INTERROGATOR,_("File %s, opened successfully\n"),filename);
	tests = g_array_new(FALSE,TRUE,sizeof(Detection_Test *));

	cfg_read_int(cfgfile,"interrogation_tests","total_tests",&total_tests);
	for (i=0;i<total_tests;i++)
	{
		test = g_new0(Detection_Test, 1);
		cfg_read_boolean(cfgfile,"interrogation_tests","ms3_crc32",&test->ms3_crc32);
		section = g_strdup_printf("test_%.2i",i);
		if (!cfg_read_string(cfgfile,section,"test_name",&test->test_name))
		{
			MTXDBG(INTERROGATOR|CRITICAL,_("test_name for %s is NULL\n"),section);
			g_free(section);
			break;
		}
		if (!cfg_read_string(cfgfile,section,"test_result_type",&tmpbuf))
		{
			MTXDBG(INTERROGATOR|CRITICAL,_("test_result_type for %s is NULL\n"),section);
			g_free(section);
			break;
		}
		else
		{
			test->result_type=translate_string_f(tmpbuf);
			g_free(tmpbuf);
		}
		if (!cfg_read_string(cfgfile,section,"actual_test",&test->actual_test))
		{
			MTXDBG(INTERROGATOR|CRITICAL,_("actual_test for %s is NULL\n"),section);
			g_free(section);
			break;
		}
		else
			len = strlen(test->actual_test);

		if (test->ms3_crc32)
		{
			test->send_buf = g_new0(guint8, len + 6); /*length + CRC32 */
			test->send_buf[0] = (len >> 8 ) & 0xff;
			test->send_buf[1] = len & 0xff;
			g_memmove(&test->send_buf[2],test->actual_test,len);
			crc32 = crc32_computebuf(0,&test->send_buf[2],len);
			test->send_buf[2 + len + 0] = (crc32 >> 24) & 0xff;
			test->send_buf[2 + len + 1] = (crc32 >> 16) & 0xff;
			test->send_buf[2 + len + 2] = (crc32 >> 8) & 0xff;
			test->send_buf[2 + len + 3] = crc32 & 0xff;
			test->send_len = len + 6;
		}
		else  /* NON CRC edition */
		{
			test->send_buf = (guint8 *)g_memdup(test->actual_test,len);
			test->send_len = len;
		}

		cfg_read_string(cfgfile,section,"test_desc",
				&test->test_desc);

		g_free(section);
		g_array_append_val(tests,test);
		g_hash_table_insert(*tests_hash,test->test_name,test);
	}
	cfg_free(cfgfile);
	g_free(filename);
	EXIT();
	return tests;
}


/*!
 \brief translate_capabilities() converts a stringlist into a mask of 
 enumerations and returns it
 \param string is the listing of capabilities in textual format
 \returns an integer mask of the capabilites
 */
G_MODULE_EXPORT gint translate_capabilities(const gchar *string)
{
	gchar **vector = NULL;
	gint i = 0;
	gint value = 0;
	gint tmpi = 0;

	ENTER();

	if (!string)
	{
		MTXDBG(INTERROGATOR|CRITICAL,_("String fed is NULL\n"));
		EXIT();
		return -1;
	}

	vector = g_strsplit(string,",",0);
	MTXDBG(INTERROGATOR,_("String fed is %s\n"),string);
	while (vector[i] != NULL)
	{
		MTXDBG(INTERROGATOR,_("Trying to translate %s\n"),vector[i]);
		tmpi = translate_string_f(vector[i]);
		MTXDBG(INTERROGATOR,_("Translated value of %s is %i\n"),vector[i],value);
		value += tmpi;
		i++;
	}

	g_strfreev(vector);
	EXIT();
	return value;	
}


/*!
 \brief check_for_match() compares the results of the interrogation with the
 ECU to the canidates in turn. When a match occurs TRUE is returned
 otherwise it returns FALSE
 \param tests_hash is a pointer to the hashtable of tests
 \param filename isa pointer to the  file to compare against
 \returns TRUE on match, FALSE on failure
 */
G_MODULE_EXPORT gboolean check_for_match(GHashTable *tests_hash, gchar *filename)
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
	MatchClass mclass;

	ENTER();
	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
	{
		EXIT();
		return FALSE;
	}

	get_file_api_f(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE,TRUE);
		cfg_free(cfgfile);
		EXIT();
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
		test = (Detection_Test *)g_hash_table_lookup(tests_hash,match_on[i]);
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
			MTXDBG(INTERROGATOR,_("MISMATCH,\"%s\" is NOT a match...\n\n"),filename);
			cfg_free(cfgfile);
			g_strfreev(match_on);
			EXIT();
			return FALSE;
		}
		vector = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
		/* Possible choices are "Count", "submatch" and "fullmatch", so
		 * stringparse to get them into a consistent form
		 */
		if (g_strv_length(vector) != 2)
			printf(_("ERROR interrogation check_for match vector does NOT have two args it has %i\n"),g_strv_length(vector));
		mclass = (MatchClass)translate_string_f(vector[0]);
		/*printf("potential data is %s\n",vector[1]);*/
		switch (mclass)
		{
			case COUNT:
				if (test->recv_len == atoi(vector[1]))
					pass = TRUE;
				break;
			case NUMMATCH:
				if (test->result_str)
				{
					if ((gint)(test->result_str[0]) == atoi(vector[1]))
						pass = TRUE;
				}
				else
					pass = FALSE;
				break;
			case SUBMATCH:
				if (test->result_str)
				{
					if (strstr(test->result_str,vector[1]) != NULL)
						pass = TRUE;
				}
				else
					pass = FALSE;
				break;
			case FULLMATCH:
				if (test->result_str)
				{
					if (g_ascii_strcasecmp(test->result_str,vector[1]) == 0)
						pass = TRUE;
				}
				else
					pass = FALSE;
				break;
			case REGEX:
				if (test->result_str)
				{
					if (g_regex_match_simple(vector[1],
								test->result_str,
								(GRegexCompileFlags)0,
								(GRegexMatchFlags)0))
						pass = TRUE;
				}
				else
					pass = FALSE;
				break;
			default:
				pass=FALSE;
		}
		g_strfreev(vector);
		if (pass == TRUE)
			continue;
		else
		{
			MTXDBG(INTERROGATOR,_("MISMATCH,\"%s\" is NOT a match...\n\n"),filename);
			g_strfreev(match_on);
			cfg_free(cfgfile);
			EXIT();
			return FALSE;
		}

	}
	g_strfreev(match_on);
	MTXDBG(INTERROGATOR,_("\"%s\" is a match for all conditions ...\n"),filename);
	cfg_free(cfgfile);
	EXIT();
	return TRUE;
}


/*! 
  \brief destroys Array holding Detection_Test structures
  \param tests is the pointer to the array to destroy
 */
G_MODULE_EXPORT void free_tests_array(GArray *tests)
{
	guint i = 0;
	Detection_Test *test = NULL;

	ENTER();
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
		if (test->send_buf)
			g_free(test->send_buf);
		g_free(test);
		test = NULL;
	}

	g_array_free(tests,TRUE);
	EXIT();
	return;
}


/*! 
  \brief interrogate_error, dumps an error out to the error handling
  based on passed string and numeric pararms
  \param text is the text to display
  \param num is the number to display
 */
G_MODULE_EXPORT void interrogate_error(const gchar *text, gint num)
{
	ENTER();
	EXIT();
	return;
}


/*!
  \brief updates the interrogation gui with the text revision, signature
  and ecu numerical revision
 */
G_MODULE_EXPORT void update_interrogation_gui_pf(void)
{
	GtkWidget *widget = NULL;
	GtkAdjustment *adj = NULL;
	Serial_Params *serial_params = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(serial_params);
	g_return_if_fail(firmware);
	widget = lookup_widget_f("read_wait_spin");
	if (GTK_IS_SPIN_BUTTON(widget))
	{
		gfloat min = 0.0;
		gfloat val = 0.0;
		adj = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(widget));
		val = gtk_adjustment_get_value(adj);
		if (firmware->capabilities & MS1)
		{
			min = 1000.0*(1.0/(960.0/(firmware->rtvars_size+2.0)));
			min *= 1.2; /* Add 20% buffer */
		}
		else if (firmware->capabilities & PIS)
		{
			min = 1000.0*(1.0/(819.2/(firmware->rtvars_size+2.0)));
			min *= 1.1; /* Add 10% buffer */
		}
		else
		{
			min = 1000.0*(1.0/(11520.0/(firmware->rtvars_size+5.0)));
			min *= 1.2; /* Add 20% buffer */
			if (min < 30)
				min = 30;
		}

		if (val <= min)
		{
			/* Safety net */
			val = min + 10;
			serial_params->read_wait = (gint)val;
		}
		gtk_adjustment_set_lower(adj,min);
		gtk_adjustment_set_value(adj,val);
		gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(widget),adj);
	}
	if (firmware->TextVerVia)
		io_cmd_f(firmware->TextVerVia,NULL);
	if (firmware->NumVerVia)
		io_cmd_f(firmware->NumVerVia,NULL);
	if (firmware->SignatureVia)
		io_cmd_f(firmware->SignatureVia,NULL);
	EXIT();
	return;
}


/*!
 \brief initialize_page_params() creates and initializes the page_params
 datastructure to sane defaults and returns it
 \return a pointer to an initialized Page_Params structure
 \see Page_Params
 */
G_MODULE_EXPORT Page_Params * initialize_page_params(void)
{
	Page_Params *page_params = NULL;
	ENTER();
	page_params = (Page_Params *)g_malloc0(sizeof(Page_Params));
	page_params->length = 0;
	page_params->spconfig_offset = -1;
	EXIT();
	return page_params;
}


/*!
 \brief initialize_table_params() creates and initializes the Table_Params
 datastructure to sane defaults and returns it
 \return a pointer to an initialized Table_Params structure
 \see Table_Params
 */
G_MODULE_EXPORT Table_Params * initialize_table_params(void)
{
	Table_Params *table_params = NULL;
	ENTER();
	table_params = (Table_Params *)g_malloc0(sizeof(Table_Params));
	table_params->is_fuel = FALSE;
	table_params->alternate_offset = -1;
	table_params->divider_offset = -1;
	table_params->rpmk_offset = -1;
	table_params->reqfuel_offset = -1;
	table_params->x_page = -1;
	table_params->y_page = -1;
	table_params->z_page = -1;
	table_params->x_base = -1;
	table_params->y_base = -1;
	table_params->z_base = -1;
	table_params->x_bincount = -1;
	table_params->y_bincount = -1;
	table_params->x_precision = 0;
	table_params->y_precision = 0;
	table_params->z_precision = 0;
	table_params->bind_to_list = NULL;
	table_params->x_suffix = NULL;
	table_params->y_suffix = NULL;
	table_params->z_suffix = NULL;
	table_params->x_fromecu_mults = NULL;
	table_params->y_fromecu_mults = NULL;
	table_params->z_fromecu_mults = NULL;
	table_params->x_fromecu_adds = NULL;
	table_params->y_fromecu_adds = NULL;
	table_params->z_fromecu_adds = NULL;
	table_params->x_source_key = NULL;
	table_params->y_source_key = NULL;
	table_params->z_source_key = NULL;
	table_params->table_name = NULL;
	table_params->x_ul_eval = NULL;
	table_params->y_ul_eval = NULL;
	table_params->z_ul_eval = NULL;
	table_params->x_dl_eval = NULL;
	table_params->y_dl_eval = NULL;
	table_params->z_dl_eval = NULL;
	table_params->x_multi_hash = NULL;
	table_params->y_multi_hash = NULL;
	table_params->z_multi_hash = NULL;
	table_params->x_multi_source = FALSE;
	table_params->y_multi_source = FALSE;
	table_params->z_multi_source = FALSE;

	EXIT();
	return table_params;
}


/*!
 \brief initialize_te_params() creates and initializes the TE_Params
 datastructure to sane defaults and returns it
 \return a pointer to an initialized TE_Params structure
 \see TE_Params
 */
G_MODULE_EXPORT TE_Params * initialize_te_params(void)
{
	TE_Params *te_params = NULL;
	ENTER();
	te_params = (TE_Params *)g_malloc0(sizeof(TE_Params));
	te_params->x_lock = FALSE;
	te_params->y_lock = FALSE;
	te_params->x_use_color = FALSE;
	te_params->y_use_color = FALSE;
	te_params->x_temp_dep = FALSE;
	te_params->y_temp_dep = FALSE;
	te_params->x_page = -1;
	te_params->y_page = -1;
	te_params->x_base = -1;
	te_params->y_base = -1;
	te_params->reversed = FALSE;
	te_params->bincount = -1;
	te_params->x_precision = 0;
	te_params->y_precision = 0;
	te_params->x_axis_label = NULL;
	te_params->y_axis_label = NULL;
	te_params->x_name = NULL;
	te_params->y_name = NULL;
	te_params->x_units = NULL;
	te_params->y_units = NULL;
	te_params->x_fromecu_mult = NULL;
	te_params->x_fromecu_add = NULL;
	te_params->y_fromecu_mult = NULL;
	te_params->y_fromecu_add = NULL;
	te_params->gauge_temp_dep = FALSE;
	te_params->title = NULL;

	EXIT();
	return te_params;
}
