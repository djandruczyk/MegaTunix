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

#ifndef __RUNTIME_GUI_H__
#define __RUNTIME_GUI_H__

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>

	/* This structure contains all the gui pointers to the 
	 * objects used on the runtime_gui.  These are needed  
	 * so the handlers have something to reference when updating
	 * the widget in question when data arrives....
	 * A Struct was just more convienent and kept things together.
	 */

struct v1_2_Runtime_Gui
{
	GtkWidget *secl_lab;		/* Counter label */
	GtkWidget *ego_lab;		/* O2 Voltage */
	GtkWidget *ego_pbar;		/* O2 Voltage bar */
	GtkWidget *baro_lab;		/* baro label from MS */
	GtkWidget *baro_pbar;		/* O2 Voltage bar */
	GtkWidget *map_lab;		/* map label from MS */
	GtkWidget *map_pbar;		/* map value for bar */
	GtkWidget *mat_lab;		/* mat label from MS */
	GtkWidget *mat_pbar;		/* map value for bar */
	GtkWidget *clt_lab;		/* clt label from MS */
	GtkWidget *clt_pbar;		/* map value for bar */
	GtkWidget *tps_lab;		/* tps label from MS */
	GtkWidget *tps_pbar;		/* map value for bar */
	GtkWidget *batt_lab;		/* batt label from MS */
	GtkWidget *batt_pbar;		/* map value for bar */
	GtkWidget *egocorr_lab;		/* egocorr label from MS */
	GtkWidget *egocorr_pbar;	/* egocorr label from MS */
	GtkWidget *aircorr_lab;		/* aircorr label from MS */
	GtkWidget *aircorr_pbar;	/* aircorr label from MS */
	GtkWidget *warmcorr_lab;	/* warmcorr label from MS */
	GtkWidget *warmcorr_pbar;	/* warmcorr label from MS */
	GtkWidget *rpm_lab;		/* rpm label from MS */
	GtkWidget *rpm_pbar;		/* rpm label from MS */
	GtkWidget *pw_lab;		/* pw label from MS */
	GtkWidget *pw_pbar;		/* pw label from MS */
	GtkWidget *tpsaccel_lab;	/* tpsaccel label from MS */
	GtkWidget *tpsaccel_pbar;	/* tpsaccel label from MS */
	GtkWidget *barocorr_lab;	/* barocorr label from MS */
	GtkWidget *barocorr_pbar;	/* barocorr label from MS */
	GtkWidget *gammae_lab;		/* gammae label from MS */
	GtkWidget *gammae_pbar;		/* gammae label from MS */
	GtkWidget *vecurr_lab;		/* vecurr label from MS */
	GtkWidget *vecurr_pbar;		/* vecurr label from MS */
	GtkWidget *dcycle_lab;		/* vecurr label from MS */
	GtkWidget *dcycle_pbar;		/* vecurr label from MS */
	GtkWidget *status[7];		/* Status boxes */
};


#endif
