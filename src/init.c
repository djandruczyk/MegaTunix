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
#include <globals.h>
#include <init.h>
#include <stdio.h>
#include <stdlib.h>
#include <structures.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

gint major_ver;
gint minor_ver;
gint micro_ver;
unsigned char *kpa_conversion; 
extern unsigned char turbo_map[];
extern unsigned char na_map[];
extern gint def_comm_port;
extern gint ms_reset_count;
extern gint ms_goodread_count;
extern gboolean just_starting;
extern gboolean raw_reader_running;
extern gboolean raw_reader_stopped;
extern gboolean tips_in_use;
extern gboolean fahrenheit;
extern gint main_x_origin;
extern gint main_y_origin;
extern gint width;
extern gint height;
extern gint poll_min;
extern gint poll_step;
extern gint poll_max;
extern gint interval_min;
extern gint interval_step;
extern gint interval_max;
extern GtkWidget *main_window;
extern struct Serial_Params serial_params;
struct Ve_Const_Std *ve_const_p0;
struct Ve_Const_Std *ve_const_p1;
struct Ve_Const_Std *ve_const_p0_tmp;
struct Ve_Const_Std *ve_const_p1_tmp;
struct Conversion_Chart *page0_conversions;
struct Conversion_Chart *page1_conversions;
struct Raw_Runtime_Std *raw_runtime;
struct Runtime_Std *runtime;
struct Runtime_Std *runtime_last;
struct Ve_Widgets *page0_widgets;
struct Ve_Widgets *page1_widgets;

void init()
{
	/* defaults */
	poll_min = 25;		/* 25 millisecond minimum poll delay */
	poll_step = 5;		/* 5 ms steps */
	poll_max = 500;		/* 500 millisecond maximum poll delay */
	interval_min = 25;	/* 25 millisecond minimum interval delay */
	interval_step = 5;	/* 5 ms steps */
	interval_max = 1000;	/* 1000 millisecond maximum interval delay */
	width = 700;		/* min window width */
	height = 550;		/* min window height */
	main_x_origin = 160;	/* offset from left edge of screen */
	main_y_origin = 120;	/* offset from top edge of screen */

	/* initialize all global variables to known states */
	def_comm_port = 1; /* DOS/WIN32 style, COM1 default */
	serial_params.fd = 0; /* serial port file-descriptor */
	serial_params.errcount = 0; /* I/O error count */
	serial_params.poll_timeout = 40; /* poll wait time in milliseconds */
	/* default for MS V 1.x and 2.x */
	serial_params.raw_bytes = 22; /* number of bytes for realtime vars */
	serial_params.veconst_size = 128; /* VE/Constants datablock size */
	serial_params.read_wait = 100;	/* delay between reads in milliseconds */

	/* Set flags to clean state */
	raw_reader_running = FALSE;  /* We're not reading raw data yet... */
	raw_reader_stopped = TRUE;  /* We're not reading raw data yet... */
	just_starting = TRUE; 	/* to handle initial errors */
	ms_reset_count = 0; 	/* Counts MS clock resets */
	ms_goodread_count = 0; 	/* How many reads of realtime vars completed */
	kpa_conversion = turbo_map;
	tips_in_use = TRUE;	/* Use tooltips by default */
	fahrenheit = TRUE;	/* Use SAE units by default */
}

