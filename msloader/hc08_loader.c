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
  \file msloader/hc08_loader.c
  \ingroup Loader
  \brief
  \author David Andruczyk
  */

#include <hc08_loader.h>
#include <loader_common.h>
#include <unistd.h>
#ifndef __WIN32__
 #include <termios.h>
#else
#include <winserialio.h>
#endif

#ifndef CRTSCTS
#define CRTSCTS 0
#endif

/* Globals */
#ifndef __WIN32__
struct termios oldtio;
struct termios newtio;
#endif
#define POLL_ATTEMPTS 15

void boot_jumper_prompt(void);

/*!
  \brief Load the firmware to the MS-1 Device. Implementation is a simple
  state machine.
  \param port_fd is the filedescriptor to the device (serial port)
  \param file_fd is the filedescriptor to the .s19 file
  \returns True on success, False on failure
  */
gboolean do_ms1_load(gint port_fd, gint file_fd)
{
	gboolean result = FALSE;
	EcuState ecu_state = NOT_LISTENING;
	ecu_state = detect_ecu(port_fd);
	switch (ecu_state)
	{
		case NOT_LISTENING:
			output("NO response to signature request\n",FALSE);
			break;
		case IN_BOOTLOADER:
			output("ECU is in bootloader mode, good!\n",FALSE);
			break;
		case LIVE_MODE:
			output("ECU detected in LIVE! mode, attempting to access bootloader\n",FALSE);
			result = jump_to_bootloader(port_fd);
			if (result)
			{
				ecu_state = detect_ecu(port_fd);
				if (ecu_state == IN_BOOTLOADER)
				{
					output("ECU is in bootloader mode, good!\n",FALSE);
					break;
				}
				else
					output("Could NOT attain bootloader mode\n",FALSE);
			}
			else
				output("Could NOT attain bootloader mode\n",FALSE);
			break;
	}
	if (ecu_state != IN_BOOTLOADER)
	{
		/*output("Please jump the boot jumper on the ECU and power cycle it\n\nPress any key to continue\n",FALSE);*/
		boot_jumper_prompt();
		ecu_state = detect_ecu(port_fd);
		if (ecu_state != IN_BOOTLOADER)
		{
			output("Unable to get to the bootloader, update FAILED!\n",FALSE);
			boot_jumper_prompt();
		}
		else
			output("Got into the bootloader, good!\n",FALSE);

	}
	result = prepare_for_upload(port_fd);
	if (!result)
	{
		output("Failure getting ECU into a state to accept the new firmware\n",FALSE);
		return FALSE;
	}
	upload_firmware(port_fd,file_fd);
	output("Firmware upload completed...\n",FALSE);
	reboot_ecu(port_fd);
	output("ECU reboot complete\n",FALSE);

	return TRUE;
}


/*!
  \brief detect_ecu probes ecu to try and determine if in bootloader, if so
  what mode, (i.e. waiting for data, waiting for CMD), or if in normal run mode
  \param fd is the filedescriptor of serial port
  \returns the enumeration of ECU state
  \see EcuState
  */
EcuState detect_ecu(gint fd)
{
	gint res = 0;
	gint size = 1024;
	gchar buf[1024];
	gchar *ptr = buf;
	gint total_read = 0;
	gchar  *message = NULL;

	/* Probe for response 
	 * First check for signature (running state)
	 * If that fails, see if we are in bootloader mode already
	 */

	flush_serial(fd,BOTH);
	res = write_wrapper (fd,(guchar *)"S",1);
	flush_serial(fd,OUTBOUND);
	if (res != 1)
		output("Failure sending signature request!\n",FALSE);
	total_read = read_wrapper(fd,buf,32);
	flush_serial(fd,BOTH);
	if (total_read > 0)
	{
		message = g_strndup(((gchar *)buf),total_read);
		/* Check for "what" or "Boot" */
		if (g_strrstr_len(message,total_read, "what"))
		{
			g_free(message);
			return IN_BOOTLOADER;
		}
		else if (g_strrstr_len(message,total_read, "Boot"))
		{
			g_free(message);
			return IN_BOOTLOADER;
		}
		else	
		{
			output(g_strdup_printf("ECU signature: \"%s\"\n",message),TRUE);
			g_free(message);
			return  LIVE_MODE;
		}
	}

	return NOT_LISTENING;
}



/*!
  \brief jump_to_bootloader sends the magic string to get to the 
  bootloader from normal run mode
  \param fd is the filedescriptor of serial port
  \returns True if we get a Boot prompt, false otherwise
  */
