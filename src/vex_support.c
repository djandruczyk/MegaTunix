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
#include <enums.h>
#include <fileio.h>
#include <globals.h>
#include <notifications.h>
#include <stdio.h>
#include <structures.h>
#include <time.h>
#include <vex_support.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <serialio.h>

gchar *vex_comment;
extern struct DynamicButtons buttons;
extern struct Tools tools;
extern FILE * io_file;
extern gchar * io_file_name;
extern GtkWidget *tools_view;
extern gboolean dualtable;
static gint import_page = -1; 

GIOChannel *gio_channel;
extern struct Ve_Const_Std *ve_const_p0;
extern struct Ve_Const_Std *ve_const_p1;

	/* Datastructure for VE table import fields.  */
static struct
{	
	gchar *version;
	gchar *revision;
	gchar *comment;
	gchar *date;
	gchar *time;
	gint total_rpm_bins;
	gint rpm_bins[RPM_BINS_MAX];
	gint total_load_bins;
	gint load_bins[LOAD_BINS_MAX];
	gint total_table_bins;
	gint table_bins[TABLE_BINS_MAX];
	gboolean got_page;
	gboolean got_rpm;
	gboolean got_load;
	gboolean got_ve;
	
} vex_import =
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	{0,0,0,0,0,0,0,0},
	0,
	{0,0,0,0,0,0,0,0},
	0,
	{0,0,0,0,0,0,0,0},
	FALSE,
	FALSE,
	FALSE,
	FALSE
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

gboolean vetable_export()
{
	struct tm *tm;
	time_t *t;
	gint i = 0;
	gint j = 0;
	unsigned char * ve_const_arr;
	gchar * tmpbuf;

	tm = g_malloc(sizeof(struct tm));
	t = g_malloc(sizeof(time_t));
	time(t);
	tm = localtime(t);
	if (vex_comment == NULL)
		vex_comment = g_strdup("No comment given");
	fprintf(io_file, "EVEME 1.0\n");
	fprintf(io_file, "UserRev: 1.00\n");
	fprintf(io_file, "UserComment: %s\n",vex_comment);
	fprintf(io_file, "Date: %i-%.2i-%i\n",
			1+(tm->tm_mon),tm->tm_mday,1900+(tm->tm_year));
	fprintf(io_file, "Time: %.2i:%.2i\n",tm->tm_hour,tm->tm_min);
	fprintf(io_file, "Page 0\n");
	fprintf(io_file, "VE Table RPM Range              [ 8]\n");
	ve_const_arr = (unsigned char *)ve_const_p0;
	for (i=0;i<8;i++)
	{
		fprintf(io_file,"   [%3d] = %3d\n",
				i,ve_const_arr[i+VE1_RPM_BINS_OFFSET]);
	}
	fprintf(io_file, "VE Table Load Range (MAP)       [ 8]\n");
	for (i=0;i<8;i++)
	{
		fprintf(io_file,"   [%3d] = %3d\n",
				i,ve_const_arr[i+VE1_KPA_BINS_OFFSET]);
	}
	fprintf(io_file, "VE Table                        [  8][  8]\n");
	fprintf(io_file, "           [  0] [  1] [  2] [  3] [  4] [  5] [  6] [  7]\n");
	for (i=1;i<=8;i++)
	{
		fprintf(io_file,"   [%3d] =",i-1);
		for (j=1;j<=8;j++)
		{
			if (j == 1)
			{
				fprintf (io_file, "  %3d",
						ve_const_arr[((i*j)-1)
						+VE1_TABLE_OFFSET]);
			}
			else
			{
				fprintf (io_file, "   %3d",
						ve_const_arr[((i*j)-1)
						+VE1_TABLE_OFFSET]);
			}
		}
		fprintf(io_file,"\n");
	}
	if (dualtable == TRUE)
	{
		ve_const_arr = (unsigned char *)ve_const_p1;
		fprintf(io_file, "Page 1\n");
		fprintf(io_file, "VE Table RPM Range              [ 8]\n");
		for (i=0;i<8;i++)
		{
			fprintf(io_file,"   [%3d] = %3d\n",
					i,ve_const_arr[i+VE2_RPM_BINS_OFFSET]);
		}
		fprintf(io_file, "VE Table Load Range (MAP)       [ 8]\n");
		for (i=0;i<8;i++)
		{
			fprintf(io_file,"   [%3d] = %3d\n",
					i,ve_const_arr[i+VE2_KPA_BINS_OFFSET]);
		}
		fprintf(io_file, "VE Table                        [  8][  8]\n");
		fprintf(io_file, "           [  0] [  1] [  2] [  3] [  4] [  5] [  6] [  7]\n");
		for (i=1;i<=8;i++)
		{
			fprintf(io_file,"   [%3d] =",i-1);
			for (j=1;j<=8;j++)
			{
				if (j == 1)
				{
					fprintf (io_file, "  %3d",
							ve_const_arr[((i*j)-1)
							+VE2_TABLE_OFFSET]);
				}
				else
				{
					fprintf (io_file, "   %3d",
							ve_const_arr[((i*j)-1)
							+VE2_TABLE_OFFSET]);
				}
			}
			fprintf(io_file,"\n");
		}
	}
	tmpbuf = g_strdup_printf("VE-Table(s) Exported Successfully\n");
	update_logbar(tools_view,NULL,tmpbuf,TRUE);

	g_free(tmpbuf);
	g_free(tm);
	g_free(t);
	g_free(vex_comment);
	vex_comment = NULL;
	return TRUE; /* return TRUE on success, FALSE on failure */
}

