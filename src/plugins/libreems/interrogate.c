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
  \file src/plugins/libreems/interrogate.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS specific interrogation routine (INCOMPLETE/IN FLUX)
  \author David Andruczyk
  */

#include <api-versions.h>
#include <datamgmt.h>
#include <getfiles.h>
#include <interrogate.h>
#include <libreems_plugin.h>
#include <libreems_helpers.h>
#include <libgen.h>
#include <serialio.h>
#include <stdio.h>
#include <string.h>

extern gconstpointer *global_data;


/*!
 \brief interrogate_ecu() interrogates the target ECU to determine what
 firmware it is running.  It does this by reading a list of tests, sending
 those tests in turn, reading the responses and them comparing the group of
 responses against a list of interrogation profiles until it finds a match.
 NOTE: This function is NOT yet complete for LibreEMS
 */
G_MODULE_EXPORT gboolean interrogate_ecu(void)
{
	GAsyncQueue *queue = NULL;
	GHashTable *tests_hash = NULL;
	GArray *tests = NULL;
	Detection_Test *test = NULL;
	guint i = 0;
	gboolean interrogated = FALSE;
	GList *locations = NULL;
	static GMutex mutex;

	ENTER();
	/* prevent multiple runs of interrogator simultaneously */
	g_mutex_lock(&mutex);
	MTXDBG(INTERROGATOR,_("ENTERED\n\n"));

	/* ECU has already been detected via comms test
	   Now we need to figure out its variant and adapt to it
	 */
	/* Send stream disable command */
	/*stop_streaming(); */

	/* Load tests from tests.cfg file */
	if (!validate_and_load_tests(&tests,&tests_hash))
	{
		MTXDBG(INTERROGATOR|CRITICAL,_("Didn't return a valid list of commands\n\t MegaTunix was NOT installed correctly, Aborting Interrogation\n"));
		update_logbar_f("interr_view",NULL,g_strdup(__FILE__": interrogate_ecu()\n\t validate_and_load_tests() didn't return a valid list of commands\n\t MegaTunix was NOT installed correctly, Aborting Interrogation\n"),FALSE,FALSE,TRUE);
		g_mutex_unlock(&mutex);
		EXIT();
		return FALSE;
	}
	thread_widget_set_sensitive_f("offline_button",FALSE);
	thread_widget_set_sensitive_f("interrogate_button",FALSE);
	/* how many tests.... */

	for (i=0;i<tests->len;i++)
	{
		/* Get the test */
		test = g_array_index(tests,Detection_Test *, i);
		/* Check func existance and run it */
		if (test->function)
		{
			test->result = test->function(&test->num_bytes);
			if (test->result_type == RESULT_TEXT)
				test->result_str = g_strndup((const gchar *)(test->result),test->num_bytes);
		}
		else
			test->result = NULL;
	}
	interrogated = determine_ecu(tests,tests_hash);
	DATA_SET(global_data,"interrogated",GINT_TO_POINTER(interrogated));
	if (interrogated)
	{
		thread_widget_set_sensitive_f("interrogate_button",FALSE);
		thread_widget_set_sensitive_f("offline_button",FALSE);
	}

	g_array_free(tests,TRUE);
	g_hash_table_destroy(tests_hash);

	g_mutex_unlock(&mutex);
	EXIT();
	return interrogated;
}


/*!
 \brief Issues a packet to the ECU to get its revision information. 
 \param length is a pointer to a location to store the length of the data
 received, or NULL
 \returns the Firmware version as a text string
 */
G_MODULE_EXPORT gchar *raw_request_firmware_version(gint *length)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	LibreEMS_Packet *packet = NULL;
	gchar *version = NULL;
	Serial_Params *serial_params = NULL;
	/* Raw packet */
	guint8 *buf = NULL;
	gint len = FIRMWARE_VERSION_REQ_PKT_LEN;
	guint8 pkt[FIRMWARE_VERSION_REQ_PKT_LEN];
	gint req = REQUEST_FIRMWARE_VERSION;
	gint resp = RESPONSE_FIRMWARE_VERSION;
	gint res = 0;
	gint i = 0;
	guint8 sum = 0;
	gint tmit_len = 0;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,NULL);

	if (DATA_GET(global_data,"offline"))
	{
		EXIT();
		return g_strdup("Offline");
	}

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (req & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (req & 0x00ff );
	for (i=0;i<len-1;i++)
		sum += pkt[i];
	pkt[len-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,len,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,resp);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, NULL))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,resp);
		g_free(buf);
		g_async_queue_unref(queue);
		EXIT();
		return NULL;
	}
	g_free(buf);
	packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,500000);
	deregister_packet_queue(PAYLOAD_ID,queue,resp);
	g_async_queue_unref(queue);
	if (packet)
	{
		version = g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_length);
		if (length)
			*length = packet->payload_length;
		libreems_packet_cleanup(packet);
	}
	EXIT();
	return version;
}


/*!
 \brief Issues a packet to the ECU to get its interface version. 
 \param length is a pointer to a location to store the length of the data
 received, or NULL
 \returns the Firmware interface version as a text string
 */
