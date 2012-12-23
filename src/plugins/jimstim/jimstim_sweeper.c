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

/*!
  \file src/plugins/jimstim/jimstim_sweeper.c
  \ingroup JimStimPlugin,Plugins
  \brief  JimStim Plugin speicific code for it's sweeper function
  \author David Andruczyk
  */

#include <args.h>
#include <config.h>
#include <defines.h>
#include <init.h>
#include <math.h>
#include <jimstim_sweeper.h>
#include <jimstim_plugin.h>
#include <debugging.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>



/*!
  \brief initiates sending a continual update to the jimstim in order to make
  it output a moving RPM signal.  This is limited to the serial IO rate
  and the amountof data needed to be sent
  \param widget is the pointer to the start button
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean jimstim_sweep_start(GtkWidget *widget, gpointer data)
{
	static JimStim_Data jsdata;
	gchar *text = NULL;
	gfloat steps = 0.0;
	gfloat newval = 0.0;
	gint interval = 0;
	gboolean fault = FALSE;
	gfloat lower_f = 0.0;
	gfloat upper_f = 0.0;
	gint lower = 0;
	gint upper = 0;
	gchar * tmpbuf = NULL;
	GdkColor red = { 0, 65535, 0, 0};
	GdkColor black = { 0, 0, 0, 0};

	/* Get widget ptrs */
	if (!jsdata.manual_f)
		jsdata.manual_f = lookup_widget_f("JS_manual_rpm_frame");
	if (!jsdata.manual_rpm_e)
		jsdata.manual_rpm_e = lookup_widget_f("JS_rpm_entry");
	if (!jsdata.start_e)
		jsdata.start_e = lookup_widget_f("JS_start_rpm_entry");
	if (!jsdata.end_e)
		jsdata.end_e = lookup_widget_f("JS_end_rpm_entry");
	if (!jsdata.step_e)
		jsdata.step_e = lookup_widget_f("JS_step_rpm_entry");
	if (!jsdata.sweeptime_e)
		jsdata.sweeptime_e = lookup_widget_f("JS_sweep_time_entry");
	if (!jsdata.start_b)
		jsdata.start_b = lookup_widget_f("JS_start_sweep_button");
	if (!jsdata.stop_b)
		jsdata.stop_b = lookup_widget_f("JS_stop_sweep_button");
	if (!jsdata.step_rb)
		jsdata.step_rb = lookup_widget_f("JS_step_radio");
	if (!jsdata.sweeptime_rb)
		jsdata.sweeptime_rb = lookup_widget_f("JS_sweep_radio");
	if (!jsdata.rpm_e)
		jsdata.rpm_e = lookup_widget_f("JS_commanded_rpm");
	if (!jsdata.frame)
		jsdata.frame = lookup_widget_f("JS_basics_frame");

	OBJ_SET(jsdata.stop_b,"jsdata", (gpointer)&jsdata);
	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.manual_rpm_e),0,-1); 
	jsdata.manual_rpm = (GINT)g_strtod(text,NULL);
	g_free(text);

	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.start_e),0,-1); 
	jsdata.start = (GINT)g_strtod(text,NULL);
	g_free(text);

	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.end_e),0,-1); 
	jsdata.end = (GINT)g_strtod(text,NULL);
	g_free(text);

	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.step_e),0,-1); 
	jsdata.step = (GINT)g_strtod(text,NULL);
	g_free(text);

	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.sweeptime_e),0,-1); 
	jsdata.sweep = (gfloat)g_ascii_strtod(g_strdelimit(text,",.",'.'),NULL);
	g_free(text);

	/* Validate data */
	lower = (GINT)strtol((gchar *)OBJ_GET(jsdata.start_e,"raw_lower"),NULL,10);
	upper = (GINT)strtol((gchar *)OBJ_GET(jsdata.start_e,"raw_upper"),NULL,10);
	if ((jsdata.start <= lower) || (jsdata.start >= upper))
	{
		fault = TRUE;
		gtk_widget_modify_text(jsdata.start_e,GTK_STATE_NORMAL,&red);
		update_logbar_f("jimstim_view","warning",g_strdup_printf(_("Start RPM value (%i) is out of range of %i<->%i\n"),jsdata.start,lower,upper),FALSE,FALSE,FALSE);
	}
	else if (!fault)
		gtk_widget_modify_text(jsdata.start_e,GTK_STATE_NORMAL,&black);
	lower = (GINT)strtol((gchar *)OBJ_GET(jsdata.end_e,"raw_lower"),NULL,10);
	upper = (GINT)strtol((gchar *)OBJ_GET(jsdata.end_e,"raw_upper"),NULL,10);
	if ((jsdata.end <= lower) || (jsdata.end >= upper))
	{	
		fault = TRUE;
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&red);
		update_logbar_f("jimstim_view","warning",g_strdup_printf(_("End RPM value of (%i) is out of range of %i<->%i\n"),jsdata.end,lower,upper),FALSE,FALSE,FALSE);
	}
	else if (!fault)
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&black);
	if (abs(jsdata.end - jsdata.start) <= jsdata.step)
	{	
		fault = TRUE;
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(jsdata.start_e,GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&red);
		update_logbar_f("jimstim_view","warning",g_strdup_printf(_("End RPM value (%i) is too close to start RPM (%i), or RPM step (%i) is too large\n"),jsdata.end,jsdata.start,jsdata.step),FALSE,FALSE,FALSE);
	}
	else if (!fault)
	{
		gtk_widget_modify_text(jsdata.start_e,GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&black);
	}
	lower = (GINT)strtol((gchar *)OBJ_GET(jsdata.step_e,"raw_lower"),NULL,10);
	upper = (GINT)strtol((gchar *)OBJ_GET(jsdata.step_e,"raw_upper"),NULL,10);
	if ((jsdata.step <= lower) || (jsdata.step >= upper))
	{
		fault = TRUE;
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&red);
		update_logbar_f("jimstim_view","warning",g_strdup_printf(_("RPM step value (%i) is out of range (%i<->%i\n"),jsdata.step,lower,upper),FALSE,FALSE,FALSE);
	}
	else if (!fault)
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&black);
	lower_f = (gfloat)strtod((gchar *)OBJ_GET(jsdata.sweeptime_e,"raw_lower"),NULL);
	upper_f = (gfloat)strtod((gchar *)OBJ_GET(jsdata.sweeptime_e,"raw_upper"),NULL);
	if ((jsdata.sweep <= lower_f) || (jsdata.sweep >= upper_f))
	{	
		fault = TRUE;
		gtk_widget_modify_text(jsdata.sweeptime_e,GTK_STATE_NORMAL,&red);
		update_logbar_f("jimstim_view","warning",g_strdup_printf(_("RPM sweep time value (%i) is out of range (%f<->%f\n"),jsdata.step,lower_f,upper_f),FALSE,FALSE,FALSE);
	}
	else if (!fault)
		gtk_widget_modify_text(jsdata.sweeptime_e,GTK_STATE_NORMAL,&black);

	if (fault)
	{
		MTXDBG(PLUGINS,_("Jimstim parameter issue, please check!\n"));
		return TRUE;
	}
	stop_tickler_f(RTV_TICKLER);
	g_usleep(10000);
	steps = abs(jsdata.end-jsdata.start)/jsdata.step;
	interval = (1000*jsdata.sweep)/steps;
	if (interval < 10.0)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(jsdata.step_rb)))
		{
			jsdata.sweep = (10.0*steps)/1000;
			tmpbuf = g_strdup_printf("%1$.*2$f",jsdata.sweep,1);
			gtk_entry_set_text(GTK_ENTRY(jsdata.sweeptime_e),tmpbuf);
			g_free(tmpbuf);
		}
		else
		{
			jsdata.step = abs(jsdata.end-jsdata.start)/200;
			tmpbuf = g_strdup_printf("%i",jsdata.step);
			gtk_entry_set_text(GTK_ENTRY(jsdata.step_e),tmpbuf);
			g_free(tmpbuf);
		}
	}
	/* Clamp interval at 10 ms, max 100 theoretical updates/sec */
	interval = interval > 10.0 ? interval:10.0;
	jsdata.current = jsdata.start;
	jsdata.reset = TRUE;

	gtk_widget_set_sensitive(jsdata.manual_f,FALSE);
	gtk_widget_set_sensitive(jsdata.start_e,FALSE);
	gtk_widget_set_sensitive(jsdata.end_e,FALSE);
	gtk_widget_set_sensitive(jsdata.step_e,FALSE);
	gtk_widget_set_sensitive(jsdata.sweeptime_e,FALSE);
	gtk_widget_set_sensitive(jsdata.start_b,FALSE);
	gtk_widget_set_sensitive(jsdata.stop_b,TRUE);
	gtk_widget_set_sensitive(jsdata.rpm_e,TRUE);
	gtk_widget_set_sensitive(jsdata.frame,FALSE);
	g_list_foreach(get_list_f("js_controls"),set_widget_sensitive_f,GINT_TO_POINTER(FALSE));
	newval = (((jsdata.end-jsdata.start)/jsdata.step)*interval*2)/1000.0;
	update_logbar_f("jimstim_view",NULL,g_strdup_printf(_("Sweep Parameters are OK, Enabling sweeper from %i to %i RPM with %i RPM steps for about %.2f seconds per sweep cycle\n"),jsdata.start,jsdata.end,jsdata.step,newval),FALSE,FALSE,FALSE);
	io_cmd_f("jimstim_interactive",NULL);
	jsdata.sweep_id = g_timeout_add(interval,(GSourceFunc)jimstim_rpm_sweep_wrapper,(gpointer)&jsdata);

	return TRUE;
}


