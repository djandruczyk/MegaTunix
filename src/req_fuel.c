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

#include <config.h>
#include <configfile.h>
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <gui_handlers.h>
#include <math.h>
#include <mode_select.h>
#include <ms_structures.h>
#include <notifications.h>
#include <req_fuel.h>
#include <serialio.h>
#include <structures.h>
#include <threads.h>

extern GdkColor red;
extern GdkColor black;
gint num_squirts_1 = 1;
gint num_squirts_2 = 1;
gint num_cyls_1 = 1;
gint num_cyls_2 = 1;
gint num_inj_1 = 1;
gint num_inj_2 = 1;
gint divider_1 = 0;
gint divider_2 = 0;
gint alternate_1 = 0;
gint last_num_squirts_1 = -1;
gint last_num_squirts_2 = -1;
gint last_num_cyls_1 = -1;
gint last_num_cyls_2 = -1;
gint last_num_inj_1 = -1;
gint last_num_inj_2 = -1;
gint last_divider_1= -1;
gint last_divider_2 = -1;
gint last_alternate_1 = -1;
gfloat req_fuel_total_1 = 0.0;
gfloat req_fuel_total_2 = 0.0;
gfloat last_req_fuel_total_1 = 0.0;
gfloat last_req_fuel_total_2 = 0.0;


/*!
 \brief req_fuel_change() is called whenever the req_fuel variable from 
 the reqfuel_popup is changed and this recalcualtes things and updated 
 the necesary spinbuttons
 \param widget (GtkWidget *) the spinbutton that changed
 */
