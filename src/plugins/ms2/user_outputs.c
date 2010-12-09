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

#include <assert.h>
#include <config.h>
#include <combo_mask.h>
#include <defines.h>
#include <debugging.h>
#include <rtv_map_loader.h>
#include <enums.h>
#include <keyparser.h>
#include <stdlib.h>
#include <user_outputs.h>

extern gconstpointer *global_data;


/*! \brief sets up and populates the MS2-Extra combo for output choice
 */
G_MODULE_EXPORT void ms2_output_combo_setup(GtkWidget *widget)
{
	gint i = 0;
	gchar * lower = NULL;
	gchar * upper = NULL;
	gchar * dl_conv = NULL;
	gchar * ul_conv = NULL;
	gchar * range = NULL;
	gint bitval = 0;
	gint width = 0;
	DataSize size = MTX_U08;
	gint precision = 0;
	gconstpointer * object = NULL;
	gchar * data = NULL;
	gchar * name = NULL;
	gchar *regex = NULL;
	GtkWidget *entry = NULL;
	GtkEntryCompletion *completion = NULL;
	GtkListStore *store = NULL;
	GtkTreeIter iter;

	Rtv_Map *rtv_map = NULL;

	rtv_map = DATA_GET(global_data,"rtv_map");

	if (!rtv_map)
		return;
	/* Create the store for the combo, with severla hidden values
	*/
	store = gtk_list_store_new(UO_COMBO_COLS,G_TYPE_STRING,G_TYPE_UCHAR,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_UCHAR,G_TYPE_UCHAR);
	/* Iterate across valid variables */
	while ((data = rtv_map->raw_list[i])!= NULL)
	{
		i++;
		object = NULL;
		name = NULL;
		object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,data);
		if (!object)
		{
			printf("couldn't find Raw RTV var with int name %s\n",data);
			continue;
		}
		name = (gchar *) DATA_GET(object,"dlog_gui_name");
		if (!name)
			continue;
		size = (DataSize)DATA_GET(object,"size");
		lower = DATA_GET(object,"real_lower");
		upper = DATA_GET(object,"real_upper");
		dl_conv = DATA_GET(object,"dl_conv_expr");
		ul_conv = DATA_GET(object,"ul_conv_expr");
		precision = (GINT) DATA_GET(object,"precision");
		bitval = (GINT) DATA_GET(object,"offset");
		if ((!lower) && (!upper))
			range = g_strdup_printf("Valid Range: undefined");
		else
			range = g_strdup_printf("Valid Range: %s <-> %s",lower,upper);

		regex = g_strconcat(name,NULL);
		if (rtv_map->raw_list[i])
			regex = g_strconcat("|",NULL);
		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,
				UO_CHOICE_COL,name,
				UO_BITVAL_COL,bitval,
				UO_DL_CONV_COL,dl_conv,
				UO_UL_CONV_COL,ul_conv,
				UO_LOWER_COL,lower,
				UO_UPPER_COL,upper,
				UO_RANGE_COL,range,
				UO_SIZE_COL,size,
				UO_PRECISION_COL,precision,
				-1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(widget),GTK_TREE_MODEL(store));
	if (GTK_IS_COMBO_BOX_ENTRY(widget))
	{
		gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(widget),UO_CHOICE_COL);
		entry = mask_entry_new_with_mask(regex);
		/* Nasty hack, but otherwise the entry is an obnoxious size.. */
		if ((width = (GINT)OBJ_GET((GtkWidget *)widget,"max_chars")) > 0)
			gtk_entry_set_width_chars(GTK_ENTRY(entry),width);
		else
			gtk_entry_set_width_chars(GTK_ENTRY(entry),12);

		gtk_widget_set_size_request(GTK_WIDGET(widget),-1,(3*(GINT)DATA_GET(global_data,"font_size")));

		gtk_container_remove (GTK_CONTAINER (widget), GTK_BIN (widget)->child);
		gtk_container_add (GTK_CONTAINER (widget), entry);

		completion = gtk_entry_completion_new();
		gtk_entry_set_completion(GTK_ENTRY(entry),completion);
		gtk_entry_completion_set_model(completion,GTK_TREE_MODEL(store));
		gtk_entry_completion_set_text_column(completion,UO_CHOICE_COL);
		gtk_entry_completion_set_inline_completion(completion,TRUE);
		gtk_entry_completion_set_inline_selection(completion,TRUE);
		gtk_entry_completion_set_popup_single_match(completion,FALSE);
		OBJ_SET(widget,"arrow-size",GINT_TO_POINTER(1));
	}
	g_free(regex);

}
