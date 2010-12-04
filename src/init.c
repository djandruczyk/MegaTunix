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
#include <configfile.h>
#include <conversions.h>
#include <defines.h>
#include <enums.h>
#include <debugging.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gui_handlers.h>
#include <init.h>
#include <interrogate.h>
#include <listmgmt.h>
#include <logviewer_gui.h>
#include <lookuptables.h>
#include <mtxmatheval.h>
#include <rtv_map_loader.h>
#include <runtime_sliders.h>
#include <runtime_text.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <widgetmgmt.h>
#include <unistd.h>

gint major_ver;
gint minor_ver;
gint micro_ver;
extern gint mem_view_style[];
extern gint ms_reset_count;
extern gint ms_goodread_count;
extern gboolean just_starting;
extern gint dbg_lvl;
extern Serial_Params *serial_params;
/* Support up to "x" page firmware.... */
GdkColor red = { 0, 65535, 0, 0};
GdkColor green = { 0, 0, 65535, 0};
GdkColor blue = { 0, 0, 0, 65535};
GdkColor black = { 0, 0, 0, 0};
GdkColor white = { 0, 65535, 65535, 65535};
GList ***ve_widgets = NULL;
GList **tab_gauges = NULL;
GHashTable **interdep_vars = NULL;
GHashTable *widget_group_states = NULL;
GHashTable *sources_hash = NULL;
GtkWidget **te_windows = NULL;
extern gconstpointer *global_data;


void dataset_dealloc(GQuark key_id,gpointer data, gpointer user_data);
/*!
 * init(void)
 * \brief Sets sane values to global variables for a clean startup of 
 * MegaTunix
 */
G_MODULE_EXPORT void init(void)
{
	/* defaults */
	GHashTable *table = NULL;
	GHashTable *commands = NULL;
	gboolean *hidden_list = NULL;
	GdkColormap *colormap = NULL;
	CmdLineArgs *args = NULL;
	gint i = 0;

	colormap = gdk_colormap_get_system ();
	args = DATA_GET(global_data,"args");
	gdk_colormap_alloc_color(colormap,&red,FALSE,TRUE);
	gdk_colormap_alloc_color(colormap,&green,FALSE,TRUE);
	gdk_colormap_alloc_color(colormap,&blue,FALSE,TRUE);
	gdk_colormap_alloc_color(colormap,&black,FALSE,TRUE);
	gdk_colormap_alloc_color(colormap,&white,FALSE,TRUE);
	hidden_list = g_new0(gboolean, 100); /*static, 100 max tabs... */
	for (i=0;i<100;i++)
		hidden_list[i]=FALSE;

	DATA_SET(global_data,"dash_fullscreen",GINT_TO_POINTER(FALSE));	/* Don't make dash fullscreen by default */
	DATA_SET(global_data,"gui_visible",GINT_TO_POINTER(TRUE));	/* Gui is visible on startup by default */
	DATA_SET(global_data,"main_visible",GINT_TO_POINTER(TRUE));	/* Gui is visible on startup by default */
	DATA_SET(global_data,"status_visible",GINT_TO_POINTER(TRUE));	/* Gui is visible on startup by default */
	DATA_SET(global_data,"rtt_visible",GINT_TO_POINTER(TRUE));	/* Gui is visible on startup by default */
	DATA_SET(global_data,"network_access",GINT_TO_POINTER(FALSE));	/* Disallow network connections by default */
	DATA_SET(global_data,"tips_in_use",GINT_TO_POINTER(TRUE));	/* Use tooltips by default */
	DATA_SET(global_data,"temp_units",GINT_TO_POINTER(FAHRENHEIT));/* Use SAE units by default */
	DATA_SET(global_data,"read_timeout",GINT_TO_POINTER(250));/* 250 ms */
	DATA_SET(global_data,"status_width",GINT_TO_POINTER(130));
	DATA_SET(global_data,"status_height",GINT_TO_POINTER(386));
	DATA_SET(global_data,"width",GINT_TO_POINTER(800));
	DATA_SET(global_data,"height",GINT_TO_POINTER(600));
	DATA_SET(global_data,"main_x_origin",GINT_TO_POINTER(160));
	DATA_SET(global_data,"main_y_origin",GINT_TO_POINTER(120));
	DATA_SET(global_data,"rtslider_fps",GINT_TO_POINTER(25));
	DATA_SET(global_data,"rttext_fps",GINT_TO_POINTER(15));
	DATA_SET(global_data,"dashboard_fps",GINT_TO_POINTER(30));
	DATA_SET(global_data,"ve3d_fps",GINT_TO_POINTER(20));
	DATA_SET(global_data,"last_page",GINT_TO_POINTER(-1));
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	DATA_SET(global_data,"rt_forced_update",GINT_TO_POINTER(TRUE));
	DATA_SET(global_data,"active_page",GINT_TO_POINTER(-1));
	DATA_SET(global_data,"active_table",GINT_TO_POINTER(-1));
	DATA_SET_FULL(global_data,"previous_ecu_family",g_strdup("MS-1"),cleanup);
	DATA_SET_FULL(global_data,"ecu_family",g_strdup("MS-1"),cleanup);
	DATA_SET_FULL(global_data,"hidden_list",hidden_list,cleanup);
	DATA_SET_FULL(global_data,"last_offline_profile",g_strdup(""),cleanup);
	DATA_SET_FULL(global_data,"last_offline_filename",g_strdup(""),cleanup);
	table = g_hash_table_new_full(g_str_hash,g_str_equal,cleanup,xml_arg_free);
	DATA_SET_FULL(global_data,"potential_arguments",(gpointer)table,g_hash_table_destroy);
	commands = g_hash_table_new_full(g_str_hash,g_str_equal,cleanup,xml_cmd_free);
	DATA_SET_FULL(global_data,"commands_hash",commands,g_hash_table_destroy);

	/* initialize all global variables to known states */
	cleanup(DATA_GET(global_data,"potential_ports"));
#ifdef __WIN32__
	if (!args->port)
	{
		DATA_SET(global_data,"autodetect_port",GINT_TO_POINTER(TRUE));
		DATA_SET_FULL(global_data,"override_port",g_strdup("COM1"),cleanup);
	}
	DATA_SET_FULL(global_data,"potential_ports",g_strdup("COM1,COM2,COM3,COM4,COM5,COM6,COM7,COM8,COM9,COM10"),cleanup);
#else
	if (!args->port)
	{
		DATA_SET(global_data,"autodetect_port",GINT_TO_POINTER(TRUE));
		DATA_SET_FULL(global_data,"override_port",g_strdup("/dev/ttyS0"),cleanup);
	}
	DATA_SET_FULL(global_data,"potential_ports", g_strdup("/dev/ttyUSB0,/dev/ttyS0,/dev/ttyUSB1,/dev/ttyS1,/dev/ttyUSB2,/dev/ttyS2,/dev/ttyUSB3,/dev/ttyS3,/tmp/virtual-serial"),cleanup);
#endif
	serial_params->fd = 0; /* serial port file-descriptor */

	serial_params->errcount = 0; /* I/O error count */
	/* default for MS v1.x and 2.x */
	serial_params->read_wait = 50;	/* delay between reads in milliseconds */

	/* Set flags to clean state */
	just_starting = TRUE; 	/* to handle initial errors */
	ms_reset_count = 0; 	/* Counts MS clock resets */
	ms_goodread_count = 0; 	/* How many reads of realtime vars completed */
	DATA_SET(global_data,"preferred_delimiter",GINT_TO_POINTER(TAB));


	if (!widget_group_states)
		widget_group_states = g_hash_table_new_full(g_str_hash,g_str_equal,cleanup,NULL);
		g_hash_table_insert(widget_group_states,g_strdup("temperature"),(gpointer)TRUE);
		g_hash_table_insert(widget_group_states,g_strdup("multi_expression"),(gpointer)TRUE);
}


