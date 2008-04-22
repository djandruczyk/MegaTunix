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

#include <enums.h>
#include <xmlcomm.h>
#include <gtk/gtk.h>

typedef struct _Io_Message Io_Message;
typedef struct _Text_Message Text_Message;
typedef struct _QFunction QFunction;
typedef struct _Widget_Update Widget_Update;
typedef struct _OutputData OutputData;


/*
 \brief _QFunction strcture is used for a thread to pass messages up
 a GAsyncQueue to the main gui thread for running any arbritrary function
 by name.
 */
struct _QFunction
{
	gchar *func_name;	/*! Function Name */
	gint  dummy;		/*! filler for more shit later.. */
};


/*
 \brief _Widget_Update strcture is used for a thread to pass a widget update
 call up a GAsyncQueue to the main gui thread for updating a widget in 
 a thread safe manner. A dispatch queue runs periodically checking 
 for messages to dispatch...
 */
struct _Widget_Update
{
	gchar *widget_name;	/*! Widget name */
	WidgetType type;	/*! what type of widget are we updating */
	gchar *msg;		/*! message to display */
};


/*! 
 \brief _OutputData A simple wrapper struct to pass data to the output 
 function which makes the function a lot simpler.
 */
struct _OutputData
{
	GObject *object;	/*! Opaque object for data storage */
	gboolean need_page_change;	/*! Set to true to force page change */
	gboolean queue_update;	/*! If true queues a member widget update */
};


/*!
 \brief _Io_Message structure is used for passing data around in threads.c for
 kicking off commands to send data to/from the ECU or run specified handlers.
 messages and postfunctiosn can be bound into this strucutre to do some complex
 things with a simple subcommand.
 \see Io_Cmd
 */
struct _Io_Message
{
	GArray *functions;	/*! for gui_dispatch_queue */
	GArray *sequence;	/*! for sending data to ECU */
	InputHandler handler;	/*! Command handler for inbound data */
	void *payload;		/*! data passed along, arbritrary size.. */
	void *recv_buf;		/* data that comes from ECU */
	Command *command;	/* Command struct */
};


/*
 \brief _Text_Message strcture is used for a thread to pass messages up
 a GAsyncQueue to the main gui thread for updating a textview in a thread
 safe manner. A dispatch queue runs 5 times per second checking for messages
 to dispatch...
 */
struct _Text_Message
{
	gchar *view_name;	/*! Textview name */
	gchar *tagname;		/*! Texttag to use */
	gchar *msg;		/*! message to display */
	gboolean count;		/*! display a counter */
	gboolean clear;		/*! Clear the window? */
};


/* Prototypes */
void io_cmd(gchar *, gpointer);	/* Send message down the queue */
void *thread_dispatcher(gpointer);	/* thread that processes messages */
void *restore_update(gpointer);		/* Thread to update tools status.. */
void start_restore_monitor(void);	/* Thread jumpstarter */
void send_to_ecu(gint, gint, gint, DataSize, gint, gboolean);
void thread_update_logbar(gchar *, gchar *, gchar *, gboolean, gboolean);
void thread_update_widget(gchar *, WidgetType, gchar *);
gboolean queue_function(gchar * );
void chunk_write(gint, gint, gint, gint, guint8 *);
void build_output_string(Io_Message *, Command *, gpointer);
/* Prototypes */

#endif
