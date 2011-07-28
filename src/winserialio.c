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

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */

#include <stdio.h>
#include <winserialio.h>
#ifdef __WIN32__
 #include <debugging.h>
 #include <io.h>
 #include <windows.h>
#endif


/*!
 \brief win32_setup_serial_params() sets up the serial port attributes for win32
 \param fd, filedescriptor representing the serial port
 \param baud, baud rate
 \param bits, number of data bits
 \param parity, enumeration describign odd, even or no parity
 \param stop, number of stop bits
 */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT void win32_setup_serial_params(gint fd, gint baud, gint bits, Parity parity, gint stop)
{
#ifdef __WIN32__
	extern gconstpointer *global_data;
	Serial_Params *serial_params;
	serial_params = DATA_GET(global_data,"serial_params");
	DCB dcb;
	COMMTIMEOUTS timeouts;

	if (serial_params->open == FALSE)
		return;

	ZeroMemory(&dcb, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);

	/* Populate struct with defaults from windows */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	GetCommState((HANDLE) _get_osfhandle(fd), &dcb);

	/*
	if (baud == 9600)
		dcb.BaudRate = CBR_9600;
	else if (baud == 115200)
		dcb.BaudRate = CBR_115200;
		*/

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.BaudRate = baud;
	dcb.ByteSize = bits;
	switch (parity)
	{
		case NONE:
			dcb.Parity   = NOPARITY;        
			dcb.fParity = FALSE;		/* Disabled */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
			break;
		case ODD:
			dcb.Parity   = ODDPARITY;        
			dcb.fParity = TRUE;		/* Enabled */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
			break;
		case EVEN:
			dcb.Parity   = EVENPARITY;     
			dcb.fParity = TRUE;		/* Enabled */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
			break;
	}
	if (stop == 2)
		dcb.StopBits = TWOSTOPBITS;      /* #defined in windows.h */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	else
		dcb.StopBits = ONESTOPBIT;      /* #defined in windows.h */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */

	dcb.fBinary = TRUE;		/* Enable binary mode */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fOutxCtsFlow = FALSE;	/* don't monitor CTS line */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fOutxDsrFlow = FALSE;	/* don't monitor DSR line */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fDsrSensitivity = FALSE;	/* ignore Dsr line */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fDtrControl = DTR_CONTROL_DISABLE;  /* Disable DTR line */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fRtsControl = RTS_CONTROL_DISABLE;  /* Disable RTS line */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fOutX = FALSE;		/* Disable Xoff */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fInX  = FALSE;		/* Disable Xin */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fErrorChar = FALSE;		/* Don't replace bad chars */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fNull = FALSE;		/* don't drop NULL bytes */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.fAbortOnError = FALSE;	/* Don't abort */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	dcb.wReserved = 0;		/* as per msdn */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */

	/* Set the port properties and write the string out the port. */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	if(SetCommState((HANDLE) _get_osfhandle (fd) ,&dcb) == 0)
		dbg_func(CRITICAL,g_strdup(__FILE__": win32_setup_serial_params()\n\tERROR setting serial attributes\n"));

	/* Set timeout params in a fashion that mimics linux behavior */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
	GetCommTimeouts((HANDLE) _get_osfhandle (fd), &timeouts);

	if (DATA_GET(global_data,"serial_nonblock"))
	{
		printf("nonblock\n");
		/* Buffer? */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
//		SetupComm((HANDLE) _get_osfhandle (fd),128,128);
		/*
		timeouts.ReadIntervalTimeout         = MAXDWORD;
		timeouts.ReadTotalTimeoutMultiplier  = 0;
		timeouts.WriteTotalTimeoutConstant   = 0;
		timeouts.WriteTotalTimeoutMultiplier = 20000L/baud;
		timeouts.ReadTotalTimeoutConstant    = 0;
		*/

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
		// Works, but causes deadlock in reader blocking UI 
		timeouts.ReadIntervalTimeout         = MAXDWORD;
		timeouts.ReadTotalTimeoutMultiplier  = MAXDWORD;
		timeouts.WriteTotalTimeoutConstant   = 0;
		timeouts.WriteTotalTimeoutMultiplier = 20000L/baud;
		timeouts.ReadTotalTimeoutConstant    = 100; /* 100ms timeout*/

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
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


	return;
#endif
}


/*!
 \brief win32_flush_serial() is used to flush the serial port.  It effectively
 does the same thing as "tcflush()". and a wrapper function is used to call
 this or tcflush depending what OS we are compiled for.
 \param fd, filedescriptor to flush
 \param mode, enum represnting the direction to flush...
 */

/*! @file winserialio.c
 *
 * @brief ...
 *
 *
 */
G_MODULE_EXPORT void win32_flush_serial(int fd, FlushDirection mode)
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
