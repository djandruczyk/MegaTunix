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

#include <apicheck.h>
#include <api-versions.h>
#include <config.h>
#include <configfile.h>
#include <debugging.h>
#include <getfiles.h>
#include <gtk/gtk.h>
#include <interrogate.h>
#include <freeems_plugin.h>
#include <freeems_helpers.h>
#include <packet_handlers.h>
#include <serialio.h>
#include <string.h>

extern gconstpointer *global_data;


/*!
 \brief interrogate_ecu() interrogates the target ECU to determine what
 firmware it is running.  It does this by reading a list of tests, sending
 those tests in turn, reading the responses and them comparing the group of
 responses against a list of interrogation profiles until it finds a match.
 */
G_MODULE_EXPORT gboolean interrogate_ecu(void)
{
	GAsyncQueue *queue = NULL;
	GHashTable *tests_hash = NULL;
	GArray *tests = NULL;
	Detection_Test *test = NULL;
	gchar *version = NULL;
	gchar *fw_version = NULL;
	guint8 major = 0;
	guint8 minor = 0;
	guint8 micro = 0;
	gint i = 0;
	gboolean interrogated = FALSE;
	GList *locations = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	/* prevent multiple runs of interrogator simultaneously */
	g_static_mutex_lock(&mutex);
	dbg_func_f(INTERROGATOR,g_strdup("\n"__FILE__": interrogate_ecu() ENTERED\n\n"));

	/* ECU has already been detected via comms test
	   Now we need to figure out its variant and adapt to it
	 */
	/* Send stream disable command */
	stop_streaming();

	/* Load tests from tests.cfg file */
	if (!validate_and_load_tests(&tests,&tests_hash))
	{
		dbg_func_f(INTERROGATOR|CRITICAL,g_strdup(__FILE__": interrogate_ecu()\n\t validate_and_load_tests() didn't return a valid list of commands\n\t MegaTunix was NOT installed correctly, Aborting Interrogation\n"));
		update_logbar_f("interr_view",NULL,g_strdup(__FILE__": interrogate_ecu()\n\t validate_and_load_tests() didn't return a valid list of commands\n\t MegaTunix was NOT installed correctly, Aborting Interrogation\n"),FALSE,FALSE,TRUE);
		g_static_mutex_unlock(&mutex);
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
			test->result = test->function();
		else
			test->result = NULL;
	}
//	interrogated = determine_ecu(tests,tests_hash);
	DATA_SET(global_data,"interrogated",GINT_TO_POINTER(interrogated));
	if (interrogated)
	{
		thread_widget_set_sensitive_f("interrogate_button",FALSE);
		thread_widget_set_sensitive_f("offline_button",FALSE);
	}

	g_array_free(tests,TRUE);
	g_hash_table_destroy(tests_hash);


	/* Request firmware version */
	fw_version = request_firmware_version();
	update_logbar_f("interr_view",NULL,g_strdup_printf(_("Firmware Version request returned %i bytes (%s)\n"),(gint)strlen(fw_version),fw_version),FALSE,FALSE,TRUE);
	thread_update_widget_f("ecu_signature_entry",MTX_ENTRY,g_strdup(fw_version));
	g_free(fw_version);

	/* Request interface version */
	/* version = request_detailed_interface_version(&major, &minor, &micro);*/
	version = request_interface_version();
	update_logbar_f("interr_view",NULL,g_strdup_printf(_("Interface Version request returned %i bytes ( %i.%i.%i %s)\n"),(gint)strlen(version)+3,major,minor,micro,version),FALSE,FALSE,TRUE);

	thread_update_widget_f("text_version_entry",MTX_ENTRY,g_strdup(version));
	thread_update_widget_f("ecu_revision_entry",MTX_ENTRY,g_strdup_printf("%i.%i.%i",major,minor,micro));
	g_free(version);

	/* Request List of location ID's */
	locations = request_location_ids();
	if (locations)
	{
		update_logbar_f("interr_view",NULL,g_strdup_printf(_("Location ID Query returned %i locationID's.\n"),g_list_length(locations)),FALSE,FALSE,TRUE);

		for (i=0;i<g_list_length(locations);i++)
		{
			printf("Locations ID %i\n",(gint)g_list_nth_data(locations,i));
		}
		g_list_free(locations);
	}
	else
		update_logbar_f("interr_view",NULL,g_strdup_printf(_("Location ID Query FAILED to return any locationID's.\n")),FALSE,FALSE,TRUE);
	/* FreeEMS Interrogator NOT WRITTEN YET */
	return TRUE;
}


