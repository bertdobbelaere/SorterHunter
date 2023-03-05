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

#include "htypes.h"
#include "hutils.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <random>
#include "ConfigParser.h"
#include <ctime>
#include "prefix_processor.h"


namespace sh {
	ConfigParser cp;

	bool use_symmetry = true; ///< Treat sorting network as symmetric or not
	bool force_valid_uphill_step = true; ///< "Uphill" step inserts duplicate CE if not in final layer.
	u8 N = 0;                   ///< Problem dimension, i.e. number of inputs to be sorted
	u32 EscapeRate = 0;         ///< Adds a random pair (and its symmetric complement for symmetric networks) every x iterations
	u32 MaxMutations = 1;       ///< Maximum allowed number of mutations in evolution step
	u32 PrefixType = 0;         ///< Type of prefix used (0=none, 1=fixed, 2=greedy)
	Network_t FixedPrefix;    ///< Fixed prefix to use (if applicable)
	Network_t InitialNetwork; ///< Initial starting point of network
	u32 GreedyPrefixSize = 0;   ///< Size of greedy prefix (if applicable)
	OCH_t conv_hull;          ///< "Best performing" network list found so far
	uint64_t RandomSeed;      ///< Random seed
	uint64_t RestartRate;     ///< Return to initial conditions each ... iterations (0=never)
	u32 Verbosity = 1;          ///< Overall verbosity level: 0:minimal, 1:moderate, 2:high, >2:debug        

	// Working set of pairs in the sorting network
	Network_t pairs; ///< Current core network: evolving section between prefix and postfix. For symmetric networks, mirrored pair (if not coinciding) is omitted.
	Network_t se; ///< Symmetrical expansion of current network
	Network_t newpairs;
	Network_t prefix; ///< Fixed, greedy, hybrid or empty prefix network
	Network_t postfix; ///< Fixed or empty postfix network

	// Set of all possible pairs, unique taking into account symmetric complements
	Network_t alphabet;

#define NMUTATIONTYPES 6 ///< Number of different mutation types
	u32 mutation_type_weights[NMUTATIONTYPES]; ///< Relative probabilities for each mutation type
	std::vector<u8> mutationSelector; ///< Helper variable to quickly pick a mutation with the requested probability.

