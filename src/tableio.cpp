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
#include <conversions.h>
#include <debugging.h>
#include <defines.h>
#include <getfiles.h>
#include <firmware.h>
#include <gui_handlers.h>
#include <gtk/gtk.h>
#include <multi_expr_loader.h>
#include <notifications.h>
#include <plugin.h>
#include <tableio.h>
#include <time.h>
#include <widgetmgmt.h>

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
	g_return_if_fail(firmware);
	g_return_if_fail(DATA_GET(global_data,"interrogated"));
	g_return_if_fail(((table_num >= 0) && (table_num < firmware->total_tables)));

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup(TABLE_DATA_DIR);
	fileio->parent = lookup_widget("main_window");
	fileio->project = (const gchar *)DATA_GET(global_data,"project_name");
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Select your Table backup file to import");
	fileio->default_extension = g_strdup("yaml");
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
		g_free(x_elements);
		g_free(y_elements);
		g_free(z_elements);
	}
	g_free(filename);

	delete tbl;
}


/*!\brief imports the yaml using the extraction templates
 *\param filename to import
 *\returns a populated ThreeDTable structure
 */
ThreeDTable * yaml_3d_import(gchar * filename) {
	ThreeDTable *tbl;
	std::ifstream input(filename);
	YAML::Parser parser(input);
	YAML::Node doc;
	parser.GetNextDocument(doc);
	/* BAD code, as this assumes multiple tables, and we only expect one */
	if (doc.size() > 1)
		printf("ERROR, multiple documents within this file %s\n, Only hte FIRST ONE will be imported\n",filename);
	tbl = new ThreeDTable;
	doc[0] >> *tbl;
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
	out << YAML::Value << "";
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


G_MODULE_EXPORT void export_single_table(gint table_num) {
	Firmware_Details *firmware = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;
	gchar * filename = NULL;
	MtxFileIO *fileio = NULL;

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(DATA_GET(global_data,"interrogated"));
	g_return_if_fail(((table_num >= 0) && (table_num < firmware->total_tables)));
	t = g_new0(time_t,1);
	time(t);
	tm = localtime(t);
	g_free(t);

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup(TABLE_DATA_DIR);
	fileio->parent = lookup_widget("main_window");
	fileio->project = (const gchar *)DATA_GET(global_data,"project_name");
	fileio->on_top = TRUE;
	fileio->default_filename = g_strdup_printf("%s-%.4i%.2i%.2i%.2i%.2i.yaml",g_strdelimit(firmware->table_params[table_num]->table_name," ,",'_'),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min);
	fileio->default_extension= g_strdup("yaml");
	fileio->title = g_strdup("Select your Table backup file to export");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;
printf ("before choose_file\n");
	filename = choose_file(fileio);
printf("After choose_file\n");
	if (!filename)
	{
		update_logbar("tools_view","warning",_("NO FILE chosen for Table export\n"),FALSE,FALSE,FALSE);
		return;
	}
	export_table_to_yaml(filename,table_num);
	g_free(filename);
}


/*\brief backups up ALL tables into the user selected directory using 
 * the table name+timestamp as the filename
 */
G_MODULE_EXPORT void select_all_tables_for_export(void) {
	Firmware_Details *firmware = NULL;
	gchar * dirname = NULL;
	gchar * filename = NULL;
	MtxFileIO *fileio = NULL;
	gint i = 0;
	struct tm *tm = NULL;
	time_t *t = NULL;

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(DATA_GET(global_data,"interrogated"));

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup(TABLE_DATA_DIR);
	fileio->parent = lookup_widget("main_window");
	fileio->project = (const gchar *)DATA_GET(global_data,"project_name");
	fileio->on_top = TRUE;
	fileio->default_filename = g_strdup_printf("%s",firmware->name);
	fileio->title = g_strdup("Select your directory to place your tables into");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	dirname = choose_file(fileio);
	if (!dirname)
	{
		printf("no dirname\n");
		update_logbar("tools_view","warning",_("NO FILE chosen for Table export\n"),FALSE,FALSE,FALSE);
		return;
	}
	t = g_new0(time_t,1);
	time(t);
	tm = localtime(t);
	g_free(t);

	for (i=0;i<firmware->total_tables;i++)
	{
		filename= g_strdup_printf("%s/%s-%.4i%.2i%.2i%.2i%.2i.yaml",dirname,g_strdelimit(firmware->table_params[i]->table_name," ,",'_'),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min);
		export_table_to_yaml(filename,i);
		g_free(filename);
	}
	g_free(dirname);
}


void export_table_to_yaml(gchar *filename, gint table_num)
{
	Firmware_Details *firmware = NULL;
	gfloat *bins = NULL;
	YAML::Emitter out;
	ThreeDTable yaml_table;
	const gchar *suffix = NULL;

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);

	suffix = get_table_suffix(table_num,_X_);
	yaml_table.X.label.assign(suffix);
	yaml_table.X.unit.assign(suffix);
	suffix = get_table_suffix(table_num,_Y_);
	yaml_table.Y.label.assign(suffix);
	yaml_table.Y.unit.assign(suffix);
	suffix = get_table_suffix(table_num,_Z_);
	yaml_table.Z.label.assign(suffix);
	yaml_table.Z.unit.assign(suffix);
	yaml_table.title.assign(firmware->table_params[table_num]->table_name);
	yaml_table.description.assign(firmware->table_params[table_num]->table_name);
	yaml_table.Z.rows = firmware->table_params[table_num]->x_bincount;
	yaml_table.Z.columns = firmware->table_params[table_num]->y_bincount;
	yaml_table.X.num_elements = firmware->table_params[table_num]->x_bincount;
	yaml_table.Y.num_elements = firmware->table_params[table_num]->y_bincount;
	bins = convert_fromecu_bins(table_num,_X_);
	for (unsigned i = 0;i<yaml_table.X.num_elements;i++)
		yaml_table.X.values.push_back(bins[i]);
	g_free(bins);
	bins = convert_fromecu_bins(table_num,_Y_);
	for (unsigned i = 0;i<yaml_table.Y.num_elements;i++)
		yaml_table.Y.values.push_back(bins[i]);
	g_free(bins);
	bins = convert_fromecu_bins(table_num,_Z_);
	for (unsigned i = 0;i<(yaml_table.X.num_elements * yaml_table.Y.num_elements);i++)
		yaml_table.Z.values.push_back(bins[i]);
	g_free(bins);

	std::ofstream output(filename);
	out << yaml_table;
	output << out.c_str();
	output.close();
}



