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
#include <protos.h>
#include <defines.h>
#include <globals.h>
#include <MegaTunix.xpm>



int build_about(GtkWidget *frame)
{
	char buffer[100];
	GtkWidget *label;
	GtkWidget *box;
	GdkPixbuf *pbuf;
	GtkWidget *darea;

	box = gtk_vbox_new(TRUE,10);
	gtk_container_add (GTK_CONTAINER (frame), box);
	sprintf(buffer,"MegaTunix %s Tuning Software for Unix/Linux\n\tdesigned by David J. Andruczyk",VERSION);
	label = gtk_label_new(buffer);
	gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);
        gtk_widget_show_all (box);

	pbuf = gdk_pixbuf_new_from_xpm_data((char *)MegaTunix_xpm);

	return(0);
}

