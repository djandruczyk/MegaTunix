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
#include <stringmatch.h>


static GHashTable *str_2_enum = NULL;
void dump_hash(gpointer,gpointer,gpointer);

void build_string_2_enum_table()
{
	str_2_enum = g_hash_table_new(g_str_hash,g_str_equal);

	/* Firmware capabilities */
	g_hash_table_insert(str_2_enum,"_STANDARD_",
			GINT_TO_POINTER(STANDARD));
	g_hash_table_insert(str_2_enum,"_DUALTABLE_",
			GINT_TO_POINTER(DUALTABLE));
	g_hash_table_insert(str_2_enum,"_S_N_SPARK_",
			GINT_TO_POINTER(S_N_SPARK));
	g_hash_table_insert(str_2_enum,"_S_N_EDIS_",
			GINT_TO_POINTER(S_N_EDIS));
	g_hash_table_insert(str_2_enum,"_ENHANCED_",
			GINT_TO_POINTER(ENHANCED));
	g_hash_table_insert(str_2_enum,"_IAC_PWM_",
			GINT_TO_POINTER(IAC_PWM));
	g_hash_table_insert(str_2_enum,"_IAC_STEPPER_",
			GINT_TO_POINTER(IAC_STEPPER));
	g_hash_table_insert(str_2_enum,"_BOOST_CTRL_",
			GINT_TO_POINTER(BOOST_CTRL));
	g_hash_table_insert(str_2_enum,"_OVERBOOST_SFTY_",
			GINT_TO_POINTER(OVERBOOST_SFTY));
	g_hash_table_insert(str_2_enum,"_LAUNCH_CTRL_",
			GINT_TO_POINTER(LAUNCH_CTRL));

	/* Storage Types for reading interrogation tests */
	g_hash_table_insert(str_2_enum,"_SIG_",
			GINT_TO_POINTER(SIG));
	g_hash_table_insert(str_2_enum,"_VNUM_",
			GINT_TO_POINTER(VNUM));
	g_hash_table_insert(str_2_enum,"_EXTVER_",
			GINT_TO_POINTER(EXTVER));

	/* Data Types for glade data binder.... */
	g_hash_table_insert(str_2_enum,"_INT_",
			GINT_TO_POINTER(MTX_INT));
	g_hash_table_insert(str_2_enum,"_ENUM_",
			GINT_TO_POINTER(MTX_ENUM));
	g_hash_table_insert(str_2_enum,"_STRING_",
			GINT_TO_POINTER(MTX_STRING));
	g_hash_table_insert(str_2_enum,"_BOOL_",
			GINT_TO_POINTER(MTX_BOOL));

	/* Variable handling */
	g_hash_table_insert(str_2_enum,"_IMMEDIATE_",
			GINT_TO_POINTER(IMMEDIATE));
	g_hash_table_insert(str_2_enum,"_DEFERRED_",
			GINT_TO_POINTER(DEFERRED));

	/* Complex Expressions (RT Vars)*/
	g_hash_table_insert(str_2_enum,"_VE_EMB_BIT_",
			GINT_TO_POINTER(VE_EMB_BIT));
	g_hash_table_insert(str_2_enum,"_VE_VAR_",
			GINT_TO_POINTER(VE_VAR));
	g_hash_table_insert(str_2_enum,"_RAW_VAR_",
			GINT_TO_POINTER(RAW_VAR));


	/* Signal handler mapping */
	g_hash_table_insert(str_2_enum,"_ALT_SIMUL_",
			GINT_TO_POINTER(ALT_SIMUL));
	g_hash_table_insert(str_2_enum,"_GENERIC_",
			GINT_TO_POINTER(GENERIC));
	g_hash_table_insert(str_2_enum,"_TRIGGER_ANGLE_",
			GINT_TO_POINTER(TRIGGER_ANGLE));
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
	g_hash_table_insert(str_2_enum,"_START_REALTIME_",
			GINT_TO_POINTER(START_REALTIME));
	g_hash_table_insert(str_2_enum,"_STOP_REALTIME_",
			GINT_TO_POINTER(STOP_REALTIME));
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
	/* Datalogger/logviewer */
	g_hash_table_insert(str_2_enum,"_COMMA_",
			GINT_TO_POINTER(COMMA));
	g_hash_table_insert(str_2_enum,"_TAB_",
			GINT_TO_POINTER(TAB));
	g_hash_table_insert(str_2_enum,"_SPACE_",
			GINT_TO_POINTER(SPACE));
	g_hash_table_insert(str_2_enum,"_MT_CLASSIC_LOG_",
			GINT_TO_POINTER(MT_CLASSIC_LOG));
	g_hash_table_insert(str_2_enum,"_MT_FULL_LOG_",
			GINT_TO_POINTER(MT_FULL_LOG));
	g_hash_table_insert(str_2_enum,"_CUSTOM_LOG_",
			GINT_TO_POINTER(CUSTOM_LOG));
	g_hash_table_insert(str_2_enum,"_REALTIME_VIEW_",
			GINT_TO_POINTER(REALTIME_VIEW));
	g_hash_table_insert(str_2_enum,"_PLAYBACK_VIEW_",
			GINT_TO_POINTER(PLAYBACK_VIEW));
	/* Page Identifiers */
	g_hash_table_insert(str_2_enum,"_RUNTIME_PAGE_",
			GINT_TO_POINTER(RUNTIME_PAGE));
	g_hash_table_insert(str_2_enum,"_WARMUP_WIZ_PAGE_",
			GINT_TO_POINTER(WARMUP_WIZ_PAGE));


	//g_hash_table_foreach(str_2_enum,dump_hash,NULL);

}
void dump_hash(gpointer key, gpointer value, gpointer user_data)
{
	dbg_func(g_strdup_printf(__FILE__": dump_hash()\n\tKey %s, Value %i\n",(gchar *)key, (gint)value),CRITICAL);
}


gint translate_string(gchar *string)
{
	gpointer value = 0;
	value = g_hash_table_lookup(str_2_enum,string);
	if (value == NULL)
		dbg_func(g_strdup_printf(__FILE__": translate_string()\n\tString \"%s\" NOT FOUND in hashtable....\n",string),CRITICAL);
	return (gint)value;
}
