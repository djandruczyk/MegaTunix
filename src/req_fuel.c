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
#include <configfile.h>
#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <gui_handlers.h>
#include <math.h>
#include <ms_structures.h>
#include <req_fuel.h>
#include <serialio.h>
#include <structures.h>

static gint rpmk_offset = 98;
extern struct DynamicSpinners spinners;
extern struct DynamicAdjustments adjustments;
extern GdkColor red;
extern GdkColor black;

void req_fuel_change(void *ptr)
{
	gfloat tmp1,tmp2;
	struct Reqd_Fuel *reqd_fuel = NULL;
	if (ptr)
		reqd_fuel = (struct Reqd_Fuel *) ptr;
	else
	{
		fprintf(stderr,__FILE__": req_fuel_change(), invalid pointer passed\n");
		return;
	}

	reqd_fuel->actual_inj_flow = ((double)reqd_fuel->rated_inj_flow *
			sqrt((double)reqd_fuel->actual_pressure / (double)reqd_fuel->rated_pressure));

#ifdef DEBUG
	printf("Rated injector flow is %f lbs/hr\n",reqd_fuel->rated_inj_flow);
	printf("Rated fuel pressure is %f bar\n",reqd_fuel->rated_pressure);
	printf("Actual fuel pressure is %f bar\n",reqd_fuel->actual_pressure);
	printf("Calculated injector flow rate is %f lbs/hr\n",reqd_fuel->actual_inj_flow);
	printf("Target AFR is %f lbs/hr\n",reqd_fuel->target_afr);
#endif

	tmp1 = 36.0*((double)reqd_fuel->disp)*4.27793;
	tmp2 = ((double) reqd_fuel->cyls) \
		* ((double)(reqd_fuel->target_afr)) \
		* ((double)(reqd_fuel->actual_inj_flow));

	reqd_fuel->calcd_reqd_fuel = tmp1/tmp2;
	if (GTK_IS_WIDGET(reqd_fuel->calcd_val_spin))
	{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(reqd_fuel->calcd_val_spin),reqd_fuel->calcd_reqd_fuel);


		if (reqd_fuel->calcd_reqd_fuel > 25.5)
			gtk_widget_modify_text(GTK_WIDGET(reqd_fuel->calcd_val_spin),
					GTK_STATE_NORMAL,&red);
		else
			gtk_widget_modify_text(GTK_WIDGET(reqd_fuel->calcd_val_spin),
					GTK_STATE_NORMAL,&black);
	}
}

gboolean reqd_fuel_popup(void * data)
{

	GtkWidget *button;
	GtkWidget *spinner;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *table2;
	GtkWidget *popup;
	GtkAdjustment *adj;
	gchar * tmpbuf;
	struct Reqd_Fuel *reqd_fuel = NULL;

	if (data)
		reqd_fuel = (struct Reqd_Fuel *) data;
	else
	{
		fprintf(stderr,__FILE__": reqd_fuel_popup(), pointer passed is invalid, contact author\n");
		return FALSE;
	}

	if (reqd_fuel->visible)
		return TRUE;
	else
		reqd_fuel->visible = TRUE;

	popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	reqd_fuel->popup = popup;	
	tmpbuf = g_strdup_printf("Required Fuel Calculator for Table %i\n",reqd_fuel->table);
	gtk_window_set_title(GTK_WINDOW(popup),tmpbuf);
	g_free(tmpbuf);
	gtk_container_set_border_width(GTK_CONTAINER(popup),10);
	gtk_widget_realize(popup);
	g_object_set_data(G_OBJECT(popup),"data",reqd_fuel);
	g_signal_connect_swapped(G_OBJECT(popup),"delete_event",
			G_CALLBACK (close_popup),
			(gpointer)popup);
	g_signal_connect_swapped(G_OBJECT(popup),"destroy_event",
			G_CALLBACK (close_popup),
			(gpointer)popup);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(popup),vbox);
	tmpbuf = g_strdup_printf("Required Fuel parameters for table %i\n",reqd_fuel->table);
	frame = gtk_frame_new(tmpbuf);
	g_free(tmpbuf);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	table = gtk_table_new(4,5,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacing(GTK_TABLE(table),1,25);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
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

	label = gtk_label_new("Target Air Fuel Ratio (AFR)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Rated Injector Flow (lbs/hr)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 3, 4, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Rated Fuel Pressure (bar)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 3, 4, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Actual Fuel Pressure (bar)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 3, 4, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);


	// Engine Displacement 
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->disp,1.0,1000,1.0,10.0,0);
			
	spinner = gtk_spin_button_new(adj,0,0);
	gtk_widget_set_size_request(spinner,65,-1);
	g_object_set_data(G_OBJECT(spinner),"data",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_DISP));
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	// Number of Cylinders 
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->cyls,1,12,1.0,4.0,0);
			
	spinner = gtk_spin_button_new(adj,0,0);
	gtk_widget_set_size_request(spinner,65,-1);
	g_object_set_data(G_OBJECT(spinner),"data",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_CYLS));
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	// Target AFR
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->target_afr,1.0,100.0,1.0,1.0,0);
			
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	g_object_set_data(G_OBJECT(spinner),"data",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_AFR));
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	// Rated Injector Flow
	adj =  (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->rated_inj_flow,10.0,25.5,0.1,0.1,0);
			
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	g_object_set_data(G_OBJECT(spinner),"data",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_RATED_INJ_FLOW));
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 4, 5, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	// Rated fuel pressure in bar 
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->rated_pressure,0.1,10.0,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	g_object_set_data(G_OBJECT(spinner),"data",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_RATED_PRESSURE));
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 4, 5, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	// Actual fuel pressure in bar 
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->actual_pressure,0.1,10.0,0.1,1.0,0);
			
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	g_object_set_data(G_OBJECT(spinner),"data",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_ACTUAL_PRESSURE));
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 4, 5, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	table2 = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table2),25);
	gtk_table_attach (GTK_TABLE (table), table2, 0, 5, 3, 4,
			(GtkAttachOptions) (0),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Preliminary Reqd. Fuel (1 cycle)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table2), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	// Preliminary Required Fuel Value
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->calcd_reqd_fuel,0.1,25.5,0.1,1.0,0);
			
	spinner = gtk_spin_button_new(adj,0,1);
	reqd_fuel->calcd_val_spin = spinner;
	gtk_widget_set_size_request(spinner,65,-1);
	g_object_set_data(G_OBJECT(spinner),"data",reqd_fuel);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table2), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Commands");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox),5);

	button = gtk_button_new_with_label("Save and Close");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);
	g_object_set_data(G_OBJECT(button),"data",reqd_fuel);
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(save_reqd_fuel),
			NULL);
	g_object_set_data(G_OBJECT(button),"data",reqd_fuel);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK (close_popup),
			(gpointer)popup);

	button = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);
	g_object_set_data(G_OBJECT(button),"data",reqd_fuel);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK (close_popup),
			(gpointer)popup);

	gtk_widget_show_all(popup);

	return TRUE;
}

