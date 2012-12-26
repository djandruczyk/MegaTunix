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
  \file src/apicheck.c
  \ingroup CoreMtx
  \brief Functions to get/set the API major/minor versions within .ini 
  styles files
    
  Gets or sets the API major/minor versions in an [API] section
  with the following keys "major" and "minor".
  \author David Andruczyk 
  */

#include <apicheck.h>
#include <debugging.h>

/*!
  \brief Sets the file API to version passed by major/minor
  \param cfg is the pointer to the ConfigFile structure
  \param major is the major api version number
  \param minor is the minor api version number
  \returns TRUE on api version setting, FALSE on api version set failure
  */
G_MODULE_EXPORT gboolean set_file_api(ConfigFile *cfg, gint major, gint minor)
{
	ENTER();

	cfg_write_int(cfg,"API","major",major);
	cfg_write_int(cfg,"API","minor",minor);

	EXIT();
	return TRUE;
}


/*!
  \brief gets the file API and returns it thru the passed pointers
  \param cfg is the poitner to the ConfigFile structure
  \param major is the pointer to where the major api version number should be 
  stored or NULL
  \param minor is the pointer to where the minor api version number should be 
  stored or NULL
  \returns TRUE on api version reading, FALSE on api version not readable
  */
G_MODULE_EXPORT gboolean get_file_api(ConfigFile *cfg, gint *major, gint *minor)
{
	gboolean result = TRUE;
	gboolean result2 = TRUE;

	ENTER();

	if (major)
		result = cfg_read_int(cfg,"API","major",&*major);
	if (minor)
		result2 = cfg_read_int(cfg,"API","minor",&*minor);
	if ((result) && (result2))
	{
		EXIT();
		return TRUE;
	}
	EXIT();
	return FALSE;
}
