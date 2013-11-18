/*
 * basic_spirit.cpp - Adapter class for parsers based on Boost.Spirit
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

#include <string>

#include <boost/spirit/include/qi_expect.hpp>

#define translate(str) str  // TODO: Use Boost.Locale when available

#include <cli/basic_spirit.hpp>

namespace cli { namespace parser { namespace spiritparser
{
    //
    // Class SpiritParseError
    //
    // Type used to return parse errors to the interpreter.
    //

    SpiritParseError::SpiritParseError()
        : expectationFailureWhat_(""),
          expectationFailure_(false)
    {}

    SpiritParseError::SpiritParseError(const std::string& what)
        : what_(what),
          expectationFailureWhat_(""),
          expectationFailure_(false)
    {}

    SpiritParseError::SpiritParseError(
        const qi::expectation_failure<std::string::const_iterator>& e)
        : what_(),
          expectationFailureFirst_(e.first),
          expectationFailureLast_(e.last),
          expectationFailureWhat_(e.what_),
          expectationFailure_(true)
    {
        what_ += translate("syntax error, expecting");
        what_ += " " + e.what_.tag + " " + translate("at") + ": ";
        what_ += (e.first == e.last) ? translate("<end-of-line>") :
            std::string(e.first, e.last);
    }
}}}
