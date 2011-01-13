/*
* simple_parser.hpp - Parser that only splits the command arguments and
 *                     supports quoted strings
 *
 *   Copyright 2010 Jesús Torres <jmtorres@ull.es>
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

#ifndef SIMPLE_PARSER_HPP_
#define SIMPLE_PARSER_HPP_

#include <ostream>
#include <string>
#include <vector>

#include <libintl.h>    // TODO: To use Boost.Locale when available
#define translate(str) ::gettext(str)

//#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/operator/if_else.hpp>
#include <boost/spirit/include/qi.hpp>

namespace cli { namespace parsers
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    //
    // Class SimpleParser
    //

    template<typename Iterator>
    struct SimpleParser
        : qi::grammar<Iterator, std::vector<std::string>(), ascii::space_type>
    {
        SimpleParser() : SimpleParser::base_type(start)
        {
            using qi::_2;
            using qi::_3;
            using qi::_4;
            using qi::eoi;
            using qi::fail;
            using qi::lexeme;
            using qi::on_error;
            using ascii::char_;
            using ascii::space;
            using phoenix::construct;
            using phoenix::val;

            eol = eoi;
            character %= char_;
            escape %= '\\' > character;
            word %= lexeme[+(escape | (char_ - space))];
            quotedString %= lexeme['\'' >> *(char_ - '\'') > '\''];
            doubleQuotedString %= lexeme['"' >> *(char_ - '"') > '"'];
            argument %= quotedString | doubleQuotedString | word;
            start = +argument > eol;

            character.name(translate("character"));
            eol.name(translate("end-of-line"));

            on_error<fail>(
                start,
                std::cerr
                    << val(translate("parse error, expecting"))
                    << val(" ")
                    << _4
                    << val(" ")
                    << val(translate("at"))
                    << val(": ")
                    << if_else(_3 == _2, val(translate("<end-of-line>")),
                        construct<std::string>(_3, _2))
                    << std::endl
            );

//            BOOST_SPIRIT_DEBUG_NODE(word);
//            BOOST_SPIRIT_DEBUG_NODE(quotedString);
//            BOOST_SPIRIT_DEBUG_NODE(doubleQuotedString);
//            BOOST_SPIRIT_DEBUG_NODE(argument);
            BOOST_SPIRIT_DEBUG_NODE(start);
        }

        qi::rule<Iterator> eol;
        qi::rule<Iterator, char()> character;
        qi::rule<Iterator, char()> escape;
        qi::rule<Iterator, std::string(), ascii::space_type> word;
        qi::rule<Iterator, std::string(), ascii::space_type> quotedString;
        qi::rule<Iterator, std::string(), ascii::space_type> doubleQuotedString;
        qi::rule<Iterator, std::string(), ascii::space_type> argument;
        qi::rule<Iterator, std::vector<std::string>(), ascii::space_type> start;
    };
}}

#endif /* SIMPLE_PARSER_HPP_ */
