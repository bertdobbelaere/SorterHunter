/**
 * @file prefix_processor.h
 * @brief Network prefix related operations for SorterHunter program
 * @author Bert Dobbelaere bert.o.dobbelaere[at]telenet[dot]be
 *
 * Copyright (c) 2022 Bert Dobbelaere
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#include "htypes.h"
namespace sh {
	/**
	 * Given a prefix containing of 0 or more network pairs, computes the possible outputs of the (partially ordered) output set.
	 * For an empty prefix, the result will contain 2**N patterns.
	 * If the prefix is in itself a valid sorter, the result will contain N+1 patterns.
	 * @param ninputs Number of inputs to the partially ordered network
	 * @param prefix Prefix to process
	 * @param patterns [OUT] List of output patterns
	 */
	void computePrefixOutputs(u8 ninputs, const Network_t& prefix, SinglePatternList_t& patterns);

	/**
	 * Converts a set of prefix output patterns to a bit parallel data structure to speed up testing of the "postfix" network.
	 * The word size for packing is given by PARWORDSIZE
	 * @param ninputs Number of inputs to the partially ordered network
	 * @param singles Prefix output patterns to convert
	 * @param use_symmetry Optimize using symmetry
	 * @param parallels [OUT] Bit parallel representations of the patterns
	 */
	void convertToBitParallel(u8 ninputs, const SinglePatternList_t& singles, bool use_symmetry, BitParallelList_t& parallels);

	/**
	 * Tries to create a partially ordered network that (approximately) minimizes the number of possible outputs.
	 * Function is called with the list of fixed pairs (optional, empty list if none).
	 * Caller should take care of symmetry of fixed pairs.
	 * @param ninputs Number of inputs to the partially ordered network
	 * @param maxpairs Maximum number of pairs in the prefix
	 * @param use_symmetry Set to true of the computed prefix needs to be symmetrical
	 * @param prefix Contains fixed pairs as input (if any) and best prefix as output
	 * @param rndgen Random number generator for shuffling
	 * @return Number of outputs from partially ordered network (ninputs+1 if fully sorted, 2**ninputs worst case)
	 */
	SortWord_t createGreedyPrefix(u8 ninputs, u32 maxpairs, bool use_symmetry, Network_t& prefix, RandGen_t& rndgen);
}