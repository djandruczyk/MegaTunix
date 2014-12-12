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
  \file msloader/s12x_loader.c
  \ingroup Loader
  \brief Motorola S12x based loader code. 
  \author David Andruczyk
  */

#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0
#endif

#include <loader_common.h>
#include <s12x_loader.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

#define C_WAKE_UP        0x0D
#define C_READ_BYTE      0xA1
#define C_WRITE_BYTE     0xA2
#define C_READ_WORD      0xA3
#define C_WRITE_WORD     0xA4
#define C_READ_NEXT      0xA5
#define C_WRITE_NEXT     0xA6
#define C_READ_BLOCK     0xA7
#define C_WRITE_BLOCK    0xA8
#define MAX_BLOCK        256 /* For both writes and reads.*/
#define C_READ_REGS      0xA9
#define C_WRITE_SP       0xAA
#define C_WRITE_PC       0xAB
#define C_WRITE_IY       0xAC
#define C_WRITE_IX       0xAD
#define C_WRITE_D        0xAE
#define C_WRITE_CCR      0xAF

#define C_GO             0xB1
#define C_TRACE1         0xB2
#define C_HALT           0xB3
#define C_RESET          0xB4
#define C_ERASE_ALL      0xB6
#define C_DEVICE_INFO    0xB7
#define C_ERASE_PAGE     0xB8
#define C_ERASE_EEPROM   0xB9

/*--  Error codes  ----------------------------------------------------------*/

#define E_NONE           0xE0
#define E_COMMAND        0xE1
#define E_NOT_RUN        0xE2
#define E_SP_RANGE       0xE3
#define E_SP_INVALID     0xE4
#define E_READ_ONLY      0xE5
#define E_FACCERR        0xE6
#define E_ACCERR         0xE9

/*--  Status codes  ---------------------------------------------------------*/

#define S_ACTIVE         0x00
#define S_RUNNING        0x01
#define S_HALTED         0x02
#define S_TRACE1         0x04
#define S_COLD_RESET     0x08
#define S_WARM_RESET     0x0C

static guint total_bytes = 0;
static char **fileBuf = NULL;
static guint count = 0;
static guint verify_failure_count = 0;
static guint verify_retry_success_count = 0;
static int debug = 0;
static gint s19_length = 0;
static gboolean verify_writes = TRUE;
/* debug levels
0 = Quiet
1 = Some progress
2 = Full progress
3 = + serial comms
4 = + the s19 file as parsed
5 = + comments
*/

gboolean do_ms2_load(gint port_fd, gint file_fd)
{
	GTimeVal begin;
	GTimeVal end;

	/* causes issues with FF80-FFFF 
	* verify_writes = FALSE;
	*/
	total_bytes = 0;
	flush_serial(port_fd, BOTH);
	s19_length = lseek(file_fd,0,SEEK_END);
	lseek(file_fd,0,SEEK_SET);
	count = read_s19(file_fd);
	if (count == 0)
		return FALSE;
	g_get_current_time(&begin);
	ms2_enter_boot_mode(port_fd);
	if (!wakeup_S12(port_fd))
	{
		output((gchar *)"Unable to wakeup device!\nCheck to make sure it's present\n",FALSE);
		free_s19(count);
		return FALSE;
	}
	/* Erase the full MS2 even though we do it by page later... */
	if (!erase_S12(port_fd))
	{
		output((gchar *)"Unable to ERASE device!\n",FALSE);
		free_s19(count);
		return FALSE;
	}
	if (!send_S12(port_fd,count))
	{
		output((gchar *)"Unable to Send firmware to device!\n",FALSE);
		free_s19(count);
		return FALSE;
	}
	free_s19(count);
	reset_proc(port_fd);
	g_get_current_time(&end);
	output(g_strdup_printf("Wrote %d bytes in %i seconds (%.1f Bps)\nwith %i verify errors, and %i successful recoveries.\n", total_bytes,(int)(end.tv_sec-begin.tv_sec),total_bytes/(gfloat)(end.tv_sec-begin.tv_sec),verify_failure_count,verify_retry_success_count),TRUE);
	if ((verify_failure_count > 0) && 
			(verify_failure_count != verify_retry_success_count))
		output((gchar *)"ERRORS REPORTED! The load had issues,\nYou should retry the load\n",FALSE);
	else if ((verify_failure_count > 0) && 
			(verify_failure_count == verify_retry_success_count))
		output((gchar *)"ERRORS REPORTED! However recovery WORKED, so its recommended\nthat you retry the load, but it MIGHT work.. YMMV\n",FALSE);
	else
		output((gchar *)"ALL DONE! Remove boot jumper if jumpered and power cycle ECU\n",FALSE);
	verify_failure_count = 0;
	verify_retry_success_count = 0;
	return TRUE;
}

