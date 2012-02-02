/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file include/winserialbuffer.h
  \ingroup Headers
  \brief winserialbuffer header
  \author David Andruczyk
  */

#ifndef __winserialbuffer_H__
#define __winserialbuffer_H__

#ifdef __WIN32__
#include <gtk/gtk.h>
#include <string>

/* Prototypes */
class CSerialBuffer
{
	std::string InternalBuffer;
	GMutex *lock;
	gboolean LockAlways;
	glong int_cur_pos;
	glong bytes_unread;
	void Init();
	void ClearAndReset(HANDLE &EventToReset);
public:
	void LockBuffer();
	void UnLockBuffer();

	CSerialBuffer();
	virtual ~CSerialBuffer();

	/* Public Interface */
	void AddData (gchar ch);
	void AddData (std::string &Data);
	void AddData (std::string &Data, gint len);
	void AddData (gchar *strData, gint len);
	std::string GetData() {return InternalBuffer;}
	void Flush();
	glong Read_N(std::string &Data, glong count, HANDLE &EventToReset);
	gboolean Read_Upto(std::string &Data, gchar chTerm, glong &BytesRead, HANDLE &EventToReset);
	gboolean Read_Available(std::string &Data, HANDLE &EventToReset);
	inline glong GetSize() {return InternalBuffer.size();}
	inline gboolean IsEmpty() {return InternalBuffer.size() == 0;}
	gboolean Read_Upto_FIX(std::string &Data, gchar chTerm, glong &BytesRead, HANDLE &EventToReset);
};
#endif

/* Prototypes */

#endif
