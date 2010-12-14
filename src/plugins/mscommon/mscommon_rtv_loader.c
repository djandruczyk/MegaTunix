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
#include <configfile.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <mscommon_rtv_loader.h>

extern gconstpointer *global_data;

void common_rtv_loader(gconstpointer *object, ConfigFile *cfgfile, gchar * section, gint i, ComplexExprType type)
{
	static Firmware_Details *firmware = NULL;
	gchar * name = NULL;
	gint tmpi = 0;
	gchar **expr_symbols = NULL;
	gchar *tmpbuf = NULL;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");

	expr_symbols = DATA_GET(object,"expr_symbols");

	switch (type)
	{
		case ECU_EMB_BIT:
			/* ECU Embedded bitfield 4 params */
			name=NULL;
			name=g_strdup_printf("%s_page",expr_symbols[i]);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				dbg_func(RTMLOADER|COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": load_compex_params()\n\tECU_EMB_BIT, failure looking for:%s\n",name));
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_offset",expr_symbols[i]);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				dbg_func(RTMLOADER|COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": load_compex_params()\n\tECU_EMB_BIT, failure looking for:%s\n",name));
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_canID",expr_symbols[i]);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				tmpi = firmware->canID;
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_bitmask",expr_symbols[i]);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				dbg_func(RTMLOADER|COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": load_compex_params()\n\tECU_EMB_BIT, failure looking for:%s\n",name));
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			break;
		case ECU_VAR:
			/* ECU std variable, canID/page/offset/size */
			name=NULL;
			name=g_strdup_printf("%s_canID",expr_symbols[i]);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				tmpi = firmware->canID;

			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_page",expr_symbols[i]);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				dbg_func(RTMLOADER|COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": load_compex_params()\n\tECU_VAR, failure looking for:%s\n",name));
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_offset",expr_symbols[i]);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				dbg_func(RTMLOADER|COMPLEX_EXPR|CRITICAL,g_strdup_printf(__FILE__": load_compex_params()\n\tECU_VAR, failure looking for:%s\n",name));
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_size",expr_symbols[i]);
			if (!cfg_read_string(cfgfile,section,name,&tmpbuf))
				tmpi = MTX_U08;
			else
			{
				tmpi = translate_string_f(tmpbuf);
				g_free(tmpbuf);
			}
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			break;
		default:
			return;
			break;
	}
	return;
}
