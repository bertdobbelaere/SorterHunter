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

#include <cassert>
#include <climits>
#include <cstdint>

#include <array>
#include <limits>
#include <optional>
#include <type_traits>

#if !(defined(_MSC_VER) && _MSC_VER >= 1910 && ((defined(_MSVC_LANG) && _MSVC_LANG > 201402)) || (__cplusplus > 201402))
#error "Library SLIMCPP requires a compiler that supports C++ 17!"
#endif

namespace slim
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// type definitions
////////////////////////////////////////////////////////////////////////////////////////////////////

using uint_t = std::uintptr_t;
using int_t = std::intptr_t;



////////////////////////////////////////////////////////////////////////////////////////////////////
// byte_count_v and bit_count_v
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
constexpr uint_t byte_count_v = sizeof(type_t);
template<typename type_t>
constexpr uint_t bit_count_v = byte_count_v<type_t>* CHAR_BIT;



////////////////////////////////////////////////////////////////////////////////////////////////////
// is_unsigned_v and is_signed_v
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
inline constexpr bool is_unsigned_v = std::numeric_limits<type_t>::is_integer && !std::numeric_limits<type_t>::is_signed;
template<typename type_t>
inline constexpr bool is_signed_v = std::numeric_limits<type_t>::is_integer && std::numeric_limits<type_t>::is_signed;



////////////////////////////////////////////////////////////////////////////////////////////////////
// is_unsigned_array_v
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
inline constexpr bool is_unsigned_array_v = false;
template<typename type_t, uint_t size>
inline constexpr bool is_unsigned_array_v<std::array<type_t, size>> = is_unsigned_v<type_t>;
template<typename type_t, uint_t size>
inline constexpr bool is_unsigned_array_v<const std::array<type_t, size>> = is_unsigned_v<type_t>;



////////////////////////////////////////////////////////////////////////////////////////////////////
// make_unsigned_t
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
struct make_unsigned {
    using type = typename std::make_unsigned<type_t>::type;
};
template<typename type_t>
using make_unsigned_t = typename make_unsigned<type_t>::type;



////////////////////////////////////////////////////////////////////////////////////////////////////
// make_signed_t
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
struct make_signed {
    using type = typename std::make_signed<type_t>::type;
};
template<typename type_t>
using make_signed_t = typename make_signed<type_t>::type;



////////////////////////////////////////////////////////////////////////////////////////////////////
// half_t
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
struct half_type {
    using type = type_t;
};
template<typename type_t>
using half_t = typename half_type<type_t>::type;



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone routines
////////////////////////////////////////////////////////////////////////////////////////////////////

// make array from specified array

template<uint_t size_out, typename type_t, uint_t size_in, typename func_t>
constexpr std::array<type_t, size_out> make_array(const std::array<type_t, size_in>& arr, const func_t& func);
template<uint_t size_out, typename type_t, uint_t size_in, typename func_t, uint_t... idx>
constexpr std::array<type_t, size_out> make_array(const std::array<type_t, size_in>& arr, const func_t& func, std::integer_sequence<uint_t, idx...>);

// extract low half of unsigned integer

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t half_lo(type_t value) noexcept;

// extract high half of unsigned integer

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t half_hi(type_t value) noexcept;

// move low half to high

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t half_make_hi(type_t value) noexcept;

// return most significant bit

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool sign(type_t value);

// propagate most significant bit to the right

template<typename type_t, uint_t byte_count = byte_count_v<type_t>, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t pmsbr(type_t value) noexcept;

// calculate number of set bits

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr uint_t popcnt(type_t value) noexcept;

// calculate leading zero bits

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr uint_t nlz(type_t value) noexcept;

// shift bits to the left

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t shl2(type_t value_hi, type_t value_lo, uint_t shift) noexcept;

// shift bits to the right

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t shr2(type_t value_hi, type_t value_lo, uint_t shift) noexcept;

// add with carry

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr bool add(type_t& value1, type_t value2) noexcept;
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr bool addc(type_t& value1, type_t value2, bool carry) noexcept;

// subtract with borrow

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr bool sub(type_t& value1, type_t value2) noexcept;
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr bool subb(type_t& value1, type_t value2, bool borrow) noexcept;

// multiply with carry

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t mul(type_t& value1, type_t value2) noexcept;
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t mulc(type_t& value1, type_t value2, type_t carry) noexcept;

