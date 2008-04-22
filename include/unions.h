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

#ifndef __UNIONS_H__
#define __UNIONS_H__

#include <gtk/gtk.h>

/* Megasquirt unions defined in C provided by Perry Harrington */

/* Differing endian systems need to proper setups for bitfields.  
 * Intel is little endian (LSB) Sparc is Big Endian, (MSB) as well
 * as a good number of PDA's that run linux... 
 */

/* unions used in the various structures... */
#ifdef WORDS_BIGENDIAN /* PDA's possibly? */
/* Big endian systems (MSB) */

/*!
 \brief the Squirt union contains bits showingthe injection status events
 This structure isn't used anyplace yet.
 */
union squirt
{
        guchar   value;
        struct
        {
                guchar reserved  :2;
                guchar firing2   :1;     /* 0 = not squirting 1 = squirti
                                                   ng */
                guchar sched2    :1;     /* 0 = nothing scheduled 1 = sch
                                                   eduled to squirt */
                guchar firing1   :1;     /* 0 = not squirting 1 = squirti
                                                   ng */
                guchar sched1    :1;     /* 0 = nothing scheduled 1 = sch
                                                   eduled to squirt */
                guchar inj2      :1;     /* 0 = no squirt 1 = squirt */
                guchar inj1      :1;     /* 0 = no squirt 1 = squirt */
        } bit;
};


/* Big endian systems (MSB) */
/*!
 \brief the engine union contains bits showing the engien status including
 the following mode, mapaccel, tpsaccel, tpsdecel, warmup_enrich, 
 afterstart_enrich, cranking and running
 */
union engine
{
        guchar      value;
        struct
        {
                guchar reserved	:1;
                guchar mapaccel	:1;     /* 0 = not in MAP acceleration mode 1 = MAP deaceeleration mode */
                                                  
                guchar tpsdecel	:1;     /* 0 = not in deacceleration mode 1 = in deacceleration mode */
                                                  
                guchar tpsaccel	:1;     /* 0 = not in TPS acceleration mode 1 = TPS acceleration mode */
                                                  
                guchar warmup	:1;     /* 0 = not in warmup 1 = in warmup */
                                                  
                guchar ase	:1;     /* 0 = not in afterstart enrich 1 = in afterstart enrichment */
                                                  
                guchar crank	:1;     /* 0 = engine not cranking 1 = engine cranking */
                                                  
                guchar running	:1;     /* 0 = engine not running 1 = running */
                                                  
        } bit;
};


/* Big endian systems (MSB) */
/*!
 \brief the config11 unions embedds the map_sensor type(115/250 kpa), 
 engine_type (2/4 stroke), injection type (tbi/multiport), and the number of
 cylinders
 */
union config11
{
        guchar      value;
        struct
        {
                guchar cylinders	:4;     /* 0000 = 1 cyl 
                                                 * 0001 = 2 cyl
                                                 * 0010 = 3 cyl
                                                 * 0011 = 4 cyl
                                                 * 0100 = 5 cyl
                                                 * 0101 = 6 cyl
                                                 * 0110 = 7 cyl
                                                 * 0111 = 8 cyl
                                                 * 1000 = 9 cyl
                                                 * 1001 = 10 cyl
                                                 * 1010 = 11 cyl
                                                 * 1011 = 12 cyl
                                                 */
                guchar inj_type		:1;     /* 0 = multi-port, 1 = TBI */
                guchar eng_type		:1;     /* 0 = 4-stroke, 1 = 2-stroke */
                guchar map_type		:2;     /* 00 115KPA, 01-250kpa, 10,11
                                                 * user-defined */
        } bit;
};


/* Big endian systems (MSB) */
/*!
 \brief the config12 unions embedds the mat sensor type(gm/unused), 
 clt sensor type (gm/undefined), and the number of injectors
 */
