/*  XMMS - Cross-platform multimedia player
 *  Copyright (C) 1998-1999  Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Hacked slightly by Dave J. Andruczyk <djandruczyk@yahoo.com> to
 *  fit into MegaTunix a bit better..
 */

/*!
  \file mtxcommon/configfile.c
  \ingroup MtxCommon
  \brief .ini handling functions ported from the XMMS project
  \author David Andruczyk, XMMS
  */

#include <configfile.h>
#include <debugging.h>
#include <glib/gprintf.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
 #define ENTER() ""
 #define EXIT() "" 
#endif
static ConfigSection *cfg_create_section(ConfigFile * cfg, const gchar * name);
static ConfigLine *cfg_create_string(ConfigSection * section, const gchar * key, const gchar * value);

static ConfigLine *cfg_find_string(ConfigSection * section, const gchar * key);


/*!
 \brief Creates a new ConfigFile structure and returns it
 \see ConfigFile
 \returns a pointer to a ConfigFile structure
 */
ConfigFile *cfg_new(void)
{
	ConfigFile *cfg;
	ENTER();

	cfg = (ConfigFile *)g_malloc0(sizeof (ConfigFile));

	EXIT();
	return cfg;
}


/*!
 \brief Opens a file, reads it's entire contents and populates
 the cfg internal structures.
 \param filename: name of file to open and load
 \returns the pointer to a poulated ConfigFile structure
 */
ConfigFile *cfg_open_file(const gchar * filename)
{
	ConfigFile *cfg;
	gchar *tmp = NULL;
	gchar *line = NULL;
	gchar *decomp = NULL;
	gint i = 0;
	GIOChannel * iochannel = NULL;
	GError *error = NULL;
	ConfigSection *section = NULL;

	ENTER();
	iochannel = g_io_channel_new_file(filename,"r",&error);
	if(error)
	{
		g_error_free(error);
		EXIT();
		return NULL;
	}

	cfg = (ConfigFile *)g_malloc0(sizeof (ConfigFile));

	i = 0;
	while (g_io_channel_read_line(iochannel,&line,NULL,NULL,&error) == G_IO_STATUS_NORMAL)
	{
		if (g_str_has_prefix(line,"["))
		{
			if ((tmp = g_strrstr(line, "]")))
			{
				*tmp = '\0';
				section = cfg_create_section(cfg, &line[1]);
			}
		}
		else if ((!g_str_has_prefix(line,"#") && section))
		{
			if ((tmp = g_strrstr(line, "=")))
			{
				*tmp = '\0';
				tmp++;
				/* Allow extended chars */
				decomp = g_strcompress(tmp);
				cfg_create_string(section, line, 
						decomp);
				g_free(decomp);
			}
		}
		i++;
		g_free(line);
	}
	g_io_channel_unref(iochannel);
	cfg->filename = g_strdup(filename);
	EXIT();
	return cfg;
}


/*!
 \brief writes the configfile to disk from the internal structure
 \param cfg is the pointer to the ConfigFile structure
 \param filename is the name of filename to save the data to
 \returns TRUE on success, FALSE on failure
 */
gboolean cfg_write_file(ConfigFile * cfg, const gchar * filename)
{
	FILE *file;
	GList *section_list, *line_list;
	ConfigSection *section;
	ConfigLine *line;

	ENTER();
	if (!(file = fopen(filename, "wb")))
	{
		EXIT();
		return FALSE;
	}

	section_list = cfg->sections;
	while (section_list)
	{
		section = (ConfigSection *) section_list->data;
		if (section->lines)
		{
			g_fprintf(file, "[%s]\n", section->name);
			line_list = section->lines;
			while (line_list)
			{
				line = (ConfigLine *) line_list->data;
				fprintf(file, "%s=%s\n", line->key, line->value);
				line_list = g_list_next(line_list);
			}
			g_fprintf(file, "\n");
		}
		section_list = g_list_next(section_list);
	}
	fclose(file);
	EXIT();
	return TRUE;
}