// divide with remainder

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t divr(type_t value1, type_t value2, std::optional<type_t>& remainder) noexcept;
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t divr2(type_t value1_hi, type_t value1_lo, type_t value2, std::optional<type_t>& remainder) noexcept;



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone routines
////////////////////////////////////////////////////////////////////////////////////////////////////

template<uint_t size_out, typename type_t, uint_t size_in, typename func_t>
constexpr std::array<type_t, size_out> make_array(const std::array<type_t, size_in>& arr, const func_t& func)
{
    constexpr uint_t size = std::min(size_in, size_out);
    return make_array<size_out>(arr, func, std::make_integer_sequence<uint_t, size>{});
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<uint_t size_out, typename type_t, uint_t size_in, typename func_t, uint_t... idx>
constexpr std::array<type_t, size_out> make_array(const std::array<type_t, size_in>& arr, const func_t& func, std::integer_sequence<uint_t, idx...>)
{
    return std::array<type_t, size_out>{ (func(arr, idx))... };
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t half_lo(type_t value) noexcept
{
    return value & (type_t(~type_t(0)) >> (bit_count_v<type_t> / 2));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t half_hi(type_t value) noexcept
{
    return value >> (bit_count_v<type_t> / 2);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t half_make_hi(type_t value) noexcept
{
    return value << (bit_count_v<type_t> / 2);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int>>
constexpr bool sign(type_t value)
{
    return make_signed_t<type_t>(value) < 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t byte_count, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t pmsbr(type_t value) noexcept
{
    if constexpr (byte_count == 1) {

        value |= (value >> 1);
        value |= (value >> 2);
        value |= (value >> 4);

    } else {

        value = pmsbr<type_t, byte_count / 2>(value);
        value |= (value >> (4 * byte_count));
    }

    return value;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t bit_count = bit_count_v<type_t>, uint_t mask_count = bit_count_v<type_t> / bit_count, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t popcnt_msk() noexcept
{
    if constexpr (mask_count == 1) {

        return pmsbr<type_t>(type_t(1) << (bit_count / 2 - 1));

    } else {

        constexpr type_t mask = popcnt_msk<type_t, bit_count, mask_count - 1>();
        return mask | (mask << (bit_count));
    }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t byte_count = byte_count_v<type_t>, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t popcnt_impl(type_t value) noexcept
{
    if constexpr (byte_count <= 4) {

        constexpr type_t mask2 = popcnt_msk<type_t, 2>();
        constexpr type_t mask4 = popcnt_msk<type_t, 4>();
        constexpr type_t mask8 = popcnt_msk<type_t, 8>();

        value = value - ((value >> 1) & mask2);
        value = (value & mask4) + ((value >> 2) & mask4);
        value = (value + (value >> 4)) & mask8;

        if constexpr (byte_count >= 2)
            value = value + (value >> 8);
        if constexpr (byte_count >= 3)
            value = value + (value >> 16);

    } else {

        value = popcnt_impl<type_t, byte_count / 2>(value);
        value = value + (value >> 4 * byte_count);
    }

    return value;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr uint_t popcnt(type_t value) noexcept
{
    return static_cast<uint_t>(popcnt_impl<type_t>(value) & ((bit_count_v<type_t> << 2) - 1));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr uint_t nlz(type_t value) noexcept
{
    return bit_count_v<type_t> - popcnt(pmsbr(value));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t shl2(type_t value_hi, type_t value_lo, uint_t shift) noexcept
{
    const type_t result_lo = value_lo;
    type_t result_hi = value_hi;

    if (shift > 0 && shift < bit_count_v<type_t> * 2) {

        if (shift < bit_count_v<type_t>)
            result_hi = (result_hi << shift) | (result_lo >> (bit_count_v<type_t> - shift));
        else
            result_hi = value_lo << (shift - bit_count_v<type_t>);
    }

    return result_hi;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t shr2(type_t value_hi, type_t value_lo, uint_t shift) noexcept
{
    type_t result_lo = value_lo;
    const type_t result_hi = value_hi;

    if (shift > 0 && shift < bit_count_v<type_t> * 2) {

        if (shift < bit_count_v<type_t>)
            result_lo = (result_lo >> shift) | (result_hi << (bit_count_v<type_t> - shift));
        else
            result_lo = value_hi >> (shift - bit_count_v<type_t>);
    }

    return result_lo;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr bool add(type_t& value1, type_t value2) noexcept
{
    value1 += value2;
    return value1 < value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr bool addc(type_t& value1, type_t value2, bool carry) noexcept
{
    value1 += value2;
    bool carry_new = value1 < value2;
    value1 += carry;
    carry_new = carry_new || (value1 < type_t(carry));

    return carry_new;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr bool sub(type_t& value1, type_t value2) noexcept
{
    const type_t tmp = value1;
    value1 -= value2;
    return value1 > tmp;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr bool subb(type_t& value1, type_t value2, bool borrow) noexcept
{
    type_t tmp = value1;
    value1 -= value2;
    bool borrow_new = value1 > tmp;
    tmp = value1;
    value1 -= borrow;
    borrow_new = borrow_new || (value1 > tmp);

    return borrow_new;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t mul(type_t& value1, type_t value2) noexcept
{
    const type_t value1_lo = half_lo(value1);
    const type_t value1_hi = half_hi(value1);
    const type_t value2_lo = half_lo(value2);
    const type_t value2_hi = half_hi(value2);

    const type_t t0 = value1_lo * value2_lo;
    const type_t t1 = value1_hi * value2_lo + half_hi(t0);
    const type_t t2 = value1_lo * value2_hi + half_lo(t1);
    const type_t t3 = value1_hi * value2_hi + half_hi(t2);

    const type_t result_lo = half_make_hi(half_lo(t2)) + half_lo(t0);
    const type_t result_hi = t3 + half_hi(t1);

    value1 = result_lo;

    return result_hi;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t mulc(type_t& value1, type_t value2, type_t carry) noexcept
{
    const type_t value1_lo = half_lo(value1);
    const type_t value1_hi = half_hi(value1);
    const type_t value2_lo = half_lo(value2);
    const type_t value2_hi = half_hi(value2);

    const type_t t0 = value1_lo * value2_lo;
    const type_t t1 = value1_hi * value2_lo + half_hi(t0);
    const type_t t2 = value1_lo * value2_hi + half_lo(t1);
    const type_t t3 = value1_hi * value2_hi + half_hi(t2);

    type_t result_lo = half_make_hi(half_lo(t2)) + half_lo(t0);
    type_t result_hi = t3 + half_hi(t1);

    result_hi += add(result_lo, carry);
    value1 = result_lo;

    return result_hi;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t divr(type_t value1, type_t value2, std::optional<type_t>& remainder) noexcept
{
    type_t quotient = value1 / value2;

    if (remainder)
        remainder = static_cast<type_t>(value1 - quotient * value2);

    return quotient;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t divr2(type_t value1_hi, type_t value1_lo, type_t value2, std::optional<type_t>& remainder) noexcept
{
    if (value2 != 0 && value1_hi >= value2) {

        if (remainder)
            remainder = type_t(~type_t(0));

        return type_t(~type_t(0));
    }

    const uint_t shift = nlz(value2);
    const type_t svalue2 = value2 << shift;

    const type_t nvalue2_hi = half_hi(svalue2);
    const type_t nvalue2_lo = half_lo(svalue2);

    const type_t nvalue1_32 = shl2(value1_hi, value1_lo, shift);
    const type_t nvalue1_10 = value1_lo << shift;

    const type_t nvalue1_hi = half_hi(nvalue1_10);
    const type_t nvalue1_lo = half_lo(nvalue1_10);

    std::optional<type_t> remainder_hi = type_t();
    type_t quotient_hi = divr(nvalue1_32, nvalue2_hi, remainder_hi);

    const type_t t1 = quotient_hi * nvalue2_lo;
    const type_t t2 = half_make_hi(*remainder_hi) | nvalue1_hi;

    if (t1 > t2) {

        --quotient_hi;

        if (t1 - t2 > svalue2)
            --quotient_hi;
    }

    const type_t nvalue1_21 = half_make_hi(nvalue1_32) + nvalue1_hi - quotient_hi * svalue2;

    std::optional<type_t> remainder_lo = type_t();
    type_t quotient_lo = divr(nvalue1_21, nvalue2_hi, remainder_lo);

    const type_t t3 = quotient_lo * nvalue2_lo;
    const type_t t4 = half_make_hi(*remainder_lo) | nvalue1_lo;

    if (t3 > t4) {

        --quotient_lo;

        if (t3 - t4 > svalue2)
            --quotient_lo;
    }

    if (remainder)
        remainder = static_cast<type_t>((half_make_hi(nvalue1_21) + nvalue1_lo - quotient_lo * svalue2) >> shift);

    return half_make_hi(quotient_hi) | quotient_lo;
}

} // namespace slim

////////////////////////////////////////////////////////////////////////////////////////////////////
// End of long_math.h
////////////////////////////////////////////////////////////////////////////////////////////////////
