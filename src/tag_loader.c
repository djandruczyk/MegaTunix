/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <configfile.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <math.h>
#include <keyparser.h>
#include <stdlib.h>
#include <stringmatch.h>
#include <tag_loader.h>



extern gint dbg_lvl;
extern GObject *global_data;

/*!
 \brief load_tags() loads tags from the datamap file in reference to a 
 textview.  A tag defines a set of attributes that can be applied to
 text. 
 \param object (GObject *) pointer to the object where the attributes are
 to be stored
 \param cfgfile (ConfigFile *) pointer to the configfile to read the 
 necessary data from.
 \param section (gchar *) section name in the config file to search for the
 tags
 */
void load_tags(GObject *object, ConfigFile *cfgfile, gchar * section)
{
	gchar *tmpbuf = NULL;
	gchar *key = NULL;
	gchar ** tagnames = NULL;
	gchar ** attrs = NULL;
	gint num_attrs = 0;
	gint num_tags = 0;
	gint i = 0;
	GtkTextBuffer *textbuffer = NULL;

	cfg_read_string(cfgfile,section,"create_tags",&tmpbuf);
	tagnames = parse_keys(tmpbuf,&num_tags,",");
	g_free(tmpbuf);

	textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(object));

	for (i=0;i<num_tags;i++)
	{
		key = g_strdup_printf("%s",tagnames[i]);
		if (!cfg_read_string(cfgfile,section,key,&tmpbuf))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": load_tag()\n\t Key \"%s\" NOT FOUND in section \"[%s]\", EXITING!!\n",key,section));
			exit (-5);
		}
		else
		{
			attrs = parse_keys(tmpbuf,&num_attrs,",");
			g_free(tmpbuf);
			if (num_attrs%2)
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_tags()\n\t number of attributes is incorrect for widget \"%s\", key \"%s\" \n",section,key));
				return;
			}
			switch (num_attrs)
			{
				case 2:
					/*gtk_text_buffer_create_tag(textbuffer,g_strdup(key),attrs[0],attrs[1],NULL);*/
					gtk_text_buffer_create_tag(textbuffer,key,attrs[0],attrs[1],NULL);
					break;
				case 4:
					/*gtk_text_buffer_create_tag(textbuffer,g_strdup(key),attrs[0],attrs[1],attrs[2],attrs[3],NULL);*/
					gtk_text_buffer_create_tag(textbuffer,key,attrs[0],attrs[1],attrs[2],attrs[3],NULL);
					break;
				case 6:
					/*gtk_text_buffer_create_tag(textbuffer,g_strdup(key),attrs[0],attrs[1],attrs[2],attrs[3],attrs[4],attrs[5],NULL);*/
					gtk_text_buffer_create_tag(textbuffer,key,attrs[0],attrs[1],attrs[2],attrs[3],attrs[4],attrs[5],NULL);
					break;
				default:
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup(__FILE__": load_tags()\n\t numer of attributes is too many, 3 pairs of attribute pairs per tag is the maximum supported\n"));

			}

			g_strfreev(attrs);
		}
		g_free(key);

	}
	g_strfreev(tagnames);
	return;
}


