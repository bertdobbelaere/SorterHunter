/**
 * @file SorterHunter.cpp
 * @brief Main file of the sorting network hunter program
 * @author Bert Dobbelaere bert.o.dobbelaere[at]telenet[dot]be
 * Handles symmetrical and asymmetrical sorting networks, with or without predefined or "greedy" prefix.
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

#define VERSION "SorterHunter_V0.4"

#define  _SILENCE_CXX23_DENORM_DEPRECATION_WARNING


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <array>
#include <chrono>


#include "htypes.h"
#include "hutils.h"
#include "ConfigParser.h"
#include "prefix_processor.h"
#include "linear_to_layers.h"
#include "State.h"


namespace sh {

	/**
	 * Send a bit-parallel set of test patterns through a sorting network. Maximum PARWORDSIZE patterns are processed
	 * together.
	 * 'Data' contains N words. Each bit position corresponds to an independent data set {0,1}^N to be sorted
	 * Bit level truth table:
	 * In    Out
	 * 00 ->  00
	 * 01 ->  01
	 * 10 ->  01 ("swap")
	 * 11 ->  11
	 * @param data Input/output vectors
	 * @param nw Network to be tested
	 */
	template <int N>
	void applyBitParallelSort(INOUT std::array<BPWord_t, N>& data, const Network_t& nw)
	{
		// NOTE: 20% of time is spend in this loop
		for (const Pair_t& p : nw)
		{
			const auto i = p.lo;
			const auto j = p.hi;
			const BPWord_t iold = data[i];
			data[i] &= data[j];
			data[j] |= iold;
		}
	}

	/**
	 * Test vectors filled with input data sets fed to parallel sorter tester
	 */
	BitParallelList_t parallelpatterns_from_prefix;

	/**
	 * Initialise test vectors with patterns produced by the prefix.
	 * Test vectors are stored in parallelpatterns_from_prefix
	 * @param prefix Network prefix to use
	 */
	template <int N>
	void prepareTestVectorsFromPrefix(const Network_t& prefix)
	{
		constexpr bool is_even = ((N % 2) == 0);

		SinglePatternList_t singles;
		computePrefixOutputs<N>(prefix, singles);

		std::shuffle(singles.begin(), singles.end(), state::mtRand); // Shuffle test vectors: improve probability of early rejection of non-sorters

		convertToBitParallel<N>(singles, state::use_symmetry && is_even, parallelpatterns_from_prefix);
	}

	// Initialize "alphabet" of CEs to use
	template <int N>
	void initalphabet()
	{
		state::alphabet.clear();
		for (int i = 0; i < (N - 1); i++) {
			const int jsym = N - 1 - i;
			for (int j = i + 1; j < N; j++) {
				const int isym = N - 1 - j;

				if (!state::use_symmetry || (isym > i) || ((isym == i) && (jsym >= j)))
				{
					state::alphabet.push_back(Pair_t(i, j));
				}
			}
		}
	}

	/**
	 * Heuristic test vector reordering - attempt to speed up rejection of failing networks.
	 * Core idea is to move the test vectors that most likely reject a non-sorter to the front of the list.
	 * Withing the first group of PARWORDSIZE test vectors, the individual vectors are competing for the lowest bit position in a ladder tournament.
	 * Within that group, each time the vector with the lowest failing index is moving one step closer towards bit 0 by swapping it with its neighbour.
	 * Vectors within the 2nd group are competing with the highest bit position i.e. the "degradation candidate" of the 1st group. Vectors in higher
	 * numbered groups (3rd group or later) are not individually rewarded, but the whole group is swapped with a group that is evaluated earlier in the ranking.
	 * As the network evolves, so will the selection of "best" vectors for detecting failing mutant networks. The method described attempts to dynamically
	 * optimize the order to the evolving situation. Note that to accept a sorting network, still all test vectors need to pass, no shortcuts are taken.
	 * @param bpl List of test vectors matching the prefix (regrouped for parallel execution)
	 * @param failvector Index of first failing vector
	 */
	template<int N>
	void bumpVectorPosition(INOUT BitParallelList_t& bpl, int failvector)
	{
		const int groupno = failvector / PARWORDSIZE;
		const int idx = N * groupno;

		if (groupno > 1)
		{
			const int delta = N * ((groupno + 7) / 8);
			// Move up failing vector group about 1/8 the distance to the front
			for (int k = 0; k < N; k++)
			{
				const SortWord_t z = bpl[idx + k - delta];
				bpl[idx + k - delta] = bpl[idx + k];
				bpl[idx + k] = z;
			}
		}
		else if (groupno == 1)
		{
			// Swap with last bit position of group 0
			const SortWord_t m0 = static_cast<SortWord_t>(1) << (PARWORDSIZE - 1);
			const SortWord_t m1 = static_cast<SortWord_t>(1) << (failvector % PARWORDSIZE);
			const int shift = (PARWORDSIZE - 1) - (failvector % PARWORDSIZE);
			for (int k = 0; k < N; k++)
			{
				const SortWord_t old0 = bpl[k];
				const SortWord_t old1 = bpl[k + N];
				bpl[k] = (old0 & ~m0) | ((old1 & m1) << shift);
				bpl[k + N] = (old1 & ~m1) | ((old0 & m0) >> shift);
			}
		}
		else if (failvector > 0) // groupno==0, bit position >0
		{
			// Swap with neighbouring bit position within group 0
			const BPWord_t m0 = static_cast<BPWord_t>(1) << (failvector - 1);
			const BPWord_t m1 = static_cast<BPWord_t>(1) << failvector;
			for (int k = 0; k < N; k++)
			{
				const BPWord_t old = bpl[k];
				bpl[k] = (old & ~m0 & ~m1) | ((old & m1) >> 1) | ((old & m0) << 1);
			}
		}
	}


	/**
	 * Test a candidate network complementing the prefix.
	 * This function is called during the regular evolution loop and attempts to
	 * optimize the future order of test vectors in the background
	 * @param pairs Candidated network
	 * @param bpl List of test vectors matching the prefix
	 * @return true if prefix+pairs form a valid sorter
	 */
	template <int N> [[nodiscard]]
	bool testpairsFromPrefixOutput(const Network_t& pairs, INOUT BitParallelList_t& bpl)
	{
		int idx = 0;
		int failvector = 0;

		while (idx < bpl.size())
		{
			std::array<BPWord_t, N> data;
			BPWord_t accum = 0;

			for (int k = 0; k < N; k++) {
				data[k] = bpl[idx + k];
			}
			applyBitParallelSort<N>(data, pairs);

			for (int k = 0; k < (N - 1); k++) {
				accum |= data[k] & ~data[k + 1]; // Scan for forbidden 1 -> 0 transition
			}
			if (accum != 0)
			{
				while ((accum & static_cast<BPWord_t>(1)) == 0)
				{
					accum >>= 1;
					failvector++;
				}

				bumpVectorPosition<N>(bpl, failvector);

				return false;
			}
			idx += N;
			failvector += PARWORDSIZE;
		}
		return true;
	}

	/**
	 * Test a candidate network complementing the prefix.
	 * This function is called during the search for an initial sorter
	 * @param pairs Candidated network
	 * @param bpl List of test vectors matching the prefix
	 * @param failed_output_pattern First unsorted output pattern detected. Used to determine candidate elements to be appended.
	 * @return true if prefix+pairs form a valid sorter
	 */
	template <int N> [[nodiscard]]
	bool testInitialPairsFromPrefixOutput(const Network_t& pairs, const BitParallelList_t& bpl, INOUT SortWord_t& failed_output_pattern)
	{
		int idx = 0;
		failed_output_pattern = 0;

		while (idx < bpl.size())
		{
			std::array<BPWord_t, N> data;
			BPWord_t accum = 0;

			for (int k = 0; k < N; k++) {
				data[k] = bpl[idx + k];
			}
			applyBitParallelSort<N>(data, pairs);

			for (int k = 0; k < (N - 1); k++) {
				accum |= data[k] & ~data[k + 1]; // Scan for forbidden 1 -> 0 transition
			}
			if (accum != 0)
			{
				while ((accum & static_cast<BPWord_t>(1)) == 0)
				{
					accum >>= 1;
					for (int k = 0; k < N; k++) {
						data[k] >>= 1;
					}
				}
				for (int k = 0; k < N; k++) {
					failed_output_pattern |= (data[k] & static_cast<BPWord_t>(1)) << k;
				}
				return false;
			}
			idx += N;
		}
		return true;
	}


	/**
	 * Filter a network to obtain only the pairs that are in range 0..ninputs-1 and properly sorted
	 * @param nw input network
	 * @param ninputs Number of inputs
	 * @return Filtered input network
	 */
	template <int N>
	[[nodiscard]] Network_t copyValidPairs(const Network_t& nw)
	{
		Network_t result;
		result.clear(); //TODO remove this clear
		for (Network_t::const_iterator it = nw.begin(); it != nw.end(); it++)
		{
			if ((it->hi < N) && (it->lo < it->hi))
			{
				result.push_back(*it);
			}
		}
		return result;
	}

	/**
	 * Create a prefix network using greedy algorithm A.
	 * @param prefix [OUT] generated prefix
	 * @param npairs Number of inputs to the network
	 */
	template <int N>
	void fillprefixGreedyA(INOUT Network_t& prefix, int npairs)
	{
		prefix.clear();
		SortWord_t sizetmp = createGreedyPrefix<N>(npairs, state::use_symmetry, prefix, state::mtRand);
		if (state::Verbosity > 1)
		{
			std::cout << "Greedy prefix size " << prefix.size() << ", span " << sizetmp << std::endl;
		}
	}

	/**
	 * Create a hybrid prefix network using first the fixed prefix, then append elements with greedy algorithm A.
	 * @param prefix [OUT] generated prefix
	 * @param npairs Number of inputs to the network
	 */
	template <int N>
	void fillprefixFixedThenGreedyA(INOUT Network_t& prefix, int npairs)
	{
		prefix = copyValidPairs<N>(state::FixedPrefix);
		const SortWord_t sizetmp = createGreedyPrefix<N>(npairs + static_cast<int>(prefix.size()), state::use_symmetry, prefix, state::mtRand);
		if (state::Verbosity > 2)
		{
			std::cout << "Hybrid prefix size " << prefix.size() << ", span " << sizetmp << std::endl;
		}
	}


	/**
	 * Attempt to apply a single mutation to the network. If the mutation is a priory rejected, 0 is returned and we will try again.
	 * @param newpairs [IN/OUT] candidate network
	 * @return Positive integer identifying type of mutation applied, or 0 if none.
	 */
	[[nodiscard]] inline u32 attemptMutation(INOUT Network_t& newpairs)
	{
		u32 applied = 0; // Nothing
		const u32 mtype = 1 + RANDELEM(state::mutationSelector);

		const int new_pairs_size = static_cast<int>(newpairs.size());

		switch (mtype)
		{
		case 1:
			if (new_pairs_size > 0)   // Removal of random pair from list
			{
				const int a = static_cast<int>(RANDIDX(newpairs));
				newpairs.erase(newpairs.begin() + a);
				applied = mtype;
			}
			break;
		case 2:
			if (new_pairs_size > 1) // Swap two pairs at random positions in list
			{
				int a = static_cast<int>(RANDIDX(newpairs));
				int b = static_cast<int>(RANDIDX(newpairs));
				if (a > b) {
					const auto z = a;
					a = b;
					b = z;
				}
				if (newpairs[a] != newpairs[b])
				{
					bool dependent = false;
					const ChannelT alo = newpairs[a].lo;
					const ChannelT ahi = newpairs[a].hi;
					const ChannelT blo = newpairs[b].lo;
					const ChannelT bhi = newpairs[b].hi;

					// Pairs should either intersect, or another pair should exist between them that uses
					// one of the same 4 inputs. Otherwise, comparisons can be executed in parallel and
					// swapping them has no effect. 
					if ((blo == alo) || (blo == ahi) || (bhi == alo) || (bhi == ahi)) {
						dependent = true;
					}
					else
					{
						for (int k = a + 1; k < b; k++)
						{
							const ChannelT clo = newpairs[k].lo;
							const ChannelT chi = newpairs[k].hi;
							if ((clo == alo) || (clo == ahi) || (chi == alo) || (chi == ahi) ||
								(clo == blo) || (clo == bhi) || (chi == blo) || (chi == bhi))
							{
								dependent = true;
								break;
							}
						}
					}
					if (dependent)
					{
						Pair_t z = newpairs[a];
						newpairs[a] = newpairs[b];
						newpairs[b] = std::move(z);
						applied = mtype;
					}
				}
			}
			break;
		case 3:
			if (new_pairs_size > 0)  // Replace a pair at a random position with another random pair
			{
				const int a = static_cast<int>(RANDIDX(newpairs));
				const Pair_t p = RANDELEM(state::alphabet);
				if (newpairs[a] != p)
				{
					newpairs[a] = p;
					applied = mtype;
				}
			}
			break;
		case 4:
			if (new_pairs_size > 1) // Cross two pairs at random positions in list
			{
				const int a = static_cast<int>(RANDIDX(newpairs));
				const int b = static_cast<int>(RANDIDX(newpairs));
				const ChannelT alo = newpairs[a].lo;
				const ChannelT ahi = newpairs[a].hi;
				const ChannelT blo = newpairs[b].lo;
				const ChannelT bhi = newpairs[b].hi;

				if ((alo != blo) && (alo != bhi) && (ahi != blo) && (ahi != bhi))
				{
					const int r2 = state::mtRand() % 2;
					const ChannelT x = r2 ? bhi : blo;
					const ChannelT y = r2 ? blo : bhi;
					newpairs[a].lo = std::min(alo, x);
					newpairs[a].hi = std::max(alo, x);
					newpairs[b].lo = std::min(ahi, y);
					newpairs[b].hi = std::max(ahi, y);
					applied = mtype;
				}
			}
			break;
		case 5:
			if (new_pairs_size > 1) // Swap neighbouring intersecting pairs - special case of type r=2.
			{
				const int a = static_cast<int>(RANDIDX(newpairs));
				const ChannelT alo = newpairs[a].lo;
				const ChannelT ahi = newpairs[a].hi;
				for (int b = a + 1; b < new_pairs_size; b++)
				{
					const ChannelT blo = newpairs[b].lo;
					const ChannelT bhi = newpairs[b].hi;
					if ((blo == alo) || (blo == ahi) || (bhi == alo) || (bhi == ahi))
					{
						if (newpairs[a] != newpairs[b])
						{
							const Pair_t z = newpairs[a];
							newpairs[a] = newpairs[b];
							newpairs[b] = z;
							applied = mtype;
						}
						break;
					}
				}
			}
			break;
		case 6:
			if (new_pairs_size > 0) // Change one half of a pair - special case of type r=3.
			{
				const int a = static_cast<int>(RANDIDX(newpairs));
				const Pair_t p = newpairs[a];
				Pair_t q(0, 0);
				do {
					q = RANDELEM(state::alphabet);
				} while ((q.lo != p.lo) && (q.hi != p.lo) && (q.lo != p.hi) && (q.hi != p.hi));

				if (q != p)
				{
					newpairs[a] = q;
					applied = mtype;
				}
			}
			break;
		default:
			break;
		}

		return applied;
	}

	/**
	 * Report sorting network if it is an improved (size,depth) combination
	 * @param nw Valid sorting network
	 */
	template <int N>
	void checkImproved(const Network_t& nw, const Network_t& prefix, const Network_t& postfix)
	{
		const int depth = computeDepth(nw);
		if (state::conv_hull.improved(static_cast<int>(nw.size()), depth))
		{
			/* Print only if the sorter is an improved (size,depth) combination */
			if ((state::Verbosity > 1) || (nw.size() <= ((N * (N - 1)) / 2))) // Reduce rubbish listing. Should at least compete with bubble sort before reporting
			{
				std::cout << " {'N':" << N << ",'L':" << nw.size() << ",'D':" << depth << ",'sw':'" << VERSION << "','ESC':" << state::EscapeRate << ",'Prefix':" << prefix.size() << ",'Postfix':" << state::postfix.size() << ",'nw':" << std::endl;
				//printnw(nw);

				constexpr bool print_layers = true;

				if (print_layers) {					
					constexpr bool remove_prefix = false;
					constexpr bool remove_postfix = true;

					if (remove_prefix) {
						std::cout << "prefix:" << std::endl << tools::layers_to_string_mojo(tools::linear_to_layers(prefix)) << std::endl;
						std::cout << "layers:" << std::endl << tools::layers_to_string_mojo(tools::linear_to_layers(nw)) << std::endl;
						const auto layers = tools::linear_to_layers(tools::remove_prefix(nw, prefix));
						std::cout << "removed prefix: layers:" << std::endl << tools::layers_to_string_mojo(layers) << std::endl;
					}
					else if (remove_postfix) {
						std::cout << "postfix:" << std::endl << tools::layers_to_string_mojo(tools::linear_to_layers(postfix)) << std::endl;
						std::cout << "layers:" << std::endl << tools::layers_to_string_mojo(tools::linear_to_layers(nw)) << std::endl;
						const auto layers = tools::linear_to_layers(tools::remove_postfix(nw, postfix));
						std::cout << "removed postfix: layers:" << std::endl << tools::layers_to_string_mojo(layers) << std::endl;
					}
					else {
						std::cout << "layers:" << std::endl << tools::layers_to_string_mojo(tools::linear_to_layers(nw)) << std::endl;
					}
				}
				else { // print linear list of ce's
					std::cout << std::endl << to_string(nw) << std::endl;
				}

				state::conv_hull.print();

				const auto wall_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				std::cout << "time: " << ctime(&wall_time) << std::endl;
			}
		}
	}

	/**
	 * General help message
	 */
	inline void usage()
	{
		printf("Usage: SorterHunter <config_file_name>\n\n");
		printf("A sample config file containing help text is provided, named 'sample_config.txt'\n");
		printf("SorterHunter is a program that tries to find efficient sorting networks by applying\n");
		printf("an evolutionary approach. It is offered under MIT license\n");
		printf("Program version: %s\n", VERSION);
		exit(1);
	}


	uint64_t itercount = 0;
	uint64_t iter_next_report = 1;
	uint64_t iter_last_report = 0;
	time_t t0 = clock();
	time_t t1 = t0;


	template <int N>
	int payload() {

		if ((N % 2) && state::use_symmetry)
		{
			if (state::Verbosity > 0)
			{
				std::cout << "Warning: option 'Symmetric' ignored for odd number of inputs" << std::endl;
			}
			state::use_symmetry = false;
		}

		/* Initialize set of CEs to pick from */
		initalphabet<N>();

		/* Create initial prefix network */
		switch (state::PrefixType)
		{
		case 1: // Fixed prefix
			state::prefix = copyValidPairs<N>(state::FixedPrefix);
			break;
		case 2: // Greedy algorithm A 
			fillprefixGreedyA<N>(state::prefix, state::GreedyPrefixSize);
			break;
		case 3: // Hybrid prefix
			fillprefixFixedThenGreedyA<N>(state::prefix, state::GreedyPrefixSize);
			break;
		default: // No prefix
			state::prefix.clear();
			break;
		}

		if (state::Verbosity > 0)
		{
			std::cout << "Prefix size: " << state::prefix.size() << std::endl;
		}

		/* Prepare a set of test vectors matching the prefix */
		prepareTestVectorsFromPrefix<N>(state::prefix);


		for (;;) // Outer loop - restart from here if restart is triggered (only applies if RestartRate!=0)
		{
			state::pairs = copyValidPairs<N>(state::cp.getNetwork("InitialNetwork"));

			// Produce initial solution, simply by adding random pairs until we found a valid network. In case no postfix is present, we demand that the added pair
			// fixes at least one of the output inversions in the first detected error output vector, so it does at least some useful work to help sorting the outputs.
			// In case there is a postfix network, this check is not implemented.
			for (;;)
			{
				if (state::use_symmetry) {
					symmetricExpansion<N>(state::pairs, state::se);
				}
				else {
					state::se = state::pairs;
				}
				appendNetwork(state::se, state::postfix);

				SortWord_t failed_output_pattern = 0;

				if (testInitialPairsFromPrefixOutput<N>(state::se, parallelpatterns_from_prefix, failed_output_pattern)) {
					break;
				}
				Pair_t p = Pair_t(0, 0);

				if (state::postfix.size() == 0) // Empty postfix: find a pattern that fixes an arbitrary inversion in the first failed output
				{
					bool found_useful_ce = false;
					do {
						p = RANDELEM(state::alphabet);

						if ((((failed_output_pattern >> p.lo) & 1) == 1) && (((failed_output_pattern >> p.hi) & 1) == 0)) {
							found_useful_ce = true;
						}

						if (state::use_symmetry)
						{
							if ((((failed_output_pattern >> ((N - 1) - p.hi)) & 1) == 1) && (((failed_output_pattern >> ((N - 1) - p.lo)) & 1) == 0)) {
								found_useful_ce = true;
							}
						}

					} while (!found_useful_ce);
				}
				else // In case of postfix: just append a random initial pair to the core network, cannot directly determine good candidate from failed output pattern.
				{
					p = RANDELEM(state::alphabet);
				}

				state::pairs.push_back(std::move(p));
			}

			Network_t totalnw;
			concatNetwork(state::prefix, state::se, totalnw);

			if (state::Verbosity > 1)
			{
				std::cout << "Initial network size: " << totalnw.size() << std::endl;
			}

			checkImproved<N>(totalnw, state::prefix, state::postfix);

			for (;;) // Program never ends, keep trying to improve, we may restart in the outer loop however.
			{
				if (state::Verbosity > 2)
				{
					itercount++;
					if (itercount >= iter_next_report)
					{
						const clock_t t2 = clock();

						if ((t2 > t1) && (t2 > t0))
						{
							double t = (t2 - t0) / (double)CLOCKS_PER_SEC;
							double dt = (t2 - t1) / (double)CLOCKS_PER_SEC;
							std::cout << "Iteration " << itercount << "  t=" << t << " s     " << ((iter_next_report - iter_last_report) / dt) << " it/s" << std::endl;
						}

						t1 = t2;
						iter_last_report = iter_next_report;
						iter_next_report += (1 + iter_next_report / 10); // Report about each 10% increase of iteration count, avoid all too frequent output
					}
				}
				/* Determine number of mutations to use in this iteration */
				u32 nmods = 1;

				if (state::MaxMutations > 1)
				{
					nmods += state::mtRand() % state::MaxMutations;
				}

				/* Create a copy of the accepted set of pairs */
				state::newpairs = state::pairs;

				/* Apply the mutations */
				u32 modcount = 0;
				while (modcount < nmods)
				{
					u32 r = attemptMutation(state::newpairs);
					if (r != 0)
					{
						modcount++;
					}
				}

				/* Create a symmetric expansion of the modified pairs (or just a copy if non-symmetric network) */
				if (state::use_symmetry)
				{
					symmetricExpansion<N>(state::newpairs, state::se);
				}
				else
				{
					state::se = state::newpairs;
				}

				appendNetwork(state::se, state::postfix);

				/* Test whether the new postfix network yields a valid sorter when combined with the prefix */
				if ((state::se.size() > 0) && testpairsFromPrefixOutput<N>(state::se, parallelpatterns_from_prefix))
				{
					concatNetwork(state::prefix, state::se, totalnw);

					/* Accept the new postfix */
					state::pairs = state::newpairs;

					checkImproved<N>(totalnw, state::prefix, state::postfix);
				}

				/* With low probability, add another pair random pair at a random place. Attempt to escape from local optimum. */
				if ((state::EscapeRate > 0) && ((state::mtRand() % state::EscapeRate) == 0))
				{
					const int a = static_cast<int>(state::mtRand() % (state::pairs.size() + 1)); // Random insertion position
					Pair_t p = RANDELEM(state::alphabet);

					// Determine if the random pair p could be added in the last layer
					bool hit_successor = false;
					for (Network_t::const_iterator it = state::pairs.begin() + a; it != state::pairs.end(); it++)
					{
						if ((it->lo == p.lo) || (it->hi == p.lo) || (it->lo == p.hi) || (it->hi == p.hi))
						{
							hit_successor = true;
							break;
						}
					}

					if (state::force_valid_uphill_step && hit_successor)
					{
						state::pairs.insert(state::pairs.begin() + a, state::pairs[a]); // Prepend duplicate of existing pair right in front of it => Sorter with redundant pair will remain valid
					}
					else
					{
						state::pairs.insert(state::pairs.begin() + a, p); // Add random pair at the end of the network
					}
				}

				if ((state::RestartRate > 0) && ((state::mtRand() % state::RestartRate) == 0))
				{
					if (state::Verbosity > 1)
					{
						printf("Restart.\n");
					}
					switch (state::PrefixType) // Recompute prefix if not fixed
					{
					case 1: // Fixed prefix - no update: vectors remain the same after restart
						break;
					case 2: // Greedy algorithm A 
						fillprefixGreedyA<N>(state::prefix, state::GreedyPrefixSize);
						prepareTestVectorsFromPrefix<N>(state::prefix);
						break;
					case 3: // Hybrid prefix
						fillprefixFixedThenGreedyA<N>(state::prefix, state::GreedyPrefixSize);
						prepareTestVectorsFromPrefix<N>(state::prefix);
						break;
					default: // No prefix - no update: vectors remain the same after restart
						break;
					}

					break; // Restart using outer loop
				}
			}
		}
		return 0; // unreachable
	}
}

