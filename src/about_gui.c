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
	GtkWidget *vbox;
	GtkWidget *alignment;
	GdkPixbuf *pixbuf;
	GtkWidget *drawing_area;
	gint w,h;

	vbox = gtk_vbox_new(FALSE,10);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	buffer = g_strdup_printf("MegaTunix %s Tuning Software for Unix/Linux\n\tdesigned by David J. Andruczyk",VERSION);
	label = gtk_label_new(buffer);
	g_free(buffer);
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);

	alignment = gtk_alignment_new(0.5,0.5,0,0);
	gtk_box_pack_start(GTK_BOX(vbox),alignment,TRUE,FALSE,0);

	pixbuf = gdk_pixbuf_new_from_inline(sizeof(Logo),Logo,TRUE,NULL);

	drawing_area = gtk_drawing_area_new();
	w = gdk_pixbuf_get_width (pixbuf);
        h = gdk_pixbuf_get_height (pixbuf);
	gtk_widget_set_size_request (GTK_WIDGET (drawing_area), w, h);

	gtk_container_add (GTK_CONTAINER (alignment), drawing_area);

	g_signal_connect (drawing_area, "expose_event",
                          G_CALLBACK (expose_event), NULL);
        g_signal_connect (drawing_area, "configure_event",
                          G_CALLBACK (config_event), NULL);

	g_object_set_data (G_OBJECT (drawing_area), "pixbuf", pixbuf);
	g_object_set_data (G_OBJECT (drawing_area), "parent", vbox);


	return(0);
}