	// Random generation
	std::random_device rd;
	RandGen_t mtRand(rd()); // Mersenne twister is a rather good PRNG. Seeding quality varies between systems, but OK ; this is no crypto application.


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
	void applyBitParallelSort(BPWord_t data[], const Network_t& nw)
	{
		size_t l = nw.size();
		for (size_t n = 0; n < l; n++)
		{
			u32 i = nw[n].lo;
			u32 j = nw[n].hi;
			BPWord_t iold = data[i];
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
	void prepareTestVectorsFromPrefix(const Network_t& prefix)
	{
		bool is_even = ((N % 2) == 0);

		SinglePatternList_t singles;
		computePrefixOutputs(N, prefix, singles);

		std::shuffle(singles.begin(), singles.end(), mtRand); // Shuffle test vectors: improve probability of early rejection of non-sorters

		convertToBitParallel(N, singles, use_symmetry && is_even, parallelpatterns_from_prefix);
	}

	/**
	 * Initialize "alphabet" of CEs to use
	 */

	void initalphabet()
	{
		alphabet.clear();
		for (u32 i = 0; i < (N - 1u); i++)
			for (u32 j = i + 1; j < N; j++)
			{
				u32 isym = N - 1 - j;
				u32 jsym = N - 1 - i;

				if (!use_symmetry || (isym > i) || ((isym == i) && (jsym >= j)))
				{
					Pair_t p = { (u8)i,(u8)j };
					alphabet.push_back(p);
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
	void bumpVectorPosition(BitParallelList_t& bpl, size_t failvector)
	{
		size_t groupno = failvector / PARWORDSIZE;
		size_t idx = N * groupno;

		if (groupno > 1)
		{
			size_t delta = N * ((groupno + 7) / 8);
			// Move up failing vector group about 1/8 the distance to the front
			for (size_t k = 0; k < N; k++)
			{
				BPWord_t z = bpl[idx + k - delta];
				bpl[idx + k - delta] = bpl[idx + k];
				bpl[idx + k] = z;
			}
		}
		else if (groupno == 1)
		{
			// Swap with last bit position of group 0
			BPWord_t m0 = 1ull << (PARWORDSIZE - 1);
			BPWord_t m1 = 1ull << (failvector % PARWORDSIZE);
			int shift = (PARWORDSIZE - 1) - (failvector % PARWORDSIZE);
			for (size_t k = 0; k < N; k++)
			{
				BPWord_t old0 = bpl[k];
				BPWord_t old1 = bpl[k + N];
				bpl[k] = (old0 & ~m0) | ((old1 & m1) << shift);
				bpl[k + N] = (old1 & ~m1) | ((old0 & m0) >> shift);
			}
		}
		else if (failvector > 0) // groupno==0, bit position >0
		{
			//assert(failvector<PARWORDSIZE);
			// Swap with neighbouring bit position within group 0
			BPWord_t m0 = 1ull << (failvector - 1);
			BPWord_t m1 = 1ull << failvector;
			for (size_t k = 0; k < N; k++)
			{
				BPWord_t old = bpl[k];
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
	bool testpairsFromPrefixOutput(const Network_t& pairs, BitParallelList_t& bpl)
	{
		size_t idx = 0;
		size_t failvector = 0;

		while (idx < bpl.size())
		{
			static BPWord_t data[NMAX];
			BPWord_t accum = 0;

			for (size_t k = 0; k < N; k++)
				data[k] = bpl[idx + k];

			applyBitParallelSort(data, pairs);

			for (size_t k = 0; k < (N - 1u); k++)
				accum |= data[k] & ~data[k + 1]; // Scan for forbidden 1 -> 0 transition
			if (accum != 0ULL)
			{
				while ((accum & 1ull) == 0)
				{
					accum >>= 1;
					failvector++;
				}

				bumpVectorPosition(bpl, failvector);

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
	bool testInitialPairsFromPrefixOutput(const Network_t& pairs, const BitParallelList_t& bpl, SortWord_t& failed_output_pattern)
	{
		size_t idx = 0;
		failed_output_pattern = 0;

		while (idx < bpl.size())
		{
			static BPWord_t data[NMAX];
			BPWord_t accum = 0;

			for (size_t k = 0; k < N; k++)
				data[k] = bpl[idx + k];

			applyBitParallelSort(data, pairs);

			for (size_t k = 0; k < (N - 1u); k++)
				accum |= data[k] & ~data[k + 1]; // Scan for forbidden 1 -> 0 transition
			if (accum != 0ULL)
			{
				while ((accum & 1ull) == 0)
				{
					accum >>= 1;
					for (size_t k = 0; k < N; k++)
						data[k] >>= 1;
				}

				for (size_t k = 0; k < N; k++)
					failed_output_pattern |= (data[k] & 1) << k;

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
	static const Network_t copyValidPairs(const Network_t& nw, u32 ninputs)
	{
		static Network_t result;
		result.clear();
		for (Network_t::const_iterator it = nw.begin(); it != nw.end(); it++)
		{
			if ((it->hi < ninputs) && (it->lo < it->hi))
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
	void fillprefixGreedyA(Network_t& prefix, u32 npairs)
	{
		prefix.clear();
		SortWord_t sizetmp = createGreedyPrefix(N, npairs, use_symmetry, prefix, mtRand);
		if (Verbosity > 1)
		{
			printf("Greedy prefix size %llu, span %llu.\n", prefix.size(), (size_t)sizetmp);
		}
	}

	/**
	 * Create a hybrid prefix network using first the fixed prefix, then append elements with greedy algorithm A.
	 * @param prefix [OUT] generated prefix
	 * @param npairs Number of inputs to the network
	 */
	void fillprefixFixedThenGreedyA(Network_t& prefix, u32 npairs)
	{
		prefix = copyValidPairs(FixedPrefix, N);
		SortWord_t sizetmp = createGreedyPrefix(N, npairs + prefix.size(), use_symmetry, prefix, mtRand);
		if (Verbosity > 2)
		{
			printf("Hybrid prefix size %llu, span %llu.\n", prefix.size(), (size_t)sizetmp);
		}
	}


	/**
	 * Attempt to apply a single mutation to the network. If the mutation is a priory rejected, 0 is returned and we will try again.
	 * @param newpairs [IN/OUT] candidate network
	 * @return Positive integer identifying type of mutation applied, or 0 if none.
	 */
	u32 attemptMutation(Network_t& newpairs)
	{
		u32 applied = 0; // Nothing
		u32 mtype = 1 + RANDELEM(mutationSelector);

		switch (mtype)
		{
		case 1:
			if (newpairs.size() > 0)   // Removal of random pair from list
			{
				u32 a = RANDIDX(newpairs);
				newpairs.erase(newpairs.begin() + a);
				applied = mtype;
			}
			break;
		case 2:
			if (newpairs.size() > 1) // Swap two pairs at random positions in list
			{
				u32 a = RANDIDX(newpairs);
				u32 b = RANDIDX(newpairs);
				if (a > b) { u32 z = a; a = b; b = z; }
				if (newpairs[a] != newpairs[b])
				{
					bool dependent = false;
					u8 alo = newpairs[a].lo;
					u8 ahi = newpairs[a].hi;
					u8 blo = newpairs[b].lo;
					u8 bhi = newpairs[b].hi;

					// Pairs should either intersect, or another pair should exist between them that uses
					// one of the same 4 inputs. Otherwise, comparisons can be executed in parallel and
					// swapping them has no effect. 
					if ((blo == alo) || (blo == ahi) || (bhi == alo) || (bhi == ahi))
						dependent = true;
					else
					{
						for (u32 k = a + 1; k < b; k++)
						{
							u8 clo = newpairs[k].lo;
							u8 chi = newpairs[k].hi;
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
						newpairs[b] = z;
						applied = mtype;
					}
				}
			}
			break;
		case 3:
			if (newpairs.size() > 0)  // Replace a pair at a random position with another random pair
			{
				u32 a = RANDIDX(newpairs);
				Pair_t p = RANDELEM(alphabet);
				if (newpairs[a] != p)
				{
					newpairs[a] = p;
					applied = mtype;
				}
			}
			break;
		case 4:
			if (newpairs.size() > 1) // Cross two pairs at random positions in list
			{
				u32 a = RANDIDX(newpairs);
				u32 b = RANDIDX(newpairs);
				u8 alo = newpairs[a].lo;
				u8 ahi = newpairs[a].hi;
				u8 blo = newpairs[b].lo;
				u8 bhi = newpairs[b].hi;

				if ((alo != blo) && (alo != bhi) && (ahi != blo) && (ahi != bhi))
				{
					u32 r2 = mtRand() % 2;
					u32 x = r2 ? bhi : blo;
					u32 y = r2 ? blo : bhi;
					newpairs[a].lo = min(alo, x);
					newpairs[a].hi = max(alo, x);
					newpairs[b].lo = min(ahi, y);
					newpairs[b].hi = max(ahi, y);
					applied = mtype;
				}
			}
			break;
		case 5:
			if (newpairs.size() > 1) // Swap neighbouring intersecting pairs - special case of type r=2.
			{
				u32 a = RANDIDX(newpairs);
				u8 alo = newpairs[a].lo;
				u8 ahi = newpairs[a].hi;
				for (u32 b = a + 1; b < newpairs.size(); b++)
				{
					u8 blo = newpairs[b].lo;
					u8 bhi = newpairs[b].hi;
					if ((blo == alo) || (blo == ahi) || (bhi == alo) || (bhi == ahi))
					{
						if (newpairs[a] != newpairs[b])
						{
							Pair_t z = newpairs[a];
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
			if (newpairs.size() > 0) // Change one half of a pair - special case of type r=3.
			{
				u32 a = RANDIDX(newpairs);
				Pair_t p = newpairs[a];
				Pair_t q;
				do {
					q = RANDELEM(alphabet);
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
	static void checkImproved(const Network_t& nw)
	{
		u32 depth = computeDepth(nw);
		if (conv_hull.improved(nw.size(), depth))
		{
			/* Print only if the sorter is an improved (size,depth) combination */
			if ((Verbosity > 1) || (nw.size() <= ((N * (N - 1u)) / 2u))) // Reduce rubbish listing. Should at least compete with bubble sort before reporting
			{
				printf(" {'N':%u,'L':%llu,'D':%u,'sw':'%s','ESC':%u,'Prefix':%llu,'Postfix':%llu,'nw':", N, nw.size(), depth, VERSION, EscapeRate, prefix.size(), postfix.size());
				printnw(nw);

				printf("\n%s\n", to_string(nw).c_str());
				conv_hull.print();
			}
		}
	}

	/**
	 * General help message
	 */
	static void usage()
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
	if (!cp.parseConfig(argv[1]))
	{
		printf("Error parsing config options.\n");
		return -1;
	}

	if (cp.getInt("RandomSeed") != 0u)
	{
		RandomSeed = cp.getInt("RandomSeed");
		mtRand.seed(RandomSeed);
	}

	N = cp.getInt("Ninputs", 0);
	use_symmetry = (cp.getInt("Symmetric") > 0u);
	force_valid_uphill_step = (cp.getInt("ForceValidUphillStep", 1) > 0);
	EscapeRate = cp.getInt("EscapeRate", 0);
	MaxMutations = cp.getInt("MaxMutations", 1);
	mutation_type_weights[0] = cp.getInt("WeigthRemovePair", 1);
	mutation_type_weights[1] = cp.getInt("WeigthSwapPairs", 1);
	mutation_type_weights[2] = cp.getInt("WeigthReplacePair", 1);
	mutation_type_weights[3] = cp.getInt("WeightCrossPairs", 1);
	mutation_type_weights[4] = cp.getInt("WeightSwapIntersectingPairs", 1);
	mutation_type_weights[5] = cp.getInt("WeightReplaceHalfPair", 1);
	for (u32 n = 0; n < NMUTATIONTYPES; n++)
	{
		for (u32 k = 0; k < mutation_type_weights[n]; k++)
			mutationSelector.push_back(n);
	}
	if (mutationSelector.size() == 0)
	{
		printf("No mutation types selected.\n");
		exit(1);
	}
	PrefixType = cp.getInt("PrefixType", 0);
	FixedPrefix = cp.getNetwork("FixedPrefix");
	GreedyPrefixSize = cp.getInt("GreedyPrefixSize", 0);
	RestartRate = cp.getInt("RestartRate", 0);
	Verbosity = cp.getInt("Verbosity", 1);
	postfix = cp.getNetwork("Postfix");

	if ((N % 2) && use_symmetry)
	{
		if (Verbosity > 0)
		{
			printf("Warning: option 'Symmetric' ignored for odd number of inputs\n");
		}
		use_symmetry = false;
	}

	/* Initialize set of CEs to pick from */
	initalphabet();

	/* Create initial prefix network */
	switch (PrefixType)
	{
	case 1: // Fixed prefix
		prefix = copyValidPairs(FixedPrefix, N);
		break;
	case 2: // Greedy algorithm A 
		fillprefixGreedyA(prefix, GreedyPrefixSize);
		break;
	case 3: // Hybrid prefix
		fillprefixFixedThenGreedyA(prefix, GreedyPrefixSize);
		break;
	default: // No prefix
		prefix.clear();
		break;
	}

	if (Verbosity > 0)
	{
		printf("Prefix size: %llu\n", prefix.size());
	}

	/* Prepare a set of test vectors matching the prefix */
	prepareTestVectorsFromPrefix(prefix);


	for (;;) // Outer loop - restart from here if restart is triggered (only applies if RestartRate!=0)
	{
		pairs = copyValidPairs(cp.getNetwork("InitialNetwork"), N);

		// Produce initial solution, simply by adding random pairs until we found a valid network. In case no postfix is present, we demand that the added pair
		// fixes at least one of the output inversions in the first detected error output vector, so it does at least some useful work to help sorting the outputs.
		// In case there is a postfix network, this check is not implemented.
		for (;;)
		{
			if (use_symmetry)
				symmetricExpansion(N, pairs, se);
			else
				se = pairs;

			appendNetwork(se, postfix);

			SortWord_t failed_output_pattern;

			if (testInitialPairsFromPrefixOutput(se, parallelpatterns_from_prefix, failed_output_pattern))
				break;

			Pair_t p;

			if (postfix.size() == 0) // Empty postfix: find a pattern that fixes an arbitrary inversion in the first failed output
			{
				bool found_useful_ce = false;
				do {
					p = RANDELEM(alphabet);

					if ((((failed_output_pattern >> p.lo) & 1) == 1) && (((failed_output_pattern >> p.hi) & 1) == 0))
						found_useful_ce = true;

					if (use_symmetry)
					{
						if ((((failed_output_pattern >> ((N - 1) - p.hi)) & 1) == 1) && (((failed_output_pattern >> ((N - 1) - p.lo)) & 1) == 0))
							found_useful_ce = true;
					}

				} while (!found_useful_ce);
			}
			else // In case of postfix: just append a random initial pair to the core network, cannot directly determine good candidate from failed output pattern.
			{
				p = RANDELEM(alphabet);
			}

			pairs.push_back(p);
		}

		Network_t totalnw;
		concatNetwork(prefix, se, totalnw);

		if (Verbosity > 1)
		{
			printf("Initial network size: %llu\n", totalnw.size());
		}

		checkImproved(totalnw);

		for (;;) // Program never ends, keep trying to improve, we may restart in the outer loop however.
		{
			if (Verbosity > 2)
			{
				itercount++;
				if (itercount >= iter_next_report)
				{
					clock_t t2 = clock();

					if ((t2 > t1) && (t2 > t0))
					{
						double t = (t2 - t0) / (double)CLOCKS_PER_SEC;
						double dt = (t2 - t1) / (double)CLOCKS_PER_SEC;
						printf("Iteration %llu  t=%.3lf s     %.1lf it/s\n", itercount, t, (iter_next_report - iter_last_report) / dt);
					}

					t1 = t2;
					iter_last_report = iter_next_report;
					iter_next_report += (1 + iter_next_report / 10); // Report about each 10% increase of iteration count, avoid all too frequent output
				}
			}
			/* Determine number of mutations to use in this iteration */
			u32 nmods = 1;

			if (MaxMutations > 1)
			{
				nmods += mtRand() % MaxMutations;
			}

			/* Create a copy of the accepted set of pairs */
			newpairs = pairs;

			/* Apply the mutations */
			u32 modcount = 0;
			while (modcount < nmods)
			{
				u32 r = attemptMutation(newpairs);
				if (r != 0)
				{
					modcount++;
				}
			}

			/* Create a symmetric expansion of the modified pairs (or just a copy if non-symmetric network) */
			if (use_symmetry)
			{
				symmetricExpansion(N, newpairs, se);
			}
			else
			{
				se = newpairs;
			}

			appendNetwork(se, postfix);

			/* Test whether the new postfix network yields a valid sorter when combined with the prefix */
			if ((se.size() > 0) && testpairsFromPrefixOutput(se, parallelpatterns_from_prefix))
			{
				concatNetwork(prefix, se, totalnw);

				/* Accept the new postfix */
				pairs = newpairs;

				checkImproved(totalnw);
			}

			/* With low probability, add another pair random pair at a random place. Attempt to escape from local optimum. */
			if ((EscapeRate > 0) && ((mtRand() % EscapeRate) == 0))
			{
				int a = mtRand() % (pairs.size() + 1); // Random insertion position
				Pair_t p = RANDELEM(alphabet);

				// Determine if the random pair p could be added in the last layer
				bool hit_successor = false;
				for (Network_t::const_iterator it = pairs.begin() + a; it != pairs.end(); it++)
				{
					if ((it->lo == p.lo) || (it->hi == p.lo) || (it->lo == p.hi) || (it->hi == p.hi))
					{
						hit_successor = true;
						break;
					}
				}

				if (force_valid_uphill_step && hit_successor)
				{
					pairs.insert(pairs.begin() + a, pairs[a]); // Prepend duplicate of existing pair right in front of it => Sorter with redundant pair will remain valid
				}
				else
				{
					pairs.insert(pairs.begin() + a, p); // Add random pair at the end of the network
				}
			}

			if ((RestartRate > 0) && ((mtRand() % RestartRate) == 0))
			{
				if (Verbosity > 1)
				{
					printf("Restart.\n");
				}
				switch (PrefixType) // Recompute prefix if not fixed
				{
				case 1: // Fixed prefix - no update: vectors remain the same after restart
					break;
				case 2: // Greedy algorithm A 
					fillprefixGreedyA(prefix, GreedyPrefixSize);
					prepareTestVectorsFromPrefix(prefix);
					break;
				case 3: // Hybrid prefix
					fillprefixFixedThenGreedyA(prefix, GreedyPrefixSize);
					prepareTestVectorsFromPrefix(prefix);
					break;
				default: // No prefix - no update: vectors remain the same after restart
					break;
				}

				break; // Restart using outer loop
			}
		}
	}

	return 0;
}