/*!
 \brief Reads a string value from the cfg 
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want the value for
 \param value is the pointer to where to store the value
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_string(ConfigFile * cfg, const gchar * section, const gchar * key, gchar ** value)
{
	ConfigSection *sect;
	ConfigLine *line;

	ENTER();
	if (!(sect = cfg_find_section(cfg, section)))
	{
		EXIT();
		return FALSE;
	}
	if (!(line = cfg_find_string(sect, key)))
	{
		EXIT();
		return FALSE;
	}
	*value = g_strcompress(line->value);
	EXIT();
	return TRUE;
}


/*!
 \brief reads an int value from the cfg 
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want the value for
 \param value is the pointer to where to store that value
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_int(ConfigFile * cfg, const gchar * section, const gchar * key, gint * value)
{
	gchar *str;

	ENTER();
	if (!cfg_read_string(cfg, section, key, &str))
	{
		EXIT();
		return FALSE;
	}
	*value = atoi(str);
	g_free(str);

	EXIT();
	return TRUE;
}


/*!
 \brief Reads an boolean value from the cfg 
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want the value for
 \param value is the pointer to where to store that value
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_boolean(ConfigFile * cfg, const gchar * section, const gchar * key, gboolean * value)
{
	gchar *str;

	ENTER();
	if (!cfg_read_string(cfg, section, key, &str))
	{
		EXIT();
		return FALSE;
	}
	if (!g_ascii_strcasecmp(str, "TRUE"))
		*value = TRUE;
	else
		*value = FALSE;
	g_free(str);
	EXIT();
	return TRUE;
}


/*!
 \brief Reads a float value from the cfg 
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want the value for
 \param value is the pointer to where to store that value
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_float(ConfigFile * cfg, const gchar * section, const gchar * key, gfloat * value)
{
	gchar *str;

	ENTER();
	if (!cfg_read_string(cfg, section, key, &str))
	{
		EXIT();
		return FALSE;
	}

	*value = (gfloat) g_ascii_strtod(g_strdelimit(str,",.",'.'), NULL);
	g_free(str);

	EXIT();
	return TRUE;
}


/*!
 \brief Reads a double value from the cfg 
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want the value for
 \param value is the pointer to where to store that value
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_double(ConfigFile * cfg, const gchar * section, const gchar * key, gdouble * value)
{
	gchar *str;

	ENTER();
	if (!cfg_read_string(cfg, section, key, &str))
	{
		EXIT();
		return FALSE;
	}

	*value = g_ascii_strtod(str, NULL);
	g_free(str);

	EXIT();
	return TRUE;
}


/*!
 \brief Writes a string value to the cfg 
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want to write the value for
 \param value is the value we want to store
 */
void cfg_write_string(ConfigFile * cfg, const gchar * section, const gchar * key, const gchar * value)
{
	ConfigSection *sect;
	ConfigLine *line;
	gchar * tmpbuf = NULL;

	ENTER();
	sect = cfg_find_section(cfg, section);
	if (!sect)
		sect = cfg_create_section(cfg, section);
	if ((line = cfg_find_string(sect, key)))
	{
		g_free(line->value);
		tmpbuf = g_strdup(value);
		line->value = g_strescape(g_strstrip(tmpbuf),NULL);
		g_free(tmpbuf);
	}
	else
		cfg_create_string(sect, key, value);
	EXIT();
	return;
}


/*!
 \brief Writes a int value to the cfg 
 \param cfg: source of the data
 \param section: section in the file
 \param key: key we want to write the value for
 \param value: value to write
 */
void cfg_write_int(ConfigFile * cfg, const gchar * section, const gchar * key, gint value)
{
	gchar *strvalue;

	ENTER();
	strvalue = g_strdup_printf("%d", value);
	cfg_write_string(cfg, section, key, strvalue);
	g_free(strvalue);
	EXIT();
	return;
}


/*!
 \brief Writes a boolean value to the cfg 
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want to write the value for
 \param value is the value we want to store
 */
void cfg_write_boolean(ConfigFile * cfg, const gchar * section, const gchar * key, gboolean value)
{
	ENTER();
	if (value)
		cfg_write_string(cfg, section, key, "TRUE");
	else
		cfg_write_string(cfg, section, key, "FALSE");
	EXIT();
	return;
}


/*!
 \brief Writes a float value to the cfg 
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want to write the value for
 \param value is the value we want to store
 */
void cfg_write_float(ConfigFile * cfg, const gchar * section, const gchar * key, gfloat value)
{
	gchar *strvalue;

	ENTER();
	strvalue = g_strdup_printf("%g", value);
	cfg_write_string(cfg, section, key, strvalue);
	g_free(strvalue);
	EXIT();
	return;
}


/*!
 \brief Writes a double value to the cfg 
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want to write the value for
 \param value is the value we want to store
 */
