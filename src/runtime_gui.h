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

#ifndef _RUNTIME_GUI_H_
#define _RUNTIME_GUI_H_

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>

struct v1_2_Runtime_Gui
{
	GtkWidget *secl_val;		/* Counter entry */
	GtkWidget *baro_val;		/* baro entry from MS */
	GtkWidget *map_val;		/* map entry from MS */
	GtkWidget *mat_val;		/* mat entry from MS */
	GtkWidget *clt_val;		/* clt entry from MS */
	GtkWidget *tps_val;		/* tps entry from MS */
	GtkWidget *batt_val;		/* batt entry from MS */
	GtkWidget *ego_val;		/* ego entry from MS */
	GtkWidget *egocorr_val;		/* egocorr entry from MS */
	GtkWidget *aircorr_val;		/* aircorr entry from MS */
	GtkWidget *warmcorr_val;	/* warmcorr entry from MS */
	GtkWidget *rpm_val;		/* rpm entry from MS */
	GtkWidget *pw_val;		/* pw entry from MS */
	GtkWidget *tpsaccel_val;	/* tpsaccel entry from MS */
	GtkWidget *barocorr_val;	/* barocorr entry from MS */
	GtkWidget *gammae_val;		/* gammae entry from MS */
	GtkWidget *vecurr_val;		/* vecurr entry from MS */
};


#endif
