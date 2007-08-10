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
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <init.h>
#include <listmgmt.h>
#include <structures.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <widgetmgmt.h>
#include <unistd.h>

gint major_ver;
gint minor_ver;
gint micro_ver;
gint preferred_delimiter;
gint baudrate;
gchar * serial_port_name = NULL;
extern gint dbg_lvl;
extern GStaticMutex comms_mutex;
extern gint mem_view_style[];
extern gint ms_reset_count;
extern gint ms_goodread_count;
extern gboolean just_starting;
extern gboolean tips_in_use;
extern gint temp_units;
extern gint main_x_origin;
extern gint main_y_origin;
extern gint lv_zoom;
extern gint width;
extern gint height;
extern gint interval_min;
extern gint interval_step;
extern gint interval_max;
extern GtkWidget *main_window;
extern gint dbg_lvl;
extern Serial_Params *serial_params;
/* Support up to "x" page firmware.... */
gint **ms_data = NULL;
gint **ms_data_last = NULL;
gint **ms_data_backup = NULL;
GList ***ve_widgets = NULL;
GList **tab_gauges = NULL;
GHashTable **interdep_vars = NULL;
GHashTable *widget_group_states = NULL;
GHashTable *sources_hash = NULL;
GObject *global_data = NULL;
gint *algorithm = NULL;
gboolean *tracking_focus = NULL;


/*!
 * init()
 * \brief Sets sane values to global variables for a clean startup of 
 * MegaTunix
 */
void init(void)
{
	/* defaults */
	global_data = g_object_new(GTK_TYPE_INVISIBLE,NULL);
	interval_min = 5;	/* 5 millisecond minimum interval delay */
	interval_step = 5;	/* 5 ms steps */
	interval_max = 1000;	/* 1000 millisecond maximum interval delay */
	g_object_set_data(global_data,"status_width",GINT_TO_POINTER(130));
	g_object_set_data(global_data,"status_height",GINT_TO_POINTER(386));
	g_object_set_data(global_data,"rtt_width",GINT_TO_POINTER(125));
	g_object_set_data(global_data,"rtt_height",GINT_TO_POINTER(480));
	g_object_set_data(global_data,"width",GINT_TO_POINTER(640));
	g_object_set_data(global_data,"height",GINT_TO_POINTER(480));
	g_object_set_data(global_data,"main_x_origin",GINT_TO_POINTER(160));
	g_object_set_data(global_data,"main_y_origin",GINT_TO_POINTER(120));

	/* initialize all global variables to known states */
#ifdef __WIN32__
	serial_port_name = g_strdup("COM1");
#else
	serial_port_name = g_strdup("/dev/ttyS0");
#endif
	serial_params->fd = 0; /* serial port file-descriptor */

	serial_params->errcount = 0; /* I/O error count */
	/* default for MS V 1.x and 2.x */
	serial_params->read_wait = 10;	/* delay between reads in milliseconds */
	baudrate = 9600;	/* default to MS-I */

	/* Set flags to clean state */
	just_starting = TRUE; 	/* to handle initial errors */
	ms_reset_count = 0; 	/* Counts MS clock resets */
	ms_goodread_count = 0; 	/* How many reads of realtime vars completed */
	tips_in_use = TRUE;	/* Use tooltips by default */
	temp_units = FAHRENHEIT;/* Use SAE units by default */
	lv_zoom = 1;		/* Logviewer scroll speed */
	preferred_delimiter = TAB;
}


/*!
 * read_config()
 * \brief Reads state of various control variables, like window position
 * size, serial port and parameters and other user defaults from the default
 * config file located at ~/.MegaTunix/config
 * \see save_config()
 */
