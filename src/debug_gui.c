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
#include <defines.h>
#include <debug_gui.h>
#include <enums.h>
#include <gui_handlers.h>
#include <structures.h>
#include <vex_support.h>

GtkWidget *debug_view;

void build_debug(GtkWidget *parent_frame)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *sw;
	GtkWidget *view;
	GtkAdjustment *adj;
	GtkWidget *spinner;
	GtkTextBuffer *textbuffer;
	//extern GtkTooltips *tip;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	frame = gtk_frame_new("Debug Status Messages");
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
	gtk_box_pack_end(GTK_BOX(vbox),frame,TRUE,TRUE,0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	sw = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(sw,0,65);
	gtk_box_pack_start(GTK_BOX(hbox),sw,TRUE,TRUE,5);

	view = gtk_text_view_new();
	debug_view = view;
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view),FALSE);
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_create_tag(textbuffer,
			"warning",
			"foreground",
			"red", NULL);
	gtk_container_add(GTK_CONTAINER(sw),view);

	frame = gtk_frame_new("Debugging Level Settings");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,TRUE,TRUE,0);

	label = gtk_label_new("Debug Level");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,3);

	label = gtk_label_new(NULL);
	gtk_box_pack_end(GTK_BOX(hbox),label,TRUE,TRUE,3);

	adj = (GtkAdjustment *)gtk_adjustment_new(0,0,9,1,1,0);
	spinner = gtk_spin_button_new(adj,1,0);;

	g_object_set_data(G_OBJECT(spinner),"info",(gpointer)label);
	gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,10);
	g_signal_connect(G_OBJECT (spinner), "value_changed",
			G_CALLBACK (spin_button_handler), \
			GINT_TO_POINTER(DEBUG_LEVEL));

	return;
}
