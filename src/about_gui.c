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
#include <globals.h>
#include <logo.h>
#include <about_gui.h>


int build_about(GtkWidget *frame)
{
	char *buffer;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *alignment;
	GdkPixbuf *pixbuf;
	GtkWidget *image;

	vbox = gtk_vbox_new(FALSE,10);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	buffer = g_strdup_printf("MegaTunix %s Tuning Software for Unix/Linux\n\tdesigned by David J. Andruczyk",VERSION);
	label = gtk_label_new(buffer);
	g_free(buffer);
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);

	alignment = gtk_alignment_new(0.5,0.5,0,0);
	gtk_box_pack_start(GTK_BOX(vbox),alignment,TRUE,FALSE,0);

	pixbuf = gdk_pixbuf_new_from_inline(sizeof(Logo),Logo,TRUE,NULL);

	image = gtk_image_new_from_pixbuf(pixbuf);

	gtk_container_add (GTK_CONTAINER (alignment), image);

	return(0);
}

