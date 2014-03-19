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
  \file src/plugins/libreems/libreems_plugin.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS Plugin init/shutdown functionality
  \author David Andruczyk
  */

#define __LIBREEMS_PLUGIN_C__
#include <config.h>
#include <defines.h>
#include <firmware.h>
#include <libreems_comms.h>
#include <libreems_gui_handlers.h>
#include <libreems_helpers.h>
#include <libreems_plugin.h>
#include <gtk/gtk.h>
#include <interrogate.h>
#include <packet_handlers.h>


gconstpointer *global_data = NULL;


/*!
  \brief initializes the LibreEMS plugin via locating and attaching to all
  needed functions within the core MegaTunix. It also registers any 
  plugin specific enumerations, and registers/sets up any additional resources
  needed like threads, Queues, memory, etc
  \param data is a pointer to the global data container.
 */ 
G_MODULE_EXPORT void plugin_init(gconstpointer *data)
{
	GAsyncQueue *queue = NULL;
	GThread *thread = NULL;
	GHashTable *hash = NULL;
	GCond *serial_reader_cond = NULL;
	GCond *packet_handler_cond = NULL;
	GMutex *rtv_subscriber_mutex = NULL;
	GMutex *atomic_sequence_mutex = NULL;
	GMutex *queue_mutex = NULL;

	global_data = data;
	*(void **)(&error_msg_f) = DATA_GET(global_data,"error_msg_f");
	g_assert(error_msg_f);
	*(void **)(&get_symbol_f) = DATA_GET(global_data,"get_symbol_f");
	g_assert(get_symbol_f);
	get_symbol_f("dbg_func",(void **)&dbg_func_f);
	ENTER();
	/* Initializes function pointers since on Winblows you can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	get_symbol_f("_get_sized_data",(void **)&_get_sized_data_f);
	get_symbol_f("_set_sized_data",(void **)&_set_sized_data_f);
	get_symbol_f("check_tab_existance",(void **)&check_tab_existance_f);
	get_symbol_f("cleanup",(void **)&cleanup_f);
	get_symbol_f("combo_set_labels",(void **)&combo_set_labels_f);
	get_symbol_f("combo_toggle_groups_linked",(void **)&combo_toggle_groups_linked_f);
	get_symbol_f("combo_toggle_labels_linked",(void **)&combo_toggle_labels_linked_f);
	get_symbol_f("convert_after_upload",(void **)&convert_after_upload_f);
	get_symbol_f("convert_before_download",(void **)&convert_before_download_f);
	get_symbol_f("dump_output",(void **)&dump_output_f);
	get_symbol_f("entry_changed_handler",(void **)&entry_changed_handler_f);
	get_symbol_f("evaluator_create",(void **)&evaluator_create_f);
	get_symbol_f("evaluator_destroy",(void **)&evaluator_destroy_f);
	get_symbol_f("evaluator_evaluate_x",(void **)&evaluator_evaluate_x_f);
	get_symbol_f("flush_binary_logs",(void **)&flush_binary_logs_f);
	get_symbol_f("flush_serial",(void **)&flush_serial_f);
	get_symbol_f("get_colors_from_hue",(void **)&get_colors_from_hue_f);
	get_symbol_f("get_file_api",(void **)&get_file_api_f);
	get_symbol_f("get_list",(void **)&get_list_f);
	get_symbol_f("get_multiplier",(void **)&get_multiplier_f);
	get_symbol_f("insert_text_handler",(void **)&insert_text_handler_f);
	get_symbol_f("io_cmd",(void **)&io_cmd_f);
	get_symbol_f("initialize_outputdata",(void **)&initialize_outputdata_f);
	get_symbol_f("jump_to_tab",(void **)&jump_to_tab_f);
	get_symbol_f("log_inbound_data",(void **)&log_inbound_data_f);
	get_symbol_f("log_outbound_data",(void **)&log_outbound_data_f);
	get_symbol_f("lookup_current_value",(void **)&lookup_current_value_f);
	get_symbol_f("lookup_widget",(void **)&lookup_widget_f);
	get_symbol_f("mem_alloc",(void **)&mem_alloc_f);
	get_symbol_f("parse_keys",(void **)&parse_keys_f);
	get_symbol_f("queue_function",(void **)&queue_function_f);
	get_symbol_f("process_rt_vars",(void **)&process_rt_vars_f);
	get_symbol_f("read_data",(void **)&read_data_f);
	get_symbol_f("read_wrapper",(void **)&read_wrapper_f);
	get_symbol_f("recalc_table_limits",(void **)&recalc_table_limits_f);
	get_symbol_f("std_entry_handler",(void **)&std_entry_handler_f);
	get_symbol_f("set_file_api",(void **)&set_file_api_f);
	get_symbol_f("start_restore_monitor",(void **)&start_restore_monitor_f);
	get_symbol_f("thread_set_group_color",(void **)&thread_set_group_color_f);
	get_symbol_f("set_widget_labels",(void **)&set_widget_labels_f);
	get_symbol_f("set_widget_sensitive",(void **)&set_widget_sensitive_f);
	get_symbol_f("set_title",(void **)&set_title_f);
	get_symbol_f("std_entry_handler",(void **)&std_entry_handler_f);
	get_symbol_f("swap_labels",(void **)&swap_labels_f);
	get_symbol_f("table_color_refresh_wrapper",(void **)&table_color_refresh_wrapper_f);
	get_symbol_f("temp_to_ecu",(void **)&temp_to_ecu_f);
	get_symbol_f("temp_to_host",(void **)&temp_to_host_f);

	get_symbol_f("thread_refresh_widget_range",(void **)&thread_refresh_widget_range_f);
	get_symbol_f("thread_refresh_widgets_at_offset",(void **)&thread_refresh_widgets_at_offset_f);
	get_symbol_f("thread_update_logbar",(void **)&thread_update_logbar_f);
	get_symbol_f("thread_update_widget",(void **)&thread_update_widget_f);
	get_symbol_f("thread_widget_set_sensitive",(void **)&thread_widget_set_sensitive_f);
	get_symbol_f("translate_string",(void **)&translate_string_f);
	get_symbol_f("update_current_notebook_page",(void **)&update_current_notebook_page_f);
	get_symbol_f("update_entry_color",(void **)&update_entry_color_f);
	get_symbol_f("update_logbar",(void **)&update_logbar_f);
	get_symbol_f("warn_user",(void **)&warn_user_f);
	get_symbol_f("write_wrapper",(void **)&write_wrapper_f);

	register_common_enums();

	/* Packet handling queue */
	serial_reader_cond = g_new0(GCond, 1);
	g_cond_init(serial_reader_cond);
	DATA_SET(global_data,"serial_reader_cond",serial_reader_cond);
	packet_handler_cond = g_new0(GCond, 1);
	g_cond_init(packet_handler_cond);
	DATA_SET(global_data,"packet_handler_cond",packet_handler_cond);
	rtv_subscriber_mutex = g_new0(GMutex, 1);
	g_mutex_init(rtv_subscriber_mutex);
	DATA_SET(global_data,"rtv_subscriber_mutex",rtv_subscriber_mutex);
	atomic_sequence_mutex = g_new0(GMutex, 1);
	g_mutex_init(atomic_sequence_mutex);
	DATA_SET(global_data,"atomic_sequence_mutex",atomic_sequence_mutex);
	queue_mutex = g_new0(GMutex, 1);
	g_mutex_init(queue_mutex);
	DATA_SET(global_data,"queue_mutex",queue_mutex);
	thread = g_thread_new("Packet Handler Thread",packet_handler,NULL);
	DATA_SET(global_data,"packet_handler_thread",thread);
	/* Packet subscribers */
	hash =  g_hash_table_new(g_direct_hash,g_direct_equal);
	DATA_SET(global_data,"sequence_num_queue_hash",hash);
	hash = g_hash_table_new(g_direct_hash,g_direct_equal);
	DATA_SET(global_data,"payload_id_queue_hash",hash);
	queue = g_async_queue_new();
	DATA_SET(global_data,"packet_queue",queue);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_UPDATE_BLOCK_IN_RAM);
	DATA_SET(global_data,"RAM_write_queue",queue);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_REPLACE_BLOCK_IN_FLASH);
	DATA_SET(global_data,"FLASH_write_queue",queue);
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_BURN_BLOCK_FROM_RAM_TO_FLASH);
	DATA_SET(global_data,"burn_queue",queue);
	EXIT();
	return;
}


