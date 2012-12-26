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
  \file src/stringmatch.c
  \ingroup CoreMtx
  \brief Manages string to ENUM matching/relationships
  \author David Andruczyk
  */

#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <gui_handlers.h>

extern gconstpointer *global_data;

/*!
  \brief build_string_2_enum_table() constructs a hashtable that maps a textual
  name to it's matching enumeration.  It's used for mapping things from all the 
  configuration files so that things just plain look better
  */
G_MODULE_EXPORT void build_string_2_enum_table(void)
{
	GHashTable *str_2_enum = NULL;
	str_2_enum = g_hash_table_new_full(g_str_hash,g_str_equal,NULL,NULL);

	ENTER();
	/* Interrogation field types */
	g_hash_table_insert(str_2_enum,(gpointer)"_CHAR_",
			GINT_TO_POINTER(MTX_CHAR));
	g_hash_table_insert(str_2_enum,(gpointer)"_U08_",
			GINT_TO_POINTER(MTX_U08));
	g_hash_table_insert(str_2_enum,(gpointer)"_S08_",
			GINT_TO_POINTER(MTX_S08));
	g_hash_table_insert(str_2_enum,(gpointer)"_U16_",
			GINT_TO_POINTER(MTX_U16));
	g_hash_table_insert(str_2_enum,(gpointer)"_S16_",
			GINT_TO_POINTER(MTX_S16));
	g_hash_table_insert(str_2_enum,(gpointer)"_U32_",
			GINT_TO_POINTER(MTX_U32));
	g_hash_table_insert(str_2_enum,(gpointer)"_S32_",
			GINT_TO_POINTER(MTX_S32));
	g_hash_table_insert(str_2_enum,(gpointer)"_UNDEF_",
			GINT_TO_POINTER(MTX_UNDEF));

	/* Data Types for glade data binder.... */
	g_hash_table_insert(str_2_enum,(gpointer)"_INT_",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"_ENUM_",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,(gpointer)"_STRING_",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"_BOOL_",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"_FLOAT_",
			GINT_TO_POINTER(MTX_FLOAT));

	/* Widget Types */
	g_hash_table_insert(str_2_enum,(gpointer)"_MTX_ENTRY_",
			GINT_TO_POINTER(MTX_ENTRY));
	g_hash_table_insert(str_2_enum,(gpointer)"_MTX_LABEL_",
			GINT_TO_POINTER(MTX_LABEL));
	g_hash_table_insert(str_2_enum,(gpointer)"_MTX_RANGE_",
			GINT_TO_POINTER(MTX_RANGE));
	g_hash_table_insert(str_2_enum,(gpointer)"_MTX_SPINBUTTON_",
			GINT_TO_POINTER(MTX_SPINBUTTON));

	/* Variable handling */
	g_hash_table_insert(str_2_enum,(gpointer)"_IMMEDIATE_",
			GINT_TO_POINTER(IMMEDIATE));
	g_hash_table_insert(str_2_enum,(gpointer)"_DEFERRED_",
			GINT_TO_POINTER(DEFERRED));
	g_hash_table_insert(str_2_enum,(gpointer)"_IGNORED_",
			GINT_TO_POINTER(IGNORED));

	/* Complex Expressions (RT Vars)*/
	g_hash_table_insert(str_2_enum,(gpointer)"_ECU_EMB_BIT_",
			GINT_TO_POINTER(ECU_EMB_BIT));
	g_hash_table_insert(str_2_enum,(gpointer)"_ECU_VAR_",
			GINT_TO_POINTER(ECU_VAR));
	g_hash_table_insert(str_2_enum,(gpointer)"_RAW_EMB_BIT_",
			GINT_TO_POINTER(RAW_EMB_BIT));
	g_hash_table_insert(str_2_enum,(gpointer)"_RAW_VAR_",
			GINT_TO_POINTER(RAW_VAR));


	/* Signal handler mapping */
	g_hash_table_insert(str_2_enum,(gpointer)"_GENERIC_",
			GINT_TO_POINTER(GENERIC));
	g_hash_table_insert(str_2_enum,(gpointer)"_RESCALE_TABLE_",
			GINT_TO_POINTER(RESCALE_TABLE));
	g_hash_table_insert(str_2_enum,(gpointer)"_EXPORT_SINGLE_TABLE_",
			GINT_TO_POINTER(EXPORT_SINGLE_TABLE));
	g_hash_table_insert(str_2_enum,(gpointer)"_IMPORT_SINGLE_TABLE_",
			GINT_TO_POINTER(IMPORT_SINGLE_TABLE));
	g_hash_table_insert(str_2_enum,(gpointer)"_TE_TABLE_",
			GINT_TO_POINTER(TE_TABLE));
	g_hash_table_insert(str_2_enum,(gpointer)"_TE_TABLE_GROUP_",
			GINT_TO_POINTER(TE_TABLE_GROUP));
	g_hash_table_insert(str_2_enum,(gpointer)"_READ_VE_CONST_",
			GINT_TO_POINTER(READ_VE_CONST));
	g_hash_table_insert(str_2_enum,(gpointer)"_BURN_FLASH_",
			GINT_TO_POINTER(BURN_FLASH));
	g_hash_table_insert(str_2_enum,(gpointer)"_START_DATALOGGING_",
			GINT_TO_POINTER(START_DATALOGGING));
	g_hash_table_insert(str_2_enum,(gpointer)"_STOP_DATALOGGING_",
			GINT_TO_POINTER(STOP_DATALOGGING));
	g_hash_table_insert(str_2_enum,(gpointer)"_START_PLAYBACK_",
			GINT_TO_POINTER(START_PLAYBACK));
	g_hash_table_insert(str_2_enum,(gpointer)"_STOP_PLAYBACK_",
			GINT_TO_POINTER(STOP_PLAYBACK));
	g_hash_table_insert(str_2_enum,(gpointer)"_START_REALTIME_",
			GINT_TO_POINTER(START_REALTIME));
	g_hash_table_insert(str_2_enum,(gpointer)"_STOP_REALTIME_",
			GINT_TO_POINTER(STOP_REALTIME));
	g_hash_table_insert(str_2_enum,(gpointer)"_DLOG_SELECT_ALL_",
			GINT_TO_POINTER(DLOG_SELECT_ALL));
	g_hash_table_insert(str_2_enum,(gpointer)"_DLOG_DESELECT_ALL_",
			GINT_TO_POINTER(DLOG_DESELECT_ALL));
	g_hash_table_insert(str_2_enum,(gpointer)"_DLOG_SELECT_DEFAULTS_",
			GINT_TO_POINTER(DLOG_SELECT_DEFAULTS));
	g_hash_table_insert(str_2_enum,(gpointer)"_DLOG_DUMP_INTERNAL_",
			GINT_TO_POINTER(DLOG_DUMP_INTERNAL));
	g_hash_table_insert(str_2_enum,(gpointer)"_SELECT_PARAMS_",
			GINT_TO_POINTER(SELECT_PARAMS));
	g_hash_table_insert(str_2_enum,(gpointer)"_SELECT_DLOG_IMP_",
			GINT_TO_POINTER(SELECT_DLOG_IMP));
	g_hash_table_insert(str_2_enum,(gpointer)"_SELECT_DLOG_EXP_",
			GINT_TO_POINTER(SELECT_DLOG_EXP));
	g_hash_table_insert(str_2_enum,(gpointer)"_CLOSE_LOGFILE_",
			GINT_TO_POINTER(CLOSE_LOGFILE));
	g_hash_table_insert(str_2_enum,(gpointer)"_LOGVIEW_ZOOM_",
			GINT_TO_POINTER(LOGVIEW_ZOOM));
	g_hash_table_insert(str_2_enum,(gpointer)"_IMPORT_VETABLE_",
			GINT_TO_POINTER(IMPORT_VETABLE));
	g_hash_table_insert(str_2_enum,(gpointer)"_EXPORT_VETABLE_",
			GINT_TO_POINTER(EXPORT_VETABLE));
	g_hash_table_insert(str_2_enum,(gpointer)"_REVERT_TO_BACKUP_",
			GINT_TO_POINTER(REVERT_TO_BACKUP));
	g_hash_table_insert(str_2_enum,(gpointer)"_BACKUP_ALL_",
			GINT_TO_POINTER(BACKUP_ALL));
	g_hash_table_insert(str_2_enum,(gpointer)"_RESTORE_ALL_",
			GINT_TO_POINTER(RESTORE_ALL));
	g_hash_table_insert(str_2_enum,(gpointer)"_READ_RAW_MEMORY_",
			GINT_TO_POINTER(READ_RAW_MEMORY));
	g_hash_table_insert(str_2_enum,(gpointer)"_TRACKING_FOCUS_",
			GINT_TO_POINTER(TRACKING_FOCUS));
	g_hash_table_insert(str_2_enum,(gpointer)"_TOGGLE_KELVIN_",
			GINT_TO_POINTER(TOGGLE_KELVIN));
	g_hash_table_insert(str_2_enum,(gpointer)"_TOGGLE_CELSIUS_",
			GINT_TO_POINTER(TOGGLE_CELSIUS));
	g_hash_table_insert(str_2_enum,(gpointer)"_TOGGLE_FAHRENHEIT_",
			GINT_TO_POINTER(TOGGLE_FAHRENHEIT));

	/* Datalogger/logviewer */
	g_hash_table_insert(str_2_enum,(gpointer)"_COMMA_",
			GINT_TO_POINTER(COMMA));
	g_hash_table_insert(str_2_enum,(gpointer)"_TAB_",
			GINT_TO_POINTER(TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_REALTIME_VIEW_",
			GINT_TO_POINTER(REALTIME_VIEW));
	g_hash_table_insert(str_2_enum,(gpointer)"_PLAYBACK_VIEW_",
			GINT_TO_POINTER(PLAYBACK_VIEW));

	/* Page Identifiers */
	g_hash_table_insert(str_2_enum,(gpointer)"_SETTINGS_TAB_",
			GINT_TO_POINTER(SETTINGS_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_CORRECTIONS_TAB_",
			GINT_TO_POINTER(CORRECTIONS_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_DATALOGGING_TAB_",
			GINT_TO_POINTER(DATALOGGING_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_ACCEL_WIZ_TAB_",
			GINT_TO_POINTER(ACCEL_WIZ_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_ENRICHMENTS_TAB_",
			GINT_TO_POINTER(ENRICHMENTS_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_RUNTIME_TAB_",
			GINT_TO_POINTER(RUNTIME_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_WARMUP_WIZ_TAB_",
			GINT_TO_POINTER(WARMUP_WIZ_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_VETABLES_TAB_",
			GINT_TO_POINTER(VETABLES_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_SPARKTABLES_TAB_",
			GINT_TO_POINTER(SPARKTABLES_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_AFRTABLES_TAB_",
			GINT_TO_POINTER(AFRTABLES_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_ALPHA_N_TAB_",
			GINT_TO_POINTER(ALPHA_N_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_BOOSTTABLES_TAB_",
			GINT_TO_POINTER(BOOSTTABLES_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_STAGING_TAB_",
			GINT_TO_POINTER(STAGING_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_ROTARYTABLES_TAB_",
			GINT_TO_POINTER(ROTARYTABLES_TAB));
	g_hash_table_insert(str_2_enum,(gpointer)"_ERROR_STATUS_TAB_",
			GINT_TO_POINTER(ERROR_STATUS_TAB));

	/* Algorithm */
	g_hash_table_insert(str_2_enum,(gpointer)"_SPEED_DENSITY_",
			GINT_TO_POINTER(SPEED_DENSITY));
	g_hash_table_insert(str_2_enum,(gpointer)"_ALPHA_N_",
			GINT_TO_POINTER(ALPHA_N));
	g_hash_table_insert(str_2_enum,(gpointer)"_MAF_",
			GINT_TO_POINTER(MAF));
	g_hash_table_insert(str_2_enum,(gpointer)"_SD_AN_HYBRID_",
			GINT_TO_POINTER(SD_AN_HYBRID));
	g_hash_table_insert(str_2_enum,(gpointer)"_MAF_AN_HYBRID_",
			GINT_TO_POINTER(MAF_AN_HYBRID));
	g_hash_table_insert(str_2_enum,(gpointer)"_SD_MAF_HYBRID_",
			GINT_TO_POINTER(SD_MAF_HYBRID));

	/* Function Call */
	g_hash_table_insert(str_2_enum,(gpointer)"_FUNC_CALL_",
			GINT_TO_POINTER(FUNC_CALL));
	g_hash_table_insert(str_2_enum,(gpointer)"_WRITE_CMD_",
			GINT_TO_POINTER(WRITE_CMD));
	g_hash_table_insert(str_2_enum,(gpointer)"_NULL_CMD_",
			GINT_TO_POINTER(NULL_CMD));

	/* Action's */
	g_hash_table_insert(str_2_enum,(gpointer)"_SLEEP_",
			GINT_TO_POINTER(SLEEP));
	/* XMLcomm processing */
	g_hash_table_insert(str_2_enum,(gpointer)"_DATA_",
			GINT_TO_POINTER(DATA));
	g_hash_table_insert(str_2_enum,(gpointer)"_ACTION_",
			GINT_TO_POINTER(ACTION));
	g_hash_table_insert(str_2_enum,(gpointer)"_STATIC_STRING_",
			GINT_TO_POINTER(STATIC_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"_SLEEP_",
			GINT_TO_POINTER(SLEEP));
	/* Axis's */
	g_hash_table_insert(str_2_enum,(gpointer)"_X_",
			GINT_TO_POINTER(_X_));
	g_hash_table_insert(str_2_enum,(gpointer)"_Y_",
			GINT_TO_POINTER(_Y_));
	g_hash_table_insert(str_2_enum,(gpointer)"_Z_",
			GINT_TO_POINTER(_Z_));
	/* Tab datamap processing */
	g_hash_table_insert(str_2_enum,(gpointer)"active_fg",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"algorithms",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"alt_c_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"alt_f_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"alt_k_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"alt_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"alt_lookuptable",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"alt_lookuptables",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"applicable_tables",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"axis",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,(gpointer)"bind_to_list",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"bitmask",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"bitval",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"bitvals",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"choices",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"c_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"create_tags",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"ctrl_name",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"data",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"datasource",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"depend_on",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"toecu_conv_expr",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"toecu_conv_exprs",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"dl_type",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,(gpointer)"dlog_field_name",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"dlog_gui_name",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"dont_backup",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"f_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"function_name",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"gaugexml",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"group_2_update",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"handler",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,(gpointer)"hys_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"hyst_widget",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"ignore_algorithm",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"inactive_fg",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"index",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"initializer",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"internal_names",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"k_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"labels",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"lim_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"log_by_default",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"lookuptable",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"lookuptables",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"match_type",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,(gpointer)"max_chars",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"modspecific",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"mt_names",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"multi_expr_keys",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"oddfire_bit_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"orientation",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,(gpointer)"output_num",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"page",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"post_function_with_arg",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"post_functions_with_arg",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"precision",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"prefix",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"real_lower",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"real_upper",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"range_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"raw_lower",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"raw_upper",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"register_as",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"require_reboot",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"rescaler_frame",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"reverse_keys",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"set_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"set_widgets_label",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"set_tab_labels",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"show_prefix",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"show_widget",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"size_offset",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"size",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,(gpointer)"source",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"sources",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"source_key",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"source_value",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"special",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"source_values",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"spconfig_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"src_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"sub-notebook",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"swap_labels",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"tab_ident",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,(gpointer)"table_2_update",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"table_num",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"temp_dep",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"te_table_num",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"te_tables",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"thresh_widget",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"toggle_labels",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"toggle_groups",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"tooltip",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"fromecu_complex",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"fromecu_conv_expr",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"fromecu_mult",
			GINT_TO_POINTER(MTX_FLOAT));
	g_hash_table_insert(str_2_enum,(gpointer)"fromecu_mults",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"fromecu_add",
			GINT_TO_POINTER(MTX_FLOAT));
	g_hash_table_insert(str_2_enum,(gpointer)"fromecu_adds",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"fromecu_conv_exprs",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"ulimit_offset",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,(gpointer)"visible_function",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,(gpointer)"use_color",
			GINT_TO_POINTER(MTX_BOOL));
	g_hash_table_insert(str_2_enum,(gpointer)"widget_type",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,(gpointer)"width",
			GINT_TO_POINTER(MTX_INT));

	/* Match conditions */
	g_hash_table_insert(str_2_enum,(gpointer)"_OR_",
			GINT_TO_POINTER(OR));
	g_hash_table_insert(str_2_enum,(gpointer)"_AND_",
			GINT_TO_POINTER(AND));

	/* Temp Scales */
	g_hash_table_insert(str_2_enum,(gpointer)"_KELVIN_",
			GINT_TO_POINTER(KELVIN));
	g_hash_table_insert(str_2_enum,(gpointer)"_CELSIUS_",
			GINT_TO_POINTER(CELSIUS));
	g_hash_table_insert(str_2_enum,(gpointer)"_FAHRENHEIT_",
			GINT_TO_POINTER(FAHRENHEIT));
	/* MtxPbar orientations */
	g_hash_table_insert(str_2_enum,(gpointer)"_L_TO_R_",
			GINT_TO_POINTER(GTK_PROGRESS_LEFT_TO_RIGHT));
	g_hash_table_insert(str_2_enum,(gpointer)"_R_TO_L_",
			GINT_TO_POINTER(GTK_PROGRESS_RIGHT_TO_LEFT));
	g_hash_table_insert(str_2_enum,(gpointer)"_B_TO_T_",
			GINT_TO_POINTER(GTK_PROGRESS_BOTTOM_TO_TOP));
	g_hash_table_insert(str_2_enum,(gpointer)"_T_TO_B_",
			GINT_TO_POINTER(GTK_PROGRESS_TOP_TO_BOTTOM));

	/*g_hash_table_foreach(str_2_enum,dump_hash,NULL);*/

	DATA_SET_FULL(global_data,"str_2_enum",str_2_enum,g_hash_table_destroy);
	EXIT();
	return;
}


/*!
  \brief dump_hash() is a debug function to dump the contents of the str_2_enum
  hashtable to check for errors or problems
  \param key is the key name in the hashtable
  \param value is the value (enumeration value) in the hashtable
  \param user_data is unused...
  */
G_MODULE_EXPORT void dump_hash(gpointer key, gpointer value, gpointer user_data)
{
	ENTER();
	MTXDBG(CRITICAL,_("Key %s, Value %i\n"),(gchar *)key, (GINT)value);
	EXIT();
	return;
}


/*!
  \brief translate_string() is called passing in a string name to be translated
  into an enumeration
  \param string is the string to be translated
  \returns enumeration equivalent
  */
G_MODULE_EXPORT gint translate_string(const gchar *string)
{
	static GHashTable *str_2_enum = NULL;
	gint value = 0;

	ENTER();
	if (!str_2_enum)
        	str_2_enum = (GHashTable *)DATA_GET(global_data,"str_2_enum");

	value = (GINT)g_hash_table_lookup(str_2_enum,string);
	if (value == 0)
	{
		/*MTXDBG(CRITICAL,_("String \"%s\" NOT FOUND in hashtable....\n"),string);*/
		EXIT();
		return (MTX_UNKNOWN);
	}
	EXIT();	
	return (GINT)value;
}
