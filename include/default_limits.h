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

#ifndef __DEFAULT_LIMITS_H__
#define __DEFAULT_LIMITS_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <structures.h>

/* Structure defined in structures.h */
static struct Default_Limits def_limits[] = {
	/* Default visible Controls */
	{"HR_Clock\0\0",0.0,4294967269.0},
	{"MS_Clock\0\0",0.0,255.0},
	{"RPM\0\0",0.0,25500.0},
	{"EngineBits\0\0",0.0,25500.0},
	{"Idle_DC\0\0",0.0,100.0},
	{"TPS_Volts\0\0",0.0,5.0},
	{"MAP_Volts\0\0",0.0,5.0},
	{"BARO_Volts\0",0.0,5.0},
	{"MAT_Volts\0",0.0,5.0},
	{"CLT_Volts\0",0.0,5.0},
	{"TPS_Counts\0",0.0,255.0},
	{"MAP_Counts\0",0.0,255.0},
	{"BARO_Counts\0",0.0,255.0},
	{"MAT_Counts\0",0.0,255.0},
	{"CLT_Counts\0",0.0,255.0},
	{"TPS_%\0",0.0,100.0},
	{"MAP_KPA\0",0.0,255.0},
	{"BARO_KPA\0",0.0,255.0},
	{"MAT_(Deg)\0",-40.0,215.0},
	{"CLT_(Deg)\0",-40.0,215.0},
	{"O2_Volts\0",0.0,5.0},
	{"O2_Counts\0",0.0,255.0},
	{"GammaE\0",0.0,255.0},
	{"BATT_Volts\0",8.0,18.0},
	{"BATT_Counts\0",0.0,255.0},
	{"AIRcorr\0",0.0,255.0},
	{"BAROcorr\0",0.0,255.0},
	{"EGOcorr\0",0.0,255.0},
	{"WARMcorr\0",0.0,255.0},
	{"TPSaccel\0",0.0,255.0},
	{"VE1\0",0.0,255.0},
	{"VE2\0",0.0,255.0},
	{"PW1\0",0.0,25.5},
	{"PW2\0",0.0,25.5},
	{"DutyCycle1\0",0.0,100.0},
	{"DutyCycle2\0",0.0,100.},
	{"CycleTimeH\0",0.0,255.0},
	{"CycleTimeL\0",0.0,255.0},
	{"SparkAngle\0",0.0,90.0},
	{"BSPOT1\0",0.0,255.0},
	{"BSPOT2\0",0.0,255.0},
	{"BSPOT3\0",0.0,255.0},
	{"Seconds\0",0.0,255.0},
	{"MAP\0",0.0,255.0},
	{"O2\0",0.0,5.0},
	{"EngineBit\0",0.0,255.0},
	{"Gego\0",0.0,255.0},
	{"Gair\0",0.0,255.0},
	{"Gwarm\0",0.0,255.0},
	{"Gbaro\0",0.0,255.0},
	{"Gve\0",0.0,255.0},
	{"Gammae\0",0.0,255.0},
	{"Time\0",0.0,4294967269.0},
	{"SecL\0",0.0,255.0},
	{"TP\0",0.0,255.0},
	{"MAT\0",0.0,255.0},
	{"CLT\0",0.0,255.0},
	{"Engine\0",0.0,255.0},
	{"TPSacc\0",0.0,255.0},
	{"PW\0",0.0,255.0},
	{"Gve2\0",0.0,255.0},
	{"UserData1\0",0.0,255.0},
	{"UserData2\0",0.0,255.0},
	{"UserData3\0",0.0,255.0},
	{"Rsecl\0",0.0,255.0},
	{"Rsquirt\0",0.0,255.0},
	{"Rengine\0",0.0,255.0},
	{"Rbaro\0",0.0,255.0},
	{"Rmap\0",0.0,255.0},
	{"Rmat\0",0.0,255.0},
	{"Rclt\0",0.0,255.0},
	{"Rtps\0",0.0,255.0},
	{"Rbatt\0",0.0,255.0},
	{"Rego\0",0.0,255.0},
	{"Regocorr\0",0.0,255.0},
	{"Raircorr\0",0.0,255.0},
	{"Rwarmcorr\0",0.0,255.0},
	{"Rrpm\0",0.0,255.0},
	{"Rpw\0",0.0,255.0},
	{"Rtpsaccel\0",0.0,255.0},
	{"Rbarocorr\0",0.0,255.0},
	{"Rgammae\0",0.0,255.0},
	{"Rvecurr\0",0.0,255.0},
	{"Rpw2\0",0.0,255.0},
	{"Rvecurr2\0",0.0,255.0},
	{"RpgOfs\0",0.0,255.0},
	{"RpgOfs\0",0.0,255.0}

};

#endif