/*!
  \brief Shuts down any plugin allocated resources (Serial IO/threads/queues,
  etc) and deallocates anything that was allocated with plugin_init(), as well
  as de-registering any enums specific to this plugin. This allows the plugin
  to be safely uninstalled
  */
G_MODULE_EXPORT void plugin_shutdown()
{
	GThread *thread = NULL;
	GAsyncQueue *queue = NULL;
	GHashTable *hash = NULL;
	GCond *cond;
	GMutex *mutex;
	gint id = 0;

	ENTER();
	teardown_rtv();
	libreems_serial_disable();

	thread = (GThread *)DATA_GET(global_data,"packet_handler_thread");
	if (thread)
	{
		DATA_SET(global_data,"packet_handler_thread_exit",GINT_TO_POINTER(TRUE));
		g_thread_join(thread);
		DATA_SET(global_data,"packet_handler_thread",NULL);
		DATA_SET(global_data,"packet_handler_thread_exit",NULL);
	}
	queue = (GAsyncQueue *)DATA_GET(global_data,"burn_queue");
	if (queue)
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_BURN_BLOCK_FROM_RAM_TO_FLASH);
		g_async_queue_unref(queue);
		DATA_SET(global_data,"burn_queue",NULL);
		queue = NULL;
	}
	queue = (GAsyncQueue *)DATA_GET(global_data,"FLASH_write_queue");
	if (queue)
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_REPLACE_BLOCK_IN_FLASH);
		g_async_queue_unref(queue);
		DATA_SET(global_data,"FLASH_write_queue",NULL);
		queue = NULL;
	}
	queue = (GAsyncQueue *)DATA_GET(global_data,"RAM_write_queue");
	if (queue)
	{
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_UPDATE_BLOCK_IN_RAM);
		g_async_queue_unref(queue);
		DATA_SET(global_data,"RAM_write_queue",NULL);
		queue = NULL;
	}
	queue = (GAsyncQueue *)DATA_GET(global_data,"packet_queue");
	if (queue)
	{
		g_async_queue_unref(queue);
		DATA_SET(global_data,"packet_queue",NULL);
		queue = NULL;
	}
	hash = (GHashTable *)DATA_GET(global_data,"payload_id_queue_hash");
	if (hash)
		g_hash_table_destroy(hash);
	hash = NULL;
	DATA_SET(global_data,"payload_id_queue_hash",NULL);

	hash = (GHashTable *)DATA_GET(global_data,"sequence_num_queue_hash");
	if (hash)
		g_hash_table_destroy(hash);
	hash = NULL;
	DATA_SET(global_data,"sequence_num_queue_hash",NULL);

	cond = (GCond *)DATA_GET(global_data,"packet_handler_cond");
	if (cond)
	{
		g_cond_clear(cond);
		g_free(cond);
	}
	DATA_SET(global_data,"packet_handler_cond",NULL);
	cond = (GCond *)DATA_GET(global_data,"serial_reader_cond");
	if (cond)
	{
		g_cond_clear(cond);
		g_free(cond);
	}
	DATA_SET(global_data,"serial_reader_cond",NULL);
	mutex = (GMutex *)DATA_GET(global_data,"queue_mutex");
	if (mutex)
	{
		g_mutex_clear(mutex);
		g_free(mutex);
	}
	DATA_SET(global_data,"queue_mutex",NULL);
	mutex = (GMutex *)DATA_GET(global_data,"atomic_sequence_mutex");
	if (mutex)
	{
		g_mutex_clear(mutex);
		g_free(mutex);
	}
	DATA_SET(global_data,"atomic_sequence_mutex",NULL);
	mutex = (GMutex *)DATA_GET(global_data,"rtv_subscriber_mutex");
	if (mutex)
	{
		g_mutex_clear(mutex);
		g_free(mutex);
	}
	DATA_SET(global_data,"rtv_subscriber_mutex",NULL);
	deregister_common_enums();
	EXIT();
	return;
}


