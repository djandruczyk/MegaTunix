/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/sleep_calib.c
  \ingroup CoreMtx
  \brief Calibrates the system overhead to adjust the timouts to suite 
  the users machine speed
  \author David Andruczyk
  */

#include <defines.h>
#include <debugging.h>
#include <sleep_calib.h>
#include <stdio.h>

extern gconstpointer *global_data;

/*
  \brief calculates the accuracy of g_usleep and sets up a fudge factor
  to account for OS/platform differences in precision of the call
  */
void sleep_calib(void)
{
	GTimer *timer = NULL;
	gdouble time = 0.0;
	gfloat tmpf = 0.0;
	gfloat *factor = NULL;
	gfloat factors[10] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	gfloat test = 1500;
	gint i = 0;

	ENTER();
	factor = g_new0(gfloat, 1);
	timer = g_timer_new();
	for (i=0;i<10;i++)
	{
		g_timer_start(timer);
		g_usleep(test);
		g_timer_stop(timer);
		time = g_timer_elapsed(timer,NULL);
		factors[i] = test/(time*1000000);
		/*printf("Sleep Correction Factor test %i is %f, %f/%f\n",i,factors[i],test,time*1000000);*/
		tmpf += factors[i];
		g_timer_reset(timer);
	}
	*factor = tmpf/(gfloat)i;
	/*printf("Sleep Correction Factor is %f\n",*factor);*/
	DATA_SET_FULL(global_data,"sleep_correction",(gpointer)factor,g_free);
	g_timer_destroy(timer);
	EXIT();
	return;
}
