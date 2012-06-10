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
  \file include/api-versions.h
  \ingroup Headers
  \brief API Versions for the various config files used in Mtx
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __API_VERSIONS_H__
#define __API_VERSIONS_H__

/* BACKUP API VERSIONS */
#define BACKUP_MAJOR_API 1
#define BACKUP_MINOR_API 1

/* INTERROGATE API VERSIONS */
#define INTERROGATE_MAJOR_API 1
#define INTERROGATE_MINOR_API 11

/* REALTIME MAP API VERSIONS */
#define RTV_MAP_MAJOR_API 1
#define RTV_MAP_MINOR_API 7

/* REALTIME SLIDERS API VERSIONS */
#define RT_SLIDERS_MAJOR_API 1
#define RT_SLIDERS_MINOR_API 0

/* REALTIME STATUS API VERSIONS */
#define RT_STATUS_MAJOR_API 2
#define RT_STATUS_MINOR_API 0

/* REALTIME TEXT API VERSIONS */
#define RT_TEXT_MAJOR_API 1
#define RT_TEXT_MINOR_API 0

/* MTXSOCKET API VERSION */
#define MTXSOCKET_MAJOR_API 0
#define MTXSOCKET_MINOR_API 1

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
