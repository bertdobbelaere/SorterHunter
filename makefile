# makefile for SorterHunter program. Kept very simple.
# Tested with g++ 5.4.0 and clang++ 3.8.0

CXX=g++
CXXFLAGS= -O4 -Wall
RM=rm -f

all: SorterHunter

SorterHunter: prefix_processor.cpp hutils.cpp SorterHunter.cpp ConfigParser.cpp htypes.h
	$(CXX) $(CXXFLAGS) -o $@ prefix_processor.cpp hutils.cpp SorterHunter.cpp ConfigParser.cpp

clean:
	-$(RM) SorterHunter

