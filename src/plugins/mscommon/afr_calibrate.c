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
 * file genO2.cpp, copyright Eric Fahlgren.
 *
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/plugins/mscommon/afr_calibrate.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS Specific afr_calibration functionality
  \author David Andruczyk
  */

#ifdef GTK_DISABLE_DEPRECATED
#undef GTK_DISABLE_DEPRECATED
#endif

#include <firmware.h>
#include <math.h>
#include <mscommon_comms.h>
#include <mscommon_plugin.h>
#include <stdio.h>
#include <time.h>


static gint afr_enum;
static const gchar * afr_name;
typedef enum
{
	COL_NAME,
	COL_SYMBOL,
	NUM_COLS
}columns;

/*!
  \brief Enumerations for thetypes of Pre-canned AFR tables
  */
typedef enum
{
	narrowBand,
	aemLinear,
	aemNonLinear,
	twintec,
	diyWB,
	dynojetLinear,
	fjo,
	innovate05,
	innovate12,
	innovateLC1,
	lambdaBoy,
	techEdgeNonLinear,
	techEdgeLinear,
	zeitronix,
	genericWB,
	num_symbols
}afr_table_enums;

/*!
  \brief Pre-cannel array of structures matching the  name to the enumeration for the Gui
  */
static struct 
{
	const gchar *name;
	gint symbol;
}AFR_Tables[] = {
	{"Narrow Band",			narrowBand},
	{"AEM Linear AEM-30-42xx",	aemLinear},
	{"AEM Non-linear AEM-30-230x",	aemNonLinear},
	{"Daytona TwinTec",		twintec},
	{"DIY-WB",			diyWB},
	{"DynoJet Wideband Commander",	dynojetLinear},
	{"FJO WB",			fjo},
	{"Innovate 0.0-5.0 v",		innovate05},
	{"Innovate 1.0-2.0 v",		innovate12},
	{"Innovate LC-1 0.5-1.5 L",	innovateLC1},
	{"LambdaBoy",			lambdaBoy},
	{"TechEdge SVout",		techEdgeNonLinear},
	{"TechEdge WBlin",		techEdgeLinear},
	{"Zeitronix",			zeitronix},
	{"Generic Linear WB",		genericWB},
};

#define nADC 1024
/*!
  \brief converts an ADC value to a narrowband AFR value
  */
G_MODULE_EXPORT gdouble NBFv (gint adc) 
{ 
	ENTER();
	EXIT();
	return 100.0 * (1.0 - adc * 5.0/nADC); 
}


/*!
  \brief converts an ADC value to a Innovate motorsports 1-2V value
  */
G_MODULE_EXPORT gdouble inno12Fv (gint adc) 
{ 
	ENTER();
	EXIT();
	return  adc * 50.0 / (nADC-1.0); 
}


/*!
  \brief converts an ADC value to a Innovate motorsports 0-5V value
  */
G_MODULE_EXPORT gdouble inno05Fv (gint adc) 
{
	ENTER();
	EXIT();
	return  10.0 + adc * 10.0 / (nADC-1.0); 
}


/*!
  \brief converts an ADC value to a Innovate motorsports LC-1 value
  */
G_MODULE_EXPORT gdouble innoLC1Fv (gint adc)
{
	ENTER();
	EXIT();
	return  7.35 + adc * 14.7 / (nADC-1.0);
}


/*!
  \brief converts an ADC value to a TechEdge Linear wideband value
  */
G_MODULE_EXPORT gdouble teWBlinFv (gint adc) 
{
	ENTER();
	EXIT();
	return   9.0 + adc * 10.0 / (nADC-1.0);
}


/*!
  \brief converts an ADC value to a Dynojet Linear wideband value
  */
G_MODULE_EXPORT gdouble djWBlinFv (gint adc) 
{
	ENTER();
	EXIT();
	return  10.0 + adc *  8.0 / (nADC-1.0);
}


