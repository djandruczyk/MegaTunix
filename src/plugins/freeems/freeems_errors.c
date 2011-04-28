/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <freeems_errors.h>


G_MODULE_EXPORT const gchar * lookup_error(gint code)
{
	gint i = 0;
	for (i=0;i< sizeof(Errors)/sizeof(Errors[0]);i++)
		if (Errors[i].code == code)
			return Errors[i].message;
	return "Couldn't find error code";
}
