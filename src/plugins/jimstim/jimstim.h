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

#ifndef __JIMSTIM_H__
#define __JIMSTIM_H__

#include <gtk/gtk.h>
#include <threads.h>

/* Externs */
extern void (*error_msg_f)(const gchar *);
extern GtkWidget *(*lookup_widget_f)(const gchar *);
extern void (*io_cmd_f)(const gchar *,void *);
extern OutputData *(*initialize_outputdata_f)(void);
extern void *(*dbg_func_f)(int,gchar *);
extern void (*start_tickler_f)(gint);
extern void (*stop_tickler_f)(gint);
extern GList *(*get_list_f)(gchar *);
extern void (*set_widget_sensitive_f)(gpointer, gpointer);
extern void (*update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean, gboolean);
/* Externs */

typedef struct _JimStim_Data JimStim_Data;

struct _JimStim_Data
{
	GtkWidget *start_e;	/* start entry */
	GtkWidget *end_e;	/* end entry */
	GtkWidget *step_e;	/* step entry */
	GtkWidget *sweep_e;	/* sweep time entry */
	GtkWidget *start_b;	/* start button */
	GtkWidget *stop_b;	/* end button */
	GtkWidget *rpm_e;	/* commanded rpm entry */
	GtkWidget *step_rb;	/* Favor step radiobutton */
	GtkWidget *sweep_rb;	/* Favor sweep radiobutton */
	GtkWidget *frame;	/* JS controls frame */
	gint start;		/* start value */
	gint end;		/* end value */
	gint step;		/* step value */
	gfloat sweep;		/* sweep value */
	gint current;		/* Current value */
	gint sweep_id;		/*! Timeout identifier */
	gboolean reset;		/*! Reset*/
};

/* Prototypes */
G_MODULE_EXPORT gboolean jimstim_sweep_start(GtkWidget *, gpointer);
G_MODULE_EXPORT gboolean jimstim_sweep_end(GtkWidget *, gpointer);
G_MODULE_EXPORT gboolean jimstim_rpm_sweep(JimStim_Data *);
G_MODULE_EXPORT void jimstim_sweeper_init(GtkWidget *);
/* Prototypes */

#endif
