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
#include <gui_handlers.h>
#include <interrogate.h>
#include <math.h>
#include <structures.h>
#include <tabloader.h>
#include <widgetmgmt.h>

extern gboolean tips_in_use;
extern gint temp_units;
extern GdkColor black;
extern guint dbg_lvl;
GtkWidget *ms_ecu_revision_entry;

GtkTextBuffer *textbuffer;

static struct DebugLevel dbglevels[] = {
{ "Interrogation", DEBUG_LEVEL, INTERROGATOR, INTERROGATOR_SHIFT, FALSE},
{ "OpenGL", DEBUG_LEVEL, OPENGL, OPENGL_SHIFT, FALSE},
{ "Conversions", DEBUG_LEVEL, CONVERSIONS, CONVERSIONS_SHIFT, FALSE},
{ "Serial Reads", DEBUG_LEVEL, SERIAL_RD, SERIAL_RD_SHIFT, FALSE},
{ "Serial Writes", DEBUG_LEVEL, SERIAL_WR, SERIAL_WR_SHIFT, FALSE},
{ "I/O Processing", DEBUG_LEVEL, IO_PROCESS, IO_PROCESS_SHIFT, FALSE},
{ "Threads", DEBUG_LEVEL, THREADS, THREADS_SHIFT, FALSE},
{ "Req Fuel", DEBUG_LEVEL, REQ_FUEL, REQ_FUEL_SHIFT, FALSE},
{ "Tabloader", DEBUG_LEVEL, TABLOADER, TABLOADER_SHIFT, FALSE},
{ "RealTime Maps", DEBUG_LEVEL, RTMLOADER, RTMLOADER_SHIFT, FALSE},
{ "Complex Math", DEBUG_LEVEL, COMPLEX_EXPR, COMPLEX_EXPR_SHIFT, FALSE},
{ "Critical Errors", DEBUG_LEVEL, CRITICAL, CRITICAL_SHIFT, TRUE},
};


