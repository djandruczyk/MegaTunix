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

#ifndef __DASHBOARD_H__
#define __DASHBOARD_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


typedef struct _Dash_Gauge Dash_Gauge;
/*! 
 \brief The _Dash_Gauge struct contains info on the dashboard guages for 
 megatunix's two potential dashboards.
 */
struct _Dash_Gauge
{
	GObject *object;		/* Data storage object for RT vars */
	gchar * source;			/* Data Source name */
	GtkWidget *gauge;		/* pointer to gauge itself */
	GtkWidget *dash;		/* pointer to gauge parent */
};

/* Prototypes */
void load_dashboard(gchar *, gpointer);
gboolean remove_dashboard(GtkWidget *, gpointer );
void load_elements(GtkWidget *, xmlNode * );
void load_geometry(GtkWidget *, xmlNode *);
void load_gauge(GtkWidget *, xmlNode *);
void update_dash_gauge(gpointer , gpointer , gpointer );
void link_dash_datasources(GtkWidget *,gpointer);
void dash_shape_combine(GtkWidget *, gboolean);
gboolean dash_motion_event(GtkWidget *, GdkEventMotion *, gpointer );
gboolean dash_button_event(GtkWidget *, GdkEventButton *, gpointer );
gboolean dash_key_event(GtkWidget *, GdkEventKey *, gpointer );
void initialize_dashboards(void);
gboolean present_dash_filechooser(GtkWidget *, gpointer );
gboolean remove_dashcluster(gpointer, gpointer , gpointer );
gboolean dummy(GtkWidget *,gpointer );
EXPORT void create_gauge(GtkWidget *);
gboolean hide_dash_resizers(gpointer );
void update_tab_gauges(void);
/* Prototypes */

#endif
