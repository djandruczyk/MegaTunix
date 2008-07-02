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
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <firmware.h>
#include <gtk/gtk.h>
#include <gui_handlers.h>
#include <math.h>
#include <mode_select.h>
#include <notifications.h>
#include <req_fuel.h>
#include <serialio.h>
#include <threads.h>
#include <unions.h>
#include <widgetmgmt.h>

extern GdkColor red;
extern GdkColor black;
extern gint dbg_lvl;
extern GObject *global_data;


/*!
 \brief req_fuel_change() is called whenever the req_fuel variable from 
 the reqfuel_popup is changed and this recalcualtes things and updated 
 the necesary spinbuttons
 \param widget (GtkWidget *) the spinbutton that changed
 */
void req_fuel_change(GtkWidget *widget)
{
	gfloat tmp1,tmp2;
	Reqd_Fuel *reqd_fuel = NULL;
	if (OBJ_GET(widget,"reqd_fuel"))
		reqd_fuel = (Reqd_Fuel *) OBJ_GET(widget,"reqd_fuel");
	else
	{
		if (dbg_lvl & (REQ_FUEL|CRITICAL))
			dbg_func(g_strdup(__FILE__": req_fuel_change()\n\treqd_fuel data NOT bound to the widget pointer passed, RETURNING...\n"));
		return;
	}

	reqd_fuel->actual_inj_flow = ((double)reqd_fuel->rated_inj_flow *
			sqrt((double)reqd_fuel->actual_pressure / (double)reqd_fuel->rated_pressure));

	if (dbg_lvl & REQ_FUEL)
	{
		dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tRated injector flow is %f lbs/hr\n",reqd_fuel->rated_inj_flow));
		dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tRated fuel pressure is %f bar\n",reqd_fuel->rated_pressure));
		dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tActual fuel pressure is %f bar\n",reqd_fuel->actual_pressure));
		dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tCalculated injector flow rate is %f lbs/hr\n",reqd_fuel->actual_inj_flow));
		dbg_func(g_strdup_printf(__FILE__": req_fuel_change()\n\tTarget AFR is %f lbs/hr\n",reqd_fuel->target_afr));
	}

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
	Reqd_Fuel *reqd_fuel = NULL;

	page = (gint)OBJ_GET(widget,"page");

	if (OBJ_GET(widget,"reqd_fuel"))
		reqd_fuel = (Reqd_Fuel *)OBJ_GET(widget,"reqd_fuel");
	else
	{
		reqd_fuel = initialize_reqd_fuel(page);
		OBJ_SET(widget,"reqd_fuel",reqd_fuel);
	}

	if (reqd_fuel->visible)
		return TRUE;
	else
		reqd_fuel->visible = TRUE;

	popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	reqd_fuel->popup = popup;	
	tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
	reqd_fuel->table_num = (gint)g_ascii_strtod(tmpbuf,NULL);

	tmpbuf = g_strdup_printf("Required Fuel Calculator for Page %i\n",reqd_fuel->page);
	gtk_window_set_title(GTK_WINDOW(popup),tmpbuf);
	g_free(tmpbuf);
	gtk_container_set_border_width(GTK_CONTAINER(popup),10);
	gtk_widget_realize(popup);
	OBJ_SET(popup,"reqd_fuel",reqd_fuel);
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


	/* Engine Displacement */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->disp,1.0,1000,1.0,10.0,0);

	spinner = gtk_spin_button_new(adj,0,0);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",GINT_TO_POINTER(REQ_FUEL_DISP));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Number of Cylinders */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->cyls,1,12,1.0,4.0,0);

	spinner = gtk_spin_button_new(adj,0,0);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",GINT_TO_POINTER(REQ_FUEL_CYLS));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Target AFR */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->target_afr,1.0,100.0,1.0,1.0,0);

	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",GINT_TO_POINTER(REQ_FUEL_AFR));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Rated Injector Flow */
	adj =  (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->rated_inj_flow,10.0,255,0.1,1,0);

	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",
			GINT_TO_POINTER(REQ_FUEL_RATED_INJ_FLOW));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 4, 5, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Rated fuel pressure in bar */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->rated_pressure,0.1,10.0,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",	
			GINT_TO_POINTER(REQ_FUEL_RATED_PRESSURE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 4, 5, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Actual fuel pressure in bar */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->actual_pressure,0.1,10.0,0.1,1.0,0);

	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",	
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

	/* Preliminary Required Fuel Value */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->calcd_reqd_fuel,0.1,25.5,0.1,1.0,0);

	spinner = gtk_spin_button_new(adj,0,1);
	reqd_fuel->calcd_val_spin = spinner;
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
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
	OBJ_SET(button,"reqd_fuel",reqd_fuel);
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(save_reqd_fuel),
			NULL);
	OBJ_SET(button,"reqd_fuel",reqd_fuel);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK (close_popup),
			(gpointer)popup);

	button = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);
	OBJ_SET(button,"reqd_fuel",reqd_fuel);
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
	Reqd_Fuel * reqd_fuel = NULL;
	extern GHashTable *dynamic_widgets;
	ConfigFile *cfgfile;
	gchar *filename = NULL;
	gchar *tmpbuf = NULL;

	reqd_fuel = (Reqd_Fuel *)OBJ_GET(widget,"reqd_fuel");

	tmpbuf = g_strdup_printf("req_fuel_per_cycle_%i_spin",reqd_fuel->table_num);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON
			(g_hash_table_lookup(dynamic_widgets,tmpbuf)),
			reqd_fuel->calcd_reqd_fuel);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("num_cylinders_%i_spin",reqd_fuel->table_num);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON
			(g_hash_table_lookup(dynamic_widgets,tmpbuf)),
			reqd_fuel->cyls);
	g_free(tmpbuf);

	check_req_fuel_limits(reqd_fuel->table_num);

	filename = g_strconcat(HOME(), "/.MegaTunix/config", NULL);
	tmpbuf = g_strdup_printf("Req_Fuel_Page_%i",reqd_fuel->page);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)	/* If it opened nicely  */
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
	Reqd_Fuel *reqd_fuel = NULL;

	reqd_fuel = (Reqd_Fuel *)OBJ_GET(widget,"reqd_fuel");
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
void check_req_fuel_limits(gint table_num)
{
	gfloat tmp = 0.0;
	gfloat rf_per_squirt = 0.0;
	gboolean lim_flag = FALSE;
	gint dload_val = 0;
	gint offset = 0;
	gint canID = 0;
	gint page = -1;
	DataSize size = MTX_U08;
	gint rpmk_offset = 0;
	gint num_squirts = 0;
	gint num_cyls = 0;
	gint num_inj = 0;
	gint divider = 0;
	gint alternate = 0;
	gint last_num_squirts = -1;
	gint last_num_cyls = -1;
	gint last_num_inj = -1;
	gint last_divider= -1;
	gint last_alternate = -1;
	gfloat rf_total = 0.0;
	gfloat last_rf_total = 0.0;
	gchar * g_name = NULL;
	gchar * name = NULL;
	GtkWidget *widget = NULL;
	union config11 cfg11;
	extern gboolean paused_handlers;
	extern GHashTable ** interdep_vars;
	extern GHashTable *dynamic_widgets;
	extern Firmware_Details *firmware;
	canID = firmware->canID;

	/* F&H Dualtable required Fuel calc
	 *
	 *                                  /    num_inj_x  \
	 *                 rf_per_squirt * (-----------------)
	 *                                  \    divider    /
	 * req_fuel_total = -----------------------------------
	 *                                 10
	 *
	 * where divider = num_cyls/num_squirts;
	 *
	 * rearranging to solve for rf_per_squirt...
	 *
	 *                    (req_fuel_total * 10)
	 * rf_per_squirt =  --------------------------
	 *                      /   num_inj_x   \
	 *                     (-----------------)
	 *                      \    divider    /
	 */

	/* B&G, MSnS, MSnEDIS Required Fuel Calc
	 *
	 *                                   /       num_inj         \
	 *                  rf_per_squirt * (-------------------------)
	 *                                   \ divider*(alternate+1) /
	 * req_fuel_total = -------------------------------------------
	 *                                     10
	 *
	 * where divider = num_cyls/num_squirts;
	 *
	 * rearranging to solve for rf_per_squirt...
	 *
	 *                   (req_fuel_total * 10)
	 * rf_per_squirt =  ------------------------
	 *                     /     num_inj     \
	 *                    (-------------------)
	 *                     \ divider*(alt+1) /
	 */

	page = firmware->table_params[table_num]->z_page;
	canID = firmware->canID;

	rf_total = firmware->rf_params[table_num]->req_fuel_total;
	last_rf_total = firmware->rf_params[table_num]->last_req_fuel_total;
	num_cyls = firmware->rf_params[table_num]->num_cyls;
	last_num_cyls = firmware->rf_params[table_num]->last_num_cyls;
	num_inj = firmware->rf_params[table_num]->num_inj;
	last_num_inj = firmware->rf_params[table_num]->last_num_inj;
	num_squirts = firmware->rf_params[table_num]->num_squirts;
	last_num_squirts = firmware->rf_params[table_num]->last_num_squirts;
	divider = firmware->rf_params[table_num]->divider;
	last_divider = firmware->rf_params[table_num]->last_divider;
	alternate = firmware->rf_params[table_num]->alternate;
	last_alternate = firmware->rf_params[table_num]->last_alternate;

	//printf("rf_total %.2f, num_cyls %i, num_inj %i, num_squirts %i, divider %i, alt %i\n",rf_total,num_cyls,num_inj,num_squirts,divider,alternate);
	//printf("l_rf_total %.2f, l_num_cyls %i, l_num_inj %i, l_num_squirts %i, l_divider %i, l_alt %i\n",last_rf_total,last_num_cyls,last_num_inj,last_num_squirts,last_divider,last_alternate);

	if ((rf_total == last_rf_total) &&
			(num_cyls == last_num_cyls) &&
			(num_inj == last_num_inj) &&
			(num_squirts == last_num_squirts) &&
			(alternate == last_alternate) &&
			(divider == last_divider))
		return;

	if (firmware->capabilities & DUALTABLE)
	{
		//printf ("dualtable\n");
		tmp = (float)num_inj/(float)divider;
	}
	else if ((firmware->capabilities & MSNS_E) && (((get_ecu_data(canID,firmware->table_params[table_num]->dtmode_page,firmware->table_params[table_num]->dtmode_offset,size) & 0x10) >> 4) == 1))
	{
		//printf ("msns-E with DT enabled\n");
		tmp = (float)num_inj/(float)divider;
	}
	else	/* B&G style */
	{
		//printf ("B&G\n");
		tmp =	((float)(num_inj))/((float)divider*(float)(alternate+1));
		//printf("num_inj/(divider*(alt+1)) == %i\n",tmp);
	}

	rf_per_squirt = ((float)rf_total * 10.0)/tmp;

	if (rf_per_squirt > 255)
		lim_flag = TRUE;
	if (rf_per_squirt < 0)
		lim_flag = TRUE;
	if (num_cyls % num_squirts)
		lim_flag = TRUE;

	/* Throw warning if an issue */
	g_name = g_strdup_printf("interdep_%i_ctrl",table_num);
	if (lim_flag)
		set_group_color(RED,g_name);
	else
	{
		set_group_color(BLACK,g_name);
		/* Required Fuel per SQUIRT */
		name = g_strdup_printf("req_fuel_per_squirt_%i_spin",table_num);
		widget = g_hash_table_lookup(dynamic_widgets,name);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON
				(widget),rf_per_squirt/10.0);
		g_free(name);

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
		cfg11.value = get_ecu_data(canID,page,firmware->table_params[table_num]->cfg11_offset,size);
		rpmk_offset = firmware->table_params[table_num]->rpmk_offset;
		/* Top is two stroke, botton is four stroke.. */
		if (cfg11.bit.eng_type)
			dload_val = (int)(6000.0/((double)num_cyls));
		else
			dload_val = (int)(12000.0/((double)num_cyls));

		send_to_ecu(canID, page, rpmk_offset+1, MTX_U08, (dload_val &0xff00) >> 8, TRUE);
		send_to_ecu(canID, page, rpmk_offset, MTX_U08, (dload_val& 0x00ff), TRUE);

		offset = firmware->table_params[table_num]->reqfuel_offset;
		send_to_ecu(canID, page, offset, MTX_U08, (gint)rf_per_squirt, TRUE);
		//printf("rf per squirt is offset %i, val %i\n",offset,(gint)rf_per_squirt);
		/* Call handler to empty interdependant hash table */
		g_hash_table_foreach_remove(interdep_vars[page],drain_hashtable,NULL);
	}
	g_free(g_name);

	return ;

}


/*!
 \brief initialize_reqd_fuel() initializes the reqd_fuel datastructure for 
 use This will load any previously saved defaults from the global config file.
 \param page (gint) page to create this structure for.
 \returns a pointer to a Reqd_Fuel
 */
Reqd_Fuel * initialize_reqd_fuel(gint page)
{
	Reqd_Fuel *reqd_fuel = NULL;
	ConfigFile * cfgfile;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;

	filename = g_strconcat(HOME(), "/.MegaTunix/config", NULL);

	reqd_fuel = g_new0(Reqd_Fuel, 1);
	reqd_fuel->page = page;
	tmpbuf = g_strdup_printf("Req_Fuel_Page_%i",page);
	cfgfile = cfg_open_file(filename);
	/* Set defaults */
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
