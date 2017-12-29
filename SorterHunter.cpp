/**
 * @file SorterHunter.cpp 
 * @brief Main file of the sorting network hunter program
 * @author Bert Dobbelaere bert.o.dobbelaere[at]telenet[dot]be
 * Handles symmetrical and asymmetrical sorting networks, with or without predefined or "greedy" prefix.
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


#define VERSION "SorterHunter_V0.1"

#include "htypes.h"
#include "hutils.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <ctime>
#include "ConfigParser.h"

#include "prefix_processor.h"

ConfigParser cp;

bool use_symmetry = true; ///< Treat sorting network as symmetric or not
u8 N=0;                   ///< Problem dimension, i.e. number of inputs to be sorted
u32 EscapeRate=0;         ///< Adds a random pair and its symmetric complement every x iterations
u32 MaxMutations=1;       ///< Maximum allowed number of mutations in evolution step
u32 PrefixType=0;         ///< Type of prefix used (0=none, 1=fixed, 2=greedy)
Network_t FixedPrefix;    ///< Fixed prefix to use (if applicable)
Network_t InitialNetwork; ///< Initial starting point of network
u32 GreedyPrefixSize=0;   ///< Size of greedy prefix (if applicable)
OCH_t conv_hull;          ///< "Best performing" network list found so far
u32 RandomSeed;           ///< Random seed

// Working set of pairs in the sorting network
Network_t pairs;
Network_t se; ///< Symmetrical expansion of current network
Network_t newpairs;
Network_t prefix;

// Set of all possible pairs, unique taking into account symmetric complements
Network_t alphabet;

#define NMUTATIONTYPES 6 ///< Number of different mutation types
u32 mutation_type_weights[NMUTATIONTYPES]; ///< Relative probabilities for each mutation type
std::vector<u8> mutationSelector; ///< Helper variable to quickly pick a mutation with the requested probability.


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
void applyBitParallelSort(BPWord_t data[], const Network_t nw)
{
	for(size_t n=0;n<nw.size();n++)
	{
		u32 i=nw[n].lo;
		u32 j=nw[n].hi;
		BPWord_t iold=data[i];
		data[i]&=data[j];
		data[j]|=iold;
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
void prepareTestVectorsFromPrefix(const Network_t &prefix)
{
	SinglePatternList_t singles;
	computePrefixOutputs(N, prefix, singles);

	std::random_shuffle(singles.begin(),singles.end());

	convertToBitParallel(N, singles, parallelpatterns_from_prefix);
}

/**
 * Initialize "alphabet" of CEs to use
 */

void initalphabet()
{
	alphabet.clear();
	for(u32 i=0;i<(N-1u);i++)
		for(u32 j=i+1;j<N;j++)
		{
			u32 isym=N-1-j;
			u32 jsym=N-1-i;
			
			if(!use_symmetry || (isym>i) || ((isym==i) && (jsym>=j)))
			{
				Pair_t p={(u8)i,(u8)j};
				alphabet.push_back(p);
			}	
		}
}		

/**
 * Test a candidate network complementing the prefix.
 * @param pairs Candidated network
 * @param bpl List of test vectors matching the prefix
 * @return true if prefix+pairs form a valid sorter
 */
bool testpairsFromPrefixOutput(const Network_t pairs, const BitParallelList_t &bpl)
{
	size_t idx=0;
	
	while(idx<bpl.size())
	{
		static BPWord_t data[NMAX];
		BPWord_t accum=0;
		
		for(size_t k=0;k<N;k++)
			data[k] = bpl[idx+k];
		
		applyBitParallelSort(data,pairs);
		
		for(size_t k=0;k<(N-1u);k++)
			accum|= data[k]&~data[k+1]; // Scan for forbidden 1 -> 0 transition
		if(accum!=0ULL)
			return false;
		idx+=N;
	}	
	return true;
}

/**
 * Create a prefix network using greedy algorithm A.
 * @param prefix [OUT] generated prefix
 * @param npairs Number of inputs to the network
 */
void fillprefixGreedyA(Network_t &prefix, u32 npairs )
{
	prefix.clear();
	SortWord_t sizetmp=createGreedyPrefix(N, npairs, use_symmetry, 0, prefix);
	printf("Greedy prefix size %lu, span %lu.\n",prefix.size(),(size_t)sizetmp);
}


/**
 * Attempt to apply a single mutation to the network. If the mutation is a priory rejected, 0 is returned and we will try again.
 * @param newpairs [IN/OUT] candidate network
 * @return Positive integer identifying type of mutation applied, or 0 if none.
 */
