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

#if __has_include("long_math_gcc.h")
#include "long_math_gcc.h"
#endif // __has_include("long_math_gcc.h")

#if __has_include("long_math_msvc.h")
#include "long_math_msvc.h"
#endif // __has_include("long_math_msvc.h")

namespace slim
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// type definitions
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, uint_t size>
class long_uint_t;
template<typename type_t, uint_t size>
class long_int_t;

} // namespace slim

namespace std
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// class numeric_limits<slim::long_uint_t<type_t, size>>
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, size_t size>
struct numeric_limits<slim::long_uint_t<type_t, size>> {
public:

    using long_uint_t = slim::long_uint_t<type_t, size>;

    static const bool is_specialized = true;
    static const bool is_signed = false;
    static const bool is_integer = true;
    static const bool is_exact = true;

    static const bool has_infinity = false;
    static const bool has_quiet_NaN = false;
    static const bool has_signaling_NaN = false;
    static const float_denorm_style has_denorm = denorm_absent;
    static const bool has_denorm_loss = false;
    static const float_round_style round_style = round_toward_zero;

    static const bool is_iec559 = false;
    static const bool is_bounded = false;
    static const bool is_modulo = false;

    static const int digits = 0;
    static const int digits10 = 0;
    static const int radix = 2;

    static const int min_exponent = 0;
    static const int min_exponent10 = 0;
    static const int max_exponent = 0;
    static const int max_exponent10 = 0;

    static const bool traps = false;
    static const bool tinyness_before = false;

    static long_uint_t min() noexcept
    {
        return long_uint_t(0);
    }

    static constexpr long_uint_t lowest() noexcept
    {
        return long_uint_t(0);
    }

    static long_uint_t max() noexcept
    {
        return long_uint_t(-1);
    }

    static long_uint_t epsilon() noexcept
    {
        return long_uint_t(0);
    }

    static long_uint_t round_error() noexcept
    {
        return long_uint_t(0);
    }

    static long_uint_t infinity() noexcept
    {
        return long_uint_t(0);
    }

    static long_uint_t quiet_NaN() noexcept
    {
        return long_uint_t(0);
    }

    static long_uint_t signaling_NaN() noexcept
    {
        return long_uint_t(0);
    }

    static long_uint_t denorm_min() noexcept
    {
        return long_uint_t(0);
    }
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// class numeric_limits<slim::long_int_t<type_t, size>>
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, size_t size>
struct numeric_limits<slim::long_int_t<type_t, size>> {
public:
    using long_int_t = slim::long_int_t<type_t, size>;

    static const bool is_specialized = true;
    static const bool is_signed = true;
    static const bool is_integer = true;
    static const bool is_exact = true;

    static const bool has_infinity = false;
    static const bool has_quiet_NaN = false;
    static const bool has_signaling_NaN = false;
    static const float_denorm_style has_denorm = denorm_absent;
    static const bool has_denorm_loss = false;
    static const float_round_style round_style = round_toward_zero;

    static const bool is_iec559 = false;
    static const bool is_bounded = false;
    static const bool is_modulo = false;

    static const int digits = 0;
    static const int digits10 = 0;
    static const int radix = 2;

    static const int min_exponent = 0;
    static const int min_exponent10 = 0;
    static const int max_exponent = 0;
    static const int max_exponent10 = 0;

    static const bool traps = false;
    static const bool tinyness_before = false;

    static long_int_t min() noexcept
    {
        return slim::long_uint_t<type_t, size>(1) << (slim::bit_count_v<long_int_t> - 1);
    }

    static constexpr long_int_t lowest() noexcept
    {
        return min();
    }

    static long_int_t max() noexcept
    {
        return ~slim::long_uint_t<type_t, size>(min());
    }

    static long_int_t epsilon() noexcept
    {
        return long_int_t(0);
    }

    static long_int_t round_error() noexcept
    {
        return long_int_t(0);
    }

    static long_int_t infinity() noexcept
    {
        return long_int_t(0);
    }

    static long_int_t quiet_NaN() noexcept
    {
        return long_int_t(0);
    }

    static long_int_t signaling_NaN() noexcept
    {
        return long_int_t(0);
    }

    static long_int_t denorm_min() noexcept
    {
        return long_int_t(0);
    }
};

} // namespace std