/**
 * SorterHunter main routine
 */

using namespace sh;


int main(int argc, char* argv[])
{
	// Handle validity of command line options - extremely simple
	if (argc != 2)
	{
		usage();
		return -1;
	}

	// Process configuration file
	if (!state::cp.parseConfig(argv[1]))
	{
		printf("Error parsing config options.\n");
		static_cast<void>(std::getchar());
		return -1;
	}

	if (state::cp.getInt("RandomSeed") != 0u)
	{
		state::RandomSeed = state::cp.getInt("RandomSeed");
		state::mtRand.seed(state::RandomSeed);
	}

	const int n_inputs = static_cast<int>(state::cp.getInt("Ninputs", 0));
	state::use_symmetry = (state::cp.getInt("Symmetric") > 0);
	state::force_valid_uphill_step = (state::cp.getInt("ForceValidUphillStep", 1) > 0);
	state::EscapeRate = static_cast<int>(state::cp.getInt("EscapeRate", 0));
	state::MaxMutations = static_cast<int>(state::cp.getInt("MaxMutations", 1));
	state::mutation_type_weights[0] = static_cast<int>(state::cp.getInt("WeigthRemovePair", 1));
	state::mutation_type_weights[1] = static_cast<int>(state::cp.getInt("WeigthSwapPairs", 1));
	state::mutation_type_weights[2] = static_cast<int>(state::cp.getInt("WeigthReplacePair", 1));
	state::mutation_type_weights[3] = static_cast<int>(state::cp.getInt("WeightCrossPairs", 1));
	state::mutation_type_weights[4] = static_cast<int>(state::cp.getInt("WeightSwapIntersectingPairs", 1));
	state::mutation_type_weights[5] = static_cast<int>(state::cp.getInt("WeightReplaceHalfPair", 1));
	for (int n = 0; n < state::NMUTATIONTYPES; n++)
	{
		for (int k = 0; k < state::mutation_type_weights[n]; k++) {
			state::mutationSelector.push_back(n);
		}
	}
	if (state::mutationSelector.empty())
	{
		printf("No mutation types selected.\n");
		exit(1);
	}
	state::PrefixType = static_cast<int>(state::cp.getInt("PrefixType", 0));
	state::FixedPrefix = state::cp.getNetwork("FixedPrefix");
	state::GreedyPrefixSize = static_cast<int>(state::cp.getInt("GreedyPrefixSize", 0));
	state::RestartRate = static_cast<int>(state::cp.getInt("RestartRate", 0));
	state::Verbosity = static_cast<int>(state::cp.getInt("Verbosity", 1));
	state::postfix = state::cp.getNetwork("Postfix");


	//for (int i = 2; i < 256; ++i) {
	//	std::cout << "   case " << i << ": return payload<" << i << ">();" << std::endl;
	//}

	switch (n_inputs) {
	case 2: return payload<2>();
	case 3: return payload<3>();
	case 4: return payload<4>();
	case 5: return payload<5>();
	case 6: return payload<6>();
	case 7: return payload<7>();
	case 8: return payload<8>();
	case 9: return payload<9>();
	case 10: return payload<10>();
	case 11: return payload<11>();
	case 12: return payload<12>();
	case 13: return payload<13>();
	case 14: return payload<14>();
	case 15: return payload<15>();
	case 16: return payload<16>();
	case 17: return payload<17>();
	case 18: return payload<18>();
	case 19: return payload<19>();
	case 20: return payload<20>();
	case 21: return payload<21>();
	case 22: return payload<22>();
	case 23: return payload<23>();
	case 24: return payload<24>();
	case 25: return payload<25>();
	case 26: return payload<26>();
	case 27: return payload<27>();
	case 28: return payload<28>();
	case 29: return payload<29>();
	case 30: return payload<30>();
	case 31: return payload<31>();
	case 32: return payload<32>();
	case 33: return payload<33>();
	case 34: return payload<34>();
	case 35: return payload<35>();
	case 36: return payload<36>();
	case 37: return payload<37>();
	case 38: return payload<38>();
	case 39: return payload<39>();
	case 40: return payload<40>();
	case 41: return payload<41>();
	case 42: return payload<42>();
	case 43: return payload<43>();
	case 44: return payload<44>();
	case 45: return payload<45>();
	case 46: return payload<46>();
	case 47: return payload<47>();
	case 48: return payload<48>();
	case 49: return payload<49>();
	case 50: return payload<50>();
	case 51: return payload<51>();
	case 52: return payload<52>();
	case 53: return payload<53>();
	case 54: return payload<54>();
	case 55: return payload<55>();
	case 56: return payload<56>();
	case 57: return payload<57>();
	case 58: return payload<58>();
	case 59: return payload<59>();
	case 60: return payload<60>();
	case 61: return payload<61>();
	case 62: return payload<62>();
	case 63: return payload<63>();
	case 64: return payload<64>();
	case 65: return payload<65>();
	case 66: return payload<66>();
	case 67: return payload<67>();
	case 68: return payload<68>();
	case 69: return payload<69>();
	case 70: return payload<70>();
	case 71: return payload<71>();
	case 72: return payload<72>();
	case 73: return payload<73>();
	case 74: return payload<74>();
	case 75: return payload<75>();
	case 76: return payload<76>();
	case 77: return payload<77>();
	case 78: return payload<78>();
	case 79: return payload<79>();
	case 80: return payload<80>();
	case 81: return payload<81>();
	case 82: return payload<82>();
	case 83: return payload<83>();
	case 84: return payload<84>();
	case 85: return payload<85>();
	case 86: return payload<86>();
	case 87: return payload<87>();
	case 88: return payload<88>();
	case 89: return payload<89>();
	case 90: return payload<90>();
	case 91: return payload<91>();
	case 92: return payload<92>();
	case 93: return payload<93>();
	case 94: return payload<94>();
	case 95: return payload<95>();
	case 96: return payload<96>();
	case 97: return payload<97>();
	case 98: return payload<98>();
	case 99: return payload<99>();
	case 100: return payload<100>();
	case 101: return payload<101>();
	case 102: return payload<102>();
	case 103: return payload<103>();
	case 104: return payload<104>();
	case 105: return payload<105>();
	case 106: return payload<106>();
	case 107: return payload<107>();
	case 108: return payload<108>();
	case 109: return payload<109>();
	case 110: return payload<110>();
	case 111: return payload<111>();
	case 112: return payload<112>();
	case 113: return payload<113>();
	case 114: return payload<114>();
	case 115: return payload<115>();
	case 116: return payload<116>();
	case 117: return payload<117>();
	case 118: return payload<118>();
	case 119: return payload<119>();
	case 120: return payload<120>();
	case 121: return payload<121>();
	case 122: return payload<122>();
	case 123: return payload<123>();
	case 124: return payload<124>();
	case 125: return payload<125>();
	case 126: return payload<126>();
	case 127: return payload<127>();
	case 128: return payload<128>();
	case 129: return payload<129>();
	case 130: return payload<130>();
	case 131: return payload<131>();
	case 132: return payload<132>();
	case 133: return payload<133>();
	case 134: return payload<134>();
	case 135: return payload<135>();
	case 136: return payload<136>();
	case 137: return payload<137>();
	case 138: return payload<138>();
	case 139: return payload<139>();
	case 140: return payload<140>();
	case 141: return payload<141>();
	case 142: return payload<142>();
	case 143: return payload<143>();
	case 144: return payload<144>();
	case 145: return payload<145>();
	case 146: return payload<146>();
	case 147: return payload<147>();
	case 148: return payload<148>();
	case 149: return payload<149>();
	case 150: return payload<150>();
	case 151: return payload<151>();
	case 152: return payload<152>();
	case 153: return payload<153>();
	case 154: return payload<154>();
	case 155: return payload<155>();
	case 156: return payload<156>();
	case 157: return payload<157>();
	case 158: return payload<158>();
	case 159: return payload<159>();
	case 160: return payload<160>();
	case 161: return payload<161>();
	case 162: return payload<162>();
	case 163: return payload<163>();
	case 164: return payload<164>();
	case 165: return payload<165>();
	case 166: return payload<166>();
	case 167: return payload<167>();
	case 168: return payload<168>();
	case 169: return payload<169>();
	case 170: return payload<170>();
	case 171: return payload<171>();
	case 172: return payload<172>();
	case 173: return payload<173>();
	case 174: return payload<174>();
	case 175: return payload<175>();
	case 176: return payload<176>();
	case 177: return payload<177>();
	case 178: return payload<178>();
	case 179: return payload<179>();
	case 180: return payload<180>();
	case 181: return payload<181>();
	case 182: return payload<182>();
	case 183: return payload<183>();
	case 184: return payload<184>();
	case 185: return payload<185>();
	case 186: return payload<186>();
	case 187: return payload<187>();
	case 188: return payload<188>();
	case 189: return payload<189>();
	case 190: return payload<190>();
	case 191: return payload<191>();
	case 192: return payload<192>();
	case 193: return payload<193>();
	case 194: return payload<194>();
	case 195: return payload<195>();
	case 196: return payload<196>();
	case 197: return payload<197>();
	case 198: return payload<198>();
	case 199: return payload<199>();
	case 200: return payload<200>();
	case 201: return payload<201>();
	case 202: return payload<202>();
	case 203: return payload<203>();
	case 204: return payload<204>();
	case 205: return payload<205>();
	case 206: return payload<206>();
	case 207: return payload<207>();
	case 208: return payload<208>();
	case 209: return payload<209>();
	case 210: return payload<210>();
	case 211: return payload<211>();
	case 212: return payload<212>();
	case 213: return payload<213>();
	case 214: return payload<214>();
	case 215: return payload<215>();
	case 216: return payload<216>();
	case 217: return payload<217>();
	case 218: return payload<218>();
	case 219: return payload<219>();
	case 220: return payload<220>();
	case 221: return payload<221>();
	case 222: return payload<222>();
	case 223: return payload<223>();
	case 224: return payload<224>();
	case 225: return payload<225>();
	case 226: return payload<226>();
	case 227: return payload<227>();
	case 228: return payload<228>();
	case 229: return payload<229>();
	case 230: return payload<230>();
	case 231: return payload<231>();
	case 232: return payload<232>();
	case 233: return payload<233>();
	case 234: return payload<234>();
	case 235: return payload<235>();
	case 236: return payload<236>();
	case 237: return payload<237>();
	case 238: return payload<238>();
	case 239: return payload<239>();
	case 240: return payload<240>();
	case 241: return payload<241>();
	case 242: return payload<242>();
	case 243: return payload<243>();
	case 244: return payload<244>();
	case 245: return payload<245>();
	case 246: return payload<246>();
	case 247: return payload<247>();
	case 248: return payload<248>();
	case 249: return payload<249>();
	case 250: return payload<250>();
	case 251: return payload<251>();
	case 252: return payload<252>();
	case 253: return payload<253>();
	case 254: return payload<254>();
	case 255: return payload<255>();
	default:
		std::cout << "payload for N = " << n_inputs << " is not implemented." << std::endl;
		return 0;
	}
}
