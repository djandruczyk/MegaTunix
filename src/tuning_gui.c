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

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <defines.h>
#include <protos.h>
#include <globals.h>

int build_tuning(GtkWidget *parent_frame)
{
        GtkWidget *vbox;
        GtkWidget *label;

        vbox = gtk_vbox_new(FALSE,0);
        gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
        label = gtk_label_new("Not Implemented yet");
        gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);



	/* Not written yet */
	return TRUE;
}