G_MODULE_EXPORT gchar * raw_request_interface_version(gint *length)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	LibreEMS_Packet *packet = NULL;
	gchar *version = NULL;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	/* Raw packet */
	gint len = INTERFACE_VERSION_REQ_PKT_LEN;
	guint8 pkt[INTERFACE_VERSION_REQ_PKT_LEN];
	gint req = REQUEST_INTERFACE_VERSION;
	gint resp = RESPONSE_INTERFACE_VERSION;
	gint res = 0;
	gint i = 0;
	guint8 sum = 0;
	gint tmit_len = 0;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,NULL);

	if (DATA_GET(global_data,"offline"))
	{
		EXIT();
		return g_strdup("Offline");
	}

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (req & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (req & 0x00ff );
	for (i=0;i<len-1;i++)
		sum += pkt[i];
	pkt[len-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,len,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,resp);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, NULL))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,resp);
		g_free(buf);
		g_async_queue_unref(queue);
		EXIT();
		return NULL;
	}
	g_free(buf);
	packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,500000);
	deregister_packet_queue(PAYLOAD_ID,queue,resp);
	g_async_queue_unref(queue);
	/*
	   if (packet)
	   printf("Firmware version PACKET ARRIVED!\n");
	   else
	   printf("TIMEOUT\n");
	 */

	if (packet)
	{
		version = g_strndup((const gchar *)(packet->data+packet->payload_base_offset),packet->payload_length);
		if (length)
			*length = packet->payload_length;
		libreems_packet_cleanup(packet);
	}
	EXIT();
	return version;
}



/*!
 \brief Issues a packet to the ECU to get its revision information. 
 received, or NULL
 */
G_MODULE_EXPORT void request_firmware_version()
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	gint seq = atomic_sequence();
	ENTER();
	output = initialize_outputdata_f();
	queue = g_async_queue_new();
//	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_FIRMWARE_VERSION);
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_FIRMWARE_VERSION));
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("empty_payload_pkt",output);
	EXIT();
	return;
}


/*!
 \brief Issues a packet to the ECU to get its build date
 */
G_MODULE_EXPORT void request_firmware_build_date()
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	gint seq = atomic_sequence();
	ENTER();
	output = initialize_outputdata_f();
	queue = g_async_queue_new();
	//register_packet_queue(PAYLOAD_ID,queue,RESPONSE_FIRMWARE_BUILD_DATE);
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_FIRMWARE_BUILD_DATE));
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("empty_payload_pkt",output);
	EXIT();
	return;
}


/*!
 \brief Issues a packet to the ECU to get its build date
 */
G_MODULE_EXPORT void request_firmware_decoder_name()
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	gint seq = atomic_sequence();
	ENTER();
	output = initialize_outputdata_f();
	queue = g_async_queue_new();
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_DECODER_NAME));
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("empty_payload_pkt",output);
	EXIT();
	return;
}


/*!
 \brief Issues a packet to the ECU to get its build compiler
 \param length is a pointer to a location to store the length of the data
 received, or NULL
 \returns the Firmware version as a text string
 */
G_MODULE_EXPORT void request_firmware_compiler()
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	gint seq = atomic_sequence();
	ENTER();
	output = initialize_outputdata_f();
	queue = g_async_queue_new();
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_FIRMWARE_COMPILER_VERSION));
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("empty_payload_pkt",output);
	EXIT();
	return;
}


/*!
 \brief Issues a packet to the ECU to get its build os
 \param length is a pointer to a location to store the length of the data
 received, or NULL
 \returns the Firmware version as a text string
 */
G_MODULE_EXPORT void request_firmware_build_os()
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	gint seq = atomic_sequence();
	ENTER();
	output = initialize_outputdata_f();
	queue = g_async_queue_new();
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_FIRMWARE_COMPILER_OS));
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("empty_payload_pkt",output);
	EXIT();
	return;
}


/*!
 \brief Issues a packet to the ECU to get its interface version. 
 \param length is a pointer to a location to store the length of the data
 received, or NULL
 \returns the Firmware interface version as a text string
 */
G_MODULE_EXPORT void request_interface_version()
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	gint seq = atomic_sequence();
	ENTER();
	output = initialize_outputdata_f();
	queue = g_async_queue_new();
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_INTERFACE_VERSION));
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("empty_payload_pkt",output);
	EXIT();
	return;
}


/*
 \brief Queries the ECU for a location ID list
 \paranm length is a pointer to a location where this function can store the
 length of data received, or NULL
 \returns a pointer to a Link list (GList) of location ID's
 */
