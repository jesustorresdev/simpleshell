/*
 * basic_spirit.cpp - Support for command interpreters which uses parsers
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

#include <cli/basic_spirit.hpp>

namespace cli
{
    //
    // Class SpiritParseError
    //
    // Type used to return parse errors to the interpreter.
    //

    SpiritParseError::SpiritParseError(bool fail)
        : ParseError(fail),
          expectationFailureWhat(""),
          expectationFailure_(false)
    {
        what_ = fail ? translate("syntax error") : "";
    }

    SpiritParseError::SpiritParseError(const std::string& what)
        : ParseError(what),
          expectationFailureWhat(""),
          expectationFailure_(false)
    {}

    SpiritParseError::SpiritParseError(
        const qi::expectation_failure<std::string::const_iterator>& e)
        : ParseError(true),
          expectationFailureFirst(e.first),
          expectationFailureLast(e.last),
          expectationFailureWhat(e.what_),
          expectationFailure_(true)
    {
        what_ += translate("syntax error, expecting");
        what_ += " " + e.what_.tag + " " + translate("at") + ": ";
        what_ += (e.first == e.last) ? translate("<end-of-line>") :
            std::string(e.first, e.last);
    }

    SpiritParseError& SpiritParseError::operator= (
        const SpiritParseError& other)
    {
        fail_ = other.fail_;
        what_ = other.what_;
        expectationFailure_ = other.expectationFailure_;
        return *this;
    }
}
