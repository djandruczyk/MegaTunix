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
#include <stdlib.h>
#include <stringmatch.h>
#include <tabloader.h>
#include <tag_loader.h>

void load_tags(GObject *object, ConfigFile *cfgfile, gchar * section)
{
	gchar *tmpbuf = NULL;
	gchar *key = NULL;
	gchar ** tagnames = NULL;
	gchar ** attrs = NULL;
	gint num_attrs = 0;
	gint num_tags = 0;
	gint num_pairs = 0;
	gint i = 0;
	GtkTextBuffer *textbuffer = NULL;
	
	cfg_read_string(cfgfile,section,"create_tags",&tmpbuf);
	tagnames = parse_keys(tmpbuf,&num_tags);
	g_free(tmpbuf);

	textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(object));
	
	for (i=0;i<num_tags;i++)
	{
		key = g_strdup_printf("%s",tagnames[i]);
		if (!cfg_read_string(cfgfile,section,key,&tmpbuf))
		{
			dbg_func(g_strdup_printf(__FILE__": load_tag()\n\t Key \"%s\" NOT FOUND in section \"[%s]\", EXITING!!\n",key,section),CRITICAL);
			exit (-5);
		}
		else
		{
			attrs = parse_keys(tmpbuf,&num_attrs);
			g_free(tmpbuf);
			if (num_attrs%2)
			{
				dbg_func(g_strdup_printf(__FILE__": load_tags()\n\t numer of attributes is incorrect for widget \"%s\", key \"%s\" \n",section,key),CRITICAL);
				return;
			}
			switch (num_attrs)
			{
				case 2:
					gtk_text_buffer_create_tag(textbuffer,g_strdup(key),attrs[0],attrs[1],NULL);
					break;
				case 4:
					gtk_text_buffer_create_tag(textbuffer,g_strdup(key),attrs[0],attrs[1],attrs[2],attrs[3],NULL);
					break;
				case 6:
					gtk_text_buffer_create_tag(textbuffer,g_strdup(key),attrs[0],attrs[1],attrs[2],attrs[3],attrs[4],attrs[5],NULL);
					break;
				default:
					dbg_func(__FILE__": load_tags()\n\t numer of attributes is too many, 3 pairs of attribute pairs per tag is the maximum supported\n",CRITICAL);

			}
			
			g_strfreev(attrs);
		}
		g_free(key);
			
	}
	return;
}
