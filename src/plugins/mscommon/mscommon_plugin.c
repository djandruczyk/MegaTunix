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
  \file src/plugins/mscommon/mscommon_plugin.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS common plugin init/shutdown functions
  \author David Andruczyk
  */

#define __MSCOMMON_PLUGIN_C__
#include <config.h>
#include <defines.h>
#include <mscommon_plugin.h>
#include <mscommon_helpers.h>
#include <mscommon_gui_handlers.h>
#include <interrogate.h>
#include <mtxsocket.h>
#include <gtk/gtk.h>

gconstpointer *global_data = NULL;

/*!
  \brief initializes the MegaSquirt plugin via locating and attaching to all
  needed functions within the core MegaTunix. It also registers any 
  plugin specific enumerations, and registers/sets up any additional resources
  needed like threads, Queues, memory, etc
  \param data is a pointer to the global data container.
  */
G_MODULE_EXPORT void plugin_init(gconstpointer *data)
{
	global_data = data;
	*(void **)(&error_msg_f) = (void **)DATA_GET(global_data,"error_msg_f");
	g_assert(error_msg_f);
	*(void **)(&get_symbol_f) = (void **)DATA_GET(global_data,"get_symbol_f");
	g_assert(get_symbol_f);
	get_symbol_f("dbg_func",(void **)&dbg_func_f);
	ENTER();
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	get_symbol_f("_get_sized_data",(void **)&_get_sized_data_f);
	get_symbol_f("_set_sized_data",(void **)&_set_sized_data_f);
	get_symbol_f("add_additional_rtt",(void **)&add_additional_rtt_f);
	get_symbol_f("alter_widget_state",(void **)&alter_widget_state_f);
	get_symbol_f("bind_to_lists",(void **)&bind_to_lists_f);
	get_symbol_f("c_to_k",(void **)&c_to_k_f);
	get_symbol_f("check_tab_existance",(void **)&check_tab_existance_f);
	get_symbol_f("cleanup",(void **)&cleanup_f);
	get_symbol_f("combo_set_labels",(void **)&combo_set_labels_f);
	get_symbol_f("combo_toggle_groups_linked",(void **)&combo_toggle_groups_linked_f);
	get_symbol_f("combo_toggle_labels_linked",(void **)&combo_toggle_labels_linked_f);
	get_symbol_f("convert_after_upload",(void **)&convert_after_upload_f);
	get_symbol_f("convert_before_download",(void **)&convert_before_download_f);
	get_symbol_f("create_rtv_value_change_watch",(void **)&create_rtv_value_change_watch_f);
	get_symbol_f("direct_lookup_data",(void **)&direct_lookup_data_f);
	get_symbol_f("direct_reverse_lookup",(void **)&direct_reverse_lookup_f);
	get_symbol_f("entry_changed_handler",(void **)&entry_changed_handler_f);
	get_symbol_f("evaluator_create_w",(void **)&evaluator_create_f);
	get_symbol_f("evaluator_destroy_w",(void **)&evaluator_destroy_f);
	get_symbol_f("evaluator_evaluate_x_w",(void **)&evaluator_evaluate_x_f);
	get_symbol_f("f_to_k",(void **)&f_to_k_f);
	get_symbol_f("flush_serial",(void **)&flush_serial_f);
	get_symbol_f("focus_out_handler",(void **)&focus_out_handler_f);
	get_symbol_f("get_bitshift",(void **)&get_bitshift_f);
	get_symbol_f("get_choice_count",(void **)&get_choice_count_f);
	get_symbol_f("get_colors_from_hue",(void **)&get_colors_from_hue_f);
	get_symbol_f("get_extreme_from_size",(void **)&get_extreme_from_size_f);
	get_symbol_f("get_file_api",(void **)&get_file_api_f);
	get_symbol_f("get_list",(void **)&get_list_f);
	get_symbol_f("get_multiplier",(void **)&get_multiplier_f);
	get_symbol_f("get_table",(void **)&get_table_f);
	get_symbol_f("io_cmd",(void **)&io_cmd_f);
	get_symbol_f("initialize_outputdata",(void **)&initialize_outputdata_f);
	get_symbol_f("jump_to_tab",(void **)&jump_to_tab_f);
	get_symbol_f("key_event",(void **)&key_event_f);
	get_symbol_f("lookup_current_value",(void **)&lookup_current_value_f);
	get_symbol_f("lookup_data",(void **)&lookup_data_f);
	get_symbol_f("lookup_precision",(void **)&lookup_precision_f);
	get_symbol_f("lookup_widget",(void **)&lookup_widget_f);
	get_symbol_f("mem_alloc",(void **)&mem_alloc_f);
	get_symbol_f("parse_keys",(void **)&parse_keys_f);
	get_symbol_f("process_rt_vars",(void **)&process_rt_vars_f);
	get_symbol_f("queue_function",(void **)&queue_function_f);
	get_symbol_f("read_data",(void **)&read_data_f);
	get_symbol_f("read_wrapper",(void **)&read_wrapper_f);
	get_symbol_f("recalc_table_limits",(void **)&recalc_table_limits_f);
	get_symbol_f("register_widget",(void **)&register_widget_f);
	get_symbol_f("remove_rtv_watch",(void **)&remove_rtv_watch_f);
	get_symbol_f("remove_from_lists",(void **)&remove_from_lists_f);
	get_symbol_f("reverse_lookup",(void **)&reverse_lookup_f);
	get_symbol_f("search_model",(void **)&search_model_f);
	get_symbol_f("set_file_api",(void **)&set_file_api_f);
	get_symbol_f("table_color_refresh_wrapper",(void **)&table_color_refresh_wrapper_f);
	get_symbol_f("thread_set_group_color",(void **)&thread_set_group_color_f);
	get_symbol_f("set_reqfuel_color",(void **)&set_reqfuel_color_f);
	get_symbol_f("set_title",(void **)&set_title_f);
	get_symbol_f("set_widget_labels",(void **)&set_widget_labels_f);
	get_symbol_f("set_widget_sensitive",(void **)&set_widget_sensitive_f);
	get_symbol_f("spin_button_handler",(void **)&spin_button_handler_f);
	get_symbol_f("start_restore_monitor",(void **)&start_restore_monitor_f);
	get_symbol_f("start_tickler",(void **)&start_tickler_f);
	get_symbol_f("std_entry_handler",(void **)&std_entry_handler_f);
	get_symbol_f("stop_tickler",(void **)&stop_tickler_f);
	get_symbol_f("swap_labels",(void **)&swap_labels_f);
	get_symbol_f("temp_to_ecu",(void **)&temp_to_ecu_f);
	get_symbol_f("temp_to_host",(void **)&temp_to_host_f);
	get_symbol_f("thread_refresh_widget",(void **)&thread_refresh_widget_f);
	get_symbol_f("thread_refresh_widget_range",(void **)&thread_refresh_widget_range_f);
	get_symbol_f("thread_refresh_widgets_at_offset",(void **)&thread_refresh_widgets_at_offset_f);
	get_symbol_f("thread_update_logbar",(void **)&thread_update_logbar_f);
	get_symbol_f("thread_update_widget",(void **)&thread_update_widget_f);
	get_symbol_f("thread_widget_set_sensitive",(void **)&thread_widget_set_sensitive_f);
	get_symbol_f("translate_string",(void **)&translate_string_f);
	get_symbol_f("update_current_notebook_page",(void **)&update_current_notebook_page_f);
	get_symbol_f("update_entry_color",(void **)&update_entry_color_f);
	get_symbol_f("update_logbar",(void **)&update_logbar_f);
	get_symbol_f("update_ve3d_if_necessary",(void **)&update_ve3d_if_necessary_f);
	get_symbol_f("warn_user",(void **)&warn_user_f);
	get_symbol_f("write_wrapper",(void **)&write_wrapper_f);

	register_common_enums();
	EXIT();
	return;
}


