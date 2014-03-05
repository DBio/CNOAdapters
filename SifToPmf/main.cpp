#include "../general/common_functions.hpp"

struct Regul {
	string source;
	string target;
	string label;
};

void outputModel(const vector<Regul> & regulations, const string & filename) {
	string last_specie;
	fstream output(filename, ios::out);
	output << "<NETWORK>" << endl;
	for (const Regul & regul : regulations) {
		bool new_specie = (regul.target != last_specie);
		if (new_specie && !last_specie.empty())
			output << "    </SPECIE>" << endl;
		if (new_specie)
			output << "    <SPECIE name=\"" << regul.target << "\">" << endl;
			
		output << "        <REGUL source=\"" << regul.source << "\" label=\"" << regul.label << "\" />" << endl;
			
		
		
		last_specie = regul.target;
	}
	
	output << "    </SPECIE>" << endl;	
	output << "</NETWORK>" << endl;
}

int main(int argc, char ** argv) {
	bool observable = false;
	if (argc == 4) {
		string obs_param{argv[3]};
		observable = obs_param == "true"  || obs_param == "1";
	}
	else if (argc != 3)
		throw invalid_argument("Wrong number of parameters.");
	string pos_cons = observable ? "ActivatingOnly" : "NotInhibiting";
	string neg_cons = observable ? "InhibitingOnly" : "NotActivating";
	
	vector<Regul> regulations;
	
	// Read input
	fstream input_file(argv[1], ios::in);
	Regul temporary;
	string label;
	set<string> sources, targets;
	while (input_file >> temporary.source && input_file >> label && input_file >> temporary.target) {
		temporary.label = label == "1" ? pos_cons : neg_cons;
		regulations.emplace_back(temporary);
		sources.insert(temporary.source);
		targets.insert(temporary.target);
	}
	
	// Add self-loops on inputs
	set<string> inputs;
	set_difference(begin(sources), end(sources), begin(targets), end(targets), inserter(inputs, inputs.begin()));
	for (const string & input : inputs) {
		regulations.emplace_back(Regul({input, input, "ActivatingOnly"}));
	}
	
	// Sort by target
	sort(begin(regulations), end(regulations), [](const Regul & A, const Regul & B) {
		return (A.target < B.target);
	});
	
	// output to the file
	outputModel(regulations, argv[2]);
	
	return 0;
}