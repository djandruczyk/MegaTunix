/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/mscommon/req_fuel.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS personality specific Require Fuel handling stuff..
  \author David Andruczyk
  */

#include <config.h>
#include <configfile.h>
#include <datamgmt.h>
#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <gtk/gtk.h>
#include <math.h>
#include <mscommon_comms.h>
#include <mscommon_plugin.h>
#include <req_fuel.h>

extern gconstpointer *global_data;


/*!
 \brief reqd_fuel_change() is called whenever the req_fuel variable from 
 the reqfuel_popup is changed and this recalcualtes things and updates 
 the necesary spinbuttons
 \param widget (GtkWidget *) the spinbutton that changed
 */
G_MODULE_EXPORT void reqd_fuel_change(GtkWidget *widget)
{
	gfloat tmp1,tmp2;
	Reqd_Fuel *reqd_fuel = NULL;
	gfloat limit = 0.0;
	Firmware_Details *firmware = NULL;
	GdkColor red = { 0, 65535, 0, 0};
	GdkColor black = { 0, 0, 0, 0};

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (OBJ_GET(widget,"reqd_fuel"))
		reqd_fuel = (Reqd_Fuel *) OBJ_GET(widget,"reqd_fuel");
	else
	{
		MTXDBG(REQ_FUEL|CRITICAL,_("reqd_fuel data NOT bound to the widget pointer passed, RETURNING...\n"));
		EXIT();
		return;
	}

	if (firmware->capabilities &MS2)
		limit = 65.535;
	else
		limit = 25.5;

	reqd_fuel->actual_inj_flow = ((double)reqd_fuel->rated_inj_flow *
			sqrt((double)reqd_fuel->actual_pressure / (double)reqd_fuel->rated_pressure));

	MTXDBG(REQ_FUEL,_("Rated injector flow is %f lbs/hr\n"),reqd_fuel->rated_inj_flow);
	MTXDBG(REQ_FUEL,_("Rated fuel pressure is %f bar\n"),reqd_fuel->rated_pressure);
	MTXDBG(REQ_FUEL,_("Actual fuel pressure is %f bar\n"),reqd_fuel->actual_pressure);
	MTXDBG(REQ_FUEL,_("Calculated injector flow rate is %f lbs/hr\n"),reqd_fuel->actual_inj_flow);
	MTXDBG(REQ_FUEL,_("Target AFR is %f lbs/hr\n"),reqd_fuel->target_afr);

	tmp1 = 36.0*((double)reqd_fuel->disp)*4.27793;
	tmp2 = ((double) reqd_fuel->cyls) \
	       * ((double)(reqd_fuel->target_afr)) \
	       * ((double)(reqd_fuel->actual_inj_flow));

	reqd_fuel->calcd_reqd_fuel = tmp1/tmp2;
	if (GTK_IS_WIDGET(reqd_fuel->calcd_val_spin))
	{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(reqd_fuel->calcd_val_spin),reqd_fuel->calcd_reqd_fuel);


		if (reqd_fuel->calcd_reqd_fuel > limit)
			gtk_widget_modify_text(GTK_WIDGET(reqd_fuel->calcd_val_spin),
					GTK_STATE_NORMAL,&red);
		else
			gtk_widget_modify_text(GTK_WIDGET(reqd_fuel->calcd_val_spin),
					GTK_STATE_NORMAL,&black);
	}
	EXIT();
	return;
}


/*!
 \brief reqd_fuel_popup() is called when the user requests it.  IT provides
 spinbuttons for cylinders, injectors, flow rates, fuel pressure and so on
 to calculate the amount of fuel needed for 1 cylinder for 1 complete cycle
 \param widget (GtkWidget *) pointer to widget that called this function
 */
