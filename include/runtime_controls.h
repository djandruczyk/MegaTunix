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

#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
void create_default_controls();
/* Prototypes */

static struct Controls all_controls[] = {
/* Default visible Controls */
{"Clock",0,0,0,"Seconds",1,TRUE,0},
{"O2_Volts",0,1,0,"O<sub>2</sub> (Volts)",20,TRUE,0}, /*scale varies */
{"Batt_Volts",0,2,0,"Batt (Volts)",23,TRUE,0},
{"TPS_percent",0,3,0,"TPS (%)",15,TRUE,0},
{"RPM",0,4,0,"RPM",2,TRUE,0},
{"Inj_1_PW",0,5,0,"Ch. 1 PW (ms)",32,TRUE,0},
{"Inj_1_DC",0,6,0,"Ch. 1 DC (%)",34,TRUE,0},
{"GammaE",1,0,0,"GammaE (%)",22,TRUE,0},
{"MAP",1,1,0,"MAP (kPa)",16,TRUE,0},	/* Special,  scale varies based on ECU*/
{"Coolant",1,2,0,"Coolant (\302\260 F.)",19,TRUE,0}, /* Temp SPECIAL!!! */
{"MAT",1,3,0,"MAT (\302\260 F.)",18,TRUE,0},	/* Temp SPECIAL!!! */
{"Idle_DC",1,4,0,"Idle DC (%)",4,TRUE,0},
{"Inj_2_PW",1,5,0,"Ch. 2 PW (ms)",33,TRUE,0},
{"Inj_2_DC",1,6,0,"Ch. 2 DC (%)",35,TRUE,0},
{"EGOcorr",2,0,0,"EGO (%)",27,TRUE,0},
{"BAROcorr",2,1,0,"Baro (%)",26,TRUE,0},
{"WARMcorr",2,2,0,"Warmup (%)",28,TRUE,0},
{"AIRcorr",3,0,0,"Air Density (%)",25,TRUE,0},
{"VE_1",3,0,0,"VE 1 (%)",30,TRUE,0},
{"TPSaccel",3,0,0,"Accel (ms)",29,TRUE,0},

/* Other usable controls,  but not normally used */
{"VE_2",-1,-1,-1,"VE 2 (%)",31,FALSE,0},
{"SparkAngle",-1,-1,-1,"SparkAngle",38,FALSE,0},
{"TPS_Volts",-1,-1,-1,"TPS (Volts)",5,FALSE,0},
{"MAP_Volts",-1,-1,-1,"MAP (Volts)",6,FALSE,0},
{"BARO_Volts",-1,-1,-1,"BARO (Volts)",7,FALSE,0},
{"MAT_Volts",-1,-1,-1,"MAT (Volts)",8,FALSE,0},
{"CLT_Volts",-1,-1,-1,"CLT (Volts)",9,FALSE,0},
{"TPS_Counts",-1,-1,-1,"TPS (Counts)",10,FALSE,0},
{"MAP_Counts",-1,-1,-1,"MAP (Counts)",11,FALSE,0},
{"BARO_Counts",-1,-1,-1,"BARO (Counts)",12,FALSE,0},
{"MAT_Counts",-1,-1,-1,"MAT (Counts)",13,FALSE,0},
{"CLT_Counts",-1,-1,-1,"CLT (Counts)",14,FALSE,0},
{"O2_Counts",-1,-1,-1,"O<sub>2</sub> (Counts)",21,FALSE,0},
{"BATT_Counts",-1,-1,-1,"BATT (Counts)",24,FALSE,0},
{"BARO",-1,-1,-1,"BARO (kPa)",17,FALSE,0},
{"CycleTimeH",-1,-1,-1,"CycleTimeH",36,FALSE,0},
{"CycleTimeL",-1,-1,-1,"CycleTimeL",37,FALSE,0},
{"BSPOT1",-1,-1,-1,"BSPOT1",39,FALSE,0},
{"BSPOT2",-1,-1,-1,"BSPOT2",40,FALSE,0},
{"BSPOT3",-1,-1,-1,"BSPOT3",41,FALSE,0}
};


#endif
