#pragma once

#include "ConfigParser.h"

namespace sh::state {

	///< Overall verbosity level: 0:minimal, 1:moderate, 2:high, >2:debug        
	int Verbosity = 1;

	SortWord_t all_n_inputs_mask; ///< ninputs lowest bit to be set

	ConfigParser cp;

	bool use_symmetry = true; ///< Treat sorting network as symmetric or not
	bool force_valid_uphill_step = true; ///< "Uphill" step inserts duplicate CE if not in final layer.
	//int N = 0;                   ///< Problem dimension, i.e. number of inputs to be sorted
	int EscapeRate = 0;         ///< Adds a random pair (and its symmetric complement for symmetric networks) every x iterations
	int MaxMutations = 1;       ///< Maximum allowed number of mutations in evolution step
	int PrefixType = 0;         ///< Type of prefix used (0=none, 1=fixed, 2=greedy)
	Network_t FixedPrefix;    ///< Fixed prefix to use (if applicable)
	Network_t InitialNetwork; ///< Initial starting point of network
	int GreedyPrefixSize = 0;   ///< Size of greedy prefix (if applicable)
	OCH_t conv_hull;          ///< "Best performing" network list found so far
	uint64_t RandomSeed;      ///< Random seed
	uint64_t RestartRate;     ///< Return to initial conditions each ... iterations (0=never)

	// Working set of pairs in the sorting network
	Network_t pairs; ///< Current core network: evolving section between prefix and postfix. For symmetric networks, mirrored pair (if not coinciding) is omitted.
	Network_t se; ///< Symmetrical expansion of current network
	Network_t newpairs;
	Network_t prefix; ///< Fixed, greedy, hybrid or empty prefix network
	Network_t postfix; ///< Fixed or empty postfix network

	// Set of all possible pairs, unique taking into account symmetric complements
	Network_t alphabet; ///< "Alphabet" of possible CEs defined by their vertical positions.

	constexpr int NMUTATIONTYPES = 6; ///< Number of different mutation types
	int mutation_type_weights[NMUTATIONTYPES]; ///< Relative probabilities for each mutation type
	std::vector<int> mutationSelector; ///< Helper variable to quickly pick a mutation with the requested probability.

	// Random generation
	std::random_device rd;
	RandGen_t mtRand(rd()); // Mersenne twister is a rather good PRNG. Seeding quality varies between systems, but OK ; this is no crypto application.
}