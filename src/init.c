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
#include <init.h>
#include <ms_structures.h>
#include <structures.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

gint major_ver;
gint minor_ver;
gint micro_ver;
gint preferred_delimiter;
extern guint dbg_lvl;
gint ecu_caps = 0;	/* Assume stock B&G code */
extern gint mem_view_style[];
extern gint ms_reset_count;
extern gint ms_goodread_count;
extern gboolean just_starting;
extern gboolean raw_reader_running;
extern gboolean tips_in_use;
extern gint temp_units;
extern gint main_x_origin;
extern gint main_y_origin;
extern gint lv_scroll;
extern gint width;
extern gint height;
extern gint interval_min;
extern gint interval_step;
extern gint interval_max;
extern GtkWidget *main_window;
struct Serial_Params *serial_params;	
/* Support up to "x" page firmware.... */
guchar *ms_data[MAX_SUPPORTED_PAGES];
guchar *ms_data_last[MAX_SUPPORTED_PAGES];
guchar *ms_data_backup[MAX_SUPPORTED_PAGES];
GList *ve_widgets[MAX_SUPPORTED_PAGES][2*MS_PAGE_SIZE];
GHashTable *interdep_vars_1 = NULL;
GHashTable *interdep_vars_2 = NULL;
struct IoCmds *cmds; 


/*!
 * init()
 * \brief Sets sane values to global variables for a clean startup of 
 * MegaTunix
 */
void init(void)
{
	/* defaults */
	interval_min = 5;	/* 5 millisecond minimum interval delay */
	interval_step = 5;	/* 5 ms steps */
	interval_max = 1000;	/* 1000 millisecond maximum interval delay */
	width = 717;		/* min window width */
	height = 579;		/* min window height */
	main_x_origin = 160;	/* offset from left edge of screen */
	main_y_origin = 120;	/* offset from top edge of screen */

	/* initialize all global variables to known states */
	serial_params->fd = 0; /* serial port file-descriptor */
	serial_params->errcount = 0; /* I/O error count */
	/* default for MS V 1.x and 2.x */
	serial_params->read_wait = 100;	/* delay between reads in milliseconds */

	/* Set flags to clean state */
	raw_reader_running = FALSE;  /* We're not reading raw data yet... */
	just_starting = TRUE; 	/* to handle initial errors */
	ms_reset_count = 0; 	/* Counts MS clock resets */
	ms_goodread_count = 0; 	/* How many reads of realtime vars completed */
	tips_in_use = TRUE;	/* Use tooltips by default */
	temp_units = FAHRENHEIT;/* Use SAE units by default */
	lv_scroll = 1;		/* Logviewer scroll speed */
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
	ConfigFile *cfgfile;
	gchar *filename = NULL;
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/config", NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		//cfg_read_int(cfgfile, "Global", "major_ver", &major_ver);
		//cfg_read_int(cfgfile, "Global", "minor_ver", &minor_ver);
		//cfg_read_int(cfgfile, "Global", "micro_ver", &micro_ver);
		cfg_read_boolean(cfgfile, "Global", "Tooltips", &tips_in_use);
		cfg_read_int(cfgfile, "Global", "Temp_Scale", &temp_units);
		cfg_read_int(cfgfile, "Global", "dbg_lvl", &dbg_lvl);
		cfg_read_int(cfgfile, "DataLogger", "preferred_delimiter", &preferred_delimiter);
		cfg_read_int(cfgfile, "Window", "width", &width);
		cfg_read_int(cfgfile, "Window", "height", &height);
		cfg_read_int(cfgfile, "Window", "main_x_origin", 
				&main_x_origin);
		cfg_read_int(cfgfile, "Window", "main_y_origin", 
				&main_y_origin);
		cfg_read_string(cfgfile, "Serial", "port_name", 
				&serial_params->port_name);
		cfg_read_int(cfgfile, "Serial", "read_delay", 
				&serial_params->read_wait);
		cfg_read_int(cfgfile, "Logviewer", "scroll_speed", &lv_scroll);
		cfg_read_int(cfgfile, "MemViewer", "page0_style", &mem_view_style[0]);
		cfg_read_int(cfgfile, "MemViewer", "page1_style", &mem_view_style[1]);
		cfg_read_int(cfgfile, "MemViewer", "page2_style", &mem_view_style[2]);
		cfg_read_int(cfgfile, "MemViewer", "page3_style", &mem_view_style[3]);
		cfg_free(cfgfile);
		g_free(filename);
		return TRUE;
	}
	else
	{
		serial_params->port_name = g_strdup("/dev/tt???");
		dbg_func(__FILE__": read_config()\n\tConfig file not found, using defaults\n",CRITICAL);
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
	int x,y,tmp_width,tmp_height;
	ConfigFile *cfgfile;
	extern gboolean ready;
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/config", NULL);
	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		cfgfile = cfg_new();


	cfg_write_int(cfgfile, "Global", "major_ver", _MAJOR_);
	cfg_write_int(cfgfile, "Global", "minor_ver", _MINOR_);
	cfg_write_int(cfgfile, "Global", "micro_ver", _MICRO_);
	cfg_write_boolean(cfgfile, "Global", "Tooltips", tips_in_use);
	cfg_write_int(cfgfile, "Global", "Temp_Scale", temp_units);
	cfg_write_int(cfgfile, "Global", "dbg_lvl", dbg_lvl);

	if (ready)
	{
		gdk_drawable_get_size(main_window->window, &tmp_width,&tmp_height);
		cfg_write_int(cfgfile, "Window", "width", tmp_width);
		cfg_write_int(cfgfile, "Window", "height", tmp_height);
		gdk_window_get_position(main_window->window,&x,&y);
		cfg_write_int(cfgfile, "Window", "main_x_origin", x);
		cfg_write_int(cfgfile, "Window", "main_y_origin", y);
	}
	cfg_write_int(cfgfile, "DataLogger", "preferred_delimiter", preferred_delimiter);
	cfg_write_string(cfgfile, "Serial", "port_name", 
			serial_params->port_name);
	cfg_write_int(cfgfile, "Serial", "read_delay", 
			serial_params->read_wait);
	cfg_write_int(cfgfile, "Logviewer", "scroll_speed", lv_scroll);
	cfg_write_int(cfgfile, "MemViewer", "page0_style", mem_view_style[0]);
	cfg_write_int(cfgfile, "MemViewer", "page1_style", mem_view_style[1]);
	cfg_write_int(cfgfile, "MemViewer", "page2_style", mem_view_style[2]);
	cfg_write_int(cfgfile, "MemViewer", "page3_style", mem_view_style[3]);

	cfg_write_file(cfgfile, filename);
	cfg_free(cfgfile);

	g_free(cfgfile);
	g_free(filename);

}


