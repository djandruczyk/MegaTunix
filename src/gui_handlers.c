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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"


static int req_fuel_popup=FALSE;
static GtkWidget *popup;
extern int raw_reader_running;
extern int raw_reader_stopped;
extern int ser_context_id;
extern int read_wait_time;
extern GtkWidget *ser_statbar;
struct {
	GtkAdjustment *displacement;	/* Engine size  1-1000 Cu-in */
	GtkAdjustment *cyls;		/* # of Cylinders  1-16 */
	GtkAdjustment *inj_rate;	/* injector slow rate (lbs/hr) */
	GtkAdjustment *afr;		/* Air fuel ratio 10-25.5 */
} reqd_fuel;

void leave(GtkWidget *widget, gpointer *data)
{
        save_config();
	stop_serial_thread();
        /* Free all buffers */
	close_serial();
        mem_dealloc();
        gtk_main_quit();
}

void text_entry_handler(GtkWidget * widget, gpointer *data)
{
	gchar *entry_text;
	gfloat tmpf = 0;
	entry_text = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
	switch ((gint)data)
	{
		case INJ_OPEN_TIME:
			tmpf = atof(entry_text);
			printf("inj open time set to %f\n",tmpf);
	}
	/* update the widget in case data was out of bounds */
}



int std_button_handler(GtkWidget *widget, gpointer *data)
{
	switch ((gint)data)
	{
		case START_REALTIME:
			start_serial_thread();
			break;
		case STOP_REALTIME:
			stop_serial_thread();
			break;
		case REQD_FUEL_POPUP:
			if (!req_fuel_popup)
				reqd_fuel_popup();
			break;
		case READ_FROM_MS:
			printf("Going to begin read VE/constants \n");
			read_ve_const();
			printf("read VE/constants complete\n");
		//	update_const_ve();
			break;
		case WRITE_TO_MS:
			write_ve_const();
			break;
	}
	return TRUE;
}

void update_statusbar(GtkWidget *status_bar,int context_id, gchar * message)
{
	/* takes 3 args, 
	 * the GtkWidget pointer to the statusbar,
	 * the context_id of the statusbar in arg[0],
	 * and the string to be sent to that bar
	 *
	 * Fairly generic, should work for multiple statusbars
	 *
	 */
	

	gtk_statusbar_pop(GTK_STATUSBAR(status_bar),
			context_id);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar),
			context_id,
			message);
}

	
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
	adj =  (GtkAdjustment *) gtk_adjustment_new(350.0,1.0,1000,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_widget_set_size_request(spinner,65,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(REQ_FUEL_DISP));
        reqd_fuel.displacement = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	/* Number of Cylinders */
	adj =  (GtkAdjustment *) gtk_adjustment_new(8.0,1.0,16,1,1,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_widget_set_size_request(spinner,65,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(REQ_FUEL_CYLS));
        reqd_fuel.cyls = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	/* Fuel injector flow rate in lbs/hr */
	adj =  (GtkAdjustment *) gtk_adjustment_new(19.0,1.0,100.0,1.0,1.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_widget_set_size_request(spinner,65,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(REQ_FUEL_INJ_RATE));
        reqd_fuel.inj_rate = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	/* Target Air Fuel Ratio */
	adj =  (GtkAdjustment *) gtk_adjustment_new(14.7,10.0,25.5,0.1,0.1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,65,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(REQ_FUEL_AFR));
        reqd_fuel.afr = adj;
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
	printf("update required fuel\n");
	return TRUE;
}

int spinner_changed(GtkWidget *widget, gpointer *data)
{
	/* Gets the value from the spinbutton then modifues the 
	 * necessary deta in the the app and calls any handlers 
	 * if necessary.  works well,  one generic function with a 
	 * select/case branch to handle the choices..
	 */
	gdouble value;
	value = gtk_spin_button_get_value((GtkSpinButton *)widget);
	
	switch ((gint)data)
	{
		case SET_SER_PORT:
			if(serial_params.open)
			{
				if (raw_reader_running)
					stop_serial_thread();
				close_serial();
			}
			open_serial((int)value);
			setup_serial_params();
			break;
		case SER_POLL_TIMEO:
			serial_params.poll_timeout = (gint)value;
			break;
		case SER_INTERVAL_DELAY:
			serial_params.read_wait = (gint)value;
			break;
		default:
			break;
	}
	return TRUE;

}

void update_const_ve()
{
	// Stub function, does nothing yet... 
}
