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



int build_about(GtkWidget *frame)
{
	char buffer[80];
	GtkWidget *label;
	GtkWidget *box;

	box = gtk_vbox_new(TRUE,10);
	gtk_container_add (GTK_CONTAINER (frame), box);
	sprintf(buffer,"MegaTunix %s\n designed by David J. Andruczyk",VERSION);
	label = gtk_label_new(buffer);
	gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);
        gtk_widget_show_all (box);

	return(0);
}