/*!
 * read_config(void)
 * \brief Reads state of various control variables, like window position
 * size, serial port and parameters and other user defaults from the default
 * config file located at ~/.MegaTunix/config
 * \see save_config(void)
 */
G_MODULE_EXPORT gboolean read_config(void)
{
	gint tmpi = 0;
	guint i = 0;
	gfloat tmpf = 0.0;
	gchar * tmpbuf = NULL;
	gchar **vector = NULL;
	ConfigFile *cfgfile;
	gchar *filename = NULL;
	gboolean *hidden_list;
	CmdLineArgs *args = NULL;

	filename = g_strconcat(HOME(), PSEP,".MegaTunix",PSEP,"config", NULL);
	args = DATA_GET(global_data,"args");
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		if(cfg_read_boolean(cfgfile, "Global", "Tooltips", &tmpi))
			DATA_SET(global_data,"tips_in_use",GINT_TO_POINTER(tmpi));
//		if(cfg_read_boolean(cfgfile, "Global", "NetworkAccess", &tmpi))
//			DATA_SET(global_data,"network_access",GINT_TO_POINTER(tmpi));
		if(cfg_read_string(cfgfile, "Global", "Previous_ECU_Family", &tmpbuf))
		{
			DATA_SET_FULL(global_data,"previous_ecu_family",g_strdup(tmpbuf),cleanup);
			cleanup(tmpbuf);
		}
		if(cfg_read_int(cfgfile, "Global", "Temp_Scale", &tmpi))
			DATA_SET(global_data,"temp_units",GINT_TO_POINTER(tmpi));
		if(cfg_read_int(cfgfile, "Global", "RTSlider_FPS", &tmpi))
			DATA_SET(global_data,"rtslider_fps",GINT_TO_POINTER(tmpi));
		if(cfg_read_int(cfgfile, "Global", "RTText_FPS", &tmpi))
			DATA_SET(global_data,"rttext_fps",GINT_TO_POINTER(tmpi));
		if(cfg_read_int(cfgfile, "Global", "Dashboard_FPS", &tmpi))
			DATA_SET(global_data,"dashboard_fps",GINT_TO_POINTER(tmpi));
		if(cfg_read_int(cfgfile, "Global", "VE3D_FPS", &tmpi))
			DATA_SET(global_data,"ve3d_fps",GINT_TO_POINTER(tmpi));
		cfg_read_int(cfgfile, "Global", "dbg_lvl", &dbg_lvl);
		if(cfg_read_string(cfgfile, "Global", "last_offline_profile", &tmpbuf))
		{
			DATA_SET_FULL(global_data,"last_offline_profile",g_strdup(tmpbuf),cleanup);
			cleanup(tmpbuf);
		}
		if(cfg_read_string(cfgfile, "Global", "last_offline_filename", &tmpbuf))
		{
			DATA_SET_FULL(global_data,"last_offline_filename",g_strdup(tmpbuf),cleanup);
			cleanup(tmpbuf);
		}
		if ((cfg_read_string(cfgfile, "Dashboards", "dash_1_name", &tmpbuf)) && (strlen(tmpbuf) != 0))
		{
			DATA_SET_FULL(global_data,"dash_1_name",g_strdup(tmpbuf),cleanup);
			cleanup(tmpbuf);
		}
		if (cfg_read_int(cfgfile, "Dashboards", "dash_1_x_origin", &tmpi))
			DATA_SET(global_data,"dash_1_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Dashboards", "dash_1_y_origin", &tmpi))
			DATA_SET(global_data,"dash_1_y_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_float(cfgfile, "Dashboards", "dash_1_size_ratio", &tmpf))
			DATA_SET_FULL(global_data,"dash_1_size_ratio",g_memdup(&tmpf,sizeof(gfloat)),cleanup);
		if ((cfg_read_string(cfgfile, "Dashboards", "dash_2_name", &tmpbuf)) && (strlen(tmpbuf) != 0))
		{
			DATA_SET_FULL(global_data,"dash_2_name",g_strdup(tmpbuf),cleanup);
			cleanup(tmpbuf);
		}
		if (cfg_read_int(cfgfile, "Dashboards", "dash_2_x_origin", &tmpi))
			DATA_SET(global_data,"dash_2_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Dashboards", "dash_2_y_origin", &tmpi))
			DATA_SET(global_data,"dash_2_y_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_float(cfgfile, "Dashboards", "dash_2_size_ratio", &tmpf))
			DATA_SET_FULL(global_data,"dash_2_size_ratio",g_memdup(&tmpf,sizeof(gfloat)),cleanup);
		if (cfg_read_int(cfgfile, "DataLogger", "preferred_delimiter", &tmpi))
			DATA_SET(global_data,"preferred_delimiter",GINT_TO_POINTER(tmpi));	
		if (args->network_mode)
			DATA_SET(global_data,"read_timeout",GINT_TO_POINTER(250));
		else
		{
			if (cfg_read_int(cfgfile, "Serial", "read_timeout", &tmpi))
				DATA_SET(global_data,"read_timeout",GINT_TO_POINTER(tmpi));
		}
		if (cfg_read_int(cfgfile, "Window", "status_width", &tmpi))
			DATA_SET(global_data,"status_width",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "status_height", &tmpi))
			DATA_SET(global_data,"status_height",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "status_x_origin", &tmpi))
			DATA_SET(global_data,"status_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "status_y_origin", &tmpi))
			DATA_SET(global_data,"status_y_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "rtt_x_origin", &tmpi))
			DATA_SET(global_data,"rtt_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "rtt_y_origin", &tmpi))
			DATA_SET(global_data,"rtt_y_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "width", &tmpi))
			DATA_SET(global_data,"width",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "height", &tmpi))
			DATA_SET(global_data,"height",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "main_x_origin", &tmpi))
			DATA_SET(global_data,"main_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "main_y_origin", &tmpi))
			DATA_SET(global_data,"main_y_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_string(cfgfile, "Window", "hidden_tabs_list", &tmpbuf))
		{
			hidden_list = (gboolean *)DATA_GET(global_data,"hidden_list");
			vector = g_strsplit(tmpbuf,",",-1);
			for (i=0;i<g_strv_length(vector);i++)
				hidden_list[atoi(vector[i])] = TRUE;
			g_strfreev(vector);
			cleanup(tmpbuf);
		}

		if (cfg_read_string(cfgfile, "Serial", "potential_ports", &tmpbuf))
		{
			DATA_SET_FULL(global_data,"potential_ports",g_strdup(tmpbuf),cleanup);
			cleanup(tmpbuf);
		}
		if (!args->port)
		{
			if (cfg_read_string(cfgfile, "Serial", "override_port", &tmpbuf))
			{
				DATA_SET_FULL(global_data,"override_port",g_strdup(tmpbuf),cleanup);
				cleanup(tmpbuf);
			}
			if(cfg_read_boolean(cfgfile, "Serial", "autodetect_port",&tmpi))
				DATA_SET(global_data,"autodetect_port",GINT_TO_POINTER(tmpi));
		}

		cfg_read_int(cfgfile, "Serial", "read_wait", 
				&serial_params->read_wait);
		if(cfg_read_int(cfgfile, "Logviewer", "zoom", &tmpi))
			DATA_SET(global_data,"lv_zoom",GINT_TO_POINTER(tmpi));
		read_logviewer_defaults(cfgfile);

		if ((GINT)DATA_GET(global_data,"lv_zoom") < 1)
			DATA_SET(global_data,"lv_zoom",GINT_TO_POINTER(1));
		if(cfg_read_int(cfgfile, "Logviewer", "scroll_delay", &tmpi))
			DATA_SET(global_data,"lv_scroll_delay",GINT_TO_POINTER(tmpi));
		if ((GINT)DATA_GET(global_data,"lv_scroll_delay") < 40)
			DATA_SET(global_data,"lv_scroll_delay",GINT_TO_POINTER(100));
		cfg_read_int(cfgfile, "MemViewer", "page0_style", &mem_view_style[0]);
		cfg_read_int(cfgfile, "MemViewer", "page1_style", &mem_view_style[1]);
		cfg_read_int(cfgfile, "MemViewer", "page2_style", &mem_view_style[2]);
		cfg_read_int(cfgfile, "MemViewer", "page3_style", &mem_view_style[3]);
		cfg_free(cfgfile);
		cleanup(filename);
		return TRUE;
	}
	else
	{
		serial_params->port_name = g_strdup(DEFAULT_PORT);
		dbg_func(CRITICAL,g_strdup(__FILE__": read_config()\n\tConfig file not found, using defaults\n"));
		cleanup(filename);
		save_config();
		return FALSE;	/* No file found */
	}
	return TRUE;
}


/*!
 * save_config(void)
 * \brief Saves state of various control variables, like window position
 * size, serial port and parameters and other user defaults
 * \see read_config(void)
 */
G_MODULE_EXPORT void save_config(void)
{
	gchar *filename = NULL;
	gchar * tmpbuf = NULL;
	GtkWidget *widget = NULL;
	GtkWidget *main_window = NULL;
	int x = 0;
	int y = 0;
	int i = 0;
	int count = 0;
	int tmp_width = 0;
	int tmp_height = 0;
	int orig_width = 0;
	int orig_height = 0;
	gint total = 0;
	gfloat ratio = 0.0;
	GtkWidget *dash = NULL;
	ConfigFile *cfgfile = NULL;
	gboolean * hidden_list;
	GString *string = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	filename = g_strconcat(HOME(), "/.MegaTunix/config", NULL);
	cfgfile = cfg_open_file(filename);

	if (!cfgfile)
		cfgfile = cfg_new();


	cfg_write_int(cfgfile, "Global", "major_ver", _MAJOR_);
	cfg_write_int(cfgfile, "Global", "minor_ver", _MINOR_);
	cfg_write_int(cfgfile, "Global", "micro_ver", _MICRO_);
	cfg_write_boolean(cfgfile, "Global", "Tooltips",(GBOOLEAN)DATA_GET(global_data,"tips_in_use"));
	cfg_write_boolean(cfgfile, "Global", "NetworkAccess",(GBOOLEAN)DATA_GET(global_data,"network_access"));
		
	cfg_write_int(cfgfile, "Global", "Temp_Scale", (GINT)DATA_GET(global_data,"temp_units"));
	cfg_write_string(cfgfile, "Global", "Previous_ECU_Family",DATA_GET(global_data,"ecu_family"));
	cfg_write_int(cfgfile, "Global", "RTSlider_FPS", (GINT)DATA_GET(global_data,"rtslider_fps"));
	cfg_write_int(cfgfile, "Global", "RTText_FPS", (GINT)DATA_GET(global_data,"rttext_fps"));
	cfg_write_int(cfgfile, "Global", "Dashboard_FPS", (GINT)DATA_GET(global_data,"dashboard_fps"));
	cfg_write_int(cfgfile, "Global", "VE3D_FPS", (GINT)DATA_GET(global_data,"ve3d_fps"));
	cfg_write_int(cfgfile, "Global", "dbg_lvl", dbg_lvl);
	if (DATA_GET(global_data,"last_offline_profile"))
		cfg_write_string(cfgfile, "Global", "last_offline_profile", DATA_GET(global_data,"last_offline_profile"));
	if (DATA_GET(global_data,"last_offline_filename"))
		cfg_write_string(cfgfile, "Global", "last_offline_filename", DATA_GET(global_data,"last_offline_filename"));
	cfg_write_int(cfgfile, "Serial", "read_timeout", (GINT)DATA_GET(global_data,"read_timeout"));
	tmpbuf = DATA_GET(global_data,"dash_1_name");
	if ((tmpbuf) && (strlen(tmpbuf) != 0 ))
	{
		cfg_write_string(cfgfile, "Dashboards", "dash_1_name", tmpbuf);
		widget =  lookup_widget(tmpbuf);
		if (GTK_IS_WIDGET(widget))
		{
			gtk_window_get_position(GTK_WINDOW(widget),&x,&y);
			cfg_write_int(cfgfile, "Dashboards", "dash_1_x_origin", x);
			cfg_write_int(cfgfile, "Dashboards", "dash_1_y_origin", y);
			dash = OBJ_GET(widget,"dash");
			orig_width = (GINT) OBJ_GET(dash,"orig_width");
		        orig_height = (GINT) OBJ_GET(dash,"orig_height");
			if (GTK_WIDGET_VISIBLE(widget))
			{
				gdk_drawable_get_size(gtk_widget_get_toplevel(widget)->window, &tmp_width,&tmp_height);
				ratio = (((gfloat)tmp_height/(gfloat)orig_height)+((gfloat)tmp_width/(gfloat)orig_width))/2.0;
				cfg_write_float(cfgfile, "Dashboards", "dash_1_size_ratio", ratio);
			}
		}
		tmpbuf = NULL;
	}
	else
	{
		cfg_remove_key(cfgfile, "Dashboards", "dash_1_name");
		cfg_remove_key(cfgfile, "Dashboards", "dash_1_x_origin");
		cfg_remove_key(cfgfile, "Dashboards", "dash_1_y_origin");
		cfg_remove_key(cfgfile, "Dashboards", "dash_1_size_ratio");
	}
	tmpbuf = DATA_GET(global_data,"dash_2_name");
	if ((tmpbuf) && (strlen(tmpbuf) != 0 ))
	{
		cfg_write_string(cfgfile, "Dashboards", "dash_2_name", tmpbuf);
		widget =  lookup_widget(tmpbuf);
		if (GTK_IS_WIDGET(widget))
		{
			gtk_window_get_position(GTK_WINDOW(widget),&x,&y);
			cfg_write_int(cfgfile, "Dashboards", "dash_2_x_origin", x);
			cfg_write_int(cfgfile, "Dashboards", "dash_2_y_origin", y);
			dash = OBJ_GET(widget,"dash");
			orig_width = (GINT) OBJ_GET(dash,"orig_width");
		        orig_height = (GINT) OBJ_GET(dash,"orig_height");
			if (GTK_WIDGET_VISIBLE(widget))
			{
				gdk_drawable_get_size(gtk_widget_get_toplevel(widget)->window, &tmp_width,&tmp_height);
				ratio = (((gfloat)tmp_height/(gfloat)orig_height)+((gfloat)tmp_width/(gfloat)orig_width))/2.0;
				cfg_write_float(cfgfile, "Dashboards", "dash_2_size_ratio", ratio);
			}
		}
		tmpbuf = NULL;
	}
	else
	{
		cfg_remove_key(cfgfile, "Dashboards", "dash_2_name");
		cfg_remove_key(cfgfile, "Dashboards", "dash_2_x_origin");
		cfg_remove_key(cfgfile, "Dashboards", "dash_2_y_origin");
		cfg_remove_key(cfgfile, "Dashboards", "dash_2_size_ratio");
	}
				

	if (DATA_GET(global_data,"ready"))
	{
		main_window = lookup_widget("main_window");
		if (GTK_WIDGET_VISIBLE(main_window))
		{
			gdk_drawable_get_size(main_window->window, &tmp_width,&tmp_height);
			cfg_write_int(cfgfile, "Window", "width", tmp_width);
			cfg_write_int(cfgfile, "Window", "height", tmp_height);
			gtk_window_get_position(GTK_WINDOW(main_window),&x,&y);
			if (x > 0)
				cfg_write_int(cfgfile, "Window", "main_x_origin", x);
			if (y > 0)
				cfg_write_int(cfgfile, "Window", "main_y_origin", y);
		}
		widget = lookup_widget("status_window");
		if (widget)
		{
			if ((GTK_IS_WIDGET(widget)) && (GTK_WIDGET_VISIBLE(widget)))
			{
				gdk_drawable_get_size(widget->window, &tmp_width,&tmp_height);

				cfg_write_int(cfgfile, "Window", "status_width", tmp_width);
				cfg_write_int(cfgfile, "Window", "status_height", tmp_height);
				gtk_window_get_position(GTK_WINDOW(widget),&x,&y);
				if (x > 0)
					cfg_write_int(cfgfile, "Window", "status_x_origin", x);
				if (y > 0)
					cfg_write_int(cfgfile, "Window", "status_y_origin", y);
			}
		}
		widget = lookup_widget("rtt_window");
		if (widget)
		{
			if ((GTK_IS_WIDGET(widget)) && (GTK_WIDGET_VISIBLE(widget)))
			{
				gtk_window_get_position(GTK_WINDOW(widget),&x,&y);
				if (x > 0)
					cfg_write_int(cfgfile, "Window", "rtt_x_origin", x);
				if (y > 0)
					cfg_write_int(cfgfile, "Window", "rtt_y_origin", y);
			}
		}
		widget = lookup_widget("toplevel_notebook");
		total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(widget));
		hidden_list = (gboolean *)DATA_GET(global_data,"hidden_list");
		string = g_string_new(NULL);
		for (i=0;i<total;i++)
		{
			if (hidden_list[i] == FALSE)
				continue;
			if (count == 0)
				g_string_printf(string,"%i",i);
			else
				g_string_append_printf(string,",%i",i);
			count++;
		}
		tmpbuf = g_strndup(string->str,string->len);
		g_string_free(string,TRUE);
		cfg_write_string(cfgfile, "Window", "hidden_tabs_list", tmpbuf);
		cleanup(tmpbuf);

	}
	cfg_write_int(cfgfile, "DataLogger", "preferred_delimiter", (gint)DATA_GET(global_data,"preferred_delimiter"));
	if (serial_params->port_name)
		cfg_write_string(cfgfile, "Serial", "override_port", 
					serial_params->port_name);
	cfg_write_string(cfgfile, "Serial", "potential_ports", 
				(gchar *)DATA_GET(global_data,"potential_ports"));
	cfg_write_boolean(cfgfile, "Serial", "autodetect_port", 
				(GBOOLEAN)DATA_GET(global_data,"autodetect_port"));
	cfg_write_int(cfgfile, "Serial", "read_wait", 
			serial_params->read_wait);
			
	cfg_write_int(cfgfile, "Logviewer", "zoom", (GINT)DATA_GET(global_data,"lv_zoom"));
	cfg_write_int(cfgfile, "Logviewer", "scroll_delay",(GINT) DATA_GET(global_data,"lv_scroll_delay"));
	write_logviewer_defaults(cfgfile);

	cfg_write_int(cfgfile, "MemViewer", "page0_style", mem_view_style[0]);
	cfg_write_int(cfgfile, "MemViewer", "page1_style", mem_view_style[1]);
	cfg_write_int(cfgfile, "MemViewer", "page2_style", mem_view_style[2]);
	cfg_write_int(cfgfile, "MemViewer", "page3_style", mem_view_style[3]);

	cfg_write_file(cfgfile, filename);
	cfg_free(cfgfile);
	cleanup(filename);
	g_static_mutex_unlock(&mutex);
}


/*!
 * make_megasquirt_dirs(void)
 * \brief Creates the directories for user modified config files in the
 * users home directory under ~/.MegaTunix
 */
G_MODULE_EXPORT void make_megasquirt_dirs(void)
{
	gchar *filename = NULL;
	const gchar *mtx = ".MegaTunix";

	filename = g_strconcat(HOME(), "/.MegaTunix", NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,GUI_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,GAUGES_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,DASHES_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,INTERROGATOR_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,INTERROGATOR_DATA_DIR,PSEP,"Profiles", NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,LOOKUPTABLES_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,REALTIME_MAPS_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,RTSLIDERS_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,RTSTATUS_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	cleanup(filename);

	return;
}


/*!
 * mem_alloc(void)
 * \brief Allocates memory allocated, to be deallocated at close by mem_dalloc
 * \see mem_dealloc
 */
G_MODULE_EXPORT void mem_alloc(void)
{
	gint i=0;
	gint j=0;
	gint *algorithm = NULL;
	gboolean *tracking_focus = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	if (!firmware->rt_data)
		firmware->rt_data = g_new0(guint8, firmware->rtvars_size);
	if (!firmware->rt_data_last)
		firmware->rt_data_last = g_new0(guint8, firmware->rtvars_size);
	if (!firmware->ecu_data)
		firmware->ecu_data = g_new0(guint8 *, firmware->total_pages);
	if (!firmware->ecu_data_last)
		firmware->ecu_data_last = g_new0(guint8 *, firmware->total_pages);
	if (!firmware->ecu_data_backup)
		firmware->ecu_data_backup = g_new0(guint8 *, firmware->total_pages);
	if (!te_windows)
		te_windows = g_new0(GtkWidget *, firmware->total_te_tables);

	if (!ve_widgets)
		ve_widgets = g_new0(GList **, firmware->total_pages);
	if (!tab_gauges)
		tab_gauges = g_new0(GList *, firmware->total_tables);
	if (!sources_hash)
		sources_hash = g_hash_table_new_full(g_str_hash,g_str_equal,cleanup,cleanup);
	/* Hash tables to store the interdependant deferred variables before
	 * download...
	 */
	if (!interdep_vars)
		interdep_vars = g_new0(GHashTable *,firmware->total_tables);
	if (!algorithm)
	{
		algorithm = g_new0(gint, firmware->total_tables);
		DATA_SET_FULL(global_data,"algorithm",algorithm,g_free);
	}
	if (!tracking_focus)
	{
		tracking_focus = g_new0(gboolean, firmware->total_tables);
		DATA_SET_FULL(global_data,"tracking_focus",tracking_focus,g_free);
	}

	for (i=0;i<firmware->total_tables;i++)
	{
		tab_gauges[i] = NULL;
		algorithm[i] = SPEED_DENSITY;
		interdep_vars[i] = g_hash_table_new(NULL,NULL);
	}

	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->ecu_data[i])
			firmware->ecu_data[i] = g_new0(guint8, firmware->page_params[i]->length);
		if (!firmware->ecu_data_last[i])
			firmware->ecu_data_last[i] = g_new0(guint8, firmware->page_params[i]->length);
		if (!firmware->ecu_data_backup[i])
			firmware->ecu_data_backup[i] = g_new0(guint8, firmware->page_params[i]->length);
		if (!ve_widgets[i])
		{
			ve_widgets[i] = g_new0(GList *, firmware->page_params[i]->length);
			for (j=0;j<firmware->page_params[i]->length;j++)
			{
				ve_widgets[i][j] = NULL;
			}
		}
	}

}


