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

extern gboolean tips_in_use;
extern gboolean fahrenheit;

int build_general(GtkWidget *parent_frame)
{
//        extern GtkTooltips *tip;
        GtkWidget *vbox;
 //       GtkWidget *vbox2;
  //      GtkWidget *label;
        GtkWidget *frame;
        GtkWidget *hbox;
        GtkWidget *hbox2;
        GtkWidget *button;
        GtkWidget *table;
	GSList *group;

        vbox = gtk_vbox_new(FALSE,0);
        gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	hbox = gtk_hbox_new(TRUE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	frame = gtk_frame_new("Context Sensitive Help");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);
	
	hbox2 = gtk_hbox_new(TRUE,0);
        gtk_container_add(GTK_CONTAINER(frame),hbox2);
	
	button = gtk_check_button_new_with_label("Use ToolTips");
	gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),tips_in_use);
	g_signal_connect (G_OBJECT(button), "toggled",
                        G_CALLBACK (toggle_button_handler),
                        GINT_TO_POINTER(TOOLTIPS_STATE));

	frame = gtk_frame_new("Fahrenheit or Celsius Temp scales.");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);

	table = gtk_table_new(1,5,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),table);
	
	button = gtk_radio_button_new_with_label(NULL,"Fahrenheit");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	if (fahrenheit)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(toggle_button_handler),
                        GINT_TO_POINTER(FAHRENHEIT));
	

	button = gtk_radio_button_new_with_label(group,"Celsius");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	if (!fahrenheit)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(toggle_button_handler),
                        GINT_TO_POINTER(CELSIUS));

	frame = gtk_frame_new("MegaSquirt Type Selection");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	
//	table = gtk_table_new(	




	

	return TRUE;
}
