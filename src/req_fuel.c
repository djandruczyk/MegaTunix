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
#include <conversions.h>
#include <defines.h>
#include <enums.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gui_handlers.h>
#include <math.h>
#include <mode_select.h>
#include <ms_structures.h>
#include <notifications.h>
#include <req_fuel.h>
#include <serialio.h>
#include <structures.h>

static gint rpmk_offset = 98;
extern struct DynamicSpinners spinners;
extern struct DynamicAdjustments adjustments;
extern GdkColor red;
extern GdkColor black;
gint num_squirts_1 = 1;
gint num_squirts_2 = 1;
gint num_cylinders_1 = 1;
gint num_cylinders_2 = 1;
gint num_injectors_1 = 1;
gint num_injectors_2 = 1;
gfloat req_fuel_total_1 = 0.0;
gfloat req_fuel_total_2 = 0.0;


void req_fuel_change(void *ptr)
{
	gfloat tmp1,tmp2;
	struct Reqd_Fuel *reqd_fuel = NULL;
	if (ptr)
		reqd_fuel = (struct Reqd_Fuel *) ptr;
	else
	{
		g_fprintf(stderr,__FILE__": req_fuel_change(), invalid pointer passed\n");
		return;
	}

	reqd_fuel->actual_inj_flow = ((double)reqd_fuel->rated_inj_flow *
			sqrt((double)reqd_fuel->actual_pressure / (double)reqd_fuel->rated_pressure));

#ifdef DEBUG
	g_printf("Rated injector flow is %f lbs/hr\n",reqd_fuel->rated_inj_flow);
	g_printf("Rated fuel pressure is %f bar\n",reqd_fuel->rated_pressure);
	g_printf("Actual fuel pressure is %f bar\n",reqd_fuel->actual_pressure);
	g_printf("Calculated injector flow rate is %f lbs/hr\n",reqd_fuel->actual_inj_flow);
	g_printf("Target AFR is %f lbs/hr\n",reqd_fuel->target_afr);
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
		g_fprintf(stderr,__FILE__": reqd_fuel_popup(), pointer passed is invalid, contact author\n");
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
	g_object_set_data(G_OBJECT(popup),"reqd_fuel",reqd_fuel);
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
	g_object_set_data(G_OBJECT(spinner),"reqd_fuel",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
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
	g_object_set_data(G_OBJECT(spinner),"reqd_fuel",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
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
	g_object_set_data(G_OBJECT(spinner),"reqd_fuel",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
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
	g_object_set_data(G_OBJECT(spinner),"reqd_fuel",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
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
	g_object_set_data(G_OBJECT(spinner),"reqd_fuel",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
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
	g_object_set_data(G_OBJECT(spinner),"reqd_fuel",reqd_fuel);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinbutton_handler),
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
	g_object_set_data(G_OBJECT(spinner),"reqd_fuel",reqd_fuel);
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
	g_object_set_data(G_OBJECT(button),"reqd_fuel",reqd_fuel);
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(save_reqd_fuel),
			NULL);
	g_object_set_data(G_OBJECT(button),"reqd_fuel",reqd_fuel);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK (close_popup),
			(gpointer)popup);

	button = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);
	g_object_set_data(G_OBJECT(button),"reqd_fuel",reqd_fuel);
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

	reqd_fuel = (struct Reqd_Fuel *)g_object_get_data(G_OBJECT(widget),"reqd_fuel");
	if (reqd_fuel->table == 1)
		ve_const = (struct Ve_Const_Std *)ms_data;
	else if (reqd_fuel->table == 2)
		ve_const = (struct Ve_Const_Std *) (ms_data + MS_PAGE_SIZE);
	else
	{
		g_fprintf(stderr,__FILE__": save_reqd_fuel(), reqd_fuel->table is invalid (%i)\n",reqd_fuel->table);
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
		write_ve_const(dload_val, rpmk_offset, FALSE);
	else
		write_ve_const(dload_val, rpmk_offset+MS_PAGE_SIZE, FALSE);

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
                cfg_write_float(cfgfile,tmpbuf,"Actual_Inj_Flow",
                                reqd_fuel->actual_inj_flow);
                cfg_write_float(cfgfile,tmpbuf,"Actual_Pressure",
                                reqd_fuel->actual_pressure);
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

	reqd_fuel = (struct Reqd_Fuel *)g_object_get_data(G_OBJECT(widget),"reqd_fuel");
	gtk_widget_destroy(reqd_fuel->popup);
	reqd_fuel->visible = FALSE;
	return TRUE;
}

void check_req_fuel_limits()
{
	gfloat tmp = 0.0;
	gfloat req_fuel_per_squirt = 0.0;
	gint lim_flag = 0;
	gint dload_val = 0;
	gint offset = 0;
	extern unsigned int ecu_caps;
	extern gboolean paused_handlers;
	extern GHashTable * interdep_vars_1;
	extern GHashTable * interdep_vars_2;
	extern unsigned char *ms_data;
	struct Ve_Const_Std *ve_const = NULL;
	struct Ve_Const_DT_1 *ve_const_dt1 = NULL;
	struct Ve_Const_DT_2 *ve_const_dt2 = NULL;


	if (ecu_caps & DUALTABLE)
	{
		ve_const_dt1 = (struct Ve_Const_DT_1 *)ms_data;
		ve_const_dt2 = (struct Ve_Const_DT_2 *) (ms_data+MS_PAGE_SIZE);
		/* F&H Dualtable required Fuel calc
		 *
		 *                                        / num_injectors \
		 *         	   req_fuel_per_squirt * (-----------------)
		 *                                        \    divider    /
		 * req_fuel_total = -------------------------------------------
		 *				10
		 *
		 * where divider = num_cylinders/num_squirts;
		 *
		 * rearranging to solve for req_fuel_per_squirt...
		 *
		 *                        (req_fuel_total * 10)
		 * req_fuel_per_squirt =  ---------------------
		 *			    / num_injectors \
		 *                         (-----------------)
		 *                          \    divider    /
		 */

		/* TABLE 1 */
		tmp = (float)(num_injectors_1)/(float)(ve_const_dt1->divider);
		req_fuel_per_squirt = ((float)req_fuel_total_1 * 10.0)/tmp;

		if (req_fuel_per_squirt != ve_const_dt1->req_fuel)
		{
			if (req_fuel_per_squirt > 255)
				lim_flag = 1;
			if (req_fuel_per_squirt < 0)
				lim_flag = 1;
			if (num_cylinders_1 % num_squirts_1)
				lim_flag = 1;
		}
		/* Required Fuel per SQUIRT */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(
					spinners.req_fuel_per_squirt_1_spin),
				req_fuel_per_squirt/10.0);

		/* Throw warning if an issue */
		if (lim_flag)
			set_interdep_state(RED,1);
		else
		{
			set_interdep_state(BLACK,1);

			if (paused_handlers)
				return;
			offset = 90;
			dload_val = convert_before_download(offset,
					req_fuel_per_squirt,FALSE);
			write_ve_const(dload_val, offset, FALSE);
			/* Call handler to empty interdependant hash table */
			g_hash_table_foreach_remove(interdep_vars_1,drain_hashtable,NULL);
					
		}

		lim_flag = 0;
		/* TABLE 2 */
		tmp = (float)(num_injectors_2)/(float)(ve_const_dt2->divider);
		req_fuel_per_squirt = ((float)req_fuel_total_2 * 10.0)/tmp;

		if (req_fuel_per_squirt != ve_const_dt2->req_fuel)
		{
			if (req_fuel_per_squirt > 255)
				lim_flag = 1;
			if (req_fuel_per_squirt < 0)
				lim_flag = 1;
			if (num_cylinders_2 % num_squirts_2)
				lim_flag = 1;
		}

		/* Required Fuel per SQUIRT */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(
					spinners.req_fuel_per_squirt_2_spin),
				req_fuel_per_squirt/10);

		/* Throw warning if an issue */
		if (lim_flag)
			set_interdep_state(RED,2);
		else
		{
			set_interdep_state(BLACK,2);

			if (paused_handlers)
				return;
			offset = 90 + MS_PAGE_SIZE;
			dload_val = convert_before_download(offset,
					req_fuel_per_squirt,FALSE);
			write_ve_const(dload_val, offset, FALSE);
			g_hash_table_foreach_remove(interdep_vars_2,drain_hashtable,NULL);
					
		}
	}// END Dualtable Req fuel checks... */
	else
	{
		ve_const = (struct Ve_Const_Std *)ms_data;

		/* B&G, MSnS, MSnEDIS Required Fuel Calc
		 *
		 *                                        /     num_injectors_1     \
		 *         	   req_fuel_per_squirt * (-------------------------)
		 *                                        \ divider*(alternate+1) /
		 * req_fuel_total = --------------------------------------------------
		 *				10
		 *
		 * where divider = num_cylinders_1/num_squirts_1;
		 *
		 * rearranging to solve for req_fuel_per_squirt...
		 *
		 *                        (req_fuel_total * 10)
		 * req_fuel_per_squirt =  ----------------------
		 *			    /  num_injectors  \
		 *                         (-------------------)
		 *                          \ divider*(alt+1) /
		 *
		 * 
		 */

		tmp =	((float)(num_injectors_1))/((float)ve_const->divider*(float)(ve_const->alternate+1));

		/* This is 1 tenth the value as the one screen stuff is 1/10th 
		 * for the ms variable,  it gets converted farther down, just 
		 * before download to the MS
		 */
		req_fuel_per_squirt = ((float)req_fuel_total_1*10.0)/tmp;

		if (req_fuel_per_squirt != ve_const->req_fuel)
		{
			if (req_fuel_per_squirt > 255)
				lim_flag = 1;
			if (req_fuel_per_squirt < 0)
				lim_flag = 1;
			if (num_cylinders_1 % num_squirts_1)
				lim_flag = 1;
		}
		/* req-fuel info box  */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(
					spinners.req_fuel_per_squirt_1_spin),
				req_fuel_per_squirt/10.0);

		if (lim_flag)
			set_interdep_state(RED,1);
		else
		{
			set_interdep_state(BLACK,1);

			/* All Tested succeeded, download Required fuel, 
			 * then iterate through the list of offsets of changed
			 * inter-dependant variables, extract the data out of 
			 * the companion array, and send to ECU.  Then free
			 * the offset GList, and clear the array...
			 */

			/* Handlers get paused during a read of MS VE/Constants. We
			 * don't need to write anything back during this window. 
			 */
			if (paused_handlers)
				return;
			offset = 90;
			dload_val = convert_before_download(offset,
					req_fuel_per_squirt,FALSE);
			write_ve_const(dload_val, offset, FALSE);
			g_hash_table_foreach_remove(interdep_vars_1,drain_hashtable,NULL);
		}
	} // End B&G style Req Fuel check 
	return ;

}

