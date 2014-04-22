/*
 * Copyright (C) 2014 - Adam Streck
 * This file is a part of the CNOAdapters suite - tools for conversion of CellNetOptR-format files into Parsybone files
 * This is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * The software is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath>.
 */

#include "../general/common_functions.hpp"

#include "program_options.hpp"

struct Regul {
	string source;
	string target;
	string label;
};

void outputModel(const vector<Regul> & regulations, const set<string> & inputs, const string & filename) {
	fstream output(filename, ios::out);
	output << "<NETWORK>" << endl;

	for (const string & input : inputs) {
		output << "    <INPUT name=\"" << input << "\" />" << endl;
	}

	string last_specie;
	for (const Regul & regul : regulations) {
		bool new_specie = (regul.target != last_specie);
		// If there was a predecessor, finish him
		if (new_specie && !last_specie.empty())
			output << "    </SPECIE>" << endl;
		if (new_specie)
			output << "    <SPECIE name=\"" << regul.target << "\">" << endl;
			
		output << "        <REGUL source=\"" << regul.source << "\" label=\"" << regul.label << "\" />" << endl;
			
		
		
		last_specie = regul.target;
	}
	
	// Finish the last
	output << "    </SPECIE>" << endl;	
	output << "</NETWORK>" << endl;
}

int main(int argc, char ** argv) {
	try {
		bpo::variables_map program_options = parseProgramOptions(argc, argv);
		bfs::path sif_file(program_options["SIF"].as<string>());
		bfs::path pmf_file(sif_file.parent_path().string() + sif_file.stem().string() + MODEL_EXTENSION);

		// Hardcode setup
		bool observable = true;
		string pos_cons = observable ? "ActivatingOnly" : "NotInhibiting";
		string neg_cons = observable ? "InhibitingOnly" : "NotActivating";

		vector<Regul> regulations;

		// Read input
		fstream input_file(sif_file.string(), ios::in);
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

		// Sort by target
		sort(begin(regulations), end(regulations), [](const Regul & A, const Regul & B) {
			return (A.target < B.target);
		});

		// output to the file
		outputModel(regulations, inputs, pmf_file.string());
	}
	catch (exception & e) {
		cerr << "An exception thrown: " << e.what() << endl;
	}
	return 0;
}