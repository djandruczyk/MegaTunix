// winserialcommhelper.cpp: implementation of the CSerialCommHelper class.
//
//////////////////////////////////////////////////////////////////////
#ifdef __WIN32__

#include <defines.h>
#include <debugging.h>
#include <winserialcommhelper.h>
#include <process.h>
#include <string>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


void CSerialCommHelper::InvalidateHandle(HANDLE& Handle )
{
	Handle = INVALID_HANDLE_VALUE;
}


void CSerialCommHelper::CloseAndCleanHandle(HANDLE& Handle)
{
	BOOL res  = CloseHandle( Handle ) ;
	if ( !res )
	{
		g_assert(0);
		MTXDBG(CRITICAL,_( "CSerialCommHelper : Failed to open Close Handle %d :Last Error: %d"),Handle,GetLastError());
	}
	InvalidateHandle ( Handle );
}


CSerialCommHelper::CSerialCommHelper()
{
	InvalidateHandle( ThreadTerm );
	InvalidateHandle( Thread	);
	InvalidateHandle( ThreadStarted );
	InvalidateHandle( CommPort );
	InvalidateHandle( DataRx );

	InitLock();
	state = SS_UnInit;

}


CSerialCommHelper::~CSerialCommHelper()
{
	state = SS_Unknown;
	DelLock();
}


HRESULT CSerialCommHelper:: Init(std::string PortName, DWORD BaudRate,BYTE byParity,BYTE byStopBits,BYTE byByteSize)
{
	HRESULT hr = S_OK;
	try
	{
		DataRx  = CreateEvent(0,0,0,0);
		//open the COM Port
		CommPort = ::CreateFile(PortName.c_str (),
				GENERIC_READ|GENERIC_WRITE,//access ( read and write)
				0,	//(share) 0:cannot share the COM port						
				0,	//security  (None)				
				OPEN_EXISTING,// creation : open_existing
				FILE_FLAG_OVERLAPPED,// we want overlapped operation
				0// no templates file for COM port...
				);
		if ( CommPort == INVALID_HANDLE_VALUE )
		{
			MTXDBG(CRITICAL,_( "CSerialCommHelper : Failed to open COM Port Reason: %d"),GetLastError());
			g_assert ( 0 );
			return E_FAIL;
		}

		MTXDBG(CRITICAL,_( "CSerialCommHelper : COM port opened successfully"));

		//now start to read but first we need to set the COM port settings and the timeouts
		if (! ::SetCommMask(CommPort,EV_RXCHAR|EV_TXEMPTY) )
		{
			g_assert(0);
			MTXDBG(CRITICAL,_("CSerialCommHelper : Failed to Set Comm Mask Reason: %d"),GetLastError());
			return E_FAIL;
		}
		MTXDBG(CRITICAL,_( "CSerialCommHelper : SetCommMask() success"));

		//now we need to set baud rate etc,
		DCB dcb = {0};

		dcb.DCBlength = sizeof(DCB);

		if (!::GetCommState (CommPort,&dcb))
		{
			MTXDBG(CRITICAL,_("CSerialCommHelper : Failed to Get Comm State Reason: %d"),GetLastError());
			return E_FAIL;
		}

		dcb.BaudRate = BaudRate;
		dcb.ByteSize = byByteSize;
		dcb.Parity = byParity;
		if ( byStopBits == 1 )
			dcb.StopBits = ONESTOPBIT;
		else if (byStopBits == 2 ) 
			dcb.StopBits = TWOSTOPBITS;
		else 
			dcb.StopBits = ONE5STOPBITS;

		dcb.fDsrSensitivity = 0;
		//		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fOutxDsrFlow = 0;

		if (!::SetCommState (CommPort,&dcb))
		{
			g_assert(0);
			MTXDBG(CRITICAL,_("CSerialCommHelper : Failed to Set Comm State Reason: %d"),GetLastError());
			return E_FAIL;
		}
		MTXDBG(CRITICAL,_("CSerialCommHelper : Current Settings, (Baud Rate %d; Parity %d; Byte Size %d; Stop Bits %d"), dcb.BaudRate, dcb.Parity,dcb.ByteSize,dcb.StopBits);

		//now set the timeouts ( we control the timeout overselves using WaitForXXX()
		COMMTIMEOUTS timeouts;

		timeouts.ReadIntervalTimeout		= MAXDWORD; 
		timeouts.ReadTotalTimeoutMultiplier	= 0;
		timeouts.ReadTotalTimeoutConstant	= 0;
		timeouts.WriteTotalTimeoutMultiplier	= 0;
		timeouts.WriteTotalTimeoutConstant	= 0;

		if (!SetCommTimeouts(CommPort, &timeouts))
		{
			g_assert(0);
			MTXDBG(CRITICAL,_("CSerialCommHelper :  Error setting time-outs. %d"),GetLastError());
			return E_FAIL;
		}
		//create thread terminator event...
		ThreadTerm = CreateEvent(0,0,0,0);
		ThreadStarted = CreateEvent(0,0,0,0);

		Thread = (HANDLE)_beginthreadex(0,0,CSerialCommHelper::ThreadFn,(void*)this,0,0 );

		DWORD Wait = WaitForSingleObject ( ThreadStarted , INFINITE );

		g_assert ( Wait == WAIT_OBJECT_0 );

		CloseHandle(ThreadStarted);

		InvalidateHandle(ThreadStarted );
		connected = true;
	}
	catch(...)
	{
		g_assert(0);
		hr = E_FAIL;
	}
	if ( SUCCEEDED(hr) ) 
	{
		state = SS_Init;
	}
	return hr;
}
	
 
HRESULT CSerialCommHelper:: Start()
{
	state = SS_Started;
	return S_OK;
}


