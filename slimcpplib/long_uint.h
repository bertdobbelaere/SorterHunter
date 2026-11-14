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

#include "long_math_long.h"

namespace slim
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// long_uint_t class
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename native_t = uintmax_t, uint_t size = 2>
class long_uint_t
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // type and constant definition

    inline static constexpr uint_t lo = 0;
    inline static constexpr uint_t hi = size - 1;

    using native_array_t = std::array<native_t, size>;

    static_assert(std::is_unsigned_v<native_t>, "unsigned long integer native type must be unsigned.");
    static_assert(size >= 2, "native array size must be 2 or bigger.");
    static_assert((size & (size - 1)) == 0, "native array size must be power of 2.");

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // construction/destruction

    constexpr long_uint_t() noexcept = default;
    constexpr long_uint_t(const long_uint_t& that) noexcept = default;
    constexpr long_uint_t(long_uint_t&& that) noexcept = default;
    template<uint_t other_size, std::enable_if_t<(other_size < size), int> = 0>
    constexpr long_uint_t(const long_uint_t<native_t, other_size>& that) noexcept;
    constexpr long_uint_t(native_array_t digits) noexcept;
    template<typename type_t, std::enable_if_t<std::is_unsigned_v<type_t>, int> = 0>
    constexpr long_uint_t(type_t value) noexcept;
    template<typename type_t, std::enable_if_t<std::is_signed_v<type_t>, int> = 0>
    constexpr long_uint_t(type_t value) noexcept;
    constexpr long_uint_t(bool value) noexcept;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // public methods

    constexpr void swap(long_uint_t& that) noexcept;
    constexpr long_uint_t& operator=(const long_uint_t& that) noexcept = default;
    constexpr long_uint_t& operator=(long_uint_t&& that) noexcept = default;

    template<uint_t other_size, std::enable_if_t<(other_size < size), int> = 0>
    explicit constexpr operator long_uint_t<native_t, other_size>() const noexcept;
    template<typename type_t, std::enable_if_t<std::is_unsigned_v<type_t>, int> = 0>
    explicit constexpr operator type_t() const noexcept;
    template<typename type_t, std::enable_if_t<std::is_signed_v<type_t>, int> = 0>
    explicit constexpr operator type_t() const noexcept;
    constexpr bool operator==(const long_uint_t& that) const noexcept;
    constexpr bool operator!=(const long_uint_t& that) const noexcept;
    constexpr bool operator<(const long_uint_t& that) const noexcept;
    constexpr bool operator<=(const long_uint_t& that) const noexcept;
    constexpr bool operator>(const long_uint_t& that) const noexcept;
    constexpr bool operator>=(const long_uint_t& that) const noexcept;
    constexpr long_uint_t operator~() const noexcept;
    constexpr long_uint_t& operator&=(const long_uint_t& that) noexcept;
    constexpr long_uint_t operator&(const long_uint_t& that) const noexcept;
    constexpr long_uint_t& operator|=(const long_uint_t& that) noexcept;
    constexpr long_uint_t operator|(const long_uint_t& that) const noexcept;
    constexpr long_uint_t& operator^=(const long_uint_t& that) noexcept;
    constexpr long_uint_t operator^(const long_uint_t& that) const noexcept;
    constexpr long_uint_t& operator<<=(uint_t shift) noexcept;
    constexpr long_uint_t operator<<(uint_t shift) const noexcept;
    constexpr long_uint_t& operator>>=(uint_t shift) noexcept;
    constexpr long_uint_t operator>>(uint_t shift) const noexcept;
    constexpr long_uint_t& operator+=(const long_uint_t& that) noexcept;
    constexpr long_uint_t operator+(const long_uint_t& that) const noexcept;
    constexpr long_uint_t& operator++() noexcept;
    constexpr long_uint_t operator++(int) noexcept;
    constexpr long_uint_t operator+() const noexcept;
    constexpr long_uint_t& operator-=(const long_uint_t& that) noexcept;
    constexpr long_uint_t operator-(const long_uint_t& that) const noexcept;
    constexpr long_uint_t& operator--() noexcept;
    constexpr long_uint_t operator--(int) noexcept;
    constexpr long_uint_t operator-() const noexcept;
    constexpr long_uint_t& operator*=(const long_uint_t& that) noexcept;
    constexpr long_uint_t operator*(const long_uint_t& that) const noexcept;
    constexpr long_uint_t& operator/=(const long_uint_t& that) noexcept;
    constexpr long_uint_t operator/(const long_uint_t& that) const noexcept;
    constexpr long_uint_t& operator%=(const long_uint_t& that) noexcept;
    constexpr long_uint_t operator%(const long_uint_t& that) const noexcept;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // data members

    native_array_t digits;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t muldiv(const type_t& value, const type_t& multiplier, const type_t& divider) noexcept;



