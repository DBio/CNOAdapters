/*
 * Copyright (C) 2014 - Adam Streck
 * This file is a part of the CNOAdapters suite - tools for conversion of CellNetOptR-format files into Parsybone files
 * This is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * The software is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath>.
 */
#pragma once

#include "../general/common_functions.hpp"

/* Parse the program options - if help or version is required, terminate the program immediatelly. */
bpo::variables_map parseProgramOptions(int argc, char ** argv) {
	bpo::variables_map result;

	// Declare the supbported options.
	bpo::options_description visible("Execution options");
	visible.add_options()
		("help,h", "display help")
		("version,v", "display version")
		;
	bpo::options_description invisible;
	invisible.add_options()
		("MIDAS", bpo::value<string>()->required(), ("MIDAS file " + MIDAS_EXTENSION + " suffix").c_str())
		;
	bpo::options_description all;
	all.add(visible).add(invisible);
	bpo::positional_options_description pos_decr; pos_decr.add("MIDAS", 1);
	bpo::store(bpo::command_line_parser(argc, argv).options(all).positional(pos_decr).run(), result);
	
	if (result.count("help")) {
		cout << "MidasToPpf filename";
		cout << "    filename: data file in the MIDAS format.";
		cout << visible << "\n";
		exit(0);
	}

	if (result.count("version")) {
		cout << VERSION << "\n";
		exit(0);
	}

	bpo::notify(result);

	return result;
}