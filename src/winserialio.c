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
  \file src/winserialio.c
  \ingroup CoreMtx
  \brief Windows specific serial I/O functions
  \author David Andruczyk
  */

#include <stdio.h>
#include <winserialio.h>
#ifdef __WIN32__
 #include <debugging.h>
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

	if (serial_params->open == FALSE)
		return;

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
		dbg_func(CRITICAL,g_strdup(__FILE__": win32_setup_serial_params()\n\tERROR setting serial attributes\n"));

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

/*
#ifdef __WIN32__
unsigned __stdcall winserial_thread(void *data)
{
	Serial_Params *serial_params = NULL;
	gboolean condition = TRUE;
	glong mask = 0;
	gchar buffer[128];
	glong comm_mask = 0;
	OVERLAPPED ov;
	glong wait = 0;
	gint accum = 0;
	HANDLE handles[2];
	gboolean res = FALSE;
	glong bytes_read = 0;
	OVERLAPPED ovread;

	serial_params = (Serial_Params *)data;
	memset(&ov,0,sizeof(ov));
	ov.hEvent = CreateEvent(0,TRUE,0,0);
	handles[0] = serial_params->term_evt;
	SetEvent(serial_params->started_evt);

	while (condition)
	{
		res = WaitCommEvent(serial_params->comm_handle, &mask, &ov);
		if (!res)
			g_return_val_if_fail(GetLastError() == ERROR_IO_PENDING,0);
		handles[1] = ov.hEvent;
		wait = WaitForMultipleObjects(2,handles,FALSE,INFINITE);
		switch (wait)
		{
			case WAIT_OBJECT_0:
				_endthreadex(1);
				break;
			case WAIT_OBJECT_0 + 1:
				if (GetCommMask(serial_params->comm_handle, &comm_mask))
				{
					if (comm_mask == EV_TXEMPTY)
					{
						printf("data Sent\n");
						reset_event(ov.hEvent);
						continue;
					}
				}
				// Read data here... 
				accum = 0;
				g_mutex_lock(serial_params->mutex);
				res = false;
				bytes_read = 0;
				memset(&ovread,0,sizeof(ovread));
				ovread.hEvent = CreateEvent (0,TRUE,0,0);
				do
				{
					ResetEvent(ovread.hEvent);
					res = ReadFile(serial_params->comm_handle,&buffer,128,&bytes_read,&ovread);
					if (!res)
					{
						condition = FALSE;
						break;
					}
					if (bytes_read > 0)
					{
						g_string_append_len(serial_params->buffer,buffer,bytes_read);
						accum += bytes_read;
					}
				} while(0);
				CloseHandle(ovread.hEvent);

				// If we are not in started state, then flush
				//  the queue
				//
				// This doesn't look right to me and will
				// probbaly deadlock here..
				if (serial_params->state != SS_Started)
				{
					accum = 0;
					winserial_flush_buffer(serial_params);
				}
				g_mutex_unlock(serial_params->mutex);
				if (accum > 0)
					SetEvent(serial_params->rx_event);
				ResetEvent(ov.hEvent);
				break;
		}
	}
	return 0;
}


void winserial_flush_buffer(Serial_Params *serial_params)
{
	g_mutex_lock(serial_params->mutex);
	g_string_erase(serial_params->int_buffer);
	serial_params->bytes_unread = 0;
	serial_params->int_cur_pos = 0;
	g_mutex_unlock(serial_params->mutex);
}
*/
