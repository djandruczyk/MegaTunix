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
#include <structures.h>

extern struct v1_2_Constants constants;
struct Adjustments adjustments;
struct Labels labels;
struct Buttons buttons;
extern GtkWidget *veconst_widgets_1[];
extern GdkColor black;

int build_constants(GtkWidget *parent_frame)
{
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
	GtkWidget *ebox;
	GtkAdjustment *adj;
	GSList	*group;
	extern GtkTooltips *tip;
	
	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);

//      Equation for determining Req_fuel_download from Req_fuel:
//
//      REQ_FUEL_DL = REQ*FUEL * (B * N)/NINJ
//
//      B = 1 if simultaneous, 2 = Alternate
//      N = divder_number = ncyl/numer_of_squirts
//      NINJ = Number of Inejctors   

	
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"  This should contain the injector(s) pulse width in milliseconds (no greater than 25.5) required to supply enough fuel for a single injection event at 14.7:1 AFR (stoichiometric) and 100% VE (Volumetric Efficiency).   The Calculator button will open a dialog for you to enter in some basic paramters of your engine and will calculate Reqd Fuel for you. (Hopefully correctly... )\n   NOTE: There are two adjustments visible in this frame,  The top one is the total required fuel per cycle for 1 cylinder, the bottom one is the amount injected with each squirt. This varies based on the number of injections per cycle, and whether you are using Alternate or Simultaneous injection as well. NOTE, you don't want the lower number to get below 1.5ms as the closer you get to zero the more chance of error due to injector opening/closing times, reducing tuning range significantly which can make getting the idle mixture right very very difficult.  In this case, decrease the number of squirts, or check to see if you can use smaller injectors.",NULL);
	frame = gtk_frame_new("Required Fuel - One Cylinder (ms)");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Calculate\nRequired Fuel...");
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
				GINT_TO_POINTER(REQD_FUEL_POPUP));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	
	/* Required Fuel Total/cycle Value*/
	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
        constants.req_fuel_total_spin = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(90));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(DEFERRED));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL));
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Required Fuel Per Squirt Value*/
	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
        constants.req_fuel_per_squirt_spin = spinner;
	gtk_widget_set_sensitive(spinner,FALSE);
	gtk_widget_modify_text(spinner,GTK_STATE_INSENSITIVE,&black);
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"  Injector Opening Time is the amount of time required for the injector to go from fully closed to fully open when full power is applied to the injector (13.2 Volts or so).  Injectors have internal mass and don't open instantaneously.  Typical values are around 1 millisecond. NOTE this will be your lower limit for the injector (can't open any less than this), thus if you can't get your idle to lean out, you can try reducing this value slightly (0.1 ms increments).\n   The battery voltage correction lengthens the injector pulse by this factor (milliseconds per volt, 0.1ms resolution) to compensate when battery voltage is lower than 13.8 Volts (cranking, or heavily loaded electrical system) ",NULL);

	frame = gtk_frame_new("Injector Opening Control");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3),0);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,5);
	
	/* Injector Open Time */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        constants.inj_open_time_spin = spinner;
        veconst_widgets_1[93] = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(93));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Inj. Open Time\n(ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Battery Voltage Correction Factor */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,10.0,0.1,1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        constants.batt_corr_spin = spinner;
        veconst_widgets_1[97] = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(97));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(60*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Batt Voltage\nCorrection (ms/V)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"  PWM (Pulse Width Modulation) is used to vary the current to the injectors.  Some cars use high impedance Injectors  (Hi-Z or \"Saturated\", over 10 ohms DC resistance typically), wherease other cars use low impedance injectors (\"Peak and Hold\", around 2 ohms, possibly less).\n   For driving high impedance injectors PWM is not needed, so it's best to set the PWM Current limit to 100% and the PWM Time Threshold to 25.5  (maximum).\n   For low impedance injectors (P&H type) setting of PWM is critical, as too low will prevent the injector from staying open, too high and you run the risk of burning out the injector (or the MS Flyback control circuitry), and borderline will cause the injector to not work in worst case situations (cold conditions, low battery, high electrical load).  The way it works with Peak and Hold injectors, is that for a brief period of time the injector is given full power to snap it open fast, then the power is limited via PWM in a \"Hold Mode\" until the injector is to be closed.  The PWM current limit is the percentage of maximum voltage that the injector gets in \"Hold\" mode, and the PWM Time threshold is how long the \"Peak\" period lasts.  Typical values are 25\% for MS V1 and MS V2 WITH the newer enhanced Flyback suppression board, and around 75\% for the stock MS V2 withthe simpler onboard flyback circuit.  The time threshold is typically at 1 millisecond,  Using values longer than 1 ms for the PWM time threshold are not recommended and may lead to injector failure. Your values may be significantly different.  See the MS FAQ for more information. ",NULL);
	frame = gtk_frame_new("Injector Current Control");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3),0);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,5);

	/* PWM Current Limit % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(50.0,0.0,100.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,1,0);
        constants.pwm_curr_lim_spin = spinner;
        veconst_widgets_1[95] = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(95));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("PWM Current\n Limit (%)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* PWM Time threshold */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,25.5,0.1,1.0,0);
        spinner = gtk_spin_button_new(adj,0,1);
        constants.pwm_time_max_spin = spinner;
        veconst_widgets_1[96] = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(96));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("PWM Time \nThreshold (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The Fast idle temp is the temperature at which if the engine is BELOW this threshold the fast idle solenoid will be activated.",NULL);
	frame = gtk_frame_new("Fast Idle Control");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* Fast Idle Temp Threshold */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,-40.0,215.0,1.0,10.0,0);
	adjustments.fast_idle_temp_adj = adj;
        spinner = gtk_spin_button_new(adj,0,0);
        constants.fast_idle_thresh_spin = spinner;
        veconst_widgets_1[121] = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"temp_dep",GINT_TO_POINTER(TRUE));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(121));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(40*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(ADD));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Fast Idle Threshold\n(Degrees F.)");
	labels.fastidletemp_lab = label;
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);

	frame = gtk_frame_new("Injection Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	/* Injection Control Section */
	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);

	/* Fuel Injection Control Strategy */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox3),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   This selects the mode by which the MegaSquirt ECU calculates the fuel the engine gets.  The two choices are \"Alpha-N\" which uses RPM and the throttle position as the main variables (engine temp,air temp and O2 are also utilized as well).  The other more commonly used mode is \"Speed Density\" which uses the MAP sensor as the primary input to the ECU (all off the other inputs are used as well, but the MAP sensor is essential for this mode to function).  Speed Density is the more commonly used of the two systems.  Alpha-N is only typically used when the vacuum signal from the engine is extremeley poor. (as in some race engines).",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_container_add(GTK_CONTAINER(ebox),table);
	
	label = gtk_label_new("Fuel Injection Control Strategy");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Speed Density");
	constants.speed_den_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(4));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	button = gtk_radio_button_new_with_label(group,"Alpha-N");
	constants.alpha_n_but = button;
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(4));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Injection Type selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox3),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The two modes for injecting fuel are via a \"Throttle Body\" which looks very similar to a carburetor and can bolt up in place of a former carb on many engines, and \"Multi-Point\" which just has the injectors mounted on the intake manifold (usually pointing at the intake valves).  If you have your injectors mounted to a fuel rail and pointing at the backsides of your intake valves, you have Multi-Point,  if you have your injector(s) in what looks like a simpler carburetor then you have a TBI setup",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("Injection Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Multi-Port");
	constants.multi_port_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(8));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	button = gtk_radio_button_new_with_label(group,"Throttle-Body");
	constants.tbi_but = button;
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(8));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Engine stroke selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox3),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   Pretty simple here, is your engine 2 stroke or 4 stroke. (4 stroke is the correct answer for 99% of people out there unless you're MS'ing an old motorcycle, or an outboard boat motor)",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_container_add(GTK_CONTAINER(ebox),table);
	
	label = gtk_label_new("Engine Stroke");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Four-Stroke");
	constants.four_stroke_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(4));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	button = gtk_radio_button_new_with_label(group,"Two-Stroke");
	constants.two_stroke_but = button;
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(4));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Engine Firing Type selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox3),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   Most engines are Even-Fire, this means that the firing order is evenly spaced in crank degrees.  For example most V8's fires a cylinder every 90 degrees of crankshaft rotation.  Some engines, like GM's oddfire V6 and some european engines fire at odd measures of crankshaft rotation (not every 60 or 90 degrees), as do Harley Davidson V-twin motorcycle engines.",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("Engine Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Even Fire");
	constants.even_fire_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	button = gtk_radio_button_new_with_label(group,"Odd Fire");
	constants.odd_fire_but = button;
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* MAP Sensor Type selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox3),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The version 1 MS kits were available with either a 1 Bar map sensor (NA) or a 2.5 Bar Sensor (Turbo).  The Version 2's and later were only available with the 2.5 Bar sensor. Select the appropriate sensor that you have",NULL); 
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("MAP Sensor Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"115 kPa");
	constants.map_115_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	button = gtk_radio_button_new_with_label(group,"250 kPa");
	constants.map_250_but = button;
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Baro Correction Selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox3),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   Do you want to use barometer correction?  If this is enabled, on power up of the MS, it will read the MAP sensor (before the engine hopefully is cranked over) and store this value as the reference barometer and use this to compensate for altitude changes,  Beware that if your MegaSquirt ECU resets when the engine is running due to power problems, this will cause it to record an erroneous value and skew the fuel calculations.",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("Barometer Correction");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Enabled");
	constants.baro_ena_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(8));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	button = gtk_radio_button_new_with_label(group,"Disabled");
	constants.baro_disa_but = button;
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(8));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Injector Staging */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox3),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The MS has two injector drivers which can drive multiple injectors each,  Do you want to fire both banks at once (Simultaneous), or do you want them to alternate?  Some engines perform better one way over another.  This also works hand in hand with the number of squirts settable below.  This requires a little trial and error,  beware that if you increase the number of squirts below, each squirt is shorter and if you get too small it may become difficult to tune easily..",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("Injector Staging");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Simultaneous");
	constants.simul_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(DEFERRED));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	button = gtk_radio_button_new_with_label(group,"Alternate");
	constants.alternate_but = button;
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(DEFERRED));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	/* Injection Control cyls/injectors, etc.. */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   You can select the number of times to inject fuel per cycle.  A cycle refers to one complete engine cycle (2 revs of a crank on a 4 stroke piston engine). The number of injectors and the number of cylinders.  These choices determine how long each injector pulse is. (don't get too low), and how often they are fired. You will probably find that more or less squirts (depending on the engine and injection type (Multi-point or TBI) can significantly affect your idle quality.  The MS FAQ describes this better and offers suggestions as to the best optiosn to choose.",NULL);
	frame = gtk_frame_new("Cylinder/Injection Configuration");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* Indirectly generates the "divider" variable */
	adj = (GtkAdjustment *) gtk_adjustment_new(0.0,1.0,12,1.0,1.0,0.0);
	spinner = gtk_spin_button_new(adj,1,0);
        constants.inj_per_cycle_spin = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(91));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(DEFERRED));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(NUM_SQUIRTS));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("# of Injections per Cycle");
	labels.squirts_lab = label;
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Number of injectors, part of config12 */
	adj = (GtkAdjustment *) gtk_adjustment_new(0.0,1.0,12,1.0,1.0,0.0);
	spinner = gtk_spin_button_new(adj,1,0);
        constants.injectors_spin = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(117));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(DEFERRED));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(NUM_INJECTORS));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("# of Fuel Injectors");
	labels.injectors_lab = label;
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Number of Cylinders part of config11 */
	adj = (GtkAdjustment *) gtk_adjustment_new(0.0,1.0,12,1.0,1.0,0.0);
	spinner = gtk_spin_button_new(adj,1,0);
        constants.cylinders_spin = spinner;
        constants.cylinders_adj = adj;
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(116));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(DEFERRED));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(NUM_CYLINDERS));
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("# of Cylinders");
	labels.cylinders_lab = label;
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Commands frame */
	frame = gtk_frame_new("Commands");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	
	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),50);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Get Data from ECU");
	gtk_tooltips_set_tip(tip,button,
	"Reads in the Constants and VEtable from the MegaSquirt ECU and populates the GUI",NULL);
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(READ_FROM_MS));
	
	button = gtk_button_new_with_label("Permanently Store Data in ECU");
	buttons.const_store_but = button;
	gtk_tooltips_set_tip(tip,button,
        "Even though MegaTunix writes data to the MS as soon as its changed, it has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(WRITE_TO_MS));
	return TRUE;
}