/*!
  \brief fills in the AFR calibrator combobutton with the appropriate
  choices. The model stores the corresponding enum for later
  \param combo is a pointer to the combobutton to initialize
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean populate_afr_calibrator_combo(GtkWidget *combo)
{
	GtkListStore *store = NULL;
	GtkTreeIter iter;
	gint i = 0;

	ENTER();
	g_return_val_if_fail(GTK_IS_COMBO_BOX(combo),FALSE);

	store = gtk_list_store_new(NUM_COLS,G_TYPE_STRING,G_TYPE_INT);
	for (i=0;i<num_symbols;i++)
	{
		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,COL_NAME,AFR_Tables[i].name,COL_SYMBOL,AFR_Tables[i].symbol,-1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(combo),GTK_TREE_MODEL(store));
	g_object_unref(store);
//#if GTK_MINOR_VERSION < 24
	if (GTK_IS_COMBO_BOX_ENTRY(combo))
	{
		gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(combo),0);
	}
//#else
//	gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(combo),COL_NAME);
//#endif
	EXIT();
	return TRUE;
}


/*!
  \brief Called when the user selects an AFR conversion combobox entry
  \param widget is the combobox the user touched
  \param data is unused
  */
G_MODULE_EXPORT void afr_combo_changed(GtkWidget *widget, gpointer data)
{
	gboolean state = FALSE;
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;

	ENTER();
	g_return_if_fail(GTK_IS_COMBO_BOX(widget));

	state = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&iter);
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	gtk_tree_model_get(model,&iter,0,&afr_name,1,&afr_enum,-1);
	
	if (afr_enum == genericWB)
		gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(widget,"generic_controls"),TRUE);
	else
		gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(widget,"generic_controls"),FALSE);
	EXIT();
	return;
}


