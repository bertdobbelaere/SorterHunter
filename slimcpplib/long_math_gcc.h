////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Simple Long Integer Math for C++
// version 1.3
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
//
// Copyright (c) 2020-2021 Yury Kalmykov <y_kalmykov@mail.ru>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "long_math.h"

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

#include <x86intrin.h>

namespace slim
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone routines
////////////////////////////////////////////////////////////////////////////////////////////////////

// calculate number of set bits

uint_t popcnt(uint8_t value) noexcept;
uint_t popcnt(uint16_t value) noexcept;
uint_t popcnt(uint32_t value) noexcept;
uint_t popcnt(uint64_t value) noexcept;

// calculate leading zero bits

uint_t nlz(uint8_t value) noexcept;
uint_t nlz(uint16_t value) noexcept;
uint_t nlz(uint32_t value) noexcept;
uint_t nlz(uint64_t value) noexcept;

// add with carry

bool add(uint32_t& value1, uint32_t value2) noexcept;
bool add(uint64_t& value1, uint64_t value2) noexcept;

bool addc(uint32_t& value1, uint32_t value2, bool carry) noexcept;
bool addc(uint64_t& value1, uint64_t value2, bool carry) noexcept;

// subtract with borrow

bool sub(uint32_t& value1, uint32_t value2) noexcept;
bool sub(uint64_t& value1, uint64_t value2) noexcept;

bool subb(uint32_t& value1, uint32_t value2, bool borrow) noexcept;
bool subb(uint64_t& value1, uint64_t value2, bool borrow) noexcept;

// multiply with carry

uint8_t mul(uint8_t& value1, uint8_t value2) noexcept;
uint16_t mul(uint16_t& value1, uint16_t value2) noexcept;
uint32_t mul(uint32_t& value1, uint32_t value2) noexcept;
uint64_t mul(uint64_t& value1, uint64_t value2) noexcept;

uint8_t mulc(uint8_t& value1, uint8_t value2, uint8_t carry) noexcept;
uint16_t mulc(uint16_t& value1, uint16_t value2, uint16_t carry) noexcept;
uint32_t mulc(uint32_t& value1, uint32_t value2, uint32_t carry) noexcept;
uint64_t mulc(uint64_t& value1, uint64_t value2, uint64_t carry) noexcept;



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone routines
////////////////////////////////////////////////////////////////////////////////////////////////////

