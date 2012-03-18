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
#include <gui_handlers.h>
#include <gtk/gtk.h>
#include <notifications.h>
#include <plugin.h>
#include <tableio.h>
#include <time.h>
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
	std::string name;
	std::string unit;
	std::string label;
	std::vector <float> values;
};
/*!\brief structure representing the YAML for a 3D table
 */
struct ThreeDTable {
	std::string title;
	std::string description;
	ThreeDAxis X;
	ThreeDAxis Y;
	ZTable Z;
};

ThreeDTable *yaml_3d_import(gchar *);
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

YAML::Emitter & operator << (YAML::Emitter &out, const ThreeDAxis& axis) {
	out << YAML::BeginMap;
	out << YAML::Key << "unit";
	out << YAML::Value << axis.unit;
	out << YAML::Key << "label";
	out << YAML::Value << axis.label;
	out << YAML::Key << "values";
	out << YAML::Value << YAML::Flow << YAML::BeginSeq;
	for (unsigned i=0;i< axis.num_elements; i++) {
		out << axis.values[i];
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;
	return out;
}

YAML::Emitter & operator << (YAML::Emitter &out, const ZTable& tbl) {
	unsigned int count = 0;
	out << YAML::BeginMap;
	out << YAML::Key << "unit";
	out << YAML::Value << tbl.unit;
	out << YAML::Key << "label";
	out << YAML::Value << tbl.label;
	out << YAML::Key << "values";
	//out << YAML::Value << YAML::Flow << YAML::BeginSeq;
	out << YAML::Value << YAML::BeginSeq;
	for (unsigned i=0;i< tbl.rows; i++) {
		out << YAML::Flow << YAML::BeginSeq;
		for (unsigned j=0; j<tbl.columns;j++) {
			out << tbl.values[count];
			count++;
		}
		out << YAML::EndSeq;
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;
	return out;
}

YAML::Emitter & operator << (YAML::Emitter &out, const ThreeDTable& table) {
	out << YAML::BeginDoc;
	out << YAML::BeginSeq;
	out << YAML::BeginMap;
	out << YAML::Key << "3DTable";
	out << YAML::Value << NULL;
	out << YAML::Key << "title";
	out << YAML::Value << table.title;
	out << YAML::Key << "description";
	out << YAML::Value << table.description;
	out << YAML::Key << "X";
	out << YAML::Value << table.X;
	out << YAML::Key << "Y";
	out << YAML::Value << table.Y;
	out << YAML::Key << "Z";
	out << YAML::Value << table.Z;
	out << YAML::EndMap;
	out << YAML::EndSeq;
	out << YAML::EndDoc;
}



/* This doesn't work yet */
G_MODULE_EXPORT void export_single_table(gint table_num) {
	Firmware_Details *firmware = NULL;
	TableExport *table = NULL;
	gboolean error = FALSE;
	TableExport * (*ecu_table_export)(gint) = NULL;
	ThreeDTable yaml_table;
	struct tm *tm = NULL;
	time_t *t = NULL;
	gchar * filename = NULL;
	MtxFileIO *fileio = NULL;
	YAML::Emitter out;

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	get_symbol("ecu_table_export",(void **)&ecu_table_export);
	g_return_if_fail(firmware);
	g_return_if_fail(ecu_table_export);
	g_return_if_fail(DATA_GET(global_data,"interrogated"));
	g_return_if_fail(((table_num >= 0) && (table_num < firmware->total_tables)));
	t = g_new0(time_t,1);
	time(t);
	tm = localtime(t);
	g_free(t);

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_VexFiles");
	fileio->parent = lookup_widget("main_window");
	fileio->on_top = TRUE;
	fileio->default_filename= g_strdup("VEX_Backup.yaml");
	fileio->default_filename= g_strdup_printf("%s-%.4i%.2i%.2i%.2i%.2i.yaml",g_strdelimit(firmware->table_params[table_num]->table_name," ,",'_'),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min);
	fileio->default_extension= g_strdup("yaml");
	fileio->title = g_strdup("Select your Table backup file to export");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;
	filename = choose_file(fileio);
	if (!filename)
	{
		update_logbar("tools_view","warning",_("NO FILE chosen for Table export\n"),FALSE,FALSE,FALSE);
		return;
	}
	table = ecu_table_export(table_num);


	yaml_table.title = table->title;
	yaml_table.description = table->desc;
	yaml_table.X.unit = table->x_unit;
	yaml_table.X.label = table->x_label;
	yaml_table.Y.unit = table->y_unit;
	yaml_table.Y.label = table->y_label;
	yaml_table.Z.unit = table->z_unit;
	yaml_table.Z.label = table->z_label;
	yaml_table.Z.rows = table->x_len;
	yaml_table.Z.columns = table->y_len;
	yaml_table.X.num_elements = table->x_len;
	yaml_table.Y.num_elements = table->y_len;
	for (unsigned i = 0;i<table->x_len;i++)
		yaml_table.X.values.push_back(table->x_bins[i]);
	for (unsigned i = 0;i<table->y_len;i++)
		yaml_table.Y.values.push_back(table->y_bins[i]);
	for (unsigned i = 0;i<( table->x_len * table->y_len);i++)
		yaml_table.Z.values.push_back(table->z_bins[i]);

	std::ofstream output(filename);
	out << yaml_table;
	output << out.c_str();
	output.close();
}