gboolean do_libreems_load(gint port_fd, gint file_fd)
{
	GTimeVal begin;
	GTimeVal end;
	total_bytes = 0;
	flush_serial(port_fd, BOTH);
	count = read_s19(file_fd);
	if (count == 0)
		return FALSE;
	g_get_current_time(&begin);
	if (!wakeup_S12(port_fd))
	{
		output((gchar *)"Unable to wakeup device!\nCheck to make sure it's present\n",FALSE);
		free_s19(count);
		return FALSE;
	}
	/*
	if (!erase_S12(port_fd))
	{
		output((gchar *)"Unable to ERASE device!\n",FALSE);
		free_s19(count);
		return FALSE;
	}
	*/
	if (!send_S12(port_fd,count))
	{
		output((gchar *)"Unable to Send firmware to device!\n",FALSE);
		free_s19(count);
		return FALSE;
	}
	free_s19(count);
	reset_libreems_proc(port_fd);
	g_get_current_time(&end);
	output(g_strdup_printf("Wrote %d bytes in %i seconds (%.1f Bps)\nwith %i verify errors, and %i successful recoveries.\n", total_bytes,(int)(end.tv_sec-begin.tv_sec),total_bytes/(gfloat)(end.tv_sec-begin.tv_sec),verify_failure_count,verify_retry_success_count),TRUE);
	if ((verify_failure_count > 0) && 
			(verify_failure_count != verify_retry_success_count))
		output((gchar *)"ERRORS REPORTED! The load had issues,\nYou should retry the load\n",FALSE);
	else if ((verify_failure_count > 0) && 
			(verify_failure_count == verify_retry_success_count))
		output((gchar *)"ERRORS REPORTED! However recovery WORKED,so it is recommended\nthat you retry the load, but it MIGHT work.. YMMV\n",FALSE);
	else
		output((gchar *)"ALL DONE! Remove boot jumper or reset load/run switch\nand power cycle ECU...\n",FALSE);
	verify_failure_count = 0;
	verify_retry_success_count = 0;
	return TRUE;
}

void ms2_chomp(gchar *inBuf)
{
	gchar *s;

	s = strrchr(inBuf, '\n');
	if (s)
		*s = 0;
	s = strrchr(inBuf, '\r');
	if (s)
		*s = 0;
}

gint read_s19 (gint file_fd)
{
	guint lines = 0;
	gchar *buf = NULL;
	gsize len = 0;
	GIOChannel *chan = NULL;
	GError *err = NULL;


#ifdef __WIN32__
	chan = g_io_channel_win32_new_fd(file_fd);
#else
	chan = g_io_channel_unix_new(file_fd);
#endif
	g_io_channel_set_encoding(chan, NULL,&err);
	count = 0;

	while(G_IO_STATUS_NORMAL == g_io_channel_read_line(chan,&buf,&len,NULL,&err)) {
		if (len > 0)	
			g_free(buf);
		lines++;
	}
	fileBuf = (gchar **)malloc(lines * sizeof(gchar *));

	/* Go to beginning of file */
	g_io_channel_seek_position(chan,0,G_SEEK_SET,&err);
	while(G_IO_STATUS_NORMAL == g_io_channel_read_line(chan,&buf,&len,NULL,&err)) {
		if (debug > 4) {
			output(g_strdup_printf("%s", buf),TRUE);
		}
		ms2_chomp(buf);
		fileBuf[count] = g_strdup(buf);
		g_free(buf);
		count++;
	}

	return count;
}

void free_s19(guint count)
{
	guint i;

	for (i = 0; i < count; i++) {
		free(fileBuf[i]);
	}

	free(fileBuf);
}

guint extract_number(gchar *data, guint nBytes)
{
	gchar number[128];

	strncpy(number, data, nBytes);
	number[nBytes] = 0;

	return strtol(number, NULL, 16);
}