gboolean read_config(void)
{
	gint tmpi = 0;
	gfloat tmpf = 0.0;
	gchar * tmpbuf = NULL;
	ConfigFile *cfgfile;
	gchar *filename = NULL;
	filename = g_strconcat(HOME(), PSEP,".MegaTunix",PSEP,"config", NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		cfg_read_boolean(cfgfile, "Global", "Tooltips", &tips_in_use);
		cfg_read_int(cfgfile, "Global", "Temp_Scale", &temp_units);
		cfg_read_int(cfgfile, "Global", "dbg_lvl", &dbg_lvl);
		if (cfg_read_string(cfgfile, "Dashboards", "dash_1_name", &tmpbuf))
			g_object_set_data(global_data,"dash_1_name",g_strdup(tmpbuf));
		if (cfg_read_int(cfgfile, "Dashboards", "dash_1_x_origin", &tmpi))
			g_object_set_data(global_data,"dash_1_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Dashboards", "dash_1_y_origin", &tmpi))
			g_object_set_data(global_data,"dash_1_y_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_float(cfgfile, "Dashboards", "dash_1_size_ratio", &tmpf))
			g_object_set_data(global_data,"dash_1_size_ratio",g_memdup(&tmpf,sizeof(gfloat)));
		if (cfg_read_string(cfgfile, "Dashboards", "dash_2_name", &tmpbuf))
			g_object_set_data(global_data,"dash_2_name",g_strdup(tmpbuf));
		if (cfg_read_int(cfgfile, "Dashboards", "dash_2_x_origin", &tmpi))
			g_object_set_data(global_data,"dash_2_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Dashboards", "dash_2_y_origin", &tmpi))
			g_object_set_data(global_data,"dash_2_y_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_float(cfgfile, "Dashboards", "dash_2_size_ratio", &tmpf))
			g_object_set_data(global_data,"dash_2_size_ratio",g_memdup(&tmpf,sizeof(gfloat)));
		cfg_read_int(cfgfile, "DataLogger", "preferred_delimiter", &preferred_delimiter);
		if (cfg_read_int(cfgfile, "Window", "status_width", &tmpi))
			g_object_set_data(global_data,"status_width",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "status_height", &tmpi))
			g_object_set_data(global_data,"status_height",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "status_x_origin", &tmpi))
			g_object_set_data(global_data,"status_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "status_y_origin", &tmpi))
			g_object_set_data(global_data,"status_y_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "rtt_width", &tmpi))
			g_object_set_data(global_data,"rtt_width",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "rtt_height", &tmpi))
			g_object_set_data(global_data,"rtt_height",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "rtt_x_origin", &tmpi))
			g_object_set_data(global_data,"rtt_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "rtt_y_origin", &tmpi))
			g_object_set_data(global_data,"rtt_y_origin",GINT_TO_POINTER(tmpi));
		if(cfg_read_int(cfgfile, "Window", "width", &tmpi))
			g_object_set_data(global_data,"width",GINT_TO_POINTER(tmpi));
		if(cfg_read_int(cfgfile, "Window", "height", &tmpi))
			g_object_set_data(global_data,"height",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "main_x_origin", &tmpi))
			g_object_set_data(global_data,"main_x_origin",GINT_TO_POINTER(tmpi));
		if (cfg_read_int(cfgfile, "Window", "main_y_origin", &tmpi))
			g_object_set_data(global_data,"main_y_origin",GINT_TO_POINTER(tmpi));
		cfg_read_string(cfgfile, "Serial", "port_name", 
				&serial_port_name);
		cfg_read_int(cfgfile, "Serial", "read_wait", 
				&serial_params->read_wait);
		cfg_read_int(cfgfile, "Serial", "baudrate", 
				&baudrate);
		cfg_read_int(cfgfile, "Logviewer", "zoom", &lv_zoom);
		if(cfg_read_int(cfgfile, "Logviewer", "scroll_delay", &tmpi))
			g_object_set_data(global_data,"lv_scroll_delay",GINT_TO_POINTER(tmpi));
		cfg_read_int(cfgfile, "MemViewer", "page0_style", &mem_view_style[0]);
		cfg_read_int(cfgfile, "MemViewer", "page1_style", &mem_view_style[1]);
		cfg_read_int(cfgfile, "MemViewer", "page2_style", &mem_view_style[2]);
		cfg_read_int(cfgfile, "MemViewer", "page3_style", &mem_view_style[3]);
		cfg_free(cfgfile);
		g_free(cfgfile);
		g_free(filename);
		return TRUE;
	}
	else
	{
		serial_params->port_name = g_strdup(DEFAULT_PORT);
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": read_config()\n\tConfig file not found, using defaults\n"));
		g_free(filename);
		save_config();
		return FALSE;	/* No file found */
	}
	return TRUE;
}


/*!
 * save_config()
 * \brief Saves state of various control variables, like window position
 * size, serial port and parameters and other user defaults
 * \see read_config()
 */
