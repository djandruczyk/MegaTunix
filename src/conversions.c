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
 * Handles locading of conversion tables to convert from MS Stored
 * data formats and on-screen user-friendly formats.
 * Ceonversion data is stored ina flat text file and accessed with 
 * cfg_read_???() functions. 
 * conversiond file format is extremely simple. 
 * a section is defined for EACH variable that needs conversion.
 * the section is defined as [ Offset x] wih x being an integer offset
 * from the beginning of hte VEconstants datablock. All ops are being 
 * converted to reference variables by their offset instead of  by a structure
 * name. ( by offset makes the code cleaner, albeit slightly harder to follow).
 * Inside each section are two definitions, one being "conv_type" and 
 * "conv_Factor".
 * conv_Type has one of 4 possibles, ADD,SUB,MULT and DIV, for addition
 * subtraction, multiplication and division respectivlely.  These mathematical
 * operaands use the conv_factor and perform the requested Op on the value from
 * the gui BEFORE downloading to the MS.  On upload, the converse operation
 * is performed (if ADD used on downlaod, SUB will be used on upload), likewise
 * for multiplication and division.
`* 
 * this API is subject to change...  Not fixed in stone yet, but from what I
 * can tell, it'll allow me to simplify the signal handlers extensively and 
 * allow MegaTunix to be extended in an easier manner in the future.
 */

extern unsigned char * ve_const_arr;
struct Conversion_Chart std_conversions;

gboolean read_conversions(char *filename)
{
	ConfigFile *cfgfile;
        gchar *conv_file;
	gint conv_major;
	gint conv_minor;
	gint conv_micro;
	gchar *section;
	gint i = 0;
        conv_file = g_strconcat(g_get_home_dir(), 
			"/.MegaTunix/", filename, NULL);
        cfgfile = cfg_open_file(conv_file);
	if (cfgfile)
	{
		cfg_read_int(cfgfile, "VERSION", "major_ver", &conv_major);
		cfg_read_int(cfgfile, "VERSION", "minor_ver", &conv_minor);
		cfg_read_int(cfgfile, "VERSION", "micro_ver", &conv_micro);
		
		for(i=0;i<VEBLOCK_SIZE;i++)
		{
			section = g_strconcat( "Offset ",g_strdup_printf("%i",i),NULL);
			cfg_read_string(cfgfile, section,"conv_type",\
					&std_conversions.conv_type[i]);
					
			cfg_read_float(cfgfile, section,"conv_factor",\
					&std_conversions.conv_factor[i]);
		}
		g_free(section);
		cfg_free(cfgfile);
		g_free(conv_file);
		return TRUE;
	}
	else
	{
		no_conversions_warning();
		printf("Conversions file NOT found, CRITICAL ERROR!!!\n");
		return FALSE; /* FAILURE opening file */
	}
	return FALSE;	/* Assume a problem if we get here... */
}

gint convert_before_download(gint offset, gfloat value)
{
	gint return_value = 0;
	gint tmp_val = (gint)(value+0.001);
	gfloat factor = std_conversions.conv_factor[offset];

//	printf("offset provided, %i\n",offset);
	if(std_conversions.conv_type[offset] == NULL)
	{	/* return the Integer version of value */
		ve_const_arr[offset]=tmp_val;
		return tmp_val;
	}
	else
	{
		if ((strcmp(std_conversions.conv_type[offset],"ADD")) == 0 )
		{	/* Addition of conv_factor to valueR*/
			return_value = tmp_val+factor;
		}
		else if((strcmp(std_conversions.conv_type[offset],"SUB")) == 0)
		{	/* Subtraction of conv_factor from value */
			return_value = tmp_val-factor;
		}
		else if((strcmp(std_conversions.conv_type[offset],"MULT")) == 0)
		{	/* Multiplication of value by conv_factor */
			return_value = (gint)((value*factor)+0.001);
		}
		else if((strcmp(std_conversions.conv_type[offset],"DIV")) == 0)
		{	/* Division of value by conv_factor */
			return_value = (gint)((value/factor)+0.001);
		}
		else
		{
			printf("ERROR\n");
		}
		ve_const_arr[offset] = return_value; 
		return (return_value);
	}
	return -1;	/* will get caught by burn routine as error */
}

gfloat convert_after_upload(gint offset)
{
	gfloat return_value = 0.0;
	gfloat factor = std_conversions.conv_factor[offset];

//	printf("offset provided, %i\n",offset);
	if(std_conversions.conv_type[offset] == NULL)
	{	/* return the Integer version of value */
		return_value = ve_const_arr[offset];
		//printf("no conversion on file... returning %f\n",return_value);
		return return_value;
	}
	else
	{	/* On upload we use the CONVERSE function to translate BACK */
		//printf("input value %u\n",ve_const_arr[offset]);
		if ((strcmp(std_conversions.conv_type[offset],"ADD")) == 0 )
		{	/* subtraction of conv_factor to valueR*/
			return_value = ve_const_arr[offset]-factor;
		}
		else if((strcmp(std_conversions.conv_type[offset],"SUB")) == 0)
		{	/* Addition of conv_factor from value */
			return_value = ve_const_arr[offset]+factor;
		}
		else if((strcmp(std_conversions.conv_type[offset],"MULT")) == 0)
		{	/* Division of value by conv_factor */
			return_value = (gfloat)ve_const_arr[offset]/factor;
		}
		else if((strcmp(std_conversions.conv_type[offset],"DIV")) == 0)
		{	/* Multiplication of value by conv_factor */
			return_value = (gfloat)ve_const_arr[offset]*factor;
		}
		else
		{
			printf("ERROR\n");
		}
		//printf("conversion complete... returning %f\n",return_value);
		return (return_value);
	}
	return -1;	/* will get caught by burn routine as error */
}