/*!
 * mem_dealloc(void)
 * \brief Deallocates memory allocated with mem_alloc
 * \see mem_alloc
 */
G_MODULE_EXPORT void mem_dealloc(void)
{
	gint i = 0;
	gint j = 0;
	gpointer data;
	GtkListStore *store = NULL;
	GList *defaults = NULL;
	extern GHashTable *dynamic_widgets;
	extern Rtv_Map *rtv_map;
	extern GStaticMutex serio_mutex;
	extern GStaticMutex rtt_mutex;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	g_static_mutex_lock(&serio_mutex);

	cleanup(serial_params->port_name);
	cleanup(serial_params);
	g_static_mutex_unlock(&serio_mutex);

	/* Firmware datastructure.... */
	if (firmware)
	{
		for (i=0;i<firmware->total_pages;i++)
		{
			if (ve_widgets[i])
			{
				for (j=0;j<firmware->page_params[i]->length;j++)
				{
					g_list_foreach(ve_widgets[i][j],dealloc_widget,NULL);
					g_list_free(ve_widgets[i][j]);
				}

			}
			cleanup(ve_widgets[i]);
		}
		cleanup (firmware->name);
		cleanup (firmware->profile_filename);
		cleanup (firmware->actual_signature);
		cleanup (firmware->text_revision);
		g_strfreev (firmware->tab_list);
		g_strfreev (firmware->tab_confs);
		cleanup (firmware->rtv_map_file);
		cleanup (firmware->sliders_map_file);
		cleanup (firmware->rtt_map_file);
		cleanup (firmware->status_map_file);
		cleanup (firmware->rt_command);
		cleanup (firmware->get_all_command);
		cleanup (firmware->ve_command);
		cleanup (firmware->write_command);
		cleanup (firmware->table_write_command);
		cleanup (firmware->chunk_write_command);
		cleanup (firmware->burn_all_command);
		cleanup (firmware->burn_command);
		cleanup (firmware->raw_mem_command);
		cleanup (firmware->page_command);
		cleanup (firmware->SignatureVia);
		cleanup (firmware->TextVerVia);
		cleanup (firmware->NumVerVia);

		for (i=0;i<firmware->total_pages;i++)
		{
			cleanup(firmware->ecu_data[i]);
			cleanup(firmware->ecu_data_last[i]);
			cleanup(firmware->ecu_data_backup[i]);
			cleanup(firmware->page_params[i]);
		}
		cleanup(firmware->ecu_data);
		cleanup(firmware->ecu_data_last);
		cleanup(firmware->ecu_data_backup);
		cleanup(firmware->page_params);

		for (i=0;i<firmware->total_te_tables;i++)
		{
			if (firmware->te_params[i])
				dealloc_te_params(firmware->te_params[i]);
		}
		cleanup(firmware->te_params);

		for (i=0;i<firmware->total_tables;i++)
		{
			if (firmware->table_params[i])
				dealloc_table_params(firmware->table_params[i]);
			cleanup (firmware->rf_params[i]);
			if (interdep_vars[i])
			{
				g_hash_table_destroy(interdep_vars[i]);
				interdep_vars[i] = NULL;
			}
		}
		cleanup(firmware->table_params);
		cleanup(interdep_vars);
		cleanup(firmware->rf_params);
		cleanup(firmware->rt_data);
		cleanup(firmware->rt_data_last);
		cleanup(firmware);
	}
	if(widget_group_states)
		g_hash_table_destroy(widget_group_states);
	if(sources_hash)
		g_hash_table_destroy(sources_hash);
	if (rtv_map)
	{
		if (rtv_map->raw_list)
			g_strfreev(rtv_map->raw_list);
		cleanup (rtv_map->applicable_signatures);
		g_array_free(rtv_map->ts_array,TRUE);
		for(i=0;i<rtv_map->rtv_list->len;i++)
		{
			data = g_ptr_array_index(rtv_map->rtv_list,i);
			dealloc_rtv_object(data);
		}
		g_hash_table_destroy(rtv_map->rtv_hash);
		g_hash_table_destroy(rtv_map->offset_hash);
		g_ptr_array_free(rtv_map->rtv_list,TRUE);
		cleanup(rtv_map);
	}
	/* Runtime Text*/
	g_static_mutex_lock(&rtt_mutex);
	store = DATA_GET(global_data,"rtt_model");
	if (store)
		gtk_tree_model_foreach(GTK_TREE_MODEL(store),dealloc_rtt_model,NULL);

	/* Logviewer settings */
	defaults = get_list("logviewer_defaults");
	if (defaults)
		g_list_foreach(defaults,(GFunc)cleanup,NULL);
	/* Free all global data and structures */
	g_dataset_destroy(global_data);
	cleanup(global_data);
	//g_dataset_foreach(global_data,dataset_dealloc,NULL);
	/* Dynamic widgets master hash  */

	if (dynamic_widgets)
		g_hash_table_destroy(dynamic_widgets);
}


