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
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"
#include "constants.h"

extern struct v1_2_Constants constants;

/* arrays of the info for the combo boxes... */
const gchar *control_strat[] = {"Speed-Density", "Alpha-N"};
const gchar *inj_per_cycle[] = {"1-Squirt", "2-Squirts","3-Squirts","4-Squirts",
			       "5-Squirts","6-Squirts","7-Squirts","8-Squirts"};
const gchar *inj_staging[] = {"Simultaneous","Alternating"};
const gchar *engine_stroke[] = {"Four-Stroke","Two-Stroke"};
const gchar *num_of_cyls[] = {"1","2","3", "4", "5", "6",
			      "7","8","9","10","11","12"};
const gchar *inject_type[] = {"Multi-Port","Throttle-Body"};
const gchar *num_of_injectors[] = {"1","2","3", "4", "5", "6",
				   "7","8","9","10","11","12"};
const gchar *map_type[] = {"115 KPa","250 KPa"};
const gchar *engine_type[] = {"Even-Fire","Odd-Fire"};
const gchar *o2_sensor_type[] = {"NarrowBand","WideBand"};
const gchar *baro_correction[] = {"Disabled","Enabled"};


int build_constants(GtkWidget *parent_frame)
{
	gint i;
	gint total;
	GtkWidget *sep;
	GtkWidget *button;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *spinner;
	GtkWidget *combo;
	GtkAdjustment *adj;
	GList *items = NULL;
	
	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,5);

	frame = gtk_frame_new("Required Fuel - One Cylinder (ms)");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Calculate\nRequired Fuel...");
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(reqd_fuel_popup),
				NULL);
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	
	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
        gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_1));
        constants.req_fuel_1 = adj;
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
        gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_2));
	gtk_widget_set_sensitive(spinner,FALSE);
        constants.req_fuel_2 = adj;
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Injector Opening Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3),0);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,5);
	
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,10.0,0.1,1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(INJ_OPEN_TIME));
        constants.inj_open_time = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);

	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,10.0,0.1,1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(BATT_CORR));
        constants.batt_corr = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);

	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Inj. Open Time\n(ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Batt Voltage\nCorrection (ms/V)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Injector Current Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3),0);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,5);

	adj =  (GtkAdjustment *) gtk_adjustment_new(50.0,0.0,100.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(PWM_CUR_LIM));
        constants.pwm_curr_lim = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,10.0,0.1,1.0,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(PWM_TIME_THRES));
        constants.pwm_time_thresh = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("PWM Current\n Limit (%)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("PWM Time \nThreshold (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Fast Idle Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);


	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	adj =  (GtkAdjustment *) gtk_adjustment_new(140.0,0.0,250.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(FAST_IDLE_THRES));
        constants.fast_idle_thresh = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Fast Idle Threshold\n(Degrees F)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);


	frame = gtk_frame_new("Fuel Injection Control Strategy");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	/* Injection Strategy Section */
	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),10);
	gtk_container_add(GTK_CONTAINER(frame),table);

	total = sizeof(control_strat)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)control_strat[i]);
	}

	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,125,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);
	frame = gtk_frame_new("Injection Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	/* Injection Control Section */

	table = gtk_table_new(14,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),15);
	gtk_container_set_border_width(GTK_CONTAINER(table),10);
	gtk_container_add(GTK_CONTAINER(frame),table);
//	gtk_table_set_row_spacing(GTK_TABLE(table),1,0);
//	gtk_table_set_row_spacing(GTK_TABLE(table),3,0);
//	gtk_table_set_row_spacing(GTK_TABLE(table),5,0);
//	gtk_table_set_row_spacing(GTK_TABLE(table),7,0);
	
	items = NULL;
	total = sizeof(inj_per_cycle)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)inj_per_cycle[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("# of Injections\nper cycle");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	items = NULL;
	total = sizeof(inj_staging)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)inj_staging[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Injector Staging");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	sep = gtk_hseparator_new();
	gtk_table_attach (GTK_TABLE (table), sep, 0, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);
	items = NULL;
	total = sizeof(engine_stroke)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)engine_stroke[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Engine Stroke");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	items = NULL;
	total = sizeof(num_of_cyls)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)num_of_cyls[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("# of Cylinders");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 4, 5,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	sep = gtk_hseparator_new();
	gtk_table_attach (GTK_TABLE (table), sep, 0, 2, 5, 6,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);
	items = NULL;
	total = sizeof(inject_type)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)inject_type[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 0, 1, 6, 7,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Injection Type");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 7, 8,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	items = NULL;
	total = sizeof(num_of_injectors)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)num_of_injectors[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 1, 2, 6, 7,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("# of Injectors");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 7, 8,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	sep = gtk_hseparator_new();
	gtk_table_attach (GTK_TABLE (table), sep, 0, 2, 8, 9,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);
	items = NULL;
	total = sizeof(map_type)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)map_type[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 0, 1, 9, 10,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("MAP Type");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 10, 11,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	items = NULL;
	total = sizeof(engine_type)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)engine_type[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 1, 2, 9, 10,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Engine Type");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 10, 11,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	sep = gtk_hseparator_new();
	gtk_table_attach (GTK_TABLE (table), sep, 0, 2, 11, 12,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);
	items = NULL;
	total = sizeof(o2_sensor_type)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)o2_sensor_type[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 0, 1, 12, 13,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("O2 Sensor Type");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 13, 14,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	items = NULL;
	total = sizeof(baro_correction)/sizeof(gpointer);
	for (i=0;i<total;i++)
	{
		items = g_list_append(items, (gpointer)baro_correction[i]);
	}
	combo = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
	gtk_combo_set_value_in_list(GTK_COMBO(combo),TRUE,TRUE);
        gtk_widget_set_size_request(combo,105,-1);
	gtk_table_attach (GTK_TABLE (table), combo, 1, 2, 12, 13,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Baro Correction");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 13, 14,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Commands");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	
	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),50);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Get Data from ECU");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	
	button = gtk_button_new_with_label("Send Data to ECU");
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	
	/* Not written yet */
	return TRUE;
}
