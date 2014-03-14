/*
 * Copyright (C) 2014 - Adam Streck
 * This file is a part of the CNOAdapters suite - tools for conversion of CellNetOptR-format files into Parsybone files
 * This is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * The software is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath>.
 */

#include "program_options.hpp"
#include "../general/common_functions.hpp"

const regex CELL_LINE{ "TR:.*:CellLine" };
const regex MEASUER_T{ "DA:.*" };
const regex DATA_VAL{ "DV:.*" };
const regex INHIBITORS{ "TR:.*:Inhibitors" };
const regex STIMULI{ "TR:.* : Stimuli" };
const regex TR_I{ "TR:.*i" };
const regex TR_NON_I{ "TR:.*[^i]" };

// A POD structure that holds results for a component
struct CompData {
	size_t column_no; ///< Index of the column holding its values.
	string name; ///< Name of the component.
	enum CompType { Stimulated, Inhibited, Measured } comp_type; ///< Type of the current component
};

// Holds data about a single experiment - the experimental set-up and the time series measurements
struct Experiment {
	map <string, size_t> stimulated;
	map <string, size_t> inhibited;
	vector<string> measured;
	vector<vector<size_t>> series;
};

// @return	the index the column that holds times
size_t findDAColumn(const vector<string> & column_names) {
	const regex DA_column(MEASUER_T);
	size_t column_no = 0;
	for (const string & name : column_names) {
		if (regex_match(name, DA_column)) {
			return column_no;
		}
		column_no++;
	}

	throw runtime_error("DA_column not found.\n");
}

// @return	a name of a component from a column name
string obtainName(const string & column_name) {
	if (regex_match(column_name, CELL_LINE))
		return "";
	if (regex_match(column_name, DATA_VAL))
		return column_name.substr(3, column_name.size() - 3);
	if (regex_match(column_name, INHIBITORS))
		return column_name.substr(3, column_name.size() - 3 - strlen(":Inhibitors"));
	if (regex_match(column_name, STIMULI))
		return column_name.substr(3, column_name.size() - 3 - strlen(":Stimuli"));
	if (regex_match(column_name, TR_I))
		return column_name.substr(3, column_name.size() - 4);
	if (regex_match(column_name, TR_NON_I))
		return column_name.substr(3, column_name.size() - 3);
	return "";
}

// @return	true iff the column holds a component
inline bool isComponent(const string & column_name) {
	return !obtainName(column_name).empty();
}

// @return	true if the component is included in some measurement
CompData::CompType getType(const string & column_name) {
	if (regex_match(column_name, DATA_VAL))
		return CompData::Measured;
	if (regex_match(column_name, INHIBITORS) || regex_match(column_name, TR_I))
		return CompData::Inhibited;
	if (regex_match(column_name, STIMULI) || regex_match(column_name, TR_NON_I))
		return CompData::Stimulated;
	throw invalid_argument("Wrong column name " + column_name);
}

// @return	a vector of components holding the data relevant to the MIDAS file
vector<CompData> getComponenets(const vector<string> & column_names) {
	vector<CompData>  components;
	size_t column_no = 0;
	for (const string & column : column_names) {
		if (isComponent(column)) {
			components.push_back({ column_no, obtainName(column), getType(column) });
		}
		column_no++;
	}
	return components;
}

// @return	a 2D vector (first columns then row) containing the data from the MIDAS file
vector<vector<string>> getData(fstream & input_file) {
	vector<vector<string>> result;

	string line;
	while (getline(input_file, line)) {
		vector<string> line_data;
		boost::split(line_data, line, boost::is_any_of(","));
		result.push_back(line_data);
	}

	return result;
}

// @return	IDs of columns that contain experimental setup
vector<size_t> getColumnsOfType(const vector<CompData> & components, const CompData::CompType comp_type) {
	vector<size_t> results;

	for (const CompData & comp : components) 
		if (comp.comp_type == comp_type)
			results.push_back(comp.column_no);

	return results;
}