////////////////////////////////////////////////////////////////////////////////////////////////////
// long_uint_t class
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// construction/destruction

template<typename native_t, uint_t size>
template<uint_t other_size, std::enable_if_t<(other_size < size), int>>
constexpr long_uint_t<native_t, size>::long_uint_t(const long_uint_t<native_t, other_size>& that) noexcept
: digits(make_array<size>(that.digits, [](const auto& digits, uint_t idx) { return digits[idx]; }))
{
}

template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>::long_uint_t(native_array_t digits) noexcept
: digits(std::move(digits))
{
}

template<typename native_t, uint_t size>
template<typename type_t, std::enable_if_t<std::is_unsigned_v<type_t>, int>>
constexpr long_uint_t<native_t, size>::long_uint_t(type_t value) noexcept
: digits(make_array<size>(digits, [&value](const auto& /*digits*/, uint_t idx) {

    constexpr uint_t value_size = std::min(size, (byte_count_v<type_t> + byte_count_v<native_t> - 1) / byte_count_v<native_t>);

    if (idx >= value_size)
        return native_t(0);

    const native_t digit = value & ~native_t(0);
    value >>= std::min(bit_count_v<type_t> - 1, bit_count_v<native_t>);
    return digit;
}))
{
}

template<typename native_t, uint_t size>
template<typename type_t, std::enable_if_t<std::is_signed_v<type_t>, int>>
constexpr long_uint_t<native_t, size>::long_uint_t(type_t value) noexcept
{
    constexpr uint_t value_size = std::min(size, (byte_count_v<type_t> + byte_count_v<native_t> - 1) / byte_count_v<native_t>);
    const native_t extension = static_cast<native_t>(value >= 0 ? 0 : native_t(~0));

    for (uint_t n = 0; n < value_size; ++n) {

        digits[n] = value & ~native_t(0);
        value >>= std::min(bit_count_v<type_t> - 1, bit_count_v<native_t>);
    }
    for (uint_t n = value_size; n < std::size(digits); ++n)
        digits[n] = extension;
}

