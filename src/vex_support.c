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
extern struct Tools tools;
extern FILE * io_file;
extern gchar * io_file_name;
extern GtkWidget *tools_view;

GIOChannel *gio_channel;
extern struct Ve_Const_Std *ve_const_p0;

gboolean vetable_export()
{
	struct tm *tm;
	time_t *t;
	gint i = 0;
	gint j = 0;
	unsigned char * ve_const_arr;
	gchar * tmpbuf;
	//extern struct Ve_Const_Std *ve_const_p1;

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
	/* DUALTABLE,  print out page 2 
	if (dualtable == TRUE)
	fprintf(io_file, "Page 1\n");
        fprintf(io_file, "VE Table RPM Range              [ 8]\n");
        for (i=0;i<8;i++)
        {
                fprintf(io_file,"   [%3d] = %3d\n",
                                i,ve_const_page1[i+VE2_RPM_BINS_OFFSET]);
        }
        fprintf(io_file, "VE Table Load Range (MAP)       [ 8]\n");
        for (i=0;i<8;i++)
        {
                fprintf(io_file,"   [%3d] = %3d\n",
                                i,ve_const_page1[i+VE2_KPA_BINS_OFFSET]);
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
                                                ve_const_page1[((i*j)-1)
                                                +VE2_TABLE_OFFSET]);
                        }
                        else
                        {
                                fprintf (io_file, "   %3d",
                                                ve_const_page1[((i*j)-1)
                                                +VE2_TABLE_OFFSET]);
                        }
                }
                fprintf(io_file,"\n");
        }
	*/
	
	tmpbuf = g_strdup_printf("VE-Table(s) Exported Successfully\n");
	update_logbar(tools_view,NULL,tmpbuf);

	g_free(tmpbuf);
	g_free(tm);
	g_free(t);
	g_free(vex_comment);
	vex_comment = NULL;
	return TRUE; /* return TRUE on success, FALSE on failure */
}

gint read_number_from_line()
{
	gint result;
	gchar *pos, *number;
	GString *a_line = g_string_new("\0");
	result = g_io_channel_read_line_string(gio_channel, a_line, NULL, NULL);
	pos = g_strrstr(a_line->str,"=\0");
	number = g_strdup(pos+sizeof(char));
	g_strstrip(number);
	result = atoi(number);
	g_string_free(a_line, TRUE);
	return result;
}

gint process_vex_rpm_range()
{
	gint i;
	for (i=0; i<8; i++) {
		ve_const_p0->rpm_bins[i] = read_number_from_line();
	}
	return TRUE;
}

gint process_vex_map_range()
{
	gint i;
	for (i=0; i<8; i++) {
		ve_const_p0->kpa_bins[i] = read_number_from_line();
	}
	return TRUE;
}


gint process_vex_table()
{
	gint i, j;
	gchar *pos, *numbers;
	GString *a_line = g_string_new("\0");
	g_io_channel_read_line_string(gio_channel, a_line, NULL, NULL);
	g_string_free(a_line, TRUE);
	/* iterate over table */
	for (i=0; i<8; i++) {
		a_line = g_string_new("\0");
		g_io_channel_read_line_string(gio_channel, a_line, NULL, NULL);
		pos = g_strrstr(a_line->str,"=\0");
		numbers = g_strdup(pos+sizeof(char));
		for (j=0; j<8; j++) {
			ve_const_p0->ve_bins[j+(i*8)] = (int)strtol(numbers,&numbers,10);
		}		
		g_string_free(a_line, TRUE);
	}
	return TRUE;
}

gint process_vex_line()
{
	gint result;
	GString *a_line = g_string_new("\0");
	result = g_io_channel_read_line_string(gio_channel, a_line, NULL, NULL);
	// the order of this is important... sadly.
	if (g_strrstr(a_line->str,"RPM Range\0") != NULL) {
		process_vex_rpm_range();
	} else if (g_strrstr(a_line->str,"Load Range\0") != NULL) {
		process_vex_map_range();
	} else if (g_strrstr(a_line->str,"VE Table\0") != NULL) {	
		process_vex_table();
	/*
	} else {
		printf("Unrecognized or otherwise useless line:\n %s \n", \
			a_line->str);
	*/
	} 
	g_string_free(a_line, TRUE);
	return result;
}

gboolean vetable_import()
{
	GIOStatus status = G_IO_STATUS_NORMAL;	
	gio_channel = g_io_channel_new_file(io_file_name, "r", NULL);
	while (status != G_IO_STATUS_EOF)
	{
		status = process_vex_line();
	}
	update_ve_const();
	return TRUE; /* return TRUE on success, FALSE on failure */
}

gint vex_comment_parse(GtkWidget *widget, gpointer data)
{
	gchar *tmpbuf;
	/* Gets data from VEX comment field in tools gui and stores it 
	 * so that it gets written to the vex file 
	 */
	vex_comment = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
	tmpbuf = g_strdup_printf("VEX Comment Stored\n");
	update_logbar(tools_view,NULL,tmpbuf);
	g_free(tmpbuf);

	return TRUE;
}
