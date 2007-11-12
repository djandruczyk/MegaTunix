/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef __THREADS_H__
#define __THREADS_H__

#include <gtk/gtk.h>
#include <enums.h>

/* Prototypes */
void io_cmd(Io_Command, gpointer);	/* Send message down the queue */
void *thread_dispatcher(gpointer);	/* thread that processes messages */
void *restore_update(gpointer);		/* Thread to update tools status.. */
void start_restore_monitor(void);	/* Thread jumpstarter */
void write_ve_const(GtkWidget *, gint, gint, gint, gboolean, gboolean);
void thread_update_logbar(gchar *, gchar *, gchar *, gboolean, gboolean);
void thread_update_widget(gchar *, WidgetType, gchar *);
gboolean queue_function(gchar * );
void chunk_write(gint, gint, gint, guchar *);
		
/* Prototypes */

#endif