void req_fuel_change(GtkWidget *widget)
{
	gfloat tmp1,tmp2;
	struct Reqd_Fuel *reqd_fuel = NULL;
	if (g_object_get_data(G_OBJECT(widget),"reqd_fuel"))
		reqd_fuel = (struct Reqd_Fuel *) g_object_get_data(G_OBJECT(widget),"reqd_fuel");
	else
	{
		dbg_func(__FILE__": req_fuel_change()\n\treqd_fuel data NOT bound to the widget pointer passed, RETURNING...\n",CRITICAL);
		return;
	}

	reqd_fuel->actual_inj_flow = ((double)reqd_fuel->rated_inj_flow *
			sqrt((double)reqd_fuel->actual_pressure / (double)reqd_fuel->rated_pressure));

	dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tRated injector flow is %f lbs/hr\n",reqd_fuel->rated_inj_flow),REQ_FUEL);
	dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tRated fuel pressure is %f bar\n",reqd_fuel->rated_pressure),REQ_FUEL);
	dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tActual fuel pressure is %f bar\n",reqd_fuel->actual_pressure),REQ_FUEL);
	dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tCalculated injector flow rate is %f lbs/hr\n",reqd_fuel->actual_inj_flow),REQ_FUEL);
	dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tTarget AFR is %f lbs/hr\n",reqd_fuel->target_afr),REQ_FUEL);

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


/*!
 \brief reqd_fuel_popup() is called when the user requests it.  IT provides
 spinbuttons for cylinders, injectors, flow rates, fuel pressure and so on
 to calculate the amount of fuel needed for 1 cylinder for 1 complete cycle
 \param widget (GtkWidget *) pointer to widget that called this function
 */
gboolean reqd_fuel_popup(GtkWidget * widget)
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
	gchar * tmpbuf = NULL;
	gint page = -1;
	struct Reqd_Fuel *reqd_fuel = NULL;

	page = (gint)g_object_get_data(G_OBJECT(widget),"page");

	if (g_object_get_data(G_OBJECT(widget),"reqd_fuel"))
		reqd_fuel = (struct Reqd_Fuel *)g_object_get_data(G_OBJECT(widget),"reqd_fuel");
	else
	{
		reqd_fuel = initialize_reqd_fuel(page);
		g_object_set_data(G_OBJECT(widget),"reqd_fuel",reqd_fuel);
	}

	if (reqd_fuel->visible)
		return TRUE;
	else
		reqd_fuel->visible = TRUE;

	popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	reqd_fuel->popup = popup;	
	tmpbuf = g_strdup_printf("Required Fuel Calculator for Page %i\n",reqd_fuel->page);
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
	tmpbuf = g_strdup_printf("Required Fuel parameters for page %i\n",reqd_fuel->page);
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
	g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(REQ_FUEL_DISP));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
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
	g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(REQ_FUEL_CYLS));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
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
	g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(REQ_FUEL_AFR));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	// Rated Injector Flow
	adj =  (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->rated_inj_flow,10.0,255,0.1,1,0);

	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	g_object_set_data(G_OBJECT(spinner),"reqd_fuel",reqd_fuel);
	g_object_set_data(G_OBJECT(spinner),"handler",
			GINT_TO_POINTER(REQ_FUEL_RATED_INJ_FLOW));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
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
	g_object_set_data(G_OBJECT(spinner),"handler",	
			GINT_TO_POINTER(REQ_FUEL_RATED_PRESSURE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
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
	g_object_set_data(G_OBJECT(spinner),"handler",	
			GINT_TO_POINTER(REQ_FUEL_ACTUAL_PRESSURE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
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


/*!
 \brief save_reqd_fuel() called when the reqfuel popup closes, to save the
 values to a config file
 \param widget (GtkWidget *) widget that contains a pointer to the Reqd_Fuel 
 datastructure to save
 \param data (gpointer) unused
 \returns TRUE
 */
gboolean save_reqd_fuel(GtkWidget *widget, gpointer data)
{
	struct Reqd_Fuel * reqd_fuel = NULL;
	gint dload_val = 0;
	gint rpmk_offset = 0;
	gint page = 0;
	union config11 cfg11;
	extern struct Firmware_Details *firmware;
	extern gint **ms_data;
	extern GHashTable *dynamic_widgets;
	ConfigFile *cfgfile;
	gchar *filename = NULL;
	gchar *tmpbuf = NULL;

	reqd_fuel = (struct Reqd_Fuel *)g_object_get_data(G_OBJECT(widget),"reqd_fuel");

	gtk_spin_button_set_value(GTK_SPIN_BUTTON
			(g_hash_table_lookup(dynamic_widgets,g_strdup_printf("req_fuel_per_cycle_%i_spin",1+reqd_fuel->page))),
			reqd_fuel->calcd_reqd_fuel);

	/* Top is two stroke, botton is four stroke.. */
	page = reqd_fuel->page;
	cfg11.value = ms_data[page][firmware->page_params[page]->cfg11_offset];
	rpmk_offset = firmware->page_params[page]->rpmk_offset;
	if (cfg11.bit.eng_type)
		ms_data[page][rpmk_offset] = (int)(6000.0/((double)reqd_fuel->cyls));
	else
		ms_data[page][rpmk_offset] = (int)(12000.0/((double)reqd_fuel->cyls));

	check_req_fuel_limits();
	dload_val = ms_data[page][rpmk_offset];
	write_ve_const(widget, reqd_fuel->page, rpmk_offset, dload_val, FALSE);

	filename = g_strconcat(HOME(), "/.MegaTunix/config", NULL);
	tmpbuf = g_strdup_printf("Req_Fuel_Page_%i",reqd_fuel->page);
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


/*!
 \brief close_popup() does exactly that.
 \param widget (GtkWidget *) is the widget that contains the pointer to the 
 Reqd_Fuel struct.
 \returns TRUE
 */
gboolean close_popup(GtkWidget * widget)
{
	struct Reqd_Fuel *reqd_fuel = NULL;

	reqd_fuel = (struct Reqd_Fuel *)g_object_get_data(G_OBJECT(widget),"reqd_fuel");
	gtk_widget_destroy(reqd_fuel->popup);
	reqd_fuel->visible = FALSE;
	return TRUE;
}


/*!
 \brief check_req_fuel_limits() gets called any time reqf_fuel OR ANY of it's
 interdependant variable changes.  This function recalculates values and shows
 error status on the gui if needed and send the new values to the ECU when 
 appropropriate
 */
void check_req_fuel_limits()
{
	gfloat tmp = 0.0;
	gfloat req_fuel_per_squirt = 0.0;
	gboolean lim_flag = FALSE;
	gint dload_val = 0;
	gint offset = 0;
	gint page = -1;
	gint rpmk_offset = 0;
	gint divider = 0;
	gint alternate = 0;
	union config11 cfg11;
	extern gint ecu_caps;
	extern gboolean paused_handlers;
	extern GHashTable ** interdep_vars;
	extern GHashTable *dynamic_widgets;
	extern gint **ms_data;
	extern struct Firmware_Details *firmware;

	if (ecu_caps & DUALTABLE)
	{
		/* F&H Dualtable required Fuel calc
		 *
		 *                                        /    num_inj_x  \
		 *         	   req_fuel_per_squirt * (-----------------)
		 *                                        \    divider    /
		 * req_fuel_total = -------------------------------------------
		 *				10
		 *
		 * where divider = num_cyls/num_squirts;
		 *
		 * rearranging to solve for req_fuel_per_squirt...
		 *
		 *                        (req_fuel_total * 10)
		 * req_fuel_per_squirt =  ---------------------
		 *			    /   num_inj_x   \
		 *                         (-----------------)
		 *                          \    divider    /
		 */

		/* TABLE 1 */
		page = 0;

		if ((req_fuel_total_1 == last_req_fuel_total_1) &&
			(num_cyls_1 == last_num_cyls_1) &&
			(num_inj_1 == last_num_inj_1) &&
			(num_squirts_1 == last_num_squirts_1) &&
			(divider_1 == last_divider_1))
			goto table2;

		tmp = (float)(num_inj_1)/(float)(ms_data[page][firmware->page_params[page]->divider_offset]);
		req_fuel_per_squirt = ((float)req_fuel_total_1 * 10.0)/tmp;

		if (req_fuel_per_squirt > 255)
			lim_flag = TRUE;
		if (req_fuel_per_squirt < 0)
			lim_flag = TRUE;
		if (num_cyls_1 % num_squirts_1)
			lim_flag = TRUE;

		/* Throw warning if an issue */
		if (lim_flag)
			set_group_color(RED,"interdep_1_ctrl");
		else
		{
			set_group_color(BLACK,"interdep_1_ctrl");
			/* Required Fuel per SQUIRT */
			gtk_spin_button_set_value(GTK_SPIN_BUTTON
					(g_hash_table_lookup(dynamic_widgets,"req_fuel_per_squirt_1_spin")),req_fuel_per_squirt/10.0);


			if (paused_handlers)
				return;

			cfg11.value = ms_data[page][firmware->page_params[page]->cfg11_offset];
			rpmk_offset = firmware->page_params[page]->rpmk_offset;
			if (cfg11.bit.eng_type)
				ms_data[page][rpmk_offset] = (int)(6000.0/((double)num_cyls_1));
			else
				ms_data[page][rpmk_offset] = (int)(12000.0/((double)num_cyls_1));

			dload_val = ms_data[page][rpmk_offset];
			write_ve_const(NULL, page, rpmk_offset, dload_val, FALSE);

			offset = firmware->page_params[page]->reqfuel_offset;
			ms_data[page][offset] = req_fuel_per_squirt;
			write_ve_const(NULL, page, offset, req_fuel_per_squirt, FALSE);
			/* Call handler to empty interdependant hash table */
			g_hash_table_foreach_remove(interdep_vars[page],drain_hashtable,GINT_TO_POINTER(page));

		}

		lim_flag = FALSE;
		/* TABLE 2 */
		table2:
		page = 1;

		/* If nothing changed , jsut break to next check */
		if ((req_fuel_total_2 == last_req_fuel_total_2) &&
			(num_cyls_2 == last_num_cyls_2) &&
			(num_inj_2 == last_num_inj_2) &&
			(num_squirts_2 == last_num_squirts_2) &&
			(divider_2 == last_divider_2))
			return;

		tmp = (float)(num_inj_2)/(float)(ms_data[page][firmware->page_params[page]->divider_offset]);
		req_fuel_per_squirt = ((float)req_fuel_total_2 * 10.0)/tmp;

		if (req_fuel_per_squirt > 255)
			lim_flag = TRUE;
		if (req_fuel_per_squirt < 0)
			lim_flag = TRUE;
		if (num_cyls_2 % num_squirts_2)
			lim_flag = TRUE;

		/* Throw warning if an issue */
		if (lim_flag)
			set_group_color(RED,"interdep_2_ctrl");
		else
		{
			set_group_color(BLACK,"interdep_2_ctrl");

			/* Required Fuel per SQUIRT */
			gtk_spin_button_set_value(GTK_SPIN_BUTTON
					(g_hash_table_lookup(dynamic_widgets,"req_fuel_per_squirt_2_spin")),req_fuel_per_squirt/10.0);

			if (paused_handlers)
				return;

			cfg11.value = ms_data[page][firmware->page_params[page]->cfg11_offset];
			rpmk_offset = firmware->page_params[page]->rpmk_offset;
			if (cfg11.bit.eng_type)
				ms_data[page][rpmk_offset] = (int)(6000.0/((double)num_cyls_2));
			else
				ms_data[page][rpmk_offset] = (int)(12000.0/((double)num_cyls_2));

			dload_val = ms_data[page][rpmk_offset];
			write_ve_const(NULL, page, rpmk_offset, dload_val, FALSE);

			offset = firmware->page_params[page]->reqfuel_offset;
			ms_data[page][offset] = req_fuel_per_squirt;
			write_ve_const(NULL, page, offset, req_fuel_per_squirt, FALSE);
			/* Call handler to empty interdependant hash table */
			g_hash_table_foreach_remove(interdep_vars[page],drain_hashtable,GINT_TO_POINTER(page));

		}


	}// END Dualtable Req fuel checks... */
	else
	{
		/* B&G, MSnS, MSnEDIS Required Fuel Calc
		 *
		 *                              /     num_inj_1     \
		 *   req_fuel_per_squirt * (-------------------------)
		 *                              \ divider*(alternate+1) /
		 * req_fuel_total = ----------------------------------------
		 *				10
		 *
		 * where divider = num_cyls_1/num_squirts_1;
		 *
		 * rearranging to solve for req_fuel_per_squirt...
		 *
		 *                        (req_fuel_total * 10)
		 * req_fuel_per_squirt =  ----------------------
		 *			    /  num_inj_1  \
		 *                         (-------------------)
		 *                          \ divider*(alt+1) /
		 *
		 * 
		 */

		page = 0;

		if ((req_fuel_total_1 == last_req_fuel_total_1) &&
			(num_cyls_1 == last_num_cyls_1) &&
			(num_inj_1 == last_num_inj_1) &&
			(num_squirts_1 == last_num_squirts_1) &&
			(alternate_1 == last_alternate_1) &&
			(divider_1 == last_divider_1))
			return;

		divider = ms_data[page][firmware->page_params[page]->divider_offset];
		alternate = ms_data[page][firmware->page_params[page]->alternate_offset];
		tmp =	((float)(num_inj_1))/((float)divider*(float)(alternate+1));

		/* This is 1/10 the value as the on screen stuff is 1/10th 
		 * for the ms variable,  it gets converted farther down, just 
		 * before download to the MS
		 */
		req_fuel_per_squirt = ((float)req_fuel_total_1*10.0)/tmp;

		if (req_fuel_per_squirt > 255)
			lim_flag = TRUE;
		if (req_fuel_per_squirt < 0)
			lim_flag = TRUE;
		if (num_cyls_1 % num_squirts_1)
			lim_flag = TRUE;

		if (lim_flag)
			set_group_color(RED,"interdep_1_ctrl");
		else
		{
			set_group_color(BLACK,"interdep_1_ctrl");
			/* req-fuel info box  */
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(g_hash_table_lookup(dynamic_widgets,"req_fuel_per_squirt_1_spin")),req_fuel_per_squirt/10.0);
							     

			/* All Tested succeeded, download Required fuel, 
			 * then iterate through the list of offsets of changed
			 * inter-dependant variables, extract the data out of 
			 * the companion array, and send to ECU.  Then free
			 * the offset GList, and clear the array...
			 */

			if (paused_handlers)
				return;
			/* Send rpmk value as it's needed for rpm calc on 
			 * spark firmwares... */
			/* Top is two stroke, botton is four stroke.. */
			page = 0;
			cfg11.value = ms_data[page][firmware->page_params[page]->cfg11_offset];
			rpmk_offset = firmware->page_params[page]->rpmk_offset;
			if (cfg11.bit.eng_type)
				ms_data[page][rpmk_offset] = (int)(6000.0/((double)num_cyls_1));
			else
				ms_data[page][rpmk_offset] = (int)(12000.0/((double)num_cyls_1));
			dload_val = ms_data[page][rpmk_offset];
			write_ve_const(NULL, page, rpmk_offset, dload_val, FALSE);

			/* Send reqd_fuel_per_squirt */
			offset = firmware->page_params[page]->reqfuel_offset;
			ms_data[page][offset] = req_fuel_per_squirt;
			write_ve_const(NULL, page, offset, req_fuel_per_squirt, FALSE);
			g_hash_table_foreach_remove(interdep_vars[page],drain_hashtable,GINT_TO_POINTER(page));
		}
	} // End B&G style Req Fuel check 
	return ;

}


/*!
 \brief initialize_reqd_fuel() initializes the reqd_fuel datastructure for 
 use This will load any previously saved defaults from the global config file.
 \param page (gint) page to create this structure for.
 \returns a pointer to a struct Reqd_Fuel
 */
struct Reqd_Fuel * initialize_reqd_fuel(gint page)
{
	struct Reqd_Fuel *reqd_fuel = NULL;
	ConfigFile * cfgfile;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;

	filename = g_strconcat(HOME(), "/.MegaTunix/config", NULL);

	reqd_fuel = g_new0(struct Reqd_Fuel, 1);
	reqd_fuel->page = page;
	tmpbuf = g_strdup_printf("Req_Fuel_Page_%i",page);
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

	return reqd_fuel;
}
