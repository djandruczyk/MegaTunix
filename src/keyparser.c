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
  \file src/keyparser.c
  \ingroup CoreMtx
  \brief String parsing functions to make things easier
  \author David Andruczyk
 */

#include <assert.h>
#include <defines.h>
#include <debugging.h>
#include <stringmatch.h>

/*!
  \brief parse_keys() splits up a string list into a vector and returns it
  and the number of keys
  \param string is the input string
  \param count is the reference to dest to place number of keys
  \param delimiter is the char to split the string with
  \returns a string vector of the original string split up with the delimiter
  */
G_MODULE_EXPORT gchar ** parse_keys(const gchar * string, gint * count, const gchar *delimiter)
{
	gchar **result = NULL;	

	ENTER();
	assert(string);
	if (!string)
	{
		MTXDBG(KEYPARSER|CRITICAL,_("String passed was NULL\n"));
		*count = 0;
		EXIT();
		return NULL;
	}
	result = g_strsplit(string,delimiter,0);
	if (count)
		*count = g_strv_length(result);
	EXIT();
	return result;
}

/*!
  \brief parse_keytypes() splits up a string list and converts the individual 
  values into enumerations
  \param string is the input string
  \param count is the reference to dest to place number of keys
  \param delimiter is the char to split the string with
  \returns a dynamic integer array of the keystypes (enums)
  */
G_MODULE_EXPORT gint * parse_keytypes(const gchar * string, gint * count, const gchar *delimiter)
{
	gchar **vector = NULL;	
	gint *keytypes = NULL;
	gint i = 0;
	gint ct = 0;

	ENTER();
	assert(string);
	if (!string)
	{
		MTXDBG(KEYPARSER|CRITICAL,_("String passed was NULL\n"));
		*count = 0;
		EXIT();
		return 0;
	}
	vector = g_strsplit(string,delimiter,0);
	while (vector[ct])
		ct++;

	keytypes = (gint *)g_malloc0(ct*sizeof(gint));	
	while (vector[i])
	{
		MTXDBG(KEYPARSER,_("Trying to translate %s\n"),vector[i]);
		keytypes[i] = translate_string(vector[i]);
		MTXDBG(KEYPARSER,_("Translated value of %s is %i\n"),vector[i],keytypes[i]);
		i++;
	}
	g_strfreev(vector);
	if (count)
		*count = i;	
	EXIT();
	return keytypes;

}
