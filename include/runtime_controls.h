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
static struct Def_Control all_controls[] = {
/* Default visible Controls */
{"Clock",0,0,"Seconds",1,54,UCHAR,TRUE,0},
{"O2_Volts",0,1,"O<sub>2</sub> (Volts)",20,12,FLOAT,TRUE,O2_DEP}, /*scale varies */
{"Batt_Volts",0,2,"Batt (Volts)",23,4,FLOAT,TRUE,0},
{"TPS_percent",0,3,"TPS (%)",15,44,FLOAT,TRUE,0},
{"RPM",0,4,"RPM",2,52,SHORT,TRUE,0},
{"Inj_1_PW",0,5,"Ch. 1 PW (ms)",32,36,FLOAT,TRUE,0},
{"Inj_1_DC",0,6,"Ch. 1 DC (%)",34,28,FLOAT,TRUE,0},
{"GammaE",1,0,"GammaE (%)",22,61,UCHAR,TRUE,0},
{"MAP",1,1,"MAP (kPa)",16,60,UCHAR,TRUE,0},	/* Special,  scale varies based on ECU*/
{"Coolant",1,2,"Coolant (\302\260 F.)",19,48,SHORT,TRUE,TEMP_DEP}, /* Temp SPECIAL!!! */
{"MAT",1,3,"MAT (\302\260 F.)",18,50,SHORT,TRUE,TEMP_DEP},	/* Temp SPECIAL!!! */
{"Idle_DC",1,4,"Idle DC (%)",4,74,UCHAR,TRUE,DUALTABLE|IAC_PWM|IAC_STEPPER},
{"Inj_2_PW",1,5,"Ch. 2 PW (ms)",33,40,FLOAT,TRUE,DUALTABLE},
{"Inj_2_DC",1,6,"Ch. 2 DC (%)",35,32,FLOAT,TRUE,DUALTABLE},
{"EGOcorr",2,0,"EGO (%)",27,71,UCHAR,TRUE,0},
{"BAROcorr",2,1,"Baro (%)",26,70,UCHAR,TRUE,0},
{"WARMcorr",2,2,"Warmup (%)   ",28,73,UCHAR,TRUE,0},
{"AIRcorr",3,0,"Air Density (%)",25,69,UCHAR,TRUE,0},
{"VE_1",3,1,"VE 1 (%)",30,57,UCHAR,TRUE,0},
{"TPSaccel",3,2,"Accel (ms)",29,72,UCHAR,TRUE,0},

/* Other usable controls,  but not normally used */
{"VE_2",1,-1,"VE 2 (%)",31,58,UCHAR,FALSE,DUALTABLE},
{"SparkAngle",1,8,"SparkAngle",38,77,UCHAR,TRUE,S_N_SPARK|S_N_EDIS},
{"TPS_Volts",1,-1,"TPS (Volts)",5,24,FLOAT,FALSE,0},
{"MAP_Volts",1,-1,"MAP (Volts)",6,16,FLOAT,FALSE,0},
{"BARO_Volts",1,-1,"BARO (Volts)",7,0,FLOAT,FALSE,0},
{"MAT_Volts",1,-1,"MAT (Volts)",8,20,FLOAT,FALSE,0},
{"CLT_Volts",1,-1,"CLT (Volts)",9,8,FLOAT,FALSE,0},
{"TPS_Counts",1,-1,"TPS (Counts)",10,68,UCHAR,FALSE,0},
{"MAP_Counts",1,-1,"MAP (Counts)",11,66,UCHAR,FALSE,0},
{"BARO_Counts",1,-1,"BARO (Counts)",12,62,UCHAR,FALSE,0},
{"MAT_Counts",1,-1,"MAT (Counts)",13,67,UCHAR,FALSE,0},
{"CLT_Counts",1,-1,"CLT (Counts)",14,64,UCHAR,FALSE,0},
{"O2_Counts",1,-1,"O<sub>2</sub> (Counts)",21,65,UCHAR,FALSE,0},
{"BATT_Counts",1,-1,"BATT (Counts)",24,63,UCHAR,FALSE,0},
{"BARO",1,-1,"BARO (kPa)",17,59,UCHAR,FALSE,0},
{"CycleTimeH",1,-1,"CycleTimeH",36,75,UCHAR,FALSE,S_N_EDIS|S_N_SPARK},
{"CycleTimeL",1,-1,"CycleTimeL",37,76,UCHAR,FALSE,S_N_EDIS|S_N_SPARK},
{"BSPOT1",1,-1,"BSPOT1",39,78,UCHAR,FALSE,0},
{"BSPOT2",1,-1,"BSPOT2",40,79,UCHAR,FALSE,0},
{"BSPOT3",1,-1,"BSPOT3",41,80,UCHAR,FALSE,0}
};


#endif