G_MODULE_EXPORT gboolean reqd_fuel_popup(GtkWidget * widget)
{

	GtkWidget *button;
	GtkWidget *spinner;
	GtkWidget *sep;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *popup;
	GtkAdjustment *adj;
	gchar * tmpbuf = NULL;
	gfloat limit = 0.0;
	gint table_num = -1;
	gint digits = 1;
	Reqd_Fuel *reqd_fuel = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (OBJ_GET(widget,"table_num"))
		table_num = (gint)strtol((gchar *)OBJ_GET(widget,"table_num"),NULL,10);
	else
	{
		printf(_("Serious Error, table_num not defined for reqfuel calc, contact author!\n"));
		return FALSE;
		EXIT();
	}

	if (OBJ_GET(widget,"reqd_fuel"))
		reqd_fuel = (Reqd_Fuel *)OBJ_GET(widget,"reqd_fuel");
	else
	{
		reqd_fuel = initialize_reqd_fuel(table_num);
		OBJ_SET(widget,"reqd_fuel",reqd_fuel);
	}

	if (reqd_fuel->visible)
	{
		EXIT();
		return TRUE;
	}
	else
		reqd_fuel->visible = TRUE;

	if (firmware->capabilities &MS2)
	{
		limit = 65.535;
		digits = 3;
	}
	else
	{
		limit = 25.5;
		digits = 1;
	}

	popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	reqd_fuel->popup = popup;	
	tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
	reqd_fuel->table_num = (gint)strtol(tmpbuf,NULL,10);

	gtk_window_set_title(GTK_WINDOW(popup),_("Required Fuel Calculator"));
	gtk_container_set_border_width(GTK_CONTAINER(popup),5);
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
	frame = gtk_frame_new(_("Required Fuel parameters"));
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	table = gtk_table_new(4,2,FALSE);
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
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Rated Fuel Pressure (bar)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Actual Fuel Pressure (bar)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	sep = gtk_hseparator_new();
	gtk_table_attach (GTK_TABLE (table), sep, 0, 2, 6, 7,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Preliminary Reqd. Fuel (1 cycle)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 7, 8,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);


	/* Engine Displacement */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->disp,1.0,5000,1.0,10.0,0);

	spinner = gtk_spin_button_new(adj,0,0);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",GINT_TO_POINTER(REQ_FUEL_DISP));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (rf_spin_button_handler),
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
			G_CALLBACK (rf_spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Target AFR */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->target_afr,1.0,100.0,1.0,1.0,0);

	spinner = gtk_spin_button_new(adj,0,digits);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",GINT_TO_POINTER(REQ_FUEL_AFR));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (rf_spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Rated Injector Flow */
	adj =  (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->rated_inj_flow,10.0,1000,0.1,1,0);

	spinner = gtk_spin_button_new(adj,0,digits);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",
			GINT_TO_POINTER(REQ_FUEL_RATED_INJ_FLOW));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (rf_spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Rated fuel pressure in bar */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->rated_pressure,0.1,100.0,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,0,digits);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",	
			GINT_TO_POINTER(REQ_FUEL_RATED_PRESSURE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (rf_spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 4, 5,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Actual fuel pressure in bar */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->actual_pressure,0.1,100.0,0.1,1.0,0);

	spinner = gtk_spin_button_new(adj,0,digits);
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	OBJ_SET(spinner,"handler",	
			GINT_TO_POINTER(REQ_FUEL_ACTUAL_PRESSURE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (rf_spin_button_handler),
			NULL);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 5, 6,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Preliminary Required Fuel Value */
	adj = (GtkAdjustment *) gtk_adjustment_new(
			reqd_fuel->calcd_reqd_fuel,0.1,limit,0.1,1.0,0);

	spinner = gtk_spin_button_new(adj,0,digits);
	reqd_fuel->calcd_val_spin = spinner;
	gtk_widget_set_size_request(spinner,65,-1);
	OBJ_SET(spinner,"reqd_fuel",reqd_fuel);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 7, 8,
			(GtkAttachOptions) (GTK_EXPAND),
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

	EXIT();
	return TRUE;
}


/*!
 \brief save_reqd_fuel() called when the reqfuel popup closes, to save the
 values to a config file
 \param widget is the pointer to the widget that contains a mapping to 
 the Reqd_Fuel datastructure to save
 \param data is unused
 \returns TRUE
 */
G_MODULE_EXPORT gboolean save_reqd_fuel(GtkWidget *widget, gpointer data)
{
	Reqd_Fuel * reqd_fuel = NULL;
	ConfigFile *cfgfile;
	GtkWidget *tmpwidget = NULL;
	gchar *filename = NULL;
	gchar *tmpbuf = NULL;
	const gchar *project = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	reqd_fuel = (Reqd_Fuel *)OBJ_GET(widget,"reqd_fuel");

	tmpbuf = g_strdup_printf("num_cylinders_%i_spin",reqd_fuel->table_num);
	tmpwidget = lookup_widget_f(tmpbuf);
	g_free(tmpbuf);
	if (GTK_IS_WIDGET(tmpwidget))
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(tmpwidget),
				reqd_fuel->cyls);
	else
		firmware->rf_params[reqd_fuel->table_num]->num_cyls = reqd_fuel->cyls;

	tmpbuf = g_strdup_printf("req_fuel_per_cycle_%i_spin",reqd_fuel->table_num);
	tmpwidget = lookup_widget_f(tmpbuf);
	g_free(tmpbuf);
	if (GTK_IS_WIDGET(tmpwidget))

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(tmpwidget),
				reqd_fuel->calcd_reqd_fuel);
	else
	{
		firmware->rf_params[reqd_fuel->table_num]->req_fuel_total = reqd_fuel->calcd_reqd_fuel;
		check_req_fuel_limits(reqd_fuel->table_num);
	}

	project = (const gchar *)DATA_GET(global_data,"project_name");
	if (!project)
		project = DEFAULT_PROJECT;
	filename = g_build_path(PSEP,HOME(),"mtx",project,"config",NULL);
	tmpbuf = g_strdup_printf("Req_Fuel_for_Table_%i",reqd_fuel->table_num);
	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		cfgfile = cfg_new();
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
	}
	g_free(filename);
	g_free(tmpbuf);

	EXIT();
	return TRUE;
}


