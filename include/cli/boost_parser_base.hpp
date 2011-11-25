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

#include <stdexcept>

#include <boost/fusion/include/unused.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/include/qi.hpp>

#define translate(str) str  // TODO: Use Boost.Locale when available

namespace cli { namespace parser
{
    namespace qi = boost::spirit::qi;
    namespace fusion = boost::fusion;

    using fusion::unused_type;
    using fusion::unused;

    //
    // Class BoostParserError
    //
    // Type used to return parser errors to the interpreter.
    //

    template <typename Iterator>
    struct BoostParserError : public std::runtime_error
    {
        //
        // This attributes are only available if containsDetails() returns
        // true. For a description of them, see qi::on_error (error handling)
        // in Boost.Spirit documentation.
        //

        Iterator first;
        Iterator last;
        Iterator error;
        boost::spirit::info what_;

        //
        // Class constructors
        //

        BoostParserError(bool fail = false)
            : runtime_error(""), what_(""), parserFail_(fail),
              containsDetails_(false) {}

        BoostParserError(const char* what)
            : runtime_error(std::string(what)), what_(""), parserFail_(true),
              containsDetails_(false){}

        BoostParserError(const std::string& what)
            : runtime_error(what), what_(""), parserFail_(true),
              containsDetails_(false) {}

        BoostParserError(const std::string& what, Iterator const& first,
            Iterator const& last, Iterator const& error,
            const boost::spirit::info& info)
            : runtime_error(what), first(first), last(last),
              error(error), what_(info), parserFail_(true),
              containsDetails_(true){}

        virtual ~BoostParserError() throw() {}

        //
        // The interpreter requires the type can be convertible to bool
        //

        operator bool()
            { return parserFail_; }

        bool containsDetails()
            { return containsDetails_; }

        private:
            bool parserFail_;
            bool containsDetails_;
    };

    //
    // Overload insertion operator (<<) for class BoostParserError.
    // It is use by the interpreter to show a message when parser fail.
    //

    template <typename CharT, typename Traits, typename Iterator>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& out,
        const BoostParserError<Iterator>& error)
    {
        return out << error.what();
    }

    //
    // Class BoostParserBase
    //
    // Base class for parsers based on Boost.Spirit.
    //

    template <typename Command, typename Arguments,
        typename T1 = unused_type, typename T2 = unused_type,
        typename T3 = unused_type>
    struct BoostParserBase
        : qi::grammar<typename Command::iterator,
              fusion::vector<Command&, Arguments&>(), T1, T2, T3>
    {
        typedef BoostParserBase<Command, Arguments, T1, T2, T3> Type;

        typedef typename Command::iterator IteratorType;
        typedef typename qi::grammar<IteratorType,
            fusion::vector<Command&, Arguments&>(), T1, T2, T3> ParserType;
        typedef BoostParserError<IteratorType> ParserErrorType;

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
        typedef Arguments& arg4_type;

        ParserErrorType operator()(IteratorType& begin, IteratorType end,
            Command& command, Arguments& arguments)
            { return parse(begin, end, command, arguments); }

        protected:

            virtual ParserErrorType parse(IteratorType& begin,
                IteratorType end, Command& command, Arguments& arguments)
            {
                // Passing the attributes 'command' and 'arguments' to the
                // parser forces that every valid grammar must to return a two
                // references Sequence.
                try {
                    bool success = qi::phrase_parse(begin, end, *this,
                        skipper_type(), command, arguments);
                    if (success) {
                        return ParserErrorType();
                    }
                    else {
                        return ParserErrorType(translate("syntax error"));
                    }
                }
                catch (ParserErrorType error) {
                    return error;
                }
            }

            static void throwParserError(const std::string& what,
                IteratorType const& first, IteratorType const& last,
                IteratorType const& error, const boost::spirit::info& info)
            {
                throw ParserErrorType(what, first, last, error, info);
            }

            //
            // Protect the constructor to avoid class instantiations
            //

            BoostParserBase(start_type const& start) : ParserType(start) {};
    };
}}

#endif /* BOOST_PARSER_BASE_HPP_ */
