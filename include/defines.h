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
  \file include/defines.h
  \ingroup Headers
  \brief Special definitions, macros and platform magic
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <config.h>
#include <libintl.h>
#include <math.h>

/* Definitions */
#define BAUDRATE B9600

#define _POSIX_SOURCE 1 /* POSIX compliant source */

/* C99 weirdness */
#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#define DEFAULT_PROJECT "default"
#define BACKUP_DATA_DIR "ECU_Backups"
#define DATALOG_DATA_DIR "Datalogs"
#define TABLE_DATA_DIR "Tables"
/* Windows specific for exporting symbols for glade... */
#ifdef __WIN32__ 
#define MTXSYSDATA "c:\\program files\\megatunix\\dist"
#define DEFAULT_PORT "COM1"
#define PSEP "\\"
#else
#define MTXSYSDATA DATA_DIR
#define DEFAULT_PORT "/dev/ttyS0"
#define PSEP "/"
#endif
#define HOME g_get_home_dir

/* Fancy casting tricks */
#ifdef __64BIT__
#define GINT gint)(gint64
#define GBOOLEAN gboolean)(gint64
#else
#define GINT gint
#define GBOOLEAN gboolean
#endif

/* attributes for unused vars/functions */
#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#ifdef __GNUC__
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#  define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif

/* Gettext support
 */
#define _(x) gettext(x)

/* g_object_get/set macros */

#define OBJ_GET(object, name) g_object_get_data(G_OBJECT(object),name)
#define OBJ_SET(object, name, data) g_object_set_data(G_OBJECT(object),name,data)
#define OBJ_SET_FULL(object, name, data, func) g_object_set_data_full(G_OBJECT(object),name,data,(GDestroyNotify)func)

/* g_dataset_get/set macros */

#define DATA_GET(dataset, name) g_dataset_get_data(dataset,name)
#define DATA_SET(dataset, name, data) g_dataset_set_data(dataset,name,data)
#define DATA_SET_FULL(dataset, name, data, func) g_dataset_set_data_full(dataset,name,data,(GDestroyNotify)func)

/* Download modes */
#define IMMEDIATE		0x10
#define DEFERRED		0x11
#define IGNORED			0x12

/* TCP socket goodness */
#define MTX_SOCKET_ASCII_PORT 12764 /* (ascii math) (m*t)+x */
#define MTX_SOCKET_BINARY_PORT 12765
#define MTX_SOCKET_CONTROL_PORT 12766

/* For datalogging and Logviewer */
#define TABLE_COLS 5

/*! Temp Sensor default Bias resistor value */
#define DEFAULT_BIAS 2490

#define TEMP_C_100_DENSITY 21.1111
#define TEMP_K_100_DENSITY 294.2611

#define C_TO_K 273

#endif

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