/*!
 \brief close_popup() does exactly that.
 \param widget is the widget that contains the pointer to the 
 Reqd_Fuel struct.
 \returns TRUE
 */
G_MODULE_EXPORT gboolean close_popup(GtkWidget * widget)
{
	Reqd_Fuel *reqd_fuel = NULL;

	ENTER();
	reqd_fuel = (Reqd_Fuel *)OBJ_GET(widget,"reqd_fuel");
	gtk_widget_destroy(reqd_fuel->popup);
	reqd_fuel->visible = FALSE;
	EXIT();
	return TRUE;
}


/*!
 \brief check_req_fuel_limits() gets called any time reqf_fuel OR ANY of it's
 interdependant variable changes.  This function recalculates values and shows
 error status on the gui if needed and send the new values to the ECU when 
 appropropriate
 \param table_num is the table number to do the calculation for
 */
G_MODULE_EXPORT void check_req_fuel_limits(gint table_num)
{
	gfloat tmp = 0.0;
	gfloat rf_per_squirt = 0.0;
	gboolean lim_flag = FALSE;
	gint canID = 0;
	gint page = -1;
	DataSize size = MTX_U08;
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
	guint8 mask = 0;
	guint8 shift = 0;
	gint limit = 0;
	gint mult = 0;
	gfloat rf_total = 0.0;
	gfloat last_rf_total = 0.0;
	gchar * g_name = NULL;
	gchar * name = NULL;
	GtkWidget *widget = NULL;
	GHashTable ** interdep_vars;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	interdep_vars = (GHashTable **)DATA_GET(global_data,"interdep_vars");
	canID = firmware->canID;

	/* Dualtable required Fuel calc
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

	/* B&G, MSnS, MS2?, MSnEDIS Required Fuel Calc
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

	/* If we'd have a floating point exception abort here,  this may
	   occur in offline mode prior to loading a backup
	   */
	if ((num_cyls == 0) && (num_squirts == 0))
	{
		EXIT();
		return;
	}

	/*
	printf("\n\n\n\n");
	printf("rf_total %.2f, num_cyls %i, num_inj %i, num_squirts %i, divider %i, alt %i\n",rf_total,num_cyls,num_inj,num_squirts,divider,alternate);
	printf("l_rf_total %.2f, l_num_cyls %i, l_num_inj %i, l_num_squirts %i, l_divider %i, l_alt %i\n",last_rf_total,last_num_cyls,last_num_inj,last_num_squirts,last_divider,last_alternate);

	printf("\nReqfuel offset is %i\n",firmware->table_params[table_num]->reqfuel_offset);
	*/

	if ((rf_total == last_rf_total) &&
			(num_cyls == last_num_cyls) &&
			(num_inj == last_num_inj) &&
			(num_squirts == last_num_squirts) &&
			(alternate == last_alternate) &&
			(divider == last_divider))
	{
		/*printf("no reqfuel change\n");*/
		EXIT();
		return;
	}

	if (firmware->capabilities & MS2)
	{
		limit=65535;
		mult=100;
	}
	else
	{
		limit=255;
		mult=1;
	}

	if (firmware->capabilities & MS1_DT)
	{
		/*
		 * printf ("dualtable\n");
		 */
		tmp = (float)num_inj/(float)divider;
	}
	else if (firmware->capabilities & MS1_E)
	{	
		shift = get_bitshift_f(firmware->table_params[table_num]->dtmode_mask);
		if ((ms_get_ecu_data(canID,firmware->table_params[table_num]->dtmode_page,firmware->table_params[table_num]->dtmode_offset,size) & firmware->table_params[table_num]->dtmode_mask) >> shift) 
		{
			/*
			 * printf ("msns-E with DT enabled\n");
			 */
			tmp = (float)num_inj/(float)divider;
		}
		else
		{
			/*
			 * printf("MSnS-E non-DT\n"); 
			 */
			tmp = (float)(num_inj)/((float)(divider)*((float)(alternate)+1.0));
		}
	}
	else	/* B&G style */
	{
		/*
		 * printf ("B&G\n");
		 */
		tmp =	((float)(num_inj))/((float)divider*(float)(alternate+1));
		/*
		 * printf("num_inj/(divider*(alt+1)) == %f\n",tmp);
		 */
	}

	rf_per_squirt = ((float)rf_total * (10.0*mult))/tmp;

	if (rf_per_squirt > limit)
		lim_flag = TRUE;
	if (rf_per_squirt < 0)
		lim_flag = TRUE;
	if (num_cyls % num_squirts)
		lim_flag = TRUE;

	/* Throw warning if an issue */
	g_name = g_strdup_printf("interdep_%i_ctrl",table_num);
	if (lim_flag)
	{
		thread_set_group_color_f(RED,g_name);
		slaves_set_color(RED,g_name);
	}
	else
	{
		gint offset = 0;
		thread_set_group_color_f(BLACK,g_name);
		slaves_set_color(BLACK,g_name);
		/* Required Fuel per SQUIRT */
		name = g_strdup_printf("req_fuel_per_squirt_%i_spin",table_num);
		widget = lookup_widget_f(name);
		if (GTK_IS_WIDGET(widget))
		{
			gtk_spin_button_set_value(GTK_SPIN_BUTTON
					(widget),rf_per_squirt/(float)(10*mult));
		}
		g_free(name);

		/* All Tested succeeded, download Required fuel, 
		 * then iterate through the list of offsets of changed
		 * inter-dependant variables, extract the data out of 
		 * the companion array, and send to ECU.  Then free
		 * the offset GList, and clear the array...
		 */

		if (DATA_GET(global_data,"paused_handlers"))
		{
			EXIT();
			return;
		}

		if (firmware->capabilities & MS2)
		{
			offset = firmware->table_params[table_num]->reqfuel_offset;
			/*printf("Sending %i to ecu to canid %i, page %i, offset %i, size %i\n",(gint)rf_per_squirt,canID,firmware->table_params[table_num]->reqfuel_page, offset, firmware->table_params[table_num]->reqfuel_size);
			 */
			ms_send_to_ecu(canID, firmware->table_params[table_num]->reqfuel_page, offset, firmware->table_params[table_num]->reqfuel_size, (gint)rf_per_squirt, TRUE);
			/*
			 * printf("MS2 reqfuel per squirt, value %i \n",(gint)(rf_per_squirt));
			 */
		}
		else
		{
			/* Send rpmk value as it's needed for rpm calc on 
			 * spark firmwares... */
			gint dload_val = 0;
			guint8 tmpi = ms_get_ecu_data(canID,firmware->table_params[table_num]->stroke_page,firmware->table_params[table_num]->stroke_offset,size);
			mask = firmware->table_params[table_num]->stroke_mask;
			shift = get_bitshift_f(firmware->table_params[table_num]->stroke_mask);
			gint rpmk_offset = firmware->table_params[table_num]->rpmk_offset;
			/* Top is two stroke, botton is four stroke.. */
			if ((tmpi & mask) >> shift)
				dload_val = (int)(6000.0/((double)num_cyls));
			else
				dload_val = (int)(12000.0/((double)num_cyls));

			ms_send_to_ecu(canID, firmware->table_params[table_num]->rpmk_page, rpmk_offset, MTX_U08, (dload_val &0xff00) >> 8, TRUE);
			ms_send_to_ecu(canID, firmware->table_params[table_num]->rpmk_page, rpmk_offset+1, MTX_U08, (dload_val& 0x00ff), TRUE);
			offset = firmware->table_params[table_num]->reqfuel_offset;
			ms_send_to_ecu(canID, firmware->table_params[table_num]->reqfuel_page, offset, MTX_U08, (gint)rf_per_squirt, TRUE);
		}
		/*
		 * printf("rf per squirt is offset %i, val %i\n",offset,(gint)rf_per_squirt);
		 */
		/* Call handler to empty interdependant hash table */
		g_hash_table_foreach_remove(interdep_vars[table_num],drain_hashtable,NULL);
	}
	g_free(g_name);

	EXIT();
	return ;

}


