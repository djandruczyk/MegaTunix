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
#include <gui_handlers.h>
#include <structures.h>
#include <tabloader.h>
#include <tools_gui.h>
#include <vex_support.h>

extern GdkColor white;
extern struct DynamicButtons buttons;
extern struct DynamicLabels labels;
extern struct DynamicEntries entries;

void build_tools(GtkWidget *parent_frame)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *button;
	GtkWidget *entry;
	GtkWidget *sw;
	GtkWidget *view;
	GtkTextBuffer *textbuffer;
	//extern GtkTooltips *tip;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	frame = gtk_frame_new("Tools Status Messages");
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
	register_widget("tools_view",view);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view),FALSE);
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_create_tag(textbuffer,
			"warning",
			"foreground",
			"red", NULL);
	gtk_container_add(GTK_CONTAINER(sw),view);

	frame = gtk_frame_new("VE/Spark Table Import/Export (VEX Files)");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,TRUE,TRUE,0);

	button = gtk_button_new_with_label("Import VE/Spark Table(s)");
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,3);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(IMPORT_VETABLE));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);

	button = gtk_button_new_with_label("Export VE/Spark Table(s)");
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,3);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(EXPORT_VETABLE));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);

	button = gtk_button_new_with_label("Revert to Last");
	buttons.tools_revert_but = button;
	gtk_widget_set_sensitive(button,FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,3);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(REVERT_TO_BACKUP));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,TRUE,TRUE,0);

	label = gtk_label_new("Vex File Comment");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

	entry = gtk_entry_new();
	entries.vex_comment_entry = entry;
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 50);
	g_signal_connect (G_OBJECT(entry), "activate",
			G_CALLBACK (vex_comment_parse),
			GINT_TO_POINTER(0));
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,0);

	frame = gtk_frame_new("Backup/Restore All Settings");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,TRUE,TRUE,0);

	button = gtk_button_new_with_label("Backup All MS Parameters");
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(BACKUP_ALL));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);

	button = gtk_button_new_with_label("Restore All MS Parameters");
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(RESTORE_ALL));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);


	return;
}
