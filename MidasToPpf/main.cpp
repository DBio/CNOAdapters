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

struct Experiment {
	vector <string> stimulated;
	vector <string> inhibited;
	vector <vector<size_t>> series;
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
vector<size_t> getColumnOfType(const vector<CompData> & components, const CompData::CompType comp_type) {
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
vector<string> getAffected(const vector<CompData> & components, const map<size_t, string> & expr_setup, const CompData::CompType comp_type) {
	vector<string> result;
	
	// Take the comonents that have the required type and see if they are active in the setup - if so, add their names
	for (const CompData component : components) {
		if (component.comp_type != comp_type)
			continue;
		if (expr_setup.find(component.column_no)->second == "1")
			result.push_back(component.name);
	}

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
				throw invalid_argument("Non-integral value of a measurement (measurements must be first discretized).");
			}
		}
		result.emplace_back(measurement);
	}

	return result;
}

// The main function expects a csv file in the MIDAS format
int main(int argc, char* argv[]) {
	// Get input file
	string filename(argv[1]);
	fstream input_file(filename, ios::in);
	if (!input_file)
		throw invalid_argument("Wrong filename \"" + filename + "\".\n");

	// Read column names
	string names_line;
	getline(input_file, names_line);
	vector<string> column_names;
	boost::split(column_names, names_line, boost::is_any_of(","));
	vector<CompData> components = getComponenets(column_names);

	// Obtain data
	vector<vector<string>> data = getData(input_file);
	vector<size_t> TR_columns = getColumnOfType(components, CompData::Inhibited);
	rng::copy(getColumnOfType(components, CompData::Stimulated), back_inserter(TR_columns));
	set<map<size_t, string>> expr_setups = getExprSetups(TR_columns, data);

	// Experiments
	vector<Experiment> experiments;
	for (const auto & expr_setup : expr_setups) {
		const vector<vector<string>> series = getSeries(expr_setup, data);
		const vector<string> inhibited = getAffected(components, expr_setup, CompData::Inhibited);
		const vector<string> stimulated = getAffected(components, expr_setup, CompData::Stimulated);
		const vector<size_t> DV_columns = getColumnOfType(components, CompData::Measured);
		const vector<vector<size_t>> measurements = getMeasurements(DV_columns, series);
		experiments.emplace_back(Experiment{ inhibited, stimulated, measurements });
	}

	return 0;
}