G_MODULE_EXPORT void dataset_dealloc(GQuark key_id,gpointer data, gpointer user_data)
{

	printf("going to free %s\n",g_quark_to_string(key_id));
	g_dataset_remove_data(global_data,g_quark_to_string(key_id));
	/* This should trigger a bug at some point */
}



/*!
 \brief initialize_io_message() allocates and initializes a pointer
 to a Io_Message datastructure,  used for passing messages 
 across the GAsyncQueue's between the threads and the main context
 \returns a allocated and initialized pointer to a single structure
 */
G_MODULE_EXPORT Io_Message * initialize_io_message(void)
{
	Io_Message *message = NULL;

	message = g_new0(Io_Message, 1);
	message->functions = NULL;
	message->command = NULL;
	message->sequence = NULL;
	message->payload = NULL;
	message->recv_buf = NULL;
	message->status = TRUE;

	return message;
}


G_MODULE_EXPORT OutputData * initialize_outputdata(void)
{
	OutputData *output = NULL;

	output = g_new0(OutputData, 1);
	output->data = g_new0(gconstpointer, 1);
	return output;
}

/*!
 *  \brief initialize_page_params() creates and initializes the page_params
 *   datastructure to sane defaults and returns it
 *    */
G_MODULE_EXPORT Page_Params * initialize_page_params(void)
{
	Page_Params *page_params = NULL;
	page_params = g_malloc0(sizeof(Page_Params));
	page_params->length = 0;
	page_params->spconfig_offset = -1;
	return page_params;
}

         
/*!
 *  \brief initialize_table_params() creates and initializes the Table_Params
 *   datastructure to sane defaults and returns it
 *    */