u32 attemptMutation(Network_t &newpairs)
{
	u32 applied=0; // Nothing
	u32 mtype=1+RANDELEM(mutationSelector);
	
	switch(mtype)
	{
		case 1:		
			if(newpairs.size()>0)   // Removal of random pair from list
			{
				u32 a=RANDIDX(newpairs);
				newpairs.erase(newpairs.begin() + a);
				applied=mtype;
			}
			break;
		case 2:
			if(newpairs.size()>1) // Swap two pairs at random positions in list
			{
				u32 a=RANDIDX(newpairs);
				u32 b=RANDIDX(newpairs);
				if(a>b){u32 z=a;a=b;b=z;}
				if(newpairs[a]!=newpairs[b])
				{
					bool dependent=false;
					u8 alo=newpairs[a].lo;
					u8 ahi=newpairs[a].hi;
					u8 blo=newpairs[b].lo;
					u8 bhi=newpairs[b].hi;
					
					// Pairs should either intersect, or another pair should exist between them that uses
					// one of the same 4 inputs. Otherwise, comparisons can be executed in parallel and
					// swapping them has no effect. 
					if((blo==alo)||(blo==ahi)||(bhi==alo)||(bhi==ahi))
						dependent=true;
					else
					{
						for(u32 k=a+1;k<b;k++)
						{
							u8 clo=newpairs[k].lo;
							u8 chi=newpairs[k].hi;
							if((clo==alo)||(clo==ahi)||(chi==alo)||(chi==ahi)||
							   (clo==blo)||(clo==bhi)||(chi==blo)||(chi==bhi))
							{
								dependent=true;
								break;
							}
						}
					}
					if(dependent)
					{			
						Pair_t z=newpairs[a];
						newpairs[a]=newpairs[b];
						newpairs[b]=z;
						applied=mtype;
					}
				}
			}
			break;
		case 3:		
			if(newpairs.size()>0)  // Replace a pair at a random position with another random pair
			{
				u32 a=RANDIDX(newpairs);
				Pair_t p=RANDELEM(alphabet);
				if(newpairs[a]!=p)
				{
					newpairs[a]=p;
					applied=mtype;
				}
			}
			break;
		case 4:
			if(newpairs.size()>1) // Cross two pairs at random positions in list
			{
				u32 a=RANDIDX(newpairs);
				u32 b=RANDIDX(newpairs);
				u8 alo=newpairs[a].lo;
				u8 ahi=newpairs[a].hi;
				u8 blo=newpairs[b].lo;
				u8 bhi=newpairs[b].hi;
				
				if ((alo!=blo)&&(alo!=bhi)&&(ahi!=blo)&&(ahi!=bhi))
				{
					u32 r2=rand()%2;
					u32 x = r2 ? bhi : blo;
					u32 y = r2 ? blo : bhi;
					newpairs[a].lo = min(alo, x);
					newpairs[a].hi = max(alo, x);
					newpairs[b].lo = min(ahi, y);
					newpairs[b].hi = max(ahi, y);
					applied=mtype;
				}
			}
			break;
		case 5:
			if(newpairs.size()>1) // Swap neighbouring intersecting pairs - special case of type r=2.
			{
				u32 a=RANDIDX(newpairs);
				u8 alo=newpairs[a].lo;
				u8 ahi=newpairs[a].hi;
				for(u32 b=a+1;b<newpairs.size();b++)
				{
					u8 blo=newpairs[b].lo;
					u8 bhi=newpairs[b].hi;
					if((blo==alo)||(blo==ahi)||(bhi==alo)||(bhi==ahi))
					{
						Pair_t z=newpairs[a];
						newpairs[a]=newpairs[b];
						newpairs[b]=z;
						applied=mtype;
						break;
					}
				}		
			}
			break;
		case 6:
			if(newpairs.size()>0) // Change one half of a pair - special case of type r=3.
			{
				u32 a=RANDIDX(newpairs);
				Pair_t p=newpairs[a];
				Pair_t q;
				do {
					q=RANDELEM(alphabet);
					}while((q.lo!=p.lo)&&(q.hi!=p.lo)&&(q.lo!=p.hi)&&(q.hi!=p.hi));
					
				if(q!=p)
				{
					newpairs[a]=q;
					applied=mtype;
				}
			}
			break;
		default:
			break;
	}
	
	return applied;
}

/**
 * Filter a network to obtain only the pairs that are in range 0..ninputs-1 and properly sorted
 * @param nw input network
 * @param ninputs Number of inputs
 * @return Filtered input network
 */
static const Network_t copyValidPairs(const Network_t &nw, u32 ninputs)
{
	static Network_t result;
	result.clear();
	for(Network_t::const_iterator it=nw.begin();it!=nw.end();it++)
	{
		if((it->hi < ninputs) && (it->lo < it->hi))
		{
			result.push_back( *it);
		}
	}
	return result;
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
	exit(1);
}

/**
 * SorterHunter main routine
 */