/*!\brief Takes 3 arrays of (2 Axis's and table) converts them the ECU UNITS
 * and loads them to the ECU. The table ID and X/Y/Z dimensions have 
 * ALREADY been validated.
 * \parm table_num, the table number we are updating
 * \param x_elements, the X axis elements in floating point "USER" units
 * \param y_elements, the Y axis elements in floating point "USER" units
 * \param z_elements, the Table elements in floating point "USER" units
 */
G_MODULE_EXPORT void ecu_table_import(gint table_num,gfloat *x_elements, gfloat *y_elements, gfloat *z_elements)
{
	Firmware_Details *firmware = NULL;
	guint8 *data = NULL;
	void* (*ecu_chunk_write_f)(gint,gint,gint,gint,guint8*) = NULL;
	guchar *ptr = NULL;
    guint16 *ptr16 = NULL;
    guint32 *ptr32 = NULL;
	gint * bins = NULL;
	gint count = 0;
	gint mult = 0;
	gint i = 0;

	get_symbol("ecu_chunk_write",(void **)&ecu_chunk_write_f);
	g_return_if_fail(ecu_chunk_write_f);
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);

	/*
	What needs to happen is to take these values which are all "REAL WORLD"
	user units and convert them to ECU units and send to the ECU
	Ideally this would be in a nice burst instead of one write/packet
	per value.
	*/
	/* X values first */
	bins = convert_toecu_bins(table_num, x_elements,_X_);
	mult = get_multiplier(firmware->table_params[table_num]->x_size);
	count = firmware->table_params[table_num]->x_bincount;
	data = g_new0(guchar, mult * count);
	if (mult == 1)
	{
		ptr = (guchar *)data;
		for (i=0;i<count;i++)
			ptr[i] = bins[i];
	}
	if (mult == 2)
	{
		ptr16 = (guint16 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr16[i] = GINT16_TO_BE(bins[i]);
			else
				ptr16[i] = GINT16_TO_LE(bins[i]);
		}
	}
	if (mult == 4)
	{
		ptr32 = (guint32 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr32[i] = GINT_TO_BE(bins[i]);
			else
				ptr32[i] = GINT_TO_LE(bins[i]);
		}
	}
	/* WRITE new X axis values */
	ecu_chunk_write_f(0,
			firmware->table_params[table_num]->x_page,
			firmware->table_params[table_num]->x_base,
			count*mult,
			data);
	g_free(bins);

	/* Y Bins */
	bins = convert_toecu_bins(table_num, y_elements,_Y_);
	mult = get_multiplier(firmware->table_params[table_num]->y_size);
	count = firmware->table_params[table_num]->y_bincount;
	data = g_new0(guchar, mult * count);
	if (mult == 1)
	{
		ptr = (guchar *)data;
		for (i=0;i<count;i++)
			ptr[i] = bins[i];
	}
	if (mult == 2)
	{
		ptr16 = (guint16 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr16[i] = GINT16_TO_BE(bins[i]);
			else
				ptr16[i] = GINT16_TO_LE(bins[i]);
		}
	}
	if (mult == 4)
	{
		ptr32 = (guint32 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr32[i] = GINT_TO_BE(bins[i]);
			else
				ptr32[i] = GINT_TO_LE(bins[i]);
		}
	}
	/* WRITE new Y axis values */
	g_free(bins);
	ecu_chunk_write_f(0,
			firmware->table_params[table_num]->y_page,
			firmware->table_params[table_num]->y_base,
			count*mult,
			data);

	/* Z Bins */
	bins = convert_toecu_bins(table_num, z_elements,_Z_);
	mult = get_multiplier(firmware->table_params[table_num]->z_size);
	count = firmware->table_params[table_num]->x_bincount * firmware->table_params[table_num]->y_bincount;
	data = g_new0(guchar, mult * count);
	if (mult == 1)
	{
		ptr = (guchar *)data;
		for (i=0;i<count;i++)
			ptr[i] = bins[i];
	}
	if (mult == 2)
	{
		ptr16 = (guint16 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr16[i] = GINT16_TO_BE(bins[i]);
			else
				ptr16[i] = GINT16_TO_LE(bins[i]);
		}
	}
	if (mult == 4)
	{
		ptr32 = (guint32 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr32[i] = GINT_TO_BE(bins[i]);
			else
				ptr32[i] = GINT_TO_LE(bins[i]);
		}
	}
	/* WRITE new Z values */
	ecu_chunk_write_f(0,
			firmware->table_params[table_num]->z_page,
			firmware->table_params[table_num]->z_base,
			count*mult,
			data);
	g_free(bins);
}