gboolean vetable_import()
{
	GIOStatus status = G_IO_STATUS_NORMAL;
	gio_channel = g_io_channel_new_file(io_file_name, "r", NULL);
	reset_import_flags();
	/* process lines while we can */
	while (status == G_IO_STATUS_NORMAL)
	{
		status = process_vex_line();
	}
	/* we're done - good or bad */
	g_io_channel_close(gio_channel);
	/* Normal EOF signifies successful completion */
	if ((status == G_IO_STATUS_EOF) 
			& (vex_import.got_page) 
			& (vex_import.got_load) 
			& (vex_import.got_rpm) 
			& (vex_import.got_ve))
	{
		//printf("Successful VEX import. Updating VE table.\n");
		feed_import_data_to_ms();
		gtk_widget_set_sensitive(buttons.ve_revert_but,TRUE);
		return TRUE;
	} 
#ifdef DEBUG
	else 
	{
		printf("Read was unsuccessful. %i %i %i %i \n",vex_import.got_page, vex_import.got_load, vex_import.got_rpm, vex_import.got_ve);
		return FALSE;
	}
#endif
	return FALSE;
}

GIOStatus process_vex_line()
{
	GString *a_line = g_string_new("\0");
	GIOStatus status = g_io_channel_read_line_string(gio_channel, a_line, NULL, NULL);
	gint i = 0;
	gint num_tests = sizeof(import_handlers)/sizeof(import_handlers[0]);
	if (status == G_IO_STATUS_NORMAL) 
	{
		for (i=0;i<num_tests;i++)
		{
			if (g_strrstr(a_line->str,
					import_handlers[i].import_tag) != NULL)
			{
				status = handler_dispatch(import_handlers[i].function, import_handlers[i].parsetag,a_line->str);
				if (status != G_IO_STATUS_NORMAL)
					printf("VEX_line parsing ERROR\n");
				goto breakout;
			}
		}
	}
breakout:
	g_string_free(a_line, TRUE);
	return status;
}


GIOStatus handler_dispatch(ImportParserFunc function, ImportParserArg arg, gchar * string)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	switch (function)
	{
		case HEADER:
			status = process_header(arg, string);
			break;
		case PAGE:
			status = process_page(string);
			break;
		case RANGE:
			status = process_vex_range(arg, string);
			break;
		case TABLE:
			status = process_vex_table(string);
			break;
	}
	return status;
}

GIOStatus process_header(ImportParserArg arg, gchar * string)
{
	gchar ** str_array;
	gchar *result;
	gchar *tmpbuf = NULL;;
	
	str_array = g_strsplit(string, " ", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	switch (arg)
	{
		case EVEME:
			vex_import.version = g_strdup(result);
			tmpbuf = g_strdup_printf("VEX Header: EVEME %s",result);
			update_logbar(tools_view, NULL, tmpbuf,TRUE);
			break;
		case USER_REV:	
			vex_import.revision = g_strdup(result);
			tmpbuf = g_strdup_printf("VEX Header: Revision %s",result);
			update_logbar(tools_view, NULL, tmpbuf,TRUE);
			break;
		case USER_COMMENT:	
			vex_import.comment = g_strdup(result);
			tmpbuf = g_strdup_printf("VEX Header: File Comment %s",result);
			update_logbar(tools_view, NULL, tmpbuf,TRUE);
			break;
		case DATE:	
			vex_import.date = g_strdup(result);
			tmpbuf = g_strdup_printf("VEX Header: Date %s",result);
			update_logbar(tools_view, NULL, tmpbuf,TRUE);
			break;
		case TIME:	
			vex_import.time = g_strdup(result);
			tmpbuf = g_strdup_printf("VEX Header: Time %s",result);
			update_logbar(tools_view, NULL, tmpbuf,TRUE);
			break;
		default:
			break;
			
	}
	g_free(tmpbuf);
	return G_IO_STATUS_NORMAL;
			
}

GIOStatus process_page(gchar *string)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	gchar ** str_array;
	gchar * tmpbuf;
	gchar *msg_type = NULL;

	str_array = g_strsplit(string, " ", 2);	
	import_page = atoi(str_array[1]);
	g_strfreev(str_array);
	if ((import_page < 0 ) || (import_page > 1))
	{
		status =  G_IO_STATUS_ERROR;
		msg_type = g_strdup("warning");
		tmpbuf = g_strdup_printf("VEX Import: Page %i <---ERROR\n",import_page);
	}
	else
	{
		status = G_IO_STATUS_NORMAL;
		vex_import.got_page = TRUE;
		tmpbuf = g_strdup_printf("VEX Import: Page %i\n",import_page);
	
	}
	
	update_logbar(tools_view,msg_type,tmpbuf,TRUE);
	g_free(tmpbuf);
	if (msg_type)
		g_free(msg_type);
	return status;
}