namespace slim
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// make_unsigned_t
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename native_t, uint_t size>
struct make_unsigned<long_uint_t<native_t, size>> {
    using type = long_uint_t<native_t, size>;
};
template<typename native_t, uint_t size>
struct make_unsigned<long_int_t<native_t, size>> {
    using type = long_uint_t<native_t, size>;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// make_signed_t
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename native_t, uint_t size>
struct make_signed<long_uint_t<native_t, size>> {
    using type = long_int_t<native_t, size>;
};
template<typename native_t, uint_t size>
struct make_signed<long_int_t<native_t, size>> {
    using type = long_int_t<native_t, size>;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// half_t
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, uint_t size>
struct half_type<long_uint_t<type_t, size>> {
    using type = long_uint_t<type_t, size / 2>;
};
template<typename type_t>
struct half_type<long_uint_t<type_t, 2>> {
    using type = type_t;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone routines
////////////////////////////////////////////////////////////////////////////////////////////////////

// extract low half of unsigned integer

template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size / 2> half_lo(const long_uint_t<type_t, size>& value) noexcept;
template<typename type_t>
constexpr type_t half_lo(const long_uint_t<type_t, 2>& value) noexcept;

// extract high half of unsigned integer

template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size / 2> half_hi(const long_uint_t<type_t, size>& value) noexcept;
template<typename type_t>
constexpr type_t half_hi(const long_uint_t<type_t, 2>& value) noexcept;

// make unsigned integer from low and high

template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> half_make(const long_uint_t<type_t, size / 2>& high, const long_uint_t<type_t, size / 2>& low) noexcept;
template<typename type_t>
constexpr long_uint_t<type_t, 2> half_make(const type_t& high, const type_t& low) noexcept;

// move low half to high

template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> half_make_hi(const long_uint_t<type_t, size>& value) noexcept;

// return most significant bit

template<typename type_t, uint_t size>
constexpr bool sign(const long_uint_t<type_t, size>& value) noexcept;
template<typename type_t, uint_t size>
constexpr bool sign(const long_int_t<type_t, size>& value) noexcept;

// calculate leading zero bits

template<typename type_t, uint_t size>
constexpr uint_t nlz(const long_uint_t<type_t, size>& value) noexcept;

// multiply with carry

template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> mul(long_uint_t<type_t, size>& value1, const long_uint_t<type_t, size>& value2) noexcept;
template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> mulc(long_uint_t<type_t, size>& value1, const long_uint_t<type_t, size>& value2, const long_uint_t<type_t, size>& carry) noexcept;

// divide with remainder

template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> divr(const long_uint_t<type_t, size>& value1, const long_uint_t<type_t, size>& value2, std::optional<long_uint_t<type_t, size>>& remainder) noexcept;

// negate vector

template<typename type_t, std::enable_if_t<is_unsigned_array_v<type_t>, int> = 0>
constexpr void negate(type_t& value) noexcept;

// add two vectors

template<typename type_t, std::enable_if_t<is_unsigned_array_v<type_t>, int> = 0>
constexpr void add(type_t& value1, const type_t& value2) noexcept;

// subtract two vectors

template<typename type_t, std::enable_if_t<is_unsigned_array_v<type_t>, int> = 0>
constexpr void sub(type_t& value1, const type_t& value2) noexcept;

// multiply two vectors

template<typename type_t, std::enable_if_t<is_unsigned_array_v<type_t>, int> = 0>
constexpr void mul(type_t& value1, const type_t& value2) noexcept;



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone routines
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size / 2> half_lo(const long_uint_t<type_t, size>& value) noexcept
{
    constexpr uint_t half_size = size / 2;
    long_uint_t<type_t, half_size> result;

    for (uint_t n = 0; n < half_size; n++)
        result.digits[n] = value.digits[n];

    return result;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
constexpr type_t half_lo(const long_uint_t<type_t, 2>& value) noexcept
{
    return value.digits[0];
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size / 2> half_hi(const long_uint_t<type_t, size>& value) noexcept
{
    constexpr uint_t half_size = size / 2;
    long_uint_t<type_t, half_size> result;

    for (uint_t n = 0; n < half_size; n++)
        result.digits[n] = value.digits[n + half_size];

    return result;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
constexpr type_t half_hi(const long_uint_t<type_t, 2>& value) noexcept
{
    return value.digits[1];
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> half_make(const long_uint_t<type_t, size / 2>& value_hi, const long_uint_t<type_t, size / 2>& value_lo) noexcept
{
    constexpr uint_t half_size = size / 2;
    long_uint_t<type_t, half_size> result;

    for (uint_t n = 0; n < half_size; n++)
        result.digits[n] = value_lo.digits[n];
    for (uint_t n = half_size; n < size; n++)
        result.digits[n] = value_hi.digits[n - half_size];

    return result;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
constexpr long_uint_t<type_t, 2> half_make(const type_t& value_hi, const type_t& value_lo) noexcept
{
    return long_uint_t<type_t, 2>({ value_lo, value_hi });
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> half_make_hi(const long_uint_t<type_t, size>& value) noexcept
{
    using long_uint_t = long_uint_t<type_t, size>;

    constexpr uint_t half_size = size / 2;
    long_uint_t result;

    for (uint_t n = 0; n < half_size; n++)
        result.digits[n] = 0;
    for (uint_t n = half_size; n < size; n++)
        result.digits[n] = value.digits[n - half_size];

    return result;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size>
constexpr bool sign(const long_uint_t<type_t, size>& value) noexcept
{
    using long_uint_t = long_uint_t<type_t, size>;

    return make_signed_t<long_uint_t>(value.digits[long_uint_t::hi]) < 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size>
constexpr bool sign(const long_int_t<type_t, size>& value) noexcept
{
    return value.sign();
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size>
constexpr uint_t nlz(const long_uint_t<type_t, size>& value) noexcept
{
    using long_uint_t = long_uint_t<type_t, size>;

    uint_t count = 0;

    for (uint_t n = std::size(value.digits); n-- > 0;) {

        const uint_t scount = nlz(value.digits[n]);
        count += scount;

        if (scount < bit_count_v<type_t>)
            break;
    }

    return count;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> mul(long_uint_t<type_t, size>& value1, const long_uint_t<type_t, size>& value2) noexcept
{
    using long_uint_t = long_uint_t<type_t, size>;
    using half_uint_t = half_t<long_uint_t>;

    half_uint_t value1_lo = half_lo(value1);
    half_uint_t value1_hi = half_hi(value1);
    const half_uint_t value2_lo = half_lo(value2);
    const half_uint_t value2_hi = half_hi(value2);

    half_uint_t t0_lo = value1_lo;
    half_uint_t t1_lo = value1_hi;
    half_uint_t t2_lo = value1_lo;
    half_uint_t t3_lo = value1_hi;

    const half_uint_t t0_hi = mul(t0_lo, value2_lo);
    const half_uint_t t1_hi = mulc(t1_lo, value2_lo, t0_hi);
    const half_uint_t t2_hi = mulc(t2_lo, value2_hi, t1_lo);
    const half_uint_t t3_hi = mulc(t3_lo, value2_hi, t2_hi);

    value1_lo = t0_lo;
    value1_hi = t2_lo;

    half_uint_t carry_lo = t3_lo;
    const half_uint_t carry_hi = t3_hi + add(carry_lo, t1_hi);

    value1 = half_make(value1_hi, value1_lo);

    return half_make(carry_hi, carry_lo);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> mulc(long_uint_t<type_t, size>& value1, const long_uint_t<type_t, size>& value2, const long_uint_t<type_t, size>& carry) noexcept
{
    using long_uint_t = long_uint_t<type_t, size>;
    using half_uint_t = half_t<long_uint_t>;

    half_uint_t value1_lo = half_lo(value1);
    half_uint_t value1_hi = half_hi(value1);
    const half_uint_t value2_lo = half_lo(value2);
    const half_uint_t value2_hi = half_hi(value2);

    half_uint_t t0_lo = value1_lo;
    half_uint_t t1_lo = value1_hi;
    half_uint_t t2_lo = value1_lo;
    half_uint_t t3_lo = value1_hi;

    const half_uint_t t0_hi = mul(t0_lo, value2_lo);
    const half_uint_t t1_hi = mulc(t1_lo, value2_lo, t0_hi);
    const half_uint_t t2_hi = mulc(t2_lo, value2_hi, t1_lo);
    const half_uint_t t3_hi = mulc(t3_lo, value2_hi, t2_hi);

    value1_lo = t0_lo;
    value1_hi = t2_lo;

    half_uint_t carry_lo = t3_lo;
    half_uint_t carry_hi = t3_hi + add(carry_lo, t1_hi);

    bool add_carry = false;

    if (carry != 0) {

        add_carry = add(value1_lo, half_lo(carry));
        add_carry = addc(value1_hi, half_hi(carry), add_carry);
    }

    carry_hi += add(carry_lo, half_uint_t(add_carry));
    value1 = half_make(value1_hi, value1_lo);

    return half_make(carry_hi, carry_lo);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, uint_t size>
constexpr long_uint_t<type_t, size> divr(const long_uint_t<type_t, size>& value1, const long_uint_t<type_t, size>& value2, std::optional<long_uint_t<type_t, size>>& remainder) noexcept
{
    using long_uint_t = long_uint_t<type_t, size>;
    using half_uint_t = half_t<long_uint_t>;

    const half_uint_t dividend_lo = half_lo(value1);
    const half_uint_t dividend_hi = half_hi(value1);
    const half_uint_t divider_lo = half_lo(value2);
    const half_uint_t divider_hi = half_hi(value2);

    long_uint_t quotient;

    if (divider_hi == 0) {

        half_uint_t quotient_lo;
        half_uint_t quotient_hi;
        std::optional<half_uint_t> remainder_lo = remainder ? half_uint_t() : std::optional<half_uint_t>();

        if (divider_lo > dividend_hi) {

            if (dividend_hi == 0)
                quotient_lo = divr<half_uint_t>(dividend_lo, divider_lo, remainder_lo);
            else
                quotient_lo = divr2<half_uint_t>(dividend_hi, dividend_lo, divider_lo, remainder_lo);

            quotient_hi = 0;

        } else {

            quotient_lo = divr2<half_uint_t>(dividend_hi % divider_lo, dividend_lo, divider_lo, remainder_lo);
            quotient_hi = dividend_hi / divider_lo;
        }

        quotient = half_make(quotient_hi, quotient_lo);

        if (remainder)
            remainder = half_make(half_uint_t(0), *remainder_lo);

    } else {

        quotient = divr2<long_uint_t>(0, value1, value2, remainder);
    }

    return quotient;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_array_v<type_t>, int>>
constexpr void negate(type_t& value) noexcept
{
    using value_t = typename type_t::value_type;

    bool borrow = true;

    for (uint_t n = 0; n < std::size(value); ++n) {

        borrow = subb(value[n], value_t(0), borrow);
        value[n] = ~value[n];
    }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_array_v<type_t>, int>>
constexpr void add(type_t& value1, const type_t& value2) noexcept
{
    bool carry = add(value1[0], value2[0]);

    for (uint_t n = 1; n < std::size(value1); ++n)
        carry = addc(value1[n], value2[n], carry);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_array_v<type_t>, int>>
constexpr void sub(type_t& value1, const type_t& value2) noexcept
{
    bool borrow = sub(value1[0], value2[0]);

    for (uint_t n = 1; n < std::size(value1); ++n)
        borrow = subb(value1[n], value2[n], borrow);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, std::enable_if_t<is_unsigned_array_v<type_t>, int>>
constexpr void mul(type_t& value1, const type_t& value2) noexcept
{
    using value_t = typename type_t::value_type;

    type_t result;
    result[0] = value1[0];
    value_t carry = mul(result[0], value2[0]);

    for (uint_t n = 1; n < std::size(value1); ++n) {

        result[n] = value1[n];
        carry = mulc(result[n], value2[0], carry);
    }

    for (uint_t n = 1; n < std::size(value1); ++n) {

        type_t tmp;
        tmp[0] = 0;

        for (uint_t k = 1; k < n; ++k)
            tmp[k] = 0;

        tmp[n] = value1[0];
        carry = mul(tmp[n], value2[n]);

        for (uint_t k = 1; k < std::size(value1) - n; ++k) {

            tmp[k + n] = value1[k];
            carry = mulc(tmp[k + n], value2[n], carry);
        }

        add(result, tmp);
    }

    value1 = result;
}

} // namespace slim

////////////////////////////////////////////////////////////////////////////////////////////////////
// End of long_math_long.h
////////////////////////////////////////////////////////////////////////////////////////////////////
