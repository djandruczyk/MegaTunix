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

#ifndef __DATALOGGING_CONST_H__
#define __DATALOGGING_CONST_H__

#include <gtk/gtk.h>
#include <enums.h>
#include <structures.h>


/* logable_names[] is an array of textual names corresponding to all logable
 * variables from the MS
 */
static const gchar *logable_names[] = 
{
"HR_Clock",	"MS_Clock",	"RPM",		"EngineBits",	"Idle_DC",
"TPS_Volts",	"MAP_Volts",	"BARO_Volts",	"MAT_Volts",	"CLT_Volts",
"TPS_Counts",	"MAP_Counts",	"BARO_Counts",	"MAT_Counts",	"CLT_Counts",
"TPS_%",	"MAP_KPA",	"BARO_KPA",	"MAT_(Deg)",	"CLT_(Deg)",
"O2_Volts",	"O2_Counts",	"GammaE",	"BATT_Volts",	"BATT_Counts",
"AIRcorr",	"BAROcorr",	"EGOcorr",	"WARMcorr",	"TPSaccel",
"VE1",		"VE2",		"PW1",		"PW2",		"DutyCycle1",
"DutyCycle2",	"CycleTimeH",	"CycleTimeL",	"SparkAngle",	"BSPOT1",	"BSPOT2",	"BSPOT3"
};

static const gchar *mt_classic_names[] = 
{
"Time",		"Seconds",	"RPM",		"EngineBit",	"IdleDC",
"TPS_Volts",	"MAP_Volts",	"BARO_Volts",	"MAT_Volts",	"CLT_Volts",
"TPS_Counts",	"MAP_Counts",	"BARO_Counts",	"MAT_Counts",	"CLT_Counts",
"TP",		"MAP",		"BARO",		"MAT",		"CLT",
"O2",		"O2_Counts",	"Gammae",	"BATT_Volts",	"BATT_Counts",
"Gair",		"Gbaro",	"Gego",		"Gwarm",	"TPSacc",
"Gve",		"Gve2",		"PW",		"PW2",		"DutyCycle1",
"DutyCycle2",	"CycleTimeH",	"CycleTimeL",	"SparkAngle",	"BSPOT1",	"BSPOT2",	"BSPOT3"
};

static const gchar *mt_full_names[] = 
{
"Time",		"SecL",		"RPM",		"Engine",	"IdleDC",
"TPS_Volts",	"MAP_Volts",	"BARO_Volts",	"MAT_Volts",	"CLT_Volts",
"TPS_Counts",	"MAP_Counts",	"BARO_Counts",	"MAT_Counts",	"CLT_Counts",
"TP",		"MAP",		"BARO",		"MAT",		"CLT",
"O2",		"O2_Counts",	"Gammae",	"BATT_Volts",	"BATT_Counts",
"Gair",		"Gbaro",	"Gego",		"Gwarm",	"TPSacc",
"Gve",		"Gve2",		"PW",		"PW2",		"DutyCycle1",
"DutyCycle2",	"CycleTimeH",	"CycleTimeL",	"SparkAngle",	"UserData1",	"UserData2",	"UserData3"
};

static const Capabilities dlog_caps[] = 
{
STANDARD,	STANDARD,	STANDARD,	STANDARD,	DUALTABLE|IAC_PWM,
STANDARD,	STANDARD,	STANDARD,	STANDARD,	STANDARD,
STANDARD,	STANDARD,	STANDARD,	STANDARD,	STANDARD,
STANDARD,	STANDARD,	STANDARD,	STANDARD,	STANDARD,
STANDARD,	STANDARD,	STANDARD,	STANDARD,	STANDARD,
STANDARD,	STANDARD,	STANDARD,	STANDARD,	STANDARD,
STANDARD,	STANDARD,	STANDARD,	DUALTABLE,	STANDARD,
DUALTABLE,S_N_SPARK|S_N_EDIS,S_N_SPARK|S_N_EDIS,S_N_SPARK|S_N_EDIS, STANDARD,
STANDARD,	STANDARD
};

/* logging_offset_map is a mapping between the logable_names[] list above 
 * and the byte offset into the Runtime_Common datastructure. The index 
 * is the index number of the logable variable from above, The value at that
 * index point is the offset into the struct for the data. Offset 0
 * is the first value in the struct (secl), offset 99 is a special case
 * for the Hi-Res clock which isn't stored in the structure...
 */
static const gint logging_offset_map[] =
{
        99,54,52,56,74,
        24,16, 0,20, 8,
        68,66,62,67,64,
        44,60,59,50,48,
        12,65,61, 4,63,
        69,70,71,73,72,
        57,58,36,40,28,
        32,75,76,77,78,
	79,80
};

