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

struct v1_2_Runtime_Gui
{
	GtkWidget *secl_ent;		/* Counter entry */
	GtkWidget *baro_ent;		/* baro entry from MS */
	GtkWidget *map_ent;		/* map entry from MS */
	GtkWidget *mat_ent;		/* mat entry from MS */
	GtkWidget *clt_ent;		/* clt entry from MS */
	GtkWidget *tps_ent;		/* tps entry from MS */
	GtkWidget *batt_ent;		/* batt entry from MS */
	GtkWidget *ego_ent;		/* ego entry from MS */
	GtkWidget *egocorr_ent;		/* egocorr entry from MS */
	GtkWidget *aircorr_ent;		/* aircorr entry from MS */
	GtkWidget *warmcorr_ent;	/* warmcorr entry from MS */
	GtkWidget *rpm_ent;		/* rpm entry from MS */
	GtkWidget *pw_ent;		/* pw entry from MS */
	GtkWidget *tpsaccel_ent;	/* tpsaccel entry from MS */
	GtkWidget *barocorr_ent;	/* barocorr entry from MS */
	GtkWidget *gammae_ent;		/* gammae entry from MS */
	GtkWidget *vecurr_ent;		/* vecurr entry from MS */
};


#endif
