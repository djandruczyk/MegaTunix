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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"


gint major_ver;
gint minor_ver;
gint micro_ver;
extern int def_comm_port;
extern int read_wait_time;
extern int reset_count;
extern int just_starting;
extern int raw_reader_running;
extern int raw_reader_stopped;
extern int main_x_origin;
extern int main_y_origin;
extern int width;
extern int height;
extern GtkWidget *main_window;

void init()
{
	/* defaults */
	poll_min = 5;		/* 5 millisecond minimum poll delay */
	poll_max = 500;		/* 500 millisecond maximum poll delay */
	interval_min = 25;	/* 25 millisecond minimum interval delay */
	interval_max = 1000;	/* 1000 millisecond maximum interval delay */


	/* initialize all global variables to known states */
	serial_params.fd = 0; /* serial port file-descriptor */
	def_comm_port = 1; /* DOS/WIN32 style, COM1 default */
	serial_params.errcount = 0; /* I/O error count */
	serial_params.poll_timeout = 40; /* poll wait time in milliseconds */
	/* default for MS V 1.1 and 2.2 */
	serial_params.raw_bytes = 22; /* number of bytes for realtime vars */
	raw_reader_running = 0;  /* We're not reading raw data yet... */
	raw_reader_stopped = 1;  /* We're not reading raw data yet... */
	read_wait_time = 1000;	/* delay between reads in milliseconds */
	just_starting = 1; 	/* to handle initial errors */
	reset_count = 0; 	/* to handle initial errors */
	width = 640;		/* default window width */
	height = 480;		/* default window height */
	main_x_origin = 160;	/* offset from left edge of screen */
	main_y_origin = 120;	/* offset from top edge of screen */


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
		cfg_read_int(cfgfile, "Window", "width", &width);
		cfg_read_int(cfgfile, "Window", "height", &height);
		cfg_read_int(cfgfile, "Window", "main_x_origin", &main_x_origin);
		cfg_read_int(cfgfile, "Window", "main_y_origin", &main_y_origin);
		cfg_read_int(cfgfile, "Serial", "comm_port", &serial_params.comm_port);
		cfg_read_int(cfgfile, "Serial", "polling_timeout", &serial_params.poll_timeout);
		cfg_read_int(cfgfile, "Serial", "read_delay", &read_wait_time);
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

	gdk_window_get_size(main_window->window, &tmp_width,&tmp_height);
	cfg_write_int(cfgfile, "Window", "width", tmp_width);
	cfg_write_int(cfgfile, "Window", "height", tmp_height);
	gdk_window_get_position(main_window->window,&x,&y);
	cfg_write_int(cfgfile, "Window", "main_x_origin", x);
	cfg_write_int(cfgfile, "Window", "main_y_origin", y);
	cfg_write_int(cfgfile, "Serial", "comm_port", serial_params.comm_port);
	cfg_write_int(cfgfile, "Serial", "polling_timeout", serial_params.poll_timeout);
	cfg_write_int(cfgfile, "Serial", "read_delay", read_wait_time);


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
//	printf("Allocating memory \n");
}

void mem_dealloc()
{
//	printf("Deallocating memory \n");
}