HRESULT CSerialCommHelper:: Stop()
{
	state = SS_Stopped;
	return S_OK;
}


HRESULT CSerialCommHelper:: UnInit()
{
	HRESULT hr = S_OK;
	try
	{
		connected = FALSE;
		SignalObjectAndWait(ThreadTerm,Thread,INFINITE,FALSE);
		CloseAndCleanHandle( ThreadTerm);
		CloseAndCleanHandle( Thread);
		CloseAndCleanHandle( DataRx );
		CloseAndCleanHandle( CommPort );
	}
	catch(...)
	{
		g_assert(0);
		hr = E_FAIL;
	}
	if ( SUCCEEDED(hr)) 
		state = SS_UnInit;
	return hr;
}


unsigned __stdcall CSerialCommHelper::ThreadFn(void*pvParam)
{

	CSerialCommHelper* CommHelper = (CSerialCommHelper*) pvParam ;
	bool condition = true;
	DWORD EventMask=0;

	OVERLAPPED ov;
	memset(&ov,0,sizeof(ov));
	ov.hEvent = CreateEvent( 0,true,0,0);
	HANDLE Handles[2];
	Handles[0] = CommHelper->ThreadTerm;

	DWORD Wait;
	SetEvent(CommHelper->ThreadStarted);
	while (  condition )
	{

		BOOL res = ::WaitCommEvent(CommHelper->CommPort,&EventMask, &ov) ;
		if ( !res )
		{

			g_assert( GetLastError () == ERROR_IO_PENDING);
		}


		Handles[1] = ov.hEvent ;

		Wait = WaitForMultipleObjects (2,Handles,FALSE,INFINITE);
		switch ( Wait )
		{
			case WAIT_OBJECT_0:
				{
					_endthreadex(1);
				}
				break;
			case WAIT_OBJECT_0 + 1:
				{
					DWORD Mask;
					if (GetCommMask(CommHelper->CommPort,&Mask) )
					{
						if ( Mask == EV_TXEMPTY )
						{
							MTXDBG(CRITICAL,_("Data sent"));
							ResetEvent ( ov.hEvent );
							continue;
						}

					}
					//read data here...
					int accum = 0;

					CommHelper->SerialBuffer.LockBuffer();

					try 
					{
						std::string Debug;
						BOOL res = FALSE;

						DWORD BytesRead = 0;
						OVERLAPPED ovRead;
						memset(&ovRead,0,sizeof(ovRead));
						ovRead.hEvent = CreateEvent( 0,true,0,0);

						do
						{
							ResetEvent( ovRead.hEvent  );
							char Tmp[1];
							int iSize  = sizeof ( Tmp );
							memset(Tmp,0,sizeof Tmp);
							res = ::ReadFile(CommHelper->CommPort,Tmp,sizeof(Tmp),&BytesRead,&ovRead);
							if (!res ) 
							{
								condition = FALSE;
								break;
							}
							if ( BytesRead > 0 )
							{
								CommHelper->SerialBuffer.AddData ( Tmp,BytesRead );
								accum += BytesRead;
							}
						}while (0);// BytesRead > 0 );
						CloseHandle(ovRead.hEvent );
					}
					catch(...)
					{
						g_assert(0);
					}

					//if we are not in started state then we should flush the queue...( we would still read the data)
					if (CommHelper->GetCurrentState() != SS_Started ) 
					{
						accum  = 0;
						CommHelper->SerialBuffer.Flush ();
					}

					CommHelper->SerialBuffer.UnLockBuffer();

					MTXDBG(CRITICAL,_("RCSeri: Q Unlocked:"));
					if ( accum > 0 )
					{
						MTXDBG(CRITICAL,_("CSerialCommHelper(worker thread):  SetDataReadEvent() len:{%d} data:{%s}"),accum,(CommHelper->SerialBuffer.GetData()).c_str ()  );
						CommHelper->SetDataReadEvent(); 
					}
					ResetEvent ( ov.hEvent );
				}
				break;
		}//switch
	}
	return 0;
}


HRESULT  CSerialCommHelper::CanProcess ()
{

	switch ( state  ) 
	{
		case SS_Unknown	:g_assert(0);return E_FAIL;
		case SS_UnInit	:return E_FAIL;
		case SS_Started :return S_OK;
		case SS_Init		:
		case SS_Stopped :
				 return E_FAIL;
		default:g_assert(0);	

	}	
	return E_FAIL;
}


