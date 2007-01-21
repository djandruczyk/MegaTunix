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
#include <defines.h>
#include <debugging.h>
#include <enums.h>


/*!
 \brief check_dependancies() extracts the dependancy information from the 
 object and checks each one in turn until one evauates to false, in that
 case it returns FALSE, otherwise if all deps succeed it'll return TRUE
 \param object (GObject *) object containing dependacy information
 \returns TRUE if dependancy evaluates to TRUE, FALSE on any dep in the chain 
 evaluating to FALSE.
 */
gboolean check_dependancies(GObject *object )
{
	gint i = 0;
	gint page = 0;
	gint offset = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	gint bitval = 0;
	gchar ** deps = NULL;
	gchar * tmpbuf = NULL;
	gint type = 0;
	gint num_deps = 0;
	extern gint **ms_data;

	num_deps = (gint)g_object_get_data(object,"num_deps");
	deps = g_object_get_data(object,"deps");
//	printf("number of deps %i, %i\n",num_deps,g_strv_length(deps));
	for (i=0;i<num_deps;i++)
	{
//		printf("dep name %s\n",deps[i]);
		tmpbuf = g_strdup_printf("%s_type",deps[i]);
		type = (gint)g_object_get_data(object,tmpbuf);
		g_free(tmpbuf);
		if (type == VE_EMB_BIT)
		{
//			printf("VE_EMB_BIT\n");
			tmpbuf = g_strdup_printf("%s_page",deps[i]);
			page = (gint)g_object_get_data(object,tmpbuf);
//			printf("page %i\n",page);
			g_free(tmpbuf);

			tmpbuf = g_strdup_printf("%s_offset",deps[i]);
			offset = (gint)g_object_get_data(object,tmpbuf);
//			printf("offset %i\n",offset);
			g_free(tmpbuf);

			tmpbuf = g_strdup_printf("%s_bitshift",deps[i]);
			bitshift = (gint)g_object_get_data(object,tmpbuf);
//			printf("bitshift %i\n",bitshift);
			g_free(tmpbuf);

			tmpbuf = g_strdup_printf("%s_bitmask",deps[i]);
			bitmask = (gint)g_object_get_data(object,tmpbuf);
//			printf("bitmask %i\n",bitmask);
			g_free(tmpbuf);

			tmpbuf = g_strdup_printf("%s_bitval",deps[i]);
			bitval = (gint)g_object_get_data(object,tmpbuf);
//			printf("bitval %i\n",bitval);
			g_free(tmpbuf);

			if (!(((ms_data[page][offset]) & bitmask) >> bitshift) == bitval)	
			{
//				printf("dep_proc returning FALSE\n");
				return FALSE;
			}
		}
/*		else if (type == VE_VAR)
		{
			tmpbuf = g_strdup_printf("%s_page",deps[i]);
			page = (gint)g_object_get_data(object,g_strdup_printf("%s_page",deps[i]));
			g_free(tmpbuf);

			tmpbuf = g_strdup_printf("%s_offset",deps[i]);
			offset = (gint)g_object_get_data(object,g_strdup_printf("%s_offset",deps[i]));
			g_free(tmpbuf);
		}
*/
	}
//	printf("dep_proc returning TRUE\n");
	return TRUE;
}
