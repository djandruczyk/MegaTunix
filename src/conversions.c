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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <defines.h>
#include <protos.h>
#include <globals.h>
#include <conversions.h>

/* Conversions.c
 * 
 * conv_Type has one of 4 possibles, ADD,SUB,MULT and DIV, for addition
 * subtraction, multiplication and division respectivlely.  These mathematical
 * operaands use the conv_factor and perform the requested Op on the value from
 * the gui BEFORE downloading to the MS.  On upload, the converse operation
 * is performed (if ADD used on downlaod, SUB will be used on upload), likewise
 * for multiplication and division.
 * 
 * As of 09/02/2003 (sept 2nd 2003), The external file has been dropped in 
 * favor of coding the conversions internally.  This makes it easier for the
 * user to install as they don't have to worry about a conversions table to 
 * put in the right place to be loaded...
 */

extern unsigned char * ve_const_arr;
struct Conversion_Chart std_conversions;
extern GtkWidget * veconst_widgets_1[];
extern GtkWidget * veconst_widgets_2[];

void read_conversions(void)
{
	gint i = 0;
	gint dl_type;

	for (i=0;i<VEBLOCK_SIZE;i++)
        {
                if (GTK_IS_OBJECT(veconst_widgets_1[i]))
                {
			dl_type = (gint)g_object_get_data(
                                G_OBJECT(veconst_widgets_1[i]),"dl_type");
                        if (dl_type == IMMEDIATE)
			{
				std_conversions.conv_type[i] = 
						(gint)g_object_get_data(
						G_OBJECT(veconst_widgets_1[i]),
						"conv_type");
				std_conversions.conv_factor[i] = 
						(gfloat)((gint)
						g_object_get_data(
						G_OBJECT(veconst_widgets_1[i]),
						"conv_factor_x100"))
						/100.0;
			}
                                
                }
        }

	return TRUE;
}

gint convert_before_download(gint offset, gfloat value)
{
	gint return_value = 0;
	gint tmp_val = (gint)(value+0.001);
	gfloat factor = std_conversions.conv_factor[offset];

	switch (std_conversions.conv_type[offset])
	{
		case (ADD):
			return_value = tmp_val + factor;
			break;
		case (SUB):
			return_value = tmp_val - factor;
			break;
		case (MULT):
			return_value = (gint)((value*factor) + 0.001);
			break;
		case (DIV):
			return_value = (gint)((value/factor) + 0.001);
			break;
		default:
			return_value = tmp_val;
			break;
	}
	/* Store value in veconst_struct (accessing it via array syntax as 
	 * it's friggin easier).... 
	 */
	ve_const_arr[offset] = return_value; 
	return (return_value);
}

gfloat convert_after_upload(gint offset)
{
	gfloat return_value = 0.0;
	gfloat factor = std_conversions.conv_factor[offset];

	/* Since this is the upload we actually do the CONVERSE mathematical 
	 * operation since the algorithm was designed for the download side, 
	 * On upload we need to "un-convert" from MS values to Gui friendly
	 * versions....
	 */
	switch (std_conversions.conv_type[offset])
	{
		case (ADD):
			return_value = ve_const_arr[offset] - factor;
			break;
		case (SUB):
			return_value = ve_const_arr[offset] + factor;
			break;
		case (MULT):
			return_value = (gfloat)ve_const_arr[offset] / factor;
			break;
		case (DIV):
			return_value = (gfloat)ve_const_arr[offset] * factor;
			break;
		default:
			return_value = ve_const_arr[offset];
			break;

	}
	return (return_value);
}