G_MODULE_EXPORT GList *raw_request_location_ids(gint * length)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	LibreEMS_Packet *packet = NULL;
	GList *list = NULL;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	/* Raw packet */
	gint len = LOCATION_ID_LIST_REQ_PKT_LEN;
	guint8 pkt[LOCATION_ID_LIST_REQ_PKT_LEN];
	gint req = REQUEST_LIST_OF_LOCATION_IDS;
	gint resp = RESPONSE_LIST_OF_LOCATION_IDS;
	gint res = 0;
	gint i = 0;
	guint8 sum = 0;
	gint tmit_len = 0;
	guint8 flag = BLOCK_BITS_AND;
	guint16 bits = 0;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,NULL);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (req & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (req & 0x00ff );
	pkt[L_PAYLOAD_IDX+1] = flag;	/* AND/OR */
	bits |= BLOCK_IS_INDEXABLE | BLOCK_IN_RAM;
	pkt[L_PAYLOAD_IDX+2] = (bits & 0xff00) >> 8;	/* H bits */
	pkt[L_PAYLOAD_IDX+3] = (bits & 0x00ff); 	/* L bits */
	for (i=0;i<len-1;i++)
		sum += pkt[i];
	pkt[len-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,len,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,resp);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, NULL))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,resp);
		g_free(buf);
		g_async_queue_unref(queue);
		EXIT();
		return NULL;
	}
	g_free(buf);
	packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,500000);
	deregister_packet_queue(PAYLOAD_ID,queue,resp);
	g_async_queue_unref(queue);
	if (packet)
	{
		gint h = 0;
		gint l = 0;
		gint tmpi = 0;
		for (i=0;i<packet->payload_length;i++)
		{
			tmpi = 0;
			h = packet->data[packet->payload_base_offset+i];
			i++;
			l = packet->data[packet->payload_base_offset+i];
			tmpi = (h << 8) + l;
			list = g_list_append(list,GINT_TO_POINTER(tmpi));
		}
		if (length)
			*length = packet->payload_length;
		libreems_packet_cleanup(packet);
	}
	EXIT();
	return list;
}



/*
 \brief Queries the ECU for the details of a specific location ID
 \param loc_id, the location ID to get more information about
 \returns a pointer to a Location_Details structure
 \see Location_Details
 */
G_MODULE_EXPORT Location_Details *request_location_id_details(guint16 loc_id)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	LibreEMS_Packet *packet = NULL;
	GList *list = NULL;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	Location_Details *details = NULL;
	/* Raw packet */
	guint8 pkt[LOCATION_ID_DETAILS_REQ_PKT_LEN];
	gint res = 0;
	gint i = 0;
	guint8 sum = 0;
	gint tmit_len = 0;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,NULL);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_LOCATION_ID_DETAILS & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_LOCATION_ID_DETAILS & 0x00ff );
	pkt[L_PAYLOAD_IDX+1] = (loc_id & 0xff00) >> 8;	/* H location bits */
	pkt[L_PAYLOAD_IDX+2] = (loc_id & 0x00ff); 	/* L location bits */
	for (i=0;i<LOCATION_ID_DETAILS_REQ_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[LOCATION_ID_DETAILS_REQ_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,LOCATION_ID_DETAILS_REQ_PKT_LEN,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_LOCATION_ID_DETAILS);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, NULL))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_LOCATION_ID_DETAILS);
		g_free(buf);
		g_async_queue_unref(queue);
		EXIT();
		return NULL;
	}
	g_free(buf);
	packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,500000);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_LOCATION_ID_DETAILS);
	g_async_queue_unref(queue);
	if (packet)
	{
		gint tmpi = 0;
		gint h = 0;
		gint l = 0;
		/*printf("packet payload length %i\n",packet->payload_length);*/
		if (packet->payload_length != 12)
			printf("ERROR in locationID details response!\n");
		details = g_new0(Location_Details, 1);
		h = packet->data[packet->payload_base_offset];
		l = packet->data[packet->payload_base_offset+1];
		details->flags = (h << 8) + l;
		/*printf("loc id details flags %i\n",details->flags);*/
		h = packet->data[packet->payload_base_offset+2];
		l = packet->data[packet->payload_base_offset+3];
		details->parent = (h << 8) + l;
		/*printf("loc id details parent %i\n",details->parent);*/
		details->ram_page = packet->data[packet->payload_base_offset+4];
		details->flash_page = packet->data[packet->payload_base_offset+5];
		/*printf("loc id details ram_page %i\n",details->ram_page);*/
		/*printf("loc id details flash_page %i\n",details->flash_page);*/
		h = packet->data[packet->payload_base_offset+6];
		l = packet->data[packet->payload_base_offset+7];
		details->ram_address = (h << 8) + l;
		/*printf("loc id details ram_address %0x\n",details->ram_address);*/
		h = packet->data[packet->payload_base_offset+8];
		l = packet->data[packet->payload_base_offset+9];
		details->flash_address = (h << 8) + l;
		/*printf("loc id details flash_address %0x\n",details->flash_address);*/
		h = packet->data[packet->payload_base_offset+10];
		l = packet->data[packet->payload_base_offset+11];
		details->length = (h << 8) + l;
		/*printf("loc id details length %i\n",details->length);*/
		libreems_packet_cleanup(packet);
	}
	EXIT();
	return details;
}


/*!
 \brief validate_and_load_tests() loads the list of tests from the system
 checks them for validity, and populates the passed array and hashtables
 with them
 \param tests is a pointer to a pointer of a GArray structure of the tests
 \param tests_hash is a pointer to a pointer of a hashtable of the Tests
 \returns TRUE on success, FALSE otherwise
 */