/* mt_classic_order[] is an arry signifying the ORDER that the variables 
 * are displayed in the logfile.  This is done ONLY to make the logs 
 * 100% compatible with ms-tweek3K. -1 means it's not logged, anything from
 * 1 on up signifies the order in which they get logged to the logfile...
 */
static const gint mt_classic_order[] = 
{
	-1, 0, 1, 4,-1,
	-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,
	-1, 2,-1,-1,-1,
	 3,-1,10,-1,-1,
	 6, 8, 5, 7,-1,
	 9,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,
	-1,-1
};

/* mt_full_order[] is an arry signifying the ORDER that the variables 
 * are displayed in the logfile.  This is done ONLY to make the logs 
 * 100% compatible with ms-tweek3K. -1 means it's not logged, anything from
 * 0 on up signifies the order in which they get logged to the logfile...
 */
static const gint mt_full_order[] = 
{
	 0, 1, 2, 8,-1,
	-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,
	 4, 3,-1, 6, 7,
	 5,-1,13,-1,-1,
	10,12, 9,11,14,
	15,17,16,18,19,
	20,-1,-1,-1,21,
	22,23
};

/* Size of each logable variable in BYTES, so 4 = a 32 bit var 
 * This strange array is needed only forthe fact that the datalogging
 * functions get all of their data from the Runtime_Common struct.
 * that struct has been deigned (And is necessary) to have all the 
 * largest varibles first (floats before shorts before chars), so that
 * the structure can be referenced in array notation, this way all data
 * is accessible by the appropriate index.
 * The values in this array correspond to the size of the referenced
 * variable in that Runtime_Common struct.
 */
static const gint logging_datasizes_map[] =
{
        FLOAT,UCHAR,SHORT,UCHAR,UCHAR,
        FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,
        UCHAR,UCHAR,UCHAR,UCHAR,UCHAR,
        FLOAT,UCHAR,UCHAR,SHORT,UCHAR,
        FLOAT,UCHAR,UCHAR,FLOAT,UCHAR,
        UCHAR,UCHAR,UCHAR,UCHAR,UCHAR,
        UCHAR,UCHAR,FLOAT,FLOAT,FLOAT,
        FLOAT,UCHAR,UCHAR,UCHAR,UCHAR,
	UCHAR,UCHAR
};

