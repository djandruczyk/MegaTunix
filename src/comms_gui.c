/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*! @file src/comms_gui.c
 *
 * @brief ...
 *
 *
 */

#include <serialio.h>
#include <widgetmgmt.h>

extern GdkColor black;
extern GdkColor white;
extern GdkColor red;
extern GdkColor black;

/*!
 \brief reset_errcounts() resets the error counters
 \param widget is unused
 \returns TRUE
 */
G_MODULE_EXPORT gboolean reset_errcounts(GtkWidget *widget)
{
	extern gconstpointer *global_data;
	Serial_Params *serial_params = NULL;
	serial_params = DATA_GET(global_data,"serial_params");

	DATA_SET(global_data,"ve_goodread_count",GINT_TO_POINTER(0));
	DATA_SET(global_data,"rt_goodread_count",GINT_TO_POINTER(0));
	DATA_SET(global_data,"reset_count",GINT_TO_POINTER(0));
	return TRUE;
}


/*!
 \brief update_errcounts() updates the text entries on the gui with the 
 current status, error and I/O counters
 \returns TRUE
 */
G_MODULE_EXPORT gboolean update_errcounts(void)
{
	static gboolean pf_red = FALSE;
	static GAsyncQueue *pf_dispatch_queue = NULL;
        static GAsyncQueue *gui_dispatch_queue = NULL;
	static GCond *statuscounts_cond = NULL;
	static GMutex *statuscounts_mutex = NULL;
	gchar *tmpbuf = NULL;
	gint tmp = 0;
	GtkWidget * widget = NULL;
	Serial_Params *serial_params = NULL;
	extern gconstpointer *global_data;

	serial_params = DATA_GET(global_data,"serial_params");
	if (!pf_dispatch_queue)
		pf_dispatch_queue = DATA_GET(global_data,"pf_dispatch_queue");
	if (!gui_dispatch_queue)
		gui_dispatch_queue = DATA_GET(global_data,"gui_dispatch_queue");
	if (!statuscounts_cond)
		statuscounts_cond = DATA_GET(global_data,"statuscounts_cond");
	if (!statuscounts_mutex)
		statuscounts_mutex = DATA_GET(global_data,"statuscounts_mutex");
	
	g_return_val_if_fail(pf_dispatch_queue,FALSE);
	g_return_val_if_fail(gui_dispatch_queue,FALSE);
	g_return_val_if_fail(statuscounts_cond,FALSE);
	g_return_val_if_fail(statuscounts_mutex,FALSE);

	if (DATA_GET(global_data,"leaving"))
	{
		g_mutex_lock(statuscounts_mutex);
		g_cond_signal(statuscounts_cond);
		g_mutex_unlock(statuscounts_mutex);
		return TRUE;
	}

	gdk_threads_enter();
	tmpbuf = g_strdup_printf("%i",(GINT)DATA_GET(global_data,"ve_goodread_count"));
	widget = lookup_widget("runtime_good_ve_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("comms_vecount_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",(GINT)DATA_GET(global_data,"rt_goodread_count"));
	widget = lookup_widget("comms_rtcount_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = lookup_widget("runtime_good_rt_read_entry");
	if (GTK_IS_ENTRY(widget))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",(GINT)DATA_GET(global_data,"reset_count"));
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

	g_mutex_lock(statuscounts_mutex);
	g_cond_signal(statuscounts_cond);
	g_mutex_unlock(statuscounts_mutex);
	return TRUE;
}