G_MODULE_EXPORT gboolean validate_and_load_tests(GArray **tests, GHashTable **tests_hash)
{
	ConfigFile *cfgfile;
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
	gchar *pathstub = NULL;

	ENTER();
	pathstub = g_build_filename(INTERROGATOR_DATA_DIR,"Profiles",DATA_GET(global_data,"ecu_family"),"tests.cfg",NULL);
	filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,NULL);
	g_free(pathstub);
	if (!filename)
	{
		update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile tests file %s not found!\n"),filename),FALSE,FALSE,TRUE);
		EXIT();
		return FALSE;
	}

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
	{
		EXIT();
		return FALSE;
	}
	get_file_api_f(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile tests API mismatch (%i.%i != %i.%i):\n\tFile %s.\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE,TRUE);
		g_free(filename);
		cfg_free(cfgfile);
		EXIT();
		return FALSE;
	}

	*tests_hash = g_hash_table_new_full(g_str_hash,g_str_equal,NULL,test_cleanup);

	MTXDBG(INTERROGATOR,_("File %s, opened successfully\n"),filename);
	*tests = g_array_new(FALSE,TRUE,sizeof(Detection_Test *));
	cfg_read_int(cfgfile,"interrogation_tests","total_tests",&total_tests);
	for (i=0;i<total_tests;i++)
	{
		test = g_new0(Detection_Test, 1);
		section = g_strdup_printf("test_%.2i",i);
		if (!cfg_read_string(cfgfile,section,"test_name",&test->test_name))
		{
			MTXDBG(INTERROGATOR,_("test_name for %s is NULL\n"),section);
			g_free(section);
			break;
		}
		if (!cfg_read_string(cfgfile,section,"test_result_type",&tmpbuf))
		{
			MTXDBG(INTERROGATOR,_("test_result_type for %s is NULL\n"),section);
			g_free(section);
			break;
		}
		else
		{
			test->result_type = translate_string_f(tmpbuf);
			g_free(tmpbuf);
		}
		if (!cfg_read_string(cfgfile,section,"test_func",&test->test_func))
		{
			MTXDBG(INTERROGATOR,_("test_function for %s is NULL\n"),section);
			g_free(section);
			break;
		}
		get_symbol_f(test->test_func,(void **)&test->function);
		cfg_read_string(cfgfile,section,"test_desc",
				&test->test_desc);
		g_free(section);
		g_array_append_val(*tests,test);
		g_hash_table_insert(*tests_hash,test->test_name,test);
	}
	cfg_free(cfgfile);
	g_free(filename);
	EXIT();
	return TRUE;
}


/*!
  \brief Frees all the resources of a Detection_Test structure
  \param data is a pointer to a Detection_Test structure
  \see Detection_Test
  */
G_MODULE_EXPORT void test_cleanup(gpointer data)
{
	Detection_Test *test = (Detection_Test *)data;
	ENTER();
	cleanup_f(test->test_func);
	cleanup_f(test->test_name);
	cleanup_f(test->test_desc);
	cleanup_f(test->result_str);
	if (test->result_type == RESULT_LIST)
		g_list_free((GList *)test->result);
	else
		if (test->result)
			cleanup_f(test->result);
	cleanup_f(test);
	EXIT();
	return;
}


/*!
  \brief Compares the results of the test runs with the available 
  interrogation profiles, favoring USER profiles over system ones.
  If we have a match, allocated resources for a Firmware structure, and load
  the firmware details.
  \param tests is a pointer to a GArray of tests
  \param tests_hash is a pointer to the hashtable of tests
  \returns TRUE on if we hit a match, FALSE otherwise
  */
