/**
 * @file hutils.cpp
 * @brief Various utility functions for SorterHunter program
 * @author Bert Dobbelaere bert.o.dobbelaere[at]telenet[dot]be
 *
 * Copyright (c) 2017 Bert Dobbelaere
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
#include "hutils.h"
#include <cstdio>

namespace sh {

	/**
	 * Create orthogonal convex hull
	 */
	OCH_t::OCH_t()
	{
	}

	/**
	 * Clear OCH data
	 */
	void OCH_t::clear()
	{
		och.clear();
	}

	/**
	 * Add a (size, depth) pair to the OCH computation
	 * @param l length of network found
	 * @param d depth of network found
	 * @param true if the network is an "improvement" i.e. belongs to the updated set of OCH pairs that minimize both criteria.
	 */
	bool OCH_t::improved(u32 l, u32 d)
	{
		bool matched = false;

		for (size_t k = 0; k < och.size(); k++)
		{
			if ((l >= och[k].size) && (d >= och[k].depth))
				matched = true;
		}

		if (matched)
			return false;

		std::vector<OCH_Entry> newch;
		OCH_Entry ce;
		ce.size = l;
		ce.depth = d;
		newch.push_back(ce);

		for (size_t k = 0; k < och.size(); k++)
		{
			if ((och[k].size < l) || (och[k].depth < d))
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
	void OCH_t::print() const
	{
		printf("Most performant: [");
		for (size_t k = 0; k < och.size(); k++)
		{
			printf("(%u,%u)", och[k].size, och[k].depth);
			printf("%c", (k < (och.size() - 1)) ? ',' : ']');
		}
		printf("\r\n");

	}



	u32 computeDepth(const Network_t& nw)
	{
		std::vector<SortWord_t> layers;
		int nlayers = 0;

		for (size_t k = 0; k < nw.size(); k++)
		{
			u32 i = nw[k].lo;
			u32 j = nw[k].hi;
			int matchidx = nlayers;
			int idx = nlayers - 1;
			while (idx >= 0)
			{
				if ((layers[idx] & ((1ULL << i) | (1ULL << j))) == 0)
				{
					matchidx = idx;
				}
				else
				{
					break;
				}
				idx--;
			}
			if (matchidx >= nlayers)
			{
				layers.push_back(0);
				nlayers++;
			}
			layers[matchidx] |= 1ULL << i;
			layers[matchidx] |= 1ULL << j;
		}

		return nlayers;
	}


	void printnw(const Network_t& nw)
	{
		printf("[");
		for (size_t k = 0; k < nw.size(); k++)
		{
			printf("(%u,%u)", nw[k].lo, nw[k].hi);
			printf("%c", ((k + 1) < nw.size()) ? ',' : ']');
		}
		printf("}\r\n");
	}

	void symmetricExpansion(u8 ninputs, const Network_t& inpairs, Network_t& outpairs)
	{
		outpairs.clear();
		for (size_t k = 0; k < inpairs.size(); k++)
		{
			outpairs.push_back(inpairs[k]);
			if ((inpairs[k].lo + inpairs[k].hi) != (ninputs - 1)) // Don't duplicate pair that maps on itself
			{
				Pair_t sp = { (u8)(ninputs - 1 - inpairs[k].hi), (u8)(ninputs - 1 - inpairs[k].lo) };
				outpairs.push_back(sp);
			}
		}
	}


	void concatNetwork(const Network_t& nw1, const Network_t& nw2, Network_t& result)
	{
		result = nw1;
		result.insert(result.end(), nw2.begin(), nw2.end());
	}


	void appendNetwork(Network_t& dst, const Network_t& src)
	{
		if (src.size() > 0)
		{
			dst.insert(dst.end(), src.begin(), src.end());
		}
	}
}