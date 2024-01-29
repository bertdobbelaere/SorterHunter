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
#include <random>
#include <bitset>
#include <string>

#include "htypes.h"

namespace sh {

	/**
	 * Compute the number of layers in a sorting network
	 * @param nw Input network
	 * @return Number of parallel operation layers
	 */
	inline [[nodiscard]] int computeDepth(const Network_t& nw)
	{
		std::vector<SortWord_t> layers;
		int nLayers = 0;

		for (const Pair_t& p : nw)
		{
			const SortWord_t xi = (static_cast<SortWord_t>(1) << p.lo);
			const SortWord_t xj = (static_cast<SortWord_t>(1) << p.hi);
			const SortWord_t y = (xi | xj);

			int matchIdx = nLayers;
			int idx = nLayers - 1;
			while (idx >= 0) {
				if ((layers[idx] & y) == 0) {
					matchIdx = idx;
				}
				else {
					break;
				}
				idx--;
			}
			if (matchIdx >= nLayers) {
				layers.push_back(0);
				nLayers++;
			}
			layers[matchIdx] |= xi;
			layers[matchIdx] |= xj;
		}
		return nLayers;
	}

	/**
	 * Create "symmetric" sorting network by creating a mirror image of each pair if it doesn't coincide with the original.
	 * Note: for networks with odd input sizes, the mirror image of a pair connected to the middle line will necessarilly belong
	 * to a neighbouring layer.
	 * @param ninputs Number of inputs
	 * @param inpairs Input network
	 * @param outpairs Symmetrical output network
	 */
	inline void symmetricExpansion(int ninputs, const Network_t& inpairs, INOUT Network_t& outpairs)
	{
		outpairs.clear();
		for (int k = 0; k < static_cast<int>(inpairs.size()); k++)
		{
			outpairs.push_back(inpairs[k]);
			if ((inpairs[k].lo + inpairs[k].hi) != (ninputs - 1)) // Don't duplicate pair that maps on itself
			{
				const ChannelT lo = ninputs - 1 - inpairs[k].hi;
				const ChannelT hi = ninputs - 1 - inpairs[k].lo;
				// NOTE: 50% of time is spend on pushing to outpairs
				outpairs.push_back(Pair_t(lo, hi));
			}
		}
	}

	/**
	 * Print a sorting network as text
	 * @param nw network to print
	 */
	inline void printnw(const Network_t& nw)
	{
		const int size = static_cast<int>(nw.size());
		printf("{");
		for (int k = 0; k < size; ++k) {
			printf("(%u,%u)", nw[k].lo, nw[k].hi);
			printf("%c", ((k + 1) < size) ? ',' : '}');
		}
		printf("}\r\n");
	}

	inline std::string to_string(const Network_t& nw)
	{
		const int size = static_cast<int>(nw.size());
		std::string result = "";
		for (int k = 0; k < size; ++k) {
			result += std::to_string(nw[k].lo) + ":" + std::to_string(nw[k].hi);
			if ((k + 1) < size) {
				result += ",";
			}
		}
		return result;
	}


	/**
	 * Concatenate two (partial) sorting networks into a new one
	 * @param nw1 First network
	 * @param nw2 Second network
	 * @param result [OUT] Concatenation of both networks
	 */
	inline void concatNetwork(const Network_t& nw1, const Network_t& nw2, INOUT Network_t& result)
	{
		result = nw1;
		result.insert(result.end(), nw2.begin(), nw2.end());
	}


	/**
	 * Append one network to another
	 * @param dst Network to be modified
	 * @param src Network to be appended
	 */
	inline void appendNetwork(INOUT Network_t& dst, const Network_t& src)
	{
		if (src.size() > 0)
		{
			dst.insert(dst.end(), src.begin(), src.end());
		}
	}


	/**
	 * Orthogonal Convex Hull, to keep track of unmatched (size,depth) combinations of the networks we found
	 */

	class OCH_t {
	public:
		// Create orthogonal convex hull
		OCH_t() {}

		// Clear OCH data
		void clear() {
			och.clear();
		}
		/**
		* Add a (size, depth) pair to the OCH computation
		* @param l length of network found
		* @param d depth of network found
		* @param true if the network is an "improvement" i.e. belongs to the updated set of OCH pairs that minimize both criteria.
		*/
		bool improved(int size, int depth)
		{
			bool matched = false;

			for (int k = 0; k < static_cast<int>(och.size()); k++)
			{
				if ((size >= och[k].size) && (depth >= och[k].depth)) {
					matched = true;
				}
			}

			if (matched) {
				return false;
			}
			std::vector<OCH_Entry> newch;
			OCH_Entry ce;
			ce.size = size;
			ce.depth = depth;
			newch.push_back(std::move(ce));

			for (int k = 0; k < static_cast<int>(och.size()); k++)
			{
				if ((och[k].size < size) || (och[k].depth < depth))
				{
					newch.push_back(och[k]);
				}
			}

			och = newch;
			return true;
		}
		/**
		* Print best performing (length, depth) pairs found so far
		*/
		void print() const
		{
			printf("Most performant: [");
			for (int k = 0; k < static_cast<int>(och.size()); k++)
			{
				printf("(%u,%u)", och[k].size, och[k].depth);
				printf("%c", (k < (och.size() - 1)) ? ',' : ']');
			}
			printf("\r\n");

		}
	private:
		struct OCH_Entry {
			int size;
			int depth;
		};

		std::vector<OCH_Entry> och;
	};

	// Random generation defs

	typedef std::mt19937_64 RandGen_t;

#define RANDIDX(v) (mtRand()%v.size())      ///< Random index from vector
#define RANDELEM(v) (v[RANDIDX(v)])         ///< Random element from vector
}