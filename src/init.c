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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <defines.h>
#include <protos.h>
#include <globals.h>
#include <configfile.h>


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
struct ve_const_std *ve_constants;
struct ve_const_std *ve_const_tmp;
struct raw_runtime_std *raw_runtime;
struct runtime_std *runtime;
struct runtime_std *runtime_last;

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
	/* default for MS V 1.1 and 2.2 */
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
	tips_in_use = TRUE;	/* USe tooltips by default */
}

int read_config(void)
{
	ConfigFile *cfgfile;
	gchar *filename;
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/config", NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		cfg_read_int(cfgfile, "Global", "major_ver", &major_ver);
		cfg_read_int(cfgfile, "Global", "minor_ver", &minor_ver);
		cfg_read_int(cfgfile, "Global", "micro_ver", &micro_ver);
		cfg_read_boolean(cfgfile, "Global", "Tooltips", &tips_in_use);
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

	ve_const_tmp = malloc(sizeof(struct ve_const_std));
	ve_constants = malloc(sizeof(struct ve_const_std));
	raw_runtime = malloc(sizeof(struct raw_runtime_std));
	runtime = malloc(sizeof(struct runtime_std));
	runtime_last = malloc(sizeof(struct runtime_std));
	
	//	printf("Allocating memory \n");
}

void mem_dealloc()
{

	free(ve_const_tmp);
	free(ve_constants);
	free(raw_runtime);
	free(runtime);
	free(runtime_last);
	//	printf("Deallocating memory \n");
}