template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>::long_uint_t(bool value) noexcept
{
    digits[0] = value;

    for (uint_t n = 1; n < std::size(digits); ++n)
        digits[n] = native_t(0);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// public methods

template<typename native_t, uint_t size>
constexpr void long_uint_t<native_t, size>::swap(long_uint_t& that) noexcept
{
    using std::swap;

    swap(digits, that.digits);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
template<uint_t other_size, std::enable_if_t<(other_size < size), int>>
constexpr long_uint_t<native_t, size>::operator long_uint_t<native_t, other_size>() const noexcept
{
    return make_array<other_size>(digits, [](const auto& digits, uint_t idx) { return digits[idx]; });
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
template<typename type_t, std::enable_if_t<std::is_unsigned_v<type_t>, int>>
constexpr long_uint_t<native_t, size>::operator type_t() const noexcept
{
    return static_cast<type_t>(digits[lo]);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
template<typename type_t, std::enable_if_t<std::is_signed_v<type_t>, int>>
constexpr long_uint_t<native_t, size>::operator type_t() const noexcept
{
    return static_cast<type_t>(digits[lo]);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_uint_t<native_t, size>::operator==(const long_uint_t& that) const noexcept
{
    for (uint_t n = 0; n < std::size(digits); ++n) {

        if (digits[n] != that.digits[n])
            return false;
    }

    return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_uint_t<native_t, size>::operator!=(const long_uint_t& that) const noexcept
{
    return !operator==(that);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_uint_t<native_t, size>::operator<(const long_uint_t& that) const noexcept
{
    native_t digit = digits[0];
    bool borrow = sub(digit, that.digits[0]);

    for (uint_t n = 1; n < std::size(digits); ++n) {

        digit = digits[n];
        borrow = subb(digit, that.digits[n], borrow);
    }

    return borrow;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_uint_t<native_t, size>::operator<=(const long_uint_t& that) const noexcept
{
    return !that.operator<(*this);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_uint_t<native_t, size>::operator>(const long_uint_t& that) const noexcept
{
    return that.operator<(*this);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr bool long_uint_t<native_t, size>::operator>=(const long_uint_t& that) const noexcept
{
    return !operator<(that);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator~() const noexcept
{
    return make_array<size>(digits, [](const auto& digits, uint_t idx) {
        return ~digits[idx];
    });
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator&=(const long_uint_t& that) noexcept
{
    for (uint_t n = 0; n < std::size(digits); ++n)
        digits[n] &= that.digits[n];

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator&(const long_uint_t& that) const noexcept
{
    return long_uint_t(*this) &= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator|=(const long_uint_t& that) noexcept
{
    for (uint_t n = 0; n < std::size(digits); ++n)
        digits[n] |= that.digits[n];

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator|(const long_uint_t& that) const noexcept
{
    return long_uint_t(*this) |= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator^=(const long_uint_t& that) noexcept
{
    for (uint_t n = 0; n < std::size(digits); ++n)
        digits[n] ^= that.digits[n];

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator^(const long_uint_t& that) const noexcept
{
    return long_uint_t(*this) ^= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator<<=(uint_t shift) noexcept
{
    if (shift < bit_count_v<native_t> * std::size(digits)) {

        const uint_t offset = shift / bit_count_v<native_t>;
        shift %= bit_count_v<native_t>;

        for (uint_t n = std::size(digits); n-- > 1;)
            digits[n] = shl2(digits[n], digits[n - 1], shift);

        digits[0] <<= shift;

        if (offset > 0) {

            for (uint_t n = offset; n < std::size(digits); ++n)
                digits[std::size(digits) + offset - n - 1] = digits[std::size(digits) - n - 1];

            for (uint_t n = 0; n < offset; ++n)
                digits[n] = 0;
        }
    }

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator<<(uint_t shift) const noexcept
{
    return long_uint_t(*this) <<= shift;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator>>=(uint_t shift) noexcept
{
    if (shift < bit_count_v<native_t> * std::size(digits)) {

        const uint_t offset = shift / bit_count_v<native_t>;
        shift %= bit_count_v<native_t>;

        for (uint_t n = 0; n < std::size(digits) - 1; ++n)
            digits[n] = shr2(digits[n + 1], digits[n], shift);

        digits[std::size(digits) - 1] >>= shift;

        if (offset > 0) {

            for (uint_t n = 0; n < std::size(digits) - offset; ++n)
                digits[n] = digits[n + offset];

            for (uint_t n = std::size(digits) - offset; n < std::size(digits); ++n)
                digits[n] = 0;
        }
    }

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator>>(uint_t shift) const noexcept
{
    return long_uint_t(*this) >>= shift;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator+=(const long_uint_t& that) noexcept
{
    add(digits, that.digits);

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator+(const long_uint_t& that) const noexcept
{
    return long_uint_t(*this) += that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator++() noexcept
{
    *this += 1;

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator++(int) noexcept
{
    long_uint_t tmp = *this;

    *this += 1;

    return tmp;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator+() const noexcept
{
    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator-=(const long_uint_t& that) noexcept
{
    sub(digits, that.digits);

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator-(const long_uint_t& that) const noexcept
{
    return long_uint_t(*this) -= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator--() noexcept
{
    *this -= 1;

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator--(int) noexcept
{
    long_uint_t tmp = *this;

    *this -= 1;

    return tmp;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator-() const noexcept
{
    bool borrow = true;

    return make_array<size>(digits, [&borrow](const auto& digits, uint_t idx) {

        native_t digit = digits[idx];
        borrow = subb<native_t>(digit, 0, borrow);
        return ~digit;
    });
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator*=(const long_uint_t& that) noexcept
{
    mul(digits, that.digits);

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator*(const long_uint_t& that) const noexcept
{
    return long_uint_t(*this) *= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator/=(const long_uint_t& that) noexcept
{
    std::optional<long_uint_t> remainder;
    *this = divr(*this, that, remainder);

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator/(const long_uint_t& that) const noexcept
{
    return long_uint_t(*this) /= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size>& long_uint_t<native_t, size>::operator%=(const long_uint_t& that) noexcept
{
    std::optional<long_uint_t> remainder = long_uint_t();
    divr(*this, that, remainder);

    *this = *remainder;

    return *this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> long_uint_t<native_t, size>::operator%(const long_uint_t& that) const noexcept
{
    return long_uint_t(*this) %= that;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator==(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) == value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator!=(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) != value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator<(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) < value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator<=(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) <= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator>(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) > value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr bool operator>=(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) >= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr long_uint_t<native_t, size> operator&(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) &= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr long_uint_t<native_t, size> operator|(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) |= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr long_uint_t<native_t, size> operator^(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) ^= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> operator<<(long_uint_t<native_t, size> value, uint_t shift) noexcept
{
    return value <<= shift;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size>
constexpr long_uint_t<native_t, size> operator>>(long_uint_t<native_t, size> value, uint_t shift) noexcept
{
    return value >>= shift;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr long_uint_t<native_t, size> operator+(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) += value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr long_uint_t<native_t, size> operator-(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) -= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr long_uint_t<native_t, size> operator*(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) *= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr long_uint_t<native_t, size> operator/(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) /= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename native_t, uint_t size, typename std::enable_if_t<is_unsigned_v<type_t> || is_signed_v<type_t>, int> = 0>
constexpr long_uint_t<native_t, size> operator%(type_t value1, const long_uint_t<native_t, size>& value2) noexcept
{
    return long_uint_t<native_t, size>(value1) %= value2;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int>>
constexpr type_t muldiv(const type_t& value, const type_t& multiplier, const type_t& divider) noexcept
{
    type_t mul_lo = value;
    const type_t mul_hi = mul(mul_lo, multiplier);

    if (mul_hi == 0)
        return mul_lo / divider;

    std::optional<type_t> remainder;
    return divr2(mul_hi, mul_lo, divider, remainder);
}

namespace impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// enum parse_result
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class parse_result { ok,
    unexpected,
    overflow };
constexpr uint8_t kSeparator = 0xff;



////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename type_t, std::enable_if_t<is_unsigned_v<type_t>, int> = 0>
constexpr type_t mulc_costexpr(const type_t& value1, const type_t& value2, type_t& carry) noexcept
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

    result_hi += add<type_t>(result_lo, carry);
    carry = result_hi;

    return result_lo;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<uint_t literal_size, typename digit_parser_t>
constexpr std::optional<std::array<uint8_t, literal_size>> parse_digits(const std::array<char, literal_size>& literal, const digit_parser_t& digit_parser) noexcept
{
    std::array<uint8_t, literal_size> digits = {};

    for (uint_t n = 0; n < std::size(digits); ++n) {

        const std::optional<uint8_t> digit = digit_parser(literal[n]);

        if (!digit)
            return std::nullopt;

        digits[n] = *digit;
    }

    return digits;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename long_t, uint8_t base, uint_t literal_size, typename digit_parser_t>
constexpr std::pair<parse_result, long_t> parse_literal(const std::array<char, literal_size>& literal, const digit_parser_t& digit_parser) noexcept
{
    const std::optional<std::array<uint8_t, literal_size>> literal_digits = parse_digits(literal, digit_parser);

    if (!literal_digits)
        return std::make_pair(parse_result::unexpected, long_t(0));

    using native_t = typename long_t::native_array_t::value_type;
    typename long_t::native_array_t digits = { 0, 0 };

    for (uint_t literal_idx = 0; literal_idx < literal_digits->size(); ++literal_idx) {

        native_t carry = (*literal_digits)[literal_idx];

        if (carry == kSeparator)
            continue;

        for (auto& digit : digits)
            digit = mulc_costexpr<native_t>(digit, native_t(base), carry);

        if (carry > 0)
            return std::make_pair(parse_result::overflow, long_t(0));
    }

    return std::make_pair(parse_result::ok, long_t(digits));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr std::optional<uint8_t> parse_oct_digit(char ch) noexcept
{
    if (ch == '\'')
        return kSeparator;

    if (ch >= '0' && ch <= '7')
        return uint8_t(uint_t(ch) - '0');

    return std::nullopt;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename long_t, typename... char_t>
constexpr std::pair<parse_result, long_t> parse_oct_literal(char, char_t... chars) noexcept
{
    constexpr uint_t literal_size = sizeof...(chars);
    const std::array<char, literal_size> literal = { chars... };

    return parse_literal<long_t, 8>(literal, &parse_oct_digit);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr std::optional<uint8_t> parse_dec_digit(char ch) noexcept
{
    if (ch == '\'')
        return kSeparator;

    if (ch >= '0' && ch <= '9')
        return uint8_t(uint_t(ch) - '0');

    return std::nullopt;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename long_t, typename... char_t>
constexpr std::pair<parse_result, long_t> parse_dec_literal(char_t... chars) noexcept
{
    constexpr uint_t literal_size = sizeof...(chars);
    const std::array<char, literal_size> literal = { chars... };

    return parse_literal<long_t, 10>(literal, &parse_dec_digit);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr std::optional<uint8_t> parse_hex_digit(char ch) noexcept
{
    if (ch == '\'')
        return kSeparator;

    if (ch >= '0' && ch <= '9')
        return uint8_t(uint_t(ch) - '0');
    if (ch >= 'A' && ch <= 'F')
        return uint8_t(10 + uint_t(ch) - 'A');
    else if (ch >= 'a' && ch <= 'f')
        return uint8_t(10 + uint_t(ch) - 'a');

    return std::nullopt;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename long_t, typename... char_t>
constexpr std::pair<parse_result, long_t> parse_hex_literal(char, char, char_t... chars) noexcept
{
    constexpr uint_t literal_size = sizeof...(chars);
    const std::array<char, literal_size> literal = { chars... };

    return parse_literal<long_t, 16>(literal, &parse_hex_digit);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename long_uint_t, char... chars>
constexpr long_uint_t parse_literal() noexcept
{
    constexpr uint_t char_count = sizeof...(chars);
    constexpr std::array<char, char_count> literal = { chars... };

    static_assert(literal[0] >= '0' && literal[0] <= '9', "unsigned long integer literal must be a number.");

    if constexpr (literal[0] == '0') {

        if constexpr (char_count == 1 || (literal[1] != 'X' && literal[1] != 'x')) {

            constexpr std::pair<parse_result, long_uint_t> value = parse_oct_literal<long_uint_t>(chars...);
            static_assert(value.first != parse_result::unexpected, "unexpected simbol unsigned in long integer oct literal");
            static_assert(value.first != parse_result::overflow, "oveflow in unsigned long integer oct literal");

            return value.second;

        } else {

            constexpr std::pair<parse_result, long_uint_t> value = parse_hex_literal<long_uint_t>(chars...);
            static_assert(value.first != parse_result::unexpected, "unexpected simbol in unsigned long integer hex literal");
            static_assert(value.first != parse_result::overflow, "oveflow in unsigned long integer hex literal");

            return value.second;
        }

    } else {

        constexpr std::pair<parse_result, long_uint_t> value = parse_dec_literal<long_uint_t>(chars...);
        static_assert(value.first != parse_result::unexpected, "unexpected simbol in unsigned long integer dec literal");
        static_assert(value.first != parse_result::overflow, "oveflow in unsigned long integer dec literal");

        return value.second;
    }
}
} // namespace impl

////////////////////////////////////////////////////////////////////////////////////////////////////
// long integer literal accept operator
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef UINT64_MAX

using uint128_t = long_uint_t<uint64_t, 2>;
using uint256_t = long_uint_t<uint64_t, 4>;

namespace literals
{

template<char... chars>
constexpr uint128_t operator"" _ui128() noexcept
{
    return slim::impl::parse_literal<uint128_t, chars...>();
}

template<char... chars>
constexpr uint256_t operator"" _ui256() noexcept
{
    return slim::impl::parse_literal<uint256_t, chars...>();
}

} // namespace literals

#endif // UINT64_MAX

} // namespace slim

////////////////////////////////////////////////////////////////////////////////////////////////////
// End of long_uint.h
////////////////////////////////////////////////////////////////////////////////////////////////////
