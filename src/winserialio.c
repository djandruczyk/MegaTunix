/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <structures.h>
#include <termios.h>
#include <winserialio.h>
#ifdef __WIN32__
 #include <io.h>
 #include <windows.h>

extern struct Serial_Params *serial_params;

void win32_open_comm_port()
{

}
void win32_setup_serial_params()
{
	DCB dcb;
	COMMTIMEOUTS timeouts;
	extern gint baudrate;

	if (serial_params->open == FALSE)
		return;

	ZeroMemory(&dcb, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);

	/* Populate struct with defaults from windows */
	GetCommState((HANDLE) _get_osfhandle(serial_params->fd), &dcb);

	if(baudrate < 9600)
	{
		dbg_func(g_strdup_printf(__FILE__": win32_setup_serial_params()\n\tBaudrate incorrectly set to %i, forcing 9600\n",baudrate),CRITICAL);
		baudrate = 9600;
	}


	dcb.BaudRate = baudrate;
	dcb.ByteSize = 8;
	dcb.Parity   = NOPARITY;        // NOPARITY and friends are
	dcb.StopBits = ONESTOPBIT;      // #defined in windows.h

	dcb.fBinary = TRUE;		// Enable binary mode
	dcb.fParity = FALSE;		// Disabled
	dcb.fOutxCtsFlow = FALSE;	// don't monitor CTS line
	dcb.fOutxDsrFlow = FALSE;	// don't monitor DSR line
	dcb.fDsrSensitivity = FALSE;	// ignore Dsr line
	dcb.fDtrControl = DTR_CONTROL_DISABLE;  // Disable DTR line
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fOutX = FALSE;		// Disable Xoff
	dcb.fInX  = FALSE;		// Disable Xin
	dcb.fErrorChar = FALSE;		// Don't replcae bad chars
	dcb.fNull = FALSE;		// don't drop NULL bytes
	dcb.fAbortOnError = FALSE;	// Don't abort
	dcb.wReserved = FALSE;		// as per msdn

	// Set the port properties and write the string out the port.
	if(SetCommState((HANDLE) _get_osfhandle (serial_params->fd) ,&dcb) == 0)
		printf("ERROR setting serial attributes\n");

	/* Set timeout params in a fashion that mimics linux behavior */
	timeouts.ReadIntervalTimeout         = 0;
	timeouts.ReadTotalTimeoutConstant    = 100;
	timeouts.ReadTotalTimeoutMultiplier  = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant   = 0;
	SetCommTimeouts((HANDLE) _get_osfhandle (serial_params->fd) ,&timeouts);


	return;
}

void win32_flush_serial(int fd, int mode)
{
	switch (mode)
	{
		case TCIFLUSH:
			PurgeComm((HANDLE) _get_osfhandle (fd),PURGE_RXCLEAR);
			break;
		case TCOFLUSH:
			FlushFileBuffers((HANDLE) _get_osfhandle (fd));
			PurgeComm((HANDLE) _get_osfhandle (fd),PURGE_TXCLEAR);
			break;
		case TCIOFLUSH:
			FlushFileBuffers((HANDLE) _get_osfhandle (fd));
			PurgeComm((HANDLE) _get_osfhandle (fd),PURGE_TXCLEAR|PURGE_RXCLEAR);
			break;
	}
}

#endif
