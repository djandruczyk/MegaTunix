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


/* Megasquirt unions defined in C provided by Perry Harrington */

/* Differing endian systems need to proper setups for bitfields.  
 * Intel is little endian (LSB) Sparc is Big Endian, (MSB) as well
 * as a good number of PDA's that run linux... 
 */

/* unions used in the various structures... */
#ifdef WORDS_BIGENDIAN /* PDA's possibly? */
/* Big endian systems (MSB) */
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
union engine
{
        guchar      value;
        struct
        {
                guchar reserved	:1;
                guchar mapaen	:1;     /* 0 = not in MAP acceleration mode 1 = MAP deaceeleration mode */
                                                  
                guchar tpsden	:1;     /* 0 = not in deacceleration mode 1 = in deacceleration mode */
                                                  
                guchar tpsaen	:1;     /* 0 = not in TPS acceleration mode 1 = TPS acceleration mode */
                                                  
                guchar warmup	:1;     /* 0 = not in warmup 1 = in warmup */
                                                  
                guchar startw	:1;     /* 0 = not in startup warmup 1 =in warmup enrichment */
                                                  
                guchar crank	:1;     /* 0 = engine not cranking 1 = engine cranking */
                                                  
                guchar running	:1;     /* 0 = engine not running 1 = running */
                                                  
        } bit;
};
#define MAPACCEL_BIT	1 << 1
#define DECEL_BIT	1 << 2
#define ACCEL_BIT	1 << 3
#define WARMUP_BIT	1 << 4
#define ASE_BIT		1 << 5
#define CRANK_BIT	1 << 6
#define RUNNING_BIT	1 << 7

/* Big endian systems (MSB) */
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
union engine
{
        guchar      value;
        struct
        {
                guchar running   :1;     /* 0 = engine not running 1 = running */
                guchar crank     :1;     /* 0 = engine not cranking 1 = engine cranking */
                guchar startw    :1;     /* 0 = not in startup warmup 1 = in warmup enrichment */
                guchar warmup    :1;     /* 0 = not in warmup 1 = in warmup */
                guchar tpsaen    :1;     /* 0 = not in TPS acceleration mode 1 = TPS acceleration mode */
                guchar tpsden    :1;     /* 0 = not in deacceleration mode 1 = in deacceleration mode */
                guchar mapaen    :1;     /* 0 = not in MAP acceleration mode 1 = MAP deaceeleration mode */

                guchar reserved  :1;
        } bit;
};
#define RUNNING_BIT	1 << 0
#define CRANK_BIT	1 << 1
#define ASE_BIT		1 << 2
#define WARMUP_BIT	1 << 3
#define ACCEL_BIT	1 << 4
#define DECEL_BIT	1 << 5
#define MAPACCEL_BIT	1 << 6

/* Little Endian systems (LSB), intel x86) */
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
#endif // Endian-ness check

#endif
