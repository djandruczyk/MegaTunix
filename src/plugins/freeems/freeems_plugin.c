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

#define __FREEEMS_PLUGIN_C__
#include <config.h>
#include <defines.h>
#include <firmware.h>
#include <freeems_comms.h>
#include <freeems_gui_handlers.h>
#include <freeems_plugin.h>
#include <gtk/gtk.h>
#include <interrogate.h>
#include <packet_handlers.h>


gconstpointer *global_data = NULL;


G_MODULE_EXPORT void plugin_init(gconstpointer *data)
{
	global_data = data;
	GAsyncQueue *queue = NULL;
	GCond *cond = NULL;
	GThread *thread = NULL;
	GHashTable *hash = NULL;
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	error_msg_f = (void *)DATA_GET(global_data,"error_msg_f");
	g_assert(error_msg_f);
	get_symbol_f = (void *)DATA_GET(global_data,"get_symbol_f");
	g_assert(get_symbol_f);
	get_symbol_f("_get_sized_data",(void *)&_get_sized_data_f);
	get_symbol_f("_set_sized_data",(void *)&_set_sized_data_f);
	get_symbol_f("dbg_func",(void *)&dbg_func_f);
	get_symbol_f("dump_output",(void *)&dump_output_f);
	get_symbol_f("flush_serial",(void *)&flush_serial_f);
	get_symbol_f("initialize_outputdata",(void *)&initialize_outputdata_f);
	get_symbol_f("queue_function",(void *)&queue_function_f);
	get_symbol_f("read_data",(void *)&read_data_f);
	get_symbol_f("read_wrapper",(void *)&read_wrapper_f);
	get_symbol_f("thread_update_logbar",(void *)&thread_update_logbar_f);
	get_symbol_f("thread_update_widget",(void *)&thread_update_widget_f);
	get_symbol_f("update_logbar",(void *)&update_logbar_f);
	get_symbol_f("warn_user",(void *)&warn_user_f);
	get_symbol_f("write_wrapper",(void *)&write_wrapper_f);
	register_common_enums();

	/* Packet handling queue */
	queue = g_async_queue_new();
	DATA_SET(global_data,"packet_queue",queue);
	cond = g_cond_new();
	DATA_SET(global_data,"serial_reader_cond",cond);
	cond = g_cond_new();
	DATA_SET(global_data,"packet_handler_cond",cond);
	thread = g_thread_create(packet_handler,NULL,TRUE,NULL);
	DATA_SET(global_data,"packet_handler_thread",thread);
	/* Packet subscribers */
	hash =  g_hash_table_new(g_direct_hash,g_direct_equal);
	DATA_SET(global_data,"sequence_num_queue_hash",hash);
	hash = g_hash_table_new(g_direct_hash,g_direct_equal);
	DATA_SET(global_data,"payload_id_queue_hash",hash);
}


G_MODULE_EXPORT void plugin_shutdown()
{
	GCond *cond = NULL;
	GHashTable *hash = NULL;

	freeems_serial_disable();
	/*
	cond = DATA_GET(global_data,"serial_reader_cond");
	if (cond)
		g_cond_free(cond);
		*/
	hash = DATA_GET(global_data,"sequence_num_queue_hash");
	if (hash)
		g_hash_table_destroy(hash);
	hash = DATA_GET(global_data,"payload_id_queue_hash");
	if (hash)
		g_hash_table_destroy(hash);
	return;
}


void register_common_enums(void)
{
	GHashTable *str_2_enum = NULL;

	str_2_enum = DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
		/* Firmware capabilities */
		g_hash_table_insert (str_2_enum, "_FREEEMS_",
				GINT_TO_POINTER (FREEEMS));
		g_hash_table_insert (str_2_enum, "_COUNT_", 
				GINT_TO_POINTER (COUNT));
		g_hash_table_insert (str_2_enum, "_SUBMATCH_",
				GINT_TO_POINTER (SUBMATCH));
		g_hash_table_insert (str_2_enum, "_NUMMATCH_",
				GINT_TO_POINTER (NUMMATCH));
		g_hash_table_insert (str_2_enum, "_FULLMATCH_",
				GINT_TO_POINTER (FULLMATCH));
		g_hash_table_insert (str_2_enum, "_REGEX_", 
				GINT_TO_POINTER (REGEX));
		/* Interrogation Test Results */
		g_hash_table_insert (str_2_enum, "_RESULT_DATA_",
				GINT_TO_POINTER (RESULT_DATA));
		g_hash_table_insert (str_2_enum, "_RESULT_TEXT_",
				GINT_TO_POINTER (RESULT_TEXT));
		/* Special Common Handlers */

		/* XMLcomm processing */
		g_hash_table_insert(str_2_enum,"_SEQUENCE_NUM_",
				GINT_TO_POINTER(SEQUENCE_NUM));
		g_hash_table_insert(str_2_enum,"_PAYLOAD_ID_",
				GINT_TO_POINTER(PAYLOAD_ID));
		g_hash_table_insert(str_2_enum,"_LOCATION_ID_",
				GINT_TO_POINTER(LOCATION_ID));
		g_hash_table_insert(str_2_enum,"_OFFSET_",
				GINT_TO_POINTER(OFFSET));
		g_hash_table_insert(str_2_enum,"_LENGTH_",
				GINT_TO_POINTER(LENGTH));
		g_hash_table_insert(str_2_enum,"_DATABYTE_",
				GINT_TO_POINTER(DATABYTE));
		/* Firmware Specific button handlers*/
		g_hash_table_insert(str_2_enum,"_WARM_BOOT_ECU_",
				GINT_TO_POINTER(WARM_BOOT_ECU));
		g_hash_table_insert(str_2_enum,"_STOP_REALTIME_",
				GINT_TO_POINTER(STOP_REALTIME));
		g_hash_table_insert(str_2_enum,"_START_REALTIME_",
				GINT_TO_POINTER(START_REALTIME));
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
}

