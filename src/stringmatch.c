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

#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <interrogate.h>
#include <mtxsocket.h>
#include <stringmatch.h>


static GHashTable *str_2_enum = NULL;


/*!
 \brief build_string_2_enum_table() constructs a hashtable that maps a textual
 name to it's matching enumeration.  It's used for mapping things from all the 
 configuration files so that things just plain look better
 */
void build_string_2_enum_table(void)
{
	extern gconstpointer *global_data;
	str_2_enum = g_hash_table_new_full(g_str_hash,g_str_equal,NULL,NULL);

	/* Firmware capabilities */
	g_hash_table_insert(str_2_enum,"_PIS_",
			GINT_TO_POINTER(PIS));
	g_hash_table_insert(str_2_enum,"_MS1_",
			GINT_TO_POINTER(MS1));
	g_hash_table_insert(str_2_enum,"_MS1_STD_",
			GINT_TO_POINTER(MS1_STD));
	g_hash_table_insert(str_2_enum,"_MS1_DT_",
			GINT_TO_POINTER(MS1_DT));
	g_hash_table_insert(str_2_enum,"_MSNS_E_",
			GINT_TO_POINTER(MSNS_E));
	g_hash_table_insert(str_2_enum,"_MS2_",
			GINT_TO_POINTER(MS2));
	g_hash_table_insert(str_2_enum,"_MS2_STD_",
			GINT_TO_POINTER(MS2_STD));
	g_hash_table_insert(str_2_enum,"_MS2_E_",
			GINT_TO_POINTER(MS2_E));
	g_hash_table_insert(str_2_enum,"_MS2_E_COMPMON_",
			GINT_TO_POINTER(MS2_E_COMPMON));
	g_hash_table_insert(str_2_enum,"_SECU_3_",
			GINT_TO_POINTER(SECU_3));
	g_hash_table_insert(str_2_enum,"_FREEEMS_",
			GINT_TO_POINTER(FREEEMS));
	g_hash_table_insert(str_2_enum,"_JIMSTIM_",
			GINT_TO_POINTER(JIMSTIM));

	/* Interrogation field types */
	g_hash_table_insert(str_2_enum,"_CHAR_",
			GINT_TO_POINTER(MTX_CHAR));
	g_hash_table_insert(str_2_enum,"_U08_",
			GINT_TO_POINTER(MTX_U08));
	g_hash_table_insert(str_2_enum,"_S08_",
			GINT_TO_POINTER(MTX_S08));
	g_hash_table_insert(str_2_enum,"_U16_",
			GINT_TO_POINTER(MTX_U16));
	g_hash_table_insert(str_2_enum,"_S16_",
			GINT_TO_POINTER(MTX_S16));
	g_hash_table_insert(str_2_enum,"_U32_",
			GINT_TO_POINTER(MTX_U32));
	g_hash_table_insert(str_2_enum,"_S32_",
			GINT_TO_POINTER(MTX_S32));
	g_hash_table_insert(str_2_enum,"_UNDEF_",
			GINT_TO_POINTER(MTX_UNDEF));
	g_hash_table_insert(str_2_enum,"_COUNT_",
			GINT_TO_POINTER(COUNT));
	g_hash_table_insert(str_2_enum,"_SUBMATCH_",
			GINT_TO_POINTER(SUBMATCH));
	g_hash_table_insert(str_2_enum,"_NUMMATCH_",
			GINT_TO_POINTER(NUMMATCH));
	g_hash_table_insert(str_2_enum,"_FULLMATCH_",
			GINT_TO_POINTER(FULLMATCH));
	g_hash_table_insert(str_2_enum,"_REGEX_",
			GINT_TO_POINTER(REGEX));

	/* Data Types for glade data binder.... */
	g_hash_table_insert(str_2_enum,"_INT_",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"_ENUM_",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,"_STRING_",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"_BOOL_",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,"_FLOAT_",
			GINT_TO_POINTER(MTX_FLOAT));

	/* Widget Types */
	g_hash_table_insert(str_2_enum,"_MTX_ENTRY_",
			GINT_TO_POINTER(MTX_ENTRY));
	g_hash_table_insert(str_2_enum,"_MTX_LABEL_",
			GINT_TO_POINTER(MTX_LABEL));
	g_hash_table_insert(str_2_enum,"_MTX_RANGE_",
			GINT_TO_POINTER(MTX_RANGE));
	g_hash_table_insert(str_2_enum,"_MTX_SPINBUTTON_",
			GINT_TO_POINTER(MTX_SPINBUTTON));

	/* Variable handling */
	g_hash_table_insert(str_2_enum,"_IMMEDIATE_",
			GINT_TO_POINTER(IMMEDIATE));
	g_hash_table_insert(str_2_enum,"_DEFERRED_",
			GINT_TO_POINTER(DEFERRED));
	g_hash_table_insert(str_2_enum,"_IGNORED_",
			GINT_TO_POINTER(IGNORED));

	/* Complex Expressions (RT Vars)*/
	g_hash_table_insert(str_2_enum,"_VE_EMB_BIT_",
			GINT_TO_POINTER(VE_EMB_BIT));
	g_hash_table_insert(str_2_enum,"_VE_VAR_",
			GINT_TO_POINTER(VE_VAR));
	g_hash_table_insert(str_2_enum,"_RAW_EMB_BIT_",
			GINT_TO_POINTER(RAW_EMB_BIT));
	g_hash_table_insert(str_2_enum,"_RAW_VAR_",
			GINT_TO_POINTER(RAW_VAR));


	/* Signal handler mapping */
	g_hash_table_insert(str_2_enum,"_ALT_SIMUL_",
			GINT_TO_POINTER(ALT_SIMUL));
	g_hash_table_insert(str_2_enum,"_REQFUEL_RESCALE_TABLE_",
			GINT_TO_POINTER(REQFUEL_RESCALE_TABLE));
	g_hash_table_insert(str_2_enum,"_RESCALE_TABLE_",
			GINT_TO_POINTER(RESCALE_TABLE));
	g_hash_table_insert(str_2_enum,"_EXPORT_SINGLE_TABLE_",
			GINT_TO_POINTER(EXPORT_SINGLE_TABLE));
	g_hash_table_insert(str_2_enum,"_IMPORT_SINGLE_TABLE_",
			GINT_TO_POINTER(IMPORT_SINGLE_TABLE));
	g_hash_table_insert(str_2_enum,"_TE_TABLE_",
			GINT_TO_POINTER(TE_TABLE));
	g_hash_table_insert(str_2_enum,"_TE_TABLE_GROUP_",
			GINT_TO_POINTER(TE_TABLE_GROUP));
	g_hash_table_insert(str_2_enum,"_MULTI_EXPRESSION_",
			GINT_TO_POINTER(MULTI_EXPRESSION));
	g_hash_table_insert(str_2_enum,"_GENERIC_",
			GINT_TO_POINTER(GENERIC));
	g_hash_table_insert(str_2_enum,"_TRIGGER_ANGLE_",
			GINT_TO_POINTER(TRIGGER_ANGLE));
	g_hash_table_insert(str_2_enum,"_ODDFIRE_ANGLE_",
			GINT_TO_POINTER(ODDFIRE_ANGLE));
	g_hash_table_insert(str_2_enum,"_NUM_SQUIRTS_1_",
			GINT_TO_POINTER(NUM_SQUIRTS_1));
	g_hash_table_insert(str_2_enum,"_NUM_SQUIRTS_2_",
			GINT_TO_POINTER(NUM_SQUIRTS_2));
	g_hash_table_insert(str_2_enum,"_NUM_CYLINDERS_1_",
			GINT_TO_POINTER(NUM_CYLINDERS_1));
	g_hash_table_insert(str_2_enum,"_NUM_CYLINDERS_2_",
			GINT_TO_POINTER(NUM_CYLINDERS_2));
	g_hash_table_insert(str_2_enum,"_NUM_INJECTORS_1_",
			GINT_TO_POINTER(NUM_INJECTORS_1));
	g_hash_table_insert(str_2_enum,"_NUM_INJECTORS_2_",
			GINT_TO_POINTER(NUM_INJECTORS_2));
	g_hash_table_insert(str_2_enum,"_LOCKED_REQ_FUEL_",
			GINT_TO_POINTER(LOCKED_REQ_FUEL));
	g_hash_table_insert(str_2_enum,"_REQ_FUEL_1_",
			GINT_TO_POINTER(REQ_FUEL_1));
	g_hash_table_insert(str_2_enum,"_REQ_FUEL_2_",
			GINT_TO_POINTER(REQ_FUEL_2));
	g_hash_table_insert(str_2_enum,"_REQ_FUEL_POPUP_",
			GINT_TO_POINTER(REQ_FUEL_POPUP));
	g_hash_table_insert(str_2_enum,"_TRIGGER_ANGLE_",
			GINT_TO_POINTER(TRIGGER_ANGLE));
	g_hash_table_insert(str_2_enum,"_READ_VE_CONST_",
			GINT_TO_POINTER(READ_VE_CONST));
	g_hash_table_insert(str_2_enum,"_BURN_MS_FLASH_",
			GINT_TO_POINTER(BURN_MS_FLASH));
	g_hash_table_insert(str_2_enum,"_START_DATALOGGING_",
			GINT_TO_POINTER(START_DATALOGGING));
	g_hash_table_insert(str_2_enum,"_STOP_DATALOGGING_",
			GINT_TO_POINTER(STOP_DATALOGGING));
	g_hash_table_insert(str_2_enum,"_START_PLAYBACK_",
			GINT_TO_POINTER(START_PLAYBACK));
	g_hash_table_insert(str_2_enum,"_STOP_PLAYBACK_",
			GINT_TO_POINTER(STOP_PLAYBACK));
	g_hash_table_insert(str_2_enum,"_START_REALTIME_",
			GINT_TO_POINTER(START_REALTIME));
	g_hash_table_insert(str_2_enum,"_STOP_REALTIME_",
			GINT_TO_POINTER(STOP_REALTIME));
	g_hash_table_insert(str_2_enum,"_DLOG_SELECT_ALL_",
			GINT_TO_POINTER(DLOG_SELECT_ALL));
	g_hash_table_insert(str_2_enum,"_DLOG_DESELECT_ALL_",
			GINT_TO_POINTER(DLOG_DESELECT_ALL));
	g_hash_table_insert(str_2_enum,"_DLOG_SELECT_DEFAULTS_",
			GINT_TO_POINTER(DLOG_SELECT_DEFAULTS));
	g_hash_table_insert(str_2_enum,"_DLOG_DUMP_INTERNAL_",
			GINT_TO_POINTER(DLOG_DUMP_INTERNAL));
	g_hash_table_insert(str_2_enum,"_SELECT_PARAMS_",
			GINT_TO_POINTER(SELECT_PARAMS));
	g_hash_table_insert(str_2_enum,"_SELECT_DLOG_IMP_",
			GINT_TO_POINTER(SELECT_DLOG_IMP));
	g_hash_table_insert(str_2_enum,"_SELECT_DLOG_EXP_",
			GINT_TO_POINTER(SELECT_DLOG_EXP));
	g_hash_table_insert(str_2_enum,"_CLOSE_LOGFILE_",
			GINT_TO_POINTER(CLOSE_LOGFILE));
	g_hash_table_insert(str_2_enum,"_LOGVIEW_ZOOM_",
			GINT_TO_POINTER(LOGVIEW_ZOOM));
	g_hash_table_insert(str_2_enum,"_REBOOT_GETERR_",
			GINT_TO_POINTER(REBOOT_GETERR));
	g_hash_table_insert(str_2_enum,"_IMPORT_VETABLE_",
			GINT_TO_POINTER(IMPORT_VETABLE));
	g_hash_table_insert(str_2_enum,"_EXPORT_VETABLE_",
			GINT_TO_POINTER(EXPORT_VETABLE));
	g_hash_table_insert(str_2_enum,"_REVERT_TO_BACKUP_",
			GINT_TO_POINTER(REVERT_TO_BACKUP));
	g_hash_table_insert(str_2_enum,"_BACKUP_ALL_",
			GINT_TO_POINTER(BACKUP_ALL));
	g_hash_table_insert(str_2_enum,"_RESTORE_ALL_",
			GINT_TO_POINTER(RESTORE_ALL));
	g_hash_table_insert(str_2_enum,"_READ_RAW_MEMORY_",
			GINT_TO_POINTER(READ_RAW_MEMORY));
	g_hash_table_insert(str_2_enum,"_HEX_VIEW_",
			GINT_TO_POINTER(HEX_VIEW));
	g_hash_table_insert(str_2_enum,"_BINARY_VIEW_",
			GINT_TO_POINTER(BINARY_VIEW));
	g_hash_table_insert(str_2_enum,"_DECIMAL_VIEW_",
			GINT_TO_POINTER(DECIMAL_VIEW));
	g_hash_table_insert(str_2_enum,"_TRACKING_FOCUS_",
			GINT_TO_POINTER(TRACKING_FOCUS));
	g_hash_table_insert(str_2_enum,"_MS2_USER_OUTPUTS_",
			GINT_TO_POINTER(MS2_USER_OUTPUTS));

	/* Datalogger/logviewer */
	g_hash_table_insert(str_2_enum,"_COMMA_",
			GINT_TO_POINTER(COMMA));
	g_hash_table_insert(str_2_enum,"_TAB_",
			GINT_TO_POINTER(TAB));
	g_hash_table_insert(str_2_enum,"_REALTIME_VIEW_",
			GINT_TO_POINTER(REALTIME_VIEW));
	g_hash_table_insert(str_2_enum,"_PLAYBACK_VIEW_",
			GINT_TO_POINTER(PLAYBACK_VIEW));

	/* Page Identifiers */
	g_hash_table_insert(str_2_enum,"_SETTINGS_TAB_",
			GINT_TO_POINTER(SETTINGS_TAB));
	g_hash_table_insert(str_2_enum,"_CORRECTIONS_TAB_",
			GINT_TO_POINTER(CORRECTIONS_TAB));
	g_hash_table_insert(str_2_enum,"_DATALOGGING_TAB_",
			GINT_TO_POINTER(DATALOGGING_TAB));
	g_hash_table_insert(str_2_enum,"_ACCEL_WIZ_TAB_",
			GINT_TO_POINTER(ACCEL_WIZ_TAB));
	g_hash_table_insert(str_2_enum,"_ENRICHMENTS_TAB_",
			GINT_TO_POINTER(ENRICHMENTS_TAB));
	g_hash_table_insert(str_2_enum,"_RUNTIME_TAB_",
			GINT_TO_POINTER(RUNTIME_TAB));
	g_hash_table_insert(str_2_enum,"_WARMUP_WIZ_TAB_",
			GINT_TO_POINTER(WARMUP_WIZ_TAB));
	g_hash_table_insert(str_2_enum,"_VETABLES_TAB_",
			GINT_TO_POINTER(VETABLES_TAB));
	g_hash_table_insert(str_2_enum,"_SPARKTABLES_TAB_",
			GINT_TO_POINTER(SPARKTABLES_TAB));
	g_hash_table_insert(str_2_enum,"_AFRTABLES_TAB_",
			GINT_TO_POINTER(AFRTABLES_TAB));
	g_hash_table_insert(str_2_enum,"_ALPHA_N_TAB_",
			GINT_TO_POINTER(ALPHA_N_TAB));
	g_hash_table_insert(str_2_enum,"_BOOSTTABLES_TAB_",
			GINT_TO_POINTER(BOOSTTABLES_TAB));
	g_hash_table_insert(str_2_enum,"_STAGING_TAB_",
			GINT_TO_POINTER(STAGING_TAB));
	g_hash_table_insert(str_2_enum,"_ROTARYTABLES_TAB_",
			GINT_TO_POINTER(ROTARYTABLES_TAB));
	g_hash_table_insert(str_2_enum,"_ERROR_STATUS_TAB_",
			GINT_TO_POINTER(ERROR_STATUS_TAB));

	/* Algorithm */
	g_hash_table_insert(str_2_enum,"_SPEED_DENSITY_",
			GINT_TO_POINTER(SPEED_DENSITY));
	g_hash_table_insert(str_2_enum,"_ALPHA_N_",
			GINT_TO_POINTER(ALPHA_N));
	g_hash_table_insert(str_2_enum,"_MAF_",
			GINT_TO_POINTER(MAF));
	g_hash_table_insert(str_2_enum,"_SD_AN_HYBRID_",
			GINT_TO_POINTER(SD_AN_HYBRID));
	g_hash_table_insert(str_2_enum,"_MAF_AN_HYBRID_",
			GINT_TO_POINTER(MAF_AN_HYBRID));
	g_hash_table_insert(str_2_enum,"_SD_MAF_HYBRID_",
			GINT_TO_POINTER(SD_MAF_HYBRID));

	/* Function Call */
	g_hash_table_insert(str_2_enum,"_FUNC_CALL_",
			GINT_TO_POINTER(FUNC_CALL));
	g_hash_table_insert(str_2_enum,"_WRITE_CMD_",
			GINT_TO_POINTER(WRITE_CMD));
	g_hash_table_insert(str_2_enum,"_NULL_CMD_",
			GINT_TO_POINTER(NULL_CMD));

	/* XmlCmdType's */
	g_hash_table_insert(str_2_enum,"_WRITE_VERIFY_",
			GINT_TO_POINTER(WRITE_VERIFY));
	g_hash_table_insert(str_2_enum,"_MISMATCH_COUNT_",
			GINT_TO_POINTER(MISMATCH_COUNT));
	g_hash_table_insert(str_2_enum,"_MS1_CLOCK_",
			GINT_TO_POINTER(MS1_CLOCK));
	g_hash_table_insert(str_2_enum,"_MS2_CLOCK_",
			GINT_TO_POINTER(MS2_CLOCK));
	g_hash_table_insert(str_2_enum,"_NUM_REV_",
			GINT_TO_POINTER(NUM_REV));
	g_hash_table_insert(str_2_enum,"_TEXT_REV_",
			GINT_TO_POINTER(TEXT_REV));
	g_hash_table_insert(str_2_enum,"_SIGNATURE_",
			GINT_TO_POINTER(SIGNATURE));
	g_hash_table_insert(str_2_enum,"_MS1_VECONST_",
			GINT_TO_POINTER(MS1_VECONST));
	g_hash_table_insert(str_2_enum,"_MS2_VECONST_",
			GINT_TO_POINTER(MS2_VECONST));
	g_hash_table_insert(str_2_enum,"_MS2_BOOTLOADER_",
			GINT_TO_POINTER(MS2_BOOTLOADER));
	g_hash_table_insert(str_2_enum,"_MS1_RT_VARS_",
			GINT_TO_POINTER(MS1_RT_VARS));
	g_hash_table_insert(str_2_enum,"_MS2_RT_VARS_",
			GINT_TO_POINTER(MS2_RT_VARS));
	g_hash_table_insert(str_2_enum,"_MS1_GETERROR_",
			GINT_TO_POINTER(MS1_GETERROR));
	g_hash_table_insert(str_2_enum,"_MS1_E_TRIGMON_",
			GINT_TO_POINTER(MS1_E_TRIGMON));
	g_hash_table_insert(str_2_enum,"_MS1_E_TOOTHMON_",
			GINT_TO_POINTER(MS1_E_TOOTHMON));
	g_hash_table_insert(str_2_enum,"_MS2_E_TRIGMON_",
			GINT_TO_POINTER(MS2_E_TRIGMON));
	g_hash_table_insert(str_2_enum,"_MS2_E_TOOTHMON_",
			GINT_TO_POINTER(MS2_E_TOOTHMON));
	g_hash_table_insert(str_2_enum,"_MS2_E_COMPOSITEMON_",
			GINT_TO_POINTER(MS2_E_COMPOSITEMON));

	/* Action's */
	g_hash_table_insert(str_2_enum,"_SLEEP_",
			GINT_TO_POINTER(SLEEP));

	/* TCP Socket Commands */
	g_hash_table_insert(str_2_enum,"HELP",
			GINT_TO_POINTER(HELP));
	g_hash_table_insert(str_2_enum,"QUIT",
			GINT_TO_POINTER(QUIT));
	g_hash_table_insert(str_2_enum,"GET_REVISION",
			GINT_TO_POINTER(GET_REVISION));
	g_hash_table_insert(str_2_enum,"GET_SIGNATURE",
			GINT_TO_POINTER(GET_SIGNATURE));
	g_hash_table_insert(str_2_enum,"GET_RT_VARS",
			GINT_TO_POINTER(GET_RT_VARS));
	g_hash_table_insert(str_2_enum,"GET_RTV_LIST",
			GINT_TO_POINTER(GET_RTV_LIST));
	g_hash_table_insert(str_2_enum,"GET_ECU_VARS",
			GINT_TO_POINTER(GET_ECU_VARS));
	g_hash_table_insert(str_2_enum,"GET_ECU_VAR_U08",
			GINT_TO_POINTER(GET_ECU_VAR_U08));
	g_hash_table_insert(str_2_enum,"GET_ECU_VAR_S08",
			GINT_TO_POINTER(GET_ECU_VAR_S08));
	g_hash_table_insert(str_2_enum,"GET_ECU_VAR_U16",
			GINT_TO_POINTER(GET_ECU_VAR_U16));
	g_hash_table_insert(str_2_enum,"GET_ECU_VAR_S16",
			GINT_TO_POINTER(GET_ECU_VAR_S16));
	g_hash_table_insert(str_2_enum,"GET_ECU_VAR_U32",
			GINT_TO_POINTER(GET_ECU_VAR_U32));
	g_hash_table_insert(str_2_enum,"GET_ECU_VAR_S32",
			GINT_TO_POINTER(GET_ECU_VAR_S32));
	g_hash_table_insert(str_2_enum,"SET_ECU_VAR_U08",
			GINT_TO_POINTER(SET_ECU_VAR_U08));
	g_hash_table_insert(str_2_enum,"SET_ECU_VAR_S08",
			GINT_TO_POINTER(SET_ECU_VAR_S08));
	g_hash_table_insert(str_2_enum,"SET_ECU_VAR_U16",
			GINT_TO_POINTER(SET_ECU_VAR_U16));
	g_hash_table_insert(str_2_enum,"SET_ECU_VAR_S16",
			GINT_TO_POINTER(SET_ECU_VAR_S16));
	g_hash_table_insert(str_2_enum,"SET_ECU_VAR_U32",
			GINT_TO_POINTER(SET_ECU_VAR_U32));
	g_hash_table_insert(str_2_enum,"SET_ECU_VAR_S32",
			GINT_TO_POINTER(SET_ECU_VAR_S32));
	g_hash_table_insert(str_2_enum,"BURN_FLASH",
			GINT_TO_POINTER(BURN_FLASH));
	g_hash_table_insert(str_2_enum,"GET_RAW_ECU",
			GINT_TO_POINTER(GET_RAW_ECU));
	g_hash_table_insert(str_2_enum,"SET_RAW_ECU",
			GINT_TO_POINTER(SET_RAW_ECU));

	/* Interrogation Test Results */
	g_hash_table_insert(str_2_enum,"_RESULT_DATA_",
			GINT_TO_POINTER(RESULT_DATA));
	g_hash_table_insert(str_2_enum,"_RESULT_TEXT_",
			GINT_TO_POINTER(RESULT_TEXT));

	/* XMLcomm processing */
	g_hash_table_insert(str_2_enum,"_DATA_",
			GINT_TO_POINTER(DATA));
	g_hash_table_insert(str_2_enum,"_ACTION_",
			GINT_TO_POINTER(ACTION));
	g_hash_table_insert(str_2_enum,"_STATIC_STRING_",
			GINT_TO_POINTER(STATIC_STRING));
	g_hash_table_insert(str_2_enum,"_SLEEP_",
			GINT_TO_POINTER(SLEEP));
	/* Tab datamap processing */
	g_hash_table_insert(str_2_enum,"active_fg",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"algorithms",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"alt_c_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"alt_f_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"alt_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"alt_lookuptable",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"alt_lookuptables",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"applicable_tables",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"bind_to_list",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"bitmask",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"bitval",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"bitvals",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"choices",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"c_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"complex_expr",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,"create_tags",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"ctrl_name",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"data",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"datasource",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"depend_on",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"dl_conv_expr",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"dl_conv_exprs",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"dl_type",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,"dlog_field_name",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"dlog_gui_name",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"dont_backup",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,"f_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"function_name",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"gaugexml",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"group_2_update",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"handler",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,"hys_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"inactive_fg",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"index",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"initializer",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"internal_names",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"labels",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"lim_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"log_by_default",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,"lookuptable",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"match_type",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,"max_chars",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"multi_expr_keys",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"oddfire_bit_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"output_num",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"page",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"post_function_with_arg",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"post_functions_with_arg",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"precision",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"real_lower",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"real_upper",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"raw_lower",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"raw_upper",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"register_as",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"require_reboot",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"rescaler_frame",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"reverse_keys",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,"set_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"set_widgets_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"show_prefix",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,"size",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,"source",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"sources",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"source_key",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"source_value",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"special",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"source_values",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"spconfig_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"src_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"sub-notebook",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"swap_labels",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"tab_ident",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,"table_2_update",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"table_num",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"temp_dep",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,"te_table_num",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"te_tables",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"toggle_labels",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"toggle_groups",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"tooltip",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"ul_complex",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,"ul_conv_expr",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"ul_conv_exprs",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"ulimit_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"use_color",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,"widget_type",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,"width",
			GINT_TO_POINTER(MTX_INT));

	/* Match conditions */
	g_hash_table_insert(str_2_enum,"_OR_",
			GINT_TO_POINTER(OR));
	g_hash_table_insert(str_2_enum,"_AND_",
			GINT_TO_POINTER(AND));

	/*g_hash_table_foreach(str_2_enum,dump_hash,NULL);*/

	DATA_SET_FULL(global_data,"str2_enum",str_2_enum,g_hash_table_destroy);
}


/*!
 \brief dump_hash() is a debug function to dump the contents of the str_2_enum
 hashtable to check for errors or problems
 \param key (gpointer) key name in the hashtable
 \param value (gpointer) value (enumeration value) in the hashtable
 \param user_data (gpointer) unused...
 */
void dump_hash(gpointer key, gpointer value, gpointer user_data)
{
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": dump_hash()\n\tKey %s, Value %i\n",(gchar *)key, (GINT)value));
}


/*!
 \brief translate_string() is called passing in a string name to be translated
 into an enumeration
 \param string (gchar *) string to be translated
 \returns enumeration equivalent
 */
gint translate_string(gchar *string)
{
	gpointer value = 0;
	value = g_hash_table_lookup(str_2_enum,string);
	if (value == NULL)
	{
		/*dbg_func(CRITICAL,g_strdup_printf(__FILE__": translate_string()\n\tString \"%s\" NOT FOUND in hashtable....\n",string));*/
		return (MTX_UNKNOWN);
	}
	else
		return (GINT)value;
}
