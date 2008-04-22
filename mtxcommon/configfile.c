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
 *  fit into eXtace a bit better..
 */

#include <configfile.h>
#include <glib/gprintf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static ConfigSection *cfg_create_section(ConfigFile * cfg, gchar * name);
static ConfigLine *cfg_create_string(ConfigSection * section, gchar * key, gchar * value);
static ConfigSection *cfg_find_section(ConfigFile * cfg, gchar * name);
static ConfigLine *cfg_find_string(ConfigSection * section, gchar * key);


/*!
 \brief cfg_new() creates a new ConfigFile * and returns it
 \returns a ConfigFile *
 */
ConfigFile *cfg_new(void)
{
	ConfigFile *cfg;

	cfg = g_malloc0(sizeof (ConfigFile));

	return cfg;
}


/*!
 \brief cfg_open_file() opens a file, reads it's entire contents and populates
 to cfg internal structures
 \param filename (gchar *) name of file to open and load
 \returns the populated ConfigFile * 
 */
ConfigFile *cfg_open_file(gchar * filename)
{
	ConfigFile *cfg;

	gchar *tmp = NULL;
	gchar *line = NULL;
	gchar *decomp = NULL;
	gint i = 0;
	GIOChannel * iochannel = NULL;
	GError *error = NULL;
	ConfigSection *section = NULL;

	iochannel = g_io_channel_new_file(filename,"r",&error);
	if(error)
		return NULL;

	cfg = g_malloc0(sizeof (ConfigFile));

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
	return cfg;
}


/*!
 \brief cfg_write_file() writes the configfile to disk from the internal
 structure
 \param cfg (ConfigFile *) source ofr data to export to disk
 \param filename (gchar *) name of filename to dave the data to
 \returns TRUE on successm FALSE on failure
 */
gboolean cfg_write_file(ConfigFile * cfg, gchar * filename)
{
	FILE *file;
	GList *section_list, *line_list;
	ConfigSection *section;
	ConfigLine *line;

	if (!(file = fopen(filename, "wb")))
		return FALSE;

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
	return TRUE;
}


/*!
 \brief cfg_read_string() reads a string value from the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want the value for
 \param value (gchar *) value to return
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_string(ConfigFile * cfg, gchar * section, gchar * key, gchar ** value)
{
	ConfigSection *sect;
	ConfigLine *line;

	if (!(sect = cfg_find_section(cfg, section)))
		return FALSE;
	if (!(line = cfg_find_string(sect, key)))
		return FALSE;
	*value = g_strcompress(line->value);
	return TRUE;
}


/*!
 \brief cfg_read_int() reads an int value from the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want the value for
 \param value (gint *) value to return
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_int(ConfigFile * cfg, gchar * section, gchar * key, gint * value)
{
	gchar *str;

	if (!cfg_read_string(cfg, section, key, &str))
		return FALSE;
	*value = atoi(str);
	g_free(str);

	return TRUE;
}


/*!
 \brief cfg_read_boolean() reads an boolean value from the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want the value for
 \param value (gboolean *) value to return
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_boolean(ConfigFile * cfg, gchar * section, gchar * key, gboolean * value)
{
	gchar *str;

	if (!cfg_read_string(cfg, section, key, &str))
		return FALSE;
	if (!g_ascii_strcasecmp(str, "TRUE"))
		*value = TRUE;
	else
		*value = FALSE;
	g_free(str);
	return TRUE;
}


/*!
 \brief cfg_read_float() reads a float value from the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want the value for
 \param value (gfloat *) value to return
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_float(ConfigFile * cfg, gchar * section, gchar * key, gfloat * value)
{
	gchar *str;

	if (!cfg_read_string(cfg, section, key, &str))
		return FALSE;

	*value = (gfloat) g_ascii_strtod(str, NULL);
	g_free(str);

	return TRUE;
}


/*!
 \brief cfg_read_double() reads a double value from the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want the value for
 \param value (gdouble *) value to return
 \returns TRUE on success, FALSE on no key found
 */
gboolean cfg_read_double(ConfigFile * cfg, gchar * section, gchar * key, gdouble * value)
{
	gchar *str;

	if (!cfg_read_string(cfg, section, key, &str))
		return FALSE;

	*value = g_ascii_strtod(str, NULL);
	g_free(str);

	return TRUE;
}


/*!
 \brief cfg_write_string() writes a string value to the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want to write the value for
 \param value (gchar *) value to write
 */
void cfg_write_string(ConfigFile * cfg, gchar * section, gchar * key, gchar * value)
{
	ConfigSection *sect;
	ConfigLine *line;
	gchar * tmpbuf = NULL;

	sect = cfg_find_section(cfg, section);
	if (!sect)
		sect = cfg_create_section(cfg, section);
	if ((line = cfg_find_string(sect, key)))
	{
		g_free(line->value);
		/*tmpbuf = g_strchug(g_strchomp(g_strdup(value)));*/
		tmpbuf = g_strstrip(g_strdup(value));
		line->value = g_strescape(tmpbuf,NULL);
		g_free(tmpbuf);
	}
	else
		cfg_create_string(sect, key, value);
}


