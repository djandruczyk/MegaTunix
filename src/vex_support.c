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
#include <globals.h>
#include <stdio.h>
#include <structures.h>
#include <vex_support.h>

//extern struct  


gboolean vetable_export()
{
	printf("export VEtable\n");
	return TRUE; /* return TRUE on success, FALSE on failure */
}


gboolean vetable_import()
{
	printf("import VEtable\n");
	return TRUE; /* return TRUE on success, FALSE on failure */
}
