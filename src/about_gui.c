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

#include <gdk-pixbuf/gdk-pixdata.h>
#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <protos.h>
#include <defines.h>
#include <globals.h>
#include <logo.h>



int build_about(GtkWidget *frame)
{
	char *buffer;
	GtkWidget *label;
	GtkWidget *box;
	GdkPixbuf *pixbuf;
	GtkWidget *image;
	GError *error;

	box = gtk_vbox_new(FALSE,10);
	gtk_container_add (GTK_CONTAINER (frame), box);
	buffer = g_strdup_printf("MegaTunix %s Tuning Software for Unix/Linux\n\tdesigned by David J. Andruczyk",VERSION);
	label = gtk_label_new(buffer);
	g_free(buffer);
	gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);

	pixbuf = gdk_pixbuf_from_pixdata(&Logo,FALSE,&error);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_box_pack_start(GTK_BOX(box),image,TRUE,TRUE,0);

        gtk_widget_show_all (box);


	return(0);
}

