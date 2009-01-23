/* This program is a VERY quick and dirty hack which will be cleaned up later
 * I wrote it very quickly in order to be able to reflash the MS2 from linux, so
 * things like comport speed, etc... are hardcoded
 * and I'm using blocking I/O when non-blocking would be better, and
 * I can get into situations that require the boot jumper on occasion
 * This program is based on efahl's ms2dl C++ program, but ported to Linux.
 *
 * $Id: ms2_loader.c,v 1.1.2.1 2009/01/23 23:10:15 extace Exp $
 */

#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0
#endif

#include <gtk/gtk.h>
#include <loader_common.h>
#include <ms2_loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#ifdef CREPRINT
#define REPRINT CREPRINT
#else
#define REPRINT CRPRNT
#endif

cc_t    ttydefchars[NCCS] = {
        CEOF,   CEOL,   CEOL,   CERASE, CWERASE, CKILL, REPRINT,
        _POSIX_VDISABLE, CINTR, CQUIT,  CSUSP,  CDSUSP, CSTART, CSTOP,  CLNEXT,
        CFLUSH, 1, 0,  0, _POSIX_VDISABLE
};


#define BAUDRATE B115200

#define C_WAKE_UP        0x0D
#define C_READ_BYTE      0xA1
#define C_WRITE_BYTE     0xA2
#define C_READ_WORD      0xA3
#define C_WRITE_WORD     0xA4
#define C_READ_NEXT      0xA5
#define C_WRITE_NEXT     0xA6
#define C_READ_BLOCK     0xA7
#define C_WRITE_BLOCK    0xA8
#define MAX_BLOCK        256 // For both writes and reads.
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

//--  Error codes  -------------------------------------------------------------

#define E_NONE           0xE0
#define E_COMMAND        0xE1
#define E_NOT_RUN        0xE2
#define E_SP_RANGE       0xE3
#define E_SP_INVALID     0xE4
#define E_READ_ONLY      0xE5
#define E_FACCERR        0xE6
#define E_ACCERR         0xE9

//--  Status codes  ------------------------------------------------------------

#define S_ACTIVE         0x00
#define S_RUNNING        0x01
#define S_HALTED         0x02
#define S_TRACE1         0x04
#define S_COLD_RESET     0x08
#define S_WARM_RESET     0x0C

#define LONG_DELAY 250000  /* 250 ms */

struct termios tio;
unsigned int total_bytes = 0;

char **fileBuf;
unsigned int count = 0;
int debug = 3;
/* debug levels
0 = Quiet
1 = Some progress
2 = Full progress
3 = + serial comms
4 = + the s19 file as parsed
5 = + comments
*/

void ms2_setup_port(int port_fd)
{
	struct termios tio;	
	fcntl(port_fd, F_SETFL, 0);

	tcgetattr(port_fd, &tio);

	memcpy(tio.c_cc, ttydefchars, NCCS);

	cfsetispeed(&tio, B115200);
	cfsetospeed(&tio, B115200);

	tio.c_cflag &= ~PARENB;
	tio.c_cflag &= ~CSTOPB;
	tio.c_cflag &= ~CSIZE;

	tio.c_cflag |=  CS8 | CLOCAL | CREAD;
	tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tio.c_oflag &= ~OPOST;

	tcsetattr(port_fd, TCSANOW, &tio);

}


void do_ms2_load(int port_fd, int file_fd)
{
	printf("MS2 loader!\n");
	count = read_s19(file_fd);
	if (count == 0)
		return;
	enter_boot_mode(port_fd);
	if (!wakeup_S12(port_fd))
		return;
	erase_S12(port_fd);
	send_S12(port_fd,count);
	free_s19(count);
	reset_proc(port_fd);
	output(g_strdup_printf("Wrote %d bytes\n", total_bytes),TRUE);
	return;
}

void ms2_chomp(char *inBuf)
{
    char *s;

    s = strrchr(inBuf, '\n');
    if (s)
	*s = 0;
    s = strrchr(inBuf, '\r');
    if (s)
	*s = 0;
}

gint read_s19 (gint file_fd)
{
    unsigned int lines = 0;
    char *buf = NULL;
    gsize len = 0;
    GIOChannel *chan = NULL;
    GError *err = NULL;


    chan = g_io_channel_unix_new(file_fd);
    g_io_channel_set_encoding(chan, NULL,&err);
    count = 0;

    while(G_IO_STATUS_NORMAL == g_io_channel_read_line(chan,&buf,&len,NULL,&err)) {
	if (len > 0)	
		g_free(buf);
	lines++;
    }
    fileBuf = (char **)malloc(lines * sizeof(char *));

    // Go to beginning of file
    g_io_channel_seek_position(chan,0,G_SEEK_SET,&err);
    while(G_IO_STATUS_NORMAL == g_io_channel_read_line(chan,&buf,&len,NULL,&err)) {
    if (debug > 4) {
        output(g_strdup_printf("%s", buf),TRUE);
    }
	ms2_chomp(buf);
	fileBuf[count] = strdup(buf);
	g_free(buf);
	count++;
    }

    return count;
}

