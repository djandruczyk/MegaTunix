/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"
#include "runtime_gui.h"


struct v1_2_Runtime_Gui runtime_data;

int build_runtime(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *scale;
	GtkWidget *sep;
	GtkObject *adj;

	//	memset(runtime_data,0,sizeof(struct v1_2_Runtime_Gui));

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	frame = gtk_frame_new("Real Time Variables");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

//	hbox = gtk_hbox_new(FALSE,0);
//	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	label = gtk_label_new("Battery Voltage");
        gtk_box_pack_start(GTK_BOX(vbox2),label,TRUE,TRUE,0);

        adj = gtk_adjustment_new((float)0,1.0,18.0,1.0,1.0,1.0);
        scale = gtk_hscale_new(GTK_ADJUSTMENT(adj));
	runtime_data.batt_adj = adj;
        gtk_scale_set_digits(GTK_SCALE(scale),1);
        gtk_box_pack_start(GTK_BOX(vbox2),scale,TRUE,TRUE,0);
        gtk_range_set_update_policy(GTK_RANGE (scale),
                        GTK_UPDATE_CONTINUOUS);

	sep = gtk_hseparator_new();
        gtk_box_pack_start(GTK_BOX(vbox2),sep,TRUE, TRUE, 0);

	label = gtk_label_new("Throttle Position");
        gtk_box_pack_start(GTK_BOX(vbox2),label,TRUE,TRUE,0);

        adj = gtk_adjustment_new((float)0,1.0,255.0,1.0,1.0,1.0);
        scale = gtk_hscale_new(GTK_ADJUSTMENT(adj));
	runtime_data.tps_adj = adj;
        gtk_scale_set_digits(GTK_SCALE(scale),1);
        gtk_box_pack_start(GTK_BOX(vbox2),scale,TRUE,TRUE,0);
        gtk_range_set_update_policy(GTK_RANGE (scale),
                        GTK_UPDATE_CONTINUOUS);

	//	
	/* Not written yet */
	return TRUE;
}

void update_runtime_vars()
{
//	char buff[10];
	/* test to see if data changed 
	 * Why bother wasting CPU to update the GUI when 
	 * you'd just print the same damn thing?
	 * Makes the code a little uglier, but the gui won't
	 * flicker the text widgets at high update rates
	 */
	if (out.batt != out_last.batt)
		gtk_adjustment_set_value(GTK_ADJUSTMENT(runtime_data.batt_adj),\
				out.batt);
	if (out.tps != out_last.tps)
		gtk_adjustment_set_value(GTK_ADJUSTMENT(runtime_data.tps_adj),\
				out.tps*51);
//	if (out.mat != out_last.mat)
//	{
//		g_snprintf(buff,10,"%.2f",out.mat);
//		gtk_entry_set_text(GTK_ENTRY(runtime_data.mat_val),buff);
//	}
}
	
