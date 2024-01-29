#include <algorithm>
#include <array>

#include "htypes.h"

#include "swap_sorted_patterns.h"

namespace sh {


	/**
	 * Helper class to efficiently compute partially ordered pattern sets.
	 * The inputs of the network are grouped together in clusters that have been connected by CEs
	 * Initial clusters contain just one input (no CEs added yet).
	 * Each cluster has a set of output patterns that it leaves behind. The global set of output patterns
	 * is at each time defined by the bitwise "ORed" combinations of the outputs that all clusters produce together.
	 * While adding CEs to the network, clusters are combined into larger clusters with a shrinking total number of
	 * output patterns. If the CE is added that combines the last two clusters, only one cluster will remain.
	 * If after that sufficient new CEs are added, only ninputs+1 patterns will remain, meaning that the network is fully sorted.
	 */
	class ClusterGroup {
	private:

		SinglePatternList_t* pattern_lists; ///< Sorted list of output patterns from each cluster of lines
		SortWord_t* masks; ///< Masks for each cluster marking the applicable lines for each cluster
		u8* clusterAlloc; ///< Allocations of lines to clusters
		int ninputs; ///< Total number of inputs (and outputs) of the network

	public:

		// Initialize an empty cluster group
		ClusterGroup(int ninputs) : ninputs(ninputs) {
			pattern_lists = new SinglePatternList_t[ninputs];
			masks = new SortWord_t[ninputs];
			clusterAlloc = new u8[ninputs];
			clear();
		}

		// Copy constructor
		ClusterGroup(const ClusterGroup& cg)
		{
			ninputs = cg.ninputs;
			pattern_lists = new SinglePatternList_t[ninputs];
			masks = new SortWord_t[ninputs];
			clusterAlloc = new u8[ninputs];
			for (int k = 0; k < ninputs; k++)
			{
				pattern_lists[k] = cg.pattern_lists[k];
				masks[k] = cg.masks[k];
				clusterAlloc[k] = cg.clusterAlloc[k];
			}
		}

		// Assignment of cluster groups to each other
		const ClusterGroup& operator=(const ClusterGroup& cg)
		{
			ninputs = cg.ninputs;
			for (int k = 0; k < ninputs; k++)
			{
				pattern_lists[k] = cg.pattern_lists[k];
				masks[k] = cg.masks[k];
				clusterAlloc[k] = cg.clusterAlloc[k];
			}
			return *this;
		}

		/**
		* Set initial state:
		* each input corresponds one to one with its own cluster. The cluster has two possible output patterns:
		* the all 0 pattern, and a single 1 bit at the bit position of the corresponding input.
		*/
		void clear()
		{
			for (int k = 0; k < ninputs; k++)
			{
				clusterAlloc[k] = k;
				masks[k] = static_cast<SortWord_t>(1) << k;
				pattern_lists[k].clear();
				pattern_lists[k].push_back(0);
				pattern_lists[k].push_back(static_cast<SortWord_t>(1) << k);
			}
		}

		/**
		* Reduces the number of patterns represented by appending a single CE to the network.
		* If the CE's lines belong to different clusters, the clusters are merged first.
		* @param p CE represented by its input/output lines
		*/
		void preSort(const Pair_t& p)
		{
			const int ci_idx = static_cast<int>(clusterAlloc[p.lo]);
			const int cj_idx = static_cast<int>(clusterAlloc[p.hi]);

			if (ci_idx != cj_idx)
			{
				combine(ci_idx, cj_idx);
			}
			swap_sorted_patterns(pattern_lists[ci_idx], p);
		}

		/**
		* Compute the list of output patterns that can leave the network composed of all
		* clusters remaining. This is done by "ORing" together output combinations of all remaining clusters.
		* @param patterns [OUT] pattern list created (not lexigraphically sorted)
		*/
		void computeOutputs(INOUT SinglePatternList_t& patterns) const
		{
			std::array<SinglePatternList_t*, NMAX> pLists;
			int n_to_combine = 0;

			for (int k = 0; k < ninputs; k++)
			{
				if (masks[k] != 0) {
					pLists[n_to_combine++] = &pattern_lists[k];
				}
			}

			assert(n_to_combine > 0);

			int level = 0;
			
			std::array<size_t, NMAX> indices;
			std::array<SortWord_t, NMAX> outmasks;
			indices.fill(0);
			outmasks.fill(0);
			patterns.clear();

			while (level >= 0)
			{
				if (indices[level] < pLists[level]->size())
				{
					if (level == 0) {
						outmasks[level] = (*pLists[level])[indices[level]];
					}
					else {
						outmasks[level] = outmasks[level - 1] | (*pLists[level])[indices[level]];
					}
					if (level < (n_to_combine - 1))
					{
						indices[level + 1] = 0;
						indices[level]++;
						level++;
					}
					else
					{
						patterns.push_back(outmasks[level]);
						indices[level]++;
					}
				}
				else
				{
					level--;
				}
			}
		}

		/**
		* Compute number of output patterns that would be produced by call to computeOutputs
		*/
		SortWord_t outputSize() const
		{
			SortWord_t prod = 1;

			for (int k = 0; k < ninputs; k++)
			{
				if (masks[k] != 0) {
					prod *= static_cast<SortWord_t>(pattern_lists[k].size());
				}
			}

#if 1
			if (prod == 0) { // Special case for N=NMAX, dirty hack avoiding wrap-around to 0 of empty network: set size to one less.
				prod -= 1;
			}
#endif

			return prod;
		}

		bool isSameCluster(const Pair_t& p) const
		{
			const auto ci_idx = clusterAlloc[p.lo];
			const auto cj_idx = clusterAlloc[p.hi];
			return ci_idx == cj_idx;
		}

		// Clean up cluster group
		~ClusterGroup()
		{
			delete[] pattern_lists;
			delete[] masks;
			delete[] clusterAlloc;
		}

	private:

		/**
		* Combines two clusters to form a larger cluster.
		* The output pattern list is produced by bitwise "oring" of both original pattern lists
		* @param ci_idx First cluster index (new result cluster)
		* @param cj_idx Second cluster index (will no longer be used)
		*/
		void combine(int ci_idx, int cj_idx)
		{
			SinglePatternList_t& p1 = pattern_lists[ci_idx];
			SinglePatternList_t& p2 = pattern_lists[cj_idx];

			for (int k = 0; k < ninputs; k++) {
				if (clusterAlloc[k] == cj_idx) {
					clusterAlloc[k] = ci_idx; // ci will take over
				}
			}
			masks[ci_idx] |= masks[cj_idx];
			SinglePatternList_t cp;
			/*
			* Combined cluster's output patterns are here simply generated by producing all
			* patterns, first disregarding their final order and sorting them afterwards.
			* At first, I had an algorithm in place that broke the masks into chunks allowing
			* in order generation that had lower theoretical complexity. For practical sizes however
			* a quicksort proved a faster and simpler alternative. (and probably has less bugs :-) )
			*/
			for (int i = 0; i < static_cast<int>(p1.size()); i++) {
				for (int j = 0; j < static_cast<int>(p2.size()); j++) {
					cp.push_back(p1[i] | p2[j]);
				}
			}
			std::sort(cp.begin(), cp.end()); // Keep the new output set sorted
			p1 = cp;
			masks[cj_idx] = 0;
			p2.clear();
		}
	};
}
