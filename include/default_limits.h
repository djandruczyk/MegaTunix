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
	{"HR_Clock\0\0",0.0,4294967269.0,MTX},
	{"MS_Clock\0\0",0.0,255.0,MTX},
	{"RPM\0\0",0.0,25500.0,MTX|MT_CLASSIC|MT_FULL},
	{"EngineBits\0\0",0.0,25500.0,MTX},
	{"Idle_DC\0\0",0.0,100.0,MTX},
	{"TPS_Volts\0\0",0.0,5.0,MTX},
	{"MAP_Volts\0\0",0.0,5.0,MTX},
	{"BARO_Volts\0",0.0,5.0,MTX},
	{"MAT_Volts\0",0.0,5.0,MTX},
	{"CLT_Volts\0",0.0,5.0,MTX},
	{"TPS_Counts\0",0.0,255.0,MTX},
	{"MAP_Counts\0",0.0,255.0,MTX},
	{"BARO_Counts\0",0.0,255.0,MTX},
	{"MAT_Counts\0",0.0,255.0,MTX},
	{"CLT_Counts\0",0.0,255.0,MTX},
	{"TPS_%\0",0.0,100.0,MTX},
	{"MAP_KPA\0",0.0,255.0,MTX},
	{"BARO_KPA\0",0.0,255.0,MTX},
	{"MAT_(Deg)\0",-40.0,215.0,MTX},
	{"CLT_(Deg)\0",-40.0,215.0,MTX},
	{"O2_Volts\0",0.0,5.0,MTX},
	{"O2_Counts\0",0.0,255.0,MTX},
	{"GammaE\0",0.0,255.0,MTX},
	{"BATT_Volts\0",8.0,18.0,MTX},
	{"BATT_Counts\0",0.0,255.0,MTX},
	{"AIRcorr\0",0.0,255.0,MTX},
	{"BAROcorr\0",0.0,255.0,MTX},
	{"EGOcorr\0",0.0,255.0,MTX},
	{"WARMcorr\0",0.0,255.0,MTX},
	{"TPSaccel\0",0.0,255.0,MTX},
	{"VE1\0",0.0,255.0,MTX},
	{"VE2\0",0.0,255.0,MTX},
	{"PW1\0",0.0,25.5,MTX},
	{"PW2\0",0.0,25.5,MTX|MT_FULL},
	{"DutyCycle1\0",0.0,100.0,MTX|MT_FULL},
	{"DutyCycle2\0",0.0,100.,MTX|MT_FULL},
	{"CycleTimeH\0",0.0,255.0,MTX},
	{"CycleTimeL\0",0.0,255.0,MTX},
	{"SparkAngle\0",0.0,90.0,MTX},
	{"BSPOT1\0",0.0,255.0,MTX},
	{"BSPOT2\0",0.0,255.0,MTX},
	{"BSPOT3\0",0.0,255.0,MTX},
	{"Seconds\0",0.0,255.0,MT_CLASSIC},
	{"MAP\0",0.0,255.0,MT_CLASSIC|MT_FULL},
	{"O2\0",0.0,5.0,MT_CLASSIC|MT_FULL},
	{"EngineBit\0",0.0,255.0,MT_CLASSIC},
	{"Gego\0",0.0,255.0,MT_CLASSIC|MT_FULL},
	{"Gair\0",0.0,255.0,MT_CLASSIC|MT_FULL},
	{"Gwarm\0",0.0,255.0,MT_CLASSIC|MT_FULL},
	{"Gbaro\0",0.0,255.0,MT_CLASSIC|MT_FULL},
	{"Gve\0",0.0,255.0,MT_CLASSIC|MT_FULL},
	{"Gammae\0",0.0,255.0,MT_CLASSIC|MT_FULL},
	{"Time\0",0.0,4294967269.0,MT_FULL},
	{"SecL\0",0.0,255.0,MT_FULL},
	{"TP\0",0.0,255.0,MT_FULL},
	{"MAT\0",0.0,255.0,MT_FULL},
	{"CLT\0",0.0,255.0,MT_FULL},
	{"Engine\0",0.0,255.0,MT_FULL},
	{"TPSacc\0",0.0,255.0,MT_FULL},
	{"PW\0",0.0,255.0,MT_FULL},
	{"Gve2\0",0.0,255.0,MT_FULL},
	{"UserData1\0",0.0,255.0,MT_FULL},
	{"UserData2\0",0.0,255.0,MT_FULL},
	{"UserData3\0",0.0,255.0,MT_FULL},
	{"Rsecl\0",0.0,255.0,MT_RAW},
	{"Rsquirt\0",0.0,255.0,MT_RAW},
	{"Rengine\0",0.0,255.0,MT_RAW},
	{"Rbaro\0",0.0,255.0,MT_RAW},
	{"Rmap\0",0.0,255.0,MT_RAW},
	{"Rmat\0",0.0,255.0,MT_RAW},
	{"Rclt\0",0.0,255.0,MT_RAW},
	{"Rtps\0",0.0,255.0,MT_RAW},
	{"Rbatt\0",0.0,255.0,MT_RAW},
	{"Rego\0",0.0,255.0,MT_RAW},
	{"Regocorr\0",0.0,255.0,MT_RAW},
	{"Raircorr\0",0.0,255.0,MT_RAW},
	{"Rwarmcorr\0",0.0,255.0,MT_RAW},
	{"Rrpm\0",0.0,255.0,MT_RAW},
	{"Rpw\0",0.0,255.0,MT_RAW},
	{"Rtpsaccel\0",0.0,255.0,MT_RAW},
	{"Rbarocorr\0",0.0,255.0,MT_RAW},
	{"Rgammae\0",0.0,255.0,MT_RAW},
	{"Rvecurr\0",0.0,255.0,MT_RAW},
	{"Rpw2\0",0.0,255.0,MT_RAW},
	{"Rvecurr2\0",0.0,255.0,MT_RAW},
	{"RpgOfs\0",0.0,255.0,MT_RAW},
	{"RpgOfs\0",0.0,255.0,MT_RAW},

};

#endif