void save_config(void)
{
	gchar *filename = NULL;
	gchar * tmpbuf = NULL;
	GtkWidget *widget = NULL;
	int x = 0;
	int y = 0;
	int tmp_width = 0;
	int tmp_height = 0;
	int orig_width = 0;
	int orig_height = 0;
	gfloat ratio = 0.0;
	GtkWidget *dash = NULL;
	extern gboolean ready;
	ConfigFile *cfgfile = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	extern GHashTable *dynamic_widgets;

	g_static_mutex_lock(&mutex);

	filename = g_strconcat(HOME(), "/.MegaTunix/config", NULL);
	cfgfile = cfg_open_file(filename);

	if (!cfgfile)
		cfgfile = cfg_new();


	cfg_write_int(cfgfile, "Global", "major_ver", _MAJOR_);
	cfg_write_int(cfgfile, "Global", "minor_ver", _MINOR_);
	cfg_write_int(cfgfile, "Global", "micro_ver", _MICRO_);
	cfg_write_boolean(cfgfile, "Global", "Tooltips", tips_in_use);
	cfg_write_int(cfgfile, "Global", "Temp_Scale", temp_units);
	cfg_write_int(cfgfile, "Global", "dbg_lvl", dbg_lvl);
	tmpbuf = (gchar *)g_object_get_data(global_data,"dash_1_name");
	if (tmpbuf)
	{
		cfg_write_string(cfgfile, "Dashboards", "dash_1_name", tmpbuf);
		widget =  g_hash_table_lookup(dynamic_widgets,tmpbuf);
		if (GTK_IS_WIDGET(widget))
		{
			gtk_window_get_position(GTK_WINDOW(widget),&x,&y);
			cfg_write_int(cfgfile, "Dashboards", "dash_1_x_origin", x);
			cfg_write_int(cfgfile, "Dashboards", "dash_1_y_origin", y);
			dash = g_object_get_data(G_OBJECT(widget),"dash");
			orig_width = (gint) g_object_get_data(G_OBJECT(dash),"orig_width");
		        orig_height = (gint) g_object_get_data(G_OBJECT(dash),"orig_height");
			gdk_drawable_get_size(gtk_widget_get_toplevel(widget)->window, &tmp_width,&tmp_height);
			ratio = (((gfloat)tmp_height/(gfloat)orig_height)+((gfloat)tmp_width/(gfloat)orig_width))/2.0;
			cfg_write_float(cfgfile, "Dashboards", "dash_1_size_ratio", ratio);
		}
	}
	else
	{
		cfg_remove_key(cfgfile, "Dashboards", "dash_1_name");
		cfg_remove_key(cfgfile, "Dashboards", "dash_1_x_origin");
		cfg_remove_key(cfgfile, "Dashboards", "dash_1_y_origin");
		cfg_remove_key(cfgfile, "Dashboards", "dash_1_size_ratio");
	}
	tmpbuf = (gchar *)g_object_get_data(global_data,"dash_2_name");
	if (tmpbuf)
	{
		cfg_write_string(cfgfile, "Dashboards", "dash_2_name", tmpbuf);
		widget =  g_hash_table_lookup(dynamic_widgets,tmpbuf);
		if (GTK_IS_WIDGET(widget))
		{
			gtk_window_get_position(GTK_WINDOW(widget),&x,&y);
			cfg_write_int(cfgfile, "Dashboards", "dash_2_x_origin", x);
			cfg_write_int(cfgfile, "Dashboards", "dash_2_y_origin", y);
			dash = g_object_get_data(G_OBJECT(widget),"dash");
			orig_width = (gint) g_object_get_data(G_OBJECT(dash),"orig_width");
		        orig_height = (gint) g_object_get_data(G_OBJECT(dash),"orig_height");
			gdk_drawable_get_size(gtk_widget_get_toplevel(widget)->window, &tmp_width,&tmp_height);
			ratio = (((gfloat)tmp_height/(gfloat)orig_height)+((gfloat)tmp_width/(gfloat)orig_width))/2.0;
			cfg_write_float(cfgfile, "Dashboards", "dash_2_size_ratio", ratio);
		}
	}
	else
	{
		cfg_remove_key(cfgfile, "Dashboards", "dash_2_name");
		cfg_remove_key(cfgfile, "Dashboards", "dash_2_x_origin");
		cfg_remove_key(cfgfile, "Dashboards", "dash_2_y_origin");
		cfg_remove_key(cfgfile, "Dashboards", "dash_2_size_ratio");
	}
				

	if (ready)
	{
		gdk_drawable_get_size(main_window->window, &tmp_width,&tmp_height);
		cfg_write_int(cfgfile, "Window", "width", tmp_width);
		cfg_write_int(cfgfile, "Window", "height", tmp_height);
		gtk_window_get_position(GTK_WINDOW(main_window),&x,&y);
		if (x > 0)
			cfg_write_int(cfgfile, "Window", "main_x_origin", x);
		if (y > 0)
			cfg_write_int(cfgfile, "Window", "main_y_origin", y);
		if (g_hash_table_lookup(dynamic_widgets,"status_window"))
		{
			gdk_drawable_get_size(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"status_window"))->window, &tmp_width,&tmp_height);

			cfg_write_int(cfgfile, "Window", "status_width", tmp_width);
			cfg_write_int(cfgfile, "Window", "status_height", tmp_height);
			gtk_window_get_position(GTK_WINDOW(g_hash_table_lookup(dynamic_widgets,"status_window")),&x,&y);
			if (x > 0)
				cfg_write_int(cfgfile, "Window", "status_x_origin", x);
			if (y > 0)
				cfg_write_int(cfgfile, "Window", "status_y_origin", y);
		}
		if (g_hash_table_lookup(dynamic_widgets,"rtt_window"))
		{
			gdk_drawable_get_size(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"rtt_window"))->window, &tmp_width,&tmp_height);

			cfg_write_int(cfgfile, "Window", "rtt_width", tmp_width);
			cfg_write_int(cfgfile, "Window", "rtt_height", tmp_height);
			gtk_window_get_position(GTK_WINDOW(g_hash_table_lookup(dynamic_widgets,"rtt_window")),&x,&y);
			if (x > 0)
				cfg_write_int(cfgfile, "Window", "rtt_x_origin", x);
			if (y > 0)
				cfg_write_int(cfgfile, "Window", "rtt_y_origin", y);
		}
	}
	cfg_write_int(cfgfile, "DataLogger", "preferred_delimiter", preferred_delimiter);
	if (serial_params->port_name)
		cfg_write_string(cfgfile, "Serial", "port_name", 
				serial_params->port_name);
	cfg_write_int(cfgfile, "Serial", "read_wait", 
			serial_params->read_wait);
	cfg_write_int(cfgfile, "Serial", "baudrate", baudrate);
			
	cfg_write_int(cfgfile, "Logviewer", "zoom", lv_zoom);
	cfg_write_int(cfgfile, "Logviewer", "scroll_delay",(gint) g_object_get_data(global_data,"lv_scroll_delay"));

	cfg_write_int(cfgfile, "MemViewer", "page0_style", mem_view_style[0]);
	cfg_write_int(cfgfile, "MemViewer", "page1_style", mem_view_style[1]);
	cfg_write_int(cfgfile, "MemViewer", "page2_style", mem_view_style[2]);
	cfg_write_int(cfgfile, "MemViewer", "page3_style", mem_view_style[3]);

	cfg_write_file(cfgfile, filename);
	cfg_free(cfgfile);
	g_free(cfgfile);
	g_free(filename);
	g_static_mutex_unlock(&mutex);
}