union config12
{
        guchar      value;
        struct
        {
                guchar injectors	:4;     /* 0000 = 1 injector 
                                                 * 0001 = 2 injectors
                                                 * 0010 = 3 injectors
                                                 * 0011 = 4 injectors
                                                 * 0100 = 5 injectors
                                                 * 0101 = 6 injectors
                                                 * 0110 = 7 injectors
                                                 * 0111 = 8 injectors
                                                 * 1000 = 9 injectors
                                                 * 1001 = 10 injectors
                                                 * 1010 = 11 injectors
                                                 * 1011 = 12 injectors
                                                 */
                guchar mat_type		:2;     /* 00 = GM,
                                                 * 01,10,11 = user defined */
                guchar clt_type		:2;     /* 00 = GM, 
                                                 * 01,10,11 = user defined */
        } bit;
};


/* Big endian systems (MSB) */
/*!
 \brief the config13 unions embedds the firing type (even/odd),
 the ego sensor type (nbo2/wbo2), injection strategy (SpeedDensity/Alpha-N),
 baro correction (enabled/disabled), idle policy (B&G/PWM)
 */
union config13
{
        guchar      value;
        struct
        {
                guchar unused		:3;         /* Last 3 bits... */
                /*Idle policy is IAC or DUALTABLE CODE SPECIFIC*/
                guchar idle_policy	:1;    /* 0 = B%G style, */
						/* 1 = Brian Fielding PWM */
                guchar baro_corr	:1;     /* 0 = Enrichment off (100%) */
                                                /* 1 = Enrichment on  */
                guchar inj_strat	:1;     /* 0 = SD, 1 = Alpha-N */
                guchar ego_type 	:1;     /* 0 = narrow, 1=wide */
                guchar firing		:1;     /* 0 = normal, 1=odd fire */

        } bit;
};


/* Big Endian systems (MSB) */
/*!
 \brief the bcfreq union embedds the boost controller frequency choices
, 39hz, 19hz and 10hz and nothing else
 */
union bcfreq
{
	guchar 	value;
	struct
	{
		guchar unused		:6;
		guchar freq		:2;	/* 00 ERROR, 01 = 39 Hz, 
						 * 10 = 19Hz, 11 = 10 Hz
						 */
	} bit;
};


/* Big Endian systems (MSB) */
/*!
 \brief the tblcnf union embedds the injection mode (simultaneous B&G/DT Mode),
 the injector_1 mapping (inj1 off/from table1/from table2),
 the injector_2 mapping (inj1 off/from table1/from table2),
 whether gammae (sum of enrichments) is applied to injector channel 1, and
 whether gammae (sum of enrichments) is applied to injector channel 2
 */
union tblcnf
{
	guchar 	value;
	struct
	{
		guchar unused		:1;	/* not used */
		guchar gammae2		:1;	/* 0 Gammae NOT applied to 
						 * Injector channel 2 */
						/* 1 Gammae applied to inj 2 */
		guchar gammae1		:1;	/* 0 Gammae NOT applied to 
						 * Injector channel 1 */
						/* 1 Gammae applied to inj 1 */
		guchar inj2		:2;	/* 00 injector 2 not driven */
						/* 01 driven from table 1 */
						/* 10 driven from table 2 */
						/* 11 undefined */
		guchar inj1		:2;	/* 00 injector 1 not drivem */
						/* 01 driven from table 1 */
						/* 10 driven from table 2 */
						/* 11 undefined */
		guchar simul		:1;	/* 0 both injectors controlled
						 * from table 1, rest of byte
						 * ignored */
						/* 1 additional modes activated
						 * as bits 1-7 */
	}bit;
};

/* Big Endian systems (MSB) */
/*!
 \brief the spark_config1 union embeds the boost_retard, multi_spark, 
 extra_long_trigger and long_trigger options specific to EDIS and spark
 firmwares
 */
