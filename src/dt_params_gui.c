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
#include <dt_params_gui.h>
#include <defines.h>
#include <enums.h>
#include <gui_handlers.h>
#include <structures.h>

GList *table_map_widgets;


void build_dt_params(GtkWidget *parent_frame)
{
	GtkWidget *button;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *table;
	GSList *group = NULL;
	extern GList *store_widgets;
	extern struct DynamicButtons buttons;
	extern GtkTooltips *tip;
	extern GList *dt_widgets;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	frame = gtk_frame_new("Injector to Table Mapping");
	dt_widgets = g_list_append(dt_widgets,(gpointer)frame);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,FALSE,0);

	table = gtk_table_new(4,5,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table), 5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	label = gtk_label_new("Dual Table Mode");
	dt_widgets = g_list_append(dt_widgets,(gpointer)label);
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Dualtable Mode Enable button */
        button = gtk_check_button_new();
	buttons.dt_mode = button;
	dt_widgets = g_list_append(dt_widgets,(gpointer)button);
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(0));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"single",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Off");
	dt_widgets = g_list_append(dt_widgets,(gpointer)label);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Table 1");
	dt_widgets = g_list_append(dt_widgets,(gpointer)label);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Table 2");
	dt_widgets = g_list_append(dt_widgets,(gpointer)label);
	gtk_table_attach (GTK_TABLE (table), label, 3, 4, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Gamma E");
	dt_widgets = g_list_append(dt_widgets,(gpointer)label);
	gtk_table_attach (GTK_TABLE (table), label, 4, 5, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 20, 0);

	label = gtk_label_new("Injector 1");
	dt_widgets = g_list_append(dt_widgets,(gpointer)label);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Injector 2");
	dt_widgets = g_list_append(dt_widgets,(gpointer)label);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Radio buttons for injector Channel 1 */
	/* Inj 1 not driven */
        button = gtk_radio_button_new(NULL);
	buttons.inj1_not_driven = button;
	table_map_widgets = g_list_append(table_map_widgets,(gpointer)button);
	dt_widgets = g_list_append(dt_widgets,(gpointer)button);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(6));
	g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Inj 1 bound to table 1 */
        button = gtk_radio_button_new(group);
	buttons.inj1_table1 = button;
	table_map_widgets = g_list_append(table_map_widgets,(gpointer)button);
	dt_widgets = g_list_append(dt_widgets,(gpointer)button);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(6));
	g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Inj 1 bound to table 2 */
        button = gtk_radio_button_new(group);
	buttons.inj1_table2 = button;
	table_map_widgets = g_list_append(table_map_widgets,(gpointer)button);
	dt_widgets = g_list_append(dt_widgets,(gpointer)button);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(2));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(6));
	g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Inj 1 Gamma E enable */
        button = gtk_check_button_new();
	buttons.inj1_gammae = button;
	table_map_widgets = g_list_append(table_map_widgets,(gpointer)button);
	dt_widgets = g_list_append(dt_widgets,(gpointer)button);
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(5));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(32));
        g_object_set_data(G_OBJECT(button),"single",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);
	gtk_table_attach (GTK_TABLE (table), button, 4, 5, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 20, 0);


	/* Radio buttons for injector Channel 2 */
	/* Inj 2 not driven */
        button = gtk_radio_button_new(NULL);
	buttons.inj2_not_driven = button;
	table_map_widgets = g_list_append(table_map_widgets,(gpointer)button);
	dt_widgets = g_list_append(dt_widgets,(gpointer)button);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(24));
	g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Inj 2 bound to table 1 */
        button = gtk_radio_button_new(group);
	buttons.inj2_table1 = button;
	table_map_widgets = g_list_append(table_map_widgets,(gpointer)button);
	dt_widgets = g_list_append(dt_widgets,(gpointer)button);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(24));
	g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Inj 2 bound to table 2 */
        button = gtk_radio_button_new(group);
	buttons.inj2_table2 = button;
	table_map_widgets = g_list_append(table_map_widgets,(gpointer)button);
	dt_widgets = g_list_append(dt_widgets,(gpointer)button);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(2));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(24));
	g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Inj 2 Gamma E enable */
        button = gtk_check_button_new();
	buttons.inj2_gammae = button;
	table_map_widgets = g_list_append(table_map_widgets,(gpointer)button);
	dt_widgets = g_list_append(dt_widgets,(gpointer)button);
	g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(92));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(14));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(6));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(64));
        g_object_set_data(G_OBJECT(button),"single",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);
	gtk_table_attach (GTK_TABLE (table), button, 4, 5, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 20, 0);

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