/*!\brief converts the float "user" values to ECU units in prep for sending
 * to the ecu
 * \param table_num, the table number identifier
 * \param elements, array of user floats
 * \param a, axis enumeration
 * \returns integer array of converted values;
 * */
gint * convert_toecu_bins(gint table_num, gfloat *elements, Axis a)
{
	Firmware_Details *firmware = NULL;
	gint mult = 0;
	gfloat *multiplier = NULL;
	gfloat *adder = NULL;
	gint *bins = NULL;
	gint count = 0;
	gint base = 0;
	gint page = 0;
	gint i = 0;
	GList *** ecu_widgets = NULL;
	GList *list = NULL;
	GtkWidget * widget = NULL;

	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	g_return_val_if_fail(ecu_widgets,NULL);
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,NULL);

	switch (a)
	{
		case _X_:
			page = firmware->table_params[table_num]->x_page;
			base = firmware->table_params[table_num]->x_base;
			mult = get_multiplier(firmware->table_params[table_num]->x_size);
			count = firmware->table_params[table_num]->x_bincount;
			break;
		case _Y_:
			page = firmware->table_params[table_num]->y_page;
			base = firmware->table_params[table_num]->y_base;
			mult = get_multiplier(firmware->table_params[table_num]->y_size);
			count = firmware->table_params[table_num]->y_bincount;
			break;
		case _Z_:
			page = firmware->table_params[table_num]->z_page;
			base = firmware->table_params[table_num]->z_base;
			mult = get_multiplier(firmware->table_params[table_num]->z_size);
			count = firmware->table_params[table_num]->x_bincount * firmware->table_params[table_num]->y_bincount;
			break;
	}

	bins = (gint *)g_new0(gint, count);
	for (i=0;i<count;i++)
	{
		list = ecu_widgets[page][base+(i*mult)];
		/* First widget is from the dataame which is what we want */
		widget = (GtkWidget *)g_list_nth_data(list,0);
		bins[i] = convert_before_download(widget,elements[i]);
	}
	return bins;
}


/*!\brief converts the integer ECU units to floating 'USER" units
 * \param table_num, the table number identifier
 * \param a, axis enumeration
 * \returns integer array of converted values;
 * */
