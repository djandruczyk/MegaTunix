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
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <fileio.h>
#include <gui_handlers.h>
#include <notifications.h>
#include <stdio.h>
#include <structures.h>
#include <time.h>
#include <vex_support.h>
#include <string.h>
#include <stdlib.h>
#include <serialio.h>

gchar *vex_comment;
extern GtkWidget *tools_view;


	/* Datastructure for VE table import fields.  */
/*!
 \brief The Vex_Import structure holds all fields (lots) needed to load and
 process a VEX (Vetace EXport file) and load it into megatunix.
 \see vetable_import
 \see vetable_export
 */
struct Vex_Import
{	
	gchar *version;		/* String */
	gchar *revision;	/* String */
	gchar *comment;		/* String */
	gchar *date;		/* String */
	gchar *time;		/* String */
	gint page;		/* Int */
	gint total_rpm_bins;	/* Int */
	gint *rpm_bins;		/* Int Array, dynamic */
	gint total_load_bins;	/* Int */
	gint *load_bins;	/* Int Array, dynamic */
	gint total_ve_bins;	/* Int */
	gint *ve_bins;	/* Int Array, dynamic */
	gboolean got_page;	/* Flag */
	gboolean got_rpm;	/* Flag */
	gboolean got_load;	/* Flag */
	gboolean got_ve;	/* Flag */
	
};

static struct
{
	gchar *import_tag;		/* string to find.. */
	ImportParserFunc function;	/* Function to call... */
	ImportParserArg parsetag;	/* Enum Tag fed to function... */

} import_handlers[] = 
{
	{ "EVEME", HEADER, EVEME},
	{ "UserRev:", HEADER, USER_REV}, 
	{ "UserComment:", HEADER, USER_COMMENT},
	{ "Date:", HEADER, DATE},
	{ "Time:", HEADER, TIME},
	{ "Page", PAGE, NONE},
	{ "VE Table RPM Range\0", RANGE, RPM_RANGE},
	{ "VE Table Load Range\0", RANGE, LOAD_RANGE},
	{ "VE Table\0", TABLE, NONE}
};

gboolean vetable_export(void *ptr)
{
	struct tm *tm = NULL;
	time_t *t = NULL;
	gint i = 0;
	gint j = 0;
	gint z = 0;
	gint page = -1;
	gsize count = 0;
	gint index = 0;
	gint load_base = 0;
	gint ve_base = 0;
	gint rpm_base = 0;
	gint rpm_bincount = 0;
	gint load_bincount = 0;
	extern gint ** ms_data;
	extern struct Firmware_Details *firmware;
	gchar * tmpbuf = NULL;
	GIOStatus status;
	GString *output = NULL;
	struct Io_File *iofile = (struct Io_File *)ptr;


	/* For Page 0.... */
	output = g_string_sized_new(64); /*pre-allocate 64 chars */
	for (z=0;z<firmware->total_pages;z++)
	{
		page = z;
		ve_base = firmware->page_params[page]->ve_base;
		load_base = firmware->page_params[page]->load_base;
		rpm_base = firmware->page_params[page]->rpm_base;
		load_bincount = firmware->page_params[page]->load_bincount;
		rpm_bincount = firmware->page_params[page]->rpm_bincount;

		t = g_malloc(sizeof(time_t));
		time(t);
		tm = localtime(t);
		g_free(t);
		if (vex_comment == NULL)
			vex_comment = g_strdup("No comment given");

		output = g_string_append(output, "EVEME 1.0\n");
		output = g_string_append(output, "UserRev: 1.00\n");
		output = g_string_append(output, g_strdup_printf("UserComment: %s\n",vex_comment));
		output = g_string_append(output, g_strdup_printf("Date: %i-%.2i-%i\n",1+(tm->tm_mon),tm->tm_mday,1900+(tm->tm_year)));

		output = g_string_append(output, g_strdup_printf("Time: %.2i:%.2i\n",tm->tm_hour,tm->tm_min));
		output = g_string_append(output, g_strdup_printf("Page %i\n",page));
		output = g_string_append(output, g_strdup_printf("VE Table RPM Range              [%2i]\n",rpm_bincount));

		for (i=0;i<rpm_bincount;i++)
			output = g_string_append(output,g_strdup_printf("   [%3d] = %3d\n",i,ms_data[page][rpm_base+i]));

		output = g_string_append(output, g_strdup_printf("VE Table Load Range (MAP)       [%2i]\n",load_bincount));
		for (i=0;i<load_bincount;i++)
			output = g_string_append(output,g_strdup_printf("   [%3d] = %3d\n",i,ms_data[page][load_base+i]));

		output = g_string_append(output, g_strdup_printf("VE Table                        [%3i][%3i]\n",rpm_bincount,load_bincount));
		output = g_string_append(output, "           ");
		for (i=0;i<rpm_bincount;i++)
		{
			output = g_string_append(output, g_strdup_printf("[%3d]",i));
			if (i < (rpm_bincount-1))
				output = g_string_append(output, " ");
		}
		output = g_string_append(output, "\n");
		index = 0;
		for (i=0;i<rpm_bincount;i++)
		{
			output = g_string_append(output,g_strdup_printf("   [%3d] =",i));
			for (j=0;j<load_bincount;j++)
			{
				if (j == 0)
					output = g_string_append (output,g_strdup_printf("  %3d",ms_data[page][index+ve_base]));
				else
					output = g_string_append (output,g_strdup_printf("   %3d",ms_data[page][index+ve_base]));
				index++;
			}
			output = g_string_append(output,"\n");
		}
	}
	status = g_io_channel_write_chars(
			iofile->iochannel,output->str,output->len,&count,NULL);
	if (status != G_IO_STATUS_NORMAL)
		dbg_func(__FILE__": vetable_export()\n\tError exporting VEX file\n",CRITICAL);
	g_string_free(output,TRUE);

	tmpbuf = g_strdup_printf("VE-Table(s) Exported Successfully\n");
	update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);

	if (tmpbuf)
		g_free(tmpbuf);
	if (vex_comment)
		g_free(vex_comment);
	vex_comment = NULL;
	return TRUE; /* return TRUE on success, FALSE on failure */
}

