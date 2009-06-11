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
#include <errno.h>
#include <fcntl.h>
#include <getfiles.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <getfiles.h>
#include <loader_common.h>
#include <ms1_loader.h>
#include <stdlib.h>
#include <string.h>
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

void boot_jumper_prompt(void);

void do_ms1_load(gint port_fd, gint file_fd)
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
		//output("Please jump the boot jumper on the ECU and power cycle it\n\nPress any key to continue\n",FALSE);
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
		exit (-1);
	}
	upload_firmware(port_fd,file_fd);
	output("Firmware upload completed...\n",FALSE);
	reboot_ecu(port_fd);
	output("ECU reboot complete\n",FALSE);

	return;
}


EcuState detect_ecu(gint fd)
{
	gint res = 0;
	gint size = 1024;
	guchar buf[1024];
	guchar *ptr = buf;
	gint total_read = 0;
	gint total_wanted = 0;
	gint zerocount = 0;
	gchar  *message = NULL;

	/* Probe for response 
	 * First check for signature (running state)
	 * If that fails, see if we are in bootloader mode already
	 */

	res = write (fd,"S",1);
	flush_serial(fd,BOTH);
	if (res != 1)
		output("Failure sending signature request!\n",FALSE);
	g_usleep(300000); /* 300ms timeout */
	total_read = 0;
	total_wanted = size;
	zerocount = 0;
	while ((total_read < total_wanted ) && (total_wanted-total_read) > 0 )
	{
		total_read += res = read(fd,
				ptr+total_read,
				total_wanted-total_read);

		/* If we get nothing back (i.e. timeout, assume done)*/
		if (res <= 0)
			zerocount++;

		if (zerocount > 1)
			break;
	}
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



gboolean jump_to_bootloader(gint fd)
{
	gint res = 0;

	flush_serial(fd,OUTBOUND);
	res = write (fd,"!!",2);
	g_usleep(100000); /* 100ms timeout  */
	if (res != 2)
	{
		output("Error trying to get \"Boot>\" Prompt,\n",FALSE);
		return FALSE;
	}

	return TRUE;
}


gboolean prepare_for_upload(gint fd)
{
	gint res = 0;
	gchar buf[1024];
	gchar * message = NULL;

	res = write(fd,"W",1);
	if (res != 1)
	{
		output("Error trying to initiate ECU wipe\n",FALSE);
		return FALSE;
	}
	flush_serial(fd,OUTBOUND);
	g_usleep(1000000); /* 1000ms timeout for flash to erase */
	res = read(fd,&buf,1024);
	message = g_strndup(((gchar *)buf),res);
	if (g_strrstr_len(buf,res,"Complete"))
	{
		g_free(message);
		output("ECU Wipe complete\n",FALSE);
		res = write(fd,"U",1);
		if (res != 1)
		{
			output("Error trying to initiate ECU upgrade\n",FALSE);
			return FALSE;
		}
		flush_serial(fd,OUTBOUND);
		g_usleep(2000000); /* 2000ms timeout for flash to erase */
		res = read(fd,&buf,1024);
		if (g_strrstr_len(buf,res,"waiting"))
		{
			output("Ready to update ECU firmware\n",FALSE);
			return TRUE;
		}
		else
		{
			message = g_strndup(buf,res);
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


void upload_firmware(gint fd, gint file_fd)
{
	gint res = 0;
	gchar buf[128];
	gint chunk = 128;
	gint i = 0;
	GTimeVal last;
	GTimeVal now;
	GTimeVal begin;
	gfloat elapsed = 0.0;
	gint rate = 0;

	g_get_current_time(&begin);
	g_get_current_time(&now);
	res = read(file_fd,buf,chunk);
	while (res > 0)
	{
		last = now;
		g_get_current_time(&now);
		i+=res;
		write (fd,buf,chunk);
		elapsed = now.tv_usec - last.tv_usec;
		if (elapsed < 0)
			elapsed += 1000000;
		rate = (chunk*1000000)/elapsed;
		output(g_strdup_printf("%6i bytes written, %i bytes/sec.\n",i,rate),TRUE);
		res = read(file_fd,buf,chunk);
	}
	flush_serial(fd,BOTH);
	g_get_current_time(&now);
	output(g_strdup_printf("Upload completed in %li Seconds\n",now.tv_sec-begin.tv_sec),TRUE);

	return;
}


void reboot_ecu(gint fd)
{
	gint res = 0;

	output("Sleeping 3 Seconds\n",FALSE);
	g_usleep(3000000);
	res = write (fd,"X",1);
	flush_serial(fd,OUTBOUND);
	if (res != 1)
		output("Error trying to Reboot ECU\n",FALSE);

	return ;
}


