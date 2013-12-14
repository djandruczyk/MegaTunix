// winserialbuffer.cpp: implementation of the CSerialBuffer class.
//
//////////////////////////////////////////////////////////////////////

#ifdef __WIN32__
#include <defines.h>
#include <debugging.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <windows.h>
#include <winserialbuffer.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerialBuffer::CSerialBuffer()
{
	Init();
}

void CSerialBuffer::Init()
{
	g_mutex_init(lock);
	lock_always = true;
 	int_cur_pos = 0;
	bytes_unread = 0;
	InternalBuffer.erase ();
}


CSerialBuffer::~CSerialBuffer()
{
	g_mutex_clear(lock);
}


void CSerialBuffer::AddData( char ch )
{
	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) AddData(char) cled "), GetCurrentThreadId ());	
	InternalBuffer += ch;
	bytes_unread += 1;
}


void CSerialBuffer::AddData( std::string& Data,int iLen ) 
{
	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) AddData(%s,%d) cled "), GetCurrentThreadId (),Data.c_str (),iLen);	
	InternalBuffer.append ( Data.c_str () ,iLen);
	bytes_unread += iLen;
}


void CSerialBuffer::AddData( char *strData,int iLen ) 
{
	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) AddData(char*,%d) cled "), GetCurrentThreadId (),iLen);	
	g_assert ( strData != NULL );
	InternalBuffer.append ( strData,iLen);
	bytes_unread += iLen;
}


void CSerialBuffer::AddData( std::string &Data ) 
{
	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) AddData(%s) cled "), GetCurrentThreadId (),Data.c_str () );	
	InternalBuffer += Data;
	bytes_unread += Data.size ();
}


void CSerialBuffer::Flush()
{
	LockBuffer();
	InternalBuffer.erase ();
	bytes_unread = 0;
	int_cur_pos  = 0;
	UnLockBuffer();
}


long CSerialBuffer::Read_N( std::string &Data,long  Count ,HANDLE & EventToReset)
{
	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) Read_N() cled "), GetCurrentThreadId ());	
	g_assert ( EventToReset != INVALID_HANDLE_VALUE ) ;
	
	LockBuffer();
	long TempCount = min( Count, bytes_unread) ;
	long ActuSize = GetSize();
   	
	Data.append (InternalBuffer,int_cur_pos ,TempCount);
	
	int_cur_pos += TempCount ;
	
	bytes_unread -= TempCount;
	if (bytes_unread == 0 )
	{
		ClearAndReset ( EventToReset );
	}
 
	UnLockBuffer();
	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) Read_N returned %d bytes (data:%s)  "), GetCurrentThreadId (),TempCount,((Data)).c_str());	
	return TempCount;
}


bool CSerialBuffer::Read_Available( std::string &Data,HANDLE & EventToReset)
{
	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) Read_Upto cled "), GetCurrentThreadId ());

	LockBuffer();
	Data += InternalBuffer ;

	ClearAndReset ( EventToReset );

	UnLockBuffer();

	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) Read_Available returned (data:%s)  "), GetCurrentThreadId (),((Data)).c_str());	
	return ( Data.size () > 0 );
}


void CSerialBuffer::ClearAndReset(HANDLE& EventToReset)
{
	InternalBuffer.erase();
	bytes_unread = 0;
	int_cur_pos = 0;
	::ResetEvent ( EventToReset );
}

bool CSerialBuffer::Read_Upto( std::string &Data,char chTerm,long &bytes_read ,HANDLE & EventToReset)
{
	return Read_Upto_FIX(Data,chTerm,bytes_read ,EventToReset);

	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) Read_Upto cled "), GetCurrentThreadId ());

	LockBuffer();

	bytes_read = 0 ;
	bool Found = false;
	if ( bytes_unread > 0 ) 
	{ //if there are some bytes un-read...

		int ActualSize = GetSize ();

		for ( int i = int_cur_pos ; i < ActualSize; ++i )
		{
			bytes_read++;
			Data.append ( InternalBuffer,i,1);
			bytes_unread -= 1;
			if ( InternalBuffer[i] == chTerm) 
			{
				Found = true;
				break;
			}
		}
		if ( bytes_unread == 0 ) 
		{
			ClearAndReset ( EventToReset );
		}
		else 
		{ 
			/*
			  If we are here it means that there is 
			  still some data in the loc buffer and
			  we have ready found what we want... 
			  maybe this is ok but we want to catch this
			  scenario --- fix is in TCP/ip SocketBuffer.
			  */
			g_assert(0); 
		} 
	}

	UnLockBuffer();
	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) Read_Upto returned %d bytes (data:%s)  "), GetCurrentThreadId (),bytes_read,((Data)).c_str());	
	return Found;
}


bool CSerialBuffer::Read_Upto_FIX(std::string &Data,char chTerm,long  &bytes_read ,HANDLE & EventToReset)
{
	MTXDBG (CRITICAL,_("CSerialBuffer : (tid:%d) Read_Upto cled "), GetCurrentThreadId ());

	LockBuffer();
	bytes_read = 0 ;

	bool Found = false;
	if ( bytes_unread > 0 ) 
	{ //if there are some bytes un-read...

		int ActualSize = GetSize ();
		int iIncrementPos = 0;
		for ( int i = int_cur_pos ; i < ActualSize; ++i )
		{
			//Data .append ( InternalBuffer,i,1);
			Data+=InternalBuffer[i];
			bytes_unread -= 1;
			if ( InternalBuffer[i] == chTerm) 
			{
				iIncrementPos++;
				Found = true;
				break;
			}
			iIncrementPos++;
		}
		int_cur_pos += iIncrementPos;
		if ( bytes_unread == 0 ) 
		{
			ClearAndReset ( EventToReset );
		}
	}
	UnLockBuffer();	
	return Found;
}

#endif
