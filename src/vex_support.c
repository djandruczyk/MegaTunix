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
#include <threads.h>
#include <serialio.h>

gchar *vex_comment;

/*!
 \brief The Vex_Import structure holds all fields (lots) needed to load and
 process a VEX (VEtabalt eXport file) and load it into megatunix.
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

/*!
 \brief import_handlers structure is used to hold the list of string 
 tags to search for and the function and function args to pass to the 
 appropriate handlers.
 */
static struct
{
	gchar *import_tag;		/* string to find.. */
	ImportParserFunc function;	/* Function to call... */
	ImportParserArg parsetag;	/* Enum Tag fed to function... */

} import_handlers[] = 
{
	{ "EVEME", HEADER, VEX_EVEME},
	{ "UserRev:", HEADER, VEX_USER_REV}, 
	{ "UserComment:", HEADER, VEX_USER_COMMENT},
	{ "Date:", HEADER, VEX_DATE},
	{ "Time:", HEADER, VEX_TIME},
	{ "Page", PAGE, VEX_NONE},
	{ "VE Table RPM Range\0", RANGE, VEX_RPM_RANGE},
	{ "VE Table Load Range\0", RANGE, VEX_LOAD_RANGE},
	{ "VE Table\0", TABLE, VEX_NONE}
};


/*!
 \brief vetable_export() is the export function to dump all Tables to a "vex"
 file.  It has been modified to handler multiple tables per page.  
 There is a major problem with this in that the currect VEX 1.0 spec 
 doesn't allow for multiple tables per page, so import is likely to be 
 a problem.
 \param, ptr (void *) a pointer to the output file 
 to write the data to.
 \see vetable_import
 \returns TRUE on success, FALSE on failure
 */
gboolean vetable_export(void *ptr)
{
	struct tm *tm = NULL;
	time_t *t = NULL;
	gint i = 0;
	gint j = 0;
	gint z = 0;
	gint table = -1;
	gsize count = 0;
	gint index = 0;
	gint y_page = 0;
	gint tbl_page = 0;
	gint x_page = 0;
	gint y_base = 0;
	gint tbl_base = 0;
	gint x_base = 0;
	gint x_bincount = 0;
	gint y_bincount = 0;
	extern gint ** ms_data;
	extern struct Firmware_Details *firmware;
	gchar * tmpbuf = NULL;
	GIOStatus status;
	GString *output = NULL;
	struct Io_File *iofile = (struct Io_File *)ptr;


	/* For Page 0.... */
	output = g_string_sized_new(64); /*pre-allocate 64 chars */
	//printf("total tables %i\n",firmware->total_tables);
	for (z=0;z<firmware->total_tables;z++)
	{
		table = z;
		tbl_page = firmware->table_params[table]->tbl_page;
		x_page = firmware->table_params[table]->x_page;
		y_page = firmware->table_params[table]->y_page;
		tbl_base = firmware->table_params[table]->tbl_base;
		y_base = firmware->table_params[table]->y_base;
		x_base = firmware->table_params[table]->x_base;
		y_bincount = firmware->table_params[table]->y_bincount;
		x_bincount = firmware->table_params[table]->x_bincount;

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
		output = g_string_append(output, g_strdup_printf("Page %i\n",tbl_page));
		output = g_string_append(output, g_strdup_printf("VE Table RPM Range              [%2i]\n",x_bincount));

		for (i=0;i<x_bincount;i++)
			output = g_string_append(output,g_strdup_printf("   [%3d] = %3d\n",i,ms_data[x_page][x_base+i]));

		output = g_string_append(output, g_strdup_printf("VE Table Load Range (MAP)       [%2i]\n",y_bincount));
		for (i=0;i<y_bincount;i++)
			output = g_string_append(output,g_strdup_printf("   [%3d] = %3d\n",i,ms_data[y_page][y_base+i]));

		output = g_string_append(output, g_strdup_printf("VE Table                        [%3i][%3i]\n",x_bincount,y_bincount));
		output = g_string_append(output, "           ");
		for (i=0;i<x_bincount;i++)
		{
			output = g_string_append(output, g_strdup_printf("[%3d]",i));
			if (i < (x_bincount-1))
				output = g_string_append(output, " ");
		}
		output = g_string_append(output, "\n");
		index = 0;
		for (i=0;i<x_bincount;i++)
		{
			output = g_string_append(output,g_strdup_printf("   [%3d] =",i));
			for (j=0;j<y_bincount;j++)
			{
				if (j == 0)
					output = g_string_append (output,g_strdup_printf("  %3d",ms_data[tbl_page][index+tbl_base]));
				else
					output = g_string_append (output,g_strdup_printf("   %3d",ms_data[tbl_page][index+tbl_base]));
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


/*!
 \brief vetable_import(void *) is called to import Tables from a file.  
 There currently exists a big problem in that newer firmwares (msns-extra 
 and MS-II) have multiple tables per page and the VEX 1.0 spec does NOT 
 account for that. 
 \param ptr (void *) pointer to the (Io_File) file to read 
 the data from.
 */
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
			feed_import_data_to_ecu(vex_import);
			dealloc_vex_struct(vex_import);
			vex_import = g_new0(struct Vex_Import, 1);
		}
	}
	dealloc_vex_struct(vex_import);

	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"tools_revert_button"),TRUE);

	if (status == G_IO_STATUS_ERROR)
	{
		dbg_func(g_strdup_printf(__FILE__": vetable_import()\n\tRead was unsuccessful. %i %i %i %i \n",vex_import->got_page, vex_import->got_load, vex_import->got_rpm, vex_import->got_ve),CRITICAL);
		return FALSE;
	}
	return TRUE;
}