/*!
 \brief initialize_reqd_fuel() initializes the reqd_fuel datastructure for 
 use This will load any previously saved defaults from the global config file.
 \param table_num is the table number to create this structure for.
 \returns a pointer to an initialized Reqd_Fuel structure
 */
G_MODULE_EXPORT Reqd_Fuel * initialize_reqd_fuel(gint table_num)
{
	Reqd_Fuel *reqd_fuel = NULL;
	ConfigFile * cfgfile;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	const gchar * project = NULL;

	ENTER();
	project = (const gchar *)DATA_GET(global_data,"project_name");
	if (!project)
		project = DEFAULT_PROJECT;
	filename = g_build_path(PSEP,HOME(),"mtx",project,"config",NULL);

	reqd_fuel = g_new0(Reqd_Fuel, 1);
	reqd_fuel->table_num = table_num;
	tmpbuf = g_strdup_printf("Req_Fuel_for_Table_%i",table_num);
	/* Set defaults */
	reqd_fuel->visible = FALSE;
	reqd_fuel->disp = 350;
	reqd_fuel->cyls = 8;
	reqd_fuel->rated_inj_flow = 19.0;
	reqd_fuel->actual_inj_flow = 0.0;
	reqd_fuel->rated_pressure = 3.0;
	reqd_fuel->actual_pressure = 3.0;
	reqd_fuel->target_afr = 14.7;

	cfgfile = cfg_open_file(filename);
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
	}
	g_free(tmpbuf);
	g_free(filename);

	EXIT();
	return reqd_fuel;
}



