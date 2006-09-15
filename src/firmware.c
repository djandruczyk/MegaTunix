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
#include <firmware.h>
#include <structures.h>


#define CTR 187
#define UNITS 188
#define PG 10

/*!
 \brief load_firmware_file loads the .s19 file into memory
 */
void load_firmware_file(struct Io_File *iofile)
{
	printf("load_firmware_file() not implemented yet\n");
}

void trigger_shit()
{
	extern gint ** ms_data;
	gint i = 0;
	gint position = ms_data[PG][CTR];
	printf("Counter position on page %i is %i\n",PG,position);
	printf("data block from position %i to 185, then wrapping to 0 to %i\n",position,position-1);
	for (i=position;i<185;i++)
	{
		printf("%0x ",ms_data[PG][i]);
		if ((i%16)==0)
			printf("\n");
	}
	printf("\n");

	if (ms_data[PG][UNITS] == 1)
		printf("0.1 ms units\n");
	else
		printf("1uS units\n");

}
