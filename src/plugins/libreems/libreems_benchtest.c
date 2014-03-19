/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \file src/plugins/libreems/libreems_benchtest.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS specific benchtest helper functions
  \author David Andruczyk
  */

#include <defines.h>
#include <libreems_benchtest.h>
#include <libreems_plugin.h>
#include <gtk/gtk.h>
#include <packet_handlers.h>
#include <serialio.h>

extern gconstpointer *global_data;
static gint id = 0;

/*!
  \brief Validates the on screen data, prompting if necessary and creating
  the packet for the ECU and sending it on it's way
  */
G_MODULE_EXPORT void benchtest_validate_and_run(void)
{
	GAsyncQueue *queue = NULL;
	OutputData *output = NULL;
	LibreEMS_Packet *packet = NULL;
	GTimeVal tval;
	gint res = 0;
	gint tmit_len = 0;
	gint len = 0;
	gint base = 0;
	guint64 clock = 0;
	guint8 byte = 0;
	gint i = 0;
	GByteArray *payload = NULL;
	Bt_Data data;
	gint seq = atomic_sequence();

	ENTER();
	pull_data_from_gui(&data);
	id = (GINT)DATA_GET(global_data,"benchtest_clock_id");
	if (id)
	{
		g_source_remove(id);
		DATA_SET(global_data,"benchtest_clock_id",NULL);
	}
	
	clock = data.events_per_cycle;
	clock *= data.cycles;
	clock *= data.ticks_per_event;
	clock /= 1250;
	payload = g_byte_array_new();
	/* Mode currently fixed at 0x01 */
	byte = BENCH_TEST_INIT;
	g_byte_array_append(payload,&byte,1);
	/* Events per Cycle (8 bit) */
	g_byte_array_append(payload,&data.events_per_cycle,1);
	/* Cycles (16 bit) */
	byte = (data.cycles & 0xff00 ) >> 8;
	g_byte_array_append(payload,&byte,1);
	byte = (data.cycles & 0x00ff );
	g_byte_array_append(payload,&byte,1);
	/* Cycles (16 bit) */
	byte = (data.ticks_per_event & 0xff00 ) >> 8;
	g_byte_array_append(payload,&byte,1);
	byte = (data.ticks_per_event & 0x00ff );
	g_byte_array_append(payload,&byte,1);
	/* Events to fire on (6 8 bit values) */
	for (i=0;i<6;i++)
		g_byte_array_append(payload,&data.events[i],1);
	/* PW Sources (6 16 bit values) */
	for (i=0;i<6;i++)
	{
		byte = (data.pw_sources[i] & 0xff00 ) >> 8;
		g_byte_array_append(payload,&byte,1);
		byte = (data.pw_sources[i] & 0x00ff );
		g_byte_array_append(payload,&byte,1);
	}
	output = initialize_outputdata_f();
	DATA_SET(output->data,"clock",GINT_TO_POINTER((gint)clock));
	DATA_SET(output->data,"start",GINT_TO_POINTER(TRUE));
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_BENCH_TEST_DATA));
	DATA_SET_FULL(output->data,"payload_data_array",payload,g_byte_array_unref);
	queue = g_async_queue_new();
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("benchtest_pkt",output);

	EXIT();
	return;
}


/*!
  \brief Stops the benchtest
  */
G_MODULE_EXPORT void benchtest_stop(void)
{
	GAsyncQueue *queue = NULL;
	OutputData *output = NULL;
	LibreEMS_Packet *packet = NULL;
	GTimeVal tval;
	guint8 byte = 0;
	gint i = 0;
	GByteArray *payload = NULL;
	gint seq = atomic_sequence();

	ENTER();
	DATA_SET(global_data,"benchtest_total",GINT_TO_POINTER(0));
	id = (GINT)DATA_GET(global_data,"benchtest_clock_id");
	if (id)
	{
		g_source_remove(id);
		DATA_SET(global_data,"benchtest_clock_id",NULL);
	}

	gtk_widget_set_sensitive(lookup_widget_f("BTest_params_table"),TRUE);
	gtk_widget_set_sensitive(lookup_widget_f("BTest_start_test_button"),TRUE);
	gtk_widget_set_sensitive(lookup_widget_f("BTest_stop_test_button"),FALSE);
	gtk_widget_set_sensitive(lookup_widget_f("BTest_bump_button"),FALSE);
	gtk_widget_set_sensitive(lookup_widget_f("BTest_bump_entry"),FALSE);
	gtk_label_set_markup(GTK_LABEL(lookup_widget_f("BTest_time_remain_label")),"<b>HH:MM:SS</b>");

	payload = g_byte_array_new();
	/* Mode currently fixed at 0x00 to stop */
	byte = BENCH_TEST_STOP;
	g_byte_array_append(payload,&byte,1);
	output = initialize_outputdata_f();
	DATA_SET(output->data,"stop",GINT_TO_POINTER(TRUE));
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_BENCH_TEST_DATA));
	DATA_SET_FULL(output->data,"payload_data_array",payload,g_byte_array_unref);
	queue = g_async_queue_new();
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("benchtest_pkt",output);
	EXIT();
	return;
}


/*!
  \brief Bumps the benchtest
  */