/*!
 \brief drain_hashtable() is called to send all the data from a hashtable to
 the ECU
 \param offset is the offset in ecu_data this value goes to
 \param value is the pointer to OutputData Struct
 \param user_data is unused.
 */
G_MODULE_EXPORT gboolean drain_hashtable(gpointer offset, gpointer value, gpointer user_data)
{
	Deferred_Data *data = (Deferred_Data *)value;

	ENTER();
	g_assert(data);
	/*printf("canID %i, page %i, offset %i, size %i, value %i\n",data->canID,data->page,data->offset,data->size,data->value);*/
	ms_send_to_ecu(data->canID,data->page,data->offset,data->size,data->value,FALSE);
	/* Don't delete it as itgets purged on replace, as well as on hash 
	 * destroy when MTX closes
	 */
	/* called per element from the hash table to drain and send to ECU */
	EXIT();
	return TRUE;
}


/*!
  \brief Handles Required Fuel spinbuton changes and updated the Reqd_Fuel 
  structure as appropriate
  \param widget is a pointer to the widget the user changes
  \param data is unused
  */
G_MODULE_EXPORT gboolean rf_spin_button_handler(GtkWidget *widget, gpointer data)
{
	Reqd_Fuel *reqd_fuel = NULL;
	RfHandler handler;
	gfloat value = 0.0;

	ENTER();
	reqd_fuel = (Reqd_Fuel *)OBJ_GET(widget,"reqd_fuel");
	handler = (RfHandler)(GINT)OBJ_GET(widget,"handler");
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);


	switch (handler)
	{
		case REQ_FUEL_DISP:
			reqd_fuel->disp = (gint)value;
			reqd_fuel_change(widget);
			break;
		case REQ_FUEL_CYLS:
			reqd_fuel->cyls = (gint)value;
			reqd_fuel_change(widget);
			break;
		case REQ_FUEL_RATED_INJ_FLOW:
			reqd_fuel->rated_inj_flow = (gfloat)value;
			reqd_fuel_change(widget);
			break;
		case REQ_FUEL_RATED_PRESSURE:
			reqd_fuel->rated_pressure = (gfloat)value;
			reqd_fuel_change(widget);
			break;
		case REQ_FUEL_ACTUAL_PRESSURE:
			reqd_fuel->actual_pressure = (gfloat)value;
			reqd_fuel_change(widget);
			break;
		case REQ_FUEL_AFR:
			reqd_fuel->target_afr = value;
			reqd_fuel_change(widget);
			break;
	}
	EXIT();
	return TRUE;
}


