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

/*!
  \file src/plugins/freeems/freeems_errors.c
  \ingroup FreeEMSPlugin,Plugins
  \brief FreeEMS specific error reporting function(s)
  \author David Andruczyk
  */

#include <freeems_errors.h>
#include <freeems_plugin.h>


/*!
  \brief looks up the error code and returns thestring equivalent (similar in
  spirit to strerror()
  \param code is the error code integer
  \returns the Textual representation of the error code
  */
G_MODULE_EXPORT gchar * lookup_error(gint code)
{
	gint i = 0;
	gchar *msg = NULL;
	for (i=0;i< sizeof(Errors)/sizeof(Errors[0]);i++)
	{
		if (Errors[i].code == code)
			return g_strdup(Errors[i].message);
	}
	return g_strdup_printf("Couldn't find error code %i",code);
}
