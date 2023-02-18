/**
 * @file hutils.h
 * @brief Various utility functions for SorterHunter program
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
#include <random>

namespace sh {
	inline u32 min(u32 x, u32 y) { return (x < y) ? x : y; } ///< Classic minimum
	inline u32 max(u32 x, u32 y) { return (x > y) ? x : y; } ///< Classic maximum

	/**
	 * Compute the number of layers in a sorting network
	 * @param nw Input network
	 * @return Number of parallel operation layers
	 */
	u32 computeDepth(const Network_t& nw);

	/**
	 * Create "symmetric" sorting network by creating a mirror image of each pair if it doesn't coincide with the original.
	 * Note: for networks with odd input sizes, the mirror image of a pair connected to the middle line will necessarilly belong
	 * to a neighbouring layer.
	 * @param ninputs Number of inputs
	 * @param inpairs Input network
	 * @param outpairs Symmetrical output network
	 */
	void symmetricExpansion(u8 ninputs, const Network_t& inpairs, Network_t& outpairs);

	/**
	 * Print a sorting network as text
	 * @param nw network to print
	 */
	void printnw(const Network_t& nw);

	/**
	 * Concatenate two (partial) sorting networks into a new one
	 * @param nw1 First network
	 * @param nw2 Second network
	 * @param result [OUT] Concatenation of both networks
	 */
	void concatNetwork(const Network_t& nw1, const Network_t& nw2, Network_t& result);


	/**
	 * Append one network to another
	 * @param dst Network to be modified
	 * @param src Network to be appended
	 */
	void appendNetwork(Network_t& dst, const Network_t& src);


	/**
	 * Orthogonal Convex Hull, to keep track of unmatched (size,depth) combinations of the networks we found
	 */

	class OCH_t {
	public:
		OCH_t();
		void clear();
		bool improved(u32 size, u32 depth);
		void print() const;
	private:
		struct OCH_Entry {
			u32 size;
			u32 depth;
		};

		std::vector<OCH_Entry> och;
	};

	// Random generation defs

	typedef std::mt19937_64 RandGen_t;

#define RANDIDX(v) (mtRand()%v.size())      ///< Random index from vector
#define RANDELEM(v) (v[RANDIDX(v)])         ///< Random element from vector
}