G_MODULE_EXPORT Table_Params * initialize_table_params(void)
{
	Table_Params *table_params = NULL;
	table_params = g_malloc0(sizeof(Table_Params));
	table_params->table = g_array_sized_new(FALSE,TRUE,sizeof(GtkWidget *),36);
	table_params->is_fuel = FALSE;
	table_params->alternate_offset = -1;
	table_params->divider_offset = -1;
	table_params->rpmk_offset = -1;
	table_params->reqfuel_offset = -1;
	table_params->x_page = -1;
	table_params->y_page = -1;
	table_params->z_page = -1;
	table_params->x_base = -1;
	table_params->y_base = -1;
	table_params->z_base = -1;
	table_params->x_bincount = -1;
	table_params->y_bincount = -1;
	table_params->x_precision = 0;
	table_params->y_precision = 0;
	table_params->z_precision = 0;
	table_params->bind_to_list = NULL;
	table_params->x_suffix = NULL;
	table_params->y_suffix = NULL;
	table_params->z_suffix = NULL;
	table_params->x_ul_conv_expr = NULL;
	table_params->x_dl_conv_expr = NULL;
	table_params->y_ul_conv_expr = NULL;
	table_params->y_dl_conv_expr = NULL;
	table_params->z_ul_conv_expr = NULL;
	table_params->z_dl_conv_expr = NULL;
	table_params->x_ul_conv_exprs = NULL;
	table_params->x_dl_conv_exprs = NULL;
	table_params->y_ul_conv_exprs = NULL;
	table_params->y_dl_conv_exprs = NULL;
	table_params->z_ul_conv_exprs = NULL;
	table_params->z_dl_conv_exprs = NULL;
	table_params->x_source_key = NULL;
	table_params->y_source_key = NULL;
	table_params->z_source_key = NULL;
	table_params->table_name = NULL;
	table_params->x_ul_eval = NULL;
	table_params->y_ul_eval = NULL;
	table_params->z_ul_eval = NULL;
	table_params->x_dl_eval = NULL;
	table_params->y_dl_eval = NULL;
	table_params->z_dl_eval = NULL;

	return table_params;
}

         
/*!
 *  \brief initialize_te_params() creates and initializes the TE_Params
 *   datastructure to sane defaults and returns it
 *    */