inline uint_t popcnt(uint8_t value) noexcept
{
    return __builtin_popcount(value);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t popcnt(uint16_t value) noexcept
{
    return __builtin_popcount(value);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t popcnt(uint32_t value) noexcept
{
    return __builtin_popcount(value);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t popcnt(uint64_t value) noexcept
{
    return __builtin_popcount(value);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t nlz(uint8_t value) noexcept
{
    constexpr uint_t offset = bit_count_v<decltype(__builtin_clz(value))> - bit_count_v<uint8_t>;
    return value ? __builtin_clz(value) - offset : bit_count_v<uint8_t>;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t nlz(uint16_t value) noexcept
{
    constexpr uint_t offset = bit_count_v<decltype(__builtin_clz(value))> - bit_count_v<uint16_t>;
    return value ? __builtin_clz(value) - offset : bit_count_v<uint16_t>;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t nlz(uint32_t value) noexcept
{
    return value ? __builtin_clz(value) : bit_count_v<uint32_t>;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t nlz(uint64_t value) noexcept
{
#ifdef __x86_64__
    return value ? __builtin_clzl(value) : bit_count_v<uint64_t>;
#else
    const uint32_t value_hi = static_cast<uint32_t>(half_hi(value));
    const uint32_t value_lo = static_cast<uint32_t>(half_lo(value));

    const uint_t count = nlz(value_hi);
    return count < bit_count_v<uint32_t> ? count : count + nlz(value_lo);
#endif // __x86_64__
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool add(uint32_t& value1, uint32_t value2) noexcept
{
    return __builtin_add_overflow(value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool add(uint64_t& value1, uint64_t value2) noexcept
{
    return __builtin_add_overflow(value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool addc(uint32_t& value1, uint32_t value2, bool carry) noexcept
{
    return __builtin_ia32_addcarryx_u32(static_cast<uint8_t>(carry), value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool addc(uint64_t& value1, uint64_t value2, bool carry) noexcept
{
#ifdef __x86_64__
    unsigned long long result;
    carry = __builtin_ia32_addcarryx_u64(static_cast<uint8_t>(carry), value1, value2, &result);
    value1 = result;

    return carry;
#else
    return addc<uint64_t>(value1, value2, carry);
#endif // __x86_64__
}



#if !defined(_M_X64) && defined(__SSE4_2__)
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size, std::enable_if_t<bit_count_v<type_t> * size == 128, int> = 0>
inline void add(std::array<type_t, size>& value1, const std::array<type_t, size>& value2) noexcept
{
    __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value1.data()));
    const __m128i v2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value2.data()));

    __m128i result = _mm_add_epi64(v1, v2);
    const __m128i sign_mask = _mm_set1_epi64x(0x8000000000000000);
    const __m128i v1_flip = _mm_xor_si128(v1, sign_mask);
    const __m128i result_flip = _mm_xor_si128(result, sign_mask);
    const __m128i cmp = _mm_unpacklo_epi64(_mm_setzero_si128(), _mm_cmpgt_epi64(v1_flip, result_flip));
    result = _mm_sub_epi64(result, cmp);

    _mm_storeu_si128(reinterpret_cast<__m128i*>(value1.data()), result);
}
#endif // !defined(_M_X64) && defined(__SSE4_2__)



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool sub(uint32_t& value1, uint32_t value2) noexcept
{
    return __builtin_sub_overflow(value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool sub(uint64_t& value1, uint64_t value2) noexcept
{
    return __builtin_sub_overflow(value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool subb(uint32_t& value1, uint32_t value2, bool borrow) noexcept
{
#ifdef __clang__
    return __builtin_ia32_subborrow_u32(static_cast<uint8_t>(borrow), value1, value2, &value1);
#else
    return __builtin_ia32_sbb_u32(static_cast<uint8_t>(borrow), value1, value2, &value1);
#endif // __CLANG__
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool subb(uint64_t& value1, uint64_t value2, bool borrow) noexcept
{
#ifdef __x86_64__
    unsigned long long result;
#ifdef __clang__
    borrow = __builtin_ia32_subborrow_u64(static_cast<uint8_t>(borrow), value1, value2, &result);
    value1 = result;
#else
    borrow = __builtin_ia32_sbb_u64(static_cast<uint8_t>(borrow), value1, value2, &result);
    value1 = result;
#endif // __CLANG__
    return borrow;
#else
    return subb<uint64_t>(value1, value2, borrow);
#endif // __x86_64__
}



#if !defined(_M_X64) && defined(__SSE4_2__)
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size, std::enable_if_t<bit_count_v<type_t> * size == 128, int> = 0>
inline void sub(std::array<type_t, size>& value1, const std::array<type_t, size>& value2) noexcept
{
    __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value1.data()));
    const __m128i v2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value2.data()));

    __m128i result = _mm_sub_epi64(v1, v2);
    const __m128i sign_mask = _mm_set1_epi64x(0x8000000000000000);
    const __m128i v1_flip = _mm_xor_si128(v1, sign_mask);
    const __m128i result_flip = _mm_xor_si128(result, sign_mask);
    const __m128i cmp = _mm_unpacklo_epi64(_mm_setzero_si128(), _mm_cmpgt_epi64(result_flip, v1_flip));
    result = _mm_add_epi64(result, cmp);

    _mm_storeu_si128(reinterpret_cast<__m128i*>(value1.data()), result);
}
#endif // !defined(_M_X64) && defined(__SSE4_2__)



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint8_t mul(uint8_t& value1, uint8_t value2) noexcept
{
    const uint16_t result = uint16_t(value1) * uint16_t(value2);
    value1 = static_cast<uint8_t>(result);

    return static_cast<uint8_t>(half_hi(result));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint16_t mul(uint16_t& value1, uint16_t value2) noexcept
{
    const uint32_t result = uint32_t(value1) * uint32_t(value2);
    value1 = static_cast<uint16_t>(result);
    return static_cast<uint16_t>(half_hi(result));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint32_t mul(uint32_t& value1, uint32_t value2) noexcept
{
    const uint64_t result = uint64_t(value1) * uint64_t(value2);
    value1 = static_cast<uint32_t>(result);

    return static_cast<uint32_t>(half_hi(result));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint64_t mul(uint64_t& value1, uint64_t value2) noexcept
{
#ifdef __x86_64__
    const unsigned __int128 result = static_cast<unsigned __int128>(value1) * value2;
    value1 = static_cast<uint64_t>(result & 0xffffffffffffffff);
    return static_cast<uint64_t>(result >> 64);
#else
    return mul<uint64_t>(value1, value2);
#endif // __x86_64__
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint8_t mulc(uint8_t& value1, uint8_t value2, uint8_t carry) noexcept
{
    const uint16_t result = uint16_t(value1) * uint16_t(value2) + uint16_t(carry);
    value1 = static_cast<uint8_t>(result);

    return static_cast<uint8_t>(half_hi(result));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint16_t mulc(uint16_t& value1, uint16_t value2, uint16_t carry) noexcept
{
    const uint32_t result = uint32_t(value1) * uint32_t(value2) + uint32_t(carry);
    value1 = static_cast<uint16_t>(result);
    return static_cast<uint16_t>(half_hi(result));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint32_t mulc(uint32_t& value1, uint32_t value2, uint32_t carry) noexcept
{
    const uint64_t result = uint64_t(value1) * uint64_t(value2) + uint64_t(carry);
    value1 = static_cast<uint32_t>(result);

    return static_cast<uint32_t>(half_hi(result));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint64_t mulc(uint64_t& value1, uint64_t value2, uint64_t carry) noexcept
{
#ifdef __x86_64__
    const unsigned __int128 result = static_cast<unsigned __int128>(value1) * value2;
    uint64_t result_lo = static_cast<uint64_t>(result & 0xffffffffffffffff);
    uint64_t result_hi = static_cast<uint64_t>(result >> 64);

    result_hi += add(result_lo, carry);
    value1 = result_lo;

    return result_hi;
#else
    return mulc<uint64_t>(value1, value2, carry);
#endif // __x86_64__
}

} // namespace slim

#endif // defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

////////////////////////////////////////////////////////////////////////////////////////////////////
// End of long_math_gcc.h
////////////////////////////////////////////////////////////////////////////////////////////////////
