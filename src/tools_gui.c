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
#include <globals.h>
#include <gui_handlers.h>
#include <structures.h>
#include <tools_gui.h>
#include <vex_support.h>

extern GdkColor white;
extern struct DynamicButtons buttons;
extern struct DynamicLabels labels;
extern struct DynamicEntries entries;
struct Tools tools;
GtkWidget *tools_statbar;
gint tools_context_id;

int build_tools(GtkWidget *parent_frame)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *ebox;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *button;
	GtkWidget *entry;
	//extern GtkTooltips *tip;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	frame = gtk_frame_new("Tools Status Messages");
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	tools_statbar = gtk_statusbar_new();
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(tools_statbar),FALSE);
	gtk_container_add(GTK_CONTAINER(ebox),tools_statbar);
	tools_context_id = gtk_statusbar_get_context_id(
			GTK_STATUSBAR(tools_statbar),
			"Tools Status");
	gtk_widget_modify_bg(GTK_WIDGET(ebox),
			GTK_STATE_NORMAL,&white);


	frame = gtk_frame_new("VE Table Export (VEX Files)");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,TRUE,TRUE,0);

	button = gtk_button_new_with_label("Select VEX File");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(SELECT_VEXFILE));

	label = gtk_label_new("No VEX File Selected Yet");
	labels.vex_file_lab = label;
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,30);

	button = gtk_button_new_with_label("Export VE Table(s)");
	buttons.ve_export_but = button;
	gtk_widget_set_sensitive(button,FALSE);
	gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,FALSE,3);
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(EXPORT_VETABLE));

	button = gtk_button_new_with_label("Clear VEX File");
	buttons.ve_clear_vex_but = button;
	gtk_widget_set_sensitive(button,FALSE);
	gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,FALSE,3);
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(TRUNCATE_VEXFILE));

	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,TRUE,TRUE,0);

	label = gtk_label_new("Vex File Comment");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

	entry = gtk_entry_new();
	entries.vex_comment_entry = entry;
	tools.export_comment_entry = entry;
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 50);
	g_signal_connect (G_OBJECT(entry), "activate",
			G_CALLBACK (vex_comment_parse),
			GINT_TO_POINTER(0));
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,10);


	return TRUE;
}
