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
#include <tableio.h>

//extern gconstpointer *global_data;


/*!\brief Yaml Table representing a 2D table data block
 */
struct Table {
	int rows;
	int columns;
	std::string unit;
	std::string label;
	std::vector <float> values;
};
/*!\brief Yaml Axis representing a 1 AXIS of a 3D table
 */
struct Axis {
	int num_elements;
	std::string unit;
	std::string label;
	std::vector <float> values;
};
/*!\brief Yaml Axis representing the combined datastructures of a 
 * single 3D table
 */
struct ThreeDTable {
	std::string title;
	std::string description;
	int rows;
	int cols;
	Axis X;
	Axis Y;
	Table Z;
};

/*!brief parses a yaml node represneting a Table structure of a 3D VE/Spark/etc
 * table
 */
void operator >> (const YAML::Node& node, Table& table) { 
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
void operator >> (const YAML::Node& node, Axis &axis) {
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