/*!
  \brief Frees up any resources of the plugin
  */
G_MODULE_EXPORT void plugin_shutdown()
{
	ENTER();
	deregister_common_enums();
	EXIT();
	return;
}


/*!
  \brief Registers enumerations common to this plugin
  */
void register_common_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
		/* TCP Socket Commands */
		g_hash_table_insert (str_2_enum, (gpointer)"HELP", GINT_TO_POINTER (HELP));
		g_hash_table_insert (str_2_enum, (gpointer)"QUIT", GINT_TO_POINTER (QUIT));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_REVISION",
				GINT_TO_POINTER (GET_REVISION));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_SIGNATURE",
				GINT_TO_POINTER (GET_SIGNATURE));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_RT_VARS",
				GINT_TO_POINTER (GET_RT_VARS));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_RTV_LIST",
				GINT_TO_POINTER (GET_RTV_LIST));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_ECU_PAGE",
				GINT_TO_POINTER (GET_ECU_PAGE));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_ECU_VAR_U08",
				GINT_TO_POINTER (GET_ECU_VAR_U08));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_ECU_VAR_S08",
				GINT_TO_POINTER (GET_ECU_VAR_S08));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_ECU_VAR_U16",
				GINT_TO_POINTER (GET_ECU_VAR_U16));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_ECU_VAR_S16",
				GINT_TO_POINTER (GET_ECU_VAR_S16));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_ECU_VAR_U32",
				GINT_TO_POINTER (GET_ECU_VAR_U32));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_ECU_VAR_S32",
				GINT_TO_POINTER (GET_ECU_VAR_S32));
		g_hash_table_insert (str_2_enum, (gpointer)"SET_ECU_VAR_U08",
				GINT_TO_POINTER (SET_ECU_VAR_U08));
		g_hash_table_insert (str_2_enum, (gpointer)"SET_ECU_VAR_S08",
				GINT_TO_POINTER (SET_ECU_VAR_S08));
		g_hash_table_insert (str_2_enum, (gpointer)"SET_ECU_VAR_U16",
				GINT_TO_POINTER (SET_ECU_VAR_U16));
		g_hash_table_insert (str_2_enum, (gpointer)"SET_ECU_VAR_S16",
				GINT_TO_POINTER (SET_ECU_VAR_S16));
		g_hash_table_insert (str_2_enum, (gpointer)"SET_ECU_VAR_U32",
				GINT_TO_POINTER (SET_ECU_VAR_U32));
		g_hash_table_insert (str_2_enum, (gpointer)"SET_ECU_VAR_S32",
				GINT_TO_POINTER (SET_ECU_VAR_S32));
		g_hash_table_insert (str_2_enum, (gpointer)"BURN_FLASH",
				GINT_TO_POINTER (BURN_FLASH));
		g_hash_table_insert (str_2_enum, (gpointer)"GET_RAW_ECU",
				GINT_TO_POINTER (GET_RAW_ECU));
		g_hash_table_insert (str_2_enum, (gpointer)"SET_RAW_ECU",
				GINT_TO_POINTER (SET_RAW_ECU));
		/* Firmware capabilities */
		g_hash_table_insert (str_2_enum, (gpointer)"_PIS_", 
				GINT_TO_POINTER (PIS));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS1_", 
				GINT_TO_POINTER (MS1));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS1_STD_",
				GINT_TO_POINTER (MS1_STD));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS1_DT_", 
				GINT_TO_POINTER (MS1_DT));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS1_E_", 
				GINT_TO_POINTER (MS1_E));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS2_", 
				GINT_TO_POINTER (MS2));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS2_STD_",
				GINT_TO_POINTER (MS2_STD));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS2_E_", 
				GINT_TO_POINTER (MS2_E));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS2_E_COMPMON_",
				GINT_TO_POINTER (MS2_E_COMPMON));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS3_",
				GINT_TO_POINTER (MS3));
		g_hash_table_insert (str_2_enum, (gpointer)"_MS3_NEWSERIAL_",
				GINT_TO_POINTER (MS3_NEWSERIAL));
		g_hash_table_insert (str_2_enum, (gpointer)"_JIMSTIM_",
				GINT_TO_POINTER (JIMSTIM));
		g_hash_table_insert (str_2_enum, (gpointer)"_COUNT_", 
				GINT_TO_POINTER (COUNT));
		g_hash_table_insert (str_2_enum, (gpointer)"_SUBMATCH_",
				GINT_TO_POINTER (SUBMATCH));
		g_hash_table_insert (str_2_enum, (gpointer)"_NUMMATCH_",
				GINT_TO_POINTER (NUMMATCH));
		g_hash_table_insert (str_2_enum, (gpointer)"_FULLMATCH_",
				GINT_TO_POINTER (FULLMATCH));
		g_hash_table_insert (str_2_enum, (gpointer)"_REGEX_", 
				GINT_TO_POINTER (REGEX));
		/* Interrogation Test Results */
		g_hash_table_insert (str_2_enum, (gpointer)"_RESULT_DATA_",
				GINT_TO_POINTER (RESULT_DATA));
		g_hash_table_insert (str_2_enum, (gpointer)"_RESULT_TEXT_",
				GINT_TO_POINTER (RESULT_TEXT));
		/* Common Handlers */
		g_hash_table_insert (str_2_enum, (gpointer)"_NUM_SQUIRTS_1_",
				GINT_TO_POINTER (NUM_SQUIRTS_1));
		g_hash_table_insert (str_2_enum, (gpointer)"_NUM_SQUIRTS_2_",
				GINT_TO_POINTER (NUM_SQUIRTS_2));
		g_hash_table_insert (str_2_enum, (gpointer)"_NUM_CYLINDERS_1_",
				GINT_TO_POINTER (NUM_CYLINDERS_1));
		g_hash_table_insert (str_2_enum, (gpointer)"_NUM_CYLINDERS_2_",
				GINT_TO_POINTER (NUM_CYLINDERS_2));
		g_hash_table_insert (str_2_enum, (gpointer)"_NUM_INJECTORS_1_",
				GINT_TO_POINTER (NUM_INJECTORS_1));
		g_hash_table_insert (str_2_enum, (gpointer)"_NUM_INJECTORS_2_",
				GINT_TO_POINTER (NUM_INJECTORS_2));
		g_hash_table_insert (str_2_enum, (gpointer)"_LOCKED_REQ_FUEL_",
				GINT_TO_POINTER(LOCKED_REQ_FUEL));
		g_hash_table_insert (str_2_enum, (gpointer)"_REQ_FUEL_1_",
				GINT_TO_POINTER(REQ_FUEL_1));
		g_hash_table_insert (str_2_enum, (gpointer)"_REQ_FUEL_2_",
				GINT_TO_POINTER(REQ_FUEL_2));
		g_hash_table_insert (str_2_enum, (gpointer)"_MULTI_EXPRESSION_",
				GINT_TO_POINTER(MULTI_EXPRESSION));
		g_hash_table_insert (str_2_enum, (gpointer)"_ALT_SIMUL_",
				GINT_TO_POINTER(ALT_SIMUL));
		g_hash_table_insert (str_2_enum, (gpointer)"_INCREMENT_VALUE_",
				GINT_TO_POINTER(INCREMENT_VALUE));
		g_hash_table_insert (str_2_enum, (gpointer)"_DECREMENT_VALUE_",
				GINT_TO_POINTER(DECREMENT_VALUE));
		g_hash_table_insert (str_2_enum, (gpointer)"_REQFUEL_RESCALE_TABLE_",
				GINT_TO_POINTER(REQFUEL_RESCALE_TABLE));
		g_hash_table_insert (str_2_enum, (gpointer)"_REQ_FUEL_POPUP_",
				GINT_TO_POINTER(REQ_FUEL_POPUP));
		/* XmlCmdType's */
		g_hash_table_insert(str_2_enum,(gpointer)"_WRITE_VERIFY_",
				GINT_TO_POINTER(WRITE_VERIFY));
		g_hash_table_insert(str_2_enum,(gpointer)"_MISMATCH_COUNT_",
				GINT_TO_POINTER(MISMATCH_COUNT));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS1_CLOCK_",
				GINT_TO_POINTER(MS1_CLOCK));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS2_CLOCK_",
				GINT_TO_POINTER(MS2_CLOCK));
		g_hash_table_insert(str_2_enum,(gpointer)"_NUM_REV_",
				GINT_TO_POINTER(NUM_REV));
		g_hash_table_insert(str_2_enum,(gpointer)"_TEXT_REV_",
				GINT_TO_POINTER(TEXT_REV));
		g_hash_table_insert(str_2_enum,(gpointer)"_SIGNATURE_",
				GINT_TO_POINTER(SIGNATURE));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS1_VECONST_",
				GINT_TO_POINTER(MS1_VECONST));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS2_VECONST_",
				GINT_TO_POINTER(MS2_VECONST));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS2_BOOTLOADER_",
				GINT_TO_POINTER(MS2_BOOTLOADER));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS1_RT_VARS_",
				GINT_TO_POINTER(MS1_RT_VARS));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS2_RT_VARS_",
				GINT_TO_POINTER(MS2_RT_VARS));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS1_GETERROR_",
				GINT_TO_POINTER(MS1_GETERROR));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS1_E_TRIGMON_",
				GINT_TO_POINTER(MS1_E_TRIGMON));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS1_E_TOOTHMON_",
				GINT_TO_POINTER(MS1_E_TOOTHMON));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS2_E_TRIGMON_",
				GINT_TO_POINTER(MS2_E_TRIGMON));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS2_E_TOOTHMON_",
				GINT_TO_POINTER(MS2_E_TOOTHMON));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS2_E_COMPOSITEMON_",
				GINT_TO_POINTER(MS2_E_COMPOSITEMON));
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
	EXIT();
	return;
}