int main(int argc, char *argv[])
{
	/* Handle validity of command line options - extremely simple */
	if(argc!=2)
	{
		usage();
		return -1;
	}
	
	/* Process configuration file */
	if(!cp.parseConfig(argv[1]))
	{
		printf("Error parsing config options.\n");
		return -1;
	}
	
	if(cp.getU32("RandomSeed")!=0u)
	{
		RandomSeed=cp.getU32("RandomSeed");
	}
	else
	{
		RandomSeed=clock()+time(NULL);  
	}
	srand(RandomSeed);
	
	N=cp.getU32("Ninputs",0);
	use_symmetry = (cp.getU32("Symmetric")>0u);
	EscapeRate = cp.getU32("EscapeRate",0);
	MaxMutations= cp.getU32("MaxMutations",1);
	mutation_type_weights[0]=cp.getU32("WeigthRemovePair",1);
	mutation_type_weights[1]=cp.getU32("WeigthSwapPairs",1);
	mutation_type_weights[2]=cp.getU32("WeigthReplacePair",1);
	mutation_type_weights[3]=cp.getU32("WeightCrossPairs",1);
	mutation_type_weights[4]=cp.getU32("WeightSwapIntersectingPairs",1);
	mutation_type_weights[5]=cp.getU32("WeightReplaceHalfPair",1);
	for(u32 n=0;n<NMUTATIONTYPES;n++)
	{
		for(u32 k=0;k<mutation_type_weights[n];k++)
			mutationSelector.push_back(n);
	}
	if(mutationSelector.size()==0)
	{
		printf("No mutation types selected.\n");
		exit(1);
	}
	PrefixType=cp.getU32("PrefixType",0);
	FixedPrefix=cp.getNetwork("FixedPrefix");
	GreedyPrefixSize=cp.getU32("GreedyPrefixSize",0);
	pairs=copyValidPairs(cp.getNetwork("InitialNetwork"),N);

	/* Initialize set of CEs to pick from */
	initalphabet();

	/* Create prefix network */
	if(PrefixType==1u) // Fixed
	{
		prefix=copyValidPairs(FixedPrefix, N);
	}
	else if(PrefixType==2u) // Greedy algorithm A 
	{
		fillprefixGreedyA(prefix, GreedyPrefixSize);
	}

	printf("Prefix size: %lu\n",prefix.size());
	
	/* Prepare a set of test vectors maching the prefix */
	prepareTestVectorsFromPrefix(prefix);

	// Produce initial solution, simply by adding random pairs until we found a valid network
	// TODO: Improve this without giving too much direction to the search
	for(;;)
	{
		pairs.push_back( RANDELEM(alphabet) );
		
		if(use_symmetry)
			symmetricExpansion(N, pairs,se);
		else
			se=pairs;
			
		if(testpairsFromPrefixOutput(se, parallelpatterns_from_prefix))
			break;
	}

	Network_t totalnw;
	concat(prefix,se,totalnw);
	u32 depth=computeDepth(totalnw);

	printf("Initial network size: %lu\n",totalnw.size());

	if(conv_hull.improved(totalnw.size(),depth))
	{
		if(totalnw.size()< ((N*(N-1u))/2u)) // Reduce rubbish listing. Should at least compete with bubble sort...
		{
			printf(" {'N':%u,'L':%lu,'D':%u,'sw':'%s','ESC':%u,'FP':%lu,'nw':",N,totalnw.size(),depth,VERSION,EscapeRate,prefix.size());
			printnw(totalnw); 
			conv_hull.print();
		}
	}

	u32 nmods;

	for(;;) // Program never ends, keep trying to improve
	{
		/* Determine number of mutations to use in this iteration */
		if(MaxMutations>1)
		{
			nmods=1u+rand()%MaxMutations;
		}
		else
		{
			nmods=1u;
		}
		
		/* Create a copy of the accepted set of pairs */
		newpairs=pairs;
		
		/* Apply the mutations */
		u32 modcount=0;
		while(modcount<nmods)
		{
			u32 r=attemptMutation(newpairs);
			if(r!=0)
			{
				modcount++;
			}
		}
		
		/* Create a symmetric expansion of the modified pairs (or just a copy if non-symmetric network) */
		if(use_symmetry)
			symmetricExpansion(N, newpairs,se);
		else
			se=newpairs;
		
		/* Test whether the new postfix network yields a valid sorter when combined with the prefix */
		if((se.size()>0) && testpairsFromPrefixOutput(se,parallelpatterns_from_prefix))
		{
			Network_t totalnw;
			concat(prefix,se,totalnw);
			u32 depth=computeDepth(totalnw);
			/* Accept the new postfix */
			pairs=newpairs;

			/* Print only if the sorter is an improved (size,depth) combination */
			if(conv_hull.improved(totalnw.size(),depth))
			{
				if(totalnw.size()< ((N*(N-1u))/2u)) // Reduce rubbish listing. Should at least compete with bubble sort...
				{
					printf(" {'N':%u,'L':%lu,'D':%u,'sw':'%s','ESC':%u,'FP':%lu,'nw':",N,totalnw.size(),depth,VERSION,EscapeRate,prefix.size());
					printnw(totalnw); // We found an improvement: (size,depth) unmatched by previous network.
					conv_hull.print();
				}
			}
		}

        /* With low probability, add another pair random pair at a random place. Attempt to escape from local optimum. */
		if((EscapeRate>0) && ((rand()%EscapeRate)==0))
		{
			int a=rand()%(pairs.size()+1);
			pairs.insert(pairs.begin()+a, RANDELEM(alphabet));
		}
	}

	return 0;
}
