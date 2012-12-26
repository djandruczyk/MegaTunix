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
  \file src/winserialio.c
  \ingroup CoreMtx
  \brief Windows specific serial I/O functions
  \author David Andruczyk
  */

#include <debugging.h>
#include <stdio.h>
#include <winserialio.h>
#ifdef __WIN32__
 #include <io.h>
 #include <windows.h>
#endif

/*!
  \brief win32_setup_serial_params() sets up the serial port attributes 
  for windows platforms
  \param fd is the filedescriptor representing the serial port
  \param baud is the baud rate
  \param bits is the number of data bits
  \param parity is the enumeration describing odd, even or no parity
  \param stop is the number of stop bits
  */
G_MODULE_EXPORT void win32_setup_serial_params(gint fd, gint baud, gint bits, Parity parity, gint stop)
{
#ifdef __WIN32__
	extern gconstpointer *global_data;
	Serial_Params *serial_params;
	serial_params = DATA_GET(global_data,"serial_params");
	DCB dcb;
	COMMTIMEOUTS timeouts;
	ENTER();

	if (serial_params->open == FALSE)
	{
		EXIT();
		return;
	}

	ZeroMemory(&dcb, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);

	/* Populate struct with defaults from windows */
	GetCommState((HANDLE) _get_osfhandle(fd), &dcb);

	dcb.BaudRate = baud;
	dcb.ByteSize = bits;
	switch (parity)
	{
		case NONE:
			dcb.Parity   = NOPARITY;        
			dcb.fParity = FALSE;		/* Disabled */
			break;
		case ODD:
			dcb.Parity   = ODDPARITY;        
			dcb.fParity = TRUE;		/* Enabled */
			break;
		case EVEN:
			dcb.Parity   = EVENPARITY;     
			dcb.fParity = TRUE;		/* Enabled */
			break;
	}
	if (stop == 2)
		dcb.StopBits = TWOSTOPBITS;      /* #defined in windows.h */
	else
		dcb.StopBits = ONESTOPBIT;      /* #defined in windows.h */

	dcb.fBinary = TRUE;		/* Enable binary mode */
	dcb.fOutxCtsFlow = FALSE;	/* don't monitor CTS line */
	dcb.fOutxDsrFlow = FALSE;	/* don't monitor DSR line */
	dcb.fDsrSensitivity = FALSE;	/* ignore Dsr line */
	dcb.fDtrControl = DTR_CONTROL_DISABLE;  /* Disable DTR line */
	dcb.fRtsControl = RTS_CONTROL_DISABLE;  /* Disable RTS line */
	dcb.fOutX = FALSE;		/* Disable Xoff */
	dcb.fInX  = FALSE;		/* Disable Xin */
	dcb.fErrorChar = FALSE;		/* Don't replace bad chars */
	dcb.fNull = FALSE;		/* don't drop NULL bytes */
	dcb.fAbortOnError = FALSE;	/* Don't abort */
	dcb.wReserved = 0;		/* as per msdn */

	/* Set the port properties and write the string out the port. */
	if(SetCommState((HANDLE) _get_osfhandle (fd) ,&dcb) == 0)
		MTXDBG(CRITICAL,_("ERROR setting serial attributes\n"));

	/* Set timeout params in a fashion that mimics linux behavior */
	GetCommTimeouts((HANDLE) _get_osfhandle (fd), &timeouts);

	if (DATA_GET(global_data,"serial_nonblock"))
	{
		printf("win32 serial nonblock\n");
		/* Buffer? */
//		SetupComm((HANDLE) _get_osfhandle (fd),128,128);
		/*
		timeouts.ReadIntervalTimeout         = MAXDWORD;
		timeouts.ReadTotalTimeoutMultiplier  = 0;
		timeouts.WriteTotalTimeoutConstant   = 0;
		timeouts.WriteTotalTimeoutMultiplier = 20000L/baud;
		timeouts.ReadTotalTimeoutConstant    = 0;
		*/
		// Works, but causes deadlock in reader blocking UI 
		//timeouts.ReadTotalTimeoutConstant    = 100; /* 100ms timeout*/
		//timeouts.ReadIntervalTimeout         = MAXDWORD;
		//timeouts.ReadTotalTimeoutMultiplier  = MAXDWORD;
		//timeouts.WriteTotalTimeoutConstant   = 0;
		//timeouts.WriteTotalTimeoutMultiplier = 20000L/baud;
		timeouts.ReadIntervalTimeout         = MAXDWORD;
		timeouts.ReadTotalTimeoutMultiplier  = 0;
		timeouts.ReadTotalTimeoutConstant    = 0;
		timeouts.WriteTotalTimeoutMultiplier = 0;
		timeouts.WriteTotalTimeoutConstant   = 0;
	}
	else
	{
		timeouts.ReadIntervalTimeout         = 0;
		timeouts.ReadTotalTimeoutMultiplier  = 0;
		timeouts.WriteTotalTimeoutConstant   = 0;
		timeouts.WriteTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant    = 100;
	}

	SetCommTimeouts((HANDLE) _get_osfhandle (fd) ,&timeouts);

//	if (DATA_GET(global_data,"serial_nonblock"))
//	{
		//serial_params->rx_event = CreateEvent(0,0,0,0);
		//if (! SetCommMask(serial_params->comm_handle, EV_RXCHAR|EV_TXEMPTY))
	//		printf("Unable to set comm mask events, reason %d\n",GetLastError());
//		serial_params->term_evt = CreateEvent(0,0,0,0);
//		serial_params->started_evt = CreateEvent(0,0,0,0);
//		serial_params->thread = (HANDLE)_beginthreadex(0,0,winserial_thread,(void *)serial_params,0,0);
		/* Wait until thread has fired up */
//		tmpi = WaitForSingleObject(serial_params->started_evt, INFINITE);
//		g_return_if_fail(tmpi == WAIT_OBJECT_0);
//		CloseHandle(serial_params->started_evt);
//		InvalidateHandle(serial_params->started_evt);
//		serial_params->state = SS_Init;
//	}
	EXIT();
	return;
#endif
}


/*!
  \brief win32_flush_serial() is used to flush the serial port.  It effectively
  does the same thing as "tcflush()". and a wrapper function is used to call
  this or tcflush depending what OS we are compiled for.
  \param fd is the filedescriptor to flush
  \param mode is the enum represnting the direction to flush...
  */
G_MODULE_EXPORT void win32_flush_serial(int fd, FlushDirection mode)
{
	ENTER();
#ifdef __WIN32__
	switch (mode)
	{
		case INBOUND:
			PurgeComm((HANDLE) _get_osfhandle (fd),PURGE_RXCLEAR);
			break;
		case OUTBOUND:
			FlushFileBuffers((HANDLE) _get_osfhandle (fd));
			PurgeComm((HANDLE) _get_osfhandle (fd),PURGE_TXCLEAR);
			break;
		case BOTH:
			FlushFileBuffers((HANDLE) _get_osfhandle (fd));
			PurgeComm((HANDLE) _get_osfhandle (fd),PURGE_TXCLEAR|PURGE_RXCLEAR);
			break;
	}
#endif
	EXIT();
    return;
}