gboolean vetable_import(void *ptr)
{
	struct Io_File *iofile = NULL;
	gboolean go=TRUE;
	GIOStatus status = G_IO_STATUS_NORMAL;
	struct Vex_Import *vex_import = NULL;
	extern GHashTable *dynamic_widgets;

	if (ptr != NULL)
		iofile = (struct Io_File *)ptr;
	else
	{
		dbg_func(__FILE__": vetable_import()\n\tIo_File undefined, returning!!\n",CRITICAL);
		return FALSE;
	}
	vex_import = g_new0(struct Vex_Import, 1);

	//reset_import_flags();
	status = g_io_channel_seek_position(iofile->iochannel,0,G_SEEK_SET,NULL);
	if (status != G_IO_STATUS_NORMAL)
		dbg_func(__FILE__": vetable_import()\n\tError seeking to beginning of the file\n",CRITICAL);
	/* process lines while we can */
	while (go)
	{
		status = process_vex_line(vex_import,iofile->iochannel);
		if (status == G_IO_STATUS_EOF)
		{
			go = FALSE;
			break;
		}
		/* This check means we got 1 whole ve/spark table, so we feed
		 * it to the ECU clear the data structure and start over...
		 */
		if ((status != G_IO_STATUS_EOF) 
				& (vex_import->got_page) 
				& (vex_import->got_load) 
				& (vex_import->got_rpm) 
				& (vex_import->got_ve))
		{
			feed_import_data_to_ms(vex_import);
			dealloc_ve_struct(vex_import);
			vex_import = g_new0(struct Vex_Import, 1);
		}
	}
	dealloc_ve_struct(vex_import);

	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"tools_revert_button"),TRUE);

	if (status == G_IO_STATUS_ERROR)
	{
		dbg_func(g_strdup_printf(__FILE__": vetable_import()\n\tRead was unsuccessful. %i %i %i %i \n",vex_import->got_page, vex_import->got_load, vex_import->got_rpm, vex_import->got_ve),CRITICAL);
		return FALSE;
	}
	return TRUE;
}

GIOStatus process_vex_line(void * ptr, GIOChannel *iochannel)
{
	GString *a_line = g_string_new("\0");
	GIOStatus status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
	gint i = 0;
	gint num_tests = sizeof(import_handlers)/sizeof(import_handlers[0]);

	if (status == G_IO_STATUS_NORMAL) 
	{
		for (i=0;i<num_tests;i++)
		{
			if (g_strrstr(a_line->str,import_handlers[i].import_tag) != NULL)
			{
				status = handler_dispatch(ptr, import_handlers[i].function, import_handlers[i].parsetag,a_line->str, iochannel);
				if (status != G_IO_STATUS_NORMAL)
					dbg_func(__FILE__": process_vex_line()\n\tVEX_line parsing ERROR\n",CRITICAL);
				goto breakout;
			}
		}
	}
breakout:
	g_string_free(a_line, TRUE);
	return status;
}

