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
	GtkWidget *drawing_area;
	GError *error;
	gint w,h;

	box = gtk_vbox_new(FALSE,10);
	gtk_container_add (GTK_CONTAINER (frame), box);
	buffer = g_strdup_printf("MegaTunix %s Tuning Software for Unix/Linux\n\tdesigned by David J. Andruczyk",VERSION);
	label = gtk_label_new(buffer);
	g_free(buffer);
	gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);

	pixbuf = gdk_pixbuf_new_from_inline(sizeof(Logo),Logo,TRUE,NULL);
	drawing_area = gtk_drawing_area_new();
	gtk_box_pack_start(GTK_BOX(box),drawing_area,TRUE,TRUE,0);
	w = gdk_pixbuf_get_width (pixbuf);
        h = gdk_pixbuf_get_height (pixbuf);
	gtk_widget_set_size_request (GTK_WIDGET (drawing_area), w, h);

	g_signal_connect (drawing_area, "expose_event",
                          G_CALLBACK (expose_event), NULL);
        g_signal_connect (drawing_area, "configure_event",
                          G_CALLBACK (config_event), NULL);

	g_object_set_data (G_OBJECT (drawing_area), "pixbuf", pixbuf);
	g_object_set_data (G_OBJECT (drawing_area), "parent", box);


        gtk_widget_show_all (box);


	return(0);
}

