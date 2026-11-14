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
//
// $Id: long_int.h 154 2021-09-13 22:14:28Z ykalmykov $
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "long_uint.h"

namespace slim
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// long_int_t class
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename native_t = uintmax_t, uint_t size = 2>
class long_int_t : public long_uint_t<native_t, size>
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // type and constant definition

    using long_uint_t<native_t, size>::lo;
    using long_uint_t<native_t, size>::hi;
    using long_uint_t<native_t, size>::digits;
    using native_array_t = typename long_uint_t<native_t, size>::native_array_t;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // construction/destruction

    constexpr long_int_t() noexcept = default;
    constexpr long_int_t(const long_int_t& that) noexcept = default;
    constexpr long_int_t(long_int_t&& that) noexcept = default;
    constexpr long_int_t(const long_uint_t<native_t, size>& that) noexcept;
    template<uint_t other_size, std::enable_if_t<(other_size < size), int> = 0>
    constexpr long_int_t(const long_int_t<native_t, other_size>& that) noexcept;
    constexpr long_int_t(native_array_t digits) noexcept;
    template<typename type_t, std::enable_if_t<std::is_unsigned_v<type_t>, int> = 0>
    constexpr long_int_t(type_t value) noexcept;
    template<typename type_t, std::enable_if_t<std::is_signed_v<type_t>, int> = 0>
    constexpr long_int_t(type_t value) noexcept;
    constexpr long_int_t(bool value) noexcept;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // public methods

    constexpr void swap(long_int_t& that) noexcept;
    constexpr long_int_t& operator=(const long_int_t& that) noexcept = default;
    constexpr long_int_t& operator=(long_int_t&& that) noexcept = default;

    constexpr bool sign() const noexcept;
    constexpr long_int_t& negate() noexcept;
    template<uint_t other_size, std::enable_if_t<(other_size < size), int> = 0>
    explicit constexpr operator long_int_t<native_t, other_size>() const noexcept;
    template<typename type_t, std::enable_if_t<std::is_signed_v<type_t>, int> = 0>
    explicit constexpr operator type_t() const noexcept;
    constexpr bool operator==(const long_int_t& that) const noexcept;
    constexpr bool operator!=(const long_int_t& that) const noexcept;
    constexpr bool operator<(const long_int_t& that) const noexcept;
    constexpr bool operator<=(const long_int_t& that) const noexcept;
    constexpr bool operator>(const long_int_t& that) const noexcept;
    constexpr bool operator>=(const long_int_t& that) const noexcept;
    constexpr long_int_t& operator+=(const long_int_t& that) noexcept;
    constexpr long_int_t operator+(const long_int_t& that) const noexcept;
    constexpr long_int_t& operator++() noexcept;
    constexpr long_int_t operator++(int) noexcept;
    constexpr long_int_t operator+() const noexcept;
    constexpr long_int_t& operator-=(const long_int_t& that) noexcept;
    constexpr long_int_t operator-(const long_int_t& that) const noexcept;
    constexpr long_int_t& operator--() noexcept;
    constexpr long_int_t operator--(int) noexcept;
    constexpr long_int_t operator-() const noexcept;
    constexpr long_int_t& operator*=(const long_int_t& that) noexcept;
    constexpr long_int_t operator*(const long_int_t& that) const noexcept;
    constexpr long_int_t& operator/=(const long_int_t& that) noexcept;
    constexpr long_int_t operator/(const long_int_t& that) const noexcept;
    constexpr long_int_t& operator%=(const long_int_t& that) noexcept;
    constexpr long_int_t operator%(const long_int_t& that) const noexcept;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, std::enable_if_t<is_signed_v<type_t>, int> = 0>
constexpr type_t muldiv(const type_t& value, const type_t& multiplier, const type_t& divider) noexcept;



////////////////////////////////////////////////////////////////////////////////////////////////////
// long_int_t class
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// construction/destruction

