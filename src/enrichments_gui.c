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
#include <enrichments_gui.h>
#include <enums.h>
#include <gui_handlers.h>
#include <structures.h>

struct DynamicSpinners spinners;
extern struct DynamicButtons buttons;
extern struct DynamicAdjustments adjustments;
extern struct DynamicLabels labels;
extern struct Ve_Widgets *ve_widgets;
const gchar *F_warmup_labels[] = {"-40","-20",  "0", "20", "40",
			         "60", "80","100","130","160"};
const gchar *C_warmup_labels[] = {"-40","-28.8",  "-17.7", "-6.6", "4.4",
			         "15.5", "26.6","37.7","54.4","71.1"};
const gchar *accel_labels[] = {"2V/Sec","4V/sec","8V/Sec","15V/Sec"};

void build_enrichments(GtkWidget *parent_frame)
{
	gint i;
	GtkWidget *sep;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *button;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *ebox;
	gchar *string;
	GSList *group;
	GtkAdjustment *adj;
	GtkWidget *spinner;
	extern GtkTooltips *tip;
	extern GList *store_buttons;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(hbox),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The Cranking Pulsewidth determines how many milliseconds the injectors are pulsed open ONLY during engine crank-over.  Currently the ms is hardcoded to consider cranking to be from 0-300 RPM, above that it considers the engine running and uses the VEtable entries instead.  There are two fields, the enrichment at -40deg Fahreheit (-40\302\260 C as well) and at 170deg F (76.6 deg C).  At temperatures between these extremes the MS code interpolates the value.  The priming pulse is a one time pulse upon bootup of the MS unit.  This is similar in concept to pumping the gas pedel on an old carbureted car to get the accel pump to shoot at little extra in to assist with starting though the MS only does this once at startup.  Some engines need this more than others. (rotary engines especially)  You may not need thie priming pulse...",NULL);
	frame = gtk_frame_new("Cranking Pulsewidth (ms)");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	table = gtk_table_new(2,3,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	gtk_container_add(GTK_CONTAINER(frame),table);

	/* Cranking pulsewidth at -40deg F */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	ve_widgets->widget[64] = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(64));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("-40 \302\260 F.");
	labels.cr_pulse_lowtemp_lab = label;
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Cranking pulsewidth at 170deg F */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	ve_widgets->widget[65] = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(65));
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

	label = gtk_label_new("170 \302\260 F.");
	labels.cr_pulse_hightemp_lab = label;
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Priming pulse  */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	ve_widgets->widget[119] = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(119));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Priming Pulse");
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(hbox),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The Afterstart enrichment is a short term enrichment that increases the delivered fuel to the engine by the Enrichment percentage for a certain number of ignition cycles.  The enrichment tapers down for each cycle. (ignition pulse inputted into the MS ecu).  This can be shown on the runtime page after engine startup.  The Warmup enrichment bar tapers down during the afterstart enrichment period. (and the AS_ENRICH box is \"lit\") A max of 255 cycles can be entered for the Afterstart enrichment.",NULL);
	frame = gtk_frame_new("Afterstart Enrich");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	gtk_container_add(GTK_CONTAINER(frame),table);

	/* Afterstart Enrich % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(35.0,1.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,0,0);
	ve_widgets->widget[66] = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(66));
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

	label = gtk_label_new("Enrich (%)");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Afterstart Enrich Number of engine cycles */
	adj =  (GtkAdjustment *) gtk_adjustment_new(250.0,1.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	ve_widgets->widget[67] = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(67));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(GENERIC));

	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("# of Cycles");
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Warmup enrichments */
	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"   The Warmup enrichments are a longer term enrichment to assist engine performance when starting up cold.  The values are in percentage, a value of 100, means no enrichment a value of 150 means 1.5 times more fuel.  (255 is the maximum you can select) There are ten bins from the -40 \302\260 F (-40 \302\260 C.). to 160 \302\260 F (71.1 \302\260 C).  At temperatures between this the MS box interpolates the correct amount of enrichment.  See the FAQ for tuning tips and recommendations.",NULL);
	frame = gtk_frame_new("Warmup Enrichment Bins (Percent)");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	table = gtk_table_new(3,10,FALSE);
	gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,TRUE,0);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_table_set_row_spacings (GTK_TABLE (table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (table), 3);

	/* Warmup enrichment bins */
	for (i=0;i<10;i++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,255,1,10,0);
		spinner = gtk_spin_button_new(adj,1,0);
		ve_widgets->widget[WARMUP_BINS_OFFSET+i] = spinner;
		gtk_widget_set_size_request(spinner,45,-1);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(WARMUP_BINS_OFFSET+i));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(1*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(NOTHING));
		g_object_set_data(G_OBJECT(spinner), "dl_type", 
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spinner_changed),
				GINT_TO_POINTER(GENERIC));
		gtk_table_attach (GTK_TABLE (table), spinner, i, i+1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);

		string = g_strdup_printf("%s\302\260",F_warmup_labels[i]);
		label = gtk_label_new (string);
		g_free(string);
		labels.warmup_bins_lab[i]=label;
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
		gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
		gtk_table_attach (GTK_TABLE (table), label, i, i+1, 1, 2,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
	}

	label = gtk_label_new("Engine Temperature in Degrees Fahrenheit");
	labels.warmup_lab = label;
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 10, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(hbox),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"    Your VE table should be pretty well dialed in before changing these, otherwise tuning becomes harder. See the MS FAQ for more information.  The Acceleration enrichments determine when and how much extra fuel is added when the throttle is moved, or mashed to the floor...  The decel fuel cut means \"What percentage of fuel do you want the engine to get on deceleration\". Thus 100% means to not cut anything off, anything above 100% means give more fuel during decel (some turbo apps like this mode), and 1% means to cut out 99% of the fuel during decel.  The Accel enrichment bins determine how much of an accel \"Pump Shot\" is give based on how fast the throttle position is changing. Of course the MS interpolates between points.",NULL);
	frame = gtk_frame_new("Acceleration Enrichments");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	table = gtk_table_new(4,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_box_pack_start(GTK_BOX(vbox2),table,TRUE,TRUE,0);
	gtk_table_set_row_spacing(GTK_TABLE(table),1,10);

	/* TPS trigger threashold */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,1,1);
	ve_widgets->widget[83] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner), "offset", 
			GINT_TO_POINTER(83));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER((gint)(5.12*100)));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner), "dl_type", 
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("TPS Trigger Threshold\n(V/Sec)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Accel Enrich Duration (seconds) */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,1,1);
	ve_widgets->widget[84] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(84));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Accel Enrich\n Duration (Sec)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Cold Accel Addon at -40 deg */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,1,1);
	ve_widgets->widget[82] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(82));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(10*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Cold Accel Enrich\nAdd-On (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Cold Accel Multiplier */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	ve_widgets->widget[123] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(123));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Cold Accel Enrich\nMultiplier (%)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox2),sep,TRUE,TRUE,0);

	hbox2 = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox2,FALSE,TRUE,0);

	table = gtk_table_new(3,4,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),1);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_box_pack_start(GTK_BOX(hbox2),table,FALSE,TRUE,10);

	label = gtk_label_new("Acceleration Enrichment Bins (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 4, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Acceleration Enrichment Bins (4) */
	for(i=0;i<4;i++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
		spinner = gtk_spin_button_new(adj,1,1);
		ve_widgets->widget[ACCEL_BINS_OFFSET+i] = spinner;
		gtk_widget_set_size_request(spinner,45,-1);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(ACCEL_BINS_OFFSET+i));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(10*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(MULT));
		g_object_set_data(G_OBJECT(spinner), "dl_type", 
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spinner_changed),
				GINT_TO_POINTER(GENERIC));
		/* Bind data to he object for the handlers */
		gtk_table_attach (GTK_TABLE (table), spinner, i, i+1, 1, 2,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);

		label = gtk_label_new(accel_labels[i]);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
		gtk_table_attach (GTK_TABLE (table), label, i, i+1, 2, 3,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
	}

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_box_pack_start(GTK_BOX(vbox2),table,TRUE,TRUE,10);

	/* Decel Cut % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	ve_widgets->widget[85] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(85));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Decel Fuel Cut(%)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);



	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(hbox),ebox,TRUE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,"    First off select the type of O2 sensors you are using, most vehicles have a standard Narrow Band (1,2,3 or 4 wire sensor). Wide-Band sensors are available but require dedicated controller hardware to output the proper signals that refer to the O<sub>2</sub> content in the exhaust.  The Coolant Temp Activation is the minimum temp that the engine must reach before O2 correction is enabled.  The EGO Active RPM is the minimum RPM that must be exceeded before O2 correction is enabled.  Both this and the minimum temp must be reached before this feature is enabled.  The EGO switching voltage is typically set to your target AFR voltage.  For NarrowBand sensors that is about .45 Volts (for stoich), and about 2.5 Volts for Wideband (but your controller may output a different voltage consult your WB controller documentation). The EGO step is the amount of percentage change the fuel will be altered by the MS to correct a lean/rich condition. The higher the number the faster the MS will adjust, but this may cause oscillation if set too high.  See the FAQ for tips on tuning.  The # of ignition events between steps determines how fast the ECU reacts to mixture changes.  The EGO +- limit is the maximum deviation the MS will allow. (the better your VE table is set the lower this can be set).",NULL);
	frame = gtk_frame_new(NULL);
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),
			"Exhaust O<sub>2</sub> Feedback Settings");
	gtk_frame_set_label_widget(GTK_FRAME(frame),GTK_WIDGET(label));
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);
	/* O2 Sensor Type selector */
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox3),table,TRUE,TRUE,10);
	gtk_widget_set_size_request(table,-1,30);


	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),"O<sub>2</sub> Sensor Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Narrow-Band");
	buttons.nbo2_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
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
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
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
	gtk_box_pack_start(GTK_BOX(vbox3),sep,TRUE,TRUE,0);


	table = gtk_table_new(6,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,TRUE,0);
	gtk_table_set_row_spacing(GTK_TABLE(table),1,10);
	gtk_table_set_row_spacing(GTK_TABLE(table),3,10);

	/* EGO Temp Activation */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,-40.0,215.0,1.0,10.0,0);
	adjustments.ego_temp_adj = adj;
	spinner = gtk_spin_button_new(adj,1,0);
	spinners.ego_temp_active_spin = spinner;
	ve_widgets->widget[86] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(86));
	g_object_set_data(G_OBJECT(spinner),"temp_dep",GINT_TO_POINTER(TRUE));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(40*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(ADD));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Coolant Temp\nActivation(\302\260 F.)");
	labels.ego_temp_lab = label;
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO Active RPM */
	adj =  (GtkAdjustment *) gtk_adjustment_new(
			0.0,100.0,25500.0,100,1000.0,0);
	spinner = gtk_spin_button_new(adj,0,0);
	ve_widgets->widget[120] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(120));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(100*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(DIV));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("EGO Active RPM");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO Switching Voltage */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,1,2);
	ve_widgets->widget[122] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(122));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(51*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("EGO Switching\nVoltage");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO Step in % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,100,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	ve_widgets->widget[88] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(88));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("EGO Step\n(Percent)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO # of ignition events between steps */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	ve_widgets->widget[87] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(87));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 4, 5,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("# of Ignition Events\nBetween Steps");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO Limit % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,100.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	ve_widgets->widget[89] = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(89));
	g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
			GINT_TO_POINTER(1*100));
	g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(NOTHING));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(GENERIC));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 4, 5,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("EGO +/- Limit\n(Percent)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);


	frame = gtk_frame_new("Commands");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,TRUE,0);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
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
	store_buttons = g_list_append(store_buttons, (gpointer)button);
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
