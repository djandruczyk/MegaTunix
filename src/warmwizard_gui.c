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

#include <config.h>
#include <defines.h>
#include <enums.h>
#include <gui_handlers.h>
#include <structures.h>
#include <warmwizard_gui.h>

extern GdkColor red;

void warmwizard_update_status(gfloat temp)
{
	extern gint temp_units;
	extern GdkColor red;
	extern GdkColor black;
	gboolean skipnext = FALSE;
	gint i = 0;
	gchar * name;
	gfloat F_temps[10] = 
	{-40.0,-20.0,0,20.0,40.0,60.0,80.0,100.0,130.0,160.0};
	gfloat C_temps[10] = 
	{-40,-28.8,-17.7,-6.6,4.4,15.5,26.6,37.7,54.4,71.1};
	gfloat *range;
	extern GHashTable *dynamic_widgets;

	if (temp_units == FAHRENHEIT)	
		range = F_temps;
	else
		range = C_temps;

	for (i=0;i<10;i++)
	{
		if (skipnext == FALSE)
		{
			name = g_strdup_printf("ww_warmup_label_%i",i+1);
			gtk_widget_modify_fg(g_hash_table_lookup(dynamic_widgets,name),GTK_STATE_NORMAL,&black);
			g_free(name);
					
			/*			gtk_widget_modify_text(spinners.warmwizard[i],
						GTK_STATE_NORMAL,&black);
						gtk_widget_modify_fg(spinners.warmwizard[i],
						GTK_STATE_NORMAL,&black);
			 */
		}
		else
			skipnext = FALSE;
		if ((temp > range[i]) && (temp < range[i+1]))
		{
			skipnext = TRUE;
			name = g_strdup_printf("ww_warmup_label_%i",i+1);
			gtk_widget_modify_fg(g_hash_table_lookup(dynamic_widgets,name),GTK_STATE_NORMAL,&red);

			g_free(name);
			name = g_strdup_printf("ww_warmup_label_%i",i+2);
			gtk_widget_modify_fg(g_hash_table_lookup(dynamic_widgets,name),GTK_STATE_NORMAL,&red);
					
			g_free(name);
			/*			gtk_widget_modify_text(spinners.warmwizard[i],
						GTK_STATE_NORMAL,&red);
						gtk_widget_modify_fg(spinners.warmwizard[i],
						GTK_STATE_NORMAL,&red);
						gtk_widget_modify_text(spinners.warmwizard[i+1],
						GTK_STATE_NORMAL,&red);
						gtk_widget_modify_fg(spinners.warmwizard[i+1],
						GTK_STATE_NORMAL,&red);
			 */
		}
	}

}
