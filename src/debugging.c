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
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gui_handlers.h>
#include <math.h>
#include <structures.h>
#include <time.h>


gint dbg_lvl = 0;

GIOChannel * dbg_channel = NULL;
GStaticMutex dbg_mutex = G_STATIC_MUTEX_INIT;
static DebugLevel dbglevels[] = 
{
	{ "Interrogation", DEBUG_LEVEL, INTERROGATOR, INTERROGATOR_SHIFT, FALSE},
	{ "OpenGL", DEBUG_LEVEL, OPENGL, OPENGL_SHIFT, FALSE},
	{ "Conversions", DEBUG_LEVEL, CONVERSIONS, CONVERSIONS_SHIFT, FALSE},
	{ "Serial Reads", DEBUG_LEVEL, SERIAL_RD, SERIAL_RD_SHIFT, FALSE},
	{ "Serial Writes", DEBUG_LEVEL, SERIAL_WR, SERIAL_WR_SHIFT, FALSE},
	{ "I/O Processing", DEBUG_LEVEL, IO_PROCESS, IO_PROCESS_SHIFT, FALSE},
	{ "Threads", DEBUG_LEVEL, THREADS, THREADS_SHIFT, FALSE},
	{ "Req Fuel", DEBUG_LEVEL, REQ_FUEL, REQ_FUEL_SHIFT, FALSE},
	{ "Tabloader", DEBUG_LEVEL, TABLOADER, TABLOADER_SHIFT, FALSE},
	{ "KeyParser", DEBUG_LEVEL, KEYPARSER, KEYPARSER_SHIFT, FALSE},
	{ "RealTime Maps", DEBUG_LEVEL, RTMLOADER, RTMLOADER_SHIFT, FALSE},
	{ "Complex Math", DEBUG_LEVEL, COMPLEX_EXPR, COMPLEX_EXPR_SHIFT, FALSE},
	{ "Critical Errors", DEBUG_LEVEL, CRITICAL, CRITICAL_SHIFT, FALSE},
};

/*!
 \brief open_debug() opens the channel to the debugging information log.
 The path defaults to the current working directory.
 */
void open_debug()
{
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;
	gsize count = 0;
	GError *error = NULL;

	g_static_mutex_lock(&dbg_mutex);
	if(!dbg_channel)
	{
		filename = g_build_filename(HOME(), "MTXlog.txt",NULL);
		dbg_channel = g_io_channel_new_file(filename,"w",&error);
		g_io_channel_set_encoding(dbg_channel,NULL,&error);
		if (dbg_channel)
		{
			t = g_malloc(sizeof(time_t));
			time(t);
			tm = localtime(t);
			g_free(t);
			tmpbuf = g_strdup_printf("Logfile opened for appending on %i-%.2i-%i at %.2i:%.2i \n",1+(tm->tm_mon),tm->tm_mday,1900+(tm->tm_year),tm->tm_hour,tm->tm_min);
			g_io_channel_write_chars(dbg_channel,tmpbuf,-1,&count,&error);
			g_free(tmpbuf);
		}
		g_free(filename);
	}
	g_static_mutex_unlock(&dbg_mutex);
}


void close_debug()
{
	g_static_mutex_lock(&dbg_mutex);
	if (dbg_channel)
		g_io_channel_close(dbg_channel);
	g_static_mutex_unlock(&dbg_mutex);
}

/*!
 \brief dbg_func() writes debugggin output to the console based on if the
 passed debug level is marked in the current debugging mask.
 \param str (gchar *) message to print out
 \param class (Dbg_Class enumeration) the debug level
 */
void dbg_func(gchar *str)
{
	gsize count = 0;
	GError *error = NULL;
	/*
	static struct tm *tm = NULL;
	static time_t *t = NULL;
	*/

	if (!dbg_channel)
	{
		g_free(str);
		return;
	}

	g_static_mutex_lock(&dbg_mutex);

	if (dbg_channel)
	{
		g_io_channel_write_chars(dbg_channel,str,-1,&count,&error);
		g_free(str);
	}
	g_static_mutex_unlock(&dbg_mutex);
}

void populate_debugging(GtkWidget *parent)
{
	GtkWidget *vbox2 = NULL;
	GtkWidget *table = NULL;
	GtkWidget *button = NULL;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint shift = 0;
	gint mask = 0;
	gint num_levels = sizeof(dbglevels)/sizeof(dbglevels[0]);


	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent),vbox2);

	table = gtk_table_new(ceil(num_levels/4),4,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,TRUE,5);

	j = 0;
	k = 0;
	for (i=0;i<num_levels;i++)
	{
		mask = dbglevels[i].dclass;
		shift = dbglevels[i].dshift;

		button = gtk_check_button_new_with_label(dbglevels[i].name);
		g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(dbglevels[i].handler));
		g_object_set_data(G_OBJECT(button),"bitshift",GINT_TO_POINTER(shift));
		g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(mask));
		g_object_set_data(G_OBJECT(button),"bitval",GINT_TO_POINTER(1));
		g_signal_connect(G_OBJECT(button),"toggled",
				G_CALLBACK(bitmask_button_handler),
				NULL);
		gtk_table_attach (GTK_TABLE (table), button, j, j+1, k, k+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
		// If user set on turn on as well
		if ((dbg_lvl & mask) >> shift)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		// if hardcoded on, turn on..
		if (dbglevels[i].enabled)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		j++;
		if (j == 4)
		{
			k++;
			j = 0;
		}
	}

}

