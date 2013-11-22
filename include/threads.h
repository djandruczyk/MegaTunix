/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file include/threads.h
  \ingroup Headers
  \brief Header for the core thread and thread message dispatch functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __THREADS_H__
#define __THREADS_H__

#include <enums.h>
#include <xmlcomm.h>
#include <gtk/gtk.h>

typedef struct _Io_Message Io_Message;
typedef struct _Gui_Message Gui_Message;
typedef struct _Text_Message Text_Message;
typedef struct _QFunction QFunction;
typedef struct _Widget_Range Widget_Range;
typedef struct _Widget_Update Widget_Update;
typedef struct _OutputData OutputData;


/*!
 \brief _QFunction strcture is used for a thread to pass messages up
 a GAsyncQueue to the main gui thread for running any arbritrary function
 by name.
 */
struct _QFunction
{
	const gchar *func_name;	/*!< Function Name */
	gint  dummy;		/*!< filler for more shit later.. */
};


/*!
 \brief _Widget_Update strcture is used for a thread to pass a widget update
 call up a GAsyncQueue to the main gui thread for updating a widget in 
 a thread safe manner. A dispatch queue runs periodically checking 
 for messages to dispatch...
 */
struct _Widget_Update
{
	const gchar *widget_name;	/*!< Widget name */
	WidgetType type;		/*!< type of widget are we updating */
	gchar *msg;			/*!< message to display (if any) */
	gboolean state;			/*!< state to set widget (MTX_SENSITIVE) */
	gchar *group_name;		/*!< Group name */
	GuiColor color;			/*!< Color */
};


/*!
 \brief Used to update a bunch of widgets in ecu_widgets from a thread context
 */
struct _Widget_Range
{
	gint page;		/*!< Page */
	gint offset;	/*!< Offset */
	guint len;		/*!< Length */
};


/*!
 \brief _OutputData A simple wrapper struct to pass data to the output 
 function which makes the function a lot simpler.
 */
struct _OutputData
{
	gconstpointer *data;	/*!< Opaque object for data storage */
	gboolean need_page_change;	/*!< Set to true to force page change */
	gboolean queue_update;	/*!< If true queues a member widget update */
};


/*!
 * \brief _Gui_Message is to passing gui message calls to the gui_dispatcher timeout This was split out of Io_Message due to rampant misuse of that structure
 */

struct _Gui_Message 
{
	GArray *functions;	/*!< for gui_dispatch_queue */
	void *payload;		/*!< data passed along, arbritrary size.. */
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
	gconstpointer *data;/*!< Opaque object for data storage */
	GArray *sequence;	/*!< for sending data to ECU */
	void *payload;		/*!< data passed along, arbritrary size.. */
	guint8 *recv_buf;	/*!< data that comes from ECU */
	Command *command;	/*!< Command struct */
	gboolean status;	/*!< True == success, false == failure */
};


/*!
 \brief _Text_Message strcture is used for a thread to pass messages up
 a GAsyncQueue to the main gui thread for updating a textview in a thread
 safe manner. A dispatch queue runs 5 times per second checking for messages
 to dispatch...
 */
struct _Text_Message
{
	const gchar *view_name;	/*!< Textview name */
	const gchar *tag_name;	/*!< Texttag to use */
	gchar *msg;		/*!< message to display */
	gboolean count;		/*!< display a counter */
	gboolean clear;		/*!< Clear the window? */
};


/* Prototypes */
void io_cmd(const gchar *, gpointer);	/* Send message down the queue */
gboolean queue_function(const gchar * );
void *thread_dispatcher(gpointer);	/* thread that processes messages */
void thread_refresh_widget(GtkWidget *);
void thread_refresh_widget_range(gint, gint, gint);
void thread_refresh_widgets_at_offset(gint, gint);
void thread_set_group_color(GuiColor, const gchar *);
void thread_update_logbar(const gchar *, const gchar *, gchar *, gboolean, gboolean);
void thread_update_widget(const gchar *, WidgetType, gchar *);
void thread_widget_set_sensitive(const gchar * widget_name, gboolean state);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
