/*
 * boost_parser_base.hpp - Base class for parsers based on boost::spirit
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

#ifndef BOOST_PARSER_BASE_HPP_
#define BOOST_PARSER_BASE_HPP_

#include <string>

#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/include/qi.hpp>

namespace cli { namespace parser
{
    namespace qi = boost::spirit::qi;
    namespace fusion = boost::fusion;

    using fusion::unused_type;
    using fusion::unused;

    //
    // class BoostParserBase
    //

    template <typename Iterator, typename Details,
        typename T1 = unused_type, typename T2 = unused_type,
        typename T3 = unused_type>
    struct BoostParserBase
        : qi::grammar<Iterator, fusion::vector<std::string&, Details&>(),
                      T1, T2, T3>
    {
        typedef BoostParserBase<Iterator, Details, T1, T2, T3> Type;

        typedef Details CommandDetailsType;
        typedef typename qi::grammar<Iterator,
            fusion::vector<std::string&, Details&>(), T1, T2, T3> ParserType;

        //
        // Declare the typedefs needed to replace qi::grammar by
        // BoostParserBase more easier.
        //

        typedef Type base_type;
        typedef typename Type::skipper_type skipper_type;
        typedef typename Type::start_type start_type;

        //
        // The interpreter expects the parser is a callable object
        //

        bool operator()(Iterator& begin, Iterator end, std::string& command,
            Details& details)
            { return parse(begin, end, command, details); }

        protected:

            virtual bool parse(Iterator& begin, Iterator end,
                std::string& command, Details& details)
            {
                // Passing the attributes 'command' and 'details' to the parser
                // forces that every valid grammar must to return a two
                // references Sequence.
                return qi::phrase_parse(begin, end, *this, skipper_type(),
                    command, details);
            }

            //
            // Protect the constructor to avoid class instantiations
            //

            BoostParserBase(start_type const& start) : ParserType(start) {};
    };
}}

#endif /* BOOST_PARSER_BASE_HPP_ */