void build_general(GtkWidget *parent_frame)
{
	extern GtkTooltips *tip;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *sw;
	GtkWidget *view;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *ebox;
	gint i,j,k;
	gint shift = 0;
	gint mask = 0;
	GSList *group;
	gint num_levels = sizeof(dbglevels)/sizeof(dbglevels[0]);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	hbox = gtk_hbox_new(TRUE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	frame = gtk_frame_new("Context Sensitive Help");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);

	hbox2 = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox2);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2),3);

	button = gtk_check_button_new_with_label("Use ToolTips");
	gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,2);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),tips_in_use);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(TOOLTIPS_STATE));
	g_signal_connect (G_OBJECT(button), "toggled",
			G_CALLBACK (toggle_button_handler),
			NULL);

	frame = gtk_frame_new("Fahrenheit or Celsius Temp scales.");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);

	table = gtk_table_new(1,5,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_container_set_border_width(GTK_CONTAINER(table),3);

	button = gtk_radio_button_new_with_label(NULL,"Fahrenheit");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	if (temp_units == FAHRENHEIT)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(FAHRENHEIT));
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);


	button = gtk_radio_button_new_with_label(group,"Celsius");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	if (temp_units == CELSIUS)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(CELSIUS));
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,
			"This box gives you the debugging choices.  Each one is independantly selectable.  Logging output will appear on the console that MegaTunix was started from...",NULL);

	frame = gtk_frame_new("MegaTunix Debugging");
	gtk_container_add(GTK_CONTAINER(ebox),frame);
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	table = gtk_table_new(ceil(num_levels/4),4,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,TRUE,5);

	j = 0;
	k = 0;
	for (i=0;i<num_levels;i++)
	{
		mask = dbglevels[i].dclass;
		shift = dbglevels[i].dshift;

		button = gtk_check_button_new_with_label(dbglevels[i].name);
		g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(dbglevels[i].handler));
		g_object_set_data(G_OBJECT(button),"bitshift",GINT_TO_POINTER(shift));
		g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(mask));
		g_object_set_data(G_OBJECT(button),"bitval",GINT_TO_POINTER(1));
		g_signal_connect(G_OBJECT(button),"toggled",
				G_CALLBACK(bitmask_button_handler),
				NULL);
		gtk_table_attach (GTK_TABLE (table), button, j, j+1, k, k+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
		// if hardcoded on, turn on..
		if (dbglevels[i].enabled)
			gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),TRUE);
		// If user set on turn on as well
		if ((dbg_lvl & mask) >> shift)
			gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),TRUE);
		j++;
		if (j == 4)
		{
			k++;
			j = 0;
		}
	}

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,
			"This box shows you the MegaSquirt Interrogation report.  Due to the rise of various MegaSquirt variants, several of them unfortunately return the same version number except that their API's aren't compatible.  This window give you some feedback about how the MS responds to various commands and suggests what it thinks is the closest match.",NULL);

	frame = gtk_frame_new("MegaSquirt ECU Information");
	gtk_container_add(GTK_CONTAINER(ebox),frame);
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	table = gtk_table_new(4,5,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),7);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(vbox2),table,TRUE,TRUE,5);

	ebox = gtk_event_box_new();
	gtk_tooltips_set_tip(tip,ebox,
			"This button interrogates the connected ECU to attempt to determine what firmware is loaded and to setup the gui to adapt to the capabilities of the loaded version. This method is not 100\% foolproof, so we offer the choice to select the API to use below",NULL);
	gtk_table_attach (GTK_TABLE (table), ebox, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	button = gtk_button_new_with_label("Interrogate ECU capabilities");
	gtk_container_add(GTK_CONTAINER(ebox),button);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(INTERROGATE_ECU));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);

	hbox = gtk_hbox_new(FALSE,18);
	gtk_table_attach (GTK_TABLE (table), hbox, 3, 5, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("ECU Revision Number");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

	entry = gtk_entry_new();
	register_widget("ecu_revision_entry",entry);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);

	label = gtk_label_new("Extended Revision");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 5, 1, 2,
			(GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new();
	register_widget("ext_revision_entry",entry);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 64);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,0);


	label = gtk_label_new("ECU Signature");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 5, 2, 3,
			(GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new();
	register_widget("ecu_signature_entry",entry);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 64);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,0);

	ebox = gtk_event_box_new();
	gtk_tooltips_set_tip(tip,ebox,
			"This window shows the status of the ECU interrogation progress.  The way it works is that we send commands to the ECU and count how much data is returned, which helps us hone in to which firmware for the MS is in use.  This method is not 100\% foolproof, as some firmware editions return the same amount of data, AND the same version number making them indistinguishable from the outside interface.  The commands sent are:\n \"A\" which returns the runtime variables (22 bytes usually)\n \"C\" which should return the MS clock (1 byte,  but this call fails on the (very old) version 1 MS's)\n \"Q\" Which should return the version number of the firmware multipled by 10\n \"V\" which should return the VEtable and constants, this size varies based on the firmware\n \"S\" which is a \"Signature Echo\" used in some of the variants.  Similar to the \"?\" command (Extend version)\n \"I\" which returns the igntion table and related constants (ignition variants ONLY)\n The \"F0/1\" Commands return the raw memory of the MegaSquirt ECU, This is only supported on some firmware and is used for debugging and manipulating features that do NOT have gui controls yet.  As of April 2004 MegaTunix does NOT yet support raw memory viewing/editing.",NULL);

	gtk_table_attach (GTK_TABLE (table), ebox, 0, 5, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
			(GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL), 0, 0);

	frame = gtk_frame_new ("ECU Output");
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	sw = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
	//	gtk_widget_set_size_request(sw,-1,200);
	gtk_box_pack_start(GTK_BOX(vbox2),sw,TRUE,TRUE,5);

	view = gtk_text_view_new ();
	register_widget("interr_view",view);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view),FALSE);
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_create_tag(textbuffer,
			"warning",
			"foreground",
			"red", NULL);
	gtk_container_add(GTK_CONTAINER(sw),view);

	return;
}