gchar *request_firmware_version(void)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	gchar *version = NULL;
	GTimeVal tval;
	Serial_Params *serial_params = NULL;
	/* Raw packet */
	guint8 *buf = NULL;
	guint8 pkt[FIRM_REQ_PKT_LEN];
	gint res = 0;
	gint len = 0;
	gint i = 0;
	guint8 sum = 0;
	gint tmit_len = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,NULL);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_FIRMWARE_VERSION & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_FIRMWARE_VERSION & 0x00ff );
	for (i=0;i<FIRM_REQ_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[FIRM_REQ_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,FIRM_REQ_PKT_LEN,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_FIRMWARE_VERSION);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, &len))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_FIRMWARE_VERSION);
		g_free(buf);
		g_async_queue_unref(queue);
		return NULL;
	}
	g_free(buf);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_FIRMWARE_VERSION);
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

		freeems_packet_cleanup(packet);
	}
	return version;
}


gchar * request_interface_version(void)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	gchar *version = NULL;
	GTimeVal tval;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	/* Raw packet */
	guint8 pkt[INTVER_REQ_PKT_LEN];
	gint res = 0;
	gint len = 0;
	gint i = 0;
	guint8 sum = 0;
	gint tmit_len = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,NULL);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0x00ff );
	for (i=0;i<INTVER_REQ_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[INTVER_REQ_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,INTVER_REQ_PKT_LEN,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, &len))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
		g_free(buf);
		g_async_queue_unref(queue);
		return NULL;
	}
	g_free(buf);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
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
		freeems_packet_cleanup(packet);
	}
	return version;
}


gchar * request_detailed_interface_version(guint8 *major, guint8 *minor, guint8 *micro)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	gchar *version = NULL;
	GTimeVal tval;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	/* Raw packet */
	guint8 pkt[INTVER_REQ_PKT_LEN];
	gint res = 0;
	gint len = 0;
	gint i = 0;
	guint8 sum = 0;
	gint tmit_len = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,NULL);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0x00ff );
	for (i=0;i<INTVER_REQ_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[INTVER_REQ_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,INTVER_REQ_PKT_LEN,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, &len))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
		g_free(buf);
		g_async_queue_unref(queue);
		return NULL;
	}
	g_free(buf);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
	g_async_queue_unref(queue);
	/*
	   if (packet)
	   printf("Firmware version PACKET ARRIVED!\n");
	   else
	   printf("TIMEOUT\n");
	 */

	if (packet)
	{
		version = g_strndup((const gchar *)(packet->data+packet->payload_base_offset+3),packet->payload_length-3);
		*major = (guint8)(packet->data[packet->payload_base_offset]);
		*minor = (guint8)(packet->data[packet->payload_base_offset+1]);
		*micro = (guint8)(packet->data[packet->payload_base_offset+2]);
		freeems_packet_cleanup(packet);
	}
	return version;
}


/*
 \brief Queries the ECU for a location ID list
 */
GList *request_location_ids(void)
{
	OutputData *output = NULL;
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	GList *list = NULL;
	Serial_Params *serial_params = NULL;
	guint8 *buf = NULL;
	/* Raw packet */
	guint8 pkt[LOC_ID_LIST_REQ_PKT_LEN];
	gint res = 0;
	gint len = 0;
	gint i = 0;
	gint h = 0;
	gint l = 0;
	gint tmpi = 0;
	guint8 sum = 0;
	gint tmit_len = 0;
	guint8 flag = BLOCK_BITS_OR;
	guint16 bits = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	g_return_val_if_fail(serial_params,NULL);

	pkt[HEADER_IDX] = 0;
	pkt[H_PAYLOAD_IDX] = (REQUEST_RETRIEVE_LIST_OF_LOCATION_IDS & 0xff00 ) >> 8;
	pkt[L_PAYLOAD_IDX] = (REQUEST_RETRIEVE_LIST_OF_LOCATION_IDS & 0x00ff );
	pkt[L_PAYLOAD_IDX+1] = flag;	/* AND/OR */
	bits |= BLOCK_IS_INDEXABLE;
	pkt[L_PAYLOAD_IDX+2] = (bits & 0xff00) >> 8;	/* H bits */
	pkt[L_PAYLOAD_IDX+3] = (bits & 0x00ff); 	/* L bits */
	for (i=0;i<LOC_ID_LIST_REQ_PKT_LEN-1;i++)
		sum += pkt[i];
	pkt[LOC_ID_LIST_REQ_PKT_LEN-1] = sum;
	buf = finalize_packet((guint8 *)&pkt,LOC_ID_LIST_REQ_PKT_LEN,&tmit_len);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_RETRIEVE_LIST_OF_LOCATION_IDS);
	if (!write_wrapper_f(serial_params->fd,buf, tmit_len, &len))
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_RETRIEVE_LIST_OF_LOCATION_IDS);
		g_free(buf);
		g_async_queue_unref(queue);
		return NULL;
	}
	g_free(buf);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_RETRIEVE_LIST_OF_LOCATION_IDS);
	g_async_queue_unref(queue);
	if (packet)
	{
		for (i=0;i<packet->payload_length;i++)
		{
			tmpi = 0;
			h = packet->data[packet->payload_base_offset+i];
			i++;
			l = packet->data[packet->payload_base_offset+i];
			tmpi = (h << 8) + l;
			list = g_list_append(list,GINT_TO_POINTER(tmpi));
		}
		freeems_packet_cleanup(packet);
	}
	return list;
}


