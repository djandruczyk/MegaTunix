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

#ifndef __RUNTIME_CONTROLS_H__
#define __RUNTIME_CONTROLS_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
void create_default_controls();
void load_controls();
void add_control(gchar *,gchar *);
void save_controls();
/* Prototypes */

/* Structure defined in structures.h */
static struct Rt_Control all_controls[] = {
/* Default visible Controls */
{"Clock",NULL,NULL,NULL,NULL,0,0,0,"Seconds",1,54,UCHAR,TRUE,0,0},
{"O2_Volts",NULL,NULL,NULL,NULL,0,1,0,"O<sub>2</sub> (Volts)",20,12,FLOAT,TRUE,0,O2_DEP}, /*scale varies */
{"Batt_Volts",NULL,NULL,NULL,NULL,0,2,0,"Batt (Volts)",23,4,FLOAT,TRUE,0,0},
{"TPS_percent",NULL,NULL,NULL,NULL,0,3,0,"TPS (%)",15,44,FLOAT,TRUE,0,0},
{"RPM",NULL,NULL,NULL,NULL,0,4,0,"RPM",2,52,SHORT,TRUE,0,0},
{"Inj_1_PW",NULL,NULL,NULL,NULL,0,5,0,"Ch. 1 PW (ms)",32,36,FLOAT,TRUE,0,0},
{"Inj_1_DC",NULL,NULL,NULL,NULL,0,6,0,"Ch. 1 DC (%)",34,28,FLOAT,TRUE,0,0},
{"GammaE",NULL,NULL,NULL,NULL,1,0,0,"GammaE (%)",22,61,UCHAR,TRUE,0,0},
{"MAP",NULL,NULL,NULL,NULL,1,1,0,"MAP (kPa)",16,60,UCHAR,TRUE,0,0},	/* Special,  scale varies based on ECU*/
{"Coolant",NULL,NULL,NULL,NULL,1,2,0,"Coolant (\302\260 F.)",19,48,SHORT,TRUE,0,TEMP_DEP}, /* Temp SPECIAL!!! */
{"MAT",NULL,NULL,NULL,NULL,1,3,0,"MAT (\302\260 F.)",18,50,SHORT,TRUE,0,TEMP_DEP},	/* Temp SPECIAL!!! */
{"Idle_DC",NULL,NULL,NULL,NULL,1,4,0,"Idle DC (%)",4,74,UCHAR,TRUE,0,DUALTABLE|IAC_PWM|IAC_STEPPER},
{"Inj_2_PW",NULL,NULL,NULL,NULL,1,5,0,"Ch. 2 PW (ms)",33,40,FLOAT,TRUE,0,DUALTABLE},
{"Inj_2_DC",NULL,NULL,NULL,NULL,1,6,0,"Ch. 2 DC (%)",35,32,FLOAT,TRUE,0,DUALTABLE},
{"EGOcorr",NULL,NULL,NULL,NULL,2,0,0,"EGO (%)",27,71,UCHAR,TRUE,0,0},
{"BAROcorr",NULL,NULL,NULL,NULL,2,1,0,"Baro (%)",26,70,UCHAR,TRUE,0,0},
{"WARMcorr",NULL,NULL,NULL,NULL,2,2,0,"Warmup (%)",28,73,UCHAR,TRUE,0,0},
{"AIRcorr",NULL,NULL,NULL,NULL,3,0,0,"Air Density (%)",25,69,UCHAR,TRUE,0,0},
{"VE_1",NULL,NULL,NULL,NULL,3,1,0,"VE 1 (%)",30,57,UCHAR,TRUE,0,0},
{"TPSaccel",NULL,NULL,NULL,NULL,3,2,0,"Accel (ms)",29,72,UCHAR,TRUE,0,0},

/* Other usable controls,  but not normally used */
{"VE_2",NULL,NULL,NULL,NULL,-1,-1,-1,"VE 2 (%)",31,58,UCHAR,FALSE,0,DUALTABLE},
{"SparkAngle",NULL,NULL,NULL,NULL,-1,-1,-1,"SparkAngle",38,77,UCHAR,FALSE,0,S_N_SPARK|S_N_EDIS},
{"TPS_Volts",NULL,NULL,NULL,NULL,-1,-1,-1,"TPS (Volts)",5,24,FLOAT,FALSE,0,0},
{"MAP_Volts",NULL,NULL,NULL,NULL,-1,-1,-1,"MAP (Volts)",6,16,FLOAT,FALSE,0,0},
{"BARO_Volts",NULL,NULL,NULL,NULL,-1,-1,-1,"BARO (Volts)",7,0,FLOAT,FALSE,0,0},
{"MAT_Volts",NULL,NULL,NULL,NULL,-1,-1,-1,"MAT (Volts)",8,20,FLOAT,FALSE,0,0},
{"CLT_Volts",NULL,NULL,NULL,NULL,-1,-1,-1,"CLT (Volts)",9,8,FLOAT,FALSE,0,0},
{"TPS_Counts",NULL,NULL,NULL,NULL,-1,-1,-1,"TPS (Counts)",10,68,UCHAR,FALSE,0,0},
{"MAP_Counts",NULL,NULL,NULL,NULL,-1,-1,-1,"MAP (Counts)",11,66,UCHAR,FALSE,0,0},
{"BARO_Counts",NULL,NULL,NULL,NULL,-1,-1,-1,"BARO (Counts)",12,62,UCHAR,FALSE,0,0},
{"MAT_Counts",NULL,NULL,NULL,NULL,-1,-1,-1,"MAT (Counts)",13,67,UCHAR,FALSE,0,0},
{"CLT_Counts",NULL,NULL,NULL,NULL,-1,-1,-1,"CLT (Counts)",14,64,UCHAR,FALSE,0,0},
{"O2_Counts",NULL,NULL,NULL,NULL,-1,-1,-1,"O<sub>2</sub> (Counts)",21,65,UCHAR,FALSE,0,0},
{"BATT_Counts",NULL,NULL,NULL,NULL,-1,-1,-1,"BATT (Counts)",24,63,UCHAR,FALSE,0,0},
{"BARO",NULL,NULL,NULL,NULL,-1,-1,-1,"BARO (kPa)",17,59,UCHAR,FALSE,0,0},
{"CycleTimeH",NULL,NULL,NULL,NULL,-1,-1,-1,"CycleTimeH",36,75,UCHAR,FALSE,0,S_N_EDIS|S_N_SPARK},
{"CycleTimeL",NULL,NULL,NULL,NULL,-1,-1,-1,"CycleTimeL",37,76,UCHAR,FALSE,0,S_N_EDIS|S_N_SPARK},
{"BSPOT1",NULL,NULL,NULL,NULL,-1,-1,-1,"BSPOT1",39,78,UCHAR,FALSE,0,0},
{"BSPOT2",NULL,NULL,NULL,NULL,-1,-1,-1,"BSPOT2",40,79,UCHAR,FALSE,0,0},
{"BSPOT3",NULL,NULL,NULL,NULL,-1,-1,-1,"BSPOT3",41,80,UCHAR,FALSE,0,0}
};


#endif