GIOStatus handler_dispatch(void *ptr, ImportParserFunc function, ImportParserArg arg, gchar * string, GIOChannel *iochannel)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	switch (function)
	{
		case HEADER:
			status = process_header(ptr, arg, string);
			break;
		case PAGE:
			status = process_page(ptr, string);
			break;
		case RANGE:
			status = process_vex_range(ptr, arg, string, iochannel);
			break;
		case TABLE:
			status = process_vex_table(ptr, string, iochannel);
			break;
	}
	return status;
}

GIOStatus process_header(void *ptr, ImportParserArg arg, gchar * string)
{
	gchar ** str_array = NULL;
	gchar *result = NULL;
	struct Vex_Import *vex_import = ptr;

	str_array = g_strsplit(string, " ", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	switch (arg)
	{
		case EVEME:
			vex_import->version = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: EVEME %s",result),TRUE,FALSE);
			break;
		case USER_REV:	
			vex_import->revision = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: Revision %s",result),TRUE,FALSE);
			break;
		case USER_COMMENT:	
			vex_import->comment = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: UserComment: %s",result),TRUE,FALSE);
			break;
		case DATE:	
			vex_import->date = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: Date %s",result),TRUE,FALSE);
			break;
		case TIME:	
			vex_import->time = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: Time %s",result),TRUE,FALSE);
			break;
		default:
			break;

	}
	g_free(result);
	return G_IO_STATUS_NORMAL;

}

GIOStatus process_page(void *ptr, gchar *string)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	gchar ** str_array = NULL;
	gchar * tmpbuf= NULL;
	gchar *msg_type = NULL;
	gint page = -1;
	struct Vex_Import *vex_import = ptr;
	extern struct Firmware_Details *firmware;

	str_array = g_strsplit(string, " ", 2);	
	page = atoi(str_array[1]);
	g_strfreev(str_array);
	if ((page < 0 ) || (page > firmware->total_pages))
	{
		status =  G_IO_STATUS_ERROR;
		msg_type = g_strdup("warning");
		tmpbuf = g_strdup_printf("VEX Import: Page %i out of range <---ERROR\n",page);
	}
	else
	{
		status = G_IO_STATUS_NORMAL;
		vex_import->page = page;
		vex_import->got_page = TRUE;
		tmpbuf = g_strdup_printf("VEX Import: Page %i\n",page);

	}

	update_logbar("tools_view",msg_type,tmpbuf,TRUE,FALSE);
	g_free(tmpbuf);
	if (msg_type)
		g_free(msg_type);
	return status;
}

GIOStatus read_number_from_line(gint *dest, GIOChannel *iochannel)
{
	GIOStatus status;
	gchar ** str_array = NULL;
	gchar * result = NULL;
	GString *a_line = g_string_new("\0");
	status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
	if (status != G_IO_STATUS_NORMAL) 
	{
		return status;
	}

	/* Make sure the line contains an "=" sign, otherwise we'll segfault*/
	if (strstr(a_line->str, "=") != NULL)
	{
		str_array = g_strsplit(a_line->str, "=", 2);
		result = g_strdup(str_array[1]);	
		g_strfreev(str_array);
		*dest = atoi(result);
	}
	else
		status = G_IO_STATUS_ERROR;

	g_string_free(a_line, TRUE);
	return status;
}

