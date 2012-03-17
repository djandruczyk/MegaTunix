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
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/tableio.c
  \ingroup CoreMtx
  \brief tableio is for yaml table import/export
  \author David Andruczyk
  */

#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
extern "C" {
#include <defines.h>
#include <getfiles.h>
#include <firmware.h>
#include <gtk/gtk.h>
#include <notifications.h>
#include <plugin.h>
#include <tableio.h>
#include <widgetmgmt.h>
}

extern gconstpointer *global_data;


/*!\brief Yaml Table representing a 2D table data block
 */
struct ZTable {
	int rows;
	int columns;
	std::string unit;
	std::string label;
	std::vector <float> values;
};
/*!\brief Yaml ThreeDAxis representing a 1 AXIS of a 3D table
 */
struct ThreeDAxis {
	int num_elements;
	std::string unit;
	std::string label;
	std::vector <float> values;
};
/*!\brief structure representing the YAML for a 3D table
 */
struct ThreeDTable {
	std::string title;
	std::string description;
	int rows;
	int cols;
	ThreeDAxis X;
	ThreeDAxis Y;
	ZTable Z;
};

ThreeDTable * yaml_3d_import(gchar *);
/*!brief parses a yaml node represneting a Table structure of a 3D VE/Spark/etc
 * table
 */
void operator >> (const YAML::Node& node, ZTable& table) { 
	int cols = 0;
	node["unit"] >> table.unit;
	node["label"] >> table.label;
	const YAML::Node& values = node["values"];
	table.rows = values.size();
	for(unsigned int i=0; i<values.size(); i++) {
		for(YAML::Iterator j=values[i].begin(); j!=values[i].end(); ++j) {
			cols++;
			float value;
			*j >> value;
			table.values.push_back(value);
		}
		table.columns = cols;
		cols = 0;
	}
}

/*!brief parses a yaml node represneting an Axis strucutre of a 3D table
 * table
 */
void operator >> (const YAML::Node& node, ThreeDAxis &axis) {
	node["unit"] >> axis.unit;
	node["label"] >> axis.label;
	const YAML::Node& values = node["values"];
	axis.num_elements = values.size();
	for (unsigned i=0; i<values.size(); i++) {
		float value;
		values[i] >> value;
		axis.values.push_back(value);
	}
}

/*!brief parses a yaml node represneting the whole 3D table itself
 * table
 */
void operator >> (const YAML::Node &node, ThreeDTable &tbl) {
	node["title"] >> tbl.title;
	node["description"] >> tbl.description;
	node["X"] >> tbl.X;
	node["Y"] >> tbl.Y;
	node["Z"] >> tbl.Z;
}


/* This doesn't work yet */
G_MODULE_EXPORT void import_single_table(gint table_num) {
	gboolean error = FALSE;
	void (*ecu_table_import)(gint, gfloat *, gfloat *, gfloat *) = NULL;
	gfloat *x_elements = NULL;
	gfloat *y_elements = NULL;
	gfloat *z_elements = NULL;
	Firmware_Details *firmware = NULL;
	MtxFileIO *fileio = NULL;
	ThreeDTable *tbl = NULL;
	gchar *filename = NULL;
	gint x_len,y_len,z_len;
	gint x_cur,y_cur,z_cur;

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	get_symbol("ecu_table_import",(void **)&ecu_table_import);
	g_return_if_fail(firmware);
	g_return_if_fail(ecu_table_import);
	g_return_if_fail(DATA_GET(global_data,"interrogated"));
	g_return_if_fail(((table_num >= 0) && (table_num < firmware->total_tables)));

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_VexFiles");
	fileio->parent = lookup_widget("main_window");
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Select your Table backup file to import");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	filename = choose_file(fileio);
	if (!filename)
	{
		update_logbar("tools_view","warning",_("NO FILE chosen for Table import\n"),FALSE,FALSE,FALSE);
		return;
	}
	tbl = yaml_3d_import(filename);
	/* Need to validate the axis ddimensions and table size */
	x_len = tbl->X.num_elements;
	y_len = tbl->Y.num_elements;
	z_len = tbl->Z.values.size();
	x_cur = firmware->table_params[table_num]->x_bincount;
	y_cur = firmware->table_params[table_num]->y_bincount;
	z_cur = firmware->table_params[table_num]->x_bincount * firmware->table_params[table_num]->y_bincount;
	/*
	printf("Import X size %i, Y size %i, Z length %i\n",x_len,y_len,z_len);
	printf("Firmware table x size is %i, y size is %i, Z length is %i\n",x_cur,y_cur,z_cur);
	*/
	if (x_len != x_cur) {
		printf("X axis lengths (%i != %i) do NOT MATCH!\n",x_len,x_cur);
		error = TRUE;
	}
	if (y_len != y_cur) {
		printf("Y axis lengths (%i != %i) do NOT MATCH!\n",y_len,y_cur);
		error = TRUE;
	}
	if (z_len != z_cur) {
		printf("Z cellcounts (%i != %i) do NOT MATCH!\n",z_len,z_cur);
		error = TRUE;
	}
	if (error)
		printf("Proposed file %s\nDOES NOT match up with running firmware,aborting load...\n",filename);
	else
	{
		x_elements = g_new0(gfloat, z_len);
		for (unsigned i=0;i<tbl->X.values.size();i++)
			x_elements[i] = tbl->X.values[i];
		y_elements = g_new0(gfloat, z_len);
		for (unsigned i=0;i<tbl->Y.values.size();i++)
			y_elements[i] = tbl->Y.values[i];
		z_elements = g_new0(gfloat, z_len);
		for (unsigned i=0;i<tbl->Z.values.size();i++)
			z_elements[i] = tbl->Z.values[i];
		ecu_table_import(table_num,x_elements,y_elements,z_elements);
	}
	g_free(filename);
			
	delete tbl;
}

ThreeDTable * yaml_3d_import(gchar * filename) {
	ThreeDTable *tbl;
	std::ifstream input(filename);
	YAML::Parser parser(input);
	YAML::Node doc;
	parser.GetNextDocument(doc);
	/* BAD code, as this assumes multiple tables, and we only expect one */
	for (unsigned i=0;i<doc.size();i++) {
		tbl = new ThreeDTable;
		doc[i] >> *tbl;
	}
	return tbl;
}

/* This doesn't work yet */
G_MODULE_EXPORT void export_single_table(gint table_num) {
	printf("told to import table number %i\n",table_num);
}
