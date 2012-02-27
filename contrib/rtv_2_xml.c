/* Compile with
 * gcc `pkg-config --cflags --libs glib-2.0` -L ../mtxcommon/ -l mtxcommon -o rtv_2_xml rtv_2_xml.c
 * */
#include <stdio.h>
#include "../include/configfile.h"

int main (int argc, char *argv[])
{
	ConfigFile *cfg = NULL;
	gchar * tmpbuf = NULL;
	gchar *filename = NULL;
	gchar *section = NULL;
	gint i = 0;

	if (argc != 2)
	{
		printf("invalid args need file to convert!\n");
		return (-1);
	}
	filename = argv[1];

	cfg = cfg_open_file(filename);
	if (!cfg)
		printf("File %s open failure...\n",filename);
	else
		printf("<?xml version=\"1.0\"?>\n<rtv_map>\n\t<api>\n\t\t<major>1</major>\n\t\t<minor>7</minor>\n\t</api>\n");

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
			if (cfg_read_string(cfg,section,"expr_symbols",&tmpbuf))
			{
				printf("\t\t<expr_symbols>%s</expr_symbols>\n",tmpbuf);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfg,section,"expr_types",&tmpbuf))
			{
				printf("\t\t<expr_types>%s</expr_types>\n",tmpbuf);
				g_free(tmpbuf);
			}
			printf("\t</derived>\n");
		}
	}
	printf("</rtv_map>\n");
	cfg_free(cfg);
}