template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>::long_int_t(const long_uint_t<native_t, size>& that) noexcept
: long_uint_t<native_t, size>(that)
{
}

template<typename native_t, uint_t size>
template<uint_t other_size, std::enable_if_t<(other_size < size), int>>
constexpr long_int_t<native_t, size>::long_int_t(const long_int_t<native_t, other_size>& that) noexcept
: long_uint_t<native_t, size>(that)
{
    constexpr uint_t value_size = std::min(size, other_size);
    const native_t extension = static_cast<native_t>(!that.sign() ? 0 : native_t(~0));

    for (uint_t n = value_size; n < std::size(digits); ++n)
        digits[n] = extension;
}

template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>::long_int_t(native_array_t digits) noexcept
: long_uint_t<native_t, size>(std::move(digits))
{
}

template<typename native_t, uint_t size>
template<typename type_t, std::enable_if_t<std::is_unsigned_v<type_t>, int>>
constexpr long_int_t<native_t, size>::long_int_t(type_t value) noexcept
: long_uint_t<native_t, size>(value)
{
}

template<typename native_t, uint_t size>
template<typename type_t, std::enable_if_t<std::is_signed_v<type_t>, int>>
constexpr long_int_t<native_t, size>::long_int_t(type_t value) noexcept
: long_uint_t<native_t, size>(value)
{
}

