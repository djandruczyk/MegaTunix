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

#define VEBLOCK_SIZE 128
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
					
			cfg_read_int(cfgfile, section,"conv_factor",\
					&std_conversions.conv_factor[i]);
		}
		g_free(section);
		cfg_free(cfgfile);
		g_free(conv_file);
		return TRUE;
	}
	else
		printf("file NOT found,  now leaving\n");
		return FALSE; /* FAILURE opening file */
}