guchar cs(guint l)
{
	return ((l >> 24) & 0xff) + ((l >> 16) & 0xff) + ((l >> 8) & 0xff) + ((l >> 0) & 0xff);
}

guchar extract_data(gchar *data, guint nBytes, guchar checksum, guchar *binary)
{
	guint i;

	for (i = 0; i < nBytes; i++) {
		guint n = extract_number(data +i*2, 2);
		binary[i] = (gchar)n;
		checksum += n;
	}

	return checksum;
}

gboolean wakeup_S12(gint port_fd)
{
	guchar c;
	guchar prompt;
	gint res = 0;
	int i;

	c = C_WAKE_UP;

	for (i = 0; i < 6; i++) {
		prompt = 0;
		output((gchar *)"Attempting Wakeup...\n",FALSE);
		res = write_wrapper(port_fd, &c, 1);
		if (res != 1)
			output(g_strdup_printf("wakeup_S12(): SHORT WRITE! %i of 1\n", res),TRUE);

		if (debug >= 4) {
			output(g_strdup_printf("TX: %02x\n", c),TRUE);
		}

		if (!check_status(port_fd))
		{
			output((gchar *)"Serial Read failure, try unplugging\nand replugging your Serial adapter/cable\n",FALSE);
			return FALSE;
		}
		else 
			break;
	}

	if (i > 5) {
		output((gchar *)"Could not wake up processor, try jumpering\nthe boot jumper and power cycling the ECU...\n",FALSE);
		return FALSE;
	}
	return TRUE;
}
   
gboolean check_status(gint port_fd)
{
	guchar errorCode = 0;
	guchar statusCode = 0;
	guchar prompt = 0;
	gchar buf[3];
	gint res = 0;
	gboolean retval = TRUE;

	res = read_wrapper(port_fd, buf, 3);
	if (res == -1)
	{
		output((gchar *)"READ FAILURE in check_status!\n",FALSE);
		flush_serial(port_fd, BOTH);
		return FALSE;
	}
	if (res != 3)
	{
		output(g_strdup_printf("Error reading error/status/prompt Code\nrequested 3 bytes got %i\n",res),TRUE);
		return FALSE;
	}
	errorCode = buf[0];
	statusCode = buf[1];
	prompt = buf[2];

	if (debug >= 4) {
		output(g_strdup_printf("RX: %02x %02x %02x\n", errorCode,statusCode, prompt),TRUE);
	}

	if ((errorCode == E_COMMAND) && (statusCode == S_ACTIVE) && (prompt == '>'))
	{
		output(g_strdup_printf("Loader Ready\n"),TRUE);
		return TRUE;
	}
	switch(errorCode) 
	{
		case E_NONE:
			retval = TRUE;
			break;
		case E_COMMAND: 
			retval = FALSE;
			output(g_strdup_printf("Command not recognized, e=0x%02x\n",errorCode),TRUE);
			break;
		case E_NOT_RUN: 
			retval = FALSE;
			output(g_strdup_printf("Command not allowed in run mode, e=0x%02x\n",errorCode),TRUE);
			break;
		case E_SP_RANGE:
			retval = FALSE;
			output(g_strdup_printf("Stack pointer out of range, e=0x%02x\n",errorCode),TRUE);
			break;
		case E_SP_INVALID:
			retval = FALSE;
			output(g_strdup_printf("Stack pointer invalid, e=0x%02x\n",errorCode),TRUE);
			break;
		case E_READ_ONLY:
			retval = FALSE;
			output(g_strdup_printf("Attempt to write read-only memory, e=0x%02x\n",errorCode),TRUE);
			break;
		case E_FACCERR:
			retval = FALSE;
			output(g_strdup_printf("Access error when writing FLASH/EEPROM memory, e=0x%02x\n",errorCode),TRUE);
			break;
		case E_ACCERR:
			retval = FALSE;
			output(g_strdup_printf("Memory protection violation writing EEPROM memory, e=0x%02x\n",errorCode),TRUE);
			break;
		default:
			retval = FALSE;
			output(g_strdup_printf("Error code 0x%02x\n", errorCode),TRUE);
			break;
	}

	switch(statusCode) 
	{
		case S_ACTIVE:
			/* Good! */
			retval = TRUE;
			break;
		case S_RUNNING:
			retval = FALSE;
			output(g_strdup_printf("Program running, download failed, s=0x%02x\n",statusCode),TRUE);
			break;
		case S_HALTED:
			retval = FALSE;
			output(g_strdup_printf("Program halted, download failed, s=0x%02x\n",statusCode),TRUE);
			break;
		case S_TRACE1:
			retval = FALSE;
			output(g_strdup_printf("Program trace detected, download failed, s=0x%02x\n",statusCode),TRUE);
			break;
		case S_COLD_RESET:
			retval = FALSE;
			output(g_strdup_printf("Cold reset detected, s=0x%02x\n",statusCode),TRUE);
			break;
		case S_WARM_RESET:
			retval = FALSE;
			output(g_strdup_printf("Warm reset detected, s=0x%02x\n",statusCode),TRUE);
			break;
		default:
			retval = FALSE;
			output(g_strdup_printf("Unknown status received, s=0x%02x\n",statusCode),TRUE);
			break;

	}

	if (prompt != '>') 
	{
		output(g_strdup_printf("Prompt was expected to be \">\" but returned as \"0x%02x\" \n",prompt),TRUE);
		retval = FALSE;
	} 
	else
		retval = TRUE;
	return retval;
}

