/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/* DataLogging related stuff */

#ifndef __DATALOGGING_H__
#define __DATALOGGING_H__

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>

union Logbits  
{
	unsigned long value;	/* 32 bits, 32 possible things to log */
	struct
	{
		unsigned char hr_clock		:1;
		unsigned char ms_clock		:1;
		unsigned char rpm		:1;
		unsigned char tps		:1;
		unsigned char batt		:1;
		unsigned char map		:1;
		unsigned char baro		:1;
		unsigned char o2		:1;
		unsigned char mat		:1;
		unsigned char clt		:1;
		unsigned char ve		:1;
		unsigned char barocorr		:1;
		unsigned char egocorr		:1;
		unsigned char matcorr		:1;
		unsigned char cltcorr		:1;
		unsigned char pw		:1;
		unsigned char dcycle		:1;
		unsigned char engbits		:1;
		unsigned char gammae		:1;
		unsigned short filler		:12;
		
	} bit;
	
};

struct Logables
{
	GtkWidget *widgets[64];
	gboolean index[64];
};

#endif
