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

/* Runtime Gui Structures */

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>

struct v1_2_Runtime_Gui
{
	GtkObject *secl_adj;		/* Counter entry */
	GtkObject *baro_adj;		/* baro entry from MS */
	GtkObject *map_adj;		/* map entry from MS */
	GtkObject *mat_adj;		/* mat entry from MS */
	GtkObject *clt_adj;		/* clt entry from MS */
	GtkObject *tps_adj;		/* tps entry from MS */
	GtkObject *batt_adj;		/* batt entry from MS */
	GtkWidget *ego_val;		/* ego entry from MS */
	GtkWidget *egocorr_val;		/* ego entry from MS */
	GtkWidget *aircorr_val;		/* ego entry from MS */
	GtkWidget *warmcorr_val;	/* ego entry from MS */
	GtkWidget *rpm_val;		/* ego entry from MS */
	GtkWidget *pw_val;		/* ego entry from MS */
	GtkWidget *tpsaccel_val;	/* ego entry from MS */
	GtkWidget *barocorr_val;	/* ego entry from MS */
	GtkWidget *gammae_val;		/* ego entry from MS */
	GtkWidget *vecurr_val;		/* ego entry from MS */
};