/*!
  \brief stops the feed to the jimstim and returns controls to their
  normal state
  \param widget is a pointer to the stop button
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean jimstim_sweep_end(GtkWidget *widget, gpointer data)
{
	OutputData *output = NULL;
	JimStim_Data *jsdata = NULL;
	gchar *tmpbuf = NULL;
	jsdata = (JimStim_Data *)OBJ_GET(widget,"jsdata");
	if (jsdata)
	{
		if (jsdata->sweep_id)
			g_source_remove(jsdata->sweep_id);
		jsdata->reset = TRUE;
		gtk_widget_set_sensitive(jsdata->manual_f,TRUE);
		gtk_widget_set_sensitive(jsdata->start_e,TRUE);
		gtk_widget_set_sensitive(jsdata->end_e,TRUE);
		gtk_widget_set_sensitive(jsdata->step_e,TRUE);
		gtk_widget_set_sensitive(jsdata->sweeptime_e,TRUE);
		gtk_widget_set_sensitive(jsdata->start_b,TRUE);
		gtk_widget_set_sensitive(jsdata->stop_b,FALSE);
		gtk_widget_set_sensitive(jsdata->rpm_e,FALSE);
		gtk_widget_set_sensitive(jsdata->frame,TRUE);
		g_list_foreach(get_list_f("js_controls"),set_widget_sensitive_f,GINT_TO_POINTER(TRUE));
		update_logbar_f("jimstim_view",NULL,g_strdup(_("Sweeper stopped\n")),FALSE,FALSE,FALSE);
	}
	/* Send 65535 to disable dynamic mode */
	gtk_entry_set_text(GTK_ENTRY(jsdata->rpm_e),"");
	/* Highbyte of rpm */
	output = initialize_outputdata_f();
	DATA_SET(output->data,"mode",GINT_TO_POINTER(MTX_CMD_WRITE));
	DATA_SET(output->data,"value",GINT_TO_POINTER(255));
	io_cmd_f("jimstim_interactive_write",output);
	/* Lowbyte of rpm */
	output = initialize_outputdata_f();
	DATA_SET(output->data,"mode",GINT_TO_POINTER(MTX_CMD_WRITE));
	DATA_SET(output->data,"value",GINT_TO_POINTER(255));
	io_cmd_f("jimstim_interactive_write",output);
	start_tickler_f(RTV_TICKLER);
	/* Reset RPM to original value, however we need to do it TWICE,
	   once to something DIFFERENT than the original as Mtx optimizes
	   out duplicate sets of the same value,, so we set it to orig+1, then
	   original value to get around this optimization.
	 */ 
	tmpbuf = g_strdup_printf("%i",jsdata->manual_rpm+1);
	gtk_entry_set_text(GTK_ENTRY(jsdata->manual_rpm_e),tmpbuf);
	g_signal_emit_by_name(jsdata->manual_rpm_e,"activate");
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%i",jsdata->manual_rpm);
	gtk_entry_set_text(GTK_ENTRY(jsdata->manual_rpm_e),tmpbuf);
	g_signal_emit_by_name(jsdata->manual_rpm_e,"activate");
	g_free(tmpbuf);
	return TRUE;
}


