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

#include <args.h>
#include <config.h>
#include <comms.h>
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <firmware.h>
#include <init.h>
#include <listmgmt.h>
#include <mode_select.h>
#include <notifications.h>
#include <plugin.h>
#include <runtime_gui.h>
#include <runtime_sliders.h>
#include <runtime_text.h>
#include <timeout_handlers.h>
#include <widgetmgmt.h>


extern gconstpointer *global_data;

G_MODULE_EXPORT void start_statuscounts_pf(void)
{
	start_tickler(SCOUNTS_TICKLER);
}


G_MODULE_EXPORT void enable_get_data_buttons_pf(void)
{
	gdk_threads_enter();
	g_list_foreach(get_list("get_data_buttons"),set_widget_sensitive,GINT_TO_POINTER(TRUE));
	gdk_threads_leave();
}


G_MODULE_EXPORT void conditional_start_rtv_tickler_pf(void)
{
	static gboolean just_starting = TRUE;

	if (just_starting)
	{
		start_tickler(RTV_TICKLER);
		just_starting = FALSE;
	}
}


G_MODULE_EXPORT void set_store_black_pf(void)
{
	gint j = 0;
	Firmware_Details *firmware = NULL;
	static void (*slaves_set_color_f)(gint,const gchar *) = NULL;

	firmware = DATA_GET(global_data,"firmware");
	if (!slaves_set_color_f)
		get_symbol("slaves_set_color",(void *)&slaves_set_color_f);

	gdk_threads_enter();
	set_group_color(BLACK,"burners");

	slaves_set_color_f(BLACK,"burners");
	for (j=0;j<firmware->total_tables;j++)
		set_reqfuel_color(BLACK,j);
	gdk_threads_leave();
}

G_MODULE_EXPORT void enable_3d_buttons_pf(void)
{
	gdk_threads_enter();
	g_list_foreach(get_list("3d_buttons"),set_widget_sensitive,GINT_TO_POINTER(TRUE));
	gdk_threads_leave();
}


G_MODULE_EXPORT void disable_burner_buttons_pf(void)
{
	gdk_threads_enter();
	g_list_foreach(get_list("burners"),set_widget_sensitive,GINT_TO_POINTER(FALSE));
	gdk_threads_leave();
}

G_MODULE_EXPORT void reset_temps_pf(void)
{
	gdk_threads_enter();
	set_title(g_strdup(_("Adjusting for local Temp units...")));
	reset_temps(DATA_GET(global_data,"temp_units"));
	set_title(g_strdup(_("Ready...")));
	gdk_threads_leave();
}

G_MODULE_EXPORT void ready_msg_pf(void)
{
	gdk_threads_enter();
	set_title(g_strdup(_("Ready...")));
	gdk_threads_leave();
}


G_MODULE_EXPORT void startup_default_timeouts_pf(void)
{
	gint source = 0;
	gint rate = 0;

	gdk_threads_enter();
	set_title(g_strdup(_("Starting up data renderers...")));
	gdk_threads_leave();
	rate = (GINT)DATA_GET(global_data,"rtslider_fps");
	source = g_timeout_add((gint)(1000.0/(gfloat)rate),(GSourceFunc)update_rtsliders,NULL);
	DATA_SET(global_data,"rtslider_id", GINT_TO_POINTER(source));

	rate = (GINT)DATA_GET(global_data,"rttext_fps");
	source = g_timeout_add((gint)(1000.0/(gfloat)rate),(GSourceFunc)update_rttext,NULL);
	DATA_SET(global_data,"rttext_id", GINT_TO_POINTER(source));

	rate = (GINT)DATA_GET(global_data,"dashboard_fps");
	source = g_timeout_add((gint)(1000.0/(gfloat)rate),(GSourceFunc)update_dashboards,NULL);
	DATA_SET(global_data,"dashboard_id", GINT_TO_POINTER(source));

	rate = (GINT)DATA_GET(global_data,"ve3d_fps");
	source = g_timeout_add((gint)(1000.0/(gfloat)rate),(GSourceFunc)update_ve3ds,NULL);
	DATA_SET(global_data,"ve3d_id", GINT_TO_POINTER(source));
}