G_MODULE_EXPORT TE_Params * initialize_te_params(void)
{
	TE_Params *te_params = NULL;
	te_params = g_malloc0(sizeof(TE_Params));
	te_params->x_lock = FALSE;
	te_params->y_lock = FALSE;
	te_params->x_use_color = FALSE;
	te_params->y_use_color = FALSE;
	te_params->x_temp_dep = FALSE;
	te_params->y_temp_dep = FALSE;
	te_params->x_page = -1;
	te_params->y_page = -1;
	te_params->x_base = -1;
	te_params->y_base = -1;
	te_params->reversed = FALSE;
	te_params->bincount = -1;
	te_params->x_precision = 0;
	te_params->y_precision = 0;
	te_params->x_axis_label = NULL;
	te_params->y_axis_label = NULL;
	te_params->x_name = NULL;
	te_params->y_name = NULL;
	te_params->x_units = NULL;
	te_params->y_units = NULL;
	te_params->x_dl_conv_expr = NULL;
	te_params->x_ul_conv_expr = NULL;
	te_params->y_dl_conv_expr = NULL;
	te_params->y_ul_conv_expr = NULL;
	te_params->gauge_temp_dep = FALSE;
	te_params->title = NULL;

	return te_params;
}


/*!
 \brief dealloc_client_data() deallocates the structure used for MTX TCP/IP
sockets
*/
G_MODULE_EXPORT void dealloc_client_data(MtxSocketClient *client)
{
	gint i = 0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	/*printf("dealloc_client_data\n");*/
	if (client)
	{
		cleanup (client->ip);

		if (client->ecu_data)
		{
			for (i=0;i<firmware->total_pages;i++)
				cleanup (client->ecu_data[i]);
			cleanup(client->ecu_data);
		}
		cleanup(client);
	}
}

/*!
 \brief dealloc_message() deallocates the structure used to pass an I/O
 message from a thread to here..
 \param message (Io_Message *) pointer to message data
 */