gboolean sendPPAGE(gint port_fd, guint a, gboolean erasing)
{
	guchar c;
	static guint page = 0;
	guchar command [4];
	gint res = 0;

	a = 0xFFFF & (a >> 16);

	if (a != page) {
		page = a & 0xFF;
		output(g_strdup_printf("Setting page to 0x%02x\n", page),TRUE);
		command[0] = C_WRITE_BYTE;
		command[1] = 0x00;
		command[2] = 0x30;
		command[3] = page;
		res = write_wrapper(port_fd, command, 4);
		if (res != 4)
			output(g_strdup_printf("sendPPAGE(): SHORT WRITE %i of 4",res),TRUE);
		if (debug >= 4) {
			output(g_strdup_printf("TX: %02x %02x %02x %02x\n", command[0], command[1], command[2], command[3] ),TRUE);
		}
		if (!check_status(port_fd))
			return FALSE;

		c = C_ERASE_PAGE;
		if (erasing) {
			output(g_strdup_printf("Erasing page 0x%02x\n", page),TRUE);
			res = write_wrapper(port_fd, &c, 1);
			if (res != 1)
				output(g_strdup_printf("sendPPAGE():(erasing) SHORT WRITE %i of 1",res),TRUE);
			output(g_strdup_printf("Page 0x%02x erased...\n",page),TRUE);
#ifdef __WIN32__
	g_usleep(750000);
#endif
			if(!check_status(port_fd))
				return FALSE;
		}
	}
	return TRUE;
}


gboolean readback_block(gint port_fd, gint a, guchar *b, gint n)
{
	guchar command[4];
	gint res = 0;
	gboolean was_a_retry = FALSE;
	gint errcount = 0;
	
	command[0] = C_READ_BLOCK;
	command[1] = (a & 0xFF00) >> 8;
	command[2] = (a & 0x00FF);
	command[3] = n-1;

retry:
	res = write_wrapper(port_fd, command, 4);
	if (res != 4)
		output(g_strdup_printf("readback_block(): SHORT WRITE %i of 4",res),TRUE);
	res = read_wrapper(port_fd, (gchar *)b, n);
	if (res != n)
	{
		output((gchar *)"readback_block error, retrying\n",FALSE);
		return FALSE;
	}
	if (!check_status(port_fd))
	{
		output((gchar *)"readback_block error, retrying\n",FALSE);
		was_a_retry = TRUE;
		flush_serial(port_fd,BOTH);
		errcount++;
		if (errcount > 5)
		{
			output((gchar *)"too many errors, aborting!\n",FALSE);
			return FALSE;
		}
		goto retry;
	}
	else if (was_a_retry)
	{
		output((gchar *)"Retry was successful!\n",FALSE);
		was_a_retry = FALSE;
	}
	return TRUE;
}