union spark_config1
{
	guchar 	value;
	struct
	{
		guchar unused		:4;
		guchar boost_ret	:1;	/* (EDIS) Boost Retard ? */
		guchar multi_sp		:1;	/* (EDIS) Multi-Spark mode ? */
		guchar xlong_trig	:1;	/* Mask xtra long trigger ? */
		guchar long_trig	:1;	/* Mask long trigger ? */
	} bit;
};


#else
/* Little Endian systems (LSB), intel x86) */


/*!
 \brief the Squirt union contains bits showingthe injection status events
 This structure isn't used anyplace yet.
 */
union squirt
{
        guchar      value;
        struct
        {
                guchar inj1      :1;     /* 0 = no squirt 1 = squirt */
                guchar inj2      :1;     /* 0 = no squirt 1 = squirt */
                guchar sched1    :1;     /* 0 = nothing scheduled 1 = sch
                                                   eduled to squirt */
                guchar firing1   :1;     /* 0 = not squirting 1 = squirti
                                                   ng */
                guchar sched2    :1;     /* 0 = nothing scheduled 1 = sch
                                                   eduled to squirt */
                guchar firing2   :1;     /* 0 = not squirting 1 = squirti
                                                   ng */
                guchar reserved  :2;
        } bit;
};


/* Little Endian systems (LSB), intel x86) */
/*!
 \brief the engine union contains bits showing the engien status including
 the following mode, mapaccel, tpsaccel, tpsdecel, warmup_enrich, 
 afterstart_enrich, cranking and running
 */
union engine
{
        guchar      value;
        struct
        {
                guchar running	:1;     /* 0 = engine not running 1 = running */
                guchar crank	:1;     /* 0 = engine not cranking 1 = engine cranking */
                guchar ase	:1;     /* 0 = not in afterstart enrich 1 = in afterstart enrichment */
                guchar warmup	:1;     /* 0 = not in warmup 1 = in warmup */
                guchar tpsaccel	:1;     /* 0 = not in TPS acceleration mode 1 = TPS acceleration mode */
                guchar tpsdecel	:1;     /* 0 = not in deacceleration mode 1 = in deacceleration mode */
                guchar mapaccel	:1;     /* 0 = not in MAP acceleration mode 1 = MAP deaceeleration mode */

                guchar reserved  :1;
        } bit;
};


/* Little Endian systems (LSB), intel x86) */
/*!
 \brief the config11 unions embedds the map_sensor type(115/250 kpa), 
 engine_type (2/4 stroke), injection type (tbi/multiport), and the number of
 cylinders
 */
union config11
{
        guchar      value;
        struct
        {
                guchar map_type  :2;     /* 00 115KPA, 01-250kpa, 10,11
                                                 * user-defined */
                guchar eng_type  :1;     /* 0 = 4-stroke, 1 = 2-stroke */
                guchar inj_type  :1;     /* 0 = multi-port, 1 = TBI */
                guchar cylinders :4;     /* 0000 = 1 cyl 
                                                 * 0001 = 2 cyl
                                                 * 0010 = 3 cyl
                                                 * 0011 = 4 cyl
                                                 * 0100 = 5 cyl
                                                 * 0101 = 6 cyl
                                                 * 0110 = 7 cyl
                                                 * 0111 = 8 cyl
                                                 * 1000 = 9 cyl
                                                 * 1001 = 10 cyl
                                                 * 1010 = 11 cyl
                                                 * 1011 = 12 cyl
                                                 */
        } bit;
};


/* Little Endian systems (LSB), intel x86) */
/*!
 \brief the config12 unions embedds the mat sensor type(gm/unused), 
 clt sensor type (gm/undefined), and the number of injectors
 */
union config12
{
        guchar      value;
        struct
        {
                guchar clt_type  :2;     /* 00 = GM, 
                                                 * 01,10,11 = user defined */
                guchar mat_type  :2;     /* 00 = GM,
                                                 * 01,10,11 = user defined */
                guchar injectors :4;     /* 0000 = 1 injector 
                                                 * 0001 = 2 injectors
                                                 * 0010 = 3 injectors
                                                 * 0011 = 4 injectors
                                                 * 0100 = 5 injectors
                                                 * 0101 = 6 injectors
                                                 * 0110 = 7 injectors
                                                 * 0111 = 8 injectors
                                                 * 1000 = 9 injectors
                                                 * 1001 = 10 injectors
                                                 * 1010 = 11 injectors
                                                 * 1011 = 12 injectors
                                                 */
        } bit;
};


