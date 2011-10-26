/*
 * simple_parser.hpp - Parser that only splits the command arguments and
 *                     supports quoted strings
 *
 *   Copyright 2010-2011 Jesús Torres <jmtorres@ull.es>
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

#include <iostream>
#include <string>
#include <vector>

#include <boost/spirit/include/qi.hpp>

#include <cli/boost_parser_base.hpp>

namespace cli { namespace parser { namespace simpleparser
{
    namespace qi = boost::spirit::qi;
    namespace iso8859_1 = boost::spirit::iso8859_1;

    typedef std::vector<std::string> CommandArguments;

    //
    // Class SimpleParser
    //
    // The parser uses ISO-8859 encoding to avoid problems with UTF-8 strings
    // because the Boost.Spirit support of ASCII encoding launches an
    // exceptions when finds 8-bit characters.
    //

    struct SimpleParser
        : BoostParserBase<std::string, CommandArguments, iso8859_1::space_type>
    {
        SimpleParser();

        //
        // Parser rules
        //

        qi::rule<IteratorType> eol;
        qi::rule<IteratorType, char()> character;
        qi::rule<IteratorType, char()> escape;
        qi::rule<IteratorType, std::string(), iso8859_1::space_type> word;
        qi::rule<IteratorType, std::string(),
            iso8859_1::space_type> quotedString;
        qi::rule<IteratorType, std::string(),
            iso8859_1::space_type> doubleQuotedString;
        qi::rule<IteratorType, std::string(), iso8859_1::space_type> argument;
        qi::rule<IteratorType, SimpleParser::sig_type,
            iso8859_1::space_type> start;

        private:

            static void throwParserError(IteratorType const& first,
                IteratorType const& last, IteratorType const& error,
                const boost::spirit::info& info);
    };
}}}

namespace cli { namespace parser
{
    using simpleparser::SimpleParser;
}}

#endif /* SIMPLE_PARSER_HPP_ */