G_MODULE_EXPORT gboolean determine_ecu(GArray *tests, GHashTable *tests_hash)
{
	extern gconstpointer *global_data;
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
		MTXDBG(INTERROGATOR|CRITICAL,_("NO Interrogation profiles found, was MegaTunix installed properly?\n\n"));
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
		if (test->result_type == RESULT_DATA)
		{
			MTXDBG(INTERROGATOR,_("Command (%s): returned %i byts\n"),test->test_desc,test->num_bytes);
			thread_update_logbar_f("interr_view","info",g_strdup_printf("Command (%s): returned %i bytes)\n",test->test_desc,test->num_bytes),FALSE,FALSE);
		}
		if (test->result_type == RESULT_TEXT)
		{
			MTXDBG(INTERROGATOR,_("Command (%s): returned (%s)\n"),test->test_desc,test->result_str);
			thread_update_logbar_f("interr_view","info",g_strdup_printf("Command (%s): returned (%s)\n",test->test_desc,test->result_str),FALSE,FALSE);
		}
		else if (test->result_type == RESULT_LIST)
		{
			MTXDBG(INTERROGATOR,_("Command (%s): returned %i elements\n"),test->test_desc,g_list_length((GList *)test->result));
			thread_update_logbar_f("interr_view","info",g_strdup_printf("Command (%s): returned %i elements\n",test->test_desc,g_list_length((GList *)test->result)),FALSE,FALSE);
		}
	}
	if (match == FALSE) /* (we DID NOT find one) */
	{
		MTXDBG(INTERROGATOR,_("Firmware NOT DETECTED, Enable Interrogation deb  ugging, retry interrogation,\nclose megatunix, and send ~/mtx/%s/debug.log to   the author for analysis with a note\ndescribing which firmware you are attempt  ing to talk to.\n"),(gchar *)DATA_GET(global_data,"project_name"));           
		update_logbar_f("interr_view","warning",g_strdup_printf("Firmware NOT   DETECTED, Enable Interrogation debugging, retry interrogation,\nclose megatuni  x, and send ~/mtx/%s/debug.log to the author for analysis with a note\ndescrib  ing which firmware you are attempting to talk to.\n",(gchar *)DATA_GET(global_data,"project_name")),FALSE,FALSE,TRUE);
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
 \brief check_for_match() compares the results of the interrogation with the
 ECU to the canidates in turn. When a match occurs TRUE is returned
 otherwise it returns FALSE
 \param tests_hash is a pointer to an hashtable of tests
 \param filename is a pointer to an interrogation profile file to check against
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
	MatchClass match_class;

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
		match_class = (MatchClass)translate_string_f(vector[0]);
		/*printf("potential data is %s\n",vector[1]);*/
		switch (match_class)
		{
			case COUNT:
				if (test->num_bytes == atoi(vector[1]))
					pass = TRUE;
				break;
			case NUMMATCH:
				if (test->result_str)
				{
					if ((GINT)(test->result_str[0]) == atoi(vector[1]))
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
					if (g_regex_match_simple(vector[1],test->result_str,
								(GRegexCompileFlags)0,
								(GRegexMatchFlags)0))
						pass = TRUE;
				}
				else
					pass = FALSE;
				break;
			default:
				pass = FALSE;
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
	MTXDBG(INTERROGATOR,_("\"%s\" is a match for all conditions ...\n\n"),filename);
	cfg_free(cfgfile);
	EXIT();
	return TRUE;
}


/*!
  \brief updates the interrogation Gui (General Tab) with the Firmware and 
  Interface Versions
  */
G_MODULE_EXPORT void update_ecu_info(void)
{
	gchar *int_version = NULL;
	gchar *fw_version = NULL;
	gchar *build_date = NULL;
	gchar *build_os = NULL;
	gchar *compiler = NULL;
	gchar *decoder = NULL;
	gchar *info = NULL;

	ENTER();
	fw_version = (gchar *)DATA_GET(global_data,"fw_version");
	int_version = (gchar *)DATA_GET(global_data,"int_version");
	build_date = (gchar *)DATA_GET(global_data,"build_date");
	build_os = (gchar *)DATA_GET(global_data,"build_os");
	compiler = (gchar *)DATA_GET(global_data,"compiler");
	decoder = (gchar *)DATA_GET(global_data,"decoder_name");
	if ((fw_version) && (int_version) && (build_date) && (build_os) && (compiler) && (decoder))
	{
		info = g_strdup_printf("<b>Firmware Version:</b> %s\n<b>Interface version:</b> %s\n<b>Decoder Name:</b>%s\nThis firmware was built on %s using the %s compiler on %s",fw_version,int_version,decoder,build_date,compiler,build_os);
		thread_update_widget_f("ecu_info_label",MTX_LABEL,g_strdup(info));
		g_free(info);
	}
	EXIT();
	return;
}


/*!
  \brief Initiates the calls to get the info to update the tab with the
  interrogated ECU infomation
  */
G_MODULE_EXPORT void update_interrogation_gui_pf(void)
{
	ENTER();
	if (!DATA_GET(global_data,"interrogated"))
	{
		EXIT();
		return;
	}
	/* Request firmware version */
	request_firmware_version();
	request_interface_version();
	request_firmware_build_date();
	request_firmware_build_os();
	request_firmware_compiler();
	EXIT();
	return;
}


/*!
  \brief Loads the remaining parts of the Interrogation profile and populates
  the firmware structure allocation resources as needed based on the contents
  of the profile
  \param firmware is a pointer to an allocated Firmware_Details structure
  \param filename is a pointer to the matched interrogation profile
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean load_firmware_details(Firmware_Details *firmware, gchar * filename)
{
	ConfigFile *cfgfile;
	Location_Details *details = NULL;
	GList *locations = NULL;
	gchar *tmpbuf = NULL;
	gchar *section = NULL;
	gchar ** list = NULL;
	gint tmpi = 0;
	gint major = 0;
	gint minor = 0;
	gint size = 0;
	guint8 *packet = NULL;
	gboolean res = FALSE;
	gint x_bins = 0;
	gint y_bins = 0;

	ENTER();

	cfgfile = cfg_open_file((gchar *)filename);
	if (!cfgfile)
		MTXDBG(INTERROGATOR|CRITICAL,_("File \"%s\" NOT OPENED successfully\n"),filename);
	get_file_api_f(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		thread_update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE);
		cfg_free(cfgfile);
		EXIT();
		return FALSE;
	}
	MTXDBG(INTERROGATOR,_("File:%s opened successfully\n"),filename);

	firmware->profile_filename = g_strdup(filename);
	cfg_read_string(cfgfile,"interrogation_profile","name",&firmware->name);
	if(cfg_read_string(cfgfile,"parameters","EcuTempUnits",&tmpbuf))
	{
		firmware->ecu_temp_units = (TempUnits)translate_string_f(tmpbuf);
		g_free(tmpbuf);
	}
	else
		MTXDBG(INTERROGATOR,_("Failed to find EcuTempUnits key in interrogation profile\n"));

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
		firmware->capabilities = translate_capabilities(tmpbuf);
		g_free(tmpbuf);
	}
	/* Commands to map against the comm.xml */
	if(!cfg_read_string(cfgfile,"parameters","RT_Command",&firmware->rt_command))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"RT_Command\" variable not found in interrogation profile, ERROR\n"));
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
	if(!cfg_read_boolean(cfgfile,"parameters","ChunkWriteSupport",
				&firmware->chunk_support))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"ChunkWriteSupport\" flag not found in parameters section in interrogation profile, ERROR\n"));
	if (firmware->chunk_support)
	{
		if(!cfg_read_string(cfgfile,"parameters","Chunk_Write_Command",
					&firmware->chunk_write_command))
			MTXDBG(INTERROGATOR|CRITICAL,_("\"Chunk_Write_Command\" flag not found in parameters section in interrogation profile, ERROR\n"));
	}

	/* Gui Section */
	if(!cfg_read_string(cfgfile,"gui","LoadTabs",&tmpbuf))
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
	if(!cfg_read_string(cfgfile,"gui","RuntimeTextMapFile",
				&firmware->rtt_map_file))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"RuntimeTextMapFile\" variable not found in interrogation profile, ERROR\n"));
	if(!cfg_read_string(cfgfile,"gui","StatusMapFile",
				&firmware->status_map_file))
		MTXDBG(INTERROGATOR|CRITICAL,_("\"StatusMapFile\" variable not found in interrogation profile, ERROR\n"));

	/* Mtx maps location id's as pseudo "pages" */
	locations = raw_request_location_ids(NULL);
	if (locations)
	{
		firmware->total_pages = g_list_length(locations);

		firmware->page_params = g_new0(Page_Params *, firmware->total_pages);
		for (gint i=0;i<firmware->total_pages;i++)
		{
			firmware->page_params[i] = initialize_page_params();
			firmware->page_params[i]->phys_ecu_page = (GINT)g_list_nth_data(locations,i);
			/*printf("mtx page %i  corresponds to ecu physicaly location id %i\n",i,firmware->page_params[i]->phys_ecu_page);*/
			details = request_location_id_details((GINT)g_list_nth_data(locations,i));
			if (details)
			{
				firmware->page_params[i]->length = details->length;
				firmware->page_params[i]->dl_by_default = (details->flags & BLOCK_IS_INDEXABLE);
				firmware->page_params[i]->read_only = (details->flags & BLOCK_IS_READONLY);
					g_free(details);
			}
		}
		g_list_free(locations);
	}

	/* MAJOR HACK ALERT,  hardcoded for fred! */
	firmware->total_tables = 3;
	firmware->table_params = g_new0(Table_Params *,firmware->total_tables);
	/* Fuel Table */
	firmware->table_params[0] = initialize_table_params();
	res = libreems_find_mtx_page(0,&tmpi);
	if (res)
	{
		firmware->table_params[0]->x_page = tmpi;
		firmware->table_params[0]->y_page = tmpi;
		firmware->table_params[0]->z_page = tmpi;
	}
	else
	{
		MTXDBG(INTERROGATOR|CRITICAL,_("CRITICAL FAULT, unable to determine mtx page for location ID %i\n"),0);
		exit(-1);
	}
	/* Hard coding this is bad,  but since Fred Doesn't provide any way to 
	 * get metadata from the ECU, there is no real clean way!
	 * */
	res = get_dimensions(0,0,4, &x_bins,&y_bins);
	if (!res)
		printf("unable to get table dimensions for location ID 0\n");
	firmware->table_params[0]->x_bincount = x_bins;
	firmware->table_params[0]->y_bincount = y_bins;
	firmware->table_params[0]->x_base = 4;
	firmware->table_params[0]->y_base = 58;
	firmware->table_params[0]->z_base = 100;
	firmware->table_params[0]->x_size = MTX_U16;
	firmware->table_params[0]->y_size = MTX_U16;
	firmware->table_params[0]->z_size = MTX_U16;
	firmware->table_params[0]->x_fromecu_mult = g_new0(gfloat, 1);
	firmware->table_params[0]->y_fromecu_mult = g_new0(gfloat, 1);
	firmware->table_params[0]->z_fromecu_mult = g_new0(gfloat, 1);
	firmware->table_params[0]->z_raw_lower = 0;
	firmware->table_params[0]->z_raw_upper = 65535;
	*(firmware->table_params[0]->x_fromecu_mult) = 0.5;
	*(firmware->table_params[0]->y_fromecu_mult) = 0.01;
	*(firmware->table_params[0]->z_fromecu_mult) = 1.0/512;
	firmware->table_params[0]->x_source = g_strdup("RPM");
	firmware->table_params[0]->y_source = g_strdup("LoadMain");
	firmware->table_params[0]->z_source = g_strdup("VEMain");
	firmware->table_params[0]->x_suffix = g_strdup("RPM");
	firmware->table_params[0]->y_suffix = g_strdup("kPa");
	firmware->table_params[0]->z_suffix = g_strdup("%");
	firmware->table_params[0]->x_precision = 0;
	firmware->table_params[0]->y_precision = 1;
	firmware->table_params[0]->z_precision = 1;
	firmware->table_params[0]->x_use_color = FALSE;
	firmware->table_params[0]->y_use_color = FALSE;
	firmware->table_params[0]->z_use_color = TRUE;
	firmware->table_params[0]->table_name = g_strdup("LibreEMS very alpha fuel table");;
	/* Lambda Table */
	firmware->table_params[1] = initialize_table_params();
	res = libreems_find_mtx_page(6,&tmpi);
	if (res)
	{
		firmware->table_params[1]->x_page = tmpi;
		firmware->table_params[1]->y_page = tmpi;
		firmware->table_params[1]->z_page = tmpi;
	}
	else
	{
		MTXDBG(INTERROGATOR|CRITICAL,_("CRITICAL FAULT, unable to determine mtx page for location ID %i\n"),0);
		exit(-1);
	}
	res = get_dimensions(6,0,4, &x_bins,&y_bins);
	if (!res)
		printf("unable to get table dimensions for location ID 1\n");
	firmware->table_params[1]->x_bincount = x_bins;
	firmware->table_params[1]->y_bincount = y_bins;
	firmware->table_params[1]->x_base = 4;
	firmware->table_params[1]->y_base = 58;
	firmware->table_params[1]->z_base = 100;
	firmware->table_params[1]->x_size = MTX_U16;
	firmware->table_params[1]->y_size = MTX_U16;
	firmware->table_params[1]->z_size = MTX_U16;
	firmware->table_params[1]->x_fromecu_mult = g_new0(gfloat, 1);
	firmware->table_params[1]->y_fromecu_mult = g_new0(gfloat, 1);
	firmware->table_params[1]->z_fromecu_mult = g_new0(gfloat, 1);
	firmware->table_params[1]->z_raw_lower = 22282;
	firmware->table_params[1]->z_raw_upper = 44565;
	*(firmware->table_params[1]->x_fromecu_mult) = 0.5;
	*(firmware->table_params[1]->y_fromecu_mult) = 0.01;
	*(firmware->table_params[1]->z_fromecu_mult) = 1.0/32768;
	firmware->table_params[1]->x_source = g_strdup("RPM");
	firmware->table_params[1]->y_source = g_strdup("LoadMain");
	firmware->table_params[1]->z_source = g_strdup("Lambda");
	firmware->table_params[1]->x_suffix = g_strdup("RPM");
	firmware->table_params[1]->y_suffix = g_strdup("kPa");
	firmware->table_params[1]->z_suffix = g_strdup("Lambda");
	firmware->table_params[1]->x_precision = 0;
	firmware->table_params[1]->y_precision = 1;
	firmware->table_params[1]->z_precision = 2;
	firmware->table_params[1]->x_use_color = FALSE;
	firmware->table_params[1]->y_use_color = FALSE;
	firmware->table_params[1]->z_use_color = TRUE;
	firmware->table_params[1]->table_name = g_strdup("LibreEMS very alpha lambda table");;

	firmware->table_params[2] = initialize_table_params();
	res = libreems_find_mtx_page(8,&tmpi);
	if (res)
	{
		firmware->table_params[2]->x_page = tmpi;
		firmware->table_params[2]->y_page = tmpi;
		firmware->table_params[2]->z_page = tmpi;
	}
	else
	{
		MTXDBG(INTERROGATOR|CRITICAL,_("CRITICAL FAULT, unable to determine mtx page for location ID %i\n"),0);
		exit(-1);
	}
	res = get_dimensions(8,0,4, &x_bins,&y_bins);
	if (!res)
		printf("unable to get table dimensions for location ID 0\n");
	firmware->table_params[2]->x_bincount = x_bins;
	firmware->table_params[2]->y_bincount = y_bins;
	firmware->table_params[2]->x_base = 4;
	firmware->table_params[2]->y_base = 58;
	firmware->table_params[2]->z_base = 100;
	firmware->table_params[2]->x_size = MTX_U16;
	firmware->table_params[2]->y_size = MTX_U16;
	firmware->table_params[2]->z_size = MTX_U16;
	firmware->table_params[2]->x_fromecu_mult = g_new0(gfloat, 1);
	firmware->table_params[2]->y_fromecu_mult = g_new0(gfloat, 1);
	firmware->table_params[2]->z_fromecu_mult = g_new0(gfloat, 1);
	firmware->table_params[2]->z_raw_lower = 0;
	firmware->table_params[2]->z_raw_upper = 65535;
	*(firmware->table_params[2]->x_fromecu_mult) = 0.5;
	*(firmware->table_params[2]->y_fromecu_mult) = 0.01;
	*(firmware->table_params[2]->z_fromecu_mult) = 1.0/1024;
	firmware->table_params[2]->x_source = g_strdup("RPM");
	firmware->table_params[2]->y_source = g_strdup("LoadMain");
	firmware->table_params[2]->z_source = g_strdup("Degrees");
	firmware->table_params[2]->x_suffix = g_strdup("RPM");
	firmware->table_params[2]->y_suffix = g_strdup("kPa");
	firmware->table_params[2]->z_suffix = g_strdup("\302\260BTDC");
	firmware->table_params[2]->x_precision = 0;
	firmware->table_params[2]->y_precision = 1;
	firmware->table_params[2]->z_precision = 1;
	firmware->table_params[2]->x_use_color = FALSE;
	firmware->table_params[2]->y_use_color = FALSE;
	firmware->table_params[2]->z_use_color = TRUE;
	firmware->table_params[2]->table_name = g_strdup("LibreEMS very alpha spark table");;

	if (mem_alloc_f)
		mem_alloc_f();
	else
		MTXDBG(INTERROGATOR|CRITICAL,_("FAILED TO LOCATE \"mem_alloc\" function within core/plugins\n"));

	/* Display firmware version in the window... */

	MTXDBG(INTERROGATOR|CRITICAL,_("Detected Firmware: %s\n"),firmware->name);
	thread_update_logbar_f("interr_view","warning",g_strdup_printf(_("Detected Firmware: %s\n"),firmware->name),FALSE,FALSE);
	thread_update_logbar_f("interr_view","info",g_strdup_printf(_("Loading Settings from: \"%s\"\n"),firmware->profile_filename),FALSE,FALSE);
	cfg_free(cfgfile);

	EXIT();
	return TRUE;
}