int read_config(void)
{
	ConfigFile *cfgfile;
	gchar *filename;
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/config", NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		//cfg_read_int(cfgfile, "Global", "major_ver", &major_ver);
		//cfg_read_int(cfgfile, "Global", "minor_ver", &minor_ver);
		//cfg_read_int(cfgfile, "Global", "micro_ver", &micro_ver);
		cfg_read_boolean(cfgfile, "Global", "Tooltips", &tips_in_use);
		cfg_read_boolean(cfgfile, "Global", "Fahrenheit", &fahrenheit);
		cfg_read_int(cfgfile, "Window", "width", &width);
		cfg_read_int(cfgfile, "Window", "height", &height);
		cfg_read_int(cfgfile, "Window", "main_x_origin", 
				&main_x_origin);
		cfg_read_int(cfgfile, "Window", "main_y_origin", 
				&main_y_origin);
		cfg_read_int(cfgfile, "Serial", "comm_port", 
				&serial_params.comm_port);
		cfg_read_int(cfgfile, "Serial", "polling_timeout", 
				&serial_params.poll_timeout);
		cfg_read_int(cfgfile, "Serial", "read_delay", 
				&serial_params.read_wait);
		cfg_free(cfgfile);
		g_free(filename);
		return(0);
	}
	else
	{
		printf("Config file not found, using defaults\n");
		g_free(filename);
		return (-1);	/* No file found */
	}
}
void save_config(void)
{
	gchar *filename;
	int x,y,tmp_width,tmp_height;
	ConfigFile *cfgfile;
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/config", NULL);
	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		cfgfile = cfg_new();


	cfg_write_int(cfgfile, "Global", "major_ver", _MAJOR_);
	cfg_write_int(cfgfile, "Global", "minor_ver", _MINOR_);
	cfg_write_int(cfgfile, "Global", "micro_ver", _MICRO_);
	cfg_write_boolean(cfgfile, "Global", "Tooltips", tips_in_use);
	cfg_write_boolean(cfgfile, "Global", "Fahrenheit", fahrenheit);

	gdk_drawable_get_size(main_window->window, &tmp_width,&tmp_height);
	cfg_write_int(cfgfile, "Window", "width", tmp_width);
	cfg_write_int(cfgfile, "Window", "height", tmp_height);
	gdk_window_get_position(main_window->window,&x,&y);
	cfg_write_int(cfgfile, "Window", "main_x_origin", x);
	cfg_write_int(cfgfile, "Window", "main_y_origin", y);
	cfg_write_int(cfgfile, "Serial", "comm_port", 
			serial_params.comm_port);
	cfg_write_int(cfgfile, "Serial", "polling_timeout", 
			serial_params.poll_timeout);
	cfg_write_int(cfgfile, "Serial", "read_delay", 
			serial_params.read_wait);

	cfg_write_file(cfgfile, filename);
	cfg_free(cfgfile);

	g_free(filename);

}
void make_megasquirt_dirs(void)
{
	gchar *filename;

	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix", NULL);
	mkdir(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	g_free(filename);
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/VE_Tables", NULL);
	mkdir(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	g_free(filename);


}


void mem_alloc()
{
	/* Allocate memory blocks */
	ve_const_p0 = g_malloc(MS_PAGE_SIZE);
	ve_const_p1 = g_malloc(MS_PAGE_SIZE);
	ve_const_p0_tmp = g_malloc(MS_PAGE_SIZE);
	ve_const_p1_tmp = g_malloc(MS_PAGE_SIZE);
	raw_runtime = g_malloc(sizeof(struct Raw_Runtime_Std));
	runtime = g_malloc(sizeof(struct Runtime_Std));
	runtime_last = g_malloc(sizeof(struct Runtime_Std));
	page0_conversions =  g_malloc(sizeof(struct Conversion_Chart));
	page1_conversions =  g_malloc(sizeof(struct Conversion_Chart));
	page0_widgets = g_malloc(sizeof(struct Ve_Widgets));
	page1_widgets = g_malloc(sizeof(struct Ve_Widgets));
	
	/* Set memory blocks to known states... */
	memset((void *)ve_const_p0, 0, MS_PAGE_SIZE);
	memset((void *)ve_const_p1, 0, MS_PAGE_SIZE);
	memset((void *)ve_const_p0_tmp, 0, MS_PAGE_SIZE);
	memset((void *)ve_const_p1_tmp, 0, MS_PAGE_SIZE);
	memset((void *)raw_runtime, 0, sizeof(struct Raw_Runtime_Std));
	memset((void *)runtime, 0, sizeof(struct Runtime_Std));
	memset((void *)runtime_last, 0, sizeof(struct Runtime_Std));
	memset((void *)page0_conversions, 0, sizeof(struct Conversion_Chart));
	memset((void *)page1_conversions, 0, sizeof(struct Conversion_Chart));
	memset((void *)page0_widgets, 0, sizeof(struct Ve_Widgets));
	memset((void *)page1_widgets, 0, sizeof(struct Ve_Widgets));
}

void mem_dealloc()
{

	g_free(ve_const_p0);
	g_free(ve_const_p1);
	g_free(ve_const_p0_tmp);
	g_free(ve_const_p1_tmp);
	g_free(raw_runtime);
	g_free(runtime);
	g_free(runtime_last);
	g_free(page0_conversions);
	g_free(page1_conversions);
	g_free(page0_widgets);
	g_free(page1_widgets);
	//	printf("Deallocating memory \n");
}
