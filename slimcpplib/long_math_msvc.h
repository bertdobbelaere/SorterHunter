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

#if defined(_MSC_VER)

#include <intrin.h>

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

// shift bits to left

uint64_t shl2(uint64_t value_hi, uint64_t value_lo, uint_t shift) noexcept;

// shift bits to right

uint64_t shr2(uint64_t value_hi, uint64_t value_lo, uint_t shift) noexcept;

// add with carry

bool add(uint8_t& value1, uint8_t value2) noexcept;
bool add(uint16_t& value1, uint16_t value2) noexcept;
bool add(uint32_t& value1, uint32_t value2) noexcept;
bool add(uint64_t& value1, uint64_t value2) noexcept;

bool addc(uint8_t& value1, uint8_t value2, bool carry) noexcept;
bool addc(uint16_t& value1, uint16_t value2, bool carry) noexcept;
bool addc(uint32_t& value1, uint32_t value2, bool carry) noexcept;
bool addc(uint64_t& value1, uint64_t value2, bool carry) noexcept;

// subtract with borrow

bool sub(uint8_t& value1, uint8_t value2) noexcept;
bool sub(uint16_t& value1, uint16_t value2) noexcept;
bool sub(uint32_t& value1, uint32_t value2) noexcept;
bool sub(uint64_t& value1, uint64_t value2) noexcept;

bool subb(uint8_t& value1, uint8_t value2, bool borrow) noexcept;
bool subb(uint16_t& value1, uint16_t value2, bool borrow) noexcept;
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
    return __popcnt16(value);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t popcnt(uint16_t value) noexcept
{
    return __popcnt16(value);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t popcnt(uint32_t value) noexcept
{
    return __popcnt(value);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t popcnt(uint64_t value) noexcept
{
#ifdef _M_X64
    return __popcnt64(value);
#else
    return __popcnt(static_cast<uint32_t>(half_hi(value))) + __popcnt(static_cast<uint32_t>(half_lo(value)));
#endif // _M_X64
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t nlz(uint8_t value) noexcept
{
    unsigned long result = 0;
    result = static_cast<uint8_t>(_BitScanReverse(&result, value) ? (31 - result) - 24 : bit_count_v<uint8_t>);

    return static_cast<uint8_t>(result);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t nlz(uint16_t value) noexcept
{
    unsigned long result = 0;
    result = static_cast<uint16_t>(_BitScanReverse(&result, value) ? (31 - result) - 16 : bit_count_v<uint16_t>);

    return static_cast<uint16_t>(result);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t nlz(uint32_t value) noexcept
{
    unsigned long result = 0;
    result = static_cast<uint32_t>(_BitScanReverse(&result, value) ? 31 - result : bit_count_v<uint32_t>);

    return static_cast<uint32_t>(result);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint_t nlz(uint64_t value) noexcept
{
#ifdef _M_X64
    unsigned long result = 0;
    result = _BitScanReverse64(&result, value) ? 63 - result : static_cast<long>(bit_count_v<uint64_t>);

    return static_cast<uint64_t>(result);
#else
    const uint32_t value_hi = static_cast<uint32_t>(half_hi(value));
    const uint32_t value_lo = static_cast<uint32_t>(half_lo(value));

    const uint_t count = nlz(value_hi);
    return count < bit_count_v<uint32_t> ? count : count + nlz(value_lo);
#endif // _M_X64
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint64_t shl2(uint64_t value_hi, uint64_t value_lo, uint_t shift) noexcept
{
#ifdef _M_X64
    if (shift < bit_count_v<uint64_t>)
        return __shiftleft128(value_lo, value_hi, static_cast<uint8_t>(shift));
    else
        return value_lo << (shift - bit_count_v<uint64_t>);
#else
    return shl2<uint64_t>(value_hi, value_lo, shift);
#endif // _M_X64
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint64_t shr2(uint64_t value_hi, uint64_t value_lo, uint_t shift) noexcept
{
#ifdef _M_X64
    if (shift < bit_count_v<uint64_t>)
        return __shiftright128(value_lo, value_hi, static_cast<uint8_t>(shift));
    else
        return value_hi >> (shift - bit_count_v<uint64_t>);
#else
    return shr2<uint64_t>(value_hi, value_lo, shift);
#endif // _M_X64
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool add(uint8_t& value1, uint8_t value2) noexcept
{
    return _addcarry_u8(0, value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool add(uint16_t& value1, uint16_t value2) noexcept
{
    return _addcarry_u16(0, value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool add(uint32_t& value1, uint32_t value2) noexcept
{
    return _addcarry_u32(0, value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool add(uint64_t& value1, uint64_t value2) noexcept
{
#ifdef _M_X64
    return _addcarry_u64(0, value1, value2, &value1);
#else
    return add<uint64_t>(value1, value2);
#endif // _M_X64
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool addc(uint8_t& value1, uint8_t value2, bool carry) noexcept
{
    return _addcarry_u8(carry, value1, value2, &value1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool addc(uint16_t& value1, uint16_t value2, bool carry) noexcept
{
    return _addcarry_u16(carry, value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool addc(uint32_t& value1, uint32_t value2, bool carry) noexcept
{
    return _addcarry_u32(carry, value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool addc(uint64_t& value1, uint64_t value2, bool carry) noexcept
{
#ifdef _M_X64
    return _addcarry_u64(carry, value1, value2, &value1);
#else
    return addc<uint64_t>(value1, value2, carry);
#endif // _M_X64
}



#ifndef _M_X64
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
#endif // _M_X64


////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool sub(uint8_t& value1, uint8_t value2) noexcept
{
    return _subborrow_u8(0, value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool sub(uint16_t& value1, uint16_t value2) noexcept
{
    return _subborrow_u16(0, value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool sub(uint32_t& value1, uint32_t value2) noexcept
{
    return _subborrow_u32(0, value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool sub(uint64_t& value1, uint64_t value2) noexcept
{
#ifdef _M_X64
    return _subborrow_u64(0, value1, value2, &value1);
#else
    return sub<uint64_t>(value1, value2);
#endif // _M_X64
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool subb(uint8_t& value1, uint8_t value2, bool borrow) noexcept
{
    return _subborrow_u8(borrow, value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool subb(uint16_t& value1, uint16_t value2, bool borrow) noexcept
{
    return _subborrow_u16(static_cast<uint8_t>(borrow), value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool subb(uint32_t& value1, uint32_t value2, bool borrow) noexcept
{
    return _subborrow_u32(static_cast<uint8_t>(borrow), value1, value2, &value1);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool subb(uint64_t& value1, uint64_t value2, bool borrow) noexcept
{
#ifdef _M_X64
    return _subborrow_u64(static_cast<uint8_t>(borrow), value1, value2, &value1);
#else
    return subb<uint64_t>(value1, value2, borrow);
#endif // _M_X64
}



#ifndef _M_X64
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
#endif // _M_X64



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
#ifdef _M_X64
    uint64_t result_hi;
    value1 = _umul128(value1, value2, &result_hi);
    return result_hi;
#else
    return mul<uint64_t>(value1, value2);
#endif // _M_X64
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
#ifdef _M_X64
    uint64_t result_hi;
    value1 = _umul128(value1, value2, &result_hi);
    return result_hi + add(value1, carry);
#else
    return mulc<uint64_t>(value1, value2, carry);
#endif // _M_X64
}

} // namespace slim

#endif // defined(_MSC_VER)

////////////////////////////////////////////////////////////////////////////////////////////////////
// End of long_math_msvc.h
////////////////////////////////////////////////////////////////////////////////////////////////////