GIOStatus process_vex_range(void *ptr, ImportParserArg arg, gchar * string, GIOChannel *iochannel)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	gint i = 0;
	gint value = 0;
	gint num_bins = 0;
	gchar ** str_array = NULL;
	gchar * result = NULL;
	gchar * tmpbuf = NULL;
	gchar * msg_type = NULL;
	struct Vex_Import *vex_import = ptr;

	str_array = g_strsplit(string, "[", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	str_array = g_strsplit(result, "]", 2);
	result = g_strdup(str_array[0]);	

	num_bins = atoi(result);
	/* Allocate memory for array of values... :) */
	switch(arg)
	{
		case RPM_RANGE:
			vex_import->total_rpm_bins = num_bins;
			vex_import->rpm_bins = g_new0(gint, num_bins);
			vex_import->got_rpm = TRUE;
			break;
		case LOAD_RANGE:
			vex_import->total_load_bins = num_bins;
			vex_import->load_bins = g_new0(gint, num_bins);
			vex_import->got_load = TRUE;
			break;
		default:
			break;
	}

	for (i=0; i<num_bins; i++) 
	{
		status = read_number_from_line(&value,iochannel);
		if (status != G_IO_STATUS_NORMAL) 
		{
			tmpbuf = g_strdup_printf("VEX Import: File I/O Read problem, file may be incomplete <---ERROR\n");
			msg_type = g_strdup("warning");
			break;
		}
		if ((value < 0) || (value > 255))
		{
			status = G_IO_STATUS_ERROR;
			tmpbuf = g_strdup_printf("VEX Import: RPM/Load bin %i value %i out of bounds <---ERROR\n",i,value);
			msg_type = g_strdup("warning");
			break;
		}
		else
			switch (arg)
			{
				case RPM_RANGE:
					vex_import->rpm_bins[i] = value;
					tmpbuf = g_strdup_printf("VEX Import: RPM bins loaded successfully \n");
					break;
				case LOAD_RANGE:
					vex_import->load_bins[i] = value;
					tmpbuf = g_strdup_printf("VEX Import: LOAD bins loaded successfully \n");
					break;
				default:
					break;
			}
	}
	update_logbar("tools_view",msg_type,tmpbuf,TRUE,FALSE);
	g_free(tmpbuf);
	if (msg_type)
		g_free(msg_type);
	return status;
}

GIOStatus process_vex_table(void *ptr, gchar * string, GIOChannel *iochannel)
{
	gint i = 0, j = 0;
	GString *a_line;
	GIOStatus status = G_IO_STATUS_ERROR;
	gchar ** str_array = NULL;
	gchar ** str_array2 = NULL;
	gchar * result = NULL;
	gchar *pos = NULL;
	gchar *numbers = NULL;
	gint value = 0;
	gint x_bins = 0;
	gint y_bins = 0;
	gchar * tmpbuf = NULL;
	gchar * msg_type = NULL;
	struct Vex_Import *vex_import = ptr;

	/* Get first number of [  x][  y] in the string line */
	str_array = g_strsplit(string, "[", 3);
	result = g_strdup(str_array[1]);	
	str_array2 = g_strsplit(result, "]", 2);
	result = g_strdup(str_array2[0]);	
	g_strfreev(str_array2);
	x_bins = atoi(result);

	/* Get first number of [  x][  y] in the string line */
	result = g_strdup(str_array[2]);
	g_strfreev(str_array);
	str_array2 = g_strsplit(result, "]", 2);
	result = g_strdup(str_array2[0]);
	g_strfreev(str_array2);
	y_bins = atoi(result);	

	vex_import->total_ve_bins = x_bins*y_bins;
	vex_import->ve_bins = g_new0(gint,(x_bins * y_bins));

	/* Need to skip down one line to the actual data.... */
	a_line = g_string_new("\0");
	status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
	if (status != G_IO_STATUS_NORMAL) 
	{
		g_string_free(a_line, TRUE);
		tmpbuf = g_strdup_printf("VEX Import: VE-Table I/O Read problem, file may be incomplete <---ERROR\n");
		update_logbar("tools_view","warning",tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
		return status;
	}
	g_string_free(a_line, TRUE);

	/* iterate over table */
	for (i=0; i<y_bins; i++) {
		a_line = g_string_new("\0");
		status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
		if (status != G_IO_STATUS_NORMAL) 
		{
			g_string_free(a_line, TRUE);
			tmpbuf = g_strdup_printf("VEX Import: VE-Table I/O Read problem, file may be incomplete <---ERROR\n");
			msg_type = g_strdup("warning");
			break;
		}
		pos = g_strrstr(a_line->str,"=\0");
		numbers = g_strdup(pos+sizeof(char));

		/* process a row */
		for (j=0; j<x_bins; j++) 
		{
			value = (int)strtol(numbers,&numbers,10);
			if ((value < 0) || (value > 255))
			{
				status = G_IO_STATUS_ERROR;
				tmpbuf = g_strdup_printf("VEX Import: VE-Table value %i at row %i column %i  is out of range. <---ERROR\n",value,i,j);
				msg_type = g_strdup("warning");
				goto breakout;
			}
			else
			{
				vex_import->ve_bins[j+(i*x_bins)] = value;
				tmpbuf = g_strdup_printf("VEX Import: VE-Table loaded successfully\n");
			}
		}		
		g_string_free(a_line, TRUE);
	}
breakout:
	update_logbar("tools_view",msg_type,tmpbuf,TRUE,FALSE);
	g_free(tmpbuf);
	if (msg_type)
		g_free(msg_type);
	if (status == G_IO_STATUS_NORMAL)
		vex_import->got_ve = TRUE;
	return status;
}

gint vex_comment_parse(GtkWidget *widget, gpointer data)
{
	gchar *tmpbuf = NULL;;
	/* Gets data from VEX comment field in tools gui and stores it 
	 * so that it gets written to the vex file 
	 */
	vex_comment = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
	tmpbuf = g_strdup_printf("VEX Comment Stored\n");
	update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);
	g_free(tmpbuf);

	return TRUE;
}

