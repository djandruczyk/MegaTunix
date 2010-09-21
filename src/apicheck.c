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
#include <configfile.h>
#include <defines.h>
#include <enums.h>
#include <apicheck.h>



/*!
 \brief Sets the file API to version passed by major/minor
 \param major  major api version number
 \param minor  minor api version number
 \returns TRUE on api version setting, FALSE on api version set failure
 */
gboolean set_file_api(ConfigFile *cfg, gint major, gint minor)
{
	cfg_write_int(cfg,"API","major",major);
	cfg_write_int(cfg,"API","minor",minor);

	return TRUE;
}


/*!
 \brief gets the file API and returns it thru the passed pointers
 \param major  major api version number
 \param minor  minor api version number
 \returns TRUE on api version reading, FALSE on api version not readable
 */
gboolean get_file_api(ConfigFile *cfg, gint *major, gint *minor)
{
	gboolean result = FALSE;
	gboolean result2 = FALSE;
	result = cfg_read_int(cfg,"API","major",&*major);
	result2 = cfg_read_int(cfg,"API","minor",&*minor);
	if ((result) && (result2))
		return TRUE;
	else
		return FALSE;

}
