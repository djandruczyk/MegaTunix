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
#include <configfile.h>
#include <constants_gui.h>
#include <defines.h>
#include <enums.h>
#include <gui_handlers.h>
#include <structures.h>


extern struct DynamicSpinners spinners;
extern struct DynamicButtons buttons;
extern GtkWidget *ve_widgets[];
extern GdkColor black;
extern GdkColor white;
GList *inv_dt_controls = NULL;
GList *dt_controls = NULL;

void build_constants_1(GtkWidget *parent_frame)
{
	GtkWidget *button;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *pri_table;
	GtkWidget *table;
	GtkWidget *spinner;
	GtkWidget *ebox;
	GtkWidget *sep;
	GSList *group = NULL;
	GtkAdjustment *adj;
	struct Reqd_Fuel *reqd_fuel_1 = NULL;
	struct Reqd_Fuel *reqd_fuel_2 = NULL;

	extern GList *store_controls;
	extern GList *interdep_1_controls;
	extern GList *interdep_2_controls;
	extern GList *reqfuel_1_controls;
	extern GList *reqfuel_2_controls;
	extern GtkTooltips *tip;

	reqd_fuel_1 = g_malloc0(sizeof(struct Reqd_Fuel));
	initialize_reqd_fuel((void *)reqd_fuel_1, 1);

	reqd_fuel_2 = g_malloc0(sizeof(struct Reqd_Fuel));
	initialize_reqd_fuel((void *)reqd_fuel_2, 2); /* 2 for TABLE 2 */

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"    Here is where you punch in some data for your vehicle and the system will calculate the required fuel to be used in the MegaSquirt calculations.\n   \"Required Fuel\" is the total amount of fuel that needs to be injected at 100% VE (max power for a NA engine assuming no resonant effects) for ONE cylinder for 1 complete engine cycle.  You will see two required fuel numbers, one for \"per cyl/cycle\" and another for \"ms per squirt\". The top one is the amount of fuel injected in one complete engine cycle, the bottom one is the amount of fuel sprayed on each injection event.\n   NOTE: If the required fuel per squirt gets below 2.0 milliseconds, you will have difficulty tuning as you are approaching the open-time limit of the injector, and fuel flow becomes less precise, and there is much less adjustability. (you may not be able to get the mixture set approriately)\n   The \"Rated Fuel Pressure\" and the \"Actual Fuel Pressure\" are there so that if you are using injectors at a differnt pressure than their original application, the required fuel calculations will take that into account, and adjust accordingly.\n   The \"Target Air-Fuel Ratio\" calculates the required fuel for a specific fuel mixture. ( It defaults to 14.7:1 for gasoline) if you want to target a richer mixture set this to the desired AFR (lower = richer)\n   The \"Number of Injectors\" is your total number of fuel injectors,  this affects required fuel as it ties in with the number of cylinders and number of squirts.\n   The \"Number of Squirts per Cycle\" determines how many times the injectors are fired during one engine cycle,  if you set this too high, the number of squirts gets too low making the injection pulsewidth too short causing inconsistent fuel flow.\n   The \"Simultaneous\"/\"Alternate\" buttons determine how the injectors fire.  (This applies to B&G code only Dualtable code can set the number of squirts/cyls/req_fue sperately per table).  If \"Simultaneous\" mode is chosen, both banks of fuel injectors fire at the same time,  if this is set to \"Alternate\", then one one bank fires at a time, and the channels are alternated back and forth. (NOTE this delivers the same amount of fuel,  but in alternate mode there are 1/2 as many squirts, (thus longer squirts)). Some engines respond better one way or the other,  Testing is recommended to determine which works best for you.",NULL);

	frame = gtk_frame_new("Table 1 Cylinders, Injectors and Squirts Configuration");
	gtk_container_add(GTK_CONTAINER(ebox),frame);


	pri_table = gtk_table_new(3,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(pri_table),10);
	gtk_table_set_row_spacings(GTK_TABLE(pri_table),10);
	gtk_container_set_border_width(GTK_CONTAINER(pri_table),5);
	gtk_container_add(GTK_CONTAINER(frame),pri_table);

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_table_set_row_spacings(GTK_TABLE(table),1);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
        gtk_table_attach (GTK_TABLE (pri_table), table, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
	
        label = gtk_label_new("# Cylinders");
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)label);
	reqfuel_1_controls = g_list_append(reqfuel_1_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);


	// Number of Cylinders part of config11 
	adj = (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,12,1.0,1.0,0.0);
	spinner = gtk_spin_button_new(adj,1,0);
	spinners.cylinders_1_spin = spinner;
	reqfuel_1_controls = g_list_append(reqfuel_1_controls,(gpointer)spinner);
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)spinner);
	g_object_set_data(G_OBJECT(spinner),"data",(gpointer)reqd_fuel_1);
	gtk_widget_set_size_request(spinner,45,-1);
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
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(NUM_CYLINDERS_1));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        label = gtk_label_new("# Injectors");
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Number of injectors, part of config12 
	adj = (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,12,1.0,1.0,0.0);
	spinner = gtk_spin_button_new(adj,1,0);
	spinners.injectors_1_spin = spinner;
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)spinner);
	g_object_set_data(G_OBJECT(spinner),"data",(gpointer)reqd_fuel_1);
	gtk_widget_set_size_request(spinner,45,-1);
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
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(NUM_INJECTORS_1));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        label = gtk_label_new("Squirts/Cycle");
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)label);
	reqfuel_1_controls = g_list_append(reqfuel_1_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Indirectly generates the "divider" variable 
	adj = (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,12,1.0,1.0,0.0);
	spinner = gtk_spin_button_new(adj,1,0);
	spinners.inj_per_cycle_1_spin = spinner;
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)spinner);
	reqfuel_1_controls = g_list_append(reqfuel_1_controls,(gpointer)spinner);
	g_object_set_data(G_OBJECT(spinner),"data",(gpointer)reqd_fuel_1);
	gtk_widget_set_size_request(spinner,45,-1);
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
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(NUM_SQUIRTS_1));
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	sep = gtk_hseparator_new();
        gtk_table_attach (GTK_TABLE (pri_table), sep, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	table = gtk_table_new(2,2,TRUE);
	inv_dt_controls = g_list_append(inv_dt_controls,(gpointer)table);
        gtk_table_set_row_spacings(GTK_TABLE(table),0);
        gtk_table_set_col_spacings(GTK_TABLE(table),20);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
        gtk_table_attach (GTK_TABLE (pri_table), table, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        label = gtk_label_new("Injector Staging");
        gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
	inv_dt_controls = g_list_append(inv_dt_controls,(gpointer)label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        button = gtk_radio_button_new_with_label(NULL,"Simultaneous");
        buttons.simul_but = button;
	inv_dt_controls = g_list_append(inv_dt_controls,(gpointer)button);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
        g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(DEFERRED));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);

        button = gtk_radio_button_new_with_label(group,"Alternate");
        buttons.alternate_but = button;
	inv_dt_controls = g_list_append(inv_dt_controls,(gpointer)button);
        g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(DEFERRED));
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);

	sep = gtk_vseparator_new();
        gtk_table_attach (GTK_TABLE (pri_table), sep, 1, 2, 0, 3,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_FILL), 0, 0);

	table = gtk_table_new(3,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_table_set_row_spacings(GTK_TABLE(table),10);
	gtk_table_set_row_spacing(GTK_TABLE(table),1,5);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
        gtk_table_attach (GTK_TABLE (pri_table), table, 2, 3, 0, 3,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
	
	button = gtk_button_new_with_label("Calculate Required Fuel");
	g_object_set_data(G_OBJECT(button),"reqd_fuel",(gpointer)reqd_fuel_1);
        gtk_table_attach (GTK_TABLE (table), button, 0, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
        g_signal_connect(G_OBJECT(button),"clicked",
                        G_CALLBACK(std_button_handler),
                        GINT_TO_POINTER(REQD_FUEL_POPUP));

	label = gtk_label_new("Required Fuel (per cyl/cycle)");
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Required Fuel Total/cycle Value
	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
	spinners.req_fuel_total_1_spin = spinner;
	reqd_fuel_1->reqd_fuel_spin = spinner;
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)spinner);
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(90));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(DEFERRED));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(REQ_FUEL_1));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	label = gtk_label_new("Required Fuel (ms. per squirt)");
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Required Fuel Per Squirt Value
	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
	spinners.req_fuel_per_squirt_1_spin = spinner;
	interdep_1_controls = g_list_append(interdep_1_controls,(gpointer)spinner);
	gtk_widget_set_sensitive(spinner,FALSE);
	gtk_widget_modify_text(spinner,GTK_STATE_INSENSITIVE,&black);
	gtk_widget_modify_base(spinner,GTK_STATE_INSENSITIVE,&white);
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The \"Injector Open Time\" is the amount of time that it takes your fuel injectors to open.  This is added to the MegaSquirt fuel calculations internally.  Typical values are are 1.1 ms for Hi-Z injectors and 0.9 ms for Lo-Z injectors.\n   The \"Battery Correction\" is a correction factor that increases injection pulsewidth based on lower battery voltage (which slows down injector opening). The values are in milliseconds per Volt. Thus if your battery is putting out 9 Volts during cranking, and you have this set to 0.2 ms/Volt, then your pulsewidth will be increased by 0.6 milliseconds.\n   The \"PWM Current Limit\" is used if you are using Low impedance (Lo-Z) injectors.  For Hi-Z (i.e. \"Saturated\") injectors, set this to 100% (which disables PWM as it is NOT used with Hi-Z injectors).  Lo-Z injectors are also known as Peak and Hold injectors.  P&H injectors require full battery voltage to snap them open,  but then power is reduced to a \"Hold\" value to keep it open (without burning it out or letting them close prematurely).  This value of \"PWM Current Limit\" determines the \"Hold\" value.  Unmodified V2 ECU's usually need this around 60-75%, ECU's with the flyback board and the the v2.986 or NEWER firmware usually have this in the 18-30% range.  If you are driving large Lo-Z injectors, (like a Holley Pro-jection TBI setup), installing a flyback board and new firmware is recommended as those injectors can damage a Stock V2 ECU in some rare scenarios.\n   The \"PWM Time Threshold\" is the amount of time that the injector is in \"Peak\" mode. (full power), This should be set to about 1 ms for Lo-Z injectors, or 25.5 for Hi-Z injectors. Setting this value too high for Lo-Z injectors can cause them to overheat and fail (freeze open, or burn out), 1-1.5 ms is a pretty safe value to use.",NULL);

	frame = gtk_frame_new("Table 1 Injector Control Parameters");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	pri_table = gtk_table_new(1,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(pri_table),10);
	gtk_table_set_row_spacings(GTK_TABLE(pri_table),10);
	gtk_container_set_border_width(GTK_CONTAINER(pri_table),5);
	gtk_container_add(GTK_CONTAINER(frame),pri_table);
	
	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_table_attach (GTK_TABLE (pri_table), table, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
	
	label = gtk_label_new("Injector Open Time (ms)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Injector Open Time 
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	ve_widgets[93] = spinner;
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(93));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Battery Correction Factor 
	label = gtk_label_new("Batt. Correction (ms/V)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Battery Correction Factor 
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,10.0,0.1,1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	ve_widgets[97] = spinner;
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(97));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(60*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	sep = gtk_vseparator_new();
        gtk_table_attach (GTK_TABLE (pri_table), sep, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_FILL), 0, 0);
	
	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_table_attach (GTK_TABLE (pri_table), table, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	label = gtk_label_new("PWM Current Limit (%)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// PWM Current Limit % 
	adj =  (GtkAdjustment *) gtk_adjustment_new(50.0,0.0,100.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	ve_widgets[95] = spinner;
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(95));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	label = gtk_label_new("PWM Time Threshold (ms)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// PWM Time threshold 
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,0,1);
	ve_widgets[96] = spinner;
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(96));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Table 2 Constants.... 

	ebox = gtk_event_box_new();
        dt_controls = g_list_append(dt_controls,(gpointer)ebox);
	gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"    Here is where you punch in some data for your vehicle and the system will calculate the required fuel to be used in the MegaSquirt calculations.\n   \"Required Fuel\" is the total amount of fuel that needs to be injected at 100% VE (max power for a NA engine assuming no resonant effects) for ONE cylinder for 1 complete engine cycle.  You will see two required fuel numbers, one for \"per cyl/cycle\" and another for \"ms per squirt\". The top one is the amount of fuel injected in one complete engine cycle, the bottom one is the amount of fuel sprayed on each injection event.\n   NOTE: If the required fuel per squirt gets below 2.0 milliseconds, you will have difficulty tuning as you are approaching the open-time limit of the injector, and fuel flow becomes less precise, and there is much less adjustability. (you may not be able to get the mixture set approriately)\n   The \"Rated Fuel Pressure\" and the \"Actual Fuel Pressure\" are there so that if you are using injectors at a differnt pressure than their original application, the required fuel calculations will take that into account, and adjust accordingly.\n   The \"Target Air-Fuel Ratio\" calculates the required fuel for a specific fuel mixture. ( It defaults to 14.7:1 for gasoline) if you want to target a richer mixture set this to the desired AFR (lower = richer)\n   The \"Number of Injectors\" is your total number of fuel injectors,  this affects required fuel as it ties in with the number of cylinders and number of squirts.\n   The \"Number of Squirts per Cycle\" determines how many times the injectors are fired during one engine cycle,  if you set this too high, the number of squirts gets too low making the injection pulsewidth too short causing inconsistent fuel flow.\n   The \"Simultaneous\"/\"Alternate\" buttons determine how the injectors fire.  (This applies to B&G code only Dualtable code can set the number of squirts/cyls/req_fue sperately per table).  If \"Simultaneous\" mode is chosen, both banks of fuel injectors fire at the same time,  if this is set to \"Alternate\", then one one bank fires at a time, and the channels are alternated back and forth. (NOTE this delivers the same amount of fuel,  but in alternate mode there are 1/2 as many squirts, (thus longer squirts)). Some engines respond better one way or the other,  Testing is recommended to determine which works best for you.",NULL);

	frame = gtk_frame_new("Table 2 Cylinders, Injectors and Squirts Configuration");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	pri_table = gtk_table_new(3,3,FALSE);
        gtk_table_set_col_spacings(GTK_TABLE(pri_table),10);
        gtk_table_set_row_spacings(GTK_TABLE(pri_table),10);
        gtk_container_set_border_width(GTK_CONTAINER(pri_table),5);
        gtk_container_add(GTK_CONTAINER(frame),pri_table);

        table = gtk_table_new(2,3,FALSE);
        gtk_table_set_col_spacings(GTK_TABLE(table),10);
        gtk_table_set_row_spacings(GTK_TABLE(table),1);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
        gtk_table_attach (GTK_TABLE (pri_table), table, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
        
        label = gtk_label_new("# Cylinders");
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)label);
        reqfuel_2_controls = g_list_append(reqfuel_2_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);


        // Number of Cylinders part of config11 
        adj = (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,12,1.0,1.0,0.0);
        spinner = gtk_spin_button_new(adj,1,0);
        g_object_set_data(G_OBJECT(spinner),"data",(gpointer)reqd_fuel_2);
        spinners.cylinders_2_spin = spinner;
        reqfuel_2_controls = g_list_append(reqfuel_2_controls,(gpointer)spinner);
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)spinner);
        g_object_set_data(G_OBJECT(spinner),"data",(gpointer)reqd_fuel_2);
        gtk_widget_set_size_request(spinner,45,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
                        GINT_TO_POINTER(116+MS_PAGE_SIZE));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER(1*100));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
                        GINT_TO_POINTER(NOTHING));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(DEFERRED));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinbutton_handler),
                        GINT_TO_POINTER(NUM_CYLINDERS_2));
        gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        label = gtk_label_new("# Injectors");
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        // Number of injectors, part of config12 
        adj = (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,12,1.0,1.0,0.0);
        spinner = gtk_spin_button_new(adj,1,0);
        spinners.injectors_2_spin = spinner;
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)spinner);
        g_object_set_data(G_OBJECT(spinner),"data",(gpointer)reqd_fuel_2);
        gtk_widget_set_size_request(spinner,45,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
                        GINT_TO_POINTER(117+MS_PAGE_SIZE));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER(1*100));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
                        GINT_TO_POINTER(NOTHING));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(DEFERRED));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinbutton_handler),
                        GINT_TO_POINTER(NUM_INJECTORS_2));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        label = gtk_label_new("Squirts/Cycle");
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)label);
        reqfuel_2_controls = g_list_append(reqfuel_2_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        // Indirectly generates the "divider" variable 
        adj = (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,12,1.0,1.0,0.0);
        spinner = gtk_spin_button_new(adj,1,0);
        spinners.inj_per_cycle_2_spin = spinner;
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)spinner);
        reqfuel_2_controls = g_list_append(reqfuel_2_controls,(gpointer)spinner);
        g_object_set_data(G_OBJECT(spinner),"data",(gpointer)reqd_fuel_2);
        gtk_widget_set_size_request(spinner,45,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
                        GINT_TO_POINTER(91+MS_PAGE_SIZE));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER(1*100));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
                        GINT_TO_POINTER(NOTHING));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(DEFERRED));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinbutton_handler),
                        GINT_TO_POINTER(NUM_SQUIRTS_2));
        gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        sep = gtk_hseparator_new();
        gtk_table_attach (GTK_TABLE (pri_table), sep, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        table = gtk_table_new(2,2,TRUE);
        inv_dt_controls = g_list_append(inv_dt_controls,(gpointer)table);
        gtk_table_set_row_spacings(GTK_TABLE(table),0);
        gtk_table_set_col_spacings(GTK_TABLE(table),20);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
        gtk_table_attach (GTK_TABLE (pri_table), table, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        label = gtk_label_new("Injector Staging");
        gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
        inv_dt_controls = g_list_append(inv_dt_controls,(gpointer)label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        button = gtk_radio_button_new_with_label(NULL,"Simultaneous");
        inv_dt_controls = g_list_append(inv_dt_controls,(gpointer)button);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        button = gtk_radio_button_new_with_label(group,"Alternate");
        inv_dt_controls = g_list_append(inv_dt_controls,(gpointer)button);
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        sep = gtk_vseparator_new();
        gtk_table_attach (GTK_TABLE (pri_table), sep, 1, 2, 0, 3,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_FILL), 0, 0);

        table = gtk_table_new(3,2,FALSE);
        gtk_table_set_col_spacings(GTK_TABLE(table),10);
        gtk_table_set_row_spacings(GTK_TABLE(table),10);
	gtk_table_set_row_spacing(GTK_TABLE(table),1,5);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
        gtk_table_attach (GTK_TABLE (pri_table), table, 2, 3, 0, 3,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        button = gtk_button_new_with_label("Calculate Required Fuel");
        g_object_set_data(G_OBJECT(button),"reqd_fuel",(gpointer)reqd_fuel_2);
        gtk_table_attach (GTK_TABLE (table), button, 0, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
        g_signal_connect(G_OBJECT(button),"clicked",
                        G_CALLBACK(std_button_handler),
                        GINT_TO_POINTER(REQD_FUEL_POPUP));

        label = gtk_label_new("Required Fuel (per cyl/cycle)");
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        // Required Fuel Total/cycle Value
        adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
        spinner = gtk_spin_button_new(adj,1.0,1);
        spinners.req_fuel_total_2_spin = spinner;
        reqd_fuel_2->reqd_fuel_spin = spinner;
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)spinner);
        gtk_widget_set_size_request(spinner,50,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(90+MS_PAGE_SIZE));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER(1*100));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
                        GINT_TO_POINTER(NOTHING));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(DEFERRED));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinbutton_handler),
                        GINT_TO_POINTER(REQ_FUEL_2));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        label = gtk_label_new("Required Fuel (ms. per squirt)");
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)label);
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

        // Required Fuel Per Squirt Value
        adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
        spinner = gtk_spin_button_new(adj,1.0,1);
        spinners.req_fuel_per_squirt_2_spin = spinner;
        interdep_2_controls = g_list_append(interdep_2_controls,(gpointer)spinner);
        gtk_widget_set_sensitive(spinner,FALSE);
        gtk_widget_modify_text(spinner,GTK_STATE_INSENSITIVE,&black);
	gtk_widget_modify_base(spinner,GTK_STATE_INSENSITIVE,&white);
        gtk_widget_set_size_request(spinner,50,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);


	ebox = gtk_event_box_new();
        dt_controls = g_list_append(dt_controls,(gpointer)ebox);
	gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The \"Injector Open Time\" is the amount of time that it takes your fuel injectors to open.  This is added to the MegaSquirt fuel calculations internally.  Typical values are are 1.1 ms for Hi-Z injectors and 0.9 ms for Lo-Z injectors.\n   The \"Battery Correction\" is a correction factor that increases injection pulsewidth based on lower battery voltage (which slows down injector opening). The values are in milliseconds per Volt. Thus if your battery is putting out 9 Volts during cranking, and you have this set to 0.2 ms/Volt, then your pulsewidth will be increased by 0.6 milliseconds.\n   The \"PWM Current Limit\" is used if you are using Low impedance (Lo-Z) injectors.  For Hi-Z (i.e. \"Saturated\") injectors, set this to 100% (which disables PWM as it is NOT used with Hi-Z injectors).  Lo-Z injectors are also known as Peak and Hold injectors.  P&H injectors require full battery voltage to snap them open,  but then power is reduced to a \"Hold\" value to keep it open (without burning it out or letting them close prematurely).  This value of \"PWM Current Limit\" determines the \"Hold\" value.  Unmodified V2 ECU's usually need this around 60-75%, ECU's with the flyback board and the the v2.986 or NEWER firmware usually have this in the 18-30% range.  If you are driving large Lo-Z injectors, (like a Holley Pro-jection TBI setup), installing a flyback board and new firmware is recommended as those injectors can damage a Stock V2 ECU in some rare scenarios.\n   The \"PWM Time Threshold\" is the amount of time that the injector is in \"Peak\" mode. (full power), This should be set to about 1 ms for Lo-Z injectors, or 25.5 for Hi-Z injectors. Setting this value too high for Lo-Z injectors can cause them to overheat and fail (freeze open, or burn out), 1-1.5 ms is a pretty safe value to use.",NULL);

	frame = gtk_frame_new("Table 2 Injector Control Parameters");
	gtk_container_add(GTK_CONTAINER(ebox),frame);


	pri_table = gtk_table_new(1,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(pri_table),10);
	gtk_table_set_row_spacings(GTK_TABLE(pri_table),10);
	gtk_container_set_border_width(GTK_CONTAINER(pri_table),5);
	gtk_container_add(GTK_CONTAINER(frame),pri_table);
	
	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_table_attach (GTK_TABLE (pri_table), table, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
	
	label = gtk_label_new("Injector Open Time (ms)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Injector Open Time 
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	ve_widgets[93+MS_PAGE_SIZE] = spinner;
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(93+MS_PAGE_SIZE));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Battery Correction Factor 
	label = gtk_label_new("Batt. Correction (ms/V)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// Battery Correction Factor 
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,10.0,0.1,1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	ve_widgets[97+MS_PAGE_SIZE] = spinner;
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(97+MS_PAGE_SIZE));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(60*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	sep = gtk_vseparator_new();
        gtk_table_attach (GTK_TABLE (pri_table), sep, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_FILL), 0, 0);
	
	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_table_attach (GTK_TABLE (pri_table), table, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	label = gtk_label_new("PWM Current Limit (%)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// PWM Current Limit % 
	adj =  (GtkAdjustment *) gtk_adjustment_new(50.0,0.0,100.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	ve_widgets[95+MS_PAGE_SIZE] = spinner;
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(95+MS_PAGE_SIZE));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	label = gtk_label_new("PWM Time Threshold (ms)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	// PWM Time threshold 
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,0,1);
	ve_widgets[96+MS_PAGE_SIZE] = spinner;
	gtk_widget_set_size_request(spinner,50,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(96+MS_PAGE_SIZE));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);


	frame = gtk_frame_new("Commands");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

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
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(READ_VE_CONST));

	button = gtk_button_new_with_label("Permanently Store Data in ECU");
        store_controls = g_list_append(store_controls,(gpointer)button);
	gtk_tooltips_set_tip(tip,button,
			"Even though MegaTunix writes data to the MS as soon as its changed, it has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(BURN_MS_FLASH));
	return;
}



void initialize_reqd_fuel(void * ptr, gint table)
{
	struct Reqd_Fuel *reqd_fuel = NULL;
	ConfigFile * cfgfile;
	gchar * filename;
	gchar * tmpbuf;

	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/config", NULL);

	reqd_fuel = (struct Reqd_Fuel *)ptr;
	reqd_fuel->table = table;
	tmpbuf = g_strdup_printf("Req_Fuel_Table_%i",table);
	cfgfile = cfg_open_file(filename);
	// Set defaults 
	reqd_fuel->visible = FALSE;
	reqd_fuel->disp = 350;
	reqd_fuel->cyls = 8;
	reqd_fuel->rated_inj_flow = 19.0;
	reqd_fuel->actual_inj_flow = 0.0;
	reqd_fuel->rated_pressure = 3.0;
	reqd_fuel->actual_pressure = 3.0;
	reqd_fuel->target_afr = 14.7;

	if (cfgfile)
	{
		cfg_read_int(cfgfile,tmpbuf,"Displacement",&reqd_fuel->disp);
		cfg_read_int(cfgfile,tmpbuf,"Cylinders",&reqd_fuel->cyls);
		cfg_read_float(cfgfile,tmpbuf,"Rated_Inj_Flow",
				&reqd_fuel->rated_inj_flow);
		cfg_read_float(cfgfile,tmpbuf,"Actual_Pressure",
				&reqd_fuel->actual_pressure);
		cfg_read_float(cfgfile,tmpbuf,"Rated_Pressure",
				&reqd_fuel->rated_pressure);
		cfg_read_float(cfgfile,tmpbuf,"Actual_Inj_Flow",
				&reqd_fuel->actual_inj_flow);
		cfg_read_float(cfgfile,tmpbuf,"Target_AFR",
				&reqd_fuel->target_afr);
		cfg_free(cfgfile);
		g_free(cfgfile);
	}
	g_free(tmpbuf);
	g_free(filename);

	return;
}