gboolean send_block(gint port_fd, guint a, guchar *b, guint n)
{
	guchar command[4] = {0,0,0,0};
	gint errcount = 0;
	gint page_err = 0;
	gboolean was_a_retry = FALSE;
	guchar i;
	guint res = 0;

retry:
	if ((a < 0x400)) {
		if (debug) {
			output(g_strdup_printf("Skipping block %04x\n", a),TRUE);
		}
		return TRUE;
	}

	if (!sendPPAGE(port_fd,a, TRUE))
	{
		page_err++;
		if (page_err > 3)
		{
			output((gchar *)"send_block(): Failure setting page more than 3x in a row, ABORTING!",FALSE);
			return FALSE;
		}
		else goto retry;
	}

	command[0] = C_WRITE_BLOCK;
	command[1] = (a & 0xFF00) >> 8;
	command[2] = a & 0x00FF;
	command[3] = n - 1;

	if (debug >= 4) {
		output(g_strdup_printf("TX: %02x %02x %02x %02x\n", command[0], command[1], command[2], command[3] ),TRUE);
	}

	res = write_wrapper(port_fd, command, 4);
	if (res != 4)
		output(g_strdup_printf("send_block(): SHORT WRITE %i of 4",res),TRUE);
	if (debug >= 4) {
		output((gchar *)"TX:",FALSE);
		for (i = 0 ; i < n ; i++) {
			output(g_strdup_printf(" %02x", b[i] ),TRUE);
		}
		output((gchar *)"\n",FALSE);
	}
	res = write_wrapper(port_fd, b, n);
	if (res != n)
		output(g_strdup_printf("send_block(): SHORT WRITE %i of %i",res,n),TRUE);
	if (!check_status(port_fd))
	{
		output((gchar *)"send_block error, retrying\n",FALSE);
		was_a_retry = TRUE;
		flush_serial(port_fd,BOTH);
		errcount++;
		if (errcount > 5)
		{
			output((gchar *)"too many errors, aborting!\n",FALSE);
			return FALSE;
		}
		goto retry;
	}
	else if (was_a_retry)
	{
		output((gchar *)"Retry was successful!\n",FALSE);
		was_a_retry = FALSE;
	}
	total_bytes += n;
	return TRUE;
}

gboolean erase_S12(gint port_fd)
{
	guchar c;
	gint res = 0;

	output((gchar *)"Attempting to erase main flash!\n",FALSE);
	c = C_ERASE_ALL;
	res = write_wrapper(port_fd, &c, 1);
	if (res != 1)
		output(g_strdup_printf("erase_S12(): SHORT WRITE %i of 1",res),TRUE);
#ifdef __WIN32__
	g_usleep(2500000);
#endif
	if(!check_status(port_fd))
		return FALSE;
	output((gchar *)"Main Flash Erased...\n",FALSE);
	return TRUE;
}