G_MODULE_EXPORT void dealloc_message(Io_Message * message)
{
	OutputData *payload;
	/*printf("dealloc_message\n");*/
	if (message->functions)
		dealloc_array(message->functions, FUNCTIONS);
	message->functions = NULL;
	if (message->sequence)
		dealloc_array(message->sequence, SEQUENCE);
	message->sequence = NULL;
	cleanup (message->recv_buf);
	if (message->command)
		if (message->command->type == NULL_CMD)
			cleanup(message->command);
	message->command = NULL;
        if (message->payload)
	{
		payload = (OutputData *)message->payload;
		if (payload->data)
		{
			g_dataset_destroy(payload->data);
			cleanup(payload->data);
		}
                cleanup(message->payload);
	}
        cleanup(message);
}


G_MODULE_EXPORT void dealloc_array(GArray *array, ArrayType type)
{
	DBlock *db = NULL;
	PotentialArg *arg = NULL;
	PostFunction *post = NULL;
	guint i = 0;

	/*printf("dealloc_array\n");*/
	switch (type)
	{
		case FUNCTIONS:
			g_array_free(array,TRUE);
			break;
		case POST_FUNCTIONS:
			for (i=0;i<array->len;i++)
			{
				post = NULL;
				post = g_array_index(array,PostFunction *,i);
				if (!post)
					continue;
				cleanup(post->name);
				cleanup(post);
			}
			g_array_free(array,TRUE);
			break;
		case SEQUENCE:
			for (i=0;i<array->len;i++)
			{
				db = NULL;
				db = g_array_index(array,DBlock *,i);
				if (!db)
					continue;
				cleanup (db->data);
				cleanup(db);
			}
			g_array_free(array,TRUE);
			break;
		case ARGS:
			for (i=0;i<array->len;i++)
			{
				arg = NULL;
				arg = g_array_index(array,PotentialArg *,i);
				if (!arg)
					continue;
				printf ("arg->name :%s\n",arg->name);
				cleanup (arg->name);
				printf ("arg->desc :%s\n",arg->desc);
				cleanup (arg->desc);
				printf ("arg->internal_name :%s\n",arg->internal_name);
				cleanup (arg->internal_name);
				cleanup (arg->static_string);
				cleanup(arg);
			}
			g_array_free(array,TRUE);
			break;
	}
}


/*!
 \brief dealloc_w_update() deallocates the structure used to pass an I/O
 widget update message from a thread to here..
 \param w_update (Widget_Update *) pointer to message data
 */
G_MODULE_EXPORT void dealloc_w_update(Widget_Update * w_update)
{
	/*printf("dealloc_w_update\n");*/
        cleanup (w_update->msg);
        cleanup (w_update);
}


/*!
 \brief dealloc_textmessage() deallocates the structure used to pass a text
 message from the thread to here..
 \param message (Text_Message *) pointer to message data
 */
G_MODULE_EXPORT void dealloc_textmessage(Text_Message * message)
{
	/*printf("dealloc_textmessage\n");*/
	cleanup(message->msg);
	cleanup(message);
	return;
}



/*!
 \brief dealloc_qfunction() deallocates the structure used to pass a function
 message from the thread to here..
 \param qfunc (QFunction *) Queded Function structure to deallocate
 */
G_MODULE_EXPORT void dealloc_qfunction(QFunction * qfunc)
{
	/*printf("dealloc_qfunction\n");*/
	cleanup (qfunc);
}


/*!
 \brief dealloc_table_params() deallocates the structure used for firmware
 table parameters
 \param table_params (Table_Params *) pointer to struct to deallocate
 */
G_MODULE_EXPORT void dealloc_table_params(Table_Params * table_params)
{
	cleanup(table_params->table_name);
	cleanup(table_params->bind_to_list);
	cleanup(table_params->x_source_key);
	cleanup(table_params->y_source_key);
	cleanup(table_params->z_source_key);
	cleanup(table_params->x_multi_expr_keys);
	cleanup(table_params->y_multi_expr_keys);
	cleanup(table_params->z_multi_expr_keys);
	cleanup(table_params->x_sources);
	cleanup(table_params->y_sources);
	cleanup(table_params->z_sources);
	cleanup(table_params->x_suffixes);
	cleanup(table_params->y_suffixes);
	cleanup(table_params->z_suffixes);
	cleanup(table_params->x_ul_conv_exprs);
	cleanup(table_params->y_ul_conv_exprs);
	cleanup(table_params->z_ul_conv_exprs);
	cleanup(table_params->x_dl_conv_exprs);
	cleanup(table_params->y_dl_conv_exprs);
	cleanup(table_params->z_dl_conv_exprs);
	cleanup(table_params->x_precisions);
	cleanup(table_params->y_precisions);
	cleanup(table_params->z_precisions);
	cleanup(table_params->x_source);
	cleanup(table_params->y_source);
	cleanup(table_params->z_source);
	cleanup(table_params->x_suffix);
	cleanup(table_params->y_suffix);
	cleanup(table_params->z_suffix);
	cleanup(table_params->x_ul_conv_expr);
	cleanup(table_params->y_ul_conv_expr);
	cleanup(table_params->z_ul_conv_expr);
	cleanup(table_params->x_dl_conv_expr);
	cleanup(table_params->y_dl_conv_expr);
	cleanup(table_params->z_dl_conv_expr);
	if (table_params->x_multi_hash)
		g_hash_table_destroy(table_params->x_multi_hash);
	if (table_params->y_multi_hash)
		g_hash_table_destroy(table_params->y_multi_hash);
	if (table_params->z_multi_hash)
		g_hash_table_destroy(table_params->z_multi_hash);
	if(table_params->x_ul_eval)
		evaluator_destroy(table_params->x_ul_eval);
	if(table_params->y_ul_eval)
		evaluator_destroy(table_params->y_ul_eval);
	if(table_params->z_ul_eval)
		evaluator_destroy(table_params->z_ul_eval);
	if(table_params->x_dl_eval)
		evaluator_destroy(table_params->x_dl_eval);
	if(table_params->y_dl_eval)
		evaluator_destroy(table_params->y_dl_eval);
	if(table_params->z_dl_eval)
		evaluator_destroy(table_params->z_dl_eval);
	if(table_params->x_object)
		g_object_unref(table_params->x_object);
	if(table_params->y_object)
		g_object_unref(table_params->y_object);
	if(table_params->z_object)
		g_object_unref(table_params->z_object);
	g_array_free(table_params->table,TRUE);

	cleanup(table_params);
	return;
}



/*!
 \brief dealloc_rtv_object() deallocates the rtv object used 
 for runtime vars data
 \param object (GData *) pointer to object to deallocate
 */
G_MODULE_EXPORT void dealloc_rtv_object(gconstpointer *object)
{
	GArray * array = NULL;
	if (!(object))
		return;
//	g_dataset_foreach(object,dump_dataset,NULL);
	array = (GArray *)DATA_GET(object, "history");
	if (array)
		g_array_free(DATA_GET(object,"history"),TRUE);
	/* This should release everything else bound via a DATA_SET_FULL */
	//g_dataset_foreach(object,dataset_dealloc,NULL);
	g_dataset_destroy(object);
}