gfloat * convert_fromecu_bins(gint table_num, Axis a)
{
	Firmware_Details *firmware = NULL;
	gint mult = 0;
	gint page = 0;
	gint base = 0;
	gint count = 0;
	gfloat *bins = NULL;
	gint i = 0;
	GList *** ecu_widgets = NULL;
	GList *list = NULL;
	GtkWidget * widget = NULL;

	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	g_return_val_if_fail(ecu_widgets,NULL);
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,NULL);

	switch (a)
	{
		case _X_:
			page = firmware->table_params[table_num]->x_page;
			mult = get_multiplier(firmware->table_params[table_num]->x_size);
			base = firmware->table_params[table_num]->x_base;
			count = firmware->table_params[table_num]->x_bincount;
			break;
		case _Y_:
			page = firmware->table_params[table_num]->y_page;
			mult = get_multiplier(firmware->table_params[table_num]->y_size);
			base = firmware->table_params[table_num]->y_base;
			count = firmware->table_params[table_num]->y_bincount;
			break;
		case _Z_:
			page = firmware->table_params[table_num]->z_page;
			base = firmware->table_params[table_num]->z_base;
			mult = get_multiplier(firmware->table_params[table_num]->z_size);
			count = firmware->table_params[table_num]->x_bincount * firmware->table_params[table_num]->y_bincount;
			break;
	}

	bins = (gfloat *)g_new0(gfloat, count);
	for (i=0;i<count;i++)
	{
		list = ecu_widgets[page][base+(i*mult)];
		/* First widget is from the dataame which is what we want */
		widget = (GtkWidget *)g_list_nth_data(list,0);
		bins[i] = convert_after_upload(widget);
	}
	return bins;
}


const gchar *get_table_suffix(gint table_num, Axis a)
{
	Firmware_Details *firmware = NULL;
	MultiSource *multi = NULL;
	GHashTable * sources_hash = NULL;
	GHashTable * hash = NULL;
	const gchar * source_key = NULL;
	const gchar * hash_key = NULL;

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,"");
	g_return_val_if_fail(DATA_GET(global_data,"interrogated"),"");
	g_return_val_if_fail(((table_num >= 0) && (table_num < firmware->total_tables)),"");

	sources_hash = (GHashTable *) DATA_GET(global_data,"sources_hash");
	switch (a)
	{
		case _X_:
			if (firmware->table_params[table_num]->x_multi_source)
			{
				source_key = firmware->table_params[table_num]->x_source_key;
				hash_key = (gchar *)g_hash_table_lookup(sources_hash,source_key);
				hash = firmware->table_params[table_num]->x_multi_hash;
				if (firmware->table_params[table_num]->x_ignore_algorithm)
					multi = (MultiSource *)g_hash_table_lookup(hash,hash_key);
				else
					multi = get_multi(table_num,hash,hash_key);
				if (!multi)
					MTXDBG(CRITICAL,_("BUG! X multi is null!\n"));
				return multi->suffix;
			}
			else 
				return firmware->table_params[table_num]->x_suffix;
			break;
		case _Y_:
			if (firmware->table_params[table_num]->y_multi_source)
			{
				source_key = firmware->table_params[table_num]->y_source_key;
				hash_key = (gchar *)g_hash_table_lookup(sources_hash,source_key);
				hash = firmware->table_params[table_num]->y_multi_hash;
				if (firmware->table_params[table_num]->y_ignore_algorithm)
					multi = (MultiSource *)g_hash_table_lookup(hash,hash_key);
				else
					multi = get_multi(table_num,hash,hash_key);
				if (!multi)
					MTXDBG(CRITICAL,_("BUG! Y multi is null!\n"));
				return multi->suffix;
			}
			else 
				return firmware->table_params[table_num]->y_suffix;
			break;
		case _Z_:
			if (firmware->table_params[table_num]->z_multi_source)
			{
				source_key = firmware->table_params[table_num]->z_source_key;
				hash_key = (gchar *)g_hash_table_lookup(sources_hash,source_key);
				hash = firmware->table_params[table_num]->z_multi_hash;
				multi = (MultiSource *)g_hash_table_lookup(hash,hash_key);
				if (firmware->table_params[table_num]->z_ignore_algorithm)
					multi = (MultiSource *)g_hash_table_lookup(hash,hash_key);
				else
					multi = get_multi(table_num,hash,hash_key);
				if (!multi)
					MTXDBG(CRITICAL,_("BUG! Z multi is null!\n"));
				return multi->suffix;
			}
			else 
				return firmware->table_params[table_num]->z_suffix;
			break;
	}
}

