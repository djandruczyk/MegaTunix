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


#ifndef _PROTOS_H_
#define _PROTOS_H

#include <config.h>
#include "defines.h"
#include <gtk/gtk.h>

/* Function Prototypes for all objects/source files */

/* ms_constants.c */
void post_process(struct ms_raw_data_v1_and_v2 *, struct ms_data_v1_and_v2 *);
/* ms_constants.c */

/* serialio.c */
int open_serial(int); /* arg is COMM (dos/win style) port number */
int setup_serial_params(void); /* Setups serial i/o settings */
void close_serial(void); /* arg is filedescriptor that was opened */
void handle_ms_data(int); /* arg is filedescriptor that was opened */
int check_ecu_comms(GtkWidget *, gpointer *);
void read_ve_const(void);
void update_const_ve(void);
void write_ve_const(void);

/* serialio.c */

/* threads.c */
void * serial_raw_thread_starter(void *); /* bootstrap function to get IO started */
void * serial_raw_thread_stopper(void *); /* Realtime thread stopper */
void * raw_reader_thread(void *); /* Serial raw reader thread */
int stop_serial_thread();	/* cancels reader thread */
void start_serial_thread(void);	/*bootstrp function fopr above */
/* threads.c */

/* Configfile.c function protos, derived from XMMS */
ConfigFile *cfg_new(void);
ConfigFile *cfg_open_file(gchar * filename);
gboolean cfg_write_file(ConfigFile * cfg, gchar * filename);
void cfg_free(ConfigFile * cfg);
gboolean cfg_read_string(ConfigFile * cfg, gchar * section, \
		gchar * key, gchar ** value);
gboolean cfg_read_int(ConfigFile * cfg, gchar * section, \
		gchar * key, gint * value);
gboolean cfg_read_boolean(ConfigFile * cfg, gchar * section, \
		gchar * key, gboolean * value);
gboolean cfg_read_float(ConfigFile * cfg, gchar * section, \
		gchar * key, gfloat * value);
gboolean cfg_read_double(ConfigFile * cfg, gchar * section, \
		gchar * key, gdouble * value);
void cfg_write_string(ConfigFile * cfg, gchar * section, \
		gchar * key, gchar * value);
void cfg_write_int(ConfigFile * cfg, gchar * section, \
		gchar * key, gint value);
void cfg_write_boolean(ConfigFile * cfg, gchar * section, \
		gchar * key, gboolean value);
void cfg_write_float(ConfigFile * cfg, gchar * section, \
		gchar * key, gfloat value);
void cfg_write_double(ConfigFile * cfg, gchar * section, \
		gchar * key, gdouble value);
void cfg_remove_key(ConfigFile * cfg, gchar * section, gchar * key);
/* Configfile.c function protos, derived from XMMS */

/* init.c */
void init(void);
int read_config(void);
void save_config(void);
void make_megasquirt_dirs(void);
void mem_alloc(void);
void mem_dealloc(void);
/* init.c */

/* core_gui.c */
int setup_gui(void);
int framebuild_dispatch(GtkWidget *, int);
/* core_gui.c */

/* gui_handlers.c */
void leave(GtkWidget *, gpointer *);
int std_button_handler(GtkWidget *, gpointer *);
void text_entry_handler(GtkWidget *, gpointer *);
void update_statusbar(GtkWidget *, int, gchar *);
int reqd_fuel_popup();
int update_reqd_fuel(GtkWidget *widget, gpointer *data);
int close_popup(GtkWidget *widget, gpointer *data);
int spinner_changed(GtkWidget *, gpointer *);

/* gui_handlers.c */

/* for each of the *_gui.c files will be  a main core function called 
 * int build_****(Gtkwidget *).  The "****" will be that core function 
 * wheter it be "tools", "datalogging", "constants", or whatever.  The 
 * arg is a "Gtkwidget *" which is the PARENT widget to which all gui
 * sub-ojects must be placed into (multi-level embedding is encouraged 
 * to improve the viewability of the data.
 */
/* about_gui.c */
int build_about(GtkWidget *);
/* abou_guit.c */

/* comms_gui.c */
int build_comms(GtkWidget *);
void update_errcounts(void);
/* comms_gui.c */

/* constants_gui.c */
int build_constants(GtkWidget *);
/* constants_gui.c */

/* runtime_gui.c */
int build_runtime(GtkWidget *);
void update_runtime_vars(void);
/* runtime_gui.c */

/* enrichments_gui.c */
int build_enrichments(GtkWidget *);
/* enrichments_gui.c */

/* vetable_gui.c */
int build_vetable(GtkWidget *);
/* vetable_gui.c */

/* tuning_gui.c */
int build_tuning(GtkWidget *);
/* tuning_gui.c */

/* tools_gui.c */
int build_tools(GtkWidget *);
/* tools_gui.c */

/* datalogging_gui.c */
int build_datalogging(GtkWidget *);
/* datalogging_gui.c */

/* general_gui.c */
int build_general(GtkWidget *);
/* general_gui.c */

/* main.c */
gboolean file_exists(const char *);
/* main.c */

#endif