/*!
  \brief De-registers enumerations common to this plugin
  */
void deregister_common_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
		/* TCP Socket Commands */
		g_hash_table_remove (str_2_enum, "HELP");
		g_hash_table_remove (str_2_enum, "QUIT");
		g_hash_table_remove (str_2_enum, "GET_REVISION");
		g_hash_table_remove (str_2_enum, "GET_SIGNATURE");
		g_hash_table_remove (str_2_enum, "GET_RT_VARS");
		g_hash_table_remove (str_2_enum, "GET_RTV_LIST");
		g_hash_table_remove (str_2_enum, "GET_ECU_PAGE");
		g_hash_table_remove (str_2_enum, "GET_ECU_VAR_U08");
		g_hash_table_remove (str_2_enum, "GET_ECU_VAR_S08");
		g_hash_table_remove (str_2_enum, "GET_ECU_VAR_U16");
		g_hash_table_remove (str_2_enum, "GET_ECU_VAR_S16");
		g_hash_table_remove (str_2_enum, "GET_ECU_VAR_U32");
		g_hash_table_remove (str_2_enum, "GET_ECU_VAR_S32");
		g_hash_table_remove (str_2_enum, "SET_ECU_VAR_U08");
		g_hash_table_remove (str_2_enum, "SET_ECU_VAR_S08");
		g_hash_table_remove (str_2_enum, "SET_ECU_VAR_U16");
		g_hash_table_remove (str_2_enum, "SET_ECU_VAR_S16");
		g_hash_table_remove (str_2_enum, "SET_ECU_VAR_U32");
		g_hash_table_remove (str_2_enum, "SET_ECU_VAR_S32");
		g_hash_table_remove (str_2_enum, "BURN_FLASH");
		g_hash_table_remove (str_2_enum, "GET_RAW_ECU");
		g_hash_table_remove (str_2_enum, "SET_RAW_ECU");
		/* Firmware capabilities */
		g_hash_table_remove (str_2_enum, "_PIS_");
		g_hash_table_remove (str_2_enum, "_MS1_"); 
		g_hash_table_remove (str_2_enum, "_MS1_STD_");
		g_hash_table_remove (str_2_enum, "_MS1_DT_"); 
		g_hash_table_remove (str_2_enum, "_MS1_E_"); 
		g_hash_table_remove (str_2_enum, "_MS2_"); 
		g_hash_table_remove (str_2_enum, "_MS2_STD_");
		g_hash_table_remove (str_2_enum, "_MS2_E_"); 
		g_hash_table_remove (str_2_enum, "_MS2_E_COMPMON_");
		g_hash_table_remove (str_2_enum, "_JIMSTIM_");
		g_hash_table_remove (str_2_enum, "_COUNT_"); 
		g_hash_table_remove (str_2_enum, "_SUBMATCH_");
		g_hash_table_remove (str_2_enum, "_NUMMATCH_");
		g_hash_table_remove (str_2_enum, "_FULLMATCH_");
		g_hash_table_remove (str_2_enum, "_REGEX_"); 
		/* Interrogation Test Results */
		g_hash_table_remove (str_2_enum, "_RESULT_DATA_");
		g_hash_table_remove (str_2_enum, "_RESULT_TEXT_");
		/* Common Handlers */
		g_hash_table_remove (str_2_enum, "_NUM_SQUIRTS_1_");
		g_hash_table_remove (str_2_enum, "_NUM_SQUIRTS_2_");
		g_hash_table_remove (str_2_enum, "_NUM_CYLINDERS_1_");
		g_hash_table_remove (str_2_enum, "_NUM_CYLINDERS_2_");
		g_hash_table_remove (str_2_enum, "_NUM_INJECTORS_1_");
		g_hash_table_remove (str_2_enum, "_NUM_INJECTORS_2_");
		g_hash_table_remove (str_2_enum, "_LOCKED_REQ_FUEL_");
		g_hash_table_remove (str_2_enum, "_REQ_FUEL_1_");
		g_hash_table_remove (str_2_enum, "_REQ_FUEL_2_");
		g_hash_table_remove (str_2_enum, "_MULTI_EXPRESSION_");
		g_hash_table_remove (str_2_enum, "_ALT_SIMUL_");
		g_hash_table_remove (str_2_enum, "_INCREMENT_VALUE_");
		g_hash_table_remove (str_2_enum, "_DECREMENT_VALUE_");
		g_hash_table_remove (str_2_enum, "_REQFUEL_RESCALE_TABLE_");
		g_hash_table_remove (str_2_enum, "_REQ_FUEL_POPUP_");
		/* XmlCmdType's */
		g_hash_table_remove (str_2_enum, "_WRITE_VERIFY_");
		g_hash_table_remove (str_2_enum, "_MISMATCH_COUNT_");
		g_hash_table_remove (str_2_enum, "_MS1_CLOCK_");
		g_hash_table_remove (str_2_enum, "_MS2_CLOCK_");
		g_hash_table_remove (str_2_enum, "_NUM_REV_");
		g_hash_table_remove (str_2_enum, "_TEXT_REV_");
		g_hash_table_remove (str_2_enum, "_SIGNATURE_");
		g_hash_table_remove (str_2_enum, "_MS1_VECONST_");
		g_hash_table_remove (str_2_enum, "_MS2_VECONST_");
		g_hash_table_remove (str_2_enum, "_MS2_BOOTLOADER_");
		g_hash_table_remove (str_2_enum, "_MS1_RT_VARS_");
		g_hash_table_remove (str_2_enum, "_MS2_RT_VARS_");
		g_hash_table_remove (str_2_enum, "_MS1_GETERROR_");
		g_hash_table_remove (str_2_enum, "_MS1_E_TRIGMON_");
		g_hash_table_remove (str_2_enum, "_MS1_E_TOOTHMON_");
		g_hash_table_remove (str_2_enum, "_MS2_E_TRIGMON_");
		g_hash_table_remove (str_2_enum, "_MS2_E_TOOTHMON_");
		g_hash_table_remove (str_2_enum, "_MS2_E_COMPOSITEMON_");
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
	EXIT();
	return;
}
