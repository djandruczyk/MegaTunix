/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * A portion of this code in this file is a near copy from MegaTune 2.25
 * file genTherm.cpp, copyright Eric Fahlgren.
 *
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/plugins/mscommon/therm.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS personality specific handling of temp tables
  \author David Andruczyk
  */

#include <firmware.h>
#include <math.h>
#include <mscommon_comms.h>
#include <mscommon_plugin.h>
#include <stdio.h>
#include <time.h>



extern gconstpointer *global_data;
static gboolean read_import_file(GIOChannel *);

/*!
  \brief flips the temperature label on the table generator tab
  \param widget is the pointer to the widget the user clicked on
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean flip_table_gen_temp_label(GtkWidget *widget, gpointer data)
{
	GtkWidget *temp_label = lookup_widget_f("temp_label");

	ENTER();
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) /* Deg C */
		gtk_label_set_text(GTK_LABEL(temp_label),(const gchar *)OBJ_GET(widget,"temp_label"));
	EXIT();
	return TRUE;
}


/*!
  \brief imports a temp table from a file
  \param widget is a pointer to the widget
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean import_table_from_file(GtkWidget *widget, gpointer data)
{
	gchar * filename = NULL;
	GIOChannel *chan = NULL;
	gboolean res = FALSE;
	/* This should load and sanity check the file, if good, store it
	   so that the send to ecu button can deliver it.
	   */
	ENTER();
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
	if (!filename)
	{
		EXIT();
		return FALSE;
	}
	chan = g_io_channel_new_file(filename, "r", NULL);
	g_free(filename);
	if (!chan)
	{
		EXIT();
		return FALSE;
	}
	res = read_import_file(chan);
	if (!res)
		DATA_SET(global_data,"import_file_table",NULL);
	g_io_channel_shutdown(chan,FALSE,NULL);
        g_io_channel_unref(chan);

	EXIT();
	return TRUE;
}


/*!
  \brief This is hacky and should be redone to do much better sanity checking
  \param chan is a pointer to the IOchannel for the file to read
  \returns TRUE on success, FALSE otherwise
  */
gboolean read_import_file(GIOChannel *chan)
{
	Firmware_Details *firmware = NULL;
	GIOStatus status = G_IO_STATUS_ERROR;
	gchar *tmpstr = NULL;
	gchar *tmpbuf = NULL;
	gsize len = 0;
	gint value = 0;
	gint lines = 0;
	gchar **vector = NULL;
	gchar **fields = NULL;
	gint16 *table;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(chan,FALSE);
	g_return_val_if_fail(firmware,FALSE);

	table = (gint16 *)DATA_GET(global_data,"import_file_table");
	if (table)
		g_free(table);
	table = g_new0(gint16, 1024);
	while (g_io_channel_read_line(chan,&tmpbuf,&len,NULL,NULL) == G_IO_STATUS_NORMAL)
	{
		/* Search for comment symbols */
		if (g_strstr_len(tmpbuf,len,"#"))
		{
			vector = g_strsplit(tmpbuf,"#",1);
			tmpstr = g_strdup(vector[0]);
			g_strfreev(vector);
		}
		else
			tmpstr = g_strdup(tmpbuf);

		value = atoi(tmpstr);
		g_free(tmpbuf);
		g_free(tmpstr);
		if (lines >= 1024)
		{
			warn_user_f("Import table exceeds appropriate number of lines (1024), The CPU/GPU and the rest of this computer refuse to accept this file...\n");
			g_free(table);
			EXIT();
			return FALSE;
		}
		if (firmware->bigendian)
			table[lines] = GINT16_TO_BE((gint16)value);
		else
			table[lines] = GINT16_TO_LE((gint16)value);
		lines++;
	}
	if (lines != 1024)
	{
		warn_user_f("Import table is NOT the appropriate number of lines (1024), The CPU/GPU and the rest of this computer refuse to accept this file...\n");
		g_free(table);
		EXIT();
		return FALSE;
	}
	else
	{
		DATA_SET(global_data,"import_file_table",(gpointer)table);
		EXIT();
		return TRUE;
	}
	EXIT();
	return FALSE;
}


