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

/*!
  \file src/plugins/freeems/freeems_benchtest.c
  \ingroup FreeEMSPlugin,Plugins
  \brief FreeEMS specific benchtest helper functions
  \author David Andruczyk
  */

#include <defines.h>
#include <freeems_benchtest.h>
#include <freeems_plugin.h>
#include <gtk/gtk.h>
#include <packet_handlers.h>
#include <serialio.h>

extern gconstpointer *global_data;

/*!
  \brief Validates the on screen data, prompting if necessary and creating
  the packet for the ECU and sending it on it's way
  */
G_MODULE_EXPORT void benchtest_validate_and_run(void)
{
	static gint id = 0;
	GAsyncQueue *queue = NULL;
	OutputData *output = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	gint res = 0;
	gint tmit_len = 0;
	gint len = 0;
	gint base = 0;
	guint8 byte = 0;
	gint i = 0;
	gint seq = 69;
	guint8 *buf = NULL;
	GByteArray *payload;
	Bt_Data data;

	pull_data_from_gui(&data);
	if (id)
		g_source_remove(id);
	
	thread_update_logbar_f("freeems_benchtest_view",NULL,g_strdup_printf(_("Total benchtest runtime should be %.1f seconds...\n"),(data.events_per_cycle*data.cycles*data.ticks_per_event)/1250000.0),FALSE,FALSE);
	id = g_timeout_add(500,benchtest_clock_update,GINT_TO_POINTER((GINT)((data.events_per_cycle*data.cycles*data.ticks_per_event)/1250)));
	payload = g_byte_array_new();
	/* Mode currently fixed at 0x01 */
	byte = 1;
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
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_BENCH_TEST_DATA));
	DATA_SET_FULL(output->data,"payload_data_array",payload,g_byte_array_unref);
	queue = g_async_queue_new();
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("benchtest_pkt",output);

	return;
}


/*!
  \brief Stops the benchtest
  */
G_MODULE_EXPORT void benchtest_stop(void)
{
	GAsyncQueue *queue = NULL;
	OutputData *output = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	gint res = 0;
	gint tmit_len = 0;
	gint len = 0;
	gint base = 0;
	guint8 byte = 0;
	gint i = 0;
	gint seq = 70;
	guint8 *buf = NULL;
	GByteArray *payload;
	Bt_Data data;

	payload = g_byte_array_new();
	/* Mode currently fixed at 0x00 to stop */
	byte = 0;
	g_byte_array_append(payload,&byte,1);
	output = initialize_outputdata_f();
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_BENCH_TEST_DATA));
	DATA_SET_FULL(output->data,"payload_data_array",payload,g_byte_array_unref);
	queue = g_async_queue_new();
	register_packet_queue(SEQUENCE_NUM,queue,seq);
	DATA_SET(output->data,"queue",queue);
	io_cmd_f("benchtest_pkt",output);

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

	return TRUE;
}


gboolean benchtest_clock_update(gpointer data)
{
	static gint total = 0;
	static GTimeVal cur;
	static GTimeVal last;
	static GtkWidget *label = NULL;
	gchar *tmpbuf = NULL;
	gint hour = 0;
	gint min = 0;
	gint sec = 0;
	gint msec = 0;
	gint diff = 0;

	if (!label)
		label = lookup_widget_f("BTest_time_remain_label");
	g_return_val_if_fail(label,FALSE);

	last = cur;
	g_get_current_time(&cur);

	if (total == 0 )
		total = (GINT)data;
	else
	{
		diff = ((cur.tv_sec-last.tv_sec) * 1000000) + (cur.tv_usec-last.tv_usec); 
		total -= (diff/1000);
		if (total <= 0 )
			total = 0;
	}
//	printf("Total time in msec %i\n",total);
	hour = total / 3600000;
	min = (total % 3600000) / 60000;
	sec = ((total % 3600000) % 60000)/1000;
	msec = (((total % 3600000) % 60000) % 1000);
//	printf("hour %i, min %i, sec %i, msec %i\n",hour,min,sec,msec);
	if (total > 0)
	{
		tmpbuf = g_strdup_printf("%.2i:%.2i:%.2i",hour,min,sec);
		gtk_label_set_text(GTK_LABEL(label),tmpbuf);
		g_free(tmpbuf);
		return TRUE;
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(label),"HH:MM:SS");
		return FALSE;
	}
}
