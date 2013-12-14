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
  \file include/winserialcommhelper.h
  \ingroup Headers
  \brief winserialcommhelper header
  \author David Andruczyk
  */


#ifdef __WIN32__

#ifndef __winserialcommhelper_H__
#define __winserialcommhelper_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <windows.h>
#include <winserialbuffer.h>
#include <map>

typedef enum tagSERIAL_STATE
{
	SS_Unknown,
	SS_UnInit,
	SS_Init,
	SS_Started ,
	SS_Stopped ,
	
} SERIAL_STATE;

class CSerialCommHelper  
{
private:
	SERIAL_STATE	state;
	HANDLE CommPort;
	HANDLE ThreadTerm ;
	HANDLE Thread;
	HANDLE ThreadStarted;
	HANDLE DataRx;
	bool connected;
	void InvalidateHandle(HANDLE& Handle );
	void CloseAndCleanHandle(HANDLE& Handle) ;
	
	CSerialBuffer SerialBuffer;
	GMutex *lock;
	SERIAL_STATE GetCurrentState() {return state;}
public:
	CSerialCommHelper();
	virtual ~CSerialCommHelper();
	//void GetEventToWaitOn(HANDLE* hEvent) {*hEvent = DataRx;}
	HANDLE GetWaitForEvent() {return DataRx;} 

	inline void LockThis() {g_mutex_lock (lock);}	
	inline void UnLockThis() {g_mutex_unlock (lock); }
	inline void InitLock() {g_mutex_init(lock);}
	inline void DelLock() {g_mutex_clear(lock );}
 	inline bool IsInputAvailable()
	{
		LockThis (); 
		bool Data = ( !SerialBuffer.IsEmpty() ) ;
		UnLockThis (); 
		return Data;
	} 
	inline bool IsConnection() {return connected ;}
 	inline void SetDataReadEvent() { SetEvent ( DataRx );	}

	HRESULT Read_N (std::string& data,long Count,long TimeOut);
	HRESULT Read_Upto (std::string& data,char chTerminator ,long	* Count,long TimeOut);
	HRESULT ReadAvailable(std::string& data);
	HRESULT Write (const char* data,DWORD Size);
	HRESULT Init(std::string PortName= "COM1", DWORD BaudRate = 9600,BYTE byParity = 0,BYTE byStopBits = 1,BYTE byByteSize  = 8);
	HRESULT Start();
	HRESULT Stop();
	HRESULT UnInit();

	static unsigned __stdcall ThreadFn(void*pvParam);
	//-- helper fn.
 	HRESULT CanProcess();
	void OnSetDebugOption(long  iOpt,BOOL bOnOff);
};
#endif

#endif
