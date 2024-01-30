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

#include <algorithm>
#include <cstdio>
#include <cassert>


#pragma once
#include "htypes.h"
#include "ClusterGroup.h"

#include "State.h"

namespace sh {

	namespace {

		/**
		 * For symmetric networks, any network that sorts a pattern successfully will also sort the reverse of the inverse,
		 * i.e. if a symmetric network sorts '00101111', if will also sort '00001011'
		 * This function is used to discard the largest of those patterns.
		 */
		template <int N>
		[[nodiscard]] bool hasSmallerMirror(const SortWord_t& w)
		{
			SortWord_t rw = 0;
			SortWord_t tmp = w;
			for (int k = 0; k < N; k++) {
				rw <<= 1;
				rw |= ~tmp & static_cast<SortWord_t>(1);
				tmp >>= 1;
			}
			return w > rw;
		}

		/**
		 * Initialize alphabet of CEs.
		 * @param ninputs Number of network inputs
		 * @param use_symmetry If set to true duplicates due to mirroring will be omitted
		 */
		template <int N>
		void initAlphabet(bool use_symmetry)
		{
			state::alphabet.clear();
			for (int i = 0; i < (N - 1); i++) {
				const ChannelT jsym = N - 1 - i;
				for (int j = i + 1; j < N; j++) {
					const ChannelT isym = N - 1 - j;

					if (!use_symmetry || (isym > i) || ((isym == i) && (jsym >= j)))
					{
						state::alphabet.push_back(Pair_t(i, j));
					}
				}
			}
		}
	}

	/**
	 * Given a prefix containing of 0 or more network pairs, computes the possible outputs of the (partially ordered) output set.
	 * For an empty prefix, the result will contain 2**N patterns.
	 * If the prefix is in itself a valid sorter, the result will contain N+1 patterns.
	 * @param ninputs Number of inputs to the partially ordered network
	 * @param prefix Prefix to process
	 * @param patterns [OUT] List of output patterns
	 */
	template <int N>
	void computePrefixOutputs(const Network_t& prefix, INOUT SinglePatternList_t& patterns)
	{
		auto cg = ClusterGroup<N>();
		Network_t todo = prefix;

		while (!todo.empty())
		{
			cg.preSort(todo[0]); // Process first remaining pair, combine related clusters

			Network_t postponed;
			SortWord_t visitmask = 0;
			for (int k = 1; k < static_cast<int>(todo.size()); k++) // Skip 1st element, we just handled it
			{
				Pair_t el = todo[k];
				const SortWord_t elmask = (static_cast<SortWord_t>(1) << el.lo) | (static_cast<SortWord_t>(1) << el.hi);

				if (((visitmask & elmask) == 0) && cg.isSameCluster(el))
				{
					// Prioritize elements that can be applied without extra cluster joining.
					// The goal is to reduce memory requirements where possible
					cg.preSort(el);
				}
				else
				{
					// Postpone till next iteration any element that requires additional clusters to be joined or has dependencies to unprocessed elements
					postponed.push_back(std::move(el));
				}
				visitmask |= elmask;
			}
			todo = postponed;
		}

		cg.computeOutputs(patterns);
	}

