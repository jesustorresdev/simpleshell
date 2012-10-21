/*
 * words.hpp - Interpreter that only splits the command arguments in words.
 *             It supports quoted strings
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

#ifndef WORDS_HPP_
#define WORDS_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/include/qi.hpp>

#include <cli/auxiliary.hpp>
#include <cli/basic_spirit.hpp>

namespace cli { namespace parser { namespace wordsparser
{
    namespace qi = boost::spirit::qi;
    namespace iso8859_1 = boost::spirit::iso8859_1;
    namespace fusion = boost::fusion;

    typedef std::vector<std::string> CommandArguments;

    //
    // Class WordsParser
    //
    // The parser must return a two-references Sequence. They have to refer to
    // the name and the arguments of parsed command respectively.
    //
    // The parser uses ISO-8859 encoding to avoid problems with UTF-8 strings
    // because the Boost.Spirit support of ASCII encoding launches an
    // exceptions when finds 8-bit characters.
    //

    template <typename Iterator>
    struct WordsParser
        : qi::grammar<Iterator,
              fusion::vector<std::string&, CommandArguments&>(),
              iso8859_1::space_type>
    {
        WordsParser();

        //
        // Parser rules
        //

        qi::rule<Iterator> eol;
        qi::rule<Iterator, char()> character;
        qi::rule<Iterator, char()> escape;
        qi::rule<Iterator, std::string(), iso8859_1::space_type> word;
        qi::rule<Iterator, std::string(), iso8859_1::space_type> quotedString;
        qi::rule<Iterator, std::string(),
            iso8859_1::space_type> doubleQuotedString;
        qi::rule<Iterator, std::string(), iso8859_1::space_type> argument;
        qi::rule<Iterator, fusion::vector<std::string&, CommandArguments&>(),
            iso8859_1::space_type> start;
    };
}}}

namespace cli
{
    using namespace cli::parser::wordsparser;

    typedef CommandArguments WordsArguments;

    //
    // Class WordsInterpreter
    //
    // Interpreter which uses WordsParser to parse the command line.
    //

    class WordsInterpreter
    : public BasicSpiritInterpreter<WordsArguments, WordsParser>
    {
        public:
            typedef BasicSpiritInterpreter<CommandArguments, WordsParser>
                BaseType;

            WordsInterpreter(bool useReadline = true);
            WordsInterpreter(std::istream& in, std::ostream& out,
                std::ostream& err, bool useReadline = true);
    };
}

#endif /* WORDS_HPP_ */