/* Tooltips for each of the above.... */
static const gchar * logable_names_tips[] = 
{
"\"HR Clock\" is a High resolution clock recorded to the logfile by this pc when data arrives, resolution of 1 millisecond (stored in seconds)",
"\"MS Clock\" is the MegaSquirt clock returned by the ECU, this runs from 0-255 and recylcles indefinitely, resolution 1 second.",
"\"RPM\" Is the engine RPM, stored in the log as it's actual RPM, resolution 100RPM",
"\"EngineBits\" is a bitfield containing information such as cranking/running, accel enrich, decel mode and warmup enrichment.  It require decoding, use the information in globals.h for the \"engine\" union.",
"\"IdleDC\" is the duty cycle for the Idle control (ONLY available when using dualtable firmware",
"\"TPS Volts\" is the reading from the Throttle Position sensor converted to volts. Range is 0-5 volts, resolution of 19.6 millivolts (8 bits)",
"\"MAP Volts\" is the reading from the Manifold Absolute Pressure sensor converted to volts. Range is 0-5 volts, resolution of 19.6 millivolts (8 bits)",
"\"BARO Volts\" is the reading from the Manifold Absolute Pressure sensor before engine startup converted to volts. Range is 0-5 volts, resolution of 19.6 millivolts (8 bits)",
"\"MAT Volts\" is the reading from the Manifold Air Temperature sensor converted to volts. Range is 0-5 volts, resolution of 19.6 millivolts (8 bits)",
"\"CLT Volts\" is the reading from the Engine CooLant Temperature sensor converted to volts. Range is 0-5 volts, resolution of 19.6 millivolts (8 bits)",
"\"TPS Counts\" is the raw ADC counts from the Throttle Position sensor reading. Range is 0-255 counts, resolution of 1 counts",
"\"MAP Counts\" is the raw ADC counts from the Manifold Absolute Pressure sensor reading. Range is 0-255 counts, resolution of 1 counts",
"\"BARO Counts\" is the raw ADC counts from the Manifold Absolute Pressure sensor before engine startup. Range is 0-255 counts, resolution of 1 counts",
"\"MAT Counts\" is the raw ADC counts from the Manifold Air Temperature sensor reading. Range is 0-255 counts, resolution of 1 counts",
"\"CLT Counts\" is the raw ADC counts from the CooLant Temperature sensor reading. Range is 0-255 counts, resolution of 1 counts",
"\"TPS %\" is the Throttle Position in percentage of open (based on counts, a count of 255 is equal to 100% open)",
"\"MAP KPA\" is the Manifold Absolute Pressure in KPA (Kilopascals), 100KPA is atmospheric pressure at seal level.",
"\"BARO KPA\" is the Manifold Absolute Pressure BEFORE engine startup in KPA (Kilopascals), 100KPA is atmospheric pressure at seal level.",
"\"MAT (Deg)\", is the Manifold Air Temperature in conventional thermal units (centigrade of fahrenheit depending on what you've set in the General Tab)",
"\"CLT (Deg)\", is the Engine CooLant Temperature in conventional thermal units (centigrade of fahrenheit depending on what you've set in the General Tab)",
"\"O2 Volts\" is the Exhaust Oxygen Sensor voltage. Range 0-5 Volts, resolution 19.6 mv",
"\"O2 Counts\" is the raw ADC counts for the Exhaust Gas Oxygen reading, Range 0-255, resolution 1 count",
"\"GammaE\" is the summation of ALL enrichments (temp enrich,accel enrich, warmup enrich, etc..) in the MegaSquirt ECU.",
"\"BATT Volts\" is the Battery voltage in DC volts",
"\"BATT Counts\" is the raw ADC counts for the Battery Voltage input, Range 0-255, resolution 1 count",
"\"AIRcorr\" is the Air temperature correction factor.  This is expressed as a percentage, 100 being NO enrichment, anything over 100 causes additional fuel to be added,  less than 100 means less fuel.",
"\"BAROcorr\" is the Barometer correction factor.  This is expressed as a percentage, 100 being NO enrichment, anything over 100 causes additional fuel to be added,  less than 100 means less fuel.",
"\"EGOcorr\" is the Exhaust Gas Correction factor.  This is expressed as a percentage, 100 being NO enrichment, anything over 100 causes additional fuel to be added,  less than 100 means less fuel. The EGO Feedback settings on the Enrichments tab have the most effect on this parameter.",
"\"WARMcorr\" is the Warmup Correction factor.  This influenced by the Coolant Temp sensor input and the Warmup Enrichment bins on the Enrichments tab.  This is expressed as a percentage, 100 being NO enrichment, anything over 100 causes additional fuel to be added,  less than 100 means less fuel.",
"\"TPSaccel\" is the amount of Acceleration enrichment being dispensed. Units are in milliseconds x10, so 10 ms shows upa as 100 in the log",
"\"VE1\" is the instantaneous volumetric efficiency that the MS calculating fuel for for channel 1. NOTE: Dualable variants can have differing values for each table.  For standard MegaSquirts this is the global VE, as both injector channels run off of one table.",
"\"VE2\" is the instantaneous volumetric efficiency for the MS for table 2 (Dualtable code variants ONLY), this value is undefined for standard MegaSquirt ECU's",
"\"PW1\" is the pulsewidth in milliseconds for the injectors tied to channel 1. (and channel two for NON-dualtable MegaSquirt).",
"\"PW2\" is the pulsewidth in millisecondws for the injectors tied to channel 2. (Dualtable ONLY, this is undefined for standard MegaSquirt)",
"\"INJ-1 Dcycle\" is the injector Duty cycle for channel 1 (and channel 2 on standard MegaSquirt ECU's)",
"\"INJ-2 Dcycle\" is the injectior Duty Cycle for channel 2 (Dualtable code variants ONLY, this is undefiend for Standard MegaSquirts)", 
"\"CycleTimeH\" is unknown (from MegaSquirtnEDIS  code)", 
"\"CycleTimeL\" is unknown (from MegaSquirtnEDIS  code)", 
"\"SparkAngle\" is unknown (from MegaSquirtnEDIS  code)", 
"\"BSPOT1\" is one of three blank spots returned in the runtime variables for standard MegaSquirt ECUs. Some of the less known MS variants use these to communicate back additional data.  These are loggable just for this reasons.  Units are 0-255, resolution of 1.",
"\"BSPOT2\" is one of three blank spots returned in the runtime variables for standard MegaSquirt ECUs. Some of the less known MS variants use these to communicate back additional data.  These are loggable just for this reasons.  Units are 0-255, resolution of 1.",
"\"BSPOT3\" is one of three blank spots returned in the runtime variables for standard MegaSquirt ECUs. Some of the less known MS variants use these to communicate back additional data.  These are loggable just for this reasons.  Units are 0-255, resolution of 1."
};
#endif