/*!
 \brief translate_capabilities() converts a stringlist into a mask of 
 enumerations and returns it
 \param string is a listing of capabilities in textual format
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
		MTXDBG(INTERROGATOR|CRITICAL,_("String fed is NULL!\n"));
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
 \brief Initializes a Page_Params datastructure and returns a pointer to it
 */
G_MODULE_EXPORT Page_Params * initialize_page_params(void)
{
	Page_Params *page_params = NULL;
	ENTER();
	page_params = (Page_Params *)g_malloc0(sizeof(Page_Params));
	page_params->length = 0;
	page_params->spconfig_offset = -1;
	page_params->needs_burn = FALSE;
	EXIT();
	return page_params;
}


/*!
 \brief Initializes a Table_Params datastructure and returns a pointer to it
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
	table_params->x_fromecu_mult = NULL;
	table_params->x_fromecu_add = NULL;
	table_params->y_fromecu_mult = NULL;
	table_params->y_fromecu_add = NULL;
	table_params->z_fromecu_mult = NULL;
	table_params->z_fromecu_add = NULL;
	table_params->x_fromecu_mults = NULL;
	table_params->x_fromecu_adds = NULL;
	table_params->y_fromecu_mults = NULL;
	table_params->y_fromecu_adds = NULL;
	table_params->z_fromecu_mults = NULL;
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

	EXIT();
	return table_params;
}


/*!
 \brief Gets the X/Y sizes for the table requested at the specified location ID/offset and length,  typically offset and length are 0 and 4 respectively (reads the first 4 bytes of the table)
 \returns TRU on success, false otherwise
 */
