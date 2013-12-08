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
  \file include/serialio.h
  \ingroup Headers
  \brief Header for serial IO handlers
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __SERIALIO_H__
#define __SERIALIO_H__

#include <gtk/gtk.h>
#include <config.h>
#include <defines.h>
#include <gio/gio.h>
#ifndef __WIN32__
 #include <termios.h>
#endif

typedef struct _Serial_Params Serial_Params;

/*!
 \brief _Serial_Params holds all variables related to the state of the serial
 port being used by MegaTunix
 */
#ifdef __WIN32__
struct _Serial_Params
{
#if GTK_MINOR_VERSION >= 18
	GSocket *socket;	/*!< Network mode socket */
	GSocket *ctrl_socket;	/*!< Network mode socket */
#else
	gint ctrl_fd;		/*!< Network mode ctrl socket fd */
#endif
	gint fd;		/*!< File descriptor */
	gchar *port_name;	/*!< textual name of comm port */
	gboolean open;		/*!< flag, TRUE for open FALSE for closed */
	gint read_wait;		/*!< time delay between each read */
	gint errcount;		/*!< Serial I/O errors read error count */
	gboolean net_mode;	/*!< When using TCP/IP socket mode */
};

#else
struct _Serial_Params
{
#if GTK_MINOR_VERSION >= 18
	GSocket *socket;	/*!< Network mode socket */
	GSocket *ctrl_socket;	/*!< Network mode socket */
#else
	gint ctrl_fd;		/*!< Network mode control socket */
#endif
	gint fd;		/*!< File descriptor */
	gchar *port_name;	/*!< textual name of comm port */
	gboolean open;		/*!< flag, TRUE for open FALSE for closed */
	gint read_wait;		/*!< time delay between each read */
	gint errcount;		/*!< Serial I/O errors read error count */
	gboolean net_mode;	/*!< When using TCP/IP socket mode */
	struct termios oldtio;	/*!< serial port settings before we touch it */
	struct termios newtio;	/*!< serial port settings we use when running */
	/* Old PIS disabled, PIS author non-responsive
	struct serial_struct oldctl;
	struct serial_struct newctl;
	*/
};
#endif

typedef enum
{
	NONE,
	ODD,
	EVEN
}Parity;

typedef enum
{
	INBOUND=0x2E0,
	OUTBOUND,
	BOTH
}FlushDirection;

/* Prototypes */
void close_serial(void);
void flush_serial(gint,FlushDirection);
gboolean open_serial(gchar *, gboolean);
gboolean parse_baud_str(gchar *, gint *, gint *, Parity *, gint *);
void *serial_repair_thread(gpointer );
void setup_serial_params(void);
void toggle_serial_control_lines(void );
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