/*!
  \brief Calculates the values for the new table and downloads to the ECU
  \param widget is a pointer to the widget the user clicked
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean table_gen_process_and_dl(GtkWidget *widget, gpointer data)
{
#define CTS 0
#define MAT 1
	GtkWidget *chooser = NULL;
	gboolean celsius = FALSE;
	gboolean fahrenheit = FALSE;
	gboolean kelvin = FALSE;
	gdouble t[3];
	guint8 tabletype = 0;
	gint16 *table = NULL;
	gchar * filename = NULL;
	time_t tim;
	FILE *f = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	/* OK Button pressed,  we need to validate all input and calculate
	 * and build the tables and send to the ECU
	 */

	tabletype = gtk_combo_box_get_active(GTK_COMBO_BOX(lookup_widget_f("thermister_sensor_combo")));

	/* If using the parameters... */
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget_f("use_params_rbutton"))))
	{
		gint bins = 0;
		gdouble temp1,temp2,temp3;
		gdouble c11,c12,c13;
		gdouble c21,c22,c23;
		gdouble c31,c32,c33;
		gdouble bias;
		table = g_new0(gint16, 1024);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget_f("thermister_celsius_rbutton"))))
			celsius = TRUE;
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget_f("thermister_kelvin_rbutton"))))
			kelvin = TRUE;
		else
			fahrenheit = TRUE;

		bias = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("bias_entry"))),NULL);
		t[0] = temp1 = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("temp1_entry"))),NULL);
		t[1] = temp2 = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("temp2_entry"))),NULL);
		t[2] = temp3 = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("temp3_entry"))),NULL);
		gdouble res1 = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("resistance1_entry"))),NULL);
		gdouble res2 = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("resistance2_entry"))),NULL);
		gdouble res3 = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("resistance3_entry"))),NULL);
		/* If Not kelvin, convert to kelvin */
		for (gint i = 0; i < 3; i++) {
			if (fahrenheit) 
				t[i] = f_to_k_f(t[i]);
			else if (celsius)
				t[i] = c_to_k_f(t[i]);
		}

		c11 = log(res1); c12 = pow(c11, 3.0); c13 = 1.0 / t[0];
		c21 = log(res2); c22 = pow(c21, 3.0); c23 = 1.0 / t[1];
		c31 = log(res3); c32 = pow(c31, 3.0); c33 = 1.0 / t[2];

		gdouble C = ((c23-c13) - (c33-c13)*(c21-c11)/(c31-c11)) / ((c22-c12) - (c32-c12)*(c21-c11)/(c31-c11));
		gdouble B = (c33-c13 - C*(c32-c12)) / (c31-c11);
		gdouble A = c13 - B*c11 - C*c12;

