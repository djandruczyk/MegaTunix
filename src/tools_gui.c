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

GtkWidget *tools_statbar;
gint tools_context_id;
extern GdkColor white;

struct Tools tools;

int build_tools(GtkWidget *parent_frame)
{
        GtkWidget *vbox;
        GtkWidget *vbox2;
        GtkWidget *ebox;
        GtkWidget *label;
	GtkWidget *entry;
        GtkWidget *frame;
        GtkWidget *table;
        GtkWidget *button;
	extern GtkTooltips *tip;

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


	table = gtk_table_new(3,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);
	

	ebox = gtk_event_box_new();
	gtk_tooltips_set_tip(tip,ebox,
	"Export VE Table(s) will create a .vex file with the contents of your VEtable(s) (more tables if tyou are using the DualTable mode). NOTE: This button is greyed out until you fill in the comment field to the right and HIT ENTER",NULL);
        gtk_table_attach (GTK_TABLE (table), ebox, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
	button = gtk_button_new_with_label("Export VE Table(s)");
	tools.export_but = button;
	gtk_widget_set_sensitive(button,FALSE);
	gtk_container_add(GTK_CONTAINER(ebox),button);
        g_signal_connect (G_OBJECT(button), "clicked",
                        G_CALLBACK (std_button_handler),
                        GINT_TO_POINTER(EXPORT_VETABLE));


	label = gtk_label_new("VEX File Comment");
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
	entry = gtk_entry_new();
	tools.export_comment_entry = entry;
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 50);
        g_signal_connect (G_OBJECT(entry), "activate",
                        G_CALLBACK (vex_comment_parse),
                        GINT_TO_POINTER(0));

        gtk_table_attach (GTK_TABLE (table), entry, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_button_new_with_label("Import VE Table(s)");
	gtk_tooltips_set_tip(tip,button,
	"Import VE Table(s) wil attempt to load a .vex file and populate you MS's VETable(s) with the contents of this vex file. See the statusbar at the bottom for the results of this operation.",NULL);
        g_signal_connect (G_OBJECT(button), "clicked",
                        G_CALLBACK (std_button_handler),
                        GINT_TO_POINTER(IMPORT_VETABLE));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

/*
	button = gtk_button_new_with_label("Clear VEX File");
        g_signal_connect (G_OBJECT(button), "clicked",
                        G_CALLBACK (std_button_handler),
                        GINT_TO_POINTER(CLEAR_VEXFILE));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
*/

	return TRUE;
}