// @return	all combinations of experimental conditions that occur in the source
set<map<size_t, string>> getExprSetups(const vector<size_t> & TR_columns, const vector<vector<string>> & data) {
	set<map<size_t, string>> result;
	
	for (const vector<string> & data_line : data) {
		map<size_t, string> experiment;
		for (const size_t column_no : TR_columns) {
			experiment.insert({ column_no, data_line[column_no] });
		}
		result.insert(experiment);
	}
	
	return result;
}

// @return timepoints for the current experimental setup
vector<vector<string>> getSeries(const map<size_t, string> & expr_setup, const vector<vector<string>> & data) {
	vector<vector<string>> result;
	
	copy_if(begin(data), end(data), back_inserter(result), [&expr_setup](const vector<string> & data_line) {
		for (const auto expr_cond : expr_setup)
			if (expr_cond.second != data_line[expr_cond.first])
				return false;
		return true;
	});

	return result;
}

// @return names of the components that were tempered with for this experiment
map<string, size_t> getAffected(const vector<CompData> & components, const map<size_t, string> & expr_setup, const CompData::CompType comp_type) {
	map<string, size_t> result;
	
	// Take the comonents that have the required type and see if they are active in the setup - if so, add their names
	for (const CompData component : components) {
		if (component.comp_type != comp_type)
			continue;
		size_t val = stoul(expr_setup.find(component.column_no)->second);
		result.insert(make_pair(component.name, val));
	}

	return result;
}

// @return names of the components that were tempered with for this experiment
vector<string> getMeasuredNames(const vector<CompData> & components) {
	vector<string> result;

	// Take the comonents that have the required type and see if they are active in the setup - if so, add their names
	for (const CompData component : components)
		if (component.comp_type == CompData::Measured)
			result.emplace_back(component.name);

	return result;
}

// @return measured data points values as integers in the series
const vector<vector<size_t>> getMeasurements(const vector<size_t> & DV_columns, const vector<vector<string>> & series) {
	vector<vector<size_t>> result;

	for (const vector<string> & timepoint : series) {
		vector<size_t> measurement;
		for (const size_t column_no : DV_columns) {
			try {
				measurement.emplace_back(stoul(timepoint[column_no]));
			}
			catch (...) {
				measurement.emplace_back(INF);
			}
		}
		result.emplace_back(measurement);
	}

	return result;
}

// @return	timeseries where experiments that have repetitions have been removed.
const vector<vector<size_t>> removeRedundant(vector<vector<size_t>> original) {
	vector<size_t> previous{ original[0] };

	// Keep only values that differe in between two steps - returns iterator to the end of the range containing these.
	auto new_end = remove_if(begin(original) + 1, end(original), [&previous](const vector<size_t> & tested) {
		bool result = (tested == previous);
		previous = tested;
		return result;
	});

	return vector<vector<size_t>>{original.begin(), new_end};
}

// @return a series where some loop has been removed if found twice
const vector<vector<size_t>> removeCycle(vector<vector<size_t>> original) {
	vector<vector<size_t>> result;

	// This devil 1. finds a loop, 2. finds another loop, 3. if they are the same, stores the series without the second loop
	for (auto it1 = begin(original); it1 != end(original); it1++) {
		for (auto it2 = it1 + 1; it2 != end(original); it2++) {
			if (*it1 != *it2)
				continue;
			for (auto it3 = it2; it3 != end(original); it3++) {
				for (auto it4 = it3 + 1; it4 != end(original); it4++) {
					if (*it3 != *it4 || (distance(it1, it2) != distance(it3, it4)))
						continue;
					vector<bool> equal;
					transform(it1, it2, it3, back_inserter(equal), equal_to<vector<size_t>>());
					if (rng::count(equal, false) == 0) {
						result = { begin(original), it3 };
						copy(it4, end(original), back_inserter(result));
						return result;
					}
				}
			}	
		}
	}
	return original;
}

// @return	timeseries where experiments that have repetitions have been removed.
const vector<vector<size_t>> removeRepetitive(vector<vector<size_t>> original) {
	vector<vector<size_t>> result = original;

	do {
		original = result;
		result = removeCycle(original);
	} while (result != original);

	return result;
}

