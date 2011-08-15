/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \file msloader/winserialio.c
  \ingroup Loader
  \brief Windows specific serial init routines
  \author David Andruczyk
  */

#include <winserialio.h>
#include <stdio.h>
#ifdef __WIN32__
 #include <io.h>
 #include <windows.h>
#endif


/*!
 \brief win32_setup_serial_params() sets up the serial port attributes for win32
 by setting things basically for 8N1, no flow, no escapes, etc....
 */
void win32_setup_serial_params(gint fd, gint baud, gint bits, Parity parity, gint stop)
{
#ifdef __WIN32__
	DCB dcb;
	WSADATA wsaData;
	gint res = 0;
	COMMTIMEOUTS timeouts;

	ZeroMemory(&dcb, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);

	/* Populate struct with defaults from windows */
	GetCommState((HANDLE) _get_osfhandle(fd), &dcb);

	if (baud == 9600)
		dcb.BaudRate = CBR_9600;
	else if (baud == 115200)
		dcb.BaudRate = CBR_115200;
	dcb.ByteSize = bits;
	switch (parity)
	{
		case NONE:
			dcb.Parity   = NOPARITY;
			dcb.fParity = FALSE;            /* Disabled */
			break;
		case ODD:
			dcb.Parity   = ODDPARITY;
			dcb.fParity = TRUE;             /* Enabled */
			break;
		case EVEN:
			dcb.Parity   = EVENPARITY;
			dcb.fParity = TRUE;             /* Enabled */
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
	dcb.wReserved = FALSE;		/* as per msdn */

	/* Set the port properties and write the string out the port. */
	if(SetCommState((HANDLE) _get_osfhandle (fd) ,&dcb) == 0)
		printf(__FILE__": win32_setup_serial_params()\n\tERROR setting serial attributes\n");

	/* Set timeout params in a fashion that mimics linux behavior */

	GetCommTimeouts((HANDLE) _get_osfhandle (fd), &timeouts);
	/*
	   if (baud == 112500)
	   timeouts.ReadTotalTimeoutConstant    = 250;
	   else
	   timeouts.ReadTotalTimeoutConstant    = 100;
	   timeouts.ReadTotalTimeoutMultiplier  = 1;
	   timeouts.WriteTotalTimeoutMultiplier = 1;
	   timeouts.WriteTotalTimeoutConstant   = 25;
	 */

	timeouts.ReadIntervalTimeout         = 0;
	timeouts.ReadTotalTimeoutConstant    = 100;
	timeouts.ReadTotalTimeoutMultiplier  = 1;
	timeouts.WriteTotalTimeoutConstant   = 100;
	timeouts.WriteTotalTimeoutMultiplier = 1;

	SetCommTimeouts((HANDLE) _get_osfhandle (fd) ,&timeouts);

	res = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (res != 0)
		printf(_("WSAStartup failed: %d\n"),res);
	return;
#endif
}


/*!
 \brief win32_flush_serial() is used to flush the serial port.  It effectively
 does the same thing as "tcflush()". and a wrapper function is used to call
 this or tcflush depending what OS we are compiled for.
 \param fd (integer) filedescriptor to flush
 \param mode (integer enum) either TCIFLUSH (input flush), TCOFLUSH (output flush), 
 or TCIOFLUSH (both input and output flush).
 */
void win32_flush_serial(int fd, FlushDirection mode)
{
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
    return;
}
