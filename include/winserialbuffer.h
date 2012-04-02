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


#ifdef __WIN32__
#ifndef __winserialbuffer_H__
#define __winserialbuffer_H__

#include <gtk/gtk.h>
#include <string>

/* Prototypes */
class CSerialBuffer
{
private:
	std::string InternalBuffer;
	GMutex *lock;
	bool lock_always;
	glong int_cur_pos;
	glong bytes_unread;
	void Init();
	void ClearAndReset(HANDLE &EventToReset);
public:
	inline void LockBuffer() { g_mutex_lock(lock); }
	inline void UnLockBuffer() { g_mutex_unlock(lock); }

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
	bool Read_Upto(std::string &Data, gchar chTerm, glong &BytesRead, HANDLE &EventToReset);
	bool Read_Available(std::string &Data, HANDLE &EventToReset);
	inline glong GetSize() {return InternalBuffer.size();}
	inline bool IsEmpty() {return InternalBuffer.size() == 0;}
	bool Read_Upto_FIX(std::string &Data, gchar chTerm, glong &BytesRead, HANDLE &EventToReset);
};
#endif

/* Prototypes */

#endif