/*!
 \brief process_vex_line() is called for to read the VEX file and
 dispatch handlers to process sections of the file.
 \param ptr (void *) pointer to Vex_Import structure.
 \param iochannel (GIOChannel *) the bytestream represented by the VEXfile
 \returns The status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL
 usually
 */
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


/*!
 \brief handler_dispatch() calls handlers based on ImportParserFunc passed. 
 It passes off the pointer to the Vex_Import struct, the function arg, 
 the string read, and the pointer to the IOchannel representing the 
 input bytestream.
 \param ptr (void *) The pointer to the Vex_Import datastructure.
 \param function (ImportParserFunc) an enumeration used to determine 
 which handler to call.
 \param arg (ImportParserArg) another enumeration passed to the functiosn 
 being dispatched from here
 \param string (gchar *) The current line of hte VEXfile just read.  Used for
 handlers than only need 1 lines's worth of data
 \param iochannel (GIOChannel *) the pointer to the input stream of the 
 vexfile for reading additional data (used by some dispatched functions)
 \see ImportParserFunc
 \see ImportParserArg
 \returns a GIOStatus of the dispatched function (usually G_IO_STATUS_NORMAL
 or G_IO_STATUS_ERROR)
 */
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

/*!
 \brief process_header() processes portions of the header of the VEX file 
 and populates the Vex_Import datastructure for this Table
 \param ptr (void *) pointer to the Vex_Import structure
 \param arg (ImportPArserArg) enumeration of header portion to process
 \param string (gchar *) text of the header line to process
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
GIOStatus process_header(void *ptr, ImportParserArg arg, gchar * string)
{
	gchar ** str_array = NULL;
	gchar *result = NULL;
	struct Vex_Import *vex_import = ptr;

	if (!string)
	{
		dbg_func(__FILE__": process_header()\n\t String passed was NULL\n",CRITICAL);
		return G_IO_STATUS_ERROR;
	}
	str_array = g_strsplit(string, " ", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	switch (arg)
	{
		case VEX_EVEME:
			vex_import->version = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: EVEME %s",result),TRUE,FALSE);
			break;
		case VEX_USER_REV:	
			vex_import->revision = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: Revision %s",result),TRUE,FALSE);
			break;
		case VEX_USER_COMMENT:	
			vex_import->comment = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: UserComment: %s",result),TRUE,FALSE);
			break;
		case VEX_DATE:	
			vex_import->date = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: Date %s",result),TRUE,FALSE);
			break;
		case VEX_TIME:	
			vex_import->time = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: Time %s",result),TRUE,FALSE);
			break;
		default:
			break;

	}
	g_free(result);
	return G_IO_STATUS_NORMAL;

}

/*!
 \brief process_page() extracts the page variable from the VEX file and 
 stores it in the Vex_Import Structure for this table.
 \param ptr (void *) Pointer to the Vex_Import structure
 \param string (gchar *) line of VEX file in which to extract the page out of
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
GIOStatus process_page(void *ptr, gchar *string)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	gchar ** str_array = NULL;
	gchar * tmpbuf= NULL;
	gchar *msg_type = NULL;
	gint page = -1;
	struct Vex_Import *vex_import = ptr;
	extern struct Firmware_Details *firmware;

	if (!string)
	{
		dbg_func(__FILE__": process_page()\n\t String passed was NULL\n",CRITICAL);
		return G_IO_STATUS_ERROR;
	}
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

/*!
 \brief read_number_from_line() is used to read the RPM/LOAD values from the
 VEX file and store them in the Vex_Import structure
 \param dest (gint *) pointer to array within the Vex_Import struct
 \param iochannel (GIOChannel *) pointer to the iochannel representing the
 VEX file
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
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

/*!
 \brief process_vex_range() is used to read the vex_ranges for RPM/LOAD 
 allocate the memory for storing the data and call the needed functions to
 read the values into the arrays.
 \see read_number_from_line
 \param ptr (void *) pointer to the Vex_Import structure
 \param arg (ImportParserArg) enumeration to decide which range we are going to
 read
 \param string (gchar *) Line of text passed to parse.
 \param iochannel (GIOChannel *) Pointer to iochannel representing VEXfile for
 retrieving additional data.
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
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

	if (!string)
	{
		dbg_func(__FILE__": process_vex_range()\n\t String passed was NULL\n",CRITICAL);
		return G_IO_STATUS_ERROR;
	}
	str_array = g_strsplit(string, "[", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	str_array = g_strsplit(result, "]", 2);
	result = g_strdup(str_array[0]);	

	num_bins = atoi(result);
	/* Allocate memory for array of values... :) */
	switch(arg)
	{
		case VEX_RPM_RANGE:
			vex_import->total_rpm_bins = num_bins;
			vex_import->rpm_bins = g_new0(gint, num_bins);
			vex_import->got_rpm = TRUE;
			break;
		case VEX_LOAD_RANGE:
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
				case VEX_RPM_RANGE:
					vex_import->rpm_bins[i] = value;
					tmpbuf = g_strdup_printf("VEX Import: RPM bins loaded successfully \n");
					break;
				case VEX_LOAD_RANGE:
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


/*!
 \brief process_vex_table() reads, processes and stores the Table data into
 the Vex_Import structure in preparation for import to the ECU.
 \param ptr (void *) pointer to the Vex_Import structure
 \param string (gchar *) pointer to the current line of the VEXfile
 \param iochannel (GIOChannel *), Pointer to the iochannel representing the
 VEX file for reading more data
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
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

	if (!string)
	{
		dbg_func(__FILE__": process_vex_table()\n\t String passed was NULL\n",CRITICAL);
		return G_IO_STATUS_ERROR;
	}
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


/*!
 \breif vex_comment_parse() stores the comment field  as entered by the
 user on the Megatunix GUI for VEX export.
 \param widget (GtkWidget *) pointer to textentry widget where user enters
 the comment
 \param data (gpointer) unused
 \returns TRUE
 */
EXPORT gboolean vex_comment_parse(GtkWidget *widget, gpointer data)
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


/*!
 \brief dealloc_vex_struct() deallocates the memory used by the Vex_Import
 datastructure
 \param ptr (void *) Pointer to the Vex_Import structure to deallocate.
 */
void dealloc_vex_struct(void *ptr)
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


/*!
 \brief feed_import_data_to_ecu() Forwards the data in the VEX file to the
 ECU.  NOTE this may have problems with firmware using multiple tables in
 a page, as the VEX format 1.0 does NOT handle that condition.
 \param ptr (void *) pointer to the Vex_Impot datastructure.
 */
void feed_import_data_to_ecu(void *ptr)
{
	gint i = 0;
	extern gint ** ms_data;
	extern gint ** ms_data_last;
	extern gint ** ms_data_backup;
	gchar * tmpbuf = NULL;
	gint page = -1;
	struct Vex_Import *vex_import = ptr;
	extern struct Firmware_Details *firmware;

	/* Since we assume the page is where the table is this can cause
	 * major problems with some firmwares that use two tables inside
	 * of one page....
	 */
	page = vex_import->page;
	/* If dimensions do NOT match, ABORT!!! */
	if (firmware->table_params[page]->x_bincount != vex_import->total_rpm_bins)
	{
		tmpbuf = g_strdup_printf("VEX Import: number of RPM bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n",firmware->table_params[page]->x_bincount,vex_import->total_rpm_bins);
		return;
	}
	if (firmware->table_params[page]->y_bincount != vex_import->total_load_bins)
	{
		tmpbuf = g_strdup_printf("VEX Import: number of LOAD bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n",firmware->table_params[page]->y_bincount,vex_import->total_load_bins);
		return;
	}

	/* Backup the ms data first... */
	memset((void *)ms_data_backup[page], 0, sizeof(gint)*firmware->page_params[page]->length);
	memcpy(ms_data_backup[page], ms_data[page],sizeof(gint)*firmware->page_params[page]->length);
			

	for (i=0;i<vex_import->total_rpm_bins;i++)
		ms_data[page][firmware->table_params[page]->x_base + i] =
			vex_import->rpm_bins[i];
	for (i=0;i<vex_import->total_load_bins;i++)
		ms_data[page][firmware->table_params[page]->y_base + i] =
			vex_import->load_bins[i];

	for (i=0;i<((vex_import->total_load_bins)*(vex_import->total_rpm_bins));i++)
		ms_data[page][firmware->table_params[page]->tbl_base + i] =
			vex_import->ve_bins[i];

	for (i=0;i<firmware->page_params[page]->length;i++)
		if (ms_data[page][i] != ms_data_last[page][i])
			write_ve_const(NULL,page,i,ms_data[page][i],firmware->page_params[page]->is_spark);

	//update_ve_const();	
	tmpbuf = g_strdup_printf("VEX Import: VEtable on page %i updated with data from the VEX file\n",vex_import->page);

	update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);
	if (tmpbuf)
		g_free(tmpbuf);
}


/*!
 \brief revert_to_previous_data() reverts the VEX import by using the backup
 of the internal datastructures.
 */
void revert_to_previous_data()
{
	gint i=0;
	gint j=0;
	/* Called to back out a load of a VEtable from VEX import */
	extern gint ** ms_data;
	extern gint ** ms_data_backup;
	extern struct Firmware_Details *firmware;
	extern GHashTable *dynamic_widgets;

	for (i=0;i<firmware->total_pages;i++)
	{
	    for (j = 0;j<firmware->page_params[i]->length;j++)
	    {
		if (ms_data_backup[i][j] != ms_data[i][j])
		    write_ve_const(NULL,i,j,ms_data_backup[i][j],firmware->page_params[i]->is_spark);
	    }
	    memcpy(ms_data[i], ms_data_backup[i], sizeof(gint)*firmware->page_params[i]->length);
	}
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"tools_revert_button"),FALSE);
	update_logbar("tools_view","warning","Reverting to previous settings....\n",TRUE,FALSE);
}
