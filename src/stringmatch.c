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
#include <enums.h>
#include <stringmatch.h>


static GHashTable *str_2_enum = NULL;

void build_str2_enum_table()
{
	str_2_enum = g_hash_table_new(NULL,NULL);
	g_hash_table_insert(str_2_enum,"_STD_",GINT_TO_POINTER(STD));
	g_hash_table_insert(str_2_enum,"_DUALTABLE_",GINT_TO_POINTER(DUALTABLE));
	g_hash_table_insert(str_2_enum,"_S_N_SPARK_",GINT_TO_POINTER(S_N_SPARK));
	g_hash_table_insert(str_2_enum,"_S_N_EDIS_",GINT_TO_POINTER(S_N_EDIS));
	g_hash_table_insert(str_2_enum,"_ENHANCED_",GINT_TO_POINTER(ENHANCED));
	g_hash_table_insert(str_2_enum,"_RAW_MEMORY_",GINT_TO_POINTER(RAW_MEMORY));
	g_hash_table_insert(str_2_enum,"_IAC_PWM_",GINT_TO_POINTER(IAC_PWM));
	g_hash_table_insert(str_2_enum,"_IAC_STEPPER_",GINT_TO_POINTER(IAC_STEPPER));
	g_hash_table_insert(str_2_enum,"_BOOST_CTRL_",GINT_TO_POINTER(BOOST_CTRL));
	g_hash_table_insert(str_2_enum,"_OVERBOOST_SFTY_",GINT_TO_POINTER(OVERBOOST_SFTY));
	g_hash_table_insert(str_2_enum,"_LAUNCH_CTRL_",GINT_TO_POINTER(LAUNCH_CTRL));
	g_hash_table_insert(str_2_enum,"_TEMP_DEP_",GINT_TO_POINTER(TEMP_DEP));

}

