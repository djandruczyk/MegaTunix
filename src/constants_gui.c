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
#include <constants_gui.h>
#include <defines.h>
#include <enums.h>
#include <gui_handlers.h>
#include <structures.h>


extern struct DynamicSpinners spinners;
struct DynamicAdjustments adjustments;
struct DynamicLabels labels;
struct DynamicButtons buttons;
extern struct Ve_Widgets *ve_widgets;
extern GdkColor black;

void build_constants_2(GtkWidget *parent_frame)
{}

void build_constants_1(GtkWidget *parent_frame)
{
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
	extern GList *store_buttons;
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
	spinners.req_fuel_total_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
	spinners.req_fuel_per_squirt_spin = spinner;
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
	ve_widgets->widget[93] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
	ve_widgets->widget[97] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
	ve_widgets->widget[95] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(95));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
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
	ve_widgets->widget[96] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
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

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
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
	spinners.inj_per_cycle_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(91));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
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
	spinners.injectors_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(117));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
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
	spinners.cylinders_spin = spinner;
	adjustments.cylinders_adj = adj;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(116));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
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
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table), 5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Get Data from ECU");
	gtk_tooltips_set_tip(tip,button,
			"Reads in the Constants and VEtable from the MegaSquirt ECU and populates the GUI",NULL);
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(READ_VE_CONST));

	button = gtk_button_new_with_label("Permanently Store Data in ECU");
        store_buttons = g_list_append(store_buttons,(gpointer)button);
	gtk_tooltips_set_tip(tip,button,
			"Even though MegaTunix writes data to the MS as soon as its changed, it has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(BURN_MS_FLASH));
	return;
}