/*!
  \brief When the user selects the OK button the new table is calculated
  packaged up, dumped to a file as well as being sent to the ECU to live 
  a long and happy life
  \param widget is the OK button the user clicked
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean afr_calibrate_calc_and_dl(GtkWidget *widget, gpointer data)
{
	static gdouble diywbBv[] = { 0.00, 1.40, 1.45, 1.50, 1.55, 1.60, 
		1.65, 1.70, 1.75, 1.80, 1.85, 1.90, 1.95, 2.00, 2.05, 2.10,
		2.15, 2.20, 2.25, 2.30, 2.35, 2.40, 2.45, 2.50, 2.55, 2.60,
		2.65,  2.70,  2.75,  2.80,  2.85,  2.90,  4.00,  5.01 };
	static gdouble diywbBa[] = { 7.42, 10.08, 10.23, 10.38, 10.53, 10.69,
		10.86, 11.03, 11.20, 11.38, 11.57, 11.76, 11.96, 12.17, 12.38,
		12.60, 12.83, 13.07, 13.31, 13.57, 13.84, 14.11, 14.40, 14.70,
		15.25, 15.84, 16.48, 17.18, 17.93, 18.76, 19.66, 20.66, 40.00,
		60.00 };
	static gdouble lbwbBv[] = {0.00,  2.05,  4.21,  4.98,  5.01};
	static gdouble lbwbBa[] = {1.00, 11.00, 14.70, 16.00, 99.00};
	static gdouble teSVoutBv[] = { 1.024, 1.076, 1.126, 1.177, 1.227,
		1.278, 1.330, 1.380, 1.431, 1.481, 1.532, 1.581,
		1.626, 1.672, 1.717, 1.761, 1.802, 1.842, 1.883,
		1.926, 1.971, 2.015, 2.053, 2.104, 2.150, 2.192,
		2.231, 2.267, 2.305, 2.347, 2.398, 2.455, 2.514,
		2.556, 2.602, 2.650, 2.698, 2.748, 2.797, 2.846,
		2.900, 2.945, 2.991, 3.037, 3.083, 3.129, 3.175,
		3.221, 3.266, 3.313, 3.359, 3.404, 3.451, 3.496,
		3.542, 3.587, 3.634, 3.680, 3.725, 3.772, 3.817,
		3.863, 3.910, 3.955, 4.001 };
	static gdouble teSVoutBa[] = { 8.95, 9.11, 9.26, 9.41, 9.56, 9.71,
		9.87, 10.02, 10.17, 10.32, 10.47, 10.63, 10.78,
		10.93, 11.08, 11.24, 11.39, 11.54, 11.69, 11.86,
		12.04, 12.23, 12.39, 12.62, 12.83, 13.03, 13.21,
		13.4, 13.59, 13.82, 14.1, 14.43, 14.83, 15.31,
		15.85, 16.47, 17.15, 17.9, 18.7, 19.57, 20.5,
		21.5, 22.59, 23.78, 25.1, 26.54, 28.14, 29.9,
		31.87, 34.11, 36.81, 40.27, 45.1, 52.38, 63.92,
		82.66, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0,
		99.0, 99.0};
	static gdouble aemLinBv[] = { 0.00, 0.16, 0.31, 0.47, 0.62, 0.78,
		0.94, 1.09, 1.25, 1.40, 1.56, 1.72, 1.87, 2.03, 2.18,
		2.34, 2.50, 2.65, 2.81, 2.96, 3.12, 3.27, 3.43, 3.59,
		3.74, 3.90, 4.05, 4.21, 4.37, 4.52, 4.68, 4.83, 4.99, 5.01 };
	static gdouble aemLinBa[] = { 9.72, 10.01, 10.35, 10.64, 10.98,
		11.27, 11.55, 11.90, 12.18, 12.47, 12.81, 13.10, 13.44,
		13.73, 14.01, 14.35, 14.64, 14.93, 15.27, 15.56, 15.84,
		16.18, 16.47, 16.81, 17.10, 17.39, 17.73, 18.01, 18.36,
		18.64, 18.93, 19.27, 19.56, 99.00 };
	static gdouble aemNonBv[] = { 0.00, 0.16, 0.31, 0.47, 0.62, 0.78,
		0.94, 1.09, 1.25, 1.40, 1.56, 1.72, 1.87, 2.03, 2.18,
		2.34, 2.50, 2.65, 2.81, 2.96, 3.12, 3.27, 3.43, 3.59,
		3.74, 3.90, 4.05, 4.21, 4.37, 4.52, 4.68, 4.83, 4.99,
		5.01 };
	static gdouble aemNonBa[] = { 8.41, 8.52, 8.64, 8.81, 8.98, 9.09,
		9.26, 9.44, 9.61, 9.78, 9.95, 10.12, 10.29, 10.47,
		10.69, 10.92, 11.15, 11.38, 11.67, 11.95, 12.24, 12.58,
		12.92, 13.27, 13.67, 14.13, 14.64, 15.21, 15.84, 16.53,
		17.27, 18.19, 19.44, 99.00 };
	static gdouble fjoBv[] = { 0.000, 0.811, 0.816, 1.256, 1.325,
		1.408, 1.447, 1.667, 1.784, 1.804, 1.872, 1.984, 2.023,
		2.126, 2.209, 2.268, 2.414, 2.454, 2.473, 2.502, 2.522,
		2.581, 2.610, 2.717, 2.766, 2.820, 2.908, 2.933, 2.977,
		3.021, 3.079, 3.099, 3.104, 5.000 };
	static gdouble fjoBa[] = { 0.000, 9.996, 10.011, 11.113, 11.290,
		11.481, 11.569, 12.142, 12.451, 12.510, 12.730, 13.024,
		13.142, 13.465, 13.715, 13.892, 14.377, 14.524, 14.597,
		14.759, 14.876, 15.273, 15.479, 16.302, 16.714, 17.170,
		18.008, 18.243, 18.684, 19.184, 19.801, 19.977, 20.007,
		29.400 };
	static gdouble zeitronixBv[] = { 0.000, 0.150, 0.310, 0.460, 0.620,
		0.780, 0.930, 1.090, 1.240, 1.400, 1.560, 1.710, 1.870,
		2.020, 2.180, 2.340, 2.500, 2.650, 2.800, 2.960, 3.000,
		3.120, 3.270, 5.010 };
	static gdouble zeitronixBa[] = { 0.000, 9.700, 9.900, 10.100,
		10.300, 10.500, 10.700, 11.000, 11.400, 11.700, 12.100,
		12.400, 12.800, 13.200, 13.700, 14.200, 14.700, 15.600,
		16.900, 18.500, 18.800, 19.900, 21.200, 99.000 };
	static gdouble genericBv[4] = { 0.0, 1.0,  4.0, 5.01 };
	static gdouble genericBa[4] = { 0.0, 9.7, 19.7, 99.0 };
	gdouble *Bv = NULL;
	gdouble *Ba = NULL;
	gdouble voltage = 0.0;
	gdouble deltaVoltage = 0.0;
	gdouble vPct = 0.0;
	gint nB = 0;
	gint iV = 0;
	gint adcCount = 0;
	gchar * filename = NULL;
	gdouble afr = 0.0;
	gdouble (*Fv)(gint adc) = NULL;
	gboolean NB = FALSE;
	guint8 table[nADC];
	time_t tim;
	FILE *f = NULL;
	Firmware_Details *firmware = NULL;
	extern gconstpointer *global_data;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

#define USE_TABLE(prefix) \
	Bv = prefix ## Bv; \
	Ba = prefix ## Ba; \
	nB = (sizeof(prefix ## Bv) / sizeof(gdouble));


#define USE_FUNC(prefix) \
	Fv = prefix ## Fv

	switch (afr_enum)
	{
		case narrowBand:
			NB = TRUE;
			USE_FUNC(NB);
			break;
		case diyWB:
			USE_TABLE(diywb);
			break; 
		case dynojetLinear:
			USE_FUNC (djWBlin);
			break;
		case fjo:
			USE_TABLE(fjo);
			break;
		case aemLinear:
			USE_TABLE(aemLin);
			break;
		case aemNonLinear:
			USE_TABLE(aemNon);
			break;
		case twintec:
			USE_FUNC (inno05);
			break;
		case techEdgeLinear:
			USE_FUNC (teWBlin);
			break;
		case techEdgeNonLinear:
			USE_TABLE(teSVout);
			break;
		case innovate12:
			USE_FUNC (inno12);
			break;
		case innovate05:
			USE_FUNC (inno05);
			break;
		case innovateLC1:
			USE_FUNC (innoLC1);
			break;
		case lambdaBoy:
			USE_TABLE(lbwb);
			break;
		case zeitronix:
			USE_TABLE(zeitronix);
			break;
		case genericWB:
			genericBv[1] = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("voltage1_entry"))),NULL);
			genericBv[2] = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("voltage2_entry"))),NULL);
			genericBa[1] = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("afr1_entry"))),NULL);
			genericBa[2] = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(lookup_widget_f("afr2_entry"))),NULL);
			USE_TABLE(generic);
			break;
		default:
			printf(_("default case, shouldn't have gotten here. afr_enum is %i"),afr_enum);
			break;
	}
	filename = g_build_filename(HOME(), "afrtable.log",NULL);
	f = fopen(filename, "w");
	g_free(filename);
	iV = 0;
	afr = 0.0;
	time(&tim);

	fprintf(f, "//------------------------------------------------------------------------------\n");
	fprintf(f, "//--  Generated by MegaTunix %s", ctime(&tim));
	fprintf(f, "//--  This file merely records what was sent to your MS-II, and may be        --\n");
	fprintf(f, "//--  deleted at any time.                                                    --\n");
	fprintf(f, "//--  Selected type: %-57s--\n", afr_name);
	fprintf(f, "//------------------------------------------------------------------------------\n");
	fprintf(f, "#ifndef GCC_BUILD\n");
	fprintf(f, "#pragma ROM_VAR EGO_ROM\n");
	fprintf(f, "#endif\n");
	fprintf(f, "const unsigned char egofactor_table[%d] EEPROM_ATTR = {\n", nADC);
	fprintf(f, "         //     afr  adcCount voltage\n");
	for (adcCount = 0; adcCount < nADC; adcCount++) 
	{
		voltage = adcCount / (nADC-1.0) * 5.0;
		if (NB) 
		{
			afr = 0.0;
			table[adcCount] = (adcCount > nADC/5.0) ? 0 : (gint16)(nADC/5.0 - adcCount);
		}
		else 
		{
			if (Fv)
				afr = Fv(adcCount);
			else 
			{
				/* Use curve data from tabular expression of transfer function.*/
				while (voltage > Bv[iV]) 
					iV++;
				deltaVoltage = Bv[iV] - Bv[iV-1];
				if (fabs(deltaVoltage) < 1e-10) /* Curve data is crap.*/
					afr = 999.0;
				else 
				{
					vPct = 1.0 - (Bv[iV] - voltage) / deltaVoltage;
					afr  = vPct * (Ba[iV] - Ba[iV-1]) + Ba[iV-1];
				}
			}
			table[adcCount] = (guint8)(afr*10.0+0.5);
		}

		fprintf(f, "   %4d%c // %7.3f   %4d   %6.3f\n", table[adcCount], (adcCount<nADC-1)?',':' ', afr, adcCount, voltage);
	}
	fprintf(f, "};\n");
	fprintf(f, "#ifndef GCC_BUILD\n");
	fprintf(f, "#pragma ROM_VAR DEFAULT\n");
	fprintf(f, "#endif\n");
	fprintf(f, "//------------------------------------------------------------------------------\n");
	fclose(f);

	ms_table_write(firmware->ego_table_page,
			firmware->page_params[firmware->ego_table_page]->length,
			(guint8 *)table);
	EXIT();
	return TRUE;
}