/*!
  \brief registers enumerations specific to the LibreEMS plugin
  */
void register_common_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
		/* Firmware capabilities */
		g_hash_table_insert (str_2_enum,(gpointer) "_LIBREEMS_",
				GINT_TO_POINTER (LIBREEMS));
		g_hash_table_insert (str_2_enum,(gpointer) "_COUNT_", 
				GINT_TO_POINTER (COUNT));
		g_hash_table_insert (str_2_enum,(gpointer) "_SUBMATCH_",
				GINT_TO_POINTER (SUBMATCH));
		g_hash_table_insert (str_2_enum,(gpointer) "_NUMMATCH_",
				GINT_TO_POINTER (NUMMATCH));
		g_hash_table_insert (str_2_enum,(gpointer) "_FULLMATCH_",
				GINT_TO_POINTER (FULLMATCH));
		g_hash_table_insert (str_2_enum,(gpointer) "_REGEX_", 
				GINT_TO_POINTER (REGEX));
		/* Interrogation Test Results */
		g_hash_table_insert (str_2_enum,(gpointer) "_RESULT_DATA_",
				GINT_TO_POINTER (RESULT_DATA));
		g_hash_table_insert (str_2_enum,(gpointer) "_RESULT_TEXT_",
				GINT_TO_POINTER (RESULT_TEXT));
		g_hash_table_insert (str_2_enum,(gpointer) "_RESULT_LIST_",
				GINT_TO_POINTER (RESULT_LIST));
		/* Special Common Handlers */

		/* XMLcomm processing */
		g_hash_table_insert (str_2_enum,(gpointer) "_SEQUENCE_NUM_",
				GINT_TO_POINTER(SEQUENCE_NUM));
		g_hash_table_insert (str_2_enum,(gpointer) "_PAYLOAD_ID_",
				GINT_TO_POINTER(PAYLOAD_ID));
		g_hash_table_insert (str_2_enum,(gpointer) "_LOCATION_ID_",
				GINT_TO_POINTER(LOCATION_ID));
		g_hash_table_insert (str_2_enum,(gpointer) "_OFFSET_",
				GINT_TO_POINTER(OFFSET));
		g_hash_table_insert (str_2_enum,(gpointer) "_LENGTH_",
				GINT_TO_POINTER(LENGTH));
		/* NOT USED
		g_hash_table_insert (str_2_enum,(gpointer) "_DATABYTE_",
				GINT_TO_POINTER(DATABYTE));
				*/
		g_hash_table_insert (str_2_enum,(gpointer) "_PAYLOAD_DATA_",
				GINT_TO_POINTER(PAYLOAD_DATA));
		g_hash_table_insert (str_2_enum,(gpointer) "_LIBREEMS_ALL_",
				GINT_TO_POINTER(LIBREEMS_ALL));
		g_hash_table_insert (str_2_enum,(gpointer) "_CALLBACK_",
				GINT_TO_POINTER(CALLBACK));
		g_hash_table_insert (str_2_enum,(gpointer) "_EMPTY_PAYLOAD_",
				GINT_TO_POINTER(EMPTY_PAYLOAD));
		g_hash_table_insert (str_2_enum,(gpointer) "_GENERIC_READ_",
				GINT_TO_POINTER(GENERIC_READ));
		g_hash_table_insert (str_2_enum,(gpointer) "_GENERIC_FLASH_WRITE_",
				GINT_TO_POINTER(GENERIC_FLASH_WRITE));
		g_hash_table_insert (str_2_enum,(gpointer) "_GENERIC_RAM_WRITE_",
				GINT_TO_POINTER(GENERIC_RAM_WRITE));
		g_hash_table_insert (str_2_enum,(gpointer) "_GENERIC_BURN_",
				GINT_TO_POINTER(GENERIC_BURN));
		g_hash_table_insert (str_2_enum,(gpointer) "_BENCHTEST_RESPONSE_",
				GINT_TO_POINTER(BENCHTEST_RESPONSE));
		/* Firmware Specific button handlers*/
		g_hash_table_insert (str_2_enum,(gpointer) "_SOFT_BOOT_ECU_",
				GINT_TO_POINTER(SOFT_BOOT_ECU));
		g_hash_table_insert (str_2_enum,(gpointer) "_HARD_BOOT_ECU_",
				GINT_TO_POINTER(HARD_BOOT_ECU));
		g_hash_table_insert (str_2_enum,(gpointer) "_BENCHTEST_START_",
				GINT_TO_POINTER(BENCHTEST_START));
		g_hash_table_insert (str_2_enum,(gpointer) "_BENCHTEST_STOP_",
				GINT_TO_POINTER(BENCHTEST_STOP));
		g_hash_table_insert (str_2_enum,(gpointer) "_BENCHTEST_BUMP_",
				GINT_TO_POINTER(BENCHTEST_BUMP));
		g_hash_table_insert (str_2_enum,(gpointer) "_NOOP_",
				GINT_TO_POINTER(NOOP));
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
	EXIT();
	return;
}


