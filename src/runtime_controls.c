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
#include <runtime_controls.h>
#include <structures.h>


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
		cfg_write_int(cfgfile,heading,"Total_Controls",total);
		cfg_write_string(cfgfile,heading,"Choices",choices);
		cfg_write_string(cfgfile,heading,"Defaults",defaults);
		for (i=0;i<total;i++)
		{
			if (all_controls[i].enabled)
			{
				ctrl_info = g_strdup_printf(
						"%i,%i,%i,%s,%i,%i",
						all_controls[i].table,
						all_controls[i].row,
						all_controls[i].col,
						all_controls[i].friendly_name,
						all_controls[i].limits_index,
						all_controls[i].special);
				cfg_write_string(cfgfile,
						heading,
						all_controls[i].ctrl_name,
						ctrl_info);
			g_free(ctrl_info);
			}
	
		}
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
