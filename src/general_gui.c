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
#include <enums.h>
#include <general_gui.h>
#include <globals.h>
#include <gui_handlers.h>
#include <interrogate.h>

extern gboolean tips_in_use;
extern gboolean fahrenheit;
extern GdkColor black;
GtkWidget *ms_ecu_revision_entry;
GtkWidget *view;
GtkWidget *sw;
GtkTextBuffer *textbuffer;

int build_general(GtkWidget *parent_frame)
{
        extern GtkTooltips *tip;
        GtkWidget *vbox;
 //       GtkWidget *vbox2;
  //      GtkWidget *label;
        GtkWidget *frame;
        GtkWidget *hbox;
        GtkWidget *hbox2;
        GtkWidget *button;
        GtkWidget *table;
        GtkWidget *label;
        GtkWidget *entry;
        GtkWidget *ebox;
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

	ebox = gtk_event_box_new();
        gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,TRUE,0);
        gtk_tooltips_set_tip(tip,ebox,
        "This box shows you the MegaSquirt Version number returned from the ECU. NOTE: The V1 MegaSquirt processors (first public release from around 2000/2001) do NOT return a version number so we display it as v1.0",NULL);

	frame = gtk_frame_new("MegaSquirt ECU Information");
	gtk_container_add(GTK_CONTAINER(ebox),frame);
        hbox = gtk_hbox_new(TRUE,0);
        gtk_container_add(GTK_CONTAINER(frame),hbox);

        table = gtk_table_new(2,4,FALSE);
        gtk_table_set_row_spacings(GTK_TABLE(table),7);
        gtk_table_set_col_spacings(GTK_TABLE(table),5);
        gtk_container_set_border_width(GTK_CONTAINER(table),5);
        gtk_box_pack_start(GTK_BOX(hbox),table,FALSE,TRUE,20);

        label = gtk_label_new("ECU Revision Number");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

        entry = gtk_entry_new();
	ms_ecu_revision_entry = entry;
        gtk_entry_set_width_chars (GTK_ENTRY (entry), 12);
        gtk_widget_set_sensitive(entry,FALSE);
        gtk_widget_modify_text(entry,GTK_STATE_INSENSITIVE,&black);
        gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	ebox = gtk_event_box_new();
        gtk_tooltips_set_tip(tip,ebox,
        "This button interrogates the connected ECU to attempt to determine what firmware is loaded and to setup the gui to adapt to the capabilities of the loaded version. This method is not 100\% foolproof, so we offer the choice to select the API to use below",NULL);
        gtk_table_attach (GTK_TABLE (table), ebox, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	button = gtk_button_new_with_label("Interrogate ECU capabilities");
	gtk_container_add(GTK_CONTAINER(ebox),button);
	g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (interrogate_ecu), \
                        NULL);

	view = gtk_text_view_new ();
 	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
//	sw = gtk_scrolled_window_new (NULL, NULL);
	frame = gtk_frame_new ("ECU Output");
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
        gtk_table_attach (GTK_TABLE (table), frame, 0, 2, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

//	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
//				      GTK_POLICY_AUTOMATIC,
//				      GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(frame),view);


	




	

	return TRUE;
}
