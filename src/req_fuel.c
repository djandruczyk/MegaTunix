/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/* Configfile structs. (derived from an older version of XMMS) */

#include <config.h>
#include <gtk/gtk.h>
#include <defines.h>
#include <protos.h>
#include <constants.h>

gint req_fuel_popup = FALSE;
static gint rpmk_offset = 99;
static GtkWidget *popup;
struct Reqd_Fuel reqd_fuel = { NULL,NULL,NULL,NULL,350,0,19,14.7};
extern struct ms_ve_constants *ve_constants;
extern struct v1_2_Constants constants;
extern gint num_cylinders;




int reqd_fuel_popup()
{
        GtkWidget *button;
        GtkWidget *spinner;
        GtkWidget *frame;
        GtkWidget *vbox;
        GtkWidget *hbox;
        GtkWidget *label;
        GtkWidget *table;
        GtkAdjustment *adj;

        req_fuel_popup=TRUE;
        popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(popup),"Required Fuel Calc");
        gtk_container_set_border_width(GTK_CONTAINER(popup),10);
        gtk_widget_realize(popup);
        g_signal_connect(G_OBJECT(popup),"delete_event",
                        G_CALLBACK (close_popup),
                        NULL);
        g_signal_connect(G_OBJECT(popup),"destroy_event",
                        G_CALLBACK (close_popup),
                        NULL);

        vbox = gtk_vbox_new(FALSE,0);
        gtk_container_add(GTK_CONTAINER(popup),vbox);
        frame = gtk_frame_new("Constants for your vehicle");
        gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

        table =gtk_table_new(4,3,FALSE);
        gtk_table_set_col_spacings(GTK_TABLE(table),5);
        gtk_container_add(GTK_CONTAINER(frame),table);
        gtk_container_set_border_width(GTK_CONTAINER(table),5);

        label = gtk_label_new("Engine Displacement (CID)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new("Number of Cylinders");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new("Injector Flow (lbs/hr)");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new("Air-Fuel Ratio");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

        /* Engine Displacement */
        adj = (GtkAdjustment *) gtk_adjustment_new(reqd_fuel.disp,1.0,1000,
                        1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_widget_set_size_request(spinner,65,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(REQ_FUEL_DISP));
        reqd_fuel.disp_spin = spinner;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

        /* Number of Cylinders */
	reqd_fuel.cyls = num_cylinders;
//        adj = (GtkAdjustment *) gtk_adjustment_new(reqd_fuel.cyls,1.0,16,
//                        1.0,1.0,0);
        spinner = gtk_spin_button_new(constants.cylinders_adj,0,0);
        gtk_widget_set_size_request(spinner,65,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(REQ_FUEL_CYLS));
        reqd_fuel.cyls_spin = spinner;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

        /* Fuel injector flow rate in lbs/hr */
        adj = (GtkAdjustment *) gtk_adjustment_new(reqd_fuel.inj_rate,1.0,100.0,
                        1.0,1.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_widget_set_size_request(spinner,65,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(REQ_FUEL_INJ_RATE));
        reqd_fuel.inj_rate_spin = spinner;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

        /* Target Air Fuel Ratio */
        adj =  (GtkAdjustment *) gtk_adjustment_new(reqd_fuel.afr,10.0,25.5,
                        0.1,0.1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,65,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(REQ_FUEL_AFR));
        reqd_fuel.afr_spin = spinner;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 3, 4,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

        frame = gtk_frame_new("Commands");
        gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
        hbox = gtk_hbox_new(TRUE,0);
        gtk_container_add(GTK_CONTAINER(frame),hbox);
        gtk_container_set_border_width(GTK_CONTAINER(hbox),5);

        button = gtk_button_new_with_label("Calculate\nand Close");
        gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);
        g_signal_connect(G_OBJECT(button),"clicked",
                        G_CALLBACK(update_reqd_fuel),
                        NULL);
        g_signal_connect(G_OBJECT(button),"clicked",
                        G_CALLBACK (close_popup),
                        NULL);

        button = gtk_button_new_with_label("Cancel");
        gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);
        g_signal_connect(G_OBJECT(button),"clicked",
                        G_CALLBACK (close_popup),
                        NULL);

        gtk_widget_show_all(popup);

        return TRUE;
}

int close_popup(GtkWidget *widget, gpointer *data)
{
        gtk_widget_destroy(popup);
        req_fuel_popup=FALSE;
        return TRUE;
}
int update_reqd_fuel(GtkWidget *widget, gpointer *data)
{
        gfloat tmp1,tmp2;

	printf("disp %i, cyls %i, afr %f, inj_rate %i\n",reqd_fuel.disp,reqd_fuel.cyls,reqd_fuel.afr, reqd_fuel.inj_rate);
        tmp1 = 36.0*((double)reqd_fuel.disp)*4.27793;
        tmp2 = ((double) reqd_fuel.cyls) \
                * ((double)(reqd_fuel.afr)) \
                * ((double)(reqd_fuel.inj_rate));

        ve_constants->req_fuel = 10.0*(tmp1/tmp2);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_spin),
                        ve_constants->req_fuel/10.0);

	ve_constants->config11.bit.cylinders = ((reqd_fuel.cyls - 1 ) << 4);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.cylinders_spin),
                        reqd_fuel.cyls);

	/* Top is two stroke, botton is four stroke.. */
	if (ve_constants->config11.bit.eng_type)
        	ve_constants->rpmk = (int)(6000.0/((double)reqd_fuel.cyls));
	else
        	ve_constants->rpmk = (int)(12000.0/((double)reqd_fuel.cyls));

	check_req_fuel_limits();
        write_ve_const(ve_constants->rpmk, rpmk_offset);

        return TRUE;
}