// @return	string naming the experimental conditions (names of components that have been tampered with)
string getExprName(const Experiment & experiment) {
	string result = "";

	auto addSetup = [&result](const map<string, size_t> conditions) {
		for (const auto & condition : conditions)
			if (condition.second != 0)
				result.append("_" + condition.first); 
	};
	
	addSetup(experiment.stimulated);
	addSetup(experiment.inhibited);

	return result;
}

// @return	string constraining the experimental conditions (as an experiment)
string getExprConst(const Experiment & experiment) {
	string result = "";

	for (const auto & stimulus : experiment.stimulated)
		result.append("&" + stimulus.first + "=" + to_string(stimulus.second));

	for (const auto & inhibiton : experiment.inhibited)
		if (inhibiton.second != 0)
			result.append("&" + inhibiton.first + "=0");

	// Remove the prefixing & if there's any
	if (!result.empty())
		result = result.substr(1u);

	return result;
}

// Produce the output
void writeProperty(const Experiment & expr, ofstream & out) {
	out << "<SERIES";
	const string constraint{ getExprConst(expr) };
	if (!constraint.empty())
		out << " experiment=\"" << constraint << "\"" ;
	out << ">" << endl;

	// Print the epxressions
	for (const vector<size_t> & measurement : expr.series) {
		out << "	<EXPR values=\"";
		vector<string> atoms;
		rng::transform(expr.measured, measurement, back_inserter(atoms), [](const string & name, const size_t val){
			if (val == 0 || val == 1)
				return name + "=" + to_string(val);
			else
				return string{ "" };
		});
		atoms.resize(distance(atoms.begin(), rng::remove_if(atoms, [](const string & atom) {return atom.empty(); })));
		std::string constraint = alg::join(atoms, "&");
		out << constraint << "\" ";

		// Set stable if there is only a single measurement
		if (expr.series.size() == 1)
			out << "stable=\"1\" ";

		out << "/>" << endl;
	}

	out << "</SERIES>";
}


// The main function expects a csv file in the MIDAS format
int main(int argc, char* argv[]) {
	bpo::variables_map po = parseProgramOptions(argc, argv);

	// Get input file
	bfs::path input_path{ po["MIDAS"].as<string>() };
	if (!bfs::exists(input_path))
		throw invalid_argument("Wrong filename \"" + input_path.string() + "\".\n");
	fstream input_stream{ input_path.string(), ios::in };

	// Read the input file
	string names_line;
	getline(input_stream, names_line);
	vector<vector<string>> data = getData(input_stream);

	// Read column names
	vector<string> column_names;
	alg::split(column_names, names_line, boost::is_any_of(","));
	vector<CompData> components = getComponenets(column_names);

	// Obtain data
	vector<size_t> TR_columns = getColumnsOfType(components, CompData::Inhibited);
	rng::copy(getColumnsOfType(components, CompData::Stimulated), back_inserter(TR_columns));
	set<map<size_t, string>> expr_setups = getExprSetups(TR_columns, data);

	// Compute the xperiments
	vector<Experiment> experiments;
	for (const auto & expr_setup : expr_setups) {
		const vector<vector<string>> series = getSeries(expr_setup, data);
		const map<string, size_t> inhibited = getAffected(components, expr_setup, CompData::Inhibited);
		const map<string, size_t> stimulated = getAffected(components, expr_setup, CompData::Stimulated);
		const vector<string> measured = getMeasuredNames(components);
		const vector<size_t> DV_columns = getColumnsOfType(components, CompData::Measured);
		const vector<vector<size_t>> measurements = getMeasurements(DV_columns, series);
		const vector<vector<size_t>> measurements2 = removeRedundant(move(measurements));
		const vector<vector<size_t>> measurements3 = removeRepetitive(move(measurements2));
		experiments.emplace_back(Experiment{ stimulated, inhibited, measured, measurements3 });
	}

	// Create output
	for (const Experiment & expr : experiments) {
		bfs::path output_path{ input_path.parent_path().string() + input_path.stem().string() + getExprName(expr) + PROPERTY_EXTENSION };
		ofstream output_stream{ output_path.string(), ios::out };
		writeProperty(expr, output_stream);
	}

	return 0;
}