gboolean get_dimensions(gint location_id,gint offset,gint length,gint *x_bins, gint *y_bins)
{
	guint8 *buf = NULL;
	LibreEMS_Packet *packet = NULL;
	gint size = 0;
	GAsyncQueue *queue = NULL;
	guint16 *ptr = NULL;
	gint resp = RESPONSE_RETRIEVE_BLOCK_FROM_RAM;
	Serial_Params *serial_params = NULL;

	ENTER();
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,FALSE);
	g_return_val_if_fail(x_bins,FALSE);
	g_return_val_if_fail(y_bins,FALSE);
	g_return_val_if_fail(length > 0,FALSE);

	buf = make_me_a_packet(&size,PAYLOAD_ID,REQUEST_RETRIEVE_BLOCK_FROM_RAM,\
			LOCATION_ID,location_id,\
			OFFSET,offset,\
			LENGTH,length,\
			-1);

	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,resp);
	if (!write_wrapper_f(serial_params->fd,buf, size, NULL))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,resp);
		g_free(buf);
		g_async_queue_unref(queue);
		EXIT();
		return FALSE;
	}
	g_free(buf);
	packet = (LibreEMS_Packet *)g_async_queue_timeout_pop(queue,500000);
	deregister_packet_queue(PAYLOAD_ID,queue,resp);
	g_async_queue_unref(queue);
	if (packet)
	{
		*x_bins = (packet->data[packet->payload_base_offset] << 8 ) + packet->data[packet->payload_base_offset +1];
		*y_bins = (packet->data[packet->payload_base_offset+2] << 8 ) + packet->data[packet->payload_base_offset +3];
		libreems_packet_cleanup(packet);
		EXIT();
		return TRUE;
	}
	EXIT();
	return FALSE;
}