/*!
  \brief De-registers enumerations specific to the LibreEMS plugin
  */
void deregister_common_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
		/* Firmware capabilities */
		g_hash_table_remove (str_2_enum, "_LIBREEMS_");
		g_hash_table_remove (str_2_enum, "_COUNT_");
		g_hash_table_remove (str_2_enum, "_SUBMATCH_");
		g_hash_table_remove (str_2_enum, "_NUMMATCH_");
		g_hash_table_remove (str_2_enum, "_FULLMATCH_");
		g_hash_table_remove (str_2_enum, "_REGEX_");
		/* Interrogation Test Results */
		g_hash_table_remove (str_2_enum, "_RESULT_DATA_");
		g_hash_table_remove (str_2_enum, "_RESULT_TEXT_");
		g_hash_table_remove (str_2_enum, "_RESULT_LIST_");
		/* Special Common Handlers */

		/* XMLcomm processing */
		g_hash_table_remove (str_2_enum, "_SEQUENCE_NUM_");
		g_hash_table_remove (str_2_enum, "_PAYLOAD_ID_");
		g_hash_table_remove (str_2_enum, "_LOCATION_ID_");
		g_hash_table_remove (str_2_enum, "_OFFSET_");
		g_hash_table_remove (str_2_enum, "_LENGTH_");
		/* NOT USED
		g_hash_table_remove (str_2_enum, "_DATABYTE_");
		*/
		g_hash_table_remove (str_2_enum, "_CALLBACK_");
		g_hash_table_remove (str_2_enum, "_EMPTY_PAYLOAD_");
		g_hash_table_remove (str_2_enum, "_LIBREEMS_ALL_");
		g_hash_table_remove (str_2_enum, "_GENERIC_READ_");
		g_hash_table_remove (str_2_enum, "_GENERIC_RAM_WRITE_");
		g_hash_table_remove (str_2_enum, "_GENERIC_FLASH_WRITE_");
		g_hash_table_remove (str_2_enum, "_GENERIC_BURN_");
		g_hash_table_remove (str_2_enum, "_BENCHTEST_RESPONSE_");
		g_hash_table_remove (str_2_enum, "_PAYLOAD_DATA_");
		/* Firmware Specific button handlers*/
		g_hash_table_remove (str_2_enum, "_SOFT_BOOT_ECU_");
		g_hash_table_remove (str_2_enum, "_HARD_BOOT_ECU_");
		g_hash_table_remove (str_2_enum, "_BENCHTEST_START_");
		g_hash_table_remove (str_2_enum, "_BENCHTEST_STOP_");
		g_hash_table_remove (str_2_enum, "_BENCHTEST_BUMP_");
		g_hash_table_remove (str_2_enum, "_NOOP_");
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
	EXIT();
	return;
}
