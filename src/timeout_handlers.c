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
#include <enums.h>
#include <gui_handlers.h>
#include <runtime_gui.h>
#include <logviewer_gui.h>
#include <timeout_handlers.h>

static gint runtime_id = 0;
static gint logviewer_id = 0;
static gfloat update_rate = 24;

void start_runtime_display()
{
	if (runtime_id == 0)
		runtime_id = gtk_timeout_add((int)((1.0/update_rate)*1000.0),
				(GtkFunction)update_runtime_vars,NULL);
	if (logviewer_id == 0)
		logviewer_id = gtk_timeout_add((int)((1.0/update_rate)*1000.0),
				(GtkFunction)update_logview_traces,NULL);
}

void stop_runtime_display()
{
	if (runtime_id)
		gtk_timeout_remove(runtime_id);
	runtime_id = 0;
	if (logviewer_id)
		gtk_timeout_remove(logviewer_id);
	logviewer_id = 0;
}

gboolean populate_gui()
{
	/* A trick used in main() to startup MegaTunix faster..
	 * the problem is that calling the READ_VE_CONST stuff before 
	 * gtk_main is that it makes the gui have to go through interrogation
	 * of the ecu before the gui appears, giving the appearance that
	 * MegaTunix is slow.  By creating this simple wrapper and kicking
	 * it off as a timeout, it'll run just after the gui is ready, and
	 * since it returns FALSE, the timeout will be canceled and deleted
	 * i.e. it acts like a one-shot behind a time delay. (the delay lets
	 * the gui get "ready" and then this kicks off the interrogator and
	 * populates the gui controls if the ECU is detected... 
	 */

	std_button_handler(NULL,GINT_TO_POINTER(READ_VE_CONST));
	return FALSE;
}
