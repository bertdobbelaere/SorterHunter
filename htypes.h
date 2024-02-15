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

#include "slimcpplib/long_uint.h"

namespace sh {
	#define INOUT

	namespace {
		template <int N_BITS> struct TypeChooser {};
		template <> struct TypeChooser<8> { using Type = uint8_t; };
		template <> struct TypeChooser<16> { using Type = uint16_t; };
		template <> struct TypeChooser<32> { using Type = uint32_t; };
		template <> struct TypeChooser<64> { using Type = uint64_t; };
		template <> struct TypeChooser<128> { using Type = slim::uint128_t; };
		template <> struct TypeChooser<256> { using Type = slim::uint256_t; };
	}

	constexpr int NMAX = 64;
	constexpr int PARWORDSIZE = 64;
	using SortWord_t = TypeChooser<NMAX>::Type;
	using BPWord_t = TypeChooser<PARWORDSIZE>::Type;

	using u32 = uint32_t ;
	using u8 = uint8_t;

	using ChannelT = uint8_t;


	/**
	 * CE representation
	 */
	struct Pair_t {
		ChannelT lo, hi; ///< low and high line indices connected by the element
		bool operator==(const Pair_t& p) const { return (lo == p.lo) && (hi == p.hi); }
		bool operator!=(const Pair_t& p) const { return (lo != p.lo) || (hi != p.hi); }

		constexpr Pair_t(ChannelT lo, ChannelT hi) : lo(lo), hi(hi) {}
	};

	using Network_t = std::vector<Pair_t>;

	using SinglePatternList_t = std::vector<SortWord_t>;

	using BitParallelList_t = std::vector<BPWord_t>;

}