#pragma once
#include "hutils.h"

#include "State.h"

namespace sh {

	inline bool isSorted(const SortWord_t& w)
	{
		const SortWord_t w2 = ~w & state::all_n_inputs_mask;
		return (w2 & (w2 + 1)) == 0;
	}


	/**
	 * Replaces a *sorted* list of patterns applied to a network containing a single CE by the sorted list of output patterns of that network.
	 * The sort order is low to high, a pattern represents the binary representation of an input/output state
	 * Restriction to sorted pattern lists allows to compute the output list in linear time.
	 * @param patterns [IN/OUT] Input and output list of patterns, sorted.
	 * @param pair Representation of CE to apply
	 */
	inline void swap_sorted_patterns(INOUT SinglePatternList_t& patterns, const Pair_t& pair)
	{
		SortWord_t p = static_cast<SortWord_t>(1) << pair.lo;
		SortWord_t q = static_cast<SortWord_t>(1) << pair.hi;
		SortWord_t mask = p | q;

		SinglePatternList_t res;

		int idxp = 0;
		int idxnp = 0;
		const int l = static_cast<int>(patterns.size());
		//std::cout << "swap_sorted_patterns; l=" << l << std::endl;


		SortWord_t last = static_cast<SortWord_t>(-1);

		while ((idxp < l) && ((patterns[idxp] & mask) != p)) { idxp++; }
		while ((idxnp < l) && ((patterns[idxnp] & mask) == p)) { idxnp++; }

		while ((idxnp < l) && (idxp < l))
		{
			SortWord_t a = patterns[idxp] ^ mask;
			SortWord_t b = patterns[idxnp];
			if (a < b)
			{
				if (a != last)
				{
					res.push_back(a);
					last = a;
				}
				idxp++;
				while ((idxp < l) && ((patterns[idxp] & mask) != p)) { 
					idxp++; 
				}
			}
			else
			{
				if (a != last)
				{
					res.push_back(b);
					last = b;
				}
				idxnp++;
				while ((idxnp < l) && ((patterns[idxnp] & mask) == p)) { 
					idxnp++; 
				}
			}
		}
		while (idxnp < l)
		{
			res.push_back(patterns[idxnp++]);
		}
		while (idxp < l)
		{
			res.push_back(patterns[idxp++] ^ mask);
		}

		patterns = res;
	}
}
