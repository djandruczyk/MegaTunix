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
	GAsyncQueue *queue = NULL;
	OutputData *output = NULL;
	FreeEMS_Packet *packet = NULL;
	GTimeVal tval;
	guint8 *buf = NULL;
	gint res = 0;
	gint tmit_len = 0;
	gint len = 0;
	gint base = 0;
	guint8 sum = 0;
	gint i = 0;
	gint seq = 69;
	Bt_Data data;


	pull_data_from_gui(&data);
	buf = g_new0(guint8,(BENCH_TEST_PKT_LEN-4));
	/* Mode currently fixed at 0x01 */
	buf[0] = 1;
	/* Events per Cycle (8 bit) */
	buf[1] = data.events_per_cycle;
	/* Cycles (16 bit) */
	buf[2] = (data.cycles & 0xff00 ) >> 8;
	buf[3] = (data.cycles & 0x00ff );
	/* Cycles (16 bit) */
	buf[4] = (data.ticks_per_event & 0xff00 ) >> 8;
	buf[5] = (data.ticks_per_event & 0x00ff );
	/* Events to fire on (6 8 bit values) */
	for (i=0;i<6;i++)
		buf[6+i] = data.events[i];
	/* PW Sources (6 16 bit values) */
	base=12;
	for (i=0;i<6;i++)
	{
		buf[base] = (data.pw_sources[i] & 0xff00 ) >> 8;
		base++;
		buf[base] = (data.pw_sources[i] & 0x00ff );
		base++;
	}
	output = initialize_outputdata_f();
	DATA_SET(output->data,"sequence_num",GINT_TO_POINTER(seq));
	DATA_SET(output->data,"payload_id",GINT_TO_POINTER(REQUEST_SET_BENCH_TEST_DATA));
	DATA_SET(output->data,"data",buf);
	DATA_SET(output->data,"data_length",GINT_TO_POINTER(BENCH_TEST_PKT_LEN-4));
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