template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>::long_int_t(bool value) noexcept
: long_uint_t<native_t, size>(value)
{
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// public methods

template<typename native_t, uint_t size>
constexpr void long_int_t<native_t, size>::swap(long_int_t& that) noexcept
{
    using std::swap;

    long_uint_t<native_t, size>::swap(digits, that.digits);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_int_t<native_t, size>::sign() const noexcept
{
    return make_signed_t<native_t>(digits[hi]) < 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>& long_int_t<native_t, size>::negate() noexcept
{
    slim::negate(digits);

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
template<uint_t other_size, std::enable_if_t<(other_size < size), int>>
constexpr long_int_t<native_t, size>::operator long_int_t<native_t, other_size>() const noexcept
{
    return make_array<other_size>(digits, [](const auto& digits, uint_t idx) { return digits[idx]; });
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
template<typename type_t, std::enable_if_t<std::is_signed_v<type_t>, int>>
constexpr long_int_t<native_t, size>::operator type_t() const noexcept
{
    return static_cast<type_t>(digits[lo]);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_int_t<native_t, size>::operator==(const long_int_t& that) const noexcept
{
    return long_uint_t<native_t, size>::operator==(that);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_int_t<native_t, size>::operator!=(const long_int_t& that) const noexcept
{
    return !operator==(that);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_int_t<native_t, size>::operator<(const long_int_t& that) const noexcept
{
    return (long_uint_t<native_t, size>::operator<(that) != sign()) != that.sign();
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_int_t<native_t, size>::operator<=(const long_int_t& that) const noexcept
{
    return !that.operator<(*this);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_int_t<native_t, size>::operator>(const long_int_t& that) const noexcept
{
    return that.operator<(*this);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_int_t<native_t, size>::operator>=(const long_int_t& that) const noexcept
{
    return !operator<(that);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>& long_int_t<native_t, size>::operator+=(const long_int_t& that) noexcept
{
    long_uint_t<native_t, size>::operator+=(that);

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size> long_int_t<native_t, size>::operator+(const long_int_t& that) const noexcept
{
    return long_int_t(*this) += that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>& long_int_t<native_t, size>::operator++() noexcept
{
    long_uint_t<native_t, size>::operator++();

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size> long_int_t<native_t, size>::operator++(int) noexcept
{
    return long_int_t(long_uint_t<native_t, size>::operator++(0));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size> long_int_t<native_t, size>::operator+() const noexcept
{
    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>& long_int_t<native_t, size>::operator-=(const long_int_t& that) noexcept
{
    long_uint_t<native_t, size>::operator-=(that);

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size> long_int_t<native_t, size>::operator-(const long_int_t& that) const noexcept
{
    return long_int_t(*this) -= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>& long_int_t<native_t, size>::operator--() noexcept
{
    long_uint_t<native_t, size>::operator--();

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size> long_int_t<native_t, size>::operator--(int) noexcept
{
    return long_int_t(long_uint_t<native_t, size>::operator--(0));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size> long_int_t<native_t, size>::operator-() const noexcept
{
    return -long_uint_t<native_t, size>(*this);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>& long_int_t<native_t, size>::operator*=(const long_int_t& that) noexcept
{
    long_uint_t<native_t, size>::operator*=(that);

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size> long_int_t<native_t, size>::operator*(const long_int_t& that) const noexcept
{
    return long_int_t(*this) *= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>& long_int_t<native_t, size>::operator/=(const long_int_t& that) noexcept
{
    const bool sing = sign();

    if (sing)
        negate();

    long_uint_t<native_t, size>::operator/=(that.sign() ? -that : that);

    if (sing != that.sign())
        negate();

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size> long_int_t<native_t, size>::operator/(const long_int_t& that) const noexcept
{
    return long_int_t(*this) /= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size>& long_int_t<native_t, size>::operator%=(const long_int_t& that) noexcept
{
    const bool sing = sign();

    if (sing)
        negate();

    long_uint_t<native_t, size>::operator%=(that.sign() ? -that : that);

    if (sing != that.sign())
        negate();

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_int_t<native_t, size> long_int_t<native_t, size>::operator%(const long_int_t& that) const noexcept
{
    return long_int_t(*this) %= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator==(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) == value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator!=(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) != value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator<(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) < value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator<=(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) <= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator>(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) > value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator>=(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) >= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<std::is_integral_v<type_t>, int> = 0>
constexpr long_int_t<native_t, size> operator+(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) += value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<std::is_integral_v<type_t>, int> = 0>
constexpr long_int_t<native_t, size> operator-(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) -= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<std::is_integral_v<type_t>, int> = 0>
constexpr long_int_t<native_t, size> operator*(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) *= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<std::is_integral_v<type_t>, int> = 0>
constexpr long_int_t<native_t, size> operator/(type_t value1, const long_int_t<native_t, size>& value2) noexcept
{
    return long_int_t<native_t, size>(value1) /= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, std::enable_if_t<is_signed_v<type_t>, int>>
constexpr type_t muldiv(const type_t& value, const type_t& multiplier, const type_t& divider) noexcept
{
    using unsigned_t = make_unsigned_t<type_t>;

    const bool value_sign = sign(value);
    const bool multiplier_sign = sign(multiplier);
    const bool divider_sign = sign(divider);

    const unsigned_t uvalue = value_sign ? -value : value;
    const unsigned_t umultiplier = multiplier_sign ? -multiplier : multiplier;
    const unsigned_t udivider = divider_sign ? -divider : divider;

    type_t result = muldiv<unsigned_t>(uvalue, umultiplier, udivider);

    if (value_sign != multiplier_sign != divider_sign)
        result = -result;

    return result;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// long integer literal accept operator
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef INT64_MAX

using int128_t = long_int_t<uint64_t, 2>;
using int256_t = long_int_t<uint64_t, 4>;

namespace literals
{

template<char... chars>
constexpr int128_t operator"" _si128() noexcept
{
    return slim::impl::parse_literal<uint128_t, chars...>();
}

template<char... chars>
constexpr int256_t operator"" _si256() noexcept
{
    return slim::impl::parse_literal<uint256_t, chars...>();
}

} // namespace literals

#endif // INT64_MAX

} // namespace slim

////////////////////////////////////////////////////////////////////////////////////////////////////
// End of long_int.h
////////////////////////////////////////////////////////////////////////////////////////////////////