void dealloc_ve_struct(void *ptr)
{
	struct Vex_Import *vex_import = ptr;
	if (vex_import->version)
		g_free(vex_import->version);
	if (vex_import->revision)
		g_free(vex_import->revision);
	if (vex_import->comment)
		g_free(vex_import->comment);
	if (vex_import->date)
		g_free(vex_import->date);
	if (vex_import->time)
		g_free(vex_import->time);
	if (vex_import->rpm_bins)
		g_free(vex_import->rpm_bins);
	if (vex_import->load_bins)
		g_free(vex_import->load_bins);
	if (vex_import->ve_bins)
		g_free(vex_import->ve_bins);
	if (vex_import)
		g_free(vex_import);
}

void feed_import_data_to_ms(void *ptr)
{
	gint i = 0;
	extern gint ** ms_data;
	extern gint ** ms_data_backup;
	gchar * tmpbuf = NULL;
	gint page = -1;
	struct Vex_Import *vex_import = ptr;
	extern struct Firmware_Details *firmware;

	page = vex_import->page;
	/* If dimensions do NOT match, ABORT!!! */
	if (firmware->page_params[page]->rpm_bincount != vex_import->total_rpm_bins)
	{
		tmpbuf = g_strdup_printf("VEX Import: number of RPM bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n",firmware->page_params[page]->rpm_bincount,vex_import->total_rpm_bins);
		return;
	}
	if (firmware->page_params[page]->load_bincount != vex_import->total_load_bins)
	{
		tmpbuf = g_strdup_printf("VEX Import: number of LOAD bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n",firmware->page_params[page]->load_bincount,vex_import->total_load_bins);
		return;
	}

	/* Backup the ms data first... */
	memset((void *)ms_data_backup[page], 0, MS_PAGE_SIZE);
	memcpy(ms_data_backup[page], 
			ms_data[page],MS_PAGE_SIZE);

	for (i=0;i<vex_import->total_rpm_bins;i++)
		ms_data[page][firmware->page_params[page]->rpm_base + i] =
			vex_import->rpm_bins[i];
	for (i=0;i<vex_import->total_load_bins;i++)
		ms_data[page][firmware->page_params[page]->load_base + i] =
			vex_import->load_bins[i];

	for (i=0;i<((vex_import->total_load_bins)*(vex_import->total_rpm_bins));i++)
		ms_data[page][firmware->page_params[page]->ve_base + i] =
			vex_import->ve_bins[i];

	update_ve_const();	
	tmpbuf = g_strdup_printf("VEX Import: VEtable on page %i updated with data from the VEX file\n",vex_import->page);

	update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);
	if (tmpbuf)
		g_free(tmpbuf);
}

void revert_to_previous_data()
{
	gint i=0;
	/* Called to back out a load of a VEtable from VEX import */
	extern gint ** ms_data;
	extern gint ** ms_data_backup;
	extern struct Firmware_Details *firmware;
	extern GHashTable *dynamic_widgets;

	for (i=0;i<firmware->total_pages;i++)
	{
		memcpy(ms_data[i], ms_data_backup[i], MS_PAGE_SIZE*sizeof(gint));
	}
	update_ve_const();
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"tools_revert_button"),FALSE);
	update_logbar("tools_view","warning","Reverting to previous settings....\n",TRUE,FALSE);
}