/*!
 \brief dealloc_te_params() deallocates the structure used for firmware
 te parameters
 \param te_params (TE_Params *) pointer to struct to deallocate
 */
G_MODULE_EXPORT void dealloc_te_params(TE_Params * te_params)
{
	/*printf("dealloc_te_params\n");*/
	cleanup(te_params->title);
	cleanup(te_params->gauge);
	cleanup(te_params->c_gauge);
	cleanup(te_params->f_gauge);
	cleanup(te_params->gauge_datasource);
	cleanup(te_params->bg_color);
	cleanup(te_params->grat_color);
	cleanup(te_params->trace_color);
	cleanup(te_params->cross_color);
	cleanup(te_params->marker_color);
	cleanup(te_params->bind_to_list);
	cleanup(te_params->x_axis_label);
	cleanup(te_params->y_axis_label);
	cleanup(te_params->x_dl_conv_expr);
	cleanup(te_params->y_dl_conv_expr);
	cleanup(te_params->x_ul_conv_expr);
	cleanup(te_params->y_ul_conv_expr);
	cleanup(te_params->x_source);
	cleanup(te_params->y_source);
	cleanup(te_params->x_name);
	cleanup(te_params->y_name);
	cleanup(te_params->x_units);
	cleanup(te_params->y_units);
	g_list_free(te_params->entries);
	cleanup(te_params);
	return;
}


G_MODULE_EXPORT void dealloc_lookuptable(gpointer data)
{
	LookupTable * table = (LookupTable *)data;
	/*printf("dealloc_lookuptable\n");*/
	cleanup(table->array);
	cleanup(table->filename);
	cleanup(table);
	return;
}


G_MODULE_EXPORT void dealloc_widget(gpointer data, gpointer user_data)
{
	GtkWidget * widget = (GtkWidget *) data;

	if (!GTK_IS_WIDGET(widget))
		return;
	/*printf("dealloc_widget\n");*/
/*
	cleanup (OBJ_GET(widget,"algorithms"));
	cleanup (OBJ_GET(widget,"alt_lookuptable"));
	cleanup (OBJ_GET(widget,"applicable_tables"));
	cleanup (OBJ_GET(widget,"bind_to_list"));
	cleanup (OBJ_GET(widget,"bitvals"));
	cleanup (OBJ_GET(widget,"choices"));
	cleanup (OBJ_GET(widget,"complex_expr"));
	cleanup (OBJ_GET(widget,"data"));
	cleanup (OBJ_GET(widget,"depend_on"));
	cleanup (OBJ_GET(widget,"dl_conv_expr"));
	cleanup (OBJ_GET(widget,"dl_conv_exprs"));
	cleanup (OBJ_GET(widget,"fullname"));
	cleanup (OBJ_GET(widget,"group"));
	cleanup (OBJ_GET(widget,"group_2_update"));
	cleanup (OBJ_GET(widget,"initializer"));
	cleanup (OBJ_GET(widget,"lookuptable"));
	cleanup (OBJ_GET(widget,"multi_expr_keys"));
	cleanup (OBJ_GET(widget,"post_function_with_arg"));
	cleanup (OBJ_GET(widget,"post_functions_with_arg"));
	cleanup (OBJ_GET(widget,"raw_lower"));
	cleanup (OBJ_GET(widget,"raw_upper"));
	cleanup (OBJ_GET(widget,"register_as"));
	cleanup (OBJ_GET(widget,"source"));
	cleanup (OBJ_GET(widget,"sources"));
	cleanup (OBJ_GET(widget,"source_key"));
	cleanup (OBJ_GET(widget,"source_value"));
	cleanup (OBJ_GET(widget,"source_values"));
	cleanup (OBJ_GET(widget,"table_num"));
	cleanup (OBJ_GET(widget,"te_table_num"));
	cleanup (OBJ_GET(widget,"tooltip"));
	cleanup (OBJ_GET(widget,"ul_conv_expr"));
	cleanup (OBJ_GET(widget,"ul_conv_exprs"));
*/
	if (OBJ_GET(widget,"dl_evaluator"))
		evaluator_destroy (OBJ_GET(widget,"dl_evaluator"));
	if (OBJ_GET(widget,"ul_evaluator"))
		evaluator_destroy (OBJ_GET(widget,"ul_evaluator"));
}


G_MODULE_EXPORT gboolean dealloc_rtt_model(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,gpointer user_data)
{
	Rt_Text *rtt = NULL;
	gtk_tree_model_get (model, iter,
			COL_RTT_OBJECT, &rtt,
			-1);
	dealloc_rtt((gpointer)rtt);
	return FALSE;
}

G_MODULE_EXPORT void dealloc_rtt(gpointer data)
{
	Rt_Text *rtt = (Rt_Text *)data;
	/*printf("dealloc_rtt\n");*/
	cleanup(rtt->ctrl_name);
	cleanup(rtt->label_prefix);
	cleanup(rtt->label_suffix);
	cleanup(rtt);
}


G_MODULE_EXPORT void dealloc_slider(gpointer data)
{
	Rt_Slider *slider = (Rt_Slider *)data;
	/*printf("dealloc_slider\n");*/
	cleanup(slider->ctrl_name);
	/* Don't free object or history as those are just ptr's to the actual
	 * data in the rtv object and that gets freed elsewhere
	 */
	cleanup(slider);
	return;
}


G_MODULE_EXPORT void xml_cmd_free(gpointer data)
{
	Command *cmd = NULL;
	cmd = (Command *)data;
	cleanup(cmd->name);
	cleanup(cmd->desc);
	cleanup(cmd->base);
	cleanup(cmd->helper_func_name);
	cleanup(cmd->func_call_name);
	dealloc_array(cmd->post_functions,POST_FUNCTIONS);
	g_array_free(cmd->args,TRUE);
	cleanup(cmd);
}

G_MODULE_EXPORT void xml_arg_free(gpointer data)
{
	PotentialArg *arg = NULL;
	arg = (PotentialArg *)data;
	cleanup(arg->name);
	cleanup(arg->desc);
	cleanup(arg->internal_name);
	cleanup(arg->static_string);
	cleanup(arg);
}


G_MODULE_EXPORT void dealloc_lists_hash(gpointer data)
{
	g_hash_table_foreach((GHashTable *)data,(GHFunc)dealloc_list,NULL);
	g_hash_table_destroy((GHashTable *)data);
}


G_MODULE_EXPORT void dealloc_list(gpointer key, gpointer value, gpointer user_data)
{
	g_list_free((GList *)value);
}


G_MODULE_EXPORT void cleanup(void *data)
{
	if (data)
		g_free(data);
	data = NULL;
	return;
}

