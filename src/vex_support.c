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

gchar *vex_comment;
extern struct Tools tools;
extern FILE * io_file;
extern gchar * io_file_name;
extern GtkWidget *tools_statbar;
extern gint tools_context_id;

gboolean vetable_export()
{
	struct tm *tm;
	time_t *t;
	gint i = 0;
	gint j = 0;
	unsigned char * ve_const_arr;
	extern struct Ve_Const_Std *ve_const_p0;
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
	
	g_free(tm);
	g_free(t);
	g_free(vex_comment);
	vex_comment = NULL;
	return TRUE; /* return TRUE on success, FALSE on failure */
}

gboolean vetable_import()
{
	return TRUE; /* return TRUE on success, FALSE on failure */
}

gint vex_comment_parse(GtkWidget *widget, gpointer data)
{
	gchar buff[100];
	/* Gets data from VEX comment field in tools gui and stores it 
	 * so that it gets written to the vex file 
	 */
	vex_comment = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
	g_snprintf(buff,100,"VEX Comment Stored");
	update_statusbar(tools_statbar,tools_context_id,buff);

	return TRUE;
}