	/**
	 * Converts a set of prefix output patterns to a bit parallel data structure to speed up testing of the "postfix" network.
	 * The word size for packing is given by PARWORDSIZE
	 * @param ninputs Number of inputs to the partially ordered network
	 * @param singles Prefix output patterns to convert
	 * @param use_symmetry Optimize using symmetry
	 * @param parallels [OUT] Bit parallel representations of the patterns
	 */
	template <int N>
	void convertToBitParallel(const SinglePatternList_t& singles, bool use_symmetry, INOUT BitParallelList_t& parallels)
	{
		int level = 0;
		std::array<BPWord_t, NMAX> buffer;
		parallels.clear();

		state::all_n_inputs_mask = 0;
		for (int k = 0; k < N; k++)
		{
			state::all_n_inputs_mask |= BPWord_t(1) << k;
		}

		for (int idx = 0; idx < singles.size(); idx++)
		{
			SortWord_t w = singles[idx];
			if (use_symmetry && hasSmallerMirror<N>(w))
			{
				continue; // Complement of reverse word is smaller, skip this vector if the network is symmetric
			}

			if (isSorted(w))
			{
				continue; // Already sorted pattern will not be affected by sorting operation - useless as test vector
			}

			for (int b = 0; b < N; b++)
			{
				buffer[b] <<= 1;
				buffer[b] |= (w & 1);
				w >>= 1;
			}
			level++;

			if (level >= PARWORDSIZE)
			{
				for (int b = 0; b < N; b++)
				{
					parallels.push_back(buffer[b]);
					buffer[b] = 0; // Needed ? Probably not, but cleaner.
				}
				level = 0;
			}
		}
		if (level > 0)
		{
			for (int b = 0; b < N; b++)
			{
				parallels.push_back(buffer[b]);
			}
		}

		if (state::Verbosity > 2)
		{
			printf("Debug: Pattern conversion: %llu single inputs -> %llu parallel words (%u * %llu) (symmetry:%d)\n", singles.size(), parallels.size(), N, parallels.size() / N, use_symmetry);
		}
	}

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

	template <int N> SortWord_t createGreedyPrefix(int maxpairs, bool use_symmetry, INOUT Network_t& prefix, INOUT RandGen_t& rndgen)
	{
		if (state::Verbosity > 2)
		{
			printf("Creating greedy prefix. Initial prefix size = %llu, max prefix size %u.\n", prefix.size(), maxpairs);
		}
		auto cg = ClusterGroup<N>();
		initAlphabet<N>(use_symmetry);

		for (int k = 0; k < static_cast<int>(prefix.size()); k++) {
			cg.preSort(prefix[k]);
		}
		SortWord_t current_size = cg.outputSize();

		while ((prefix.size() < maxpairs) || (use_symmetry && (prefix.size() < (maxpairs - 1))))
		{
			Network_t ashuf = state::alphabet;
			Pair_t best = Pair_t(0, 1);
			std::shuffle(ashuf.begin(), ashuf.end(), rndgen);
			SortWord_t minsize = current_size;

			ClusterGroup<N> cgbest = cg;
			SortWord_t min_future_size = current_size;
			for (int k = 0; k < static_cast<int>(state::alphabet.size()); k++)
			{
				ClusterGroup<N> cgnew = cg;
				cgnew.preSort(ashuf[k]);
				if (use_symmetry && ((ashuf[k].lo + ashuf[k].hi) != (N - 1)))
				{
					const ChannelT lo = N - 1 - ashuf[k].hi;
					const ChannelT hi = N - 1 - ashuf[k].lo;
					cgnew.preSort(Pair_t(lo, hi));
				}
				const SortWord_t newsize = cgnew.outputSize();
				const SortWord_t futuresize = newsize;
				if (futuresize < min_future_size)
				{
					minsize = newsize;
					min_future_size = futuresize;
					best = ashuf[k];
					cgbest = cgnew;
				}
			}

			if (minsize >= current_size)
			{
				// Found no improvement
				if (state::Verbosity > 2)
				{
					printf("Greedy algorithm: no further improvement.\n");
				}
				break;
			}
			cg = cgbest;
			if (state::Verbosity > 2)
			{
				printf("Greedy: adding pair (%u,%u)\n", best.lo, best.hi);
			}
			prefix.push_back(best);
			if (use_symmetry && ((best.lo + best.hi) != (N - 1)))
			{
				Pair_t p = Pair_t(N - 1 - best.hi, N - 1 - best.lo);
				if (state::Verbosity > 2)
				{
					printf("Greedy: adding symmetric pair (%u,%u)\n", p.lo, p.hi);
				}
				prefix.push_back(std::move(p));
			}
			current_size = minsize;
		}
		return current_size;
	}
}