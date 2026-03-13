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

#include "long_uint.h"

#include <cmath>
#include <iostream>

namespace slim
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename native_t, uint_t size, typename char_t, class traits_t = std::char_traits<char_t>>
std::basic_ostream<char_t, traits_t>& operator<<(std::basic_ostream<char_t, traits_t>& stream, const long_uint_t<native_t, size>& value);
template<typename native_t, uint_t size, typename char_t, class traits_t = std::char_traits<char_t>>
std::basic_ostream<char_t, traits_t>& operator<<(std::basic_ostream<char_t, traits_t>& stream, const long_int_t<native_t, size>& value);
template<typename native_t, uint_t size, typename char_t, class traits_t = std::char_traits<char_t>>
std::basic_istream<char_t, traits_t>& operator>>(std::basic_istream<char_t, traits_t>& stream, const long_uint_t<native_t, size>& value);
template<typename native_t, uint_t size, typename char_t, class traits_t = std::char_traits<char_t>>
std::basic_istream<char_t, traits_t>& operator>>(std::basic_istream<char_t, traits_t>& stream, const long_int_t<native_t, size>& value);

namespace impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone helper methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename char_t>
char_t to_char(uint_t digit, bool uppercase) noexcept;
template<typename char_t>
uint_t from_char(char_t ch);
template<typename char_t, class traits_t = std::char_traits<char_t>>
uint_t get_string(std::basic_istream<char_t, traits_t>& stream, char_t* str, uint_t size);

} // namespace impl

////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename native_t, uint_t size, typename char_t, class traits_t>
inline std::basic_ostream<char_t, traits_t>& operator<<(std::basic_ostream<char_t, traits_t>& stream, const long_uint_t<native_t, size>& value)
{
    using long_uint_t = long_uint_t<native_t, size>;

    uint_t base = 10;

    if ((stream.flags() & std::ios::basefield) == std::ios::oct)
        base = 8;
    else if ((stream.flags() & std::ios::basefield) == std::ios::hex)
        base = 16;

    constexpr uint_t max_digits = (bit_count_v<long_uint_t> + 2) / 3;
    const uint_t digit_count = std::max<uint_t>(1, static_cast<uint_t>(std::ceil((bit_count_v<long_uint_t> - nlz(value)) * std::log10(2) / std::log10(base))));
    const bool uppercase = (stream.flags() & std::ios::uppercase) != 0;
    std::array<char_t, max_digits> output;

    long_uint_t reminder = value;
    for (uint_t digit_idx = digit_count; digit_idx-- > 0;) {

        std::optional<long_uint_t> digit = long_uint_t();
        reminder = divr(reminder, long_uint_t(base), digit);

        output[digit_idx] = impl::to_char<char_t>(static_cast<uint_t>(*digit), uppercase);
    }

    stream.write(output.data(), digit_count);

    return stream;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size, typename char_t, class traits_t>
inline std::basic_ostream<char_t, traits_t>& operator<<(std::basic_ostream<char_t, traits_t>& stream, const long_int_t<native_t, size>& value)
{
    using long_uint_t = long_uint_t<native_t, size>;

    if (value.sign())
        return stream << '-' << long_uint_t(-value);

    return stream << long_uint_t(value);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size, typename char_t, class traits_t>
inline std::basic_istream<char_t, traits_t>& operator>>(std::basic_istream<char_t, traits_t>& stream, long_uint_t<native_t, size>& value)
{
    using long_uint_t = long_uint_t<native_t, size>;

    constexpr uint_t max_digits = (bit_count_v<long_uint_t> + 2) / 3;
    std::array<char_t, max_digits> input;
    const uint_t digit_count = impl::get_string(stream, input.data(), max_digits);

    value = 0;
    uint_t base = 10;

    if ((stream.flags() & std::ios::basefield) == std::ios::oct)
        base = 8;
    else if ((stream.flags() & std::ios::basefield) == std::ios::hex)
        base = 16;

    for (uint_t digit_idx = 0; digit_idx < digit_count; ++digit_idx) {

        long_uint_t carry = impl::from_char(input[digit_idx]);
        carry = mulc(value, long_uint_t(base), carry);

        if (carry > 0)
            throw std::ios::failure("Input integer value is out of range.");
    }

    return stream;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename native_t, uint_t size, typename char_t, class traits_t>
inline std::basic_istream<char_t, traits_t>& operator>>(std::basic_istream<char_t, traits_t>& stream, long_int_t<native_t, size>& value)
{
    using long_uint_t = long_uint_t<native_t, size>;
    using long_int_t = long_int_t<native_t, size>;

    stream >> std::ws;
    const std::ios::int_type sign = stream.get();

    if (!stream.eof()) {

        if (sign != traits_t::to_int_type('-'))
            stream.unget();

        long_uint_t ui;
        stream >> ui;

        if (sign == traits_t::to_int_type('-'))
            value = -long_int_t(ui);
        else
            value = long_int_t(ui);
    }

    return stream;
}

namespace impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// standalone helper methods
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename char_t>
inline char_t to_char(uint_t digit, bool uppercase) noexcept
{
    assert(0 <= digit && digit < 16);
    constexpr auto upper_digits = std::basic_string_view<char>("0123456789ABCDEF");
    constexpr auto lower_digits = std::basic_string_view<char>("0123456789abcdef");

    if (uppercase)
        return upper_digits[digit];
    else
        return lower_digits[digit];
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename char_t>
inline uint_t from_char(char_t ch)
{
    if (ch >= char_t('0') && ch <= char_t('9'))
        return ch - char_t('0');
    if (ch >= char_t('A') && ch <= char_t('F'))
        return ch - char_t('A') + 10;
    if (ch >= char_t('a') && ch <= char_t('f'))
        return ch - char_t('a') + 10;

    throw std::ios::failure("Unexpected character in input stream.");
}



////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename char_t, class traits_t>
inline uint_t get_string(std::basic_istream<char_t, traits_t>& stream, char_t* str, uint_t size)
{
    stream >> std::ws;

    uint_t read_count = 0;
    const std::ctype<char_t>& facet = std::use_facet<std::ctype<char_t>>(stream.getloc());

    while (read_count < size && !stream.eof()) {

        const char_t ch = std::char_traits<char_t>::to_char_type(stream.get());
        
        if (facet.is(std::ctype<char_t>::space, ch)) {

            stream.unget();
            break;
        }

        str[read_count++] = ch;
    }

    return read_count;
}

} // namespace impl
} // namespace slim

////////////////////////////////////////////////////////////////////////////////////////////////////
// End of long_io.h
////////////////////////////////////////////////////////////////////////////////////////////////////