/* Little Endian systems (LSB), intel x86) */
/*!
 \brief the config13 unions embedds the firing type (even/odd),
 the ego sensor type (nbo2/wbo2), injection strategy (SpeedDensity/Alpha-N),
 baro correction (enabled/disabled), idle policy (B&G/PWM)
 */
union config13
{
        guchar      value;
        struct
        {
                guchar firing		:1;	/* 0 = normal, 1=odd fire */
                guchar ego_type 	:1;	/* 0 = narrow, 1=wide */
                guchar inj_strat	:1;	/* 0 = SD, 1 = Alpha-N */
                guchar baro_corr	:1;	/* 0 = Enrichment off (100%) */
                                                /* 1 = Enrichment on  */
		/* idle_policy is IAC or Dualtable CODE SPECIFIC */
                guchar idle_policy	:1;	/* 0 = B%G style, */
                                                /* 1 = Brian fielding PWM */

                guchar unused		:3;	/* Last 3 bits... */
        } bit;
};


/* Little Endian systems (LSB), intel x86) */
/*!
 \brief the bcfreq union embedds the boost controller frequency choices
, 39hz, 19hz and 10hz and nothing else
 */
union bcfreq
{
	guchar 	value;
	struct
	{
		guchar freq		:2;	/* 00 ERROR, 01 = 39 Hz, 
						 * 10 = 19Hz, 11 = 10 Hz
						 */
		guchar unused		:6;
	} bit;
};


/* Little Endian systems (LSB), intel x86) */
/*!
 \brief the tblcnf union embedds the injection mode (simultaneous B&G/DT Mode),
 the injector_1 mapping (inj1 off/from table1/from table2),
 the injector_2 mapping (inj1 off/from table1/from table2),
 whether gammae (sum of enrichments) is applied to injector channel 1, and
 whether gammae (sum of enrichments) is applied to injector channel 2
 */
union tblcnf
{
	guchar 	value;
	struct
	{
		guchar simul		:1;	/* 0 both injectors controlled
						 * from table 1, rest of byte
						 * ignored */
						/* 1 additional modes activated
						 * as bits 1-7 */
		guchar inj1		:2;	/* 00 injector 1 not drivem */
						/* 01 driven from table 1 */
						/* 10 driven from table 2 */
						/* 11 undefined */
		guchar inj2		:2;	/* 00 injector 2 not driven */
						/* 01 driven from table 1 */
						/* 10 driven from table 2 */
						/* 11 undefined */
		guchar gammae1		:1;	/* 0 Gammae NOT applied to 
						 * Injector channel 1 */
						/* 1 Gammae applied to inj 1 */
		guchar gammae2		:1;	/* 0 Gammae NOT applied to 
						 * Injector channel 2 */
						/* 1 Gammae applied to inj 2 */
		guchar unused		:1;	/* not used */
	}bit;
};


/* Little Endian systems (LSB), intel x86 */
/*!
 \brief the spark_config1 union embeds the boost_retard, multi_spark, 
 extra_long_trigger and long_trigger options specific to EDIS and spark
 firmwares
 */
union spark_config1
{
	guchar 	value;
	struct
	{
		guchar long_trig	:1;	/* Mask long trigger ? */
		guchar xlong_trig	:1;	/* Mask xtra long trigger ? */
		guchar multi_sp		:1;	/* (EDIS) Multi-Spark mode ? */
		guchar boost_ret	:1;	/* (EDIS) Boost Retard ? */
		guchar unused		:4;
	} bit;
};
#endif /* Endian-ness check */

#endif
