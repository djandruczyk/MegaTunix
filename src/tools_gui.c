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
#include <tools_gui.h>

GtkWidget *tools_statbar;
gint tools_context_id;
extern GdkColor white;


int build_tools(GtkWidget *parent_frame)
{
        GtkWidget *vbox;
        GtkWidget *vbox2;
        GtkWidget *ebox;
        GtkWidget *frame;
        GtkWidget *table;
        GtkWidget *button;

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


	frame = gtk_frame_new("VE Table Export/Import (VEX Files)");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	table = gtk_table_new(1,5,TRUE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);
	

	button = gtk_button_new_with_label("Export VE Table(s)");
        g_signal_connect (G_OBJECT(button), "clicked",
                        G_CALLBACK (std_button_handler),
                        GINT_TO_POINTER(EXPORT_VETABLE));
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_button_new_with_label("Import VE Table(s)");
        g_signal_connect (G_OBJECT(button), "clicked",
                        G_CALLBACK (std_button_handler),
                        GINT_TO_POINTER(IMPORT_VETABLE));
        gtk_table_attach (GTK_TABLE (table), button, 3, 4, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);


	return TRUE;
}