/*!
 * make_megasquirt_dirs()
 * \brief Creates the directories for user modified config files in the
 * users home directory under ~/.MegaTunix
 */
void make_megasquirt_dirs(void)
{
	gchar *filename = NULL;
	const gchar *mtx = ".MegaTunix";

	filename = g_strconcat(HOME(), "/.MegaTunix", NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,GUI_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,GAUGES_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,DASHES_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,INTERROGATOR_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,INTERROGATOR_DATA_DIR,PSEP,"Profiles", NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,LOOKUPTABLES_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,REALTIME_MAPS_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,RTSLIDERS_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(HOME(),PSEP,mtx,PSEP,RTSTATUS_DATA_DIR, NULL);
	g_mkdir(filename, S_IRWXU);
	g_free(filename);

	return;
}


/*!
 * mem_alloc()
 * \brief Allocates memory allocated, to be deallocated at close by mem_dalloc
 * \see mem_dealloc
 */
void mem_alloc()
{
	gint i=0;
	gint j=0;
	extern Firmware_Details *firmware;
	/* Hash tables to store the interdependant deferred variables before
	 * download...
	 */

	if (!ms_data)
		ms_data = g_new0(gint *, firmware->total_pages);
	if (!ms_data_last)
		ms_data_last = g_new0(gint *, firmware->total_pages);
	if (!ms_data_backup)
		ms_data_backup = g_new0(gint *, firmware->total_pages);
	if (!ve_widgets)
		ve_widgets = g_new0(GList **, firmware->total_pages);
	if (!tab_gauges)
		tab_gauges = g_new0(GList *, firmware->total_tables);
	if (!sources_hash)
		sources_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	if (!widget_group_states)
		widget_group_states = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		g_hash_table_insert(widget_group_states,g_strdup("temperature"),(gpointer)TRUE);
		g_hash_table_insert(widget_group_states,g_strdup("multi_expression"),(gpointer)TRUE);
	if (!interdep_vars)
		interdep_vars = g_new0(GHashTable *,firmware->total_pages);
	if (!algorithm)
		algorithm = g_new0(gint, firmware->total_tables);
	if (!tracking_focus)
		tracking_focus = g_new0(gboolean, firmware->total_tables);

	for (i=0;i<firmware->total_tables;i++)
	{
		tab_gauges[i] = NULL;
		algorithm[i] = SPEED_DENSITY;
	}

	for (i=0;i<firmware->total_pages;i++)
	{
		interdep_vars[i] = g_hash_table_new(NULL,NULL);

		if (!ms_data[i])
			ms_data[i] = g_new0(gint, firmware->page_params[i]->length);
		if (!ms_data_last[i])
			ms_data_last[i] = g_new0(gint, firmware->page_params[i]->length);
		if (!ms_data_backup[i])
			ms_data_backup[i] = g_new0(gint, firmware->page_params[i]->length);
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
 * mem_dealloc()
 * \brief Deallocates memory allocated with mem_alloc
 * \see mem_alloc
 */
void mem_dealloc()
{
	gint i = 0;
	extern Firmware_Details *firmware;

	g_static_mutex_lock(&comms_mutex);
	if (serial_params->port_name)
		g_free(serial_params->port_name);
	serial_params->port_name = NULL;
	if (serial_params)
		g_free(serial_params);
	serial_params = NULL;
	g_static_mutex_unlock(&comms_mutex);

	/* Firmware datastructure.... */
	if (firmware)
	{
		for (i=0;i<firmware->total_pages;i++)
		{
			if (ms_data[i])
				g_free(ms_data[i]);
			if (ms_data_last[i])
				g_free(ms_data_last[i]);
			if (ms_data_backup[i])
				g_free(ms_data_backup[i]);
			if (interdep_vars[i])
			{
				g_hash_table_destroy(interdep_vars[i]);
				interdep_vars[i] = NULL;
			}
		}
		if (firmware->name)
			g_free(firmware->name);
		if (firmware->tab_list)
			g_strfreev(firmware->tab_list);
		if (firmware->tab_confs)
			g_strfreev(firmware->tab_confs);
		if (firmware->rtv_map_file)
			g_free(firmware->rtv_map_file);
		if (firmware->sliders_map_file)
			g_free(firmware->sliders_map_file);
		if (firmware->status_map_file)
			g_free(firmware->status_map_file);
		if (firmware->write_cmd)
			g_free(firmware->write_cmd);
		if (firmware->burn_cmd)
			g_free(firmware->burn_cmd);
		if (firmware->page_cmd)
			g_free(firmware->page_cmd);
		for (i=0;i<firmware->total_pages;i++)
		{
			if (firmware->page_params[i])
				g_free(firmware->page_params[i]);
		}
		g_free(firmware->page_params);
		firmware->page_params = NULL;
		for (i=0;i<firmware->total_tables;i++)
		{
			if (firmware->table_params[i])
				dealloc_table_params(firmware->table_params[i]);
		}
		g_free(firmware->table_params);
		firmware->table_params = NULL;
		for (i=0;i<firmware->total_tables;i++)
		{
			if (firmware->rf_params[i])
				g_free(firmware->rf_params[i]);
		}
		g_free(firmware->rf_params);
		firmware->rf_params = NULL;
		g_free(firmware);
		firmware = NULL;
		g_free(ms_data);
		g_free(ms_data_last);
		g_free(ms_data_backup);
	}
	if(widget_group_states)
		g_hash_table_destroy(widget_group_states);
	if(sources_hash)
		g_hash_table_destroy(sources_hash);
}


/*!
 \brief initialize_io_message() allocates and initializes a pointer
 to a Io_Message datastructure,  used for passing messages 
 across the GAsyncQueue's between the threads and the main context
 \returns a allocated and initialized pointer to a single structure
 */
Io_Message * initialize_io_message()
{
	Io_Message *message = NULL;

	message = g_new0(Io_Message, 1);
	message->out_str = NULL;
	message->funcs = NULL;
	message->payload = NULL;

	return message;
}


/*!
 *  \brief initialize_page_params() creates and initializes the page_params
 *   datastructure to sane defaults and returns it
 *    */
Page_Params * initialize_page_params(void)
{
	Page_Params *page_params = NULL;
	page_params = g_malloc0(sizeof(Page_Params));
	page_params->length = 0;
	page_params->is_spark = FALSE;
	page_params->spconfig_offset = -1;
	return page_params;
}

         
/*!
 *  \brief initialize_canidate() creates and initializes the Candidate
 *   datastructure to sane defaults and returns it
 *    */
Table_Params * initialize_table_params(void)
{
	Table_Params *table_params = NULL;
	table_params = g_malloc0(sizeof(Table_Params));
	table_params->is_fuel = FALSE;
	table_params->cfg11_offset = -1;
	table_params->cfg12_offset = -1;
	table_params->cfg13_offset = -1;
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
	table_params->x_suffix = NULL;
	table_params->y_suffix = NULL;
	table_params->z_suffix = NULL;
	table_params->x_conv_expr = NULL;
	table_params->y_conv_expr = NULL;
	table_params->z_conv_expr = NULL;
	table_params->table_name = NULL;

	return table_params;
}



/*!
 \brief dealloc_message() deallocates the structure used to pass an I/O
 message from a thread to here..
 \param message (Io_Message *) pointer to message data
 */
void dealloc_message(Io_Message * message)
{
        if (message->out_str)
                g_free(message->out_str);
        if (message->funcs)
                g_array_free(message->funcs,TRUE);
        if (message->payload)
                g_free(message->payload);
        g_free(message);

}


/*!
 \brief dealloc_w_update() deallocates the structure used to pass an I/O
 widget update message from a thread to here..
 \param w_update (Widget_Update *) pointer to message data
 */
void dealloc_w_update(Widget_Update * w_update)
{
        if (w_update->widget_name)
                g_free(w_update->widget_name);
        if (w_update->msg)
                g_free(w_update->msg);
        g_free(w_update);
	w_update = NULL;

}


/*!
 \brief dealloc_textmessage() deallocates the structure used to pass a text
 message from the thread to here..
 \param message (Text_Message *) pointer to message data
 */
void dealloc_textmessage(Text_Message * message)
{
	g_free(message);
	message = NULL;
}



/*!
 \brief dealloc_qfunction() deallocates the structure used to pass a function
 message from the thread to here..
 \param qfunc (QFunction *) Queded Function structure to deallocate
 */
void dealloc_qfunction(QFunction * qfunc)
{
	if (qfunc->func_name)
		g_free(qfunc->func_name);
	g_free(qfunc);
	qfunc = NULL;
}


/*!
 \brief dealloc_table_params() deallocates the structure used for firmware
 table parameters
 \param table_params (TableParams *) pointer to struct to deallocate
 */
void dealloc_table_params(Table_Params * table_params)
{
	/*
	if(table_params->x_suffix)
		g_free(table_params->x_suffix);
	if(table_params->y_suffix)
		g_free(table_params->y_suffix);
	if(table_params->z_suffix)
		g_free(table_params->z_suffix);
	if(table_params->x_conv_expr)
		g_free(table_params->x_conv_expr);
	if(table_params->y_conv_expr)
		g_free(table_params->y_conv_expr);
	if(table_params->z_conv_expr)
		g_free(table_params->z_conv_expr);
	if(table_params->table_name)
		g_free(table_params->table_name);
	*/
	table_params->x_suffix = NULL;
	table_params->y_suffix = NULL;
	table_params->z_suffix = NULL;
	table_params->x_conv_expr = NULL;
	table_params->y_conv_expr = NULL;
	table_params->z_conv_expr = NULL;
	table_params->table_name = NULL;

	return;
}


