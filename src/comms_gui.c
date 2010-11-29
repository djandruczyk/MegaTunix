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

#include <comms_gui.h>
#include <config.h>
#include <defines.h>
#include <enums.h>
#include <gui_handlers.h>
#include <serialio.h>
#include <stdio.h>
#include <string.h>
#include <tabloader.h>
#include <unistd.h>
#include <widgetmgmt.h>

extern gint read_wait_time;
extern gint ms_reset_count;
extern gint ms_goodread_count;
extern gint ms_ve_goodread_count;
extern GdkColor black;
extern Serial_Params *serial_params;
extern GdkColor white;
extern GdkColor red;
extern GdkColor black;


/*!
 \brief reset_errcounts() resets the error counters
 \param widget (GtkWidget *) unused
 \returns TRUE
 */
G_MODULE_EXPORT gboolean reset_errcounts(GtkWidget *widget)
{
	ms_ve_goodread_count = 0;
	ms_goodread_count = 0;
	ms_reset_count = 0;
	serial_params->errcount = 0;
	return TRUE;
}


/*!
 \brief update_errcounts() updates the text entries on the gui with the 
 current statistical error and i/O counters
 \returns TRUE
 */
G_MODULE_EXPORT gboolean update_errcounts(void)
{
	gchar *tmpbuf = NULL;
	gint tmp = 0;
	GtkWidget * widget = NULL;
	static gboolean pf_red = FALSE;
	extern volatile gboolean leaving;
	extern GAsyncQueue *pf_dispatch_queue;
        extern GAsyncQueue *gui_dispatch_queue;
	extern GCond *statuscounts_cond;
	
	if (leaving)
	{
		g_cond_signal(statuscounts_cond);
		return TRUE;
	}

	gdk_threads_enter();
	tmpbuf = g_strdup_printf("%i",ms_ve_goodread_count);
	widget = lookup_widget("runtime_good_ve_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("comms_vecount_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",ms_goodread_count);
	widget = lookup_widget("comms_rtcount_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("runtime_good_rt_read_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",ms_reset_count);
	widget = lookup_widget("comms_reset_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("runtime_hardreset_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",serial_params->errcount);
	widget = lookup_widget("comms_sioerr_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("runtime_sioerr_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);
	tmp = g_async_queue_length(pf_dispatch_queue);
	tmpbuf = g_strdup_printf("%i",tmp);
	widget = lookup_widget("comms_pf_queue_entry");
	if (GTK_IS_ENTRY(widget))
	{
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
		if ((!pf_red) && (tmp > 10))
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&red);
		if ((pf_red) && ( tmp <3))
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	}
	widget = lookup_widget("runtime_pf_queue_entry");
	if (GTK_IS_ENTRY(widget))
	{
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
		if ((!pf_red) && (tmp > 10))
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&red);
		if ((pf_red) && ( tmp <3))
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	}
	if (tmp > 10)
		pf_red = TRUE;
	if (tmp < 3)
		pf_red = FALSE;
	g_free(tmpbuf);
	tmp = g_async_queue_length(gui_dispatch_queue);
	tmpbuf = g_strdup_printf("%i",tmp);
	widget = lookup_widget("comms_gui_queue_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("runtime_gui_queue_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);
	gdk_threads_leave();

	g_cond_signal(statuscounts_cond);
	return TRUE;
}
