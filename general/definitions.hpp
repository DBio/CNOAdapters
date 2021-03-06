/*
 * Copyright (c) 2014 - Adam Streck 
 * MuSyCoS - Multivalued Synchronous Networks Constraint Solver 
 * MuSyCoS is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3. 
 * MuSyCoS is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>. 
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> 
 */
#pragma once

#include <random>
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <limits>
#include <set>
#include <iterator>
#include <string>
#include <algorithm>
#include <map>

#include <boost/range/algorithm.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

/*
 * Copyright (C) 2014 - Adam Streck
 * This file is a part of the CNOAdapters suite - tools for conversion of CellNetOptR-format files into Parsybone files
 * This is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * The software is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath>.
 */
namespace bpo = boost::program_options;
namespace rng = boost::range;
namespace bfs = boost::filesystem;
namespace alg = boost::algorithm;
using namespace std;

const string MIDAS_EXTENSION(".csv"); ///< MIDAS file format.
const string SIF_EXTENSION(".sif"); ///< SIF graph file format.
const string PROPERTY_EXTENSION(".ppf"); ///< Parsybone property format.
const string MODEL_EXTENSION(".pmf"); ///< Parsybone model format.
const size_t INF = numeric_limits<size_t>::max();
const string VERSION("1.0.2.0");

#if (_MSC_VER != 1800)
#define stoul boost::lexical_cast<unsigned long, string>
#define stod boost::lexical_cast<double, string>
#define to_string boost::lexical_cast<string>
#endif