gboolean save_reqd_fuel(GtkWidget *widget, gpointer data)
{
	struct Reqd_Fuel * reqd_fuel = NULL;
	struct Ve_Const_Std *ve_const;
	gint dload_val;
	extern unsigned char * ms_data;
	ConfigFile *cfgfile;
	gchar *filename;
	gchar *tmpbuf;

	reqd_fuel = (struct Reqd_Fuel *)g_object_get_data(G_OBJECT(widget),"data");
	if (reqd_fuel->table == 1)
		ve_const = (struct Ve_Const_Std *)ms_data;
	else if (reqd_fuel->table == 2)
		ve_const = (struct Ve_Const_Std *) (ms_data + MS_PAGE_SIZE);
	else
	{
		fprintf(stderr,__FILE__": save_reqd_fuel(), reqd_fuel->table is invalid (%i)\n",reqd_fuel->table);
		return FALSE;
	}	

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(reqd_fuel->reqd_fuel_spin),
			reqd_fuel->calcd_reqd_fuel);
	
	/* Top is two stroke, botton is four stroke.. */
	if (ve_const->config11.bit.eng_type)
		ve_const->rpmk = (int)(6000.0/((double)reqd_fuel->cyls));
	else
		ve_const->rpmk = (int)(12000.0/((double)reqd_fuel->cyls));

	check_req_fuel_limits();
	dload_val = ve_const->rpmk;
	if (reqd_fuel->table == 1)
		write_ve_const(dload_val, rpmk_offset);
	else
		write_ve_const(dload_val, rpmk_offset+MS_PAGE_SIZE);

	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/config", NULL);
	tmpbuf = g_strdup_printf("Req_Fuel_Table_%i",reqd_fuel->table);
        cfgfile = cfg_open_file(filename);
	if (cfgfile)	// If it opened nicely 
	{
                cfg_write_int(cfgfile,tmpbuf,"Displacement",reqd_fuel->disp);
                cfg_write_int(cfgfile,tmpbuf,"Cylinders",reqd_fuel->cyls);
                cfg_write_float(cfgfile,tmpbuf,"Rated_Inj_Flow",
                                reqd_fuel->rated_inj_flow);
                cfg_write_float(cfgfile,tmpbuf,"Rated_Pressure",
                                reqd_fuel->rated_pressure);
                cfg_write_float(cfgfile,tmpbuf,"Actual_Pressure",
                                reqd_fuel->actual_inj_flow);
                cfg_write_float(cfgfile,tmpbuf,"Target_AFR",
                                reqd_fuel->target_afr);

		cfg_write_file(cfgfile,filename);
		cfg_free(cfgfile);
		g_free(cfgfile);
	}
	g_free(filename);
	g_free(tmpbuf);
	
	return TRUE;
}

gboolean close_popup(GtkWidget * widget)
{
	struct Reqd_Fuel *reqd_fuel = NULL;

	reqd_fuel = (struct Reqd_Fuel *)g_object_get_data(G_OBJECT(widget),"data");
	gtk_widget_destroy(reqd_fuel->popup);
	reqd_fuel->visible = FALSE;
	return TRUE;
}