/*!
 * make_megasquirt_dirs()
 * \brief Creates the directories for user modified config files in the
 * users home directory under ~/.MegaTunix
 */
void make_megasquirt_dirs(void)
{
	gchar *filename = NULL;

	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix", NULL);
	mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/Gui", NULL);
	mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/Interrogator", NULL);
	mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/Interrogator/Profiles", NULL);
	mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/LookupTables", NULL);
	mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/RealtimeMaps", NULL);
	mkdir(filename, S_IRWXU);
	g_free(filename);
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/RuntimeSliders", NULL);
	mkdir(filename, S_IRWXU);
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
	/* Hash tables to store the interdependant deferred variables before
	 * download...
	 */
	interdep_vars_1 = g_hash_table_new(NULL,NULL);
	interdep_vars_2 = g_hash_table_new(NULL,NULL);

	/* Allocate memory blocks */
	serial_params = g_malloc0(sizeof(struct Serial_Params));

	for (i=0;i<MAX_SUPPORTED_PAGES;i++)
	{
		ms_data[i] = g_malloc0(2*MS_PAGE_SIZE);
		ms_data_last[i] = g_malloc0(2*MS_PAGE_SIZE);
		ms_data_backup[i] = g_malloc0(2*MS_PAGE_SIZE);
		for (j=0;j<2*MS_PAGE_SIZE;j++)
			ve_widgets[i][j] = NULL;
	}

	cmds = g_malloc0(sizeof(struct IoCmds));

}


/*!
 * mem_dealloc()
 * \brief Deallocates memory allocated with mem_alloc
 * \see mem_alloc
 */
void mem_dealloc()
{
	gint i = 0;
	extern struct Firmware_Details *firmware;
	/* Allocate memory blocks */

	if (serial_params->port_name)
		g_free(serial_params->port_name);
	g_free(serial_params);

	/* Firmware datastructure.... */
	if (firmware)
	{
		if (firmware->name)
			g_free(firmware->name);
		if (firmware->tab_list)
			g_strfreev(firmware->tab_list);
		for (i=0;i<firmware->total_pages;i++)
			g_free(firmware->page_params[i]);
		g_free(firmware);
	}

	for (i=0;i<MAX_SUPPORTED_PAGES;i++)
	{
		g_free(ms_data[i]);
		g_free(ms_data_last[i]);
		g_free(ms_data_backup[i]);
	}
	g_hash_table_destroy(interdep_vars_1);
	g_hash_table_destroy(interdep_vars_2);
}