void cfg_write_double(ConfigFile * cfg, const gchar * section, const gchar * key, gdouble value)
{
	gchar *strvalue;

	ENTER();
	strvalue = g_strdup_printf("%g", value);
	cfg_write_string(cfg, section, key, strvalue);
	g_free(strvalue);
	EXIT();
	return;
}


/*!
 \brief Removes a key from the cfgfile
 \param cfg is the pointer to the ConfigFile structure
 \param section is the section in the file
 \param key is the key we want to remove
 */
void cfg_remove_key(ConfigFile * cfg, const gchar * section, const gchar * key)
{
	ConfigSection *sect;
	ConfigLine *line;

	ENTER();
	if ((sect = cfg_find_section(cfg, section)))
	{
		if ((line = cfg_find_string(sect, key)))
		{
			g_free(line->key);
			g_free(line->value);
			g_free(line);
			sect->lines = g_list_remove(sect->lines, line);
		}
	}
	EXIT();
	return;
}


/*!
 \brief frees the memory/resources for a cfg
 \param cfg is the pointer to the ConfigFile structure
 */
void cfg_free(ConfigFile * cfg)
{
	ConfigSection *section;
	ConfigLine *line;
	GList *section_list, *line_list;

	ENTER();
	section_list = cfg->sections;
	while (section_list)
	{
		section = (ConfigSection *) section_list->data;
		g_free(section->name);

		line_list = section->lines;
		while (line_list)
		{
			line = (ConfigLine *) line_list->data;
			g_free(line->key);
			g_free(line->value);
			g_free(line);
			line_list = g_list_next(line_list);
		}
		g_list_free(section->lines);
		g_free(section);

		section_list = g_list_next(section_list);
	}
	g_list_free(cfg->sections);
	g_free(cfg->filename);
	g_free(cfg);
	EXIT();
	return;
}


/*!
 \brief Creates a section in the cfgfile
 \param cfg is the pointer to the ConfigFile structure
 \param name is the name of section to create
 \returns a pointer to the ConfigSection structure
 */
static ConfigSection *cfg_create_section(ConfigFile * cfg, const gchar * name)
{
	ConfigSection *section;

	ENTER();
	section = (ConfigSection *)g_malloc0(sizeof (ConfigSection));
	section->name = g_strdup(name);
	cfg->sections = g_list_prepend(cfg->sections, section);

	EXIT();
	return section;
}


/*!
 \brief Creates a string in a section
 \param section is the pointer to the ConfigSection structure
 \param key is the key to create
 \param value is the value to set the key to
 \returns a pointer to the ConfigLine structure
 */
static ConfigLine *cfg_create_string(ConfigSection * section, const gchar * key, const gchar * value)
{
	ConfigLine *line;
	gchar * tmpbuf = NULL;

	ENTER();
	line = (ConfigLine *)g_malloc0(sizeof (ConfigLine));
	tmpbuf = g_strdup(key);
	line->key = g_strdup(g_strstrip(tmpbuf));
	g_free(tmpbuf);
	tmpbuf = g_strdup(value);
	line->value = g_strescape(g_strstrip(tmpbuf),NULL);
	g_free(tmpbuf);
	section->lines = g_list_prepend(section->lines, line);

	EXIT();
	return line;
}


/*!
 \brief Locates the section requested
 \param cfg is the pointer to the ConfigFile structure
 \param name is the section naem to search for
 \returns a pointer to the  ConfigSection structure searched for
 */
ConfigSection *cfg_find_section(ConfigFile * cfg, const gchar * name)
{
	ConfigSection *section;
	GList *list;

	ENTER();
	list = cfg->sections;
	while (list)
	{
		section = (ConfigSection *) list->data;
		if (!g_ascii_strcasecmp(section->name, name))
		{
			EXIT();
			return section;
		}
		list = g_list_next(list);
	}
	EXIT();
	return NULL;
}


/*!
 \brief Returns the ConfigLine *
 \param section is the section to search in
 \param key is the key to search for
 \returns the pointer to the ConfigLine structure of the line requested
 */
static ConfigLine *cfg_find_string(ConfigSection * section, const gchar * key)
{
	ConfigLine *line;
	GList *list;
	ENTER();
	list = section->lines;
	while (list)
	{
		line = (ConfigLine *) list->data;
		if (!g_ascii_strcasecmp(line->key, key))
		{
			EXIT();
			return line;
		}
		list = g_list_next(list);
	}
	EXIT();
	return NULL;
}