/*!
 \brief reqfuel_rescale_table() is called to rescale a VEtable based on a
 newly chosen reqfuel variable.
 \param widget is the pointer to the scaler widget that was used. From 
 this widget we extract the table number and other needed data to 
 properly do the rescaling.
 */
G_MODULE_EXPORT void reqfuel_rescale_table(GtkWidget *widget)
{
	gint base = -1;
	gint page = -1;
	gint x_bins = -1;
	gint y_bins = -1;
	gint table_num = -1;
	gint old = 0;
	gint canID = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	GtkWidget *tmpwidget = NULL;
	GtkWidget *label = NULL;
	gchar * tmpbuf = NULL;
	gfloat percentage = 0.0;
	gint mult = 0;
	guint i = 0;
	guint x = 0;
	gchar **vector = NULL;
	guint8 *data = NULL;
	gint raw_lower = 0;
	gint raw_upper = 255;
	gfloat value = 0.0;
	gfloat real_value = 0.0;
	gfloat new_reqfuel = 0.0;
	GdkColor color;
	GdkColor black = { 0, 0, 0, 0};
	gboolean use_color = FALSE;
	Firmware_Details *firmware = NULL;
	GList ***ecu_widgets = NULL;

	ENTER();
	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(GTK_IS_WIDGET(widget));
	if (!OBJ_GET(widget,"applicable_tables"))
	{
		printf(_("applicable tables not defined!!!\n"));
		EXIT();
		return;
	}
	if (!OBJ_GET(widget,"table_num"))
	{
		printf(_("table_num not defined!!!\n"));
		EXIT();
		return;
	}
	tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
	table_num = (gint)g_ascii_strtod(tmpbuf,NULL);

	tmpbuf = (gchar *)OBJ_GET(widget,"data");
	tmpwidget = lookup_widget_f(tmpbuf);
	g_return_if_fail(GTK_IS_WIDGET(tmpwidget));
	/*new_reqfuel = gtk_spin_button_get_value(GTK_SPIN_BUTTON(tmpwidget));*/
	tmpbuf = gtk_editable_get_chars(GTK_EDITABLE(tmpwidget),0,-1);
	new_reqfuel = (gfloat)g_ascii_strtod(g_strdelimit(tmpbuf,",.",'.'),NULL);
	if (new_reqfuel < 0.5)
	{
		EXIT();
		return;
	}
	percentage = firmware->rf_params[table_num]->req_fuel_total/new_reqfuel;

	firmware->rf_params[table_num]->last_req_fuel_total = firmware->rf_params[table_num]->req_fuel_total;
	firmware->rf_params[table_num]->req_fuel_total = new_reqfuel;
	check_req_fuel_limits(table_num);

	tmpbuf = (gchar *)OBJ_GET(widget,"applicable_tables");
	vector = g_strsplit(tmpbuf,",",-1);
	if (!vector)
	{
		EXIT();
		return;
	}

	if  (NULL != (label = lookup_widget_f("info_label")))
		gtk_label_set_text(GTK_LABEL(label),"Rescaling Table, Please wait...");

	for (x=0;x<g_strv_length(vector);x++)
	{
		table_num = (gint)strtol(vector[x],NULL,10);

		size = firmware->table_params[table_num]->z_size;
		mult = get_multiplier_f(size);
		base = firmware->table_params[table_num]->z_base;
		x_bins = firmware->table_params[table_num]->x_bincount;
		y_bins = firmware->table_params[table_num]->y_bincount;
		page = firmware->table_params[table_num]->z_page;
		use_color = firmware->table_params[table_num]->z_use_color;
		raw_lower = firmware->table_params[table_num]->z_raw_lower;
		raw_upper = firmware->table_params[table_num]->z_raw_upper;
		canID = firmware->canID;
		data = g_new0(guint8, x_bins*y_bins*mult);

		for (i=0;i<(x_bins*y_bins);i++)
		{
			offset = i*mult;
			value = ms_get_ecu_data(canID,page,offset,size);
			value *= percentage;
			if (value < raw_lower)
				value = raw_lower;
			if (value > raw_upper)
				value = raw_upper;

			/* What we are doing is doing the 
			 * forware/reverse conversion which
			 * will give us an exact value if the 
			 * user inputs something in
			 * between,  thus we can reset the 
			 * display to a sane value...
			 */
			old = ms_get_ecu_data(canID,page,offset,size);
			ms_set_ecu_data(canID,page,offset,size,value);

			for(int j=0;j<g_list_length(ecu_widgets[page][offset]);i++)
			{
				if (GTK_IS_ENTRY(g_list_nth_data(ecu_widgets[page][offset],j)))
				{
					tmpwidget = g_list_nth_data(ecu_widgets[page][offset],j);
					real_value = convert_after_upload_f(tmpwidget);
					break;
				}
			}
			ms_set_ecu_data(canID,page,offset,size,old);

			tmpbuf = g_strdup_printf("%i",(gint)real_value);
			g_signal_handlers_block_by_func (G_OBJECT(tmpwidget),
					*(void **)(&entry_changed_handler_f),
					NULL);
			gtk_entry_set_text(GTK_ENTRY(tmpwidget),tmpbuf);
			g_signal_handlers_unblock_by_func (G_OBJECT(tmpwidget),
					*(void **)(&entry_changed_handler_f),
					NULL);
			g_free(tmpbuf);

			if (!firmware->chunk_support)
				ms_send_to_ecu(canID, page, offset, size, (gint)value, TRUE);
			if (mult == 1)
				data[offset] = (guint8)value;
			else if (mult == 2)
			{
				data[offset] = ((gint32)value & 0x00ff);
				data[(offset)+1] = ((gint32)value & 0xff00) >> 8;
			}
			else if (mult == 4)
			{
				data[offset] = ((gint32)value & 0x00ff);
				data[(offset)+1] = ((gint32)value & 0xff00) >> 8;
				data[(offset)+2] = ((gint32)value & 0xff0000) >> 16;
				data[(offset)+3] = ((gint32)value & 0xff000000) >> 24;
			}

			gtk_widget_modify_text(GTK_WIDGET(tmpwidget),GTK_STATE_NORMAL,&black);
			if (use_color)
			{
				color = get_colors_from_hue_f(((gfloat)value/raw_upper)*360.0,0.33, 1.0);
				gtk_widget_modify_base(GTK_WIDGET(tmpwidget),GTK_STATE_NORMAL,&color);
			}
		}
		if (firmware->chunk_support)
			ms_chunk_write(canID,page,base,x_bins*y_bins*mult,data);
	}
	g_strfreev(vector);
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	EXIT();
	return;
}

