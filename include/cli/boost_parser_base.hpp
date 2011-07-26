/*
 * boost_parser_base.hpp - Base class for parsers based on Boost.Spirit
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

    template <typename Command, typename Details,
        typename T1 = unused_type, typename T2 = unused_type,
        typename T3 = unused_type>
    struct BoostParserBase
        : qi::grammar<typename Command::iterator,
              fusion::vector<Command&, Details&>(), T1, T2, T3>
    {
        typedef BoostParserBase<Command, Details, T1, T2, T3> Type;

        typedef typename Command::iterator IteratorType;
        typedef typename qi::grammar<IteratorType,
            fusion::vector<Command&, Details&>(), T1, T2, T3> ParserType;
        typedef bool ParserErrorType;

        //
        // Define the types needed to replace qi::grammar by BoostParserBase
        // in derived classes more easier.
        //

        typedef Type base_type;
        typedef typename Type::skipper_type skipper_type;
        typedef typename Type::start_type start_type;

        //
        // Define the members needed to model STL function object concepts.
        //

        typedef ParserErrorType result_type;
        typedef IteratorType& arg1_type;
        typedef IteratorType arg2_type;
        typedef Command& arg3_type;
        typedef Details& arg4_type;

        ParserErrorType operator()(IteratorType& begin, IteratorType end,
            Command& command, Details& details)
            { return parse(begin, end, command, details); }

        protected:

            virtual ParserErrorType parse(IteratorType& begin,
                IteratorType end, Command& command, Details& details)
            {
                // Passing the attributes 'command' and 'details' to the parser
                // forces that every valid grammar must to return a two
                // references Sequence.
                return !qi::phrase_parse(begin, end, *this, skipper_type(),
                    command, details);
            }

            //
            // Protect the constructor to avoid class instantiations
            //

            BoostParserBase(start_type const& start) : ParserType(start) {};
    };
}}

#endif /* BOOST_PARSER_BASE_HPP_ */
