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

#include "long_int.h"
#include "long_uint.h"

namespace slim
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// long_fixed_divider class
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
class long_fixed_divider
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // construction/destruction

    constexpr long_fixed_divider() noexcept = default;
    constexpr long_fixed_divider(const long_fixed_divider& that) noexcept = default;
    constexpr long_fixed_divider(long_fixed_divider&& that) noexcept = default;
    constexpr long_fixed_divider(const type_t& divider) noexcept;
    constexpr long_fixed_divider(const type_t& multiplier, const type_t& addition, const uint_t& shift) noexcept;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // public methods

    constexpr void swap(long_fixed_divider& that) noexcept;
    constexpr long_fixed_divider& operator=(const long_fixed_divider& that) noexcept = default;
    constexpr long_fixed_divider& operator=(long_fixed_divider&& that) noexcept = default;

    static constexpr long_fixed_divider create(const type_t& divider) noexcept;
    constexpr type_t divide(const type_t& dividend) const noexcept;

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // data members

    type_t multiplier;
    type_t addition;
    uint_t shift;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
constexpr long_fixed_divider<type_t> make_fixed_divider(const type_t value) noexcept;
template<typename type_t>
constexpr type_t operator/(type_t dividend, const long_fixed_divider<type_t>& divider) noexcept;



////////////////////////////////////////////////////////////////////////////////////////////////////
// long_fixed_divider class
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// construction/destruction

template<typename type_t>
constexpr long_fixed_divider<type_t>::long_fixed_divider(const type_t& divider) noexcept
: long_fixed_divider(create(divider))
{
}

template<typename type_t>
constexpr long_fixed_divider<type_t>::long_fixed_divider(const type_t& multiplier, const type_t& addition, const uint_t& shift) noexcept
: multiplier(std::move(multiplier))
, addition(std::move(addition))
, shift(std::move(shift))
{
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// public methods

template<typename type_t>
constexpr long_fixed_divider<type_t> long_fixed_divider<type_t>::create(const type_t& divider) noexcept
{
    type_t multiplier = 0;
    type_t addition = 0;
    uint_t shift = bit_count_v<type_t> - nlz(divider) - 1;

    // check if divider is power of 2 or zero

    if (divider == 0 || (divider & (divider - 1)) != 0) {

        const type_t one = type_t(1) << shift;
        std::optional<type_t> reminder = type_t();
        multiplier = divr2(one, type_t(0), divider, reminder);

        const type_t error = divider - *reminder;

        if (error < one)
            ++multiplier;
        else
            addition = multiplier;

    } else {

        if (divider == 1) {

            multiplier = type_t(~type_t(0));
            addition = type_t(~type_t(0));
            shift = 0;

        } else {

            multiplier = type_t(1) << (bit_count_v<type_t> - 1);
            --shift;
        }
    }

    return long_fixed_divider(multiplier, addition, shift);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
constexpr type_t long_fixed_divider<type_t>::divide(const type_t& dividend) const noexcept
{
    type_t mul_lo = dividend;
    type_t mul_hi = mul(mul_lo, multiplier);

    if (addition != 0)
        mul_hi += add(mul_lo, addition);

    return mul_hi >>= shift;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
constexpr long_fixed_divider<type_t> make_fixed_divider(const type_t value) noexcept
{
    return long_fixed_divider<type_t>(value);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
constexpr type_t operator/(type_t dividend, const long_fixed_divider<type_t>& divider) noexcept
{
    return divider.divide(dividend);
}

} // namespace slim

////////////////////////////////////////////////////////////////////////////////////////////////////
// End of long_fixdiv.h
////////////////////////////////////////////////////////////////////////////////////////////////////