void free_s19(count)
{
    unsigned int i;
    
    for (i = 0; i < count; i++) {
	free(fileBuf[i]);
    }

    free(fileBuf);
}

unsigned int extract_number(char *data, unsigned int nBytes)
{
    char number[128];

    strncpy(number, data, nBytes);
    number[nBytes] = 0;

    return strtol(number, NULL, 16);
}

unsigned char cs(unsigned int l)
{
    return ((l >> 24) & 0xff) + ((l >> 16) & 0xff) + ((l >> 8) & 0xff) + ((l >> 0) & 0xff);
}

unsigned char extract_data(char *data, unsigned int nBytes, unsigned char checksum, unsigned char *binary)
{
    unsigned int i;

    for (i = 0; i < nBytes; i++) {
	unsigned int n = extract_number(data +i*2, 2);
	binary[i] = (char)n;
	checksum += n;
    }

    return checksum;
}

gboolean wakeup_S12(gint port_fd)
{
	unsigned char errorCode, statusCode, prompt;
	unsigned char c;
	unsigned char c2;
	int i;

	c = C_WAKE_UP;

	fcntl(port_fd, F_SETFL, O_NDELAY);

	for (i = 0; i < 10; i++) {
		prompt = 0;
		output("Attempting Wakeup...\n",FALSE);
		write(port_fd, &c, 1);
		if (debug >= 4) {
			output(g_strdup_printf("TX: %02x\n", c),TRUE);
		}
		usleep(25000);

		read(port_fd, &c2, 1);
		errorCode = c2;
		if (debug >= 4) {
			output(g_strdup_printf("RX: %02x", c2),TRUE);
		}
		read(port_fd, &c2, 1);
		statusCode = c2;
		if (debug >= 4) {
			output(g_strdup_printf( " %02x", c2),TRUE);
		}
		read(port_fd, &c2, 1);
		prompt = c2;
		if (debug >= 4) {
			output(g_strdup_printf( " %02x\n", c2),TRUE);
		}

		switch(errorCode) {
			case E_NONE:
				break;
			default:
				output(g_strdup_printf( "Error code 0x%02x\n", errorCode),TRUE);
		}

		switch(statusCode) {
			case S_ACTIVE:
				break;
			default:
				output(g_strdup_printf( "Status code 0x%02x\n", statusCode),TRUE);
		}

		if (prompt != '>') {
			output(g_strdup_printf( "Prompt was %c\n", prompt),TRUE);
		} else {
			output("Got Prompt, continuing...\n",FALSE);
			break;
		}

		usleep(LONG_DELAY);
	}

	fcntl(port_fd, F_SETFL, 0);

	if (i == 10) {
		output("Could not wake up processor, try jumpering\nthe boot jumper and power cycling the ECU...\n",FALSE);
		return FALSE;
	}
	return TRUE;
}
   
void check_status(gint port_fd)
{
    unsigned char errorCode, statusCode, prompt;
    unsigned char c2;

    read(port_fd, &c2, 1);
    errorCode = c2;
    if (debug >= 4) {
        output(g_strdup_printf("RX: %02x", c2),TRUE);
    }
    read(port_fd, &c2, 1);
    statusCode = c2;
    if (debug >= 4) {
        output(g_strdup_printf(" %02x", c2),TRUE);
    }
    read(port_fd, &c2, 1);
    prompt = c2;
    if (debug >= 4) {
        output(g_strdup_printf(" %02x\n", c2),TRUE);
    }

    switch(errorCode) {
	case E_NONE:
	    break;
	default:
	    output(g_strdup_printf("Error code 0x%x\n", errorCode),TRUE);
    }

    switch(statusCode) {
	case S_ACTIVE:
	    break;
	default:
	    output(g_strdup_printf("Status code 0x%x\n", statusCode),TRUE);
    }

    if (prompt != '>') {
	output(g_strdup_printf("Prompt was %c\n", prompt),TRUE);
    } 
}

void sendPPAGE(gint port_fd, unsigned int a, unsigned char erasing)
{
    unsigned char c;
    static unsigned char page = 0;
    unsigned char command [4];

    a = 0xFFFF & (a >> 16);

    if (a != page) {
	page = a & 0xFF;
	output(g_strdup_printf("Setting page to 0x%02x:\n", page),TRUE);
	command[0] = C_WRITE_BYTE;
	command[1] = 0x00;
	command[2] = 0x30;
	command[3] = page;
	write(port_fd, command, 4);
    if (debug >= 4) {
        output(g_strdup_printf("TX: %02x %02x %02x %02x\n", command[0], command[1], command[2], command[3] ),TRUE);
    }
	usleep(LONG_DELAY);
	check_status(port_fd);

	c = C_ERASE_PAGE;
	if (erasing) {
	    output(g_strdup_printf("erasing page 0x%02x:\n", page),TRUE);
	    write(port_fd, &c, 1);
        if (debug >= 4) {
            output(g_strdup_printf("TX: %02x\n", c),TRUE);
        }
	    usleep(LONG_DELAY);
	    check_status(port_fd);
	    output("Erased.\n",FALSE);
	}

    }
}