gboolean send_S12(gint port_fd, guint count)
{
	guint i = 0;
	guint j = 0;
	gboolean was_a_retry = FALSE;
	gint errorcount = 0;
	guchar checksum = 0;
	guchar *thisRec = NULL;
	guchar *verify = NULL;
	guchar recsum = 0;
	guchar difsum = 0;
	guint size = 0;
	guint dataSize = 0;
	guint addr = 0;
	guint addrSize = 0;
	guint nBlocks = 0;

	output(g_strdup_printf("Uploading ECU firmware...\n"),TRUE);
	for (i = 0; i < count; i++) {
		switch (fileBuf[i][1]) {
			case '0':
				continue;
			case '1':
				addrSize = 4;
				break;
			case '2':
				addrSize = 6;
				break;
			case '3':
				addrSize = 8;
				break;
			case '7':
				addrSize   = 8;
				break;
			case '8':
				addrSize   = 6;
				break;
			case '9':
				addrSize   = 4;
				break;
		}
		size = extract_number(fileBuf[i]+2, 2);
		dataSize = size - addrSize/2 -1;
		addr = extract_number(fileBuf[i]+4, addrSize);
		checksum = cs(addr) + size;

		thisRec = (guchar *)malloc(dataSize + 1);
		verify = (guchar *)malloc(dataSize + 1);
		/* Calculate the checksum of the block */
		checksum = extract_data(fileBuf[i]+4+addrSize, dataSize, checksum, thisRec);
		/* Read the checksum from the .s19 */
		recsum = extract_number(fileBuf[i]+4+addrSize+dataSize*2, 2);
		/* Check that they match! */
		difsum = ~(recsum + checksum);

		if (difsum != 0)
			output(g_strdup_printf("Invalid checksum, found 0x%02x, expected %#04x\n", recsum,(guchar)~checksum),TRUE);

		if (debug >= 3)
			if ((i%10) == 0)
				output(g_strdup_printf("Sending record %d:%6x\n", i, addr),TRUE);
		if (addr >= 0x8000 && addr <= 0xBFFF) addr -= 0x4000;

		nBlocks = (dataSize - 1) / MAX_BLOCK + 1;
		for (j = 0; dataSize > 0; j++) {
			guint nn = dataSize > MAX_BLOCK ? MAX_BLOCK : dataSize;
			guchar *thisRecPtr = thisRec;
			if (addr < 0x8000 && addr+nn > 0x8000) {
				nn = 0x8000 - addr;
			}
send_retry:
		/*	printf("Should send %i bytes to address 0x%.4x\n",nn,addr);*/
			if (!send_block(port_fd,addr, thisRecPtr, nn))
			{
				output((gchar *)"FAILURE to even send block, write aborted with critical failure!\n",FALSE);
				free(thisRec);
				free(verify);
				return FALSE;
			}
			if ((verify_writes) && (!(addr >= 0xFF80) && (addr <= 0xFFFF))) 
			{
				readback_block(port_fd,addr, verify, nn);
				if (memcmp(verify,thisRecPtr,nn) != 0)
				{
					errorcount++;
					output(g_strdup_printf("VERIFY ERROR at S19 line %i, address 0x%X, %i byte block!\n",i,addr,nn),TRUE);
					verify_failure_count++;
					if (errorcount > 5)
					{
						output(g_strdup_printf("TOO MANY VERIFY ERRORS at S19 line %i, address 0x%X, aborting!\n",i,addr),TRUE);
						free(thisRec);
						free(verify);
						return FALSE;
					}
					else
					{
						was_a_retry = TRUE;
						output(g_strdup_printf("Retrying write+verify, attempt #%i\n",errorcount),TRUE);
						goto send_retry;
					}
				}
				else
				{
					if (was_a_retry)
					{
						output(g_strdup_printf("Retry was successful after %i attempts\n",errorcount),TRUE);
						verify_retry_success_count++;
					}
				}
				was_a_retry = FALSE;
				errorcount = 0;
			}
			dataSize -= nn;
			thisRecPtr += nn;
			addr += nn;
			if (addr >= 0x8000 && addr <= 0xBFFF) addr += 0x4000;
		}
		free(thisRec);
		free(verify);
		progress_update ((gfloat)i/(gfloat)(count-2));
	}
	return TRUE;
}

void ms2_enter_boot_mode(gint port_fd)
{
	gint res = 0;
	res = write_wrapper(port_fd, (guchar *)"!!!SafetyFirst", 14);
	if (res != 14)
		output(g_strdup_printf("ms2_enter_boot_mode() SHORT WRITE, sent %i of 14\n",res),TRUE);
	check_status(port_fd);
	flush_serial(port_fd, BOTH);
}

void reset_proc(gint port_fd)
{
	gint res = 0;
	gchar buf[10];
	guchar command = C_RESET;

	res = write_wrapper(port_fd, &command, 1);
	if (res != 1)
		output(g_strdup_printf("reset_proc() SHORT WRITE, sent %i of 1\n",res),TRUE);
	/* Read out response and toss it */
	res = read_wrapper(port_fd,buf,1);
	if (res == 1)
		output((gchar *)"Processor Reset complete\n",FALSE);
}


void reset_libreems_proc(gint port_fd)
{
	gint res = 0;
	gchar buf[10];
	guchar command[4] = {C_WRITE_PC,0xCC,0x20,C_GO};

	if (debug >= 4) {
		output(g_strdup_printf("TX: %02x %02x %02x %02x\n", command[0], command[1], command[2], command[3] ),TRUE);
	}

	res = write_wrapper(port_fd, command, 4);
	if (res != 4)
		output(g_strdup_printf("reset_libreems_proc(): SHORT WRITE %i of 4",res),TRUE);
	/* Read out response and toss it */
	res = read_wrapper(port_fd,buf,1);
	if (res == 1)
		output(g_strdup_printf("Processor Reset complete, result 0x%X\n",(int)buf[0]),TRUE);
}



