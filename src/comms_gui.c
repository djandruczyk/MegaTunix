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
GdkColor white = { 0, 65535, 65535, 65535 };
extern GObject *global_data;


/*!
 \brief reset_errcounts() resets the error counters
 \param widget (GtkWidget *) unused
 \returns TRUE
 */
EXPORT gboolean reset_errcounts(GtkWidget *widget)
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
gboolean update_errcounts()
{
	gchar *tmpbuf = NULL;
	extern GHashTable *dynamic_widgets;
	GtkWidget * widget = NULL;
	extern volatile gboolean leaving;
	
	if (leaving)
		return TRUE;

	tmpbuf = g_strdup_printf("%i",ms_ve_goodread_count);
	widget = g_hash_table_lookup(dynamic_widgets,"runtime_good_ve_entry");
	if (widget)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = g_hash_table_lookup(dynamic_widgets,"comms_vecount_entry");
	if (widget)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",ms_goodread_count);
	widget = g_hash_table_lookup(dynamic_widgets,"comms_rtcount_entry");
	if (widget)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = g_hash_table_lookup(dynamic_widgets,"runtime_good_rt_read_entry");
	if (widget)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",ms_reset_count);
	widget = g_hash_table_lookup(dynamic_widgets,"comms_reset_entry");
	if (widget)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = g_hash_table_lookup(dynamic_widgets,"runtime_hardreset_entry");
	if (widget)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",serial_params->errcount);
	widget = g_hash_table_lookup(dynamic_widgets,"comms_sioerr_entry");
	if (widget)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	widget = g_hash_table_lookup(dynamic_widgets,"runtime_sioerr_entry");
	if (widget)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	return TRUE;
}