G_MODULE_EXPORT void benchtest_bump(void)
{
	GAsyncQueue *queue = NULL;
	OutputData *output = NULL;
	LibreEMS_Packet *packet = NULL;
	guint8 byte = 0;
	gint seq = 71;
	guint64 addition = 0;
	guint8 bump = 0;
	GByteArray *payload = NULL;
	Bt_Data data;
	gchar * text = NULL;

	ENTER();
	pull_data_from_gui(&data);

        text = gtk_editable_get_chars(GTK_EDITABLE(lookup_widget_f("BTest_bump_entry")),0,-1);
	bump = (guint16)g_strtod(text,NULL);
	g_free(text);
	addition = data.events_per_cycle;
	addition *= bump;
	addition *= data.ticks_per_event;
	addition /= 1250;

	payload = g_byte_array_new();
	/* Mode currently fixed at 0x02 to bump */
	byte = BENCH_TEST_BUMP;
	g_byte_array_append(payload,&byte,1);
	/* Add the amount of time to bump by */
	g_byte_array_append(payload,&bump,1);
	output = initialize_outputdata_f();
	DATA_SET(output->data,"bump",GINT_TO_POINTER(TRUE));
	DATA_SET(output->data,"clock",GINT_TO_POINTER((gint)addition));
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_BENCH_TEST_DATA));
	DATA_SET_FULL(output->data,"payload_data_array",payload,g_byte_array_unref);
	queue = g_async_queue_new();
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("benchtest_pkt",output);

	EXIT();
	return;
}


/*!
  \brief Pulls the user filled data from the UI and returns it in a 
  nice clean container for the packet assembly part
  \param data is a pointer to a Bt_Data structure
  \returns TRUE on success FALSE otherwise
  */
gboolean pull_data_from_gui(Bt_Data *data)
{
	GtkWidget * widget = NULL;
	gchar *text = NULL;
	g_return_val_if_fail(data,FALSE);

	ENTER();
	widget = lookup_widget_f("BTest_events_per_cycle_entry");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->events_per_cycle = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_cycles_entry");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->cycles = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_time_per_event_entry");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->ticks_per_event = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_events_to_fire_from_entry1");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->events[0] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_events_to_fire_from_entry2");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->events[1] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_events_to_fire_from_entry3");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->events[2] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_events_to_fire_from_entry4");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->events[3] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_events_to_fire_from_entry5");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->events[4] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_events_to_fire_from_entry6");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->events[5] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_pw_source_entry1");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->pw_sources[0] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_pw_source_entry2");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->pw_sources[1] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_pw_source_entry3");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->pw_sources[2] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_pw_source_entry4");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->pw_sources[3] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_pw_source_entry5");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->pw_sources[4] = (GINT)g_strtod(text,NULL);
	g_free(text);

	widget = lookup_widget_f("BTest_pw_source_entry6");
        text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	data->pw_sources[5] = (GINT)g_strtod(text,NULL);
	g_free(text);

	EXIT();
	return TRUE;
}


gboolean benchtest_clock_update_wrapper(gpointer data)
{
	ENTER();
	g_idle_add(benchtest_clock_update,data);
	EXIT();
	return FALSE;
}


gboolean benchtest_clock_update(gpointer data)
{
	static GTimeVal cur;
	static GTimeVal last;
	static GtkWidget *label = NULL;
	gchar *tmpbuf = NULL;
	gint hour = 0;
	gint min = 0;
	gint sec = 0;
	gint msec = 0;
	gint total = 0;

	ENTER();
	if (!label)
		label = lookup_widget_f("BTest_time_remain_label");
	g_return_val_if_fail(label,FALSE);

	last = cur;
	g_get_current_time(&cur);

	/* First run, grey out the main controls */
	total = (GINT)DATA_GET(global_data,"benchtest_total");
	if (total == 0 )
	{
		gtk_widget_set_sensitive(lookup_widget_f("BTest_params_table"),FALSE);
		gtk_widget_set_sensitive(lookup_widget_f("BTest_start_test_button"),FALSE);
		gtk_widget_set_sensitive(lookup_widget_f("BTest_stop_test_button"),TRUE);
		gtk_widget_set_sensitive(lookup_widget_f("BTest_bump_button"),TRUE);
		gtk_widget_set_sensitive(lookup_widget_f("BTest_bump_entry"),TRUE);
		total = (GINT)data;
	}
	else
	{
		gint diff = ((cur.tv_sec-last.tv_sec) * 1000000) + (cur.tv_usec-last.tv_usec); 
		total -= (diff/1000);
		if (total <= 0 )
			total = 0;
	}
	DATA_SET(global_data,"benchtest_total",GINT_TO_POINTER(total));
	/*printf("Total time in msec %i\n",total);*/
	hour = total / 3600000;
	min = (total % 3600000) / 60000;
	sec = ((total % 3600000) % 60000)/1000;
	msec = (((total % 3600000) % 60000) % 1000);
	/*printf("hour %i, min %i, sec %i, msec %i\n",hour,min,sec,msec);*/
	if (total > 0)
	{
		tmpbuf = g_strdup_printf("<b>%.2i:%.2i:%.2i</b>",hour,min,sec);
		gtk_label_set_markup(GTK_LABEL(label),tmpbuf);
		g_free(tmpbuf);
		EXIT();
		return TRUE;
	}
	else
	{
		gtk_widget_set_sensitive(lookup_widget_f("BTest_params_table"),TRUE);
		gtk_widget_set_sensitive(lookup_widget_f("BTest_start_test_button"),TRUE);
		gtk_widget_set_sensitive(lookup_widget_f("BTest_stop_test_button"),FALSE);
		gtk_widget_set_sensitive(lookup_widget_f("BTest_bump_button"),FALSE);
		gtk_widget_set_sensitive(lookup_widget_f("BTest_bump_entry"),FALSE);
		gtk_label_set_markup(GTK_LABEL(label),"<b>HH:MM:SS</b>");
		thread_update_logbar_f("libreems_benchtest_view",NULL,g_strdup_printf(_("Benchtest completed...\n")),FALSE,FALSE);
		EXIT();
		return FALSE;
	}
	EXIT();
	return TRUE;
}