GIOStatus read_number_from_line(gint *dest)
{
	GIOStatus status;
	gchar ** str_array;
	gchar * result;
	GString *a_line = g_string_new("\0");
	status = g_io_channel_read_line_string(gio_channel, a_line, NULL, NULL);
	if (status != G_IO_STATUS_NORMAL) 
	{
		return status;
	}

	/* Make sure hte line contains an "=" sign, otherwise we'll segfault*/
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

GIOStatus process_vex_range(ImportParserArg arg, gchar * string)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	gint i;
	gint value;
	gchar ** str_array;
	gchar * result;
	gint num_bins;
	gint tmp_array[RPM_BINS_MAX];
	gchar * tmpbuf = NULL;
	gchar * msg_type = NULL;

	str_array = g_strsplit(string, "[", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	str_array = g_strsplit(result, "]", 2);
	result = g_strdup(str_array[0]);	
	
	num_bins = atoi(result);
	
	for (i=0; i<num_bins; i++) 
	{
		status = read_number_from_line(&value);
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
			tmp_array[i]=value;
	}
	if (status == G_IO_STATUS_NORMAL)
	{
		switch (arg)
		{
			case RPM_RANGE:
				vex_import.total_rpm_bins = num_bins;
				vex_import.got_rpm = TRUE;
				for (i=0; i< num_bins; i++)
					vex_import.rpm_bins[i]=tmp_array[i];
				tmpbuf = g_strdup_printf("VEX Import: RPM bins loaded successfully \n");
				break;
			case LOAD_RANGE:
				vex_import.total_load_bins = num_bins;
				vex_import.got_load = TRUE;
				for (i=0; i< num_bins; i++)
					vex_import.load_bins[i]=tmp_array[i];
				tmpbuf = g_strdup_printf("VEX Import: LOAD bins loaded successfully \n");
				break;
			default:
				break;
		}
	}
	update_logbar(tools_view,msg_type,tmpbuf,TRUE);
	g_free(tmpbuf);
	if (msg_type)
		g_free(msg_type);
	return status;
}

GIOStatus process_vex_table(gchar * string)
{
	gint i, j;
	GString *a_line;
	GIOStatus status = G_IO_STATUS_ERROR;
	gchar ** str_array;
	gchar ** str_array2;
	gchar * result;
	gchar *pos;
	gchar *numbers;
	gint value;
	gint x_bins;
	gint y_bins;
	gchar * tmpbuf = NULL;
	gchar * msg_type = NULL;

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

	vex_import.total_table_bins = x_bins*y_bins;

	/* Need to skip down one line to the actual data.... */
	a_line = g_string_new("\0");
	status = g_io_channel_read_line_string(gio_channel, a_line, NULL, NULL);
	if (status != G_IO_STATUS_NORMAL) 
	{
		g_string_free(a_line, TRUE);
		tmpbuf = g_strdup_printf("VEX Import: VE-Table I/O Read problem, file may be incomplete <---ERROR\n");
		update_logbar(tools_view,"warning",tmpbuf,TRUE);
		g_free(tmpbuf);
		return status;
	}
	g_string_free(a_line, TRUE);
	
	/* iterate over table */
	for (i=0; i<y_bins; i++) {
		a_line = g_string_new("\0");
		status = g_io_channel_read_line_string(gio_channel, a_line, NULL, NULL);
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
#ifdef DEBUG
			printf("(%i,%i) %i ",i,j,value);
#endif
			if ((value < 0) || (value > 255))
			{
				status = G_IO_STATUS_ERROR;
				tmpbuf = g_strdup_printf("VEX Import: VE-Table value %i at row %i column %i  is out of range. <---ERROR\n",value,i,j);
				msg_type = g_strdup("warning");
				goto breakout;
			}
			else
			{
				vex_import.table_bins[j+(i*x_bins)] = value;
				tmpbuf = g_strdup_printf("VEX Import: VE-Table loaded successfully\n");
			}
		}		
#ifdef DEBUG
		printf("\n");
#endif
		g_string_free(a_line, TRUE);
	}
breakout:
	update_logbar(tools_view,msg_type,tmpbuf,TRUE);
	g_free(tmpbuf);
	if (msg_type)
		g_free(msg_type);
	if (status == G_IO_STATUS_NORMAL)
		vex_import.got_ve = TRUE;
	return status;
}
			

