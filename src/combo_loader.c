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
#include <combo_loader.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <math.h>
#include <keyparser.h>
#include <stdlib.h>
#include <stringmatch.h>



extern gint dbg_lvl;
extern GObject *global_data;

void combo_setup(GObject *object, ConfigFile *cfgfile, gchar * section)
{
	gchar *tmpbuf = NULL;
	gchar ** choices = NULL;
	gint num_choices = 0;
	gint i = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	gint bits = 0;
	gint highbit = 0;
	gint tmpi = 0;
	GtkListStore *store = NULL;
	GtkTreeIter iter;
        GtkCellRenderer *renderer;

	cfg_read_string(cfgfile,section,"choices",&tmpbuf);
	choices = parse_keys(tmpbuf,&num_choices,",");
	g_free(tmpbuf);

	cfg_read_int(cfgfile,section,"bitmask",&bitmask);
	cfg_read_int(cfgfile,section,"bitshift",&bitshift);
	bits = bitmask >> bitshift;
	for (i=0;i<8;i++)
	{
		if (bits & (gint)(pow(2,i)))
			highbit = i+1;
	}
	tmpi = (gint)pow(2,(double)highbit);

	if (tmpi != num_choices)
	{
		printf("BIG PROBLEM, combobox  choices %i and bits %i don't match up\n",num_choices,tmpi);
		return;
	}

	store = gtk_list_store_new(COMBO_COLS,G_TYPE_STRING,G_TYPE_INT,G_TYPE_INT);

	for (i=0;i<num_choices;i++)
	{
		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,CHOICE_COL,g_strdup(choices[i]),BITMASK_COL,bitmask,BITSHIFT_COL,bitshift,-1);

	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(object),GTK_TREE_MODEL(store));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(object),renderer,FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(object),renderer,"markup",CHOICE_COL,NULL);
}
