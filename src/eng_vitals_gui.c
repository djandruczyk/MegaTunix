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
extern GtkWidget *ve_widgets[];
extern GdkColor black;
GList *enh_idle_widgets = NULL;
GList *iac_idle_widgets = NULL;
GList *inv_ign_widgets = NULL;
GList *ign_widgets = NULL;

void build_eng_vitals(GtkWidget *parent_frame)
{
	GtkWidget *sep;
	GtkWidget *button;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *ebox;
	GtkWidget *spinner;
	GtkWidget *tmpspin;
	GtkAdjustment *adj;
	GSList	*group;
	extern GList *store_widgets;
	extern GtkTooltips *tip;
	extern GList *dt_widgets;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	frame = gtk_frame_new("Injection Control");
	gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox),1);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	/* Injection Control Section */
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);

	/* Fuel Injection Control Strategy */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   This selects the mode by which the MegaSquirt ECU calculates the fuel the engine gets.  The two choices are \"Alpha-N\" which uses RPM and the throttle position as the main variables (engine temp,air temp and O2 are also utilized as well).  The other more commonly used mode is \"Speed Density\" which uses the MAP sensor as the primary input to the ECU (all off the other inputs are used as well, but the MAP sensor is essential for this mode to function).  Speed Density is the more commonly used of the two systems.  Alpha-N is only typically used when the vacuum signal from the engine is extremeley poor. (as in some race engines).",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),3);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("Fuel Injection Control Strategy");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Speed Density");
	buttons.speed_den_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
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
	buttons.alpha_n_but = button;
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
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
	gtk_box_pack_start(GTK_BOX(vbox2),sep,TRUE,TRUE,0);

	/* Injection Type selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The two modes for injecting fuel are via a \"Throttle Body\" which looks very similar to a carburetor and can bolt up in place of a former carb on many engines, and \"Multi-Point\" which just has the injectors mounted on the intake manifold (usually pointing at the intake valves).  If you have your injectors mounted to a fuel rail and pointing at the backsides of your intake valves, you have Multi-Point,  if you have your injector(s) in what looks like a simpler carburetor then you have a TBI setup",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),3);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("Injection Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Multi-Port");
	buttons.multi_port_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(116));
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
	buttons.tbi_but = button;
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(116));
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
	gtk_box_pack_start(GTK_BOX(vbox2),sep,TRUE,TRUE,0);

	/* Engine stroke selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   Pretty simple here, is your engine 2 stroke or 4 stroke. (4 stroke is the correct answer for 99% of people out there unless you're MS'ing an old motorcycle, or an outboard boat motor)",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),3);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("Engine Stroke");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Four-Stroke");
	buttons.four_stroke_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(116));
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
	buttons.two_stroke_but = button;
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(116));
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
	gtk_box_pack_start(GTK_BOX(vbox2),sep,TRUE,TRUE,0);

	/* Engine Firing Type selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   Most engines are Even-Fire, this means that the firing order is evenly spaced in crank degrees.  For example most V8's fires a cylinder every 90 degrees of crankshaft rotation.  Some engines, like GM's oddfire V6 and some european engines fire at odd measures of crankshaft rotation (not every 60 or 90 degrees), as do Harley Davidson V-twin motorcycle engines.",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),3);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("Engine Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Even Fire");
	buttons.even_fire_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
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
	buttons.odd_fire_but = button;
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
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

	/* Vertical Seperator between halves of the screen */
	sep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox),sep,FALSE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);

	/* MAP Sensor Type selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The version 1 MS kits were available with either a 1 Bar map sensor (NA) or a 2.5 Bar Sensor (Turbo).  The Version 2's and later were only available with the 2.5 Bar sensor. Select the appropriate sensor that you have",NULL); 
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),3);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("MAP Sensor Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"115 kPa");
	buttons.map_115_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(116));
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
	buttons.map_250_but = button;
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(116));
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
	gtk_box_pack_start(GTK_BOX(vbox2),sep,TRUE,TRUE,0);

	/* O2 Sensor Type selector */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   Do you have a Narrow-Band (1, 3 or 4 wire) O2 Sensor or a Wideband sensor (5 Wire sensor + control module)?  You cannot just install a Wideband sensor and connect it DIRECTLY to the MegaSquirt ECU.  The Wideband sensors require a dedicated controller which will output an analog signal that connects to the MegaSquirt ECU.  A Narrowband sensor (1,3 or 4 wires) can be connected directly to the ECU (use the signal wire only).  If the sensor has only 1 wire, this wire goes to the ECU O2 input,  if it has 3 wires,  two of them are for the sensor heater element,  1 wire goes to ground, the other goes to switched 12 Volts.  Typically the heater wires are of a heavier gauge than the signal wire.  If your O2 sensor has 4 wires, 2 are for the heater, connect as above, the other two are signal, and signal ground. When hot the sensor will output a voltage between about 0-1 Volts, depending on the mixture (richer is higher for a narrowband sensor), if you connect a meter across the signal wires and get a negative reading you hooked it up backwards, (reverse the signal and ground wires)",NULL);
        table = gtk_table_new(2,2,TRUE);
        gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_container_set_border_width(GTK_CONTAINER(table),3);
	gtk_container_add(GTK_CONTAINER(ebox),table);

        label = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label),"O<sub>2</sub> Sensor Type");
        gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (GTK_EXPAND), 0, 0);

        button = gtk_radio_button_new_with_label(NULL,"Narrow-Band");
        buttons.nbo2_but = button;
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(2));
        g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);

        button = gtk_radio_button_new_with_label(group,"Wide-Band");
        buttons.wbo2_but = button;
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(2));
        g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                      G_CALLBACK(bitmask_button_handler),
                        NULL);

        sep = gtk_hseparator_new();
        gtk_box_pack_start(GTK_BOX(vbox2),sep,TRUE,TRUE,0);


	/* Baro Correction Selectors */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   Do you want to use barometer correction?  If this is enabled, on power up of the MS, it will read the MAP sensor (before the engine hopefully is cranked over) and store this value as the reference barometer and use this to compensate for altitude changes,  Beware that if your MegaSquirt ECU resets when the engine is running due to power problems, this will cause it to record an erroneous value and skew the fuel calculations.",NULL);
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),3);
	gtk_container_add(GTK_CONTAINER(ebox),table);

	label = gtk_label_new("Barometer Correction");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (GTK_EXPAND), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Enabled");
	buttons.baro_ena_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
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
	buttons.baro_disa_but = button;
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
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

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,FALSE,FALSE,0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);

	frame = gtk_frame_new("Idle Control Method");
	inv_ign_widgets = g_list_append(inv_ign_widgets,(gpointer)frame);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),15);
	gtk_container_set_border_width(GTK_CONTAINER(table),15);
	gtk_container_add(GTK_CONTAINER(frame),table);

	label = gtk_label_new("Idle Control Methodology");
        gtk_table_attach (GTK_TABLE (table), label, 0, 3, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"B&G On-Off");
	buttons.onoff_idle_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(4));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(16));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);


	button = gtk_radio_button_new_with_label(group,"PWM Controlled");
	buttons.pwm_idle_but = button;
	iac_idle_widgets = g_list_append(iac_idle_widgets, (gpointer)button);
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(118));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(4));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(16));
	g_object_set_data(G_OBJECT(button),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(bitmask_button_handler),
			NULL);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);

	frame = gtk_frame_new("Radiator Fan Control");
	ign_widgets = g_list_append(ign_widgets,(gpointer)frame);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),3);
	gtk_container_add(GTK_CONTAINER(frame),table);

	label = gtk_label_new("Cooling Fan Turn-On Temp (\302\260 F.)");
        labels.cooling_fan_temp_lab = label;
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

        /* Cooling Fan Turn-On Temp */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,-40.0,215.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        ve_widgets[121] = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);


	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(hbox),ebox,TRUE,TRUE,0);

	frame = gtk_frame_new("Idle Control Parameters");
	inv_ign_widgets = g_list_append(inv_ign_widgets,(gpointer)frame);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
	gtk_container_add(GTK_CONTAINER(ebox),frame);


	gtk_tooltips_set_tip(tip,ebox,"   The basic Bowling/Grippo MegaSquirt units, (version 1.1 or versiosn 2.2 hardware, and software versions up through version 3.01 all use a very simple idle control,  it's either on high-idle or it's not, and it's temperature controlled, once the engine temp gets above the Fast Idle temp, the high idle solenoid is disengaged and the engine goes down to its normal idle.  The DualTable code variants added in a PWM idle control (for driving a ford style 2 wire PWM actuated idle air valve (or similar).  This code supports 5 variables, the high idle rpm, high idle temp, the low idle rpm, and low idle temp and an idle threshold.  The way it works is simple; At temps below the Fast Idle Temp, the engien is kept at the Fast idle speed,  as the engine tep rises, the ECU drops the idle speed until the engine reaches the Slow idle temp, above this temp the engine temp is maintained at the slow idle speed.  Sensible numbers would be to set the high idle temp to be a low temperature, like 60 deg F, and a high idle RPM of 1800 RPM, and a slow idle RPM of 900 RPM, and slow idle temp of 145 deg F, and an idle threshold of 100RPM.  The Idle Threshold is the point at which when the throttle (TPS) input is BELOW this value idle control is enabled. Thus if when your throttle is closed and your TPS gauge reads 10%% in the runtime screen, setting this to 12 would enable idle control when you are idling, if you set it too low, idle control will never turn on and the engine may idle too low or not at all. ",NULL);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	table = gtk_table_new(5,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),25);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,20);

	/* This label is dynamic and will change based on unit preference
	 * AND based on if we are using DT code or not... 
	 */
	label = gtk_label_new("Fast Idle Temp (\302\260 F.)");
        labels.fast_idle_temp_lab = label;
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
	/* SPECIAL CASE!!!!
	 * If you notice no data is bound to this object NOR any signal 
	 * handler.  This is NOT A BUG, it is because this spinbutton is 
	 * bound to the adjustment of ANOTHER control (Cooling Fan Temp)
	 * thus any change in this spinbutton will cause the other control
	 * to emit the necessary signals and feed the changes to the ECU
	 * this had to be done as the MegaSquirtnEDIS/Spark codes do NOT have
	 * fast idle control re-used this memory position for a different 
	 * feature.  We use this method and a selective method to turn 
	 * on/off the appropriate controls based on the detected firmware.
	 * IF this does NOT make sense to you feel free to email the 
	 * author and I'll explain to you...  :) */
        /* Fast Idle Temp (tied to "Cooling Fan Temp" (MSnEDIS/spark ) */
	tmpspin = ve_widgets[121];
	adj = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(tmpspin));
        adjustments.fast_idle_temp_adj = adj;
        spinner = gtk_spin_button_new(adj,0,0);
        spinners.fast_idle_temp_spin = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	/* Fast Idle Speed */
	label = gtk_label_new("Fast Idle Speed (RPM)");
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	iac_idle_widgets = g_list_append(iac_idle_widgets, (gpointer)label);
	enh_idle_widgets = g_list_append(enh_idle_widgets, (gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
        /* Fast Idle Speed */
        adj =  (GtkAdjustment *) gtk_adjustment_new(1800.0,0.0,2550.0,10.0,100.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        ve_widgets[125] = spinner;
	dt_widgets = g_list_append(dt_widgets, (gpointer)spinner);
	iac_idle_widgets = g_list_append(iac_idle_widgets, (gpointer)spinner);
	enh_idle_widgets = g_list_append(enh_idle_widgets, (gpointer)spinner);
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(125));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER(10*100));
        g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(DIV));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(GENERIC));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	// Slow Idle Temp
	label = gtk_label_new("Slow Idle Temp (\302\260 F.)");
        labels.slow_idle_temp_lab = label;
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	iac_idle_widgets = g_list_append(iac_idle_widgets, (gpointer)label);
	enh_idle_widgets = g_list_append(enh_idle_widgets, (gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
        /* Slow Idle Temp */
        adj =  (GtkAdjustment *) gtk_adjustment_new(145.0,-40.0,215.0,1.0,10.0,0);
        adjustments.slow_idle_temp_adj = adj;
        spinner = gtk_spin_button_new(adj,0,0);
	dt_widgets = g_list_append(dt_widgets, (gpointer)spinner);
	iac_idle_widgets = g_list_append(iac_idle_widgets, (gpointer)spinner);
	enh_idle_widgets = g_list_append(enh_idle_widgets, (gpointer)spinner);
        spinners.slow_idle_temp_spin = spinner;
        ve_widgets[124] = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"temp_dep",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(124));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER(40*100));
        g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(ADD));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(GENERIC));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);


	/* Slow Idle Speed */
	label = gtk_label_new("Slow Idle Speed (RPM)");
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	iac_idle_widgets = g_list_append(iac_idle_widgets, (gpointer)label);
	enh_idle_widgets = g_list_append(enh_idle_widgets, (gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
        /* Slow Idle Speed */
        adj =  (GtkAdjustment *) gtk_adjustment_new(900.0,0.0,2550.0,10.0,100.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
	dt_widgets = g_list_append(dt_widgets, (gpointer)spinner);
	iac_idle_widgets = g_list_append(iac_idle_widgets, (gpointer)spinner);
	enh_idle_widgets = g_list_append(enh_idle_widgets, (gpointer)spinner);
        ve_widgets[126] = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(126));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER(10*100));
        g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(DIV));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(GENERIC));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 3, 4,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	/* Idle Threshold  (compared to TPS, if TPS is below this
	 * use idle control
	 */
	label = gtk_label_new("Idle Threshold (TPS%)");
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	iac_idle_widgets = g_list_append(iac_idle_widgets, (gpointer)label);
	enh_idle_widgets = g_list_append(enh_idle_widgets, (gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
        /* Idle Threshold (% of TPS )*/
        adj =  (GtkAdjustment *) gtk_adjustment_new(10.0,0.0,100.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
	dt_widgets = g_list_append(dt_widgets, (gpointer)spinner);
	iac_idle_widgets = g_list_append(iac_idle_widgets, (gpointer)spinner);
	enh_idle_widgets = g_list_append(enh_idle_widgets, (gpointer)spinner);
        ve_widgets[127] = spinner;
        gtk_widget_set_size_request(spinner,60,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(127));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER(1*100));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(GENERIC));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 4, 5,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);



	/* Commands frame */
	frame = gtk_frame_new("Commands");
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),7);
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
        store_widgets = g_list_append(store_widgets,(gpointer)button);
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