HRESULT CSerialCommHelper::Write (const char* data,DWORD Size)
{
	HRESULT hr = CanProcess();
	if ( FAILED(hr)) return hr;
	OVERLAPPED ov;
	memset(&ov,0,sizeof(ov));
	ov.hEvent = CreateEvent( 0,true,0,0);
	DWORD BytesWritten = 0;
	//do
	{
		int res = WriteFile (CommPort,data,Size,&BytesWritten  ,&ov);
		if ( res == 0 )
		{
			WaitForSingleObject(ov.hEvent ,INFINITE);
		}

	}//	while ( ov.InternHigh != Size ) ;
	CloseHandle(ov.hEvent);
	std::string Data(data);
	MTXDBG(CRITICAL,_("RCSeri:Writing:{%s} len:{%d}"),(Data).c_str(),Data.size());

	return S_OK;
}


HRESULT CSerialCommHelper::Read_Upto (std::string& data,char chTerminator ,long* Count,long TimeOut)
{
	HRESULT hr = CanProcess();
	if ( FAILED(hr)) return hr;

	MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper: Read_Upto called "));
	try
	{
		std::string Tmp;
		Tmp.erase ();
		long BytesRead;

		bool Found =  SerialBuffer.Read_Upto(Tmp ,chTerminator,BytesRead,DataRx );

		if ( Found ) 
		{
			data = Tmp ;
		}
		else
		{//there are either none or less bytes...
			long iRead = 0;
			bool condition =  true;
			while (  condition )
			{
				MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper : Read_Upto () making blocking read cl  "));
				DWORD Wait  = ::WaitForSingleObject ( DataRx , TimeOut ) ;

				if  ( Wait == WAIT_TIMEOUT) 
				{
					MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper : Read_Upto () timed out in blocking read"));
					data.erase ();
					hr = E_FAIL;
					return hr;

				}

				bool Found =  SerialBuffer.Read_Upto(Tmp ,chTerminator,BytesRead,DataRx );
				if ( Found ) 
				{
					data = Tmp;
					MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper: Read_Upto WaitForSingleObject  data:{%s}len:{%d}"),((Tmp)).c_str(),Tmp.size ());
					return S_OK;
				}
				MTXDBG(CRITICAL,("CSerialCommHelper : CSerialCommHelper: Read_Upto WaitForSingleObject  not FOUND "));

			}
		}
	}
	catch(...)
	{
		MTXDBG(CRITICAL,_("CSerialCommHelperUnhandled exception"));
		g_assert ( 0  ) ;
	}
	return hr;
}


HRESULT CSerialCommHelper::Read_N (std::string& data, long Count ,long TimeOut )
{
	HRESULT hr = CanProcess();

	if ( FAILED(hr)) 
	{
		g_assert(0);
		return hr;
	}

	MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper : Read_N called for %d bytes"),Count);
	try
	{
		std::string Tmp ;
		Tmp.erase();

		MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper : Read_N (%d) locking the queue  "),Count);

		int iLoc =  SerialBuffer.Read_N(Tmp ,Count ,DataRx );

		if ( iLoc == Count ) 
		{
			data = Tmp;
		}
		else
		{//there are either none or less bytes...
			long iRead = 0;
			int iRemaining = Count - iLoc;
			while (  1 )
			{

				MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper : Read_N (%d) making blocking read() "),Count);

				DWORD Wait  = WaitForSingleObject ( DataRx , TimeOut ) ;

				if  ( Wait == WAIT_TIMEOUT ) 
				{
					MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper : Read_N (%d) timed out in blocking read"),Count);
					data.erase ();
					hr = E_FAIL;
					return hr;
				}

				g_assert ( Wait == WAIT_OBJECT_0 );
				MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper : Read_N (%d) Woke Up from WaitXXX() locking Q"),Count);

				iRead =  SerialBuffer.Read_N(Tmp , iRemaining  ,DataRx);
				iRemaining -= iRead ;

				MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper : Read_N (%d) Woke Up from WaitXXX() Unlocking Q"),Count);

				if (  iRemaining  == 0) 
				{
					MTXDBG(CRITICAL,_("CSerialCommHelper : CSerialCommHelper : Read_N (%d) Woke Up from WaitXXX() Done reading "),Count);
					data = Tmp;
					return S_OK;
				}
			}
		}
	}
	catch(...)
	{
		MTXDBG(CRITICAL,_("CSerialCommHelper Unhandled exception"));
		g_assert ( 0  ) ;
	}
	return hr;
}


/*-----------------------------------------------------------------------
	-- Reads all the data that is available in the local buffer.. 
	does NOT make any blocking calls in case the local buffer is empty
-----------------------------------------------------------------------*/
HRESULT CSerialCommHelper::ReadAvailable(std::string& data)
{

	HRESULT hr = CanProcess();
	if ( FAILED(hr)) return hr;
	try
	{
		std::string Temp;
		bool res = SerialBuffer.Read_Available (Temp,DataRx);

		data = Temp;
	}
	catch(...)
	{
		MTXDBG(CRITICAL,_("CSerialCommHelper Unhandled exception in ReadAvailable()"));
		g_assert ( 0  ) ;
		hr = E_FAIL;
	}
	return hr;
}
#endif
