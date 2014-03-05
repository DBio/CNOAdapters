/*
 * Copyright (c) 2014 - Adam Streck 
 * MuSyCoS - Multivalued Synchronous Networks Constraint Solver 
 * MuSyCoS is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3. 
 * MuSyCoS is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>. 
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> 
 */
#pragma once
#include "definitions.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file A handful of handy functions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace CommonFunctions {
	/**
	* @brief renaming for boost::counting_range
	* @param[in] begin  first number in the range
	* @param[in] end    first number not in the range
	* @return  a counting range for [begin,end[
	*/
	template<typename IntegralType>
	inline auto crange(const IntegralType begin, const IntegralType end) -> decltype(boost::counting_range(begin, end)) {
		return boost::counting_range(begin, end);
	}

	/**
	* @brief an interface to boost::counting_range
	* @param[in] end    first number not in the range
	* @return  a counting range for [0,end[
	*/
	template<typename IntegralType>
	inline auto crange(const IntegralType end) -> decltype(boost::counting_range(end, end))  {
		return boost::counting_range(static_cast<IntegralType>(0), end);
	}

	/**
	* @brief A range adapter for containers with the size() operator.
	* @param[in] container   container whose size is in question.
	* @return  a counting range for [0,container.size()[
	*/
	template<typename SeqType>
	inline auto cscope(const SeqType & container) -> decltype(crange(container.size()))  {
		typedef decltype(container.size()) ContSizeT;
		return crange(static_cast<ContSizeT>(0), container.size());
	}

	/**
	* @brief creates a vector with the given range
	* @param[in] begin    first number in the range
	* @param[in] end    first number not in the range
	* @return  a vector for [begin,end[
	*/
	template<typename IntegralType>
	inline std::vector<IntegralType> vrange(const IntegralType begin, const IntegralType end) {
		std::vector<IntegralType> values(end - begin);

		size_t val = begin;
		generate(values.begin(), values.end(), [&val](){return val++; });

		return values;
	}

	/**
	* @brief creates a vector with the given range
	* @param[in] end    first number not in the range
	* @return  a vector for [0,end[
	*/
	template<typename IntegralType>
	inline std::vector<IntegralType> vrange(const IntegralType end)  {
		return vrange(static_cast<IntegralType>(0), end);
	}

	/**
	* @brief Increases integral value by 1.
	* @param[in,out] val  reference to value that will be increased
	*/
	template<typename IntegralType>
	void increase(IntegralType & val) { val++; }

	/**
	* @brief Sets boolean value to true.
	* @param[in,out] val  reference to value that will be increased
	*/
	template<>
	void increase<bool>(bool & val) { val = true; }

	/**
	* @brief Iterates values from left to right if it is possible. If so, return true, otherwise return false.
	* @param[in] top    vector of maximal values each component can reach
	* @param[in] bottom vector   of minimal values each component can reach
	* @param[in,out] iterated    vector of values to iterate
	* @return  true if the iteration was valid, false if it caused overflow (iterated > bottom)
	*/
	template<typename ContainerType>
	bool iterate(const ContainerType & top, const ContainerType & bottom, ContainerType & iterated) {
		for (size_t val_num = iterated.size(); val_num >= 0; val_num--) {
			if (val_num == 0)
				return false;
			if (iterated[val_num - 1] == top[val_num - 1]) {
				iterated[val_num - 1] = bottom[val_num - 1];
			}
			else {
				increase<decltype(iterated[val_num - 1])>(iterated[val_num - 1]);
				break;
			}
		}
		return true;
	}

	vector<string> getAllMatches(const string & control_regex, string source, int n = 0) {
		vector<string> result;

		smatch matches;
		while (regex_search(source, matches, regex(control_regex))) {
			result.emplace_back(matches[n]); // Capture the first (and only) submatch that holds the whole number only
			source = matches.suffix().str();
		}

		return result;
	}
} using namespace CommonFunctions;

#define NO_COPY(TypeName) \
	TypeName() = default;  \
	TypeName(TypeName && ) = default;  \
	TypeName& operator=(TypeName && ) = default; \
	TypeName(const TypeName & ) = delete; \
	TypeName& operator=(const TypeName & ) = delete; 

#define NO_COPY_SHORT(TypeName) \
	TypeName(const TypeName & ) = delete; \
	TypeName& operator=(const TypeName & ) = delete; 

#define DEFAULT_MOVE(TypeName) \
	TypeName(TypeName && ) = default;  \
	TypeName& operator=(TypeName && ) = default; 