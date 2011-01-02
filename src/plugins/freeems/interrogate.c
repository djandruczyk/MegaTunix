/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <interrogate.h>
#include <freeems_helpers.h>
#include <serialio.h>

extern GtkWidget *interr_view;

#define BUFSIZE 4096

/*!
 \brief interrogate_ecu() interrogates the target ECU to determine what
 firmware it is running.  It does this by reading a list of tests, sending
 those tests in turn, reading the responses and them comparing the group of
 responses against a list of interrogation profiles until it finds a match.
 */
G_MODULE_EXPORT gboolean interrogate_ecu(void)
{
	GAsyncQueue *queue = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	/* ECU has already been detected via comms test
	   Now we need to figure out its variant and adapt to it
	   */
	/* Send stream disable command */
	//stop_streaming();

	/* FreeEMS Interrogator NOT WRITTEN YET */
	return TRUE;
}
