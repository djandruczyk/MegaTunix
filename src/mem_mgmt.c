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
  \file src/mem_mgmt.c
  \ingroup CoreMtx
  \brief Utility functions for getting/setting data within a block of memory
  \author David Andruczyk
 */

#include <debugging.h>
#include <defines.h>
#include <mem_mgmt.h>

extern gconstpointer *global_data;

/*!
  \brief _get_sized_data() is a func to return the data requested.
  The data is casted to the passed type.
  \param data is the array of data to pull from.
  \param offset is the RAW BYTE offset
  \param size is the size enumeration to be returned.
  \param bigendian is the Flag to flip bytes or not
  */
G_MODULE_EXPORT gint _get_sized_data(guint8 *data, gint offset, DataSize size, gboolean bigendian)
{
	gint result = 0;
	guint16 u16 = 0;
	gint16 s16 = 0;
	guint32 u32 = 0;
	gint32 s32 = 0;
	g_assert(offset >= 0);

	ENTER();
	switch (size)
	{
		case MTX_CHAR:
		case MTX_U08:
			/*
			printf("8 bit, returning %i\n",(guint8)data[offset]);
			*/
			EXIT();
			return (guint8)data[offset];
			break;
		case MTX_S08:
			EXIT();
			return (gint8)data[offset];
			break;
		case MTX_U16:
			u16 = ((guint8)data[offset] +((guint8)data[offset+1] << 8));
			if (bigendian)
				result = GUINT16_FROM_BE(u16);
			else
				result = GUINT16_FROM_LE(u16);
			/*
			printf("U16 bit, returning %i\n",result);
			*/
			EXIT();
			return (guint16)result;
			break;
		case MTX_S16:
			s16 = ((guint8)data[offset] +((guint8)data[offset+1] << 8));
			if (bigendian)
				result = GINT16_FROM_BE(s16);
			else
				result = GINT16_FROM_LE(s16);
			EXIT();
			return (gint16)result;
			/*
			printf("S16 bit, returning %i\n",result);
			*/
			break;
		case MTX_U32:
			u32 = ((guint8)data[offset] +((guint8)data[offset+1] << 8)+((guint8)data[offset+2] << 16)+((guint8)data[offset+3] << 24));
			if (bigendian)
				result = GUINT_FROM_BE(u32);
			else
				result = GUINT_FROM_LE(u32);
			/*
			printf("U32 bit, returning %i\n",result);
			*/
			EXIT();
			return (guint32)result;
			break;
		case MTX_S32:
			s32 = ((guint8)data[offset] +((guint8)data[offset+1] << 8)+((guint8)data[offset+2] << 16)+((guint8)data[offset+3] << 24));
			if (bigendian)
				result = GINT_FROM_BE(u32);
			else
				result = GINT_FROM_LE(u32);
			/*
			printf("S32 bit, returning %i\n",result);
			*/
			EXIT();
			return (gint32)result;
			break;
		default:
			printf(__FILE__": _get_sized_data() Data type invalid! (SIZE INVALID!!!) Try one of U08,U16,U32,S08,S16,S32...\n");
			break;
	}
	EXIT();
	return 0;
}



/*!
  \brief _set_sized_data() is a func to set the data requested.
  The data is casted to the passed type.
  \param data is the array of data to store
  \param offset is the RAW BYTE offset
  \param size is the size to be stored
  \param newval is the data to be stored
  \param bigendian is the Flag to flip bytes or not
  */
G_MODULE_EXPORT void _set_sized_data(guint8 *data, gint offset, DataSize size, gint newval, gboolean bigendian)
{
	guint16 u16 = 0;
	gint16 s16 = 0;
	guint32 u32 = 0;
	gint32 s32 = 0;

	ENTER();
	if (!data)
	{
		EXIT();
		return;
	}
	switch (size)
	{
		case MTX_CHAR:
		case MTX_U08:
			data[offset] = (guint8)newval;
			break;
		case MTX_S08:
			data[offset] = (gint8)newval;
			break;
		case MTX_U16:
			if (bigendian)
				u16 = GUINT16_TO_BE((guint16)newval);
			else
				u16 = GUINT16_TO_LE((guint16)newval);
			data[offset] = (guint8)u16;
			data[offset+1] = (guint8)((guint16)u16 >> 8);
			break;
		case MTX_S16:
			if (bigendian)
				s16 = GINT16_TO_BE((gint16)newval);
			else
				s16 = GINT16_TO_LE((gint16)newval);
			data[offset] = (guint8)s16;
			data[offset+1] = (guint8)((gint16)s16 >> 8);
			break;
		case MTX_U32:
			if (bigendian)
				u32 = GUINT32_TO_BE((guint32)newval);
			else
				u32 = GUINT32_TO_LE((guint32)newval);
			data[offset] = (guint8)u32;
			data[offset+1] = (guint8)((guint32)u32 >> 8);
			data[offset+2] = (guint8)((guint32)u32 >> 16);
			data[offset+3] = (guint8)((guint32)u32 >> 24);
			break;
		case MTX_S32:
			if (bigendian)
				s32 = GINT32_TO_BE((gint32)newval);
			else
				s32 = GINT32_TO_LE((gint32)newval);
			data[offset] = (guint8)s32;
			data[offset+1] = (guint8)((gint32)s32 >> 8);
			data[offset+2] = (guint8)((gint32)s32 >> 16);
			data[offset+3] = (guint8)((gint32)s32 >> 24);
			break;
		default:
			printf(_("ERROR! attempted set of data with NO SIZE defined\n"));
	}
	EXIT();
	return;
}