/*
 * \brief wrapper for jimstim_sweep
 * */
G_MODULE_EXPORT gboolean jimstim_rpm_sweep_wrapper(JimStim_Data *jsdata)
{
	g_idle_add((GSourceFunc)jimstim_rpm_sweep,jsdata);
	return FALSE;
}


/*!
  \brief This is the timeout function that sends the updates to the jimstim
  \param jsdata is a pointer to the JimStim_Data structure
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean jimstim_rpm_sweep(JimStim_Data *jsdata)
{
	OutputData *output = NULL;
	gchar *tmpbuf = NULL;
	static gboolean rising = TRUE;

	g_return_val_if_fail(jsdata,FALSE);

	if (jsdata->reset)
	{
		if (jsdata->end > jsdata->start)
			rising = TRUE;
		else
			rising = FALSE;
		jsdata->reset = FALSE;
	}
	else
	{
		/* Normal start < End  */
		if (jsdata->end > jsdata->start)
		{
			if (jsdata->current >= jsdata->end)
				rising = FALSE;
			if (jsdata->current <= jsdata->start)
				rising = TRUE;
		}
		/* Odd End > start  */
		if (jsdata->end < jsdata->start)
		{
			if (jsdata->current >= jsdata->start)
				rising = FALSE;
			if (jsdata->current <= jsdata->end)
				rising = TRUE;
		}
	}

	if (rising)
		jsdata->current += jsdata->step;
	else
		jsdata->current -= jsdata->step;

	/* Highbyte of rpm */
	output = initialize_outputdata_f();
	DATA_SET(output->data,"mode",GINT_TO_POINTER(MTX_CMD_WRITE));
	DATA_SET(output->data,"value",GINT_TO_POINTER((jsdata->current &0xff00) >> 8));
	io_cmd_f("jimstim_interactive_write",output);
	/* Lowbyte of rpm */
	output = initialize_outputdata_f();
	DATA_SET(output->data,"mode",GINT_TO_POINTER(MTX_CMD_WRITE));
	DATA_SET(output->data,"value",GINT_TO_POINTER((jsdata->current &0x00ff)));
	io_cmd_f("jimstim_interactive_write",output);
	tmpbuf = g_strdup_printf("%i",jsdata->current);
	gtk_entry_set_text(GTK_ENTRY(jsdata->rpm_e),tmpbuf);
	g_free(tmpbuf);

	return TRUE;
}


/*!
  \brief init function for the jimstim tab, if we are in network mode this
  plugin doesn't work as it isn't supported in the MTXSocket implementation
  \param widget is the pointer to the jimstim sweeper tab
  */
G_MODULE_EXPORT void jimstim_sweeper_init(GtkWidget *widget)
{
	CmdLineArgs *args = NULL;
	extern gconstpointer *global_data;
	args = (CmdLineArgs *)DATA_GET(global_data,"args");

	/* If a network mode slave, we DO NOT allow sweeping as it
	   uses a non STD API that doesn't mesh with the socket stuff
	   and requires things that are NOT implemented (passing msgs
	   UP to the master/host), so it's safer and cleaner to lockout
	   the ugliness
	   */
	if (args)
		if (args->network_mode)
			gtk_widget_set_sensitive(widget,FALSE);
}
