/**
 * @file htypes.h
 * @brief Basic data types for SorterHunter program
 * @author Bert Dobbelaere bert.o.dobbelaere[at]telenet[dot]be
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

#pragma once
#include <stdint.h>
#include <vector>

namespace sh {
#define NMAX (64)
#define PARWORDSIZE (64)

	using std::size_t;

	typedef uint64_t SortWord_t; ///< Needs to contain at least NMAX bits
	typedef uint64_t BPWord_t;   ///< Bit-parallel operation word, needs to contain at least PARWORDSIZE bits
	typedef uint32_t u32;
	typedef uint8_t u8;

	/**
	 * CE representation
	 */
	struct Pair_t {
		u8 lo, hi; ///< low and high line indices connected by the element
		bool operator==(const Pair_t& p) const { return (lo == p.lo) && (hi == p.hi); }
		bool operator!=(const Pair_t& p) const { return (lo != p.lo) || (hi != p.hi); }
	};

	typedef std::vector<Pair_t> Network_t;

	typedef std::vector<SortWord_t> SinglePatternList_t;

	typedef std::vector<BPWord_t> BitParallelList_t;

}