#define k2f(t)	((t*9.0/5.0) - 459.67)
#define Tk(R)  (1.0/(A+B*log(R)+C*pow(log(R), 3)))
#define Tf(R)	(k2f(Tk(R)))
#define Tu(f,celsius) (celsius ? ((f-32.0)*5.0/9.0) : f)

		if (firmware->capabilities & PIS)
		{
			bins = 256;
			bias = 348;
		}
		else
			bins = 1024;
		time(&tim);

		filename = g_build_filename(HOME(), "thermal.log",NULL);
		f = fopen(filename, "w");
		g_free(filename);

		fprintf(f, "//------------------------------------------------------------------------------\n");
		fprintf(f, "//--  Generated by MegaTunix %s", ctime(&tim));
		fprintf(f, "//--  This file merely records what was sent to your MS-II, and may be        --\n");
		fprintf(f, "//--  deleted at any time.                                                    --\n");
		fprintf(f, "//--                                                                          --\n");
		fprintf(f, "//--  Type = %d (%s)                                                          --\n", tabletype, tabletype==CTS?"CTS":"MAT");
		fprintf(f, "//--  Bias = %7.1f                                                          --\n", bias);
		fprintf(f, "//--                                                                          --\n");
		fprintf(f, "//--  Resistance      tInput             tComputed                            --\n");
		fprintf(f, "//--  ------------    -----------------  ---------                            --\n");
		fprintf(f, "//--  %8.1f ohm  %7.1fK (% 7.1f%c) %10.1f                            --\n", res1, t[0], temp1, celsius?'C':'F', Tu(Tf(res1), celsius));
		fprintf(f, "//--  %8.1f ohm  %7.1fK (% 7.1f%c) %10.1f                            --\n", res2, t[1], temp2, celsius?'C':'F', Tu(Tf(res2), celsius));
		fprintf(f, "//--  %8.1f ohm  %7.1fK (% 7.1f%c) %10.1f                            --\n", res3, t[2], temp3, celsius?'C':'F', Tu(Tf(res3), celsius));
		fprintf(f, "//------------------------------------------------------------------------------\n");
		fprintf(f, "\n");
		fprintf(f, "#ifndef GCC_BUILD\n");
		fprintf(f, "#pragma ROM_VAR %s_ROM\n", tabletype==CTS?"CLT":"MAT");
		fprintf(f, "#endif\n");
		fprintf(f, "const int %sfactor_table[%i] EEPROM_ATTR  = {\n", tabletype==CTS?"clt":"mat",bins);
		fprintf(f, "          //  ADC    Volts    Temp       Ohms\n");

		for (gint adcCount = 0; adcCount < bins; adcCount++) 
		{
			gdouble res = bias / ((bins-1)/(double)(adcCount==0?0.01:adcCount) - 1.0);
			gdouble temp = Tf(res);
			if (!(firmware->capabilities & PIS))
			{
				if      (temp <  -40.0) temp = tabletype == CTS ? 180.0 : 70.0;
				else if (temp >  350.0) temp = tabletype == CTS ? 180.0 : 70.0;
			}
			gushort tt = (gushort)(temp*10);
			fprintf(f, "   %5d%c // %4d %7.2f  %7.1f  %9.1f\n", tt, adcCount<bins-1?',':' ', adcCount, 5.0*adcCount/(bins-1), Tu(tt/10.0, celsius), res);
			if (firmware->bigendian)
				table[adcCount] = GINT16_TO_BE((gint16)tt);
			else
				table[adcCount] = GINT16_TO_LE((gint16)tt);
			/*table[adcCount] = tt;*/
		}
		fprintf(f, "};\n");
		fprintf(f, "#ifndef GCC_BUILD\n");
		fprintf(f, "#pragma ROM_VAR DEFAULT\n");
		fprintf(f, "#endif\n");
		fprintf(f, "//------------------------------------------------------------------------------\n");
		fclose(f);
	}
	else
	{
		printf("Using data from file\n");
		table = (gint16 *)DATA_GET(global_data,"import_file_table");
		if (!table)
		{
			EXIT();
			return FALSE;
		}
	}

	if (tabletype == CTS)
		ms_table_write(firmware->clt_table_page,
				firmware->page_params[firmware->clt_table_page]->length,
				(guint8 *)table);
	else if (tabletype == MAT)
		ms_table_write(firmware->mat_table_page,
				firmware->page_params[firmware->mat_table_page]->length,
				(guint8 *)table);
	else
		printf(_("Serious ERROR!, tabletype is not CTS or MAT\n"));

	g_free(table);
	DATA_SET(global_data,"import_file_table",NULL);
	chooser = lookup_widget_f("import_filechooser_button");
	if (chooser)
		gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(chooser));

	EXIT();
	return TRUE;
}
