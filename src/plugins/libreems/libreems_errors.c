/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \file src/plugins/libreems/libreems_errors.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS specific error reporting function(s)
  \author David Andruczyk
  */

#include <libreems_errors.h>
#include <libreems_plugin.h>


/*!
  \brief looks up the error code and returns thestring equivalent (similar in
  spirit to strerror()
  \param code is the error code integer
  \returns the Textual representation of the error code
  */
G_MODULE_EXPORT const gchar * lookup_error(guint code)
{
	guint i = 0;
	gchar *msg = NULL;
	ENTER();
	for (i=0;i< sizeof(Errors)/sizeof(Errors[0]);i++)
	{
		if (Errors[i].code == code)
		{
			EXIT();
			return Errors[i].message;
		}
	}
	EXIT();
	return (const gchar *)("Couldn't find error code for 0x%X",code);
}
