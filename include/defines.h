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

#ifndef __DEFINES_H__
#define __DEFINES_H__


/* Definitions */
#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define MS_PAGE_SIZE 128

/* Memory offsets */
#define VE1_TABLE_OFFSET	0	/* From page 0 boundary */
#define VE2_TABLE_OFFSET	MS_PAGE_SIZE	/* From Page 0 boundary */
#define WARMUP_BINS_OFFSET	68	/* From page 0 boundary */
#define ACCEL_BINS_OFFSET	78	/* From page 0 boundary */
#define VE1_RPM_BINS_OFFSET	100	/* From page 0 boundary */
#define VE2_RPM_BINS_OFFSET	100+MS_PAGE_SIZE /* From Page 0 boundary */
#define VE1_KPA_BINS_OFFSET	108	/* From page 0 boundary */
#define VE2_KPA_BINS_OFFSET	108+MS_PAGE_SIZE /* From Page 0 boundary */
#define DIV_OFFSET_1		91	/* Where "divider" is */
#define DIV_OFFSET_2		91+MS_PAGE_SIZE	/* Where "divider" is */
#define IGN_TABLE_OFFSET	0	/* From ign table boundary */
#define IGN_RPM_BINS_OFFSET	64	/* From ign table boundary */
#define IGN_KPA_BINS_OFFSET	72	/* From ign table boundary */

/* Download modes */
#define IMMEDIATE		0x10
#define DEFERRED		0x11

/* Number of axis bins */
#define RPM_BINS_MAX		8
#define LOAD_BINS_MAX		8
#define TABLE_BINS_MAX		RPM_BINS_MAX*LOAD_BINS_MAX


/* For datalogging and Logviewer */
#define TABLE_COLS 5
#define MAX_LOGABLES 64

#define UCHAR sizeof(unsigned char)
#define FLOAT sizeof(float)
#define SHORT sizeof(short)

#endif
