# SorterHunter
An evolutionary approach to find small and low latency sorting networks

## About
SorterHunter is a C++ program used to find well performing sorting networks.
For a number of input sizes, the method used by the program succeeded in reducing the upper bound of S(n) compared to previous work.

n  | S(n) previous upper bound | S(n) new upper bound
--  | ------------------------- | --------------------
19 | 86 | 85
20 | 92 | 91
21 | 102 | 101
23 | 118 | 116
24 | 123 | 121
28 | 156 | 155

This while keeping also the depth (number of parallel operation steps) low. I committed the program to the public domain as inspiration source for further improvements on the subject of sorting networks.
A list of best performing networks for input sizes up to 32 (as far as known by the author) can be found [here](http://users.telenet.be/bertdobbelaere/SorterHunter/sorting_networks.html)

## The program
The program is very straightforward to build (just "make") on a Linux machine. It expects *one* command line argument, which is the name of the configuration file to use an example config file is bundled with the sources. Once started it will go into an endless optimisation loop, printing out any improvements it found to the previous results it reported

## Working principles
*coming up soon*
