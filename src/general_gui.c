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

int build_general(GtkWidget *parent_frame)
{
        extern GtkTooltips *tip;
        GtkWidget *vbox;
        GtkWidget *vbox2;
        GtkWidget *label;
        GtkWidget *frame;
        GtkWidget *hbox;

        vbox = gtk_vbox_new(FALSE,0);
        gtk_container_add(GTK_CONTAINER(parent_frame),vbox);


	/* Not written yet */
	return TRUE;
}
