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
#include <math.h>
#include <jimstim.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

static GdkColor red = { 0, 65535, 0, 0};
static GdkColor black = { 0, 0, 0, 0};
static GtkWidget *(*lookup_widget_f)(const gchar *) = NULL;
static void (*io_cmd_f)(const gchar *,void *) = NULL;
static OutputData *(*initialize_outputdata_f)(void) = NULL;

G_MODULE_EXPORT void plugin_init(gconstpointer global_data)
{
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	   */
	lookup_widget_f = (void *)DATA_GET(global_data,"lookup_widget_f");
	io_cmd_f = (void *)DATA_GET(global_data,"io_cmd_f");
	initialize_outputdata_f = (void *)DATA_GET(global_data,"initialize_outputdata_f");
}

G_MODULE_EXPORT gboolean jimstim_sweep_start(GtkWidget *widget, gpointer data)
{
	static JimStim_Data jsdata;
	gchar *text = NULL;
	gfloat steps = 0.0;
	gfloat newsweep = 0.0;
	gint interval = 0;
	gboolean fault = FALSE;
	gchar * tmpbuf = NULL;
	extern GdkColor red;
	extern GdkColor black;

	/* Get widget ptrs */
	if (!jsdata.start_e)
		jsdata.start_e = lookup_widget_f("JS_start_rpm_entry");
	if (!jsdata.end_e)
		jsdata.end_e = lookup_widget_f("JS_end_rpm_entry");
	if (!jsdata.step_e)
		jsdata.step_e = lookup_widget_f("JS_step_rpm_entry");
	if (!jsdata.sweep_e)
		jsdata.sweep_e = lookup_widget_f("JS_sweep_time_entry");
	if (!jsdata.start_b)
		jsdata.start_b = lookup_widget_f("JS_start_sweep_button");
	if (!jsdata.stop_b)
		jsdata.stop_b = lookup_widget_f("JS_stop_sweep_button");

	if (!jsdata.rpm_e)
		jsdata.rpm_e = lookup_widget_f("JS_commanded_rpm");

	OBJ_SET(jsdata.stop_b,"jsdata", (gpointer)&jsdata);
	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.start_e),0,-1); 
	jsdata.start = (gint)g_strtod(text,NULL);
	g_free(text);

	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.end_e),0,-1); 
	jsdata.end = (gint)g_strtod(text,NULL);
	g_free(text);

	text = gtk_editable_get_chars(GTK_EDITABLE(jsdata.step_e),0,-1); 
	jsdata.step = (gint)g_strtod(text,NULL);
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
	else if (!fault)
		gtk_widget_modify_text(jsdata.start_e,GTK_STATE_NORMAL,&black);
	if ((jsdata.end < 61) || (jsdata.end > 65534))
	{	
		fault = TRUE;
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&red);
	}
	else if (!fault)
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&black);
	if (abs(jsdata.end - jsdata.start) < jsdata.step)
	{	
		fault = TRUE;
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(jsdata.start_e,GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&red);
	}
	else if (!fault)
	{
		gtk_widget_modify_text(jsdata.start_e,GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(jsdata.end_e,GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&black);
	}
	if ((jsdata.step < 1) || (jsdata.step > 65535))
	{
		fault = TRUE;
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&red);
	}
	else if (!fault)
		gtk_widget_modify_text(jsdata.step_e,GTK_STATE_NORMAL,&black);
	if ((jsdata.sweep <= 0) || (jsdata.sweep > 300.0))
	{	
		fault = TRUE;
		gtk_widget_modify_text(jsdata.sweep_e,GTK_STATE_NORMAL,&red);
	}
	else if (!fault)
		gtk_widget_modify_text(jsdata.sweep_e,GTK_STATE_NORMAL,&black);

	if (fault)
	{
		dbg_func(PLUGINS,g_strdup(_("Jimstim parameter issue, please check!\n")));
		return TRUE;
	}
//	stop_tickler(RTV_TICKLER);
	g_usleep(10000);
	steps = abs(jsdata.end-jsdata.start)/jsdata.step;
	interval = (1000*jsdata.sweep)/steps;
	if (interval < 10.0)
	{
		newsweep = (10.0*steps)/1000;
		tmpbuf = g_strdup_printf("%1$.*2$f",newsweep,1);
		gtk_entry_set_text(GTK_ENTRY(jsdata.sweep_e),tmpbuf);
		g_free(tmpbuf);
	}
	/* Clamp interval at 10 ms, max 100 theoretical updates/sec */
	interval = interval > 10.0 ? interval:10.0;
	jsdata.current = jsdata.start;
	jsdata.reset = TRUE;

	gtk_widget_set_sensitive(jsdata.start_e,FALSE);
	gtk_widget_set_sensitive(jsdata.end_e,FALSE);
	gtk_widget_set_sensitive(jsdata.step_e,FALSE);
	gtk_widget_set_sensitive(jsdata.sweep_e,FALSE);
	gtk_widget_set_sensitive(jsdata.start_b,FALSE);
	gtk_widget_set_sensitive(jsdata.stop_b,TRUE);
	gtk_widget_set_sensitive(jsdata.rpm_e,TRUE);
	io_cmd_f("jimstim_interactive",NULL);
	jsdata.sweep_id = g_timeout_add(interval,(GSourceFunc)jimstim_rpm_sweep,(gpointer)&jsdata);

	return TRUE;
}


G_MODULE_EXPORT gboolean jimstim_sweep_end(GtkWidget *widget, gpointer data)
{
	OutputData *output = NULL;
	JimStim_Data *jsdata = NULL;
	jsdata = OBJ_GET(widget,"jsdata");
	if (jsdata)
	{
		g_source_remove(jsdata->sweep_id);
		jsdata->reset = TRUE;
		gtk_widget_set_sensitive(jsdata->start_e,TRUE);
		gtk_widget_set_sensitive(jsdata->end_e,TRUE);
		gtk_widget_set_sensitive(jsdata->step_e,TRUE);
		gtk_widget_set_sensitive(jsdata->sweep_e,TRUE);
		gtk_widget_set_sensitive(jsdata->start_b,TRUE);
		gtk_widget_set_sensitive(jsdata->stop_b,FALSE);
		gtk_widget_set_sensitive(jsdata->rpm_e,FALSE);
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
//	start_tickler(RTV_TICKLER);
	return TRUE;
}

G_MODULE_EXPORT gboolean jimstim_rpm_sweep(JimStim_Data *jsdata)
{
	OutputData *output = NULL;
	gchar *tmpbuf = NULL;
	static gboolean rising = TRUE;

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
	gdk_threads_enter();
	gtk_entry_set_text(GTK_ENTRY(jsdata->rpm_e),tmpbuf);
	gdk_threads_leave();
	g_free(tmpbuf);

	return TRUE;
}
