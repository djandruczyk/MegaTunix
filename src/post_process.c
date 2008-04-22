/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <post_process.h>

static GArray * raw_memory_data;
extern gint dbg_lvl;
extern GObject *global_data;


/*!
 \brief post_process_raw_memory() handles processing the data returned by the
 ecu for raw memory readback and feeds it into an array for display
 \param input (void *) pointer to the input stream
 \param offset (gint) offset of the datablock received (more correctly a page)
 */
void post_process_raw_memory(void *input, gint offset)
{
	gint i = 0;
	guchar *ptr = input;
	gint init_val = 0;
	extern gint num_mem_pages;

	if (raw_memory_data == NULL)
	{
		raw_memory_data = g_array_sized_new(FALSE,TRUE,sizeof(guchar),(256*num_mem_pages));
		/* Array initialization..... */
		for (i=0;i<(256*num_mem_pages);i++)
			raw_memory_data = g_array_insert_val(raw_memory_data,i,init_val);
	}

	for (i=0;i<256;i++)
		raw_memory_data = g_array_insert_val(raw_memory_data,i+(256*offset),ptr[i]);

}

/*!
 \brief update_raw_memory_view() updates the gui display of raw data returned
 from the ECU by the "F" command
 \param type (ToggleButton enumeration) used to determine what way to display
 the data (hex,binary.decimal)
 \param page_offset (gint) which page of data are we updating?
 */
void update_raw_memory_view(ToggleButton type, gint page_offset)
{
	extern gboolean interrogated;
	extern gboolean connected;
	extern GArray * raw_memory_widgets;
	extern GArray * raw_memory_data;
	GtkWidget *entry = NULL;
	guchar value = 0;
	extern gint mem_view_style[];
	gint i = 0;
	gchar * tmpbuf = NULL;

	if (!((connected) && (interrogated)))
		return;

	mem_view_style[page_offset] = (gint)type;

	for (i=0;i<256;i++)
	{
		value = -1;
		entry = g_array_index(raw_memory_widgets, GtkWidget *, i+(256*page_offset));
		if (entry == NULL)
			return;
		if (raw_memory_data == NULL)
			return;
		/* if data array doesn't exist, just break out... */
		value = g_array_index(raw_memory_data, guchar, i+(256*page_offset));
		switch ((ToggleButton)type)
		{
			case DECIMAL_VIEW:
				
				tmpbuf = g_strdup_printf("%.3i",value);
				break;
			case HEX_VIEW:
				tmpbuf = g_strdup_printf("%.2X",value);
				break;
			case BINARY_VIEW:
				tmpbuf = get_bin(value);
				break;
			default:
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": update_raw_memory_view(), style invalid, assuming HEX\n"));
				tmpbuf = g_strdup_printf("%.2X",value);
				break;

		}
		gtk_entry_set_text(GTK_ENTRY(entry),tmpbuf);
		g_free(tmpbuf);
	}
}

/*!
 \brief get_bin(gint) converts a decimal number into binary and returns it
 as a gchar *. used in the memory viewer to print numbers in binary
 \param x (gint) the value to conver to binary
 \returns a textual string of the value in binary
 */
gchar * get_bin(gint x)
{
	GString *string = g_string_new(NULL);
	gint n = 0;
	gchar * tmpbuf = NULL;

	for (n=0;n<8;n++)
	{
		if ((x & 0x80) != 0)
			g_string_append(string,"1");
		else
			g_string_append(string,"0");
		if (n == 3)
			g_string_append(string," ");
		x = x<<1;
	}
	tmpbuf = g_strdup(string->str);
	g_string_free(string,TRUE);
	return tmpbuf;
}