gint vex_comment_parse(GtkWidget *widget, gpointer data)
{
	gchar *tmpbuf;
	/* Gets data from VEX comment field in tools gui and stores it 
	 * so that it gets written to the vex file 
	 */
	vex_comment = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
	tmpbuf = g_strdup_printf("VEX Comment Stored\n");
	update_logbar(tools_view,NULL,tmpbuf,TRUE);
	g_free(tmpbuf);

	return TRUE;
}

void reset_import_flags()
{
	gint i = 0;
	if (vex_import.version)
		g_free(vex_import.version);
	if (vex_import.revision)
		g_free(vex_import.revision);
	if (vex_import.comment)
		g_free(vex_import.comment);
	if (vex_import.date)
		g_free(vex_import.date);
	if (vex_import.time)
		g_free(vex_import.time);
	vex_import.got_ve = FALSE;
	vex_import.got_rpm = FALSE;
	vex_import.got_load = FALSE;
	vex_import.got_page = FALSE;
	import_page = -1;
	for (i=0;i<RPM_BINS_MAX;i++)
	{	
		vex_import.rpm_bins[i] = 0;
		vex_import.load_bins[i] = 0;
	}
	for (i=0;i<TABLE_BINS_MAX;i++)
	{	
		vex_import.table_bins[i] = 0;
	}
}

void feed_import_data_to_ms()
{
	gint i = 0;
	extern struct Ve_Const_Std *backup_ve_const_p0;
	extern struct Ve_Const_Std *backup_ve_const_p1;
	unsigned char * ptr;
	gchar * tmpbuf;
	/* Backup the ms data first... */
	memset((void *)backup_ve_const_p0, 0, MS_PAGE_SIZE);
	memset((void *)backup_ve_const_p1, 0, MS_PAGE_SIZE);
	memcpy(backup_ve_const_p0, ve_const_p0,MS_PAGE_SIZE);
	memcpy(backup_ve_const_p1, ve_const_p1,MS_PAGE_SIZE);

	/* check to make sure we update the right page */
	if (import_page == 0)
	{
		ptr = (unsigned char *) ve_const_p0;
		for (i=0;i<RPM_BINS_MAX;i++)
		{
			*(ptr+VE1_RPM_BINS_OFFSET+i) = vex_import.rpm_bins[i];
			*(ptr+VE1_KPA_BINS_OFFSET+i) = vex_import.load_bins[i];
		}
		for (i=0;i<TABLE_BINS_MAX;i++);
			*(ptr+VE1_TABLE_OFFSET+i) = vex_import.table_bins[i];

		update_ve_const();	
		tmpbuf = g_strdup_printf("VEX Import: VEtable on page 0 updated with data from the VEX file\n");
		
	}
	else	/* Page 1, dualtable and ignition variants... */
	{
		ptr = (unsigned char *) ve_const_p1;
		for (i=0;i<RPM_BINS_MAX;i++)
		{
			*(ptr+VE2_RPM_BINS_OFFSET+i) = vex_import.rpm_bins[i];
			*(ptr+VE2_KPA_BINS_OFFSET+i) = vex_import.load_bins[i];
		}
		for (i=0;i<TABLE_BINS_MAX;i++);
			*(ptr+VE2_TABLE_OFFSET+i) = vex_import.table_bins[i];

		update_ve_const();	
		tmpbuf = g_strdup_printf("VEX Import: VEtable on page 1 updated with data from the VEX file\n");
	}
	update_logbar(tools_view,NULL,tmpbuf,TRUE);
	if (tmpbuf)
		g_free(tmpbuf);
}

void revert_to_previous_data()
{
	/* Called to back out a load of a VEtable from VEX import */
	extern struct Ve_Const_Std *backup_ve_const_p0;
	extern struct Ve_Const_Std *backup_ve_const_p1;
	memcpy(ve_const_p0, backup_ve_const_p0, MS_PAGE_SIZE);
	memcpy(ve_const_p1, backup_ve_const_p1, MS_PAGE_SIZE);
	update_ve_const();
	gtk_widget_set_sensitive(buttons.ve_revert_but,FALSE);
}

