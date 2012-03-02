/* Compile with
 * gcc `pkg-config --cflags --libs glib-2.0` -L ../mtxcommon/ -l mtxcommon -o rtv_2_xml rtv_2_xml.c
 * */
#include <glib.h>
#include <stdio.h>
#include "../include/configfile.h"

struct {
	gchar *suffix;
}potentials[] = { 
	{"_page"},{"_offset"},{"_bitmask"},{"_size"}
};

void export_symbols(ConfigFile *,gchar *,gchar **);

int main (int argc, char *argv[])
{
	ConfigFile *cfg = NULL;
	gchar * tmpbuf = NULL;
	gchar * tmpbuf2 = NULL;
	gchar *filename = NULL;
	gchar *section = NULL;
	gchar **vector = NULL;
	gint j = 0;

	gint i = 0;

	if (argc != 2)
	{
		printf("invalid args need file to convert!\n");
		return (-1);
	}
	filename = argv[1];

	cfg = cfg_open_file(filename);
	if (!cfg)
		return(-1);
	else
		printf("<?xml version=\"1.0\"?>\n<rtv_map>\n\t<api>\n\t\t<major>1</major>\n\t\t<minor>7</minor>\n\t</api>\n");

	if (cfg_find_section(cfg,"realtime_map"))
	{
		printf("\t<realtime_map>\n");
		if (cfg_read_string(cfg,"realtime_map","persona",&tmpbuf))
		{
			printf("\t\t<persona>%s</persona>\n",tmpbuf);
			g_free(tmpbuf);
		}
		if (cfg_read_string(cfg,"realtime_map","applicable_signatures",&tmpbuf))
		{
			printf("\t\t<applicable_signatures>%s</applicable_signatures>\n",tmpbuf);
			g_free(tmpbuf);
		}
		if (cfg_read_string(cfg,"realtime_map","raw_list",&tmpbuf))
		{
			printf("\t\t<raw_list>%s</raw_list>\n",tmpbuf);
			g_free(tmpbuf);
		}
		printf("\t</realtime_map>\n");
	}
	for (i=0;i<200;i++)
	{
		section = g_strdup_printf("derived_%i",i);
		if (cfg_find_section(cfg,section))
		{
			printf("\t<derived>\n");
			if (cfg_read_string(cfg,section,"dlog_gui_name",&tmpbuf))
			{
				printf("\t\t<dlog_gui_name>%s</dlog_gui_name>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"dlog_field_name",&tmpbuf))
			{
				printf("\t\t<dlog_field_name>%s</dlog_field_name>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"internal_names",&tmpbuf))
			{
				printf("\t\t<internal_names>%s</internal_names>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"offset",&tmpbuf))
			{
				printf("\t\t<offset>%s</offset>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"size",&tmpbuf))
			{
				printf("\t\t<size>%s</size>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"real_lower",&tmpbuf))
			{
				printf("\t\t<real_lower>%s</real_lower>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"real_upper",&tmpbuf))
			{
				printf("\t\t<real_upper>%s</real_upper>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"fromecu_mult",&tmpbuf))
			{
				printf("\t\t<fromecu_mult>%s</fromecu_mult>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"fromecu_add",&tmpbuf))
			{
				printf("\t\t<fromecu_add>%s</fromecu_add>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"temp_dep",&tmpbuf))
			{
				printf("\t\t<temp_dep>%s</temp_dep>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"precision",&tmpbuf))
			{
				printf("\t\t<precision>%s</precision>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"tooltip",&tmpbuf))
			{
				printf("\t\t<tooltip>%s</tooltip>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"log_by_default",&tmpbuf))
			{
				printf("\t\t<log_by_default>%s</log_by_default>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"special",&tmpbuf))
			{
				printf("\t\t<special>%s</special>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"fromecu_complex",&tmpbuf))
			{
				printf("\t\t<fromecu_complex>%s</fromecu_complex>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"fromecu_conv_expr",&tmpbuf))
			{
				printf("\t\t<fromecu_conv_expr>%s</fromecu_conv_expr>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"expr_types",&tmpbuf))
			{
				printf("\t\t<expr_types>%s</expr_types>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"expr_symbols",&tmpbuf))
			{
				printf("\t\t<expr_symbols>%s</expr_symbols>\n",tmpbuf);
				vector = g_strsplit(tmpbuf,",",-1);
				g_free(tmpbuf);
				export_symbols(cfg,section,vector);
			}
			if (cfg_read_string(cfg,section,"depend_on",&tmpbuf))
			{
				printf("\t\t<depend_on>%s</depend_on>\n",tmpbuf);
				if (cfg_read_string(cfg,section,tmpbuf,&tmpbuf2))
				{
					printf("\t\t<%s>%s</%s>\n",tmpbuf,tmpbuf2,tmpbuf);
					g_free(tmpbuf2);
				}
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"lookuptable",&tmpbuf))
			{
				printf("\t\t<lookuptable>%s</lookuptable>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"alt_lookuptable",&tmpbuf))
			{
				printf("\t\t<alt_lookuptable>%s</alt_lookuptable>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"upper_limits",&tmpbuf))
			{
				printf("\t\t<upper_limits>%s</upper_limits>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"lower_limits",&tmpbuf))
			{
				printf("\t\t<lower_limits>%s</lower_limits>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"fromecu_mults",&tmpbuf))
			{
				printf("\t\t<fromecu_mults>%s</fromecu_mults>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"fromecu_adds",&tmpbuf))
			{
				printf("\t\t<fromecu_adds>%s</fromecu_adds>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"multi_expr_keys",&tmpbuf))
			{
				printf("\t\t<multi_expr_keys>%s</multi_expr_keys>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"multi_lookuptables",&tmpbuf))
			{
				printf("\t\t<multi_lookuptables>%s</multi_lookuptables>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"source_key",&tmpbuf))
			{
				printf("\t\t<source_key>%s</source_key>\n",tmpbuf);
				g_free(tmpbuf);
			}
			printf("\t</derived>\n");
		}
	}
	printf("</rtv_map>\n");
	cfg_free(cfg);
}

void export_symbols(ConfigFile *cfg,gchar *section,gchar **vector)
{
	gchar *name = NULL;
	gchar *tmpbuf = NULL;

	gint i = 0;
	gint j = 0;

	for (i=0;i<g_strv_length(vector);i++)
	{
		for (j=0;j<(sizeof (potentials)/sizeof(potentials[0])); j++)
		{
			name = g_strdup_printf("%s%s",vector[i],potentials[j].suffix);
			if (cfg_read_string(cfg,section,name,&tmpbuf))
			{
				printf("\t\t<%s>%s</%s>\n",name,tmpbuf,name);
				g_free(tmpbuf);
			}
			g_free(name);
		}

	}

}
