/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/libreems/libreems_globaldefs.h
  \ingroup LibreEMSPlugin,Headers
  \brief LibreEMS Global defines
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LIBREEMS_GLOBALDEFS_H__
#define __LIBREEMS_GLOBALDEFS_H__

/* Individual bits WARNING, do not not these for notted versions, use the notted ones below instead : http://supp.iar.com/Support/?note=12582&from=search+result */
#define BIT0            0x01    /* 1st bit      = 1             */
#define BIT1            0x02    /* 2nd bit      = 2             */
#define BIT2            0x04    /* 3rd bit      = 4             */
#define BIT3            0x08    /* 4th bit      = 8             */
#define BIT4            0x10    /* 5th bit      = 16    */
#define BIT5            0x20    /* 6th bit      = 32    */
#define BIT6            0x40    /* 7th bit      = 64    */
#define BIT7            0x80    /* 8th bit      = 128   */

#define BIT0_16         0x0001  /* 1st bit      = 1             */
#define BIT1_16         0x0002  /* 2nd bit      = 2             */
#define BIT2_16         0x0004  /* 3rd bit      = 4             */
#define BIT3_16         0x0008  /* 4th bit      = 8             */
#define BIT4_16         0x0010  /* 5th bit      = 16    */
#define BIT5_16         0x0020  /* 6th bit      = 32    */
#define BIT6_16         0x0040  /* 7th bit      = 64    */
#define BIT7_16         0x0080  /* 8th bit      = 128   */

#define BIT8_16         0x0100  /* 9th bit      = 256   */
#define BIT9_16         0x0200  /* 10th bit     = 512   */
#define BIT10_16        0x0400  /* 11th bit     = 1024  */
#define BIT11_16        0x0800  /* 12th bit     = 2048  */
#define BIT12_16        0x1000  /* 13th bit     = 4096  */
#define BIT13_16        0x2000  /* 14th bit     = 8192  */
#define BIT14_16        0x4000  /* 15th bit     = 16384 */
#define BIT15_16        0x8000  /* 16th bit     = 32768 */

/* Individual bits NOTTED : http://supp.iar.com/Support/?note=12582&from=search+result */
#define NBIT0           0xFE    /* 1st bit      = 1             */
#define NBIT1           0xFD    /* 2nd bit      = 2             */
#define NBIT2           0xFB    /* 3rd bit      = 4             */
#define NBIT3           0xF7    /* 4th bit      = 8             */
#define NBIT4           0xEF    /* 5th bit      = 16    */
#define NBIT5           0xDF    /* 6th bit      = 32    */
#define NBIT6           0xBF    /* 7th bit      = 64    */
#define NBIT7           0x7F    /* 8th bit      = 128   */

#define NBIT0_16        0xFFFE  /* 1st bit      = 1             */
#define NBIT1_16        0xFFFD  /* 2nd bit      = 2             */
#define NBIT2_16        0xFFFB  /* 3rd bit      = 4             */
#define NBIT3_16        0xFFF7  /* 4th bit      = 8             */
#define NBIT4_16        0xFFEF  /* 5th bit      = 16    */
#define NBIT5_16        0xFFDF  /* 6th bit      = 32    */
#define NBIT6_16        0xFFBF  /* 7th bit      = 64    */
#define NBIT7_16        0xFF7F  /* 8th bit      = 128   */

#define NBIT8_16        0xFEFF  /* 9th bit      = 256   */
#define NBIT9_16        0xFDFF  /* 10th bit     = 512   */
#define NBIT10_16       0xFBFF  /* 11th bit     = 1024  */
#define NBIT11_16       0xF7FF  /* 12th bit     = 2048  */
#define NBIT12_16       0xEFFF  /* 13th bit     = 4096  */
#define NBIT13_16       0xDFFF  /* 14th bit     = 8192  */
#define NBIT14_16       0xBFFF  /* 15th bit     = 16384 */
#define NBIT15_16       0x7FFF  /* 16th bit     = 32768 */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
