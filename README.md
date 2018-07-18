# SorterHunter
An evolutionary approach to find small and low latency sorting networks

## About
SorterHunter is a C++ program used to find well performing sorting networks.
For a number of input sizes, the method used by the program succeeded in reducing the upper bound of S(n) compared to previous work. (Definition: see "The Art of Computer Programming Vol. 3 - Sorting and Searching" - D.E. Knuth, 1998)

n  | S(n) previous upper bound | S(n) new upper bound
--  | ------------------------- | --------------------
19 | 86 (*) | 85
20 | 92 (*) | 91
21 | 102 (*) | 100
22 | 108 (*) | 107
23 | 118 (*) | 115
24 | 123 (*) | 120
25 | 133 (*) | 132
26 | 140 (*) | 139
28 | 156 (*) | 155

Above results were obtained while keeping also the depth (i.e. number of parallel operation steps) low. I committed the program to the public domain as inspiration source for further improvements on the subject of sorting networks.
A list of best performing networks for input sizes up to 32 (as far as known by the author) can be found [here](http://users.telenet.be/bertdobbelaere/SorterHunter/sorting_networks.html)

(*) Ref: "Using Symmetry and Evolutionary Search to Minimize Sorting Networks", Valsalam&Miikkulainen, 2013.

## The program
The program is very straightforward to build (just "make") on a Linux machine. It expects *one* command line argument, which is the name of the configuration file to use. An example config file is bundled with the sources. Once initialised the program will enter an endless optimisation loop, printing out any improvements it found to previous results it reported

## Working principles
After the config file is read, the program works as follows:
#### Determining the prefix to use
During the entire run time of the (current) program, the same prefix is used. There are three options:
1. Empty prefix - all CEs (compare-exchange nodes) participate in the evolutionary optimisation
2. Fixed prefix - provided by the user in the config file
3. Greedy prefix - in a simple algorithm I call "greedy algorithm A", nodes are added one by one starting from an empty network and up to the specified prefix size is reached. For each step, a CE is randomly chosen among those that result in the minimum number of remaining (partially sorted) patterns after the prefix network (assuming application of the "*zero-one principle*". I expect that still much can be improved on the prefix selection algorithm
#### Preparing the test vectors
With an empty prefix, testing a network with N inputs by blind application of the "*zero-one principle*" would require 2^N input vectors of N elements to be sent through the candidate sorter. While CE nodes are added in the prefix, the set of possible output patterns of the prefix gradually decreases. To test the remainder of the network, only the possible output patterns of the prefix network need to be considered as input vectors. As the prefix is known before we will iteratively try to improve the network, the test vectors can be enumerated in a fixed list. Test vectors are placed in pseudorandom order. The reason for this is that in this way we increase the chances that an invalid candidate sorter (the vast majority!) will be rejected early in the test process, assuming not all test vectors are applied at once. Further, using 0's and 1's as input, the behaviour of a CE can simply be modelled as a combination of a single "and" and "or" gate. As the order of comparisons and exchanges for a sorting network is fixed, we use a bit-parallel approach to sort multiple test vectors in parallel (64 on a 64 bit machine), obtaining a considerable speed increase.
#### Determining an initial candidate sorter
Perhaps the weakest part of the algorithm today: the initial candidate is obtained by randomly adding CEs to the network until a valid sorter is obtained. Although there are many obvious ways to create a better initial network, it's hard to guarantee that a "good" initial network won't bias the solutions obtained through evolution. Work to do.
#### The never ending loop (that is: until Ctrl+C)
If you support the evolution theory, you also agree that it is a slow process. This program produces output in far less than a billion years, but it could easily run for that long.
The main iteration loop will take a copy of the latest accepted network and apply one or more mutations to it. Six mutations are currently part of the program, and their relative probabilities can be tuned via the config file.
The "regular" mutations are:
1. Removal of random CE from the variable (i.e. "postfix") list: "downhill" step
2. Swap two CEs at random positions in the list
3. Replace a CE at a random position with another random CE
4. Cross the I/O lines of two CEs at random positions in list
5. Swap neighbouring CEs that share a connection
6. Change 1 connection of a random CE

The mutations are accepted on condition that the mutant sorting network is valid. This is determined by applying the test vectors mentioned before.
An infrequent modification is the insertion of a random CE at a random position in the list. The "uphill" change allows the algorithm to escape from local minima. 
#### Reporting results
Users of sorting networks are interested in low size, low depth but maybe also in trade-offs between them. To satisfy all, the program uses an *orthogonal convex hull* approach that keeps track of all (size,depth) pairs that are not outperformed by other networks.
