/*
  Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 
  Linux Megasquirt tuning software
  
  
  This software comes under the GPL (GNU Public License)
  You may freely copy,distribute, etc. this as long as all the source code
  is made available for FREE.
  
  No warranty is made or implied. You use this program at your own risk.
 */

/*! Global Variables */

#ifndef __MS_STRUCTURES_H__
#define __MS_STRUCTURES_H__

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <unions.h>

/*! 
 \brief The Raw_Runtime_Std structure is mapped to the buffer returned from the
 ECU on a realtime read for access to some of the members of the structure.
 \warning This structure will be deprecated soon, as realtiem data is handled
 in a differrent manner since 0.6.0
 */
struct Raw_Runtime_Std
{
	guchar	secl;		/*! Offset 0, ms_clock */
	union	squirt squirt;	/*! Offset 1, squirt status */
	union	engine engine;	/*! Offset 2, engine status */
	guchar	baro;		/*! Offset 3, baro reading */
	guchar	map;		/*! Offset 4, map reading */
	guchar	mat;		/*! Offset 5, mat reading */
	guchar	clt;		/*! Offset 6, clt reading */
	guchar	tps;		/*! Offset 7, tps reading */
	guchar	batt;		/*! Offset 8, bat reading */
	guchar	ego;		/*! Offset 9, o2 reading */
	guchar	egocorr;	/*! Offset 10 o2 correction factor */
	guchar	aircorr;	/*! Offset 11, mat correction factor */
	guchar	warmcorr;	/*! Offset 12, clt correction factor */
	guchar	rpm;		/*! Offset 13, rpm */
	guchar	pw1;		/*! Offset 14, pulsewidth */
	guchar	tpsaccel;	/*! Offset 15, accel enrich amount */
	guchar	barocorr;	/*! Offset 16, baro correction */
	guchar	gammae;		/*! Offset 17, sum of all enrichments */
	guchar	vecurr1;	/*! Offset 18, ve currently */
	guchar	bspot1;		/*! Offset 19, blank spot 1 */
	guchar	bspot2;		/*! Offset 20, blank spot 2 */
	guchar	bspot3;		/*! Offset 21, blank spot 3 */
	/*! NOTE: the last 3 are used diffeently in the other MS variants
	 * like the dualtable, squirtnspark and such...
	 */
};

#endif
