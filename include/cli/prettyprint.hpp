/*
 * prettyprint.hpp - Pretty-print output stream manipulator
 *
 *   Copyright 2010-2012 Jesús Torres <jmtorres@ull.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PRETTYPRINT_HPP_
#define PRETTYPRINT_HPP_

#include <algorithm>
#include <ostream>
#include <string>

#include <cli/chart_literal.hpp>
#include <cli/detail/prettyprint.hpp>

namespace cli { namespace prettyprint
{
    //
    // Function to check if prettyprint mode was enabled for the stream
    //

    template <typename CharT, typename Traits>
    bool isPrettyprintEnabled(std::basic_ostream<CharT, Traits>& os)
    {
        return (os.iword(detail::PRETTYPRINT_ENABLED_FLAG_INDEX) != 0);
    }

    //
    // Manipulators to enable and disable prettyprint mode
    //

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& prettyprint(
        std::basic_ostream<CharT, Traits>& os)
    {
        os.iword(detail::PRETTYPRINT_ENABLED_FLAG_INDEX) = true;
        os.iword(detail::INDENT_SPACE_INDEX) = 0;
        os.iword(detail::INDENT_WIDTH_INDEX) = detail::INDENT_DEFAULT_WIDTH;
        return os;
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& noprettyprint(
        std::basic_ostream<CharT, Traits>& os)
    {
        os.iword(detail::PRETTYPRINT_ENABLED_FLAG_INDEX) = false;
        return os;
    }

    //
    // Manipulators to change indenting
    //

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& indent(
        std::basic_ostream<CharT, Traits>& os)
    {
        os.iword(detail::INDENT_SPACE_INDEX) +=
            os.iword(detail::INDENT_WIDTH_INDEX);
        return os;
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& deindent(
        std::basic_ostream<CharT, Traits>& os)
    {
        int space = os.iword(detail::INDENT_SPACE_INDEX) -
            os.iword(detail::INDENT_WIDTH_INDEX);
        os.iword(detail::INDENT_SPACE_INDEX) = std::max(space, 0);
        return os;
    }

    //
    // Parameterized manipulator to change the defined indentation width
    //

    template <typename CharT, typename Traits>
    class PrettyprintIndentWidthSetter
    {
        public:
            PrettyprintIndentWidthSetter(int width)
                : width_(width)
            {}

            void operator()(std::basic_ostream<CharT, Traits>& os) const
                { os.iword(detail::INDENT_WIDTH_INDEX) = width_; }

        private:
            int width_;
    };

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const PrettyprintIndentWidthSetter<CharT, Traits>& setter)
    {
        setter(os);
        return os;
    }

    template <typename CharT, typename Traits>
    PrettyprintIndentWidthSetter<CharT, Traits> setIndentWidth(int width)
    {
        return PrettyprintIndentWidthSetter<CharT, Traits>(width);
    }

    //
    // Manipulator to insert an end-of-line and indent the next line
    //

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& endl(
        std::basic_ostream<CharT, Traits>& os)
    {
        os << std::endl;
        if (isPrettyprintEnabled(os)) {
            os << std::basic_string<CharT>(
                os.iword(detail::INDENT_SPACE_INDEX),
                CHART_LITERAL(CharT, ' '));
        }
        return os;
    }

    //
    // Manipulator to insert an end-of-line, increment the indenting and
    // indent the next line
    //

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& endlAndIndent(
        std::basic_ostream<CharT, Traits>& os)
    {
        os << indent << endl;
        return os;
    }

    //
    // Manipulator to insert an end-of-line, decrement the indenting and
    // indent the next line
    //

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& endlAndDeindent(
        std::basic_ostream<CharT, Traits>& os)
    {
        os << deindent << endl;
        return os;
    }
}}

#endif /* PRETTYPRINT_HPP_ */
