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
  \file include/configfile.h
  \ingroup Headers
  \brief Headers for the .ini processing code
  \author David Andruczyk
  */

/* Configfile structs. (derived from an older version of XMMS) */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

#include <glib.h>

/* Structures */

/*!
 \brief The ConfigLine struct stores just the key and value for a Line within
 a ConfigSection
 \see ConfigSection
 */
typedef struct
{
        gchar *key;		/*!< Key */
        gchar *value;		/*!< Value */
}
ConfigLine;

/*!
 \brief The ConfigSection struct stores the section name anda GList of 
 lines 
 */
typedef struct
{
        gchar *name;		/*!< Section Name */
        GList *lines;		/*!< List of Lines in that Section */
}
ConfigSection;

/*!
 \brief The ConfigFile struct stores a GList of Sections and the filename
 \see ConfigSection
 */
typedef struct
{
        GList *sections;	/*!< List of Sections */
	gchar * filename;	/*!< File Name */
}
ConfigFile;

/* Structures */

/* Prototypes */
ConfigSection *cfg_find_section(ConfigFile * cfg, const gchar * name);
void cfg_free(ConfigFile * cfg);
ConfigFile *cfg_new(void);
ConfigFile *cfg_open_file(const gchar * filename);
gboolean cfg_read_boolean(ConfigFile * cfg, const gchar * section, \
                const gchar * key, gboolean * value);
gboolean cfg_read_double(ConfigFile * cfg, const gchar * section, \
                const gchar * key, gdouble * value);
gboolean cfg_read_float(ConfigFile * cfg, const gchar * section, \
                const gchar * key, gfloat * value);
gboolean cfg_read_int(ConfigFile * cfg, const gchar * section, \
                const gchar * key, gint * value);
gboolean cfg_read_string(ConfigFile * cfg, const gchar * section, \
                const gchar * key, gchar ** value);
void cfg_remove_key(ConfigFile * cfg, const gchar * section, const gchar * key);
void cfg_write_boolean(ConfigFile * cfg, const gchar * section, \
                const gchar * key, gboolean value);
gboolean cfg_write_file(ConfigFile * cfg, const gchar * filename);
void cfg_write_double(ConfigFile * cfg, const gchar * section, \
                const gchar * key, gdouble value);
void cfg_write_float(ConfigFile * cfg, const gchar * section, \
                const gchar * key, gfloat value);
void cfg_write_int(ConfigFile * cfg, const gchar * section, \
                const gchar * key, gint value);
void cfg_write_string(ConfigFile * cfg, const gchar * section, \
                const gchar * key, const gchar * value);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
