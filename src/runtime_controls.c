/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */


#include <configfile.h>
#include <default_controls.h>
#include <runtime_controls.h>
#include <stdio.h>
#include <stdlib.h>
#include <structures.h>
#include <tabloader.h>

GHashTable *rt_controls = NULL;
extern GtkWidget *rt_table[];

void create_default_controls()
{
	/* Creates the info in ~/.MegaTunix/config for all possible 
	 * controls in the Runtime_gui (ONLY). 
	 */

	ConfigFile *cfgfile = NULL;
	gchar *filename = NULL;
	gchar *heading = NULL;
	gchar *choices = NULL;
	gchar *defaults = NULL;
	gchar *ctrl_info = NULL;
	gint total = 0;
	gint i = 0;
	gboolean setup = FALSE;

	total = sizeof(all_controls) / sizeof(all_controls[0]);
	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/config", NULL);
	heading = g_strdup("Runtime_Controls");
	while (i < total)
	{
		choices = g_strjoin(",",
				all_controls[i].ctrl_name,
				choices,
				NULL);
		if (all_controls[i].enabled)
		{
			defaults = g_strjoin(",",
					all_controls[i].ctrl_name,
					defaults,
					NULL);
		}
		i++;
	}

	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		cfg_read_boolean(cfgfile,heading,"Setup",&setup);
		cfg_write_int(cfgfile,heading,"Total_Controls",total);
		cfg_write_string(cfgfile,heading,"Choices",choices);
		if (setup)
			goto exitpoint;
		cfg_write_string(cfgfile,heading,"Defaults",defaults);
		for (i=0;i<total;i++)
		{
			if (all_controls[i].enabled)
			{
				ctrl_info = g_strdup_printf(
						"%i,%i,%s,%i,%i,%i,%i",
						all_controls[i].tbl,
						all_controls[i].row,
						all_controls[i].friendly_name,
						all_controls[i].limits_index,
						all_controls[i].runtime_offset,
						all_controls[i].size,
						all_controls[i].flags);
				cfg_write_string(cfgfile,
						heading,
						all_controls[i].ctrl_name,
						ctrl_info);
				g_free(ctrl_info);
			}
			cfg_write_boolean(cfgfile,heading,"Setup",TRUE);
		}
exitpoint:
		cfg_write_file(cfgfile,filename);
		cfg_free(cfgfile);
		g_free(cfgfile);

	}
	g_free(choices);
	g_free(defaults);
	g_free(filename);
	g_free(heading);


	return;

}

void load_controls()
{
	ConfigFile *cfgfile = NULL;
	gchar *filename = NULL;
	gchar *heading = NULL;
	gchar *ctrl_list = NULL;
	gchar *ctrl_info = NULL;
	gchar *defaults = NULL;
	gchar **control_names = NULL;
	gint i = 0;

	filename = g_strconcat(g_get_home_dir(), "/.MegaTunix/config", NULL);
	heading = g_strdup("Runtime_Controls");

	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		cfg_read_string(cfgfile,heading,"Defaults",&defaults);
		cfg_read_string(cfgfile,heading,"Control_List",&ctrl_list);
	}
	if ((ctrl_list == NULL) && (defaults))
	{
//		g_printf("Using defaults\n");
		control_names = g_strsplit(defaults, ",", 0);
		i = 0;
		while (control_names[i] != NULL)
		{
			cfg_read_string(cfgfile,
					heading,
					control_names[i],
					&ctrl_info);
			add_control(control_names[i],ctrl_info);
			g_free(ctrl_info);
			i++;
		}
		g_strfreev(control_names);
	}
	else
	{
//		g_printf("using user specified controls\n");
		control_names = g_strsplit(ctrl_list, ",", 0);
		i = 0;
		while (control_names[i] != NULL)
		{
			cfg_read_string(cfgfile,
					heading,
					control_names[i],
					&ctrl_info);
			add_control(control_names[i],ctrl_info);
			g_free(ctrl_info);
			i++;
		}
		g_strfreev(control_names);
	}

	cfg_free(cfgfile);
	g_free(filename);
}

void add_control(gchar *control_name, gchar *parameters)
{
	gchar **parm_array = NULL;
	struct Rt_Control *control = NULL;
	control = g_malloc0(sizeof(struct Rt_Control));
	GtkWidget *label;
	GtkWidget *pbar;
	extern GList *dt_controls;
	extern GList *ign_controls;

	if (!rt_controls)
		rt_controls = g_hash_table_new(NULL,NULL);
	
	parm_array = g_strsplit(parameters, ",", 0);
	control->ctrl_name = g_strdup(control_name);
	control->tbl = atoi(parm_array[0]);
	control->row = atoi(parm_array[1]);
	control->friendly_name = g_strdup(parm_array[2]);
	control->limits_index = atoi(parm_array[3]);
	control->runtime_offset = atoi(parm_array[4]);
	control->size = atoi(parm_array[5]);
	control->flags = atoi(parm_array[6]);

	g_strfreev(parm_array);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),g_strdup(control->friendly_name));
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (rt_table[control->tbl]),label,
			0,1,control->row,(control->row)+1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);
	control->label = label;

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (rt_table[control->tbl]),label,
			1,2,control->row,(control->row)+1,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (GTK_FILL|GTK_SHRINK), 0, 0);
	control->data = label;
	
	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (rt_table[control->tbl]),pbar,
			2,3,control->row,(control->row)+1,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 0, 0);
	control->pbar = pbar;

	control->parent = rt_table[control->tbl];
	gtk_widget_show_all(control->parent);
	if (control->flags & DUALTABLE)
	{
		dt_controls = g_list_append(dt_controls,(gpointer)control->label);
		dt_controls = g_list_append(dt_controls,(gpointer)control->data);
		dt_controls = g_list_append(dt_controls,(gpointer)control->pbar);
	}
	if (control->flags & (S_N_SPARK|S_N_EDIS))
	{
		ign_controls = g_list_append(ign_controls,(gpointer)control->label);
		ign_controls = g_list_append(ign_controls,(gpointer)control->data);
		ign_controls = g_list_append(ign_controls,(gpointer)control->pbar);
	}
	if (control->flags & TEMP_DEP)	/* name has temp unit in it */
	{
		store_list("temperature",g_list_append(
				get_list("temperature"),(gpointer)control->label));
	}

	if (g_hash_table_lookup(rt_controls,g_strdup(control_name))==NULL)
		g_hash_table_insert(rt_controls,g_strdup(control_name),
				(gpointer)control);
	return;
}