/*!
 \brief validate_and_load_tests() loads the list of tests from the system
 checks them for validity, populates and array and returns it
 command tested against the ECU arestored
 \returns a dynamic GArray for commands
 */
gboolean validate_and_load_tests(GArray **tests, GHashTable **tests_hash)
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

	filename = get_file(g_build_filename(INTERROGATOR_DATA_DIR,"Profiles",DATA_GET(global_data,"ecu_family"),"tests.cfg",NULL),NULL);
	if (!filename)
	{
		update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile tests file %s not found!\n"),filename),FALSE,FALSE,TRUE);
		return FALSE;
	}

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		return FALSE;
	get_file_api_f(cfgfile,&major,&minor);
	if ((major != INTERROGATE_MAJOR_API) || (minor != INTERROGATE_MINOR_API))
	{
		update_logbar_f("interr_view","warning",g_strdup_printf(_("Interrogation profile tests API mismatch (%i.%i != %i.%i):\n\tFile %s.\n"),major,minor,INTERROGATE_MAJOR_API,INTERROGATE_MINOR_API,filename),FALSE,FALSE,TRUE);
		return FALSE;
	}

	*tests_hash = g_hash_table_new_full(g_str_hash,g_str_equal,NULL,test_cleanup);

	dbg_func_f(INTERROGATOR,g_strdup_printf(__FILE__": validate_and_load_tests()\n\tfile %s, opened successfully\n",filename));
	*tests = g_array_new(FALSE,TRUE,sizeof(Detection_Test *));
	cfg_read_int(cfgfile,"interrogation_tests","total_tests",&total_tests);
	for (i=0;i<total_tests;i++)
	{
		test = g_new0(Detection_Test, 1);
		section = g_strdup_printf("test_%.2i",i);
		if (!cfg_read_string(cfgfile,section,"test_name",&test->test_name))
		{
			dbg_func_f(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": validate_and_load_tests(),\n\ttest_name for %s is NULL\n",section));
			g_free(section);
			break;
		}
		if (!cfg_read_string(cfgfile,section,"test_result_type",&tmpbuf))
		{
			dbg_func_f(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": validate_and_load_tests(),\n\ttest_result_type for %s is NULL\n",section));
			g_free(section);
			break;
		}
		else
		{
			test->result_type=translate_string_f(tmpbuf);
			g_free(tmpbuf);
		}
		if (!cfg_read_string(cfgfile,section,"test_func",&test->test_func))
		{
			dbg_func_f(INTERROGATOR|CRITICAL,g_strdup_printf(__FILE__": validate_and_load_tests(),\n\ttest_function for %s is NULL\n",section));
			g_free(section);
			break;
		}
		get_symbol_f(test->test_func,(void *)&test->function);
		cfg_read_string(cfgfile,section,"test_desc",
				&test->test_desc);
		g_free(section);
		g_array_append_val(*tests,test);
		g_hash_table_insert(*tests_hash,test->test_name,test);
	}
	cfg_free(cfgfile);
	g_free(filename);
	return TRUE;
}


void test_cleanup(gpointer data)
{
	Detection_Test *test = (Detection_Test *)data;
	cleanup_f(test->test_func);
	cleanup_f(test->test_name);
	cleanup_f(test->test_desc);
	if (test->result_type == RESULT_LIST)
		g_list_free((GList *)test->result);
	else
		cleanup_f(test->result);
	cleanup_f(test);
}

