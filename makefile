# makefile for SorterHunter program. Kept very simple.
# Tested with g++ 11.4.0 and clang++ 10.0.0

CXX=g++
CXXFLAGS= -std=c++23 -O3 -march=native
RM=rm -f

all: SorterHunter

SorterHunter: SorterHunter.cpp
	$(CXX) $(CXXFLAGS) -o $@ SorterHunter.cpp

clean:
	-$(RM) SorterHunter

