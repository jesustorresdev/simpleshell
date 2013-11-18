/*
 * basic_spirit.hpp - Adapter class for parsers based on Boost.Spirit
 *
 *   Copyright 2010-2013 Jesús Torres <jmtorres@ull.es>
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
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_expect.hpp>

#include <cli/base.hpp>

#define translate(str) str  // TODO: Use Boost.Locale when available

namespace cli { namespace parser { namespace spiritparser
{
    namespace qi = boost::spirit::qi;

    //
    // Class SpiritParseError
    //
    // Type used to return parse errors to the interpreter.
    //

    struct SpiritParseError
    {
        //
        // Class constructors
        //

        SpiritParseError();
        SpiritParseError(const std::string& what);
        SpiritParseError(
            const qi::expectation_failure<std::string::const_iterator>& e);

        const std::string& what() const
            { return what_; }

        //
        // These attributes will only contains valid values if
        // hasExpectationFailure() returns true. For a description of it,
        // see qi::expectation_failure (expectation) in Boost.Spirit
        // documentation.
        //

        const std::string::const_iterator& expectationFailureFirst() const
            { return expectationFailureFirst_; }
        const std::string::const_iterator& expectationFailureLast() const
            { return expectationFailureLast_; }
        const boost::spirit::info& expectationFailureWhat() const
            { return expectationFailureWhat_; }

        bool hasExpectationFailure() const
            { return expectationFailure_; }

        private:
            std::string what_;

            std::string::const_iterator expectationFailureFirst_;
            std::string::const_iterator expectationFailureLast_;
            boost::spirit::info expectationFailureWhat_;
            bool expectationFailure_;
    };

    //
    // Class BasicSpiritParser
    //
    // Adapter class template for parsers based on Boost.Spirit.
    //

    template <typename Arguments, template <typename> class Grammar>
    class BasicSpiritParser
    {
        public:
            typedef BasicSpiritParser<Arguments, Grammar> Type;
            typedef Grammar<std::string::const_iterator> GrammarType;

            BasicSpiritParser()
                : grammar_(new GrammarType)
            {}

            BasicSpiritParser(boost::shared_ptr<GrammarType> grammar)
                : grammar_(grammar)
            {}

            bool operator()(std::string::const_iterator& begin,
                std::string::const_iterator end, std::string& command,
                Arguments& arguments, SpiritParseError& error)
            {
                // Passing the attributes 'command' and 'arguments' to the
                // parser forces that every valid grammar must return a
                // two-references Sequence.
                try {
                    bool success = qi::phrase_parse(begin, end, *grammar_,
                        skipper_, command, arguments);
                    if (success) {
                        return true;
                    }
                    error = SpiritParseError(translate("syntax error"));
                    return false;
                }
                catch (const qi::expectation_failure<
                    std::string::const_iterator>& e)
                {
                    error = SpiritParseError(e);
                    return false;
                }
            }

        private:
            typename GrammarType::skipper_type skipper_;

            boost::shared_ptr<GrammarType> grammar_;
            SpiritParseError parseError_;
    };
}}}

namespace cli
{
    using namespace cli::parser::spiritparser;

    //
    // Class BasicSpiritInterpreter
    //
    // Template for command interpreters which uses parsers based
    // on Boost.Spirit.
    //

    template <typename Arguments, template <typename> class Grammar>
    struct BasicSpiritInterpreter
        : public CommandLineInterpreterBase<
              BasicSpiritParser<Arguments, Grammar> >
    {
        typedef BasicSpiritParser<Arguments, Grammar> SpiritParserType;
        typedef typename SpiritParserType::GrammarType SpiritGrammarType;

        typedef CommandLineInterpreterBase<SpiritParserType> BaseType;

        BasicSpiritInterpreter(bool useReadline = true)
            : BaseType(useReadline)
        {}

        BasicSpiritInterpreter(std::istream& in, std::ostream& out,
            std::ostream& err = std::cerr, bool useReadline = true)
            : BaseType(in, out, err, useReadline)
        {}

        BasicSpiritInterpreter(boost::shared_ptr<SpiritGrammarType> grammar,
            bool useReadline = true)
            : BaseType(boost::shared_ptr<SpiritParserType>(
                new SpiritParserType(grammar)), useReadline)
        {}

        BasicSpiritInterpreter(boost::shared_ptr<SpiritGrammarType> grammar,
            std::istream& in, std::ostream& out, std::ostream& err = std::cerr,
            bool useReadline = true)
            : BaseType(boost::shared_ptr<SpiritParserType>(
                new SpiritParserType(grammar)), in, out, err, useReadline)
        {}
    };
}

#endif /* BASIC_SPIRIT_HPP_ */
