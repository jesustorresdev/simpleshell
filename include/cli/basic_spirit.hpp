/*
 * basic_spirit.hpp - Support for command interpreters which uses parsers
 *                    based on Boost.Spirit
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

#ifndef BASIC_SPIRIT_HPP_
#define BASIC_SPIRIT_HPP_

#include <iostream>
#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_expect.hpp>

#include <cli/base.hpp>

#define translate(str) str  // TODO: Use Boost.Locale when available

namespace cli
{
    namespace qi = boost::spirit::qi;

    //
    // Class SpiritParseError
    //
    // Type used to return parse errors to the interpreter.
    //

    struct SpiritParseError : public ParseError
    {
        //
        // These attributes will only contains valid values if
        // hasExpectationFailure() returns true. For a description of it,
        // see qi::expectation_failure (expectation) in Boost.Spirit
        // documentation.
        //

        std::string::const_iterator expectationFailureFirst;
        std::string::const_iterator expectationFailureLast;
        boost::spirit::info expectationFailureWhat;

        //
        // Class constructors
        //

        SpiritParseError(bool fail = false);
        SpiritParseError(const std::string& what);
        SpiritParseError(
            const qi::expectation_failure<std::string::const_iterator>& e);

        ~SpiritParseError() throw() {}

        SpiritParseError& operator= (const SpiritParseError& other);

        bool hasExpectationFailure() const
            { return expectationFailure_; }

        protected:
            bool expectationFailure_;
    };

    //
    // Class BasicSpiritInterpreter
    //
    // Template for command interpreters which uses parsers based
    // on Boost.Spirit.
    //

    template <typename Arguments, template <typename> class Parser>
    class BasicSpiritInterpreter : public CommandLineInterpreterBase<Arguments>
    {
        public:
            typedef BasicSpiritInterpreter<Arguments, Parser> Type;
            typedef Parser<std::string::const_iterator> ParserType;

            BasicSpiritInterpreter(boost::shared_ptr<ParserType> parser,
                bool useReadline = true)
                : CommandLineInterpreterBase<Arguments>(useReadline),
                  parser_(parser)
            {}

            BasicSpiritInterpreter(boost::shared_ptr<ParserType> parser,
                std::istream& in, std::ostream& out,
                std::ostream& err, bool useReadline = true)
                : CommandLineInterpreterBase<Arguments>(in, out, err,
                    useReadline),
                  parser_(parser)
           {}

        private:
            boost::shared_ptr<ParserType> parser_;
            typename ParserType::skipper_type skipperParser_;
            SpiritParseError parseError_;

            virtual SpiritParseError& parse(std::string::const_iterator& begin,
                std::string::const_iterator end, std::string& command,
                Arguments& arguments)
            {
                // Passing the attributes 'command' and 'arguments' to the
                // parser forces that every valid grammar must return a
                // two-references Sequence.
                try {
                    bool success = qi::phrase_parse(begin, end, *parser_,
                        skipperParser_, command, arguments);
                    parseError_ = SpiritParseError(!success);
                }
                catch (qi::expectation_failure<std::string::const_iterator> e)
                {
                    parseError_ = SpiritParseError(e);
                }
                return parseError_;
            }
    };
}

#endif /* BASIC_SPIRIT_HPP_ */
