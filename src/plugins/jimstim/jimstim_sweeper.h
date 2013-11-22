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

/*!
  \file src/plugins/jimstim/jimstim_sweeper.h
  \ingroup JimStimPlugin,Headers
  \brief JimStim sweeper code
  \author David Andruczyk
  */
 
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __JIMSTIM_SWEEPER_H__
#define __JIMSTIM_SWEEPER_H__

#include <gtk/gtk.h>
#include <jimstim_plugin.h>
#include <threads.h>

typedef struct _JimStim_Data JimStim_Data;

/*!
  \brief _JimStim_Data is a container of all widgets specific to the jimstim
  gui for automated control of the jimstim
  */
struct _JimStim_Data
{
	GtkWidget *manual_rpm_e;/*!< manual rpm entry */
	GtkWidget *start_e;	/*!< start entry */
	GtkWidget *end_e;	/*!< end entry */
	GtkWidget *step_e;	/*!< step entry */
	GtkWidget *sweeptime_e;	/*!< sweep time entry */
	GtkWidget *start_b;	/*!< start button */
	GtkWidget *stop_b;	/*!< end button */
	GtkWidget *rpm_e;	/*!< commanded rpm entry */
	GtkWidget *step_rb;	/*!< Favor step radiobutton */
	GtkWidget *sweeptime_rb;/*!< Favor sweep radiobutton */
	GtkWidget *frame;	/*!< JS controls frame */
	GtkWidget *manual_f;	/*!< Manual RPM control frame */
	gint manual_rpm;	/*!< Manual RPM value */
	gint start;		/*!< start value */
	gint end;		/*!< end value */
	gint step;		/*!< step value */
	gfloat sweep;		/*!< sweep value */
	gint current;		/*!< Current value */
	gint sweep_id;		/*!< Timeout identifier */
	gboolean reset;		/*!< Reset */
};

/* Prototypes */
G_MODULE_EXPORT gboolean jimstim_rpm_sweep(JimStim_Data *);
G_MODULE_EXPORT gboolean jimstim_rpm_sweep_wrapper(JimStim_Data *);
G_MODULE_EXPORT gboolean jimstim_sweep_end(GtkWidget *, gpointer);
G_MODULE_EXPORT gboolean jimstim_sweep_start(GtkWidget *, gpointer);
G_MODULE_EXPORT void jimstim_sweeper_init(GtkWidget *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
