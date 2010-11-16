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
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <init.h>
#include <jimstim.h>
#include <string.h>
#include <timeout_handlers.h>
#include <threads.h>
#include <widgetmgmt.h>


extern gconstpointer *global_data;


EXPORT gboolean jimstim_sweep_start(GtkWidget *widget, gpointer data)
{
	static JimStim_Data jsdata;
	gchar *text = NULL;
	gfloat steps;
	gint interval;
	gboolean fault = FALSE;
	extern GdkColor red;
	extern GdkColor black;
	
	/* Get widget ptrs */
	if (!jsdata.start_e)
		jsdata.start_e = lookup_widget("JS_start_rpm_entry");
	if (!jsdata.end_e)
		jsdata.end_e = lookup_widget("JS_end_rpm_entry");
	if (!jsdata.step_e)
		jsdata.step_e = lookup_widget("JS_step_rpm_entry");
	if (!jsdata.sweep_e)
		jsdata.sweep_e = lookup_widget("JS_sweep_time_entry");
	if (!jsdata.start_b)
		jsdata.start_b = lookup_widget("JS_start_sweep_button");
	if (!jsdata.stop_b)
		jsdata.stop_b = lookup_widget("JS_stop_sweep_button");

	OBJ_SET(jsdata.stop_b,"jsdata", (gpointer)&jsdata);
	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.start_e),0,-1); 
	jsdata.start = (gint)strtol(text,NULL,10);
	g_free(text);

	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.end_e),0,-1); 
	jsdata.end = (gint)strtol(text,NULL,10);
	g_free(text);

	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.step_e),0,-1); 
	jsdata.step = (gint)strtol(text,NULL,10);
	g_free(text);

	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.sweep_e),0,-1); 
	jsdata.sweep = (gfloat)g_ascii_strtod(g_strdelimit(text,",.",'.'),NULL);
	g_free(text);

	/* Validate data */
	if ((jsdata.start < 60) || (jsdata.start > 65534))
	{
		fault = TRUE;
		gtk_widget_modify_text(jsdata.start_e,GTK_STATE_NORMAL,&red);
	}
	else
		gtk_widget_modify_text(jsdata.start_e,GTK_STATE_NORMAL,&black);
	if ((jsdata.end < 61) || (jsdata.end > 65534))
	{	
		fault = TRUE;
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&red);
	}
	else
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&black);
	if ((jsdata.step < 1) || (jsdata.step > 1000))
	{
		fault = TRUE;
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&red);
	}
	else
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&black);
	if ((jsdata.sweep < 0.1) || (jsdata.sweep > 300.0))
	{	
		fault = TRUE;
		gtk_widget_modify_text(jsdata.sweep_e,GTK_STATE_NORMAL,&red);
	}
	else
		gtk_widget_modify_text(jsdata.sweep_e,GTK_STATE_NORMAL,&black);

	if (fault)
	{
		printf("Jimstim parameter issue, please check!\n");
		return TRUE;
	}
	stop_tickler(RTV_TICKLER);
	g_usleep(10000);
	steps = (jsdata.end-jsdata.start)/jsdata.step;
	interval = (gint)(1000*jsdata.sweep)/steps;
	interval = interval > 3 ? interval:3;
	jsdata.current = jsdata.start;
	
	gtk_widget_set_sensitive(jsdata.start_e,FALSE);
	gtk_widget_set_sensitive(jsdata.end_e,FALSE);
	gtk_widget_set_sensitive(jsdata.step_e,FALSE);
	gtk_widget_set_sensitive(jsdata.sweep_e,FALSE);
	gtk_widget_set_sensitive(jsdata.start_b,FALSE);
	gtk_widget_set_sensitive(jsdata.stop_b,TRUE);
	//io_cmd("jimstim_interactive",NULL);
	jsdata.sweep_id = g_timeout_add(interval,(GSourceFunc)jimstim_rpm_sweep,(gpointer)&jsdata);

	return TRUE;
}


EXPORT gboolean jimstim_sweep_end(GtkWidget *widget, gpointer data)
{
	OutputData *output = NULL;
	JimStim_Data *jsdata = NULL;
	jsdata = OBJ_GET(widget,"jsdata");
	if (jsdata)
	{
		g_source_remove(jsdata->sweep_id);
		gtk_widget_set_sensitive(jsdata->start_e,TRUE);
		gtk_widget_set_sensitive(jsdata->end_e,TRUE);
		gtk_widget_set_sensitive(jsdata->step_e,TRUE);
		gtk_widget_set_sensitive(jsdata->sweep_e,TRUE);
		gtk_widget_set_sensitive(jsdata->start_b,TRUE);
		gtk_widget_set_sensitive(jsdata->stop_b,FALSE);
	}
	/* Send 65535 to disable dynamic mode */
	/* Highbyte of rpm */
	output = initialize_outputdata();
	DATA_SET(output->data,"mode",GINT_TO_POINTER(MTX_CMD_WRITE));
	DATA_SET(output->data,"value",GINT_TO_POINTER(255));
	io_cmd("jimstim_interactive_write",output);
	/* Lowbyte of rpm */
	output = initialize_outputdata();
	DATA_SET(output->data,"mode",GINT_TO_POINTER(MTX_CMD_WRITE));
	DATA_SET(output->data,"value",GINT_TO_POINTER(255));
	io_cmd("jimstim_interactive_write",output);
	start_tickler(RTV_TICKLER);
	return TRUE;
}

gboolean jimstim_rpm_sweep(JimStim_Data *jsdata)
{
	OutputData *output = NULL;
	static gboolean rising = TRUE;

	if (jsdata->current >= jsdata->end)
		rising = FALSE;
	if (jsdata->current <= jsdata->start)
		rising = TRUE;

	if (rising)
		jsdata->current += jsdata->step;
	else
		jsdata->current -= jsdata->step;

	/* Highbyte of rpm */
	output = initialize_outputdata();
	DATA_SET(output->data,"mode",GINT_TO_POINTER(MTX_CMD_WRITE));
	DATA_SET(output->data,"value",GINT_TO_POINTER((jsdata->current &0xff00) >> 8));
	io_cmd("jimstim_interactive_write",output);
	/* Lowbyte of rpm */
	output = initialize_outputdata();
	DATA_SET(output->data,"mode",GINT_TO_POINTER(MTX_CMD_WRITE));
	DATA_SET(output->data,"value",GINT_TO_POINTER((jsdata->current &0x00ff)));
	io_cmd("jimstim_interactive_write",output);

	return TRUE;
}