/*!
 \brief cfg_write_int() writes a int value to the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want to write the value for
 \param value (gint) value to write
 */
void cfg_write_int(ConfigFile * cfg, gchar * section, gchar * key, gint value)
{
	gchar *strvalue;

	strvalue = g_strdup_printf("%d", value);
	cfg_write_string(cfg, section, key, strvalue);
	g_free(strvalue);
}


/*!
 \brief cfg_write_boolean() writes a boolean value to the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want to write the value for
 \param value (gboolean) value to write
 */
void cfg_write_boolean(ConfigFile * cfg, gchar * section, gchar * key, gboolean value)
{
	if (value)
		cfg_write_string(cfg, section, key, "TRUE");
	else
		cfg_write_string(cfg, section, key, "FALSE");
}


/*!
 \brief cfg_write_float() writes a float value to the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want to write the value for
 \param value (gfloat) value to write
 */
void cfg_write_float(ConfigFile * cfg, gchar * section, gchar * key, gfloat value)
{
	gchar *strvalue;

	strvalue = g_strdup_printf("%g", value);
	cfg_write_string(cfg, section, key, strvalue);
	g_free(strvalue);
}


/*!
 \brief cfg_write_double() writes a double value to the cfg 
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want to write the value for
 \param value (gdouble) value to write
 */
void cfg_write_double(ConfigFile * cfg, gchar * section, gchar * key, gdouble value)
{
	gchar *strvalue;

	strvalue = g_strdup_printf("%g", value);
	cfg_write_string(cfg, section, key, strvalue);
	g_free(strvalue);
}


/*!
 \brief cfg_remove_key() removes a key from the cfgfile
 \param cfg (ConfigFile*) source of the data
 \param section (gchar *) section in the file
 \param key (gchar *) key we want to remove
 */
void cfg_remove_key(ConfigFile * cfg, gchar * section, gchar * key)
{
	ConfigSection *sect;
	ConfigLine *line;

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
}


/*!
 \brief cfg_free() freesthe memory for a cfg
 \param cfg (ConfigFile*) source of the data
 */
void cfg_free(ConfigFile * cfg)
{
	ConfigSection *section;
	ConfigLine *line;
	GList *section_list, *line_list;

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
}


/*!
 \brief cfg_create_section() creates a section in the cfgfile
 \param cfg (ConfigFile *) cfg to create the section in
 \param name (gchar *) name of section to create
 \returns the ConfigSection *
 */
static ConfigSection *cfg_create_section(ConfigFile * cfg, gchar * name)
{
	ConfigSection *section;

	section = g_malloc0(sizeof (ConfigSection));
	section->name = g_strdup(name);
	cfg->sections = g_list_prepend(cfg->sections, section);

	return section;
}


/*!
 \brief cfg_create_string() creates a string in a section
 \param section (ConfigSection *) section pointer
 \param key (gchar *) key to create
 \param value (gchar *) value to set the key to
 \returns a ConfigLine *
 */
static ConfigLine *cfg_create_string(ConfigSection * section, gchar * key, gchar * value)
{
	ConfigLine *line;
	gchar * tmpbuf = NULL;

	line = g_malloc0(sizeof (ConfigLine));
	line->key = g_strstrip(g_strdup(key));
	tmpbuf = g_strstrip(g_strdup(value));
	line->value = g_strescape(tmpbuf,NULL);
	g_free(tmpbuf);
	section->lines = g_list_prepend(section->lines, line);

	return line;
}


/*!
 \brief cfg_find_section() locates the section requested
 \param cfg (onfigFile *) cfg to search in
 \param name (gchar *) section naem to search for
 \returns the ConfigSection * searched for
 */
static ConfigSection *cfg_find_section(ConfigFile * cfg, gchar * name)
{
	ConfigSection *section;
	GList *list;

	list = cfg->sections;
	while (list)
	{
		section = (ConfigSection *) list->data;
		if (!g_ascii_strcasecmp(section->name, name))
			return section;
		list = g_list_next(list);
	}
	return NULL;
}


/*!
 \brief cfg_find_string() returns the ConfigLine *
 \param section (ConfigSection *) section to search in
 \param key (gchar *) key to search for
 \returns ConfigLine * to the line requested
 */
static ConfigLine *cfg_find_string(ConfigSection * section, gchar * key)
{
	ConfigLine *line;
	GList *list;

	list = section->lines;
	while (list)
	{
		line = (ConfigLine *) list->data;
		if (!g_ascii_strcasecmp(line->key, key))
			return line;
		list = g_list_next(list);
	}
	return NULL;
}