void send_block(gint port_fd, unsigned int a, unsigned char *b, unsigned int n)
{
    unsigned char command[4];
    unsigned char i;

    if ((a < 0x400)) {
        if (debug) {
            output(g_strdup_printf("Skipping block %04x\n", a),TRUE);
        }
        return;
    }

    command[0] = C_WRITE_BLOCK;
    command[1] = 0;
    command[2] = 0;
    command[3] = 0;

    sendPPAGE(port_fd,a, 1);

    command[1] = 0xFF & (a >> 8);
    command[2] = 0xFF & a;
    command[3] = n - 1;

    if (debug >= 4) {
        output(g_strdup_printf("TX: %02x %02x %02x %02x\n", command[0], command[1], command[2], command[3] ),TRUE);
    }

    write(port_fd, command, 4);
    if (debug >= 4) {
        output( "TX:",FALSE);
        for (i = 0 ; i < n ; i++) {
            output(g_strdup_printf(" %02x", b[i] ),TRUE);
        }
        output( "\n",FALSE);
    }
    write(port_fd, b, n);
    total_bytes += n;
    check_status(port_fd);
}

void erase_S12(gint port_fd)
{
    unsigned char c;

    if (debug) {
        output( "Erasing main flash!\n",FALSE);
    }
    c = C_ERASE_ALL;
    usleep(LONG_DELAY);
    write(port_fd, &c, 1);
    flush_serial(port_fd,OUTBOUND);
    if (debug >= 4) {
        output(g_strdup_printf("TX: %02x\n", c ),TRUE);
    }
    usleep(LONG_DELAY);
    check_status(port_fd);
    if (debug) {
        output( "Erased.\n",FALSE);
    }
}
void send_S12(gint port_fd, gint count)
{
    unsigned int i, j;
    unsigned char checksum, *thisRec, recsum, difsum;
    unsigned int size, dataSize, addr, addrSize;
    unsigned int nBlocks;

    for (i = 0; i < count; i++) {
	switch (fileBuf[i][1]) {
	    case '0':
		addrSize = 4;
		break;
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

	thisRec = (unsigned char *)malloc(dataSize + 1);
	checksum = extract_data(fileBuf[i]+4+addrSize, dataSize, checksum, thisRec);

	recsum = extract_number(fileBuf[i]+4+addrSize+dataSize*2, 2);
	difsum = ~(recsum + checksum);

	if (difsum != 0) {
	    output(g_strdup_printf("Invalid checksum, found 0x%02x, expected %#04x\n", recsum,(unsigned char)~checksum),TRUE);
		    
	}

    if (debug >= 3) {
    	output(g_strdup_printf("Sending record %d:%6x\n", i, addr),TRUE);
    }
	if (addr >= 0x8000 && addr <= 0xBFFF) addr -= 0x4000;

	nBlocks = (dataSize - 1) / MAX_BLOCK + 1;
	for (j = 0; dataSize > 0; j++) {
	    unsigned int nn = dataSize > MAX_BLOCK ? MAX_BLOCK : dataSize;
	    unsigned char *thisRecPtr = thisRec;
	    if (addr < 0x8000 && addr+nn > 0x8000) {
		nn = 0x8000 - addr;
	    }

	    send_block(port_fd,addr, thisRecPtr, nn);
	    dataSize -= nn;
	    thisRecPtr += nn;
	    addr += nn;
	    if (addr >= 0x8000 && addr <= 0xBFFF) addr += 0x4000;
	}

	free(thisRec);
    }
}

void enter_boot_mode(gint port_fd)
{
    printf("Sending jumperless flash command\n");
    write(port_fd, "!", 1);
    if (debug>4) {
        output( "!",FALSE);
    }
    usleep(LONG_DELAY);
    flush_serial(port_fd, BOTH);
//    sleep(6);

    write(port_fd, "!", 1);
    if (debug>4) {
        output( "!",FALSE);
    }
    usleep(LONG_DELAY);
    flush_serial(port_fd, BOTH);
//    sleep(3);

    write(port_fd, "!", 1);
    if (debug>4) {
        output( "!",FALSE);
    }
    usleep(LONG_DELAY);
    flush_serial(port_fd, BOTH);

    write(port_fd, "SafetyFirst", 11);
    if (debug>4) {
        output( "SafetyFirst\n",FALSE);
    }
    usleep(LONG_DELAY);
    flush_serial(port_fd, BOTH);

}

void reset_proc(gint port_fd)
{
    unsigned char command = C_RESET;

    write(port_fd, &command, 1);
}