gboolean jump_to_bootloader(gint fd)
{
	gint res = 0;

	flush_serial(fd,OUTBOUND);
	res = write_wrapper (fd,(guchar *)"!!",2);
	if (res != 2)
	{
		output("Error trying to get \"Boot>\" Prompt,\n",FALSE);
		return FALSE;
	}

	return TRUE;
}


/*!
  \brief prepare_for_upload Gets the ECU into a state to accept the S19 file
  \param fd is the filedescriptor of serial port
  \returns True if we are ready, false otherwise
  */
gboolean prepare_for_upload(gint fd)
{
	gint res = 0;
	guchar buf[1024];
	gchar * message = NULL;

	res = write_wrapper(fd,(guchar *)"W",1);
	if (res != 1)
	{
		output("Error trying to initiate ECU wipe\n",FALSE);
		return FALSE;
	}
	flush_serial(fd,OUTBOUND);
	g_usleep(1000000); /* 1000ms timeout for flash to erase */
	res = read(fd,buf,1024);
	message = g_strndup(((gchar *)buf),res);
	if (g_strrstr_len((gchar *)buf,res,"Complete"))
	{
		g_free(message);
		output("ECU Wipe complete\n",FALSE);
		res = write_wrapper(fd,(guchar *)"U",1);
		if (res != 1)
		{
			output("Error trying to initiate ECU upgrade\n",FALSE);
			return FALSE;
		}
		flush_serial(fd,OUTBOUND);
		g_usleep(2000000); /* 2000ms timeout for flash to erase */
		res = read(fd,buf,1024);
		if (g_strrstr_len((gchar *)buf,res,"waiting"))
		{
			output("Ready to update ECU firmware\n",FALSE);
			return TRUE;
		}
		else
		{
			message = g_strndup((gchar *)buf,res);
			output(g_strdup_printf("ECU returned \"%s\"\n",message),TRUE);	
			g_free(message);
			output("Error getting \"ready to update\" message from ECU\n",FALSE);
			return FALSE;
		}
	}
	else
	{
		output(g_strdup_printf("Error wiping ECU, result \"%s\"\n",message),TRUE);
		g_free(message);
		return FALSE;
	}
}


/*!
  \brief
  upload_firmware handles the actual sending of the .s19 file to the ECU
  \param fd is the serial port filedescriptor
  \param file_fd is the .s19 file filedescriptor
  */
void upload_firmware(gint fd, gint file_fd)
{
	gint res = 0;
	guchar buf[128];
	gint chunk = 128;
	gint result = 0;
	gint i = 0;
	GTimeVal last;
	GTimeVal now;
	GTimeVal begin;
	gfloat elapsed = 0.0;
	gint rate = 0;
	gint end = 0;

	g_get_current_time(&begin);
	g_get_current_time(&now);
	end = lseek(file_fd,0,SEEK_END);
	lseek(file_fd,0,SEEK_SET);
	res = read(file_fd,buf,chunk);
	output(g_strdup_printf("Uploading ECU firmware, eta %i seconds...\n",end/960),TRUE);
	while (res > 0)
	{
		last = now;
		g_get_current_time(&now);
		i+=res;
		result = write_wrapper (fd,buf,chunk);
		if (result != chunk)
		{
			output(g_strdup_printf("Write error, tried to send %i bytes, but only managed to send %i bytes.\n",chunk,result),TRUE);
		}
		elapsed = now.tv_usec - last.tv_usec;
		if (elapsed < 0)
			elapsed += 1000000;
		rate = (chunk*1000000)/elapsed;
		/*output(g_strdup_printf("%6i bytes written, %i bytes/sec.\n",i,rate),TRUE);*/
		res = read(file_fd,buf,chunk);
		progress_update ((gfloat)i/(gfloat)(end-2));

	}
	flush_serial(fd,BOTH);
	g_get_current_time(&now);
	output(g_strdup_printf("Upload completed in %li Seconds at %i bytes/sec\n",now.tv_sec-begin.tv_sec,rate),TRUE);

	return;
}


/*!
  \brief reboot_ecu tells the ECU to reboot
  \param fd is the serial port filedescriptor
  */
void reboot_ecu(gint fd)
{
	gint res = 0;

	output("Sleeping 3 Seconds\n",FALSE);
	g_usleep(3000000);
	res = write_wrapper (fd,(guchar *)"X",1);
	flush_serial(fd,OUTBOUND);
	if (res != 1)
		output("Error trying to Reboot ECU\n",FALSE);

	return;
}
