/*
 * chart.hpp - Macro to define literal characters and strings that are
 *             independent from a character type argument in a template
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

#ifndef CHART_LITERAL_HPP_
#define CHART_LITERAL_HPP_

#define CHART_LITERAL(TYPE, LITERAL)    \
    cli::utility::CharTLiteralSelector<TYPE>::select(LITERAL, L ## LITERAL)

namespace cli { namespace utility
{
    template <typename CharT>
    struct CharTLiteralSelector;

    template <>
    struct CharTLiteralSelector<char>
    {
        static const char* select(const char* s, const wchar_t*)
            { return s; }

        static char select(char c, wchar_t)
            { return c; }
    };

    template <>
    struct CharTLiteralSelector<wchar_t>
    {
        static const wchar_t* select(const char*, const wchar_t* s)
            { return s; }

        static wchar_t select(char, wchar_t c)
            { return c; }
    };
}}

#endif /* CHART_LITERAL_HPP_ */
