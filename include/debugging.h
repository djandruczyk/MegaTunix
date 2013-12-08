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
  \file include/debugging.h
  \ingroup Headers
  \brief Handler for debug management. This SHOULD BE REFACTORED to use macros
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DEBUG_GUI_H__
#define __DEBUG_GUI_H__

#include <config.h>
#include <gtk/gtk.h>

/* Debugging Enumerations */
typedef enum
{
	NO_DEBUG		= 0,
	INTERROGATOR 	= 1<<0,
	OPENGL			= 1<<1,
	CONVERSIONS		= 1<<2,
	SERIAL_RD		= 1<<3,
	SERIAL_WR		= 1<<4,
	IO_MSG			= 1<<5,
	IO_PROCESS		= 1<<6,
	THREADS			= 1<<7,
	REQ_FUEL		= 1<<8,
	TABLOADER		= 1<<9,
	KEYPARSER		= 1<<10,
	RTMLOADER		= 1<<11,
	COMPLEX_EXPR	= 1<<12,
	MTXSOCKET		= 1<<13,
	PLUGINS			= 1<<14,
	PACKETS			= 1<<15,
	DISPATCHER		= 1<<16,
#ifdef DEBUG
	FUNC			= 1<<17,
#endif
	CRITICAL		= 1<<30
}Dbg_Class;

typedef enum 
{
	INTERROGATOR_SHIFT	= 0,
	OPENGL_SHIFT		= 1,
	CONVERSIONS_SHIFT	= 2,
	SERIAL_RD_SHIFT		= 3,
	SERIAL_WR_SHIFT		= 4,
	IO_MSG_SHIFT		= 5,
	IO_PROCESS_SHIFT	= 6,
	THREADS_SHIFT		= 7,
	REQ_FUEL_SHIFT		= 8,
	TABLOADER_SHIFT		= 9,
	KEYPARSER_SHIFT		= 10,
	RTMLOADER_SHIFT		= 11,
	COMPLEX_EXPR_SHIFT	= 12,
	MTXSOCKET_SHIFT		= 13,
	PLUGINS_SHIFT		= 14,
	PACKETS_SHIFT		= 15,
	DISPATCHER_SHIFT	= 16,
#ifdef DEBUG
	FUNC_SHIFT			= 17,
#endif
	CRITICAL_SHIFT		= 30
}Dbg_Shift;


/* When inside a plugin, use the alt dest function name (namespace issues) */
#ifdef IN_PLUGIN
 #define MTXDBG(level, ...) dbg_func_f((Dbg_Class)(level),__FILE__,__FUNCTION__,__LINE__, __VA_ARGS__)
 #define QUIET_MTXDBG(level, ...) dbg_func_f((Dbg_Class)(level),NULL,NULL,0, __VA_ARGS__)
 #ifdef DEBUG
  #define ENTER() dbg_func_f((Dbg_Class)(FUNC),NULL,NULL,0,"Entered: %s\n",__FUNCTION__)
  #define EXIT() dbg_func_f((Dbg_Class)(FUNC),NULL,NULL,0,"Leaving: %s\n",__FUNCTION__)
 #else
  #define ENTER() 
  #define EXIT() 
 #endif
#else
 /* When inside mtx core, use the default dest function name */
 #define MTXDBG(level, ...) dbg_func((Dbg_Class)(level),__FILE__,__FUNCTION__,__LINE__, __VA_ARGS__)
 #define QUIET_MTXDBG(level, ...) dbg_func((Dbg_Class)(level),NULL,NULL,0, __VA_ARGS__)
 #ifdef DEBUG
  #define ENTER() dbg_func((Dbg_Class)(FUNC),NULL,NULL,0,"Entered: %s\n",__FUNCTION__)
  #define EXIT() dbg_func((Dbg_Class)(FUNC),NULL,NULL,0,"Leaving: %s\n",__FUNCTION__)
 #else
  #define ENTER() 
  #define EXIT() 
 #endif
#endif

typedef struct _DebugLevel DebugLevel;

/*!
 \brief _DebugLevel stores the debugging name, handler, class (bitmask) and 
 shift (forgot why this is here) and a enable/disable flag. Used to make the
 debugging core a little more configurable
 */
struct _DebugLevel
{
	const gchar * name;		/*!< Debugging name */
	gint	handler;	/*!< Signal handler name */
	Dbg_Class dclass;	/*!< Bit mask for this level (0-31) */
	Dbg_Shift dshift;	/*!< Bit shift amount */
	gboolean enabled;	/*!< Enabled or not? */
};


/* Prototypes */
void close_debug(void);
void dbg_func(Dbg_Class, const gchar *, const gchar *, gint, const gchar *, ...);
void open_debug(void);
void populate_debugging(GtkWidget *);
/* Prototypes */

#endif

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
