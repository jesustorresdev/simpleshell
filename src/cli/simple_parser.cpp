/*
 * simple_parser.cpp - Parser that only splits the command arguments and
 *                     supports quoted strings
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

//#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>

#define translate(str) str  // TODO: Use Boost.Locale when available

#include <cli/boost_parser_adapter.hpp>
#include <cli/simple_parser.hpp>

namespace cli { namespace parser { namespace simpleparser
{
    namespace qi = boost::spirit::qi;
    namespace iso8859_1 = boost::spirit::iso8859_1;
    namespace phoenix = boost::phoenix;

    //
    // Class SimpleParserImpl
    //
    // The parser uses ISO-8859 encoding to avoid problems with UTF-8 strings
    // because the Boost.Spirit support of ASCII encoding launches an
    // exceptions when finds 8-bit characters.
    //

    template <typename Iterator>
    SimpleParserImpl<Iterator>::SimpleParserImpl()
        : SimpleParserImpl::base_type(start)
    {
        using qi::_1;
        using qi::_2;
        using qi::_3;
        using qi::_4;
        using qi::_val;
        using qi::eoi;
        using qi::fail;
        using qi::lexeme;
        using qi::on_error;
        using iso8859_1::char_;
        using iso8859_1::space;
        using phoenix::at;
        using phoenix::at_c;
        using phoenix::bind;

        eol = eoi;
        character %= char_;
        escape %= '\\' > character;
        word %= lexeme[+(escape | (char_ - space))];
        quotedString %= lexeme['\'' >> *(char_ - '\'') > '\''];
        doubleQuotedString %= lexeme['"' >> *(char_ - '"') > '"'];
        argument %= quotedString | doubleQuotedString | word;
        start = (+argument) [at_c<1>(_val) = _1,
                             at_c<0>(_val) = at(_1, 0)] > eol;

        character.name(translate("character"));
        eol.name(translate("end-of-line"));

        on_error<fail>(
            start, bind(&SimpleParserImpl::throwParserError, _1, _2, _3, _4)
        );

//      BOOST_SPIRIT_DEBUG_NODE(word);
//      BOOST_SPIRIT_DEBUG_NODE(quotedString);
//      BOOST_SPIRIT_DEBUG_NODE(doubleQuotedString);
//      BOOST_SPIRIT_DEBUG_NODE(argument);
        BOOST_SPIRIT_DEBUG_NODE(start);
    }

    template <typename Iterator>
    void SimpleParserImpl<Iterator>::throwParserError(const Iterator& first,
        const Iterator& last, const Iterator& error,
        const boost::spirit::info& info)
    {
        std::string what;
        what += translate("syntax error, expecting");
        what += " " + info.tag + " " + translate("at") + ": ";
        what += (error == last) ? translate("<end-of-line>")
            : std::string(error, last);

        throw BoostParserError<Iterator>(what, first, last, error, info);
    }

    //
    // Explicit instantiations of SimpleParserImpl class
    //

    template class SimpleParserImpl<std::string::const_iterator>;
}